#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

extern "C"
{
#include <tizen.h>
#include <service_app.h>
#include <app_event.h>
#include <notification_list.h>
#include <notification_text_domain.h>
#include <notification_internal.h>
#include <vconf.h>
#include <call-manager.h>
}

#include "msg.h"
#include "msg_storage.h"

#include "msg-manager-contact.h"
#include "msg-manager-debug.h"
#include "msg-manager-notification.h"
#include "msg-manager-sound.h"
#include "msg-manager-util.h"

/* below defines will be removed */
#define EVENT_KEY_OUT_MSG_TYPE "msg_type"
#define EVENT_KEY_OUT_MSG_ID "msg_id"

msg_handle_t msg_handle = NULL;
cm_client_h cm_handle = NULL;

pthread_mutex_t mx;
static int job_cnt = 0;
static bool terminated = false;

static int _service_app_exit(void *data)
{
	MSG_MGR_INFO("kill msg-manager");
	service_app_exit();

	return 0;
}

static int _check_app_terminate(void *data)
{
	pthread_mutex_lock(&mx);

	job_cnt--;
	MSG_MGR_DEBUG("job_cnt [%d]", job_cnt);
	if (job_cnt == 0) {
		terminated = true;
		g_idle_add(_service_app_exit, NULL);
	}

	pthread_mutex_unlock(&mx);

	return 0;
}

bool service_app_create(void *data)
{
	MSG_MGR_INFO("app_create");

	int msg_server_ready = 0;
	for (int i = 0; i < 100; i++) {
		vconf_get_bool(VCONFKEY_MSG_SERVER_READY, &msg_server_ready);
		if (msg_server_ready == 1) {
			int msg_err = msg_open_msg_handle(&msg_handle);
			if (msg_err != MSG_SUCCESS)
				MSG_MGR_DEBUG("msg_open_msg_handle() failed [%d]", msg_err);
			else
				MSG_MGR_DEBUG("msg_open_msg_handle() success");

			break;
		} else {
			MSG_MGR_DEBUG("msg-server is not ready.");
			sleep(1);
		}
	}

	MsgMgrInitNoti();
	initMsgMgrSoundPlayer();
	cm_init(&cm_handle);

	return true;
}

void service_app_terminate(void *data)
{
	MSG_MGR_INFO("app_terminate");

	cm_deinit(cm_handle);
	MsgMgrCloseContactSvc();
	msg_close_msg_handle(&msg_handle);

	return;
}

void _incoming_msg_func(app_control_h app_control)
{
	MSG_MGR_BEGIN();

	int ret = 0;
	char *rcv_msg_type = NULL;
	char *rcv_msg_id = NULL;

	ret = app_control_get_extra_data(app_control, EVENT_KEY_MSG_ID, &rcv_msg_id);
	if (ret != APP_CONTROL_ERROR_NONE || rcv_msg_id == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	ret = app_control_get_extra_data(app_control, EVENT_KEY_MSG_TYPE, &rcv_msg_type);
	if (ret != APP_CONTROL_ERROR_NONE || rcv_msg_type == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		g_free(rcv_msg_id);
		return;
	}

	MSG_MGR_INFO("rcv_msg_type(%s), rcv_msg_id(%s)", rcv_msg_type, rcv_msg_id);

	int msg_err = MSG_SUCCESS;
	msg_message_id_t msg_id = atoi(rcv_msg_id);
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	contactInfo contact_info = {0,};
	contact_info.msgId = msg_id;

	msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	opt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_err = msg_get_message(msg_handle, msg_id, msg, opt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_get_message() failed [%d]", msg_err);
		return;
	}

	msg_get_int_value(msg, MSG_MESSAGE_TYPE_INT, &contact_info.msgType);
	msg_get_int_value(msg, MSG_MESSAGE_FOLDER_ID_INT, &contact_info.folderId);
	msg_get_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, &contact_info.simIndex);
	msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&contact_info.addrList);
	msg_get_str_value(msg, MSG_MESSAGE_SUBJECT_STR, contact_info.subject, MAX_CONTACT_TEXT_LEN);
	int msgSize = 0;
	msg_get_int_value(msg, MSG_MESSAGE_DATA_SIZE_INT, &msgSize);
	if (msgSize > 0)
		msg_get_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, contact_info.msgText, MAX_CONTACT_TEXT_LEN);

	if ((contact_info.folderId == MSG_INBOX_ID || contact_info.folderId == MSG_SPAMBOX_ID)) {
		MsgMgrAddPhoneLog(&contact_info);
	}

	msg_release_struct(&msg);
	msg_release_struct(&opt);

	g_free(rcv_msg_id);
	g_free(rcv_msg_type);

	MSG_MGR_END();
}

