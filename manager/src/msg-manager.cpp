#include <stdlib.h>
#include <glib.h>

#include <msg.h>
#include <msg_storage.h>
#include <tizen.h>
#include <service_app.h>
#include <app_event.h>
#include <bundle.h>
#include <notification_list.h>
#include <notification_text_domain.h>
#include <notification_internal.h>

#include "msg-manager.h"
#include "msg-manager-contact.h"
#include "msg-manager-debug.h"

/* below defines will be removed */
#define EVENT_KEY_OUT_MSG_TYPE "msg_type"
#define EVENT_KEY_OUT_MSG_ID "msg_id"


bool service_app_create(void *data)
{
	MSG_MGR_INFO("app_create");

	return true;
}

void service_app_terminate(void *data)
{
	MSG_MGR_INFO("app_terminate");

	MsgMgrCloseContactSvc();

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
	msg_handle_t msg_handle = NULL;
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	contactInfo contact_info = {0,};
	contact_info.msgId = msg_id;

	msg_err = msg_open_msg_handle(&msg_handle);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_open_msg_handle() failed [%d]", msg_err);
		return;
	}

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

	msg_close_msg_handle(&msg_handle);

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
	msg_handle_t msg_handle = NULL;
	msg_struct_t msg = NULL;
	msg_struct_t opt = NULL;
	contactInfo contact_info = {0,};
	contact_info.msgId = msg_id;

	msg_err = msg_open_msg_handle(&msg_handle);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_open_msg_handle() failed [%d]", msg_err);
		return;
	}

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

	msg_close_msg_handle(&msg_handle);

	g_free(sent_msg_id);
	g_free(sent_msg_type);

	MSG_MGR_END();
}

void _refresh_noti_func(app_control_h app_control)
{

}

void service_app_control(app_control_h app_control, void *data)
{
	MSG_MGR_INFO("service_app_control called");

	int ret = 0;
	char *operation = NULL;
	char *cmd = NULL;

	ret = app_control_get_operation(app_control, &operation);
	if (ret == APP_CONTROL_ERROR_NONE && operation) {
		MSG_MGR_DEBUG("operation [%s]", operation);

		if (g_strcmp0(operation, APP_CONTROL_OPERATION_DEFAULT) == 0) {
			ret = app_control_get_extra_data(app_control, "cmd", &cmd);
			if (ret == APP_CONTROL_ERROR_NONE && cmd) {
				MSG_MGR_DEBUG("cmd [%s]", cmd);

				if (g_strcmp0(cmd, "incoming_msg") == 0) {
					_incoming_msg_func(app_control);
				} else if (g_strcmp0(cmd, "outgoing_msg") == 0) {
					_outgoing_msg_func(app_control);
				} else if (g_strcmp0(cmd, "refresh") == 0) {
					_refresh_noti_func(app_control);
				}

				g_free(cmd);
			}
		}
		g_free(operation);
	}

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