void _outgoing_msg_func(app_control_h app_control)
{
	MSG_MGR_BEGIN();

	int ret = 0;
	char *sent_msg_type = NULL;
	char *sent_msg_id = NULL;

	ret = app_control_get_extra_data(app_control, EVENT_KEY_OUT_MSG_ID, &sent_msg_id);
	if (ret != APP_CONTROL_ERROR_NONE || sent_msg_id == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	ret = app_control_get_extra_data(app_control, EVENT_KEY_OUT_MSG_TYPE, &sent_msg_type);
	if (ret != APP_CONTROL_ERROR_NONE || sent_msg_type == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		g_free(sent_msg_id);
		return;
	}

	MSG_MGR_INFO("sent_msg_type(%s) sent_msg_id(%s)", sent_msg_type, sent_msg_id);

	int msg_err = MSG_SUCCESS;
	msg_message_id_t msg_id = atoi(sent_msg_id);
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	contactInfo contact_info = {0,};
	contact_info.msgId = msg_id;

	msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	opt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_err = msg_get_message(msg_handle, msg_id, msg, opt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_get_message() failed [%d]", msg_err);
		return;
	}

	msg_get_int_value(msg, MSG_MESSAGE_TYPE_INT, &contact_info.msgType);
	msg_get_int_value(msg, MSG_MESSAGE_FOLDER_ID_INT, &contact_info.folderId);
	msg_get_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, &contact_info.simIndex);
	msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&contact_info.addrList);
	msg_get_str_value(msg, MSG_MESSAGE_SUBJECT_STR, contact_info.subject, 100);
	int msgSize = 0;
	msg_get_int_value(msg, MSG_MESSAGE_DATA_SIZE_INT, &msgSize);
	if (msgSize > 0)
		msg_get_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, contact_info.msgText, 100);

	MsgMgrAddPhoneLog(&contact_info);

	msg_release_struct(&msg);
	msg_release_struct(&opt);

	g_free(sent_msg_id);
	g_free(sent_msg_type);

	MSG_MGR_END();
}

void _refresh_noti_func(app_control_h app_control)
{
	char *type = NULL;
	char *feedback = NULL;
	char *active_type = NULL;
	msg_mgr_notification_type_t noti_type = MSG_MGR_NOTI_TYPE_NORMAL;
	bool bFeedback = true;
	msg_mgr_active_notification_type_t active_noti_type = MSG_MGR_ACTIVE_NOTI_TYPE_NONE;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		MSG_MGR_DEBUG("type [%s]", type);
		if (g_strcmp0(type, "normal") == 0)
			noti_type = MSG_MGR_NOTI_TYPE_NORMAL;

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	ret = app_control_get_extra_data(app_control, "feedback", &feedback);
	if (ret == APP_CONTROL_ERROR_NONE && feedback) {
		MSG_MGR_DEBUG("feedback [%s]", feedback);
		if (g_strcmp0(feedback, "false") == 0)
			bFeedback = false;
		else if (g_strcmp0(feedback, "true") == 0)
			bFeedback = true;

		g_free(feedback);
	}

	ret = app_control_get_extra_data(app_control, "active_type", &active_type);
	if (ret == APP_CONTROL_ERROR_NONE && active_type) {
		MSG_MGR_DEBUG("active_type [%s]", active_type);
		if (g_strcmp0(active_type, "none") == 0)
			active_noti_type = MSG_MGR_ACTIVE_NOTI_TYPE_NONE;
		else if (g_strcmp0(active_type, "active") == 0)
			active_noti_type = MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE;
		else if (g_strcmp0(active_type, "instant") == 0)
			active_noti_type = MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT;

		g_free(active_type);
	}

	MsgMgrRefreshNotification(noti_type, bFeedback, active_noti_type);
}

void _add_noti_func(app_control_h app_control)
{
	char *type;
	msg_mgr_notification_type_t noti_type = MSG_MGR_NOTI_TYPE_ALL;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		if (g_strcmp0(type, "voice1") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_VOICE_1;
		} else if (g_strcmp0(type, "voice2") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_VOICE_2;
		} else if (g_strcmp0(type, "mwi") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_MWI;
		} else if (g_strcmp0(type, "class0") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_CLASS0;
		}

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	char *msgId = NULL;
	ret = app_control_get_extra_data(app_control, "msg_id", &msgId);
	if (ret != APP_CONTROL_ERROR_NONE || msgId == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	int msg_err = MSG_SUCCESS;
	msg_message_id_t msg_id = atoi(msgId);
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	msg_list_handle_t addr_list = NULL;
	MSG_MGR_MESSAGE_INFO_S msg_info = {0,};
	msg_info.msgId = msg_id;

	msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	opt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_err = msg_get_message(msg_handle, msg_id, msg, opt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_get_message() failed [%d]", msg_err);
		return;
	}

	msg_get_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, &msg_info.sim_idx);
	msg_get_int_value(msg, MSG_MESSAGE_DISPLAY_TIME_INT, (int *)&msg_info.displayTime);
	msg_get_int_value(msg, MSG_MESSAGE_NETWORK_STATUS_INT, (int *)&msg_info.networkStatus);
	msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&addr_list);

	msg_struct_t addr_data = msg_list_nth_data(addr_list, 0);
	msg_get_str_value(addr_data, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, msg_info.addressVal, MAX_ADDRESS_VAL_LEN);
	msg_get_str_value(addr_data, MSG_ADDRESS_INFO_DISPLAYNAME_STR, msg_info.displayName, MAX_DISPLAY_NAME_LEN);

	if (noti_type == MSG_MGR_NOTI_TYPE_MWI || noti_type == MSG_MGR_NOTI_TYPE_CLASS0) {
		int msg_size = 0;
		msg_get_int_value(msg, MSG_MESSAGE_DATA_SIZE_INT, &msg_size);
		if (msg_size > 0)
			msg_get_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, msg_info.msgText, MAX_MSG_TEXT_LEN);
	}

	msg_release_struct(&msg);
	msg_release_struct(&opt);

	g_free(msgId);

	MsgMgrAddNotification(noti_type, &msg_info);
}

void _del_noti_func(app_control_h app_control)
{
	char *type;
	msg_mgr_notification_type_t noti_type = MSG_MGR_NOTI_TYPE_ALL;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		if (g_strcmp0(type, "all") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_ALL;
		} else if (g_strcmp0(type, "normal") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_NORMAL;
		} else if (g_strcmp0(type, "sim") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_SIM;
		} else if (g_strcmp0(type, "voice1") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_VOICE_1;
		} else if (g_strcmp0(type, "voice2") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_VOICE_2;
		}

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	char *simIndex = NULL;
	ret = app_control_get_extra_data(app_control, "sim_index", &simIndex);
	if (ret != APP_CONTROL_ERROR_NONE || simIndex == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	int sim_index = atoi(simIndex);

	MsgMgrDeleteNoti(noti_type, sim_index);

	g_free(simIndex);
}

void _add_report_noti_func(app_control_h app_control)
{
	char *type;
	msg_mgr_notification_type_t noti_type = MSG_MGR_NOTI_TYPE_ALL;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		if (g_strcmp0(type, "sms_delivery") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT;
		} else if (g_strcmp0(type, "mms_delivery") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT;
		} else if (g_strcmp0(type, "mms_read") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_MMS_READ_REPORT;
		}

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	char *msgId = NULL;
	ret = app_control_get_extra_data(app_control, "msg_id", &msgId);
	if (ret != APP_CONTROL_ERROR_NONE || msgId == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	int msg_err = MSG_SUCCESS;
	msg_message_id_t msg_id = atoi(msgId);
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	msg_list_handle_t addr_list = NULL;
	MSG_MGR_MESSAGE_INFO_S msg_info = {0,};
	msg_info.msgId = msg_id;

	msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	opt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_err = msg_get_message(msg_handle, msg_id, msg, opt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_get_message() failed [%d]", msg_err);
		return;
	}

	msg_get_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, &msg_info.sim_idx);
	msg_get_int_value(msg, MSG_MESSAGE_DISPLAY_TIME_INT, (int *)&msg_info.displayTime);
	msg_get_int_value(msg, MSG_MESSAGE_NETWORK_STATUS_INT, (int *)&msg_info.networkStatus);
	msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&addr_list);

	msg_struct_t addr_data = msg_list_nth_data(addr_list, 0);
	msg_get_str_value(addr_data, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, msg_info.addressVal, MAX_ADDRESS_VAL_LEN);
	msg_get_str_value(addr_data, MSG_ADDRESS_INFO_DISPLAYNAME_STR, msg_info.displayName, MAX_DISPLAY_NAME_LEN);

	msg_release_struct(&msg);
	msg_release_struct(&opt);

	g_free(msgId);

	MsgMgrAddReportNotification(noti_type, &msg_info);
}

void _del_report_noti_func(app_control_h app_control)
{
	char *addr = NULL;
	int ret = app_control_get_extra_data(app_control, "address", &addr);
	if (ret != APP_CONTROL_ERROR_NONE || addr == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	MsgMgrDeleteReportNotification(addr);

	g_free(addr);
}

void _insert_only_active_noti_func(app_control_h app_control)
{
	char *type;
	msg_mgr_notification_type_t noti_type = MSG_MGR_NOTI_TYPE_ALL;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		if (g_strcmp0(type, "normal") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_NORMAL;
		} else if (g_strcmp0(type, "class0") == 0) {
			noti_type = MSG_MGR_NOTI_TYPE_CLASS0;
		}

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	char *msgId = NULL;
	ret = app_control_get_extra_data(app_control, "msg_id", &msgId);
	if (ret != APP_CONTROL_ERROR_NONE || msgId == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	int msg_err = MSG_SUCCESS;
	msg_message_id_t msg_id = atoi(msgId);
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	msg_list_handle_t addr_list = NULL;
	MSG_MGR_MESSAGE_INFO_S msg_info = {0,};
	msg_info.msgId = msg_id;

	msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
	opt = msg_create_struct(MSG_STRUCT_SENDOPT);
	msg_err = msg_get_message(msg_handle, msg_id, msg, opt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_get_message() failed [%d]", msg_err);
		return;
	}

	msg_get_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, &msg_info.sim_idx);

	if (noti_type == MSG_MGR_NOTI_TYPE_CLASS0) {
		msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&addr_list);

		msg_struct_t addr_data = msg_list_nth_data(addr_list, 0);
		msg_get_str_value(addr_data, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, msg_info.addressVal, MAX_ADDRESS_VAL_LEN);
		msg_get_str_value(addr_data, MSG_ADDRESS_INFO_DISPLAYNAME_STR, msg_info.displayName, MAX_DISPLAY_NAME_LEN);

		int msg_size = 0;
		msg_get_int_value(msg, MSG_MESSAGE_DATA_SIZE_INT, &msg_size);
		if (msg_size > 0)
			msg_get_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, msg_info.msgText, MAX_MSG_TEXT_LEN);
	}

	msg_release_struct(&msg);
	msg_release_struct(&opt);

	g_free(msgId);

	MsgMgrInsertOnlyActiveNotification(noti_type, &msg_info);
}

void _insert_ticker_func(app_control_h app_control)
{
	char *ticker_msg = NULL;
	char *locale_ticker_msg = NULL;
	char *feedback_str = NULL;
	bool feedback = false;
	char *msg_id_str = NULL;
	int msg_id = 0;

	int ret = app_control_get_extra_data(app_control, "ticker_msg", &ticker_msg);
	if (ret != APP_CONTROL_ERROR_NONE || ticker_msg == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed [%d]", ret);
		goto _END_OF_INSERT_TICKER;
	}

	ret = app_control_get_extra_data(app_control, "locale_ticker_msg", &locale_ticker_msg);
	if (ret != APP_CONTROL_ERROR_NONE || locale_ticker_msg == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed [%d]", ret);
		goto _END_OF_INSERT_TICKER;
	}

	ret = app_control_get_extra_data(app_control, "feedback", &feedback_str);
	if (ret != APP_CONTROL_ERROR_NONE || feedback_str == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed [%d]", ret);
		goto _END_OF_INSERT_TICKER;
	} else {
		if (g_strcmp0(feedback_str, "true") == 0)
			feedback = true;
		else
			feedback = false;
	}

	ret = app_control_get_extra_data(app_control, "msg_id", &msg_id_str);
	if (ret != APP_CONTROL_ERROR_NONE || msg_id_str == NULL) {
		MSG_MGR_ERR("app_control_get_extra_data failed [%d]", ret);
		goto _END_OF_INSERT_TICKER;
	} else {
		msg_id = atoi(msg_id_str);
	}

	ret = MsgMgrInsertTicker(ticker_msg, locale_ticker_msg, feedback, msg_id);
	if (ret != 0)
		MSG_MGR_ERR("MsgMgrInsertTicker failed [%d]", ret);

_END_OF_INSERT_TICKER:
	if (ticker_msg)
		g_free(ticker_msg);

	if (locale_ticker_msg)
		g_free(locale_ticker_msg);

	if (feedback_str)
		g_free(feedback_str);

	if (msg_id_str)
		g_free(msg_id_str);
}

void _sound_play_start_func(app_control_h app_control)
{
	char *type;
	MSG_MGR_SOUND_TYPE_T sound_type = MSG_MGR_SOUND_PLAY_DEFAULT;

	int ret = app_control_get_extra_data(app_control, "type", &type);
	if (ret == APP_CONTROL_ERROR_NONE && type) {
		if (g_strcmp0(type, "default") == 0) {
			sound_type = MSG_MGR_SOUND_PLAY_DEFAULT;
		} else if (g_strcmp0(type, "user") == 0) {
			sound_type = MSG_MGR_SOUND_PLAY_USER;
		} else if (g_strcmp0(type, "emergency") == 0) {
			sound_type = MSG_MGR_SOUND_PLAY_EMERGENCY;
		} else if (g_strcmp0(type, "voicemail") == 0) {
			sound_type = MSG_MGR_SOUND_PLAY_VOICEMAIL;
		}

		g_free(type);
	} else {
		MSG_MGR_ERR("app_control_get_extra_data failed");
		return;
	}

	char *addr = NULL;
	ret = app_control_get_extra_data(app_control, "address", &addr);

	if (addr) {
		MSG_MGR_ADDRESS_INFO_S addr_info = {0,};
		snprintf(addr_info.addressVal, MAX_ADDRESS_VAL_LEN, "%s", addr);

		MsgMgrSoundPlayStart(&addr_info, sound_type);

		g_free(addr);
	} else {
		MsgMgrSoundPlayStart(NULL, sound_type);
	}
}

void _change_pm_state_func(app_control_h app_control)
{
	MsgMgrChangePmState();
}

void service_app_control(app_control_h app_control, void *data)
{
	MSG_MGR_INFO("service_app_control called");

	pthread_mutex_lock(&mx);
	job_cnt++;
	pthread_mutex_unlock(&mx);

	int ret = 0;
	char *operation = NULL;
	char *cmd = NULL;

	ret = app_control_get_operation(app_control, &operation);
	if (ret == APP_CONTROL_ERROR_NONE && operation) {
		if (g_strcmp0(operation, APP_CONTROL_OPERATION_DEFAULT) == 0) {
			ret = app_control_get_extra_data(app_control, "cmd", &cmd);
			if (ret == APP_CONTROL_ERROR_NONE && cmd) {
				MSG_MGR_DEBUG("cmd [%s]", cmd);

				if (g_strcmp0(cmd, "incoming_msg") == 0) {
					_incoming_msg_func(app_control);
				} else if (g_strcmp0(cmd, "outgoing_msg") == 0) {
					_outgoing_msg_func(app_control);
				} else if (g_strcmp0(cmd, "refresh_noti") == 0) {
					_refresh_noti_func(app_control);
				} else if (g_strcmp0(cmd, "add_noti") == 0) {
					_add_noti_func(app_control);
				} else if (g_strcmp0(cmd, "del_noti") == 0) {
					_del_noti_func(app_control);
				} else if (g_strcmp0(cmd, "add_report_noti") == 0) {
					_add_report_noti_func(app_control);
				} else if (g_strcmp0(cmd, "del_report_noti") == 0) {
					_del_report_noti_func(app_control);
				} else if (g_strcmp0(cmd, "insert_only_active_noti") == 0) {
					_insert_only_active_noti_func(app_control);
				} else if (g_strcmp0(cmd, "insert_ticker") == 0) {
					_insert_ticker_func(app_control);
				} else if (g_strcmp0(cmd, "sound_play_start") == 0) {
					_sound_play_start_func(app_control);
				} else if (g_strcmp0(cmd, "change_pm_state") == 0) {
					_change_pm_state_func(app_control);
				}

				g_free(cmd);
			}
		}
		g_free(operation);
	}

	pthread_mutex_lock(&mx);
	if (!terminated)
		g_timeout_add_seconds(60, _check_app_terminate, NULL);
	pthread_mutex_unlock(&mx);

	return;
}

int main(int argc, char* argv[])
{
	service_app_lifecycle_callback_s event_callback = {0,};

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	return service_app_main(argc, argv, &event_callback, NULL);
}
