/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include <libintl.h>
#include <locale.h>
#include <stdarg.h>

#include <app_control.h>
#include <badge_internal.h>
#include <notification_list.h>
#include <notification_text_domain.h>
#include <notification_internal.h>
#include <notification_status.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <package_manager.h>
#include <vconf.h>

#include <msg.h>
#include <msg_storage.h>

#include <msg-manager-util.h>
#include <msg-manager-contact.h>
#include <msg-manager-debug.h>
#include <msg-manager-notification.h>
#include <msg-manager-sound.h>


#define EMAIL_AT '@'

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

static GList *msg_report_notification_list = NULL;
static bool is_init = false;
extern msg_handle_t msg_handle;
int g_alarmId = 0;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
typedef struct _report_notification_s
{
	int priv_id;
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
} report_notification_s;


typedef struct _msg_mgr_noti_info_s
{
	msg_mgr_notification_type_t		type;
	int			id;
	int			layout;
	int			count;
	int			senderCount;
	time_t		time;
	char		sender[MSG_NOTI_TEXT_LEN_S+1];
	char		text[MSG_NOTI_TEXT_LEN+1];
	char		number[MSG_NOTI_TEXT_LEN_S+1];
	char		imagePath[MAX_IMAGE_PATH_LEN+1];		/**< Indicates the image path of contact. */
	int			applist;
	app_control_h		svc_h;
	app_control_h		active_noti_svc_h[MSG_ACTIVE_NOTI_BUTTON_NUM];
	msg_message_id_t		msg_id;
	unsigned char		extra_data;
	int		sim_idx;
	int			active_noti_button_num;
	int		active_media_cnt;
	int		active_media_size;
	unsigned char	active_subtype;		/**< to distinguish cb, push message */
	char		active_sender[MSG_NOTI_TEXT_LEN_S+1];
	char		active_subject[MSG_NOTI_TEXT_LEN_S+1];
	char		active_text[MSG_NOTI_TEXT_LEN+1];
} MSG_MGR_NOTI_INFO_S;


typedef struct _del_noti_info_s
{
	msg_mgr_notification_type_t			type;
	int		sim_idx;
} DEL_NOTI_INFO_S;


/*==================================================================================================
										FUNCTION DEFINE
===================================================================================================*/

void MsgMgrInitReportNotiList();
void MsgRefreshNotiCb(void *data);
void MsgMgrDeleteNotiCb(void *data);

void MsgMgrDeleteNotification(msg_mgr_notification_type_t noti_type, int simIndex);

notification_h getHandle(int *noti_id);

int getPrivId(msg_mgr_notification_type_t noti_type, int sim_idx);
void updatePrivId(msg_mgr_notification_type_t noti_type, int noti_id, int sim_idx);

void createInfoData(MSG_MGR_NOTI_INFO_S *noti_info, MSG_MGR_MESSAGE_INFO_S *msg_info); /* For addNoti() */
void createInfoData(MSG_MGR_NOTI_INFO_S *noti_info, msg_mgr_active_notification_type_t active_noti);
void createActiveInfoData(MSG_MGR_NOTI_INFO_S *noti_info, MSG_MGR_MESSAGE_INFO_S *msg_info);
void clearInfoData(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);

int getAppIcon(const char *app_id, char **icon_path);
int getLatestMsgInfo(MSG_MGR_NOTI_INFO_S *noti_info, bool isForInstantMessage);

void setProperty(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setTextDomain(notification_h noti_h);
void setText(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setIcon(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setSoundAndVibration(notification_h noti_h, char *addressVal, bool bVoiceMail);
void setActiveNotification(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setActiveProperty(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setActiveText(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);
void setActiveIcon(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info);

void setNotification(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info, bool bFeedback);

bool isExistAddressInReportTable(const char *addr);

/* Wrapper */
void createServiceHandle(app_control_h *svc_h);
void setServiceAppId(app_control_h svc_h, const char* app_id);
void setServiceUri(app_control_h svc_h, const char* uri);
void setServiceOperation(app_control_h svc_h, const char* operation);
void addServiceExtraData(app_control_h svc_h, const char* bundle_key, const char* bundle_val);
void addServiceExtraData(app_control_h svc_h, const char* bundle_key, int bundle_val);
void setServicePackageName(app_control_h svc_h, const char* pkg_name);
void sendServicelaunchRequest(app_control_h svc_h, app_control_reply_cb callback, void *user_data);

void setNotiTextDomain(notification_h noti_h, const char *pkg_name, const char *loc_dir);
void setNotiText(notification_h noti_h, notification_text_type_e type, const char *text, const char *key);
void setNotiTimeToText(notification_h noti_h, notification_text_type_e type, time_t time);
void setNotiTime(notification_h noti_h, time_t time);
void setNotiImage(notification_h noti_h, notification_image_type_e type, const char *image_path);
void setNotiSound(notification_h noti_h, notification_sound_type_e type, const char *path);
void setNotiVibration(notification_h noti_h, notification_vibration_type_e type, const char *path);
void setNotiEventHandler(notification_h noti_h, notification_event_type_e type, app_control_h event_handler);

/* Alarm */
void MsgMgrNotiSoundRepeatAlarmCB(int alarmId);
void MsgMgrSoundCreateRepeatAlarm(int RepeatTime);
void MsgMgrSoundSetRepeatAlarm();

char *get_translate_text(const char *pkg_name, const char *locale_dir, const char *text);

/*==================================================================================================
									FUNCTION IMPLEMENTATION
==================================================================================================*/
bool _is_valid_email(char *pAddress)
{
	if (!pAddress || pAddress[0] == 0)
		return false;
	if (!strchr (pAddress, EMAIL_AT))
		return false;
	return true;
}


bool isExistAddressInReportTable(const char *addr)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0, colCnt = 0;
	char **db_res = NULL;
	int msg_err = 0;

	char *normal_addr = msg_mgr_normalize_number((char *)addr);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "* FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s'", MSGFW_SMS_REPORT_TABLE_NAME, normal_addr);

	msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &rowCnt, &colCnt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
		return false;
	}

	msg_err = msg_db_free(msg_handle, db_res);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_db_free() failed [%d]", msg_err);
		return false;
	}

	if (rowCnt > 0)
		return true;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "* FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s'", MSGFW_REPORT_TABLE_NAME, normal_addr);

	msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &rowCnt, &colCnt);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
		return false;
	}

	msg_err = msg_db_free(msg_handle, db_res);
	if (msg_err != MSG_SUCCESS) {
		MSG_MGR_ERR("msg_db_free() failed [%d]", msg_err);
		return false;
	}

	if (rowCnt > 0)
		return true;

	return false;
}


void MsgMgrInitReportNotiList()
{
	MSG_MGR_BEGIN();

	if (msg_report_notification_list) {
		MSG_MGR_DEBUG("Report Noti List is already inited");
		return;
	}

	msg_report_notification_list = NULL;

	notification_h noti = NULL;
	notification_list_h noti_list = NULL;
	notification_list_h head_noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	app_control_h app_control = NULL;

	noti_err = notification_get_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("notification_get_list() is failed!!");
		return;
	}

	head_noti_list = noti_list;

	while (noti_list != NULL) {
		noti = notification_list_get_data(noti_list);

		char tempAddr[MAX_ADDRESS_VAL_LEN+1];
		memset(tempAddr, 0x00, sizeof(tempAddr));

		noti_err = notification_get_launch_option(noti, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, &app_control);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_MGR_DEBUG("notification_get_excute_option() failed!!");
			break;
		}

		char *addr = NULL;

		int ret = app_control_get_extra_data(app_control, "address", &addr);
		if (ret == APP_CONTROL_ERROR_NONE && addr != NULL) {
			if (isExistAddressInReportTable(addr)) {
				report_notification_s *info = new report_notification_s;
				memset(info, 0x00, sizeof(report_notification_s));

				notification_get_id(noti, NULL, &(info->priv_id));
				snprintf(info->addressVal, sizeof(info->addressVal), "%s", addr);

				msg_report_notification_list = g_list_append(msg_report_notification_list, (void *)info);
				MSG_MGR_SEC_DEBUG("appended list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);
			}

			g_free(addr);
			addr = NULL;
		}

		noti_list = notification_list_get_next(noti_list);
	}

	if (head_noti_list)
		notification_free_list(head_noti_list);

	MSG_MGR_END();
}


void MsgMgrInitNoti()
{
	if (is_init)
		return;

	bool bNotiSvcReady = false;

	bNotiSvcReady = notification_is_service_ready();

	if (bNotiSvcReady == true) {
		MSG_MGR_DEBUG("Notification server is available");
#ifndef MSG_NOTI_INTEGRATION
		MsgDeleteNotification(MSG_MGR_NOTI_TYPE_SIM, -1);
#endif
		MsgMgrRefreshAllNotification(false, true, MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT);		/* On Booting */
		MsgMgrInitReportNotiList();
	} else {
		MSG_MGR_DEBUG("Notification server is not available. Init is defered");
#ifndef MSG_NOTI_INTEGRATION
		MSG_MGR_NOTI_INFO_S *delNotiInfo = (MSG_MGR_NOTI_INFO_S *)calloc(1, sizeof(MSG_MGR_NOTI_INFO_S));
		if (delNotiInfo) {
			delNotiInfo->type = MSG_MGR_NOTI_TYPE_SIM;
			delNotiInfo->sim_idx = -1;
		}
		notification_add_deferred_task(MsgDeleteNotiCb, (void *)delNotiInfo);
#endif
		notification_add_deferred_task(MsgRefreshNotiCb, (void *)NULL);
	}

	is_init = true;
}


void MsgRefreshNotiCb(void *data)
{
	MsgMgrRefreshAllNotification(false, true, MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT);
	MsgMgrInitReportNotiList();

	if (data) {
		free(data);
		data = NULL;
	}

	return;
}


void MsgMgrDeleteNotiCb(void *data)
{
	if (data) {
		DEL_NOTI_INFO_S *delNotiInfo = (DEL_NOTI_INFO_S *)data;

		MsgMgrDeleteNotification(delNotiInfo->type, delNotiInfo->sim_idx);

		free(data);
		data = NULL;
	}

	return;
}


int MsgMgrInsertOnlyActiveNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info)
{
	MSG_MGR_BEGIN();

	notification_h noti_h = NULL;

	MSG_MGR_NOTI_INFO_S noti_info = {0, };

	noti_info.type = noti_type;
	noti_info.active_noti_button_num = 1;

	createActiveInfoData(&noti_info, msg_info);

	noti_h = notification_create(NOTIFICATION_TYPE_NOTI);

	setActiveNotification(noti_h, &noti_info);

	clearInfoData(noti_h, &noti_info);

	MSG_MGR_END();
	return 0;
}


int MsgMgrRefreshNotification(msg_mgr_notification_type_t noti_type, bool bFeedback, msg_mgr_active_notification_type_t active_type)
{
	int err = 0;
	notification_h noti_h = NULL;
	int bNotification = 1;
/*	bool bReplyPopup = false; */

	MSG_MGR_NOTI_INFO_S noti_info = {0, };
	noti_info.type = noti_type;
	noti_info.id = getPrivId(noti_info.type, -1);

	err = getLatestMsgInfo(&noti_info, false);

	if (err != 0) {
		MSG_MGR_DEBUG("getLatestMsgInfo() err = [%d]", err);
		goto __END_OF_REFRESH_NOTI;
	}

	if (active_type == MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT) {
		err = MsgMgrInsertInstantMessage(noti_type);

		if (err != 0) {
			MSG_MGR_DEBUG(" MsgMgrInsertInstantMessage() err = [%d]", err);
			goto __END_OF_REFRESH_NOTI;
		}
	}

	if (vconf_get_bool(MSG_SETTING_NOTIFICATION, &bNotification) != 0) {
		MSG_MGR_DEBUG("vconf_get_bool is failed.");
	}

	if (bNotification == 0) {
		MSG_MGR_DEBUG("Msg Alert notification is off.");
		goto __END_OF_REFRESH_NOTI;
	}

	createInfoData(&noti_info, active_type);

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_MGR_DEBUG("Notification handle is NULL");
		err = MSG_ERR_NULL_POINTER;
		goto __END_OF_REFRESH_NOTI;
	}

	setNotification(noti_h, &noti_info, bFeedback);

__END_OF_REFRESH_NOTI :
	clearInfoData(noti_h, &noti_info);

	return err;
}


int MsgMgrAddReportNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info)
{
	int ret = 0;

	notification_h noti_h = NULL;

	report_notification_s *info = new report_notification_s;
	memset(info, 0x00, sizeof(report_notification_s));

	MSG_MGR_NOTI_INFO_S noti_info = {0, };
	noti_info.type = noti_type;

	createInfoData(&noti_info, msg_info);

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_MGR_DEBUG("Notification handle is NULL");
		ret = -1;
		goto __END_OF_ADD_REPORT_NOTI;
	}

	setNotification(noti_h, &noti_info, true);

	info->priv_id = noti_info.id;
	snprintf(info->addressVal, sizeof(info->addressVal), "%s", msg_info->addressVal);
	msg_report_notification_list = g_list_append(msg_report_notification_list, (void *)info);
	MSG_MGR_SEC_DEBUG("appended list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);

__END_OF_ADD_REPORT_NOTI :
	clearInfoData(noti_h, &noti_info);

	return ret;
}


int MsgMgrDeleteReportNotification(const char *addr)
{
	MSG_MGR_BEGIN();

	notification_h noti_h = NULL;
	int bNotification = 1;

	if (vconf_get_bool(MSG_SETTING_NOTIFICATION, &bNotification) != 0) {
		MSG_MGR_DEBUG("vconf_get_bool is failed.");
	}

	if (bNotification == 0) {
		MSG_MGR_DEBUG("Msg Alert notification is off.");
		return 0;
	}

	char* normalAddr = NULL;
	unsigned int list_length = g_list_length(msg_report_notification_list);
	bool isDelete = false;

	MSG_MGR_DEBUG("list length [%d]", list_length);

	if (list_length > 0) {
		GList *iter = g_list_first(msg_report_notification_list);

		while (iter != NULL) {
			isDelete = false;
			report_notification_s *info = (report_notification_s*)(iter->data);
			if (info == NULL) {
				MSG_MGR_DEBUG("info is NULL!");
				return -1;
			}

			MSG_MGR_SEC_DEBUG("list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);

			noti_h = notification_load(NULL, info->priv_id);
			if (noti_h == NULL) {
				MSG_MGR_DEBUG("notification with priv_id [%d] is NULL", info->priv_id);
				isDelete = true;
			} else {
				normalAddr = msg_mgr_normalize_number(info->addressVal);

				if (normalAddr) {
					MSG_MGR_SEC_DEBUG("normalized number = %s", normalAddr);

					if (g_str_has_suffix(addr, normalAddr)) {
						if (notification_delete(noti_h) == NOTIFICATION_ERROR_NONE) {
							MSG_MGR_SEC_DEBUG("delete report notification address [%s]", info->addressVal);
							isDelete = true;
						} else {
							MSG_MGR_DEBUG("delete notification failed");
						}
					}
				}

				notification_free(noti_h);
				noti_h = NULL;
			}

			iter = g_list_next(iter);

			if (isDelete) {
				msg_report_notification_list = g_list_remove(msg_report_notification_list, (void *)info);
				if (info) {
					delete info;
					info = NULL;
				}
			}
		}
	}

	MSG_MGR_END();

	return 0;
}

int MsgMgrAddNotification(msg_mgr_notification_type_t noti_type, MSG_MGR_MESSAGE_INFO_S *msg_info)
{
	int ret = 0;

	notification_h noti_h = NULL;

	MSG_MGR_NOTI_INFO_S noti_info = {0, };

	noti_info.type = noti_type;

	createInfoData(&noti_info, msg_info);

	/* check mwi or voicemail count is 0 then skip add notification */
	if (noti_info.count == 0) {
		MSG_MGR_DEBUG("Notification count is 0");
		ret = -1;
		goto __END_OF_ADD_NOTI;
	}

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_MGR_DEBUG("Notification handle is NULL");
		ret = -1;
		goto __END_OF_ADD_NOTI;
	}

	setNotification(noti_h, &noti_info, true);

__END_OF_ADD_NOTI :
	clearInfoData(noti_h, &noti_info);

	return ret;
}


void MsgMgrDeleteNotification(msg_mgr_notification_type_t noti_type, int simIndex)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	if (noti_type == MSG_MGR_NOTI_TYPE_ALL) {
		noti_err = notification_delete_all(NOTIFICATION_TYPE_NOTI);
	} else if (noti_type == MSG_MGR_NOTI_TYPE_VOICE_1 || noti_type == MSG_MGR_NOTI_TYPE_VOICE_2 || noti_type == MSG_MGR_NOTI_TYPE_SIM) {
		int notiId = 0;

		notiId = getPrivId(noti_type, simIndex);
		MSG_MGR_DEBUG("deleted notification ID = [%d] Type = [%d]", notiId, noti_type);

		if (notiId > 0)
			noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, notiId);

	} else {
		MSG_MGR_DEBUG("No matching type [%d]", noti_type);
	}

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_delete_all noti_err [%d]", noti_err);
	}

	updatePrivId(noti_type, 0, simIndex);
}


int MsgMgrDeleteNoti(msg_mgr_notification_type_t noti_type, int simIndex)
{
	bool bNotiSvcReady = false;

	DEL_NOTI_INFO_S *delNotiInfo = (DEL_NOTI_INFO_S *)calloc(1, sizeof(DEL_NOTI_INFO_S));

	if (delNotiInfo) {
		delNotiInfo->type = noti_type;
		delNotiInfo->sim_idx = simIndex;
	}

	bNotiSvcReady = notification_is_service_ready();

	if (bNotiSvcReady == true) {
		MSG_MGR_DEBUG("Notification server is available");
		MsgMgrDeleteNotiCb((void *)delNotiInfo);
	} else {
		MSG_MGR_DEBUG("Notification server is not available. Delete is defered");
		notification_add_deferred_task(MsgMgrDeleteNotiCb, (void *)delNotiInfo);
	}
	return 0;
}


void MsgMgrRefreshAllNotification(bool bWithSimNoti, bool bFeedback, msg_mgr_active_notification_type_t active_type)
{
	MSG_MGR_BEGIN();

	int err = 0;

#ifndef MSG_NOTI_INTEGRATION
	MsgDeleteNotification(MSG_MGR_NOTI_TYPE_SIM);
#endif

#ifdef MSG_NOTI_INTEGRATION
	err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_NORMAL, bFeedback, active_type);
	if (err != 0)
		MSG_MGR_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_MGR_NOTI_TYPE_NORMAL, err);
#else
	err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_NORMAL, bFeedback, active_type);
	if (err != 0)
		MSG_MGR_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_MGR_NOTI_TYPE_NORMAL, err);

	err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_CB, bFeedback, active_type);
	if (err != 0)
		MSG_MGR_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_MGR_NOTI_TYPE_CB, err);

	if (bWithSimNoti) {
		err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_SIM, bFeedback, active_type);
		if (err != 0)
			MSG_MGR_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_MGR_NOTI_TYPE_SIM, err);
	}
#endif

	err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_FAILED, bFeedback, active_type);
	if (err != 0)
		MSG_MGR_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_MGR_NOTI_TYPE_FAILED, err);

	MSG_MGR_END();
}


void setProperty(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	/* set layout */
	noti_err = notification_set_layout(noti_h, (notification_ly_type_e)noti_info->layout);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	/* set led */
	noti_err = notification_set_led(noti_h, NOTIFICATION_LED_OP_ON, 0x00);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_led.");
	}

	/* set execute option and property */
	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL: {
		if (noti_info->count > 1) {
			notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		} else {
			if (noti_info->svc_h) { /* overwrite bundle key "type" */
				/* addServiceExtraData(noti_info->svc_h, "type", "reply"); */
				addServiceExtraData(noti_info->svc_h, "show_list", "list_show");
			}
			notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		}

		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
		break;
	}
	case MSG_MGR_NOTI_TYPE_CB:
	case MSG_MGR_NOTI_TYPE_SIM: {
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE|NOTIFICATION_PROP_VOLATILE_DISPLAY);
		break;
	}
	case MSG_MGR_NOTI_TYPE_FAILED: {
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL: {
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		break;
	}
	case MSG_MGR_NOTI_TYPE_VOICE_1:
	case MSG_MGR_NOTI_TYPE_VOICE_2:
	case MSG_MGR_NOTI_TYPE_MWI:
	case MSG_MGR_NOTI_TYPE_CLASS0:
	case MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT:
	case MSG_MGR_NOTI_TYPE_MMS_READ_REPORT:
	case MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT: {
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->svc_h);
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type for notification_set_launch_option() [%d]", noti_info->type);
		break;
	}

	/* set applist */
	noti_err = notification_set_display_applist(noti_h, noti_info->applist);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_display_applist");
	}


	MSG_MGR_END();
}


void setTextDomain(notification_h noti_h)
{
	MSG_MGR_BEGIN();

	setNotiTextDomain(noti_h, MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR);
	MSG_MGR_END();
}


void setText(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	char unreadMsgCntStr[10] = {0};
	int bPreview = 1;

	if (vconf_get_bool(MSG_SETTING_PREVIEW, &bPreview) != 0) {
		MSG_MGR_DEBUG("vconf_get_bool is failed.");
	}

	/* set title and content */
	switch (noti_info->type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_CB:
	case MSG_MGR_NOTI_TYPE_SIM: {
		if (noti_info->count > 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "New Messages", NEW_MESSAGES);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			setNotiTime(noti_h, noti_info->time);

			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);

		} else {
			if (bPreview) {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->sender, NULL);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->text, NULL);
			} else {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "New Message", NEW_MESSAGE);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			}
			setNotiTime(noti_h, noti_info->time);
		}
		break;
	}
#else
	case MSG_MGR_NOTI_TYPE_NORMAL: {
		if (noti_info->count > 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "New Messages", NEW_MESSAGES);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			setNotiTime(noti_h, noti_info->time);

			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
		} else {
			if (bPreview) {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->sender, NULL);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->text, NULL);
			} else {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "New Message", NEW_MESSAGE);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			}
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1", NULL);
			setNotiTime(noti_h, noti_info->time);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_CB: {
		if (noti_info->count > 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Broadcast message", CB_MESSAGE);
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			setNotiTime(noti_h, noti_info->time);

		} else {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1", NULL);
			setNotiTime(noti_h, noti_info->time);

			if (bPreview) {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->sender, NULL);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->text, NULL);
			} else {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Broadcast message", CB_MESSAGE);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			}
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM: {
		if (noti_info->count > 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "SIM card Message", SIM_CARD_MESSAGE);
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);

			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			setNotiTime(noti_h, noti_info->time);
		} else {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1", NULL);
			setNotiTime(noti_h, noti_info->time);

			if (bPreview) {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->sender, NULL);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->text, NULL);
			} else {
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "SIM card Message", SIM_CARD_MESSAGE);
				setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
			}
		}
		break;
	}
#endif
	case MSG_MGR_NOTI_TYPE_FAILED: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Message", MSG_MESSAGE);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, "Failed to send message.", FAILED_TO_SEND_MESSAGE);
		if (noti_info->count > 1) {
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
		}
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_VOICE_1:
	case MSG_MGR_NOTI_TYPE_VOICE_2: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Voicemail", VOICE_MAIL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);

		if (noti_info->count == 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1", NULL);
		} else if (noti_info->count > 1) {
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
		} else {
			MSG_MGR_DEBUG("Invalid notification count, [cnt = %d]", noti_info->count);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_MWI: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "MWI Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_CLASS0: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "CLASS 0 Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Delivery report", DELIVERY_MESSAGE);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);

		if (noti_info->extra_data == MSG_NETWORK_DELIVER_SUCCESS) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message delivered", DELIVERED_MESSAGE);
		} else if (noti_info->extra_data == MSG_NETWORK_DELIVER_EXPIRED) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message expired", EXPIRED_MESSAGE);
		} else {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message deferred", DEFERRED_MESSAGE);
		}

		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_MMS_READ_REPORT: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Read Report", READ_REPORT_MESSAGE);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);

		if (noti_info->extra_data == MSG_READ_REPORT_IS_DELETED) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message deleted", READ_REPORT_DELETE);
		/* CID 45672: noti_info->extra_data in unsigned char but MSG_READ_REPORT_NONE is -1. So the expression is always false */
#if 0
		} else if (noti_info->extra_data == MSG_READ_REPORT_NONE) {
			/* notification free */
#endif
		} else {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message read", READ_REPORT_READ);
		}

		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Delivery Report", DELIVERY_MESSAGE);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);

		if (noti_info->extra_data == MSG_DELIVERY_REPORT_EXPIRED)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message expired", EXPIRED_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_REJECTED)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message rejected", REJECTED_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_DEFERRED)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message deferred", DEFERRED_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_UNRECOGNISED)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message unrecognised", UNRECOGNISED_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_INDETERMINATE)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message indeterminate", INDETEMINATE_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_FORWARDED)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message forwarded", NULL);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_UNREACHABLE)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message unreachable", UNREACHABLE_MESSAGE);
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_ERROR)
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message error", NULL);
		else
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message delivered", DELIVERED_MESSAGE);

		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "SIM card full", SMS_SIM_CARD_FULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, "Not enough memory. Delete some items.", SMS_MESSAGE_MEMORY_FULL);
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_MGR_END();
}

void setIcon(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	switch (noti_info->type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_CB:
	case MSG_MGR_NOTI_TYPE_SIM: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		if (noti_info->count > 1 && noti_info->senderCount > 1) {
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_NORMAL_ICON_PATH);
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_SUB, "");
		} else {
			if (noti_info->active_subtype == MSG_CB_SMS) {
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_CB_ICON_PATH);
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
			} else if (noti_info->active_subtype == MSG_WAP_SI_SMS || noti_info->active_subtype == MSG_WAP_SL_SMS) {
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_ACTIVE_PUSH_ICON_PATH);
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_ACTIVE_PUSH_ICON_PATH);
			} else if (noti_info->active_subtype == MSG_SYNCML_CP) {
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_OTA_ICON_PATH);
			} else {
				if (noti_info->imagePath[0] != '\0') {
					setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, noti_info->imagePath);
					setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, noti_info->imagePath);
				} else {
					setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_NO_CONTACT_PROFILE_ICON_PATH);
					setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NO_CONTACT_PROFILE_ICON_PATH);
				}

				char *msg_icon_path = NULL;
				if (getAppIcon(MSG_DEFAULT_APP_ID, &msg_icon_path) == 0) {
					setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_SUB, msg_icon_path);
					g_free(msg_icon_path);
				} else {
					MSG_MGR_ERR("fail to get message-app icon");
				}
			}
		}
		break;
	}
#else
	case MSG_MGR_NOTI_TYPE_NORMAL: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_NORMAL_ICON_PATH);

		if (noti_info->count > 1) {
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		} else {
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_REPLY_ICON_PATH);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_CB: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_CB_ICON_PATH);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SIM_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_SIM_ICON_PATH);
		break;
	}
#endif
	case MSG_MGR_NOTI_TYPE_FAILED: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SMS_SENDING_FAILED_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_FAILED_STATUS_ICON);
		break;
	}
	case MSG_MGR_NOTI_TYPE_VOICE_1:
	case MSG_MGR_NOTI_TYPE_VOICE_2:
	case MSG_MGR_NOTI_TYPE_MWI: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_VOICE_MSG_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_VOICE_ICON_PATH);
		break;
	}
	case MSG_MGR_NOTI_TYPE_CLASS0:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_MGR_NOTI_TYPE_MMS_READ_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_MGR_NOTI_TYPE_SIM_FULL:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		break;
	default:
		MSG_MGR_DEBUG("No matching type for MsgNotiSetImage [%d]", noti_info->type);
		break;
	}

	MSG_MGR_END();
}

void setActiveProperty(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	/* set layout */
	noti_err = notification_set_layout(noti_h, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	/* set led */
	noti_err = notification_set_led(noti_h, NOTIFICATION_LED_OP_ON, 0x00);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_led.");
	}

	/* set execute option and property */
	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL:
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->active_noti_svc_h[2]);
		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
		break;
	case MSG_MGR_NOTI_TYPE_CLASS0:
		notification_set_launch_option(noti_h, NOTIFICATION_LAUNCH_OPTION_APP_CONTROL, noti_info->active_noti_svc_h[2]);
		break;
	default:
		MSG_MGR_DEBUG("No matching type for notification_set_launch_option() [%d]", noti_info->type);
		break;
	}

	/* set applist */
	noti_err = notification_set_display_applist(noti_h, NOTIFICATION_DISPLAY_APP_ACTIVE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_set_display_applist");
	}

	MSG_MGR_END();
}


void setActiveText(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_SIM:
	case MSG_MGR_NOTI_TYPE_CB: {
		if (noti_info->active_subject[0] == '\0') {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->active_sender, NULL);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->active_text, NULL);
		} else {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, noti_info->active_sender, NULL);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, noti_info->active_subject, NULL);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->active_text, NULL);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_CLASS0: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "CLASS 0 Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->active_sender, NULL);
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_MGR_END();
}


void setActiveIcon(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_SIM: {
		switch (noti_info->active_subtype) {
		case MSG_CB_SMS:
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
			break;
		case MSG_WAP_SI_SMS:
		case MSG_WAP_SL_SMS:
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_ACTIVE_PUSH_ICON_PATH);
			break;
		case MSG_SYNCML_CP:
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_OTA_ICON_PATH);
			break;
		default:
			if (noti_info->imagePath[0] != '\0')
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, noti_info->imagePath);
			else
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NO_CONTACT_PROFILE_ICON_PATH);

			char *msg_icon_path = NULL;
			if (getAppIcon(MSG_DEFAULT_APP_ID, &msg_icon_path) == 0) {
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_SUB, msg_icon_path);
				g_free(msg_icon_path);
			} else {
				MSG_MGR_ERR("fail to get message-app icon");
			}

			break;
		}

		break;
	}
	case MSG_MGR_NOTI_TYPE_CB: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
		break;
	}
	case MSG_MGR_NOTI_TYPE_CLASS0:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	default:
		MSG_MGR_DEBUG("No matching type for MsgNotiSetImage [%d]", noti_info->type);
		break;
	}

	MSG_MGR_END();
}


void setSoundAndVibration(notification_h noti_h, char *addressVal, bool bVoiceMail)
{
	MSG_MGR_BEGIN();

	MSG_MGR_ADDRESS_INFO_S addrInfo = {0, };
	MSG_MGR_CONTACT_INFO_S contactInfo = {0, };

	if (addressVal != NULL) {
		snprintf(addrInfo.addressVal, sizeof(addrInfo.addressVal), "%s", addressVal);
		/* Get Contact Info */
		if (MsgMgrGetContactInfo(&addrInfo, &contactInfo) != 0) {
			MSG_MGR_DEBUG("MsgMgrGetContactInfo() fail.");
		}
	} else {
		MSG_MGR_DEBUG("addressVal is NULL.");
	}

	char *msg_tone_file_path = NULL;

	MsgMgrGetRingtonePath(contactInfo.alerttonePath, &msg_tone_file_path);

	MSG_MGR_SEC_DEBUG("Sound File [%s]", msg_tone_file_path);

	bool bPlaySound = false;
	bool bPlayVibration = false;
	bool bOnCall = false;

	MsgMgrGetPlayStatus(bVoiceMail, &bPlaySound, &bPlayVibration, &bOnCall);

	if (bPlaySound) {
		if (msg_tone_file_path) {
			setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_USER_DATA, msg_tone_file_path);
		} else {
			int tmpVal = 0;
			if (vconf_get_int(MSG_SETTING_RINGTONE_TYPE, &tmpVal) != 0) {
				MSG_MGR_INFO("MsgSettingGetInt() is failed");
			}
			int ringtoneType = tmpVal;
			if (ringtoneType == MSG_RINGTONE_TYPE_SILENT)
				setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
			else
				setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL);
		}
	} else {
		setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
	}

	if (bPlayVibration) {
		if (contactInfo.vibrationPath[0] == '\0')
			setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
		else
			setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_USER_DATA, contactInfo.vibrationPath);
	} else {
		setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
	}

	if (msg_tone_file_path)
		delete [] msg_tone_file_path;

	MSG_MGR_END();
}


void setActiveNotification(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	if (noti_info->active_noti_button_num > 1) {
		createServiceHandle(&noti_info->active_noti_svc_h[0]);
		if (noti_info->active_noti_svc_h[0]) {
			setServicePackageName(noti_info->active_noti_svc_h[0], MSG_CALL_APP_ID);
			setServiceOperation(noti_info->active_noti_svc_h[0], APP_CONTROL_OPERATION_CALL);

			MSG_MGR_DEBUG("Active Notification button 1 - Msg Id = [%d]", noti_info->msg_id);

			char tel_num[MSG_NOTI_TEXT_LEN_S] = {0, };
			snprintf(tel_num, sizeof(tel_num), "tel:%s", noti_info->number);
			MSG_MGR_SEC_DEBUG("Active sender number [%s]", noti_info->number);
			setServiceUri(noti_info->active_noti_svc_h[0], tel_num);
		}

		createServiceHandle(&noti_info->active_noti_svc_h[1]);
		if (noti_info->active_noti_svc_h[1]) {
			setServicePackageName(noti_info->active_noti_svc_h[1], MSG_DEFAULT_APP_ID);

			MSG_MGR_DEBUG("Active Notification button 2 - Msg Id = [%d]", noti_info->msg_id);
			addServiceExtraData(noti_info->active_noti_svc_h[1], "type", "reply");
			addServiceExtraData(noti_info->active_noti_svc_h[1], "msgId", noti_info->msg_id);

			char slot_id[5] = {0, };
			snprintf(slot_id, sizeof(slot_id), "%d", noti_info->sim_idx - 1);
			addServiceExtraData(noti_info->active_noti_svc_h[1], "slot_id", slot_id);
		}
	}

	createServiceHandle(&noti_info->active_noti_svc_h[2]);
	if (noti_info->active_noti_svc_h[2]) {
		setServicePackageName(noti_info->active_noti_svc_h[2], MSG_DEFAULT_APP_ID);

		MSG_MGR_DEBUG("Active Notification button 3 - msgId = [%d]", noti_info->msg_id);
		addServiceExtraData(noti_info->active_noti_svc_h[2], "type", "new_msg");
		addServiceExtraData(noti_info->active_noti_svc_h[2], "msgId", noti_info->msg_id);
		addServiceExtraData(noti_info->active_noti_svc_h[2], "CALLER", "active_noti");

		char slot_id[5] = {0, };
		snprintf(slot_id, sizeof(slot_id), "%d", noti_info->sim_idx - 1);
		addServiceExtraData(noti_info->active_noti_svc_h[2], "slot_id", slot_id);
	}

	if (noti_info->active_noti_button_num > 1) {
		setNotiEventHandler(noti_h, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, noti_info->active_noti_svc_h[0]);
		setNotiEventHandler(noti_h, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_2, noti_info->active_noti_svc_h[1]);
		setNotiEventHandler(noti_h, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_3, noti_info->active_noti_svc_h[2]);

		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_BUTTON_1, "Call", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_BUTTON_2, "Reply", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_BUTTON_3, "View", NULL);
	} else {
		setNotiEventHandler(noti_h, NOTIFICATION_EVENT_TYPE_CLICK_ON_BUTTON_1, noti_info->active_noti_svc_h[2]);

		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_BUTTON_1, "View", NULL);
	}

	setActiveProperty(noti_h, noti_info);

	setTextDomain(noti_h);

	setActiveText(noti_h, noti_info);

	setActiveIcon(noti_h, noti_info);

	noti_err = notification_post(noti_h);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_MGR_DEBUG("Fail to notification_post");
	}

	MSG_MGR_END();
}


void setNotification(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info, bool bFeedback)
{
	MSG_MGR_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	MSG_MGR_DEBUG("active num [%d]", noti_info->active_noti_button_num);

	if (bFeedback && noti_info->active_noti_button_num > 0 &&
		((noti_info->type >= MSG_MGR_NOTI_TYPE_NORMAL && noti_info->type <= MSG_MGR_NOTI_TYPE_SIM) || noti_info->type == MSG_MGR_NOTI_TYPE_CLASS0)) {
		notification_h active_noti_h = notification_create(NOTIFICATION_TYPE_NOTI);

		setActiveNotification(active_noti_h, noti_info);

		notification_free(active_noti_h);
		active_noti_h = NULL;
	}

	setProperty(noti_h, noti_info);

	setTextDomain(noti_h);

	setText(noti_h, noti_info);

	setIcon(noti_h, noti_info);

	if (bFeedback) {
		if (noti_info->type == MSG_MGR_NOTI_TYPE_VOICE_1 || noti_info->type == MSG_MGR_NOTI_TYPE_VOICE_2)
			setSoundAndVibration(noti_h, noti_info->number, true);
		else
			setSoundAndVibration(noti_h, noti_info->number, false);

	} else {
		setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
		setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
	}

	if (noti_info->id > 0) {
		MSG_MGR_DEBUG("Notification update");
		noti_err = notification_update(noti_h);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_MGR_DEBUG("Fail to notification_update");
		}
	} else {
		MSG_MGR_DEBUG("Notification insert");
		noti_err = notification_insert(noti_h, &noti_info->id);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_MGR_DEBUG("Fail to notification_insert");
		}

		updatePrivId(noti_info->type, noti_info->id, noti_info->sim_idx);
	}

	MSG_MGR_END();
}


void createActiveInfoData(MSG_MGR_NOTI_INFO_S *noti_info, MSG_MGR_MESSAGE_INFO_S *msg_info)
{
	MSG_MGR_BEGIN();

	if (!msg_info) {
		MSG_MGR_DEBUG("msg_info is NULL");
		return;
	}

	noti_info->msg_id = msg_info->msgId;
	noti_info->sim_idx = msg_info->sim_idx;

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL: {
		char *senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, PUSH_MESSAGE);
		snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", senderStr);
		if (senderStr) {
			free(senderStr);
			senderStr = NULL;
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_CLASS0: {
		if (msg_info->displayName[0] == '\0')
			snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", msg_info->addressVal);
		else
			snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", msg_info->displayName);

		snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s", msg_info->msgText);
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_MGR_END();
}


void clearInfoData(notification_h noti_h, MSG_MGR_NOTI_INFO_S *noti_info)
{
	MSG_MGR_BEGIN();

	if (noti_h) {
		notification_free(noti_h);
		noti_h = NULL;
	}

	if (noti_info->svc_h) {
		app_control_destroy(noti_info->svc_h);
		noti_info->svc_h = NULL;
	}

	for (int i = 0; i < MSG_ACTIVE_NOTI_BUTTON_NUM; i++) {
		if (noti_info->active_noti_svc_h[i]) {
			app_control_destroy(noti_info->active_noti_svc_h[i]);
			noti_info->active_noti_svc_h[i] = NULL;
		}
	}

	MSG_MGR_END();
}


int getAppIcon(const char *app_id, char **icon_path)
{
	MSG_MGR_BEGIN();

	package_info_h pkg_info_h = NULL;
	int pkg_err = PACKAGE_MANAGER_ERROR_NONE;
	int ret = 0;

	if (app_id == NULL) {
		MSG_MGR_ERR("app id is NULL");
		ret = -1;
		goto END_OF_GET_APP_ICON;
	}

	pkg_err = package_info_create(app_id, &pkg_info_h);
	if (pkg_err != PACKAGE_MANAGER_ERROR_NONE) {
		MSG_MGR_ERR("package_info_create failed (%d)", pkg_err);
		ret = -1;
		goto END_OF_GET_APP_ICON;
	}

	pkg_err = package_info_get_icon(pkg_info_h, icon_path);
	if (pkg_err != PACKAGE_MANAGER_ERROR_NONE) {
		MSG_MGR_ERR("package_info_get_icon failed (%d)", pkg_err);
		ret = -1;
	} else {
		if (icon_path == NULL) {
			MSG_MGR_WARN("icon path is NULL");
			ret = -1;
		}
	}

END_OF_GET_APP_ICON:
	if (pkg_info_h) {
		pkg_err = package_info_destroy(pkg_info_h);
		if (pkg_err != PACKAGE_MANAGER_ERROR_NONE) {
			MSG_MGR_ERR("package_info_destroy failed (%d)", pkg_err);
		}

		pkg_info_h = NULL;
	}

	MSG_MGR_END();

	return ret;
}


int getLatestMsgInfo(MSG_MGR_NOTI_INFO_S *noti_info, bool isForInstantMessage)
{
	MSG_MGR_BEGIN();

	int noti_err = 0;
	msg_error_t msg_err = MSG_SUCCESS;
	char **db_res = NULL;
	int row_cnt = 0, col_cnt = 0;

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL:
#ifdef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_CB:
	case MSG_MGR_NOTI_TYPE_SIM:
#endif
	{
	int smsUnreadCnt = 0;
		int mmsUnreadCnt = 0;

		char sqlQuery[MAX_QUERY_LEN	+1];
		unsigned char mainType;
		unsigned char subType;
		int msgSize;

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery), "DISTINCT "
				"A.ADDRESS_VAL, "
				"B.SUB_TYPE "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND (B.FOLDER_ID=%d OR B.FOLDER_ID=%d) "
				"AND B.STORAGE_ID = %d "
				"GROUP BY A.ADDRESS_VAL "
				"ORDER BY B.DISPLAY_TIME DESC LIMIT 5;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_INBOX_ID, MSG_CBMSGBOX_ID,
				MSG_STORAGE_PHONE);
#else
		snprintf(sqlQuery, sizeof(sqlQuery), "DISTINCT "
				"A.ADDRESS_VAL, "
				"B.SUB_TYPE "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND B.FOLDER_ID=%d "
				"AND B.STORAGE_ID = %d "
				"GROUP BY A.ADDRESS_VAL "
				"ORDER BY B.DISPLAY_TIME DESC LIMIT 5;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_INBOX_ID,
				MSG_STORAGE_PHONE);
#endif
		MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);

		row_cnt = 0, col_cnt = 0;
		msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &row_cnt, &col_cnt);
		if (msg_err != MSG_SUCCESS) {
			MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
			return -1;
		}

		MSG_MGR_ADDRESS_INFO_S tmpAddressInfo;
		int normalAddCnt = 0;
		int index = col_cnt;

		for (int i = 1; i <= row_cnt; i++) {
			memset(&tmpAddressInfo, 0x00, sizeof(MSG_MGR_ADDRESS_INFO_S));

			char *address = db_res[index++];
			normalAddCnt++;
			if (address) {
				snprintf(tmpAddressInfo.addressVal, MAX_ADDRESS_VAL_LEN, "%s", address);
				if (_is_valid_email(address)) {
					tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_EMAIL;
				} else {
					tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_UNKNOWN;
				}
			}
			subType = atoi(db_res[index++]);

			MSG_MGR_CONTACT_INFO_S tmpContact;
			memset(&tmpContact, 0x00, sizeof(MSG_MGR_CONTACT_INFO_S));

			MsgMgrGetContactInfo(&tmpAddressInfo, &tmpContact);

			if (row_cnt == 1) {
				snprintf(noti_info->imagePath, sizeof(noti_info->imagePath), "%s", tmpContact.imagePath);
			}

			if (normalAddCnt > 1) {
				g_strlcat(noti_info->sender, ", ", sizeof(noti_info->sender)-strlen(noti_info->sender));
			}

			if (tmpContact.firstName[0] != '\0') {
				g_strlcat(noti_info->sender, tmpContact.firstName, sizeof(noti_info->sender)-strlen(noti_info->sender));
			} else if (tmpAddressInfo.addressVal[0] == '\0') {
				char *senderStr = NULL;
				senderStr = get_translate_text("message", MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
				g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
				if (senderStr) {
					free(senderStr);
					senderStr = NULL;
				}

				if (i == 1) {
					noti_info->active_noti_button_num = 1;
				}
			} else {
				char *senderStr = NULL;
				if (subType == MSG_CB_SMS) {
					senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, CB_MESSAGE);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					noti_info->active_noti_button_num = 1;
				} else if (subType == MSG_SYNCML_CP) {
					senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, CP_MESSAGE);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					noti_info->active_noti_button_num = 1;
				} else if (subType == MSG_WAP_SI_SMS || subType == MSG_WAP_SL_SMS) {
					senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, PUSH_MESSAGE);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					noti_info->active_noti_button_num = 1;
				} else {
					g_strlcat(noti_info->sender, tmpAddressInfo.addressVal, sizeof(noti_info->sender)-strlen(noti_info->sender));
				}

				if (senderStr) {
					free(senderStr);
					senderStr = NULL;
				}
			}

			if (i == 1) {
				noti_info->active_subtype = subType;
				snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", noti_info->sender);
				snprintf(noti_info->imagePath, sizeof(noti_info->imagePath), "%s", tmpContact.imagePath);
			}
		}

		noti_info->senderCount = normalAddCnt;
		msg_db_free(msg_handle, db_res);

		MSG_MGR_SEC_DEBUG("sender info = [%s]", noti_info->sender);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery),
				"A.ADDRESS_VAL, "
				"B.DISPLAY_TIME, "
				"B.MSG_ID, "
				"B.SUBJECT, "
				"B.MSG_TEXT, "
				"B.MAIN_TYPE, "
				"(COUNT(DISTINCT(CASE WHEN B.MAIN_TYPE = %d THEN B.MSG_ID END))) AS SMS_UNREAD_CNT, "
				"(COUNT(DISTINCT(CASE WHEN B.MAIN_TYPE = %d THEN B.MSG_ID END))) AS MMS_UNREAD_CNT, "
				"(CASE WHEN B.MAIN_TYPE = %d AND B.NETWORK_STATUS = %d THEN (SELECT C.MSG_SIZE FROM %s C WHERE B.MSG_ID = C.MSG_ID) ELSE -1 END) "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND (B.FOLDER_ID=%d OR B.FOLDER_ID=%d) "
				"AND B.STORAGE_ID = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSG_SMS_TYPE,
				MSG_MMS_TYPE,
				MSG_MMS_TYPE, MSG_NETWORK_RECEIVED, MMS_PLUGIN_MESSAGE_TABLE_NAME,
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_INBOX_ID, MSG_CBMSGBOX_ID,
				MSG_STORAGE_PHONE);
#else
		snprintf(sqlQuery, sizeof(sqlQuery),
				"A.ADDRESS_VAL, "
				"B.DISPLAY_TIME, "
				"B.MSG_ID, "
				"B.SUBJECT, "
				"B.MSG_TEXT, "
				"B.MAIN_TYPE, "
				"(COUNT(CASE WHEN B.MAIN_TYPE = %d THEN 1 END)) AS SMS_UNREAD_CNT, "
				"(COUNT(CASE WHEN B.MAIN_TYPE = %d THEN 1 END)) AS MMS_UNREAD_CNT "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND B.FOLDER_ID=%d "
				"AND B.STORAGE_ID = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSG_SMS_TYPE,
				MSG_MMS_TYPE,
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_INBOX_ID,
				MSG_STORAGE_PHONE);
#endif
		MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);

		msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &row_cnt, &col_cnt);
		if (msg_err != MSG_SUCCESS) {
			MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
			return -1;
		}

		if (row_cnt > 0) {
			smsUnreadCnt = atoi(db_res[col_cnt+6]);
			mmsUnreadCnt = atoi(db_res[col_cnt+7]);
			msgSize = atoi(db_res[col_cnt+8]);

			noti_info->count = smsUnreadCnt + mmsUnreadCnt;

			if (noti_info->count > 0) {
				snprintf(noti_info->number, sizeof(noti_info->number), "%s", db_res[col_cnt]);

				noti_info->time = (time_t)atoi(db_res[col_cnt+1]);

				noti_info->msg_id = (msg_message_id_t)atoi(db_res[col_cnt+2]);

				mainType = (unsigned char)atoi(db_res[col_cnt+5]);

				if (mainType == MSG_MMS_TYPE) {
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", db_res[col_cnt+3]);
					if (noti_info->text[0] == '\0') {
						char *noti_text = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_NO_SUBJECT);
						snprintf(noti_info->text, sizeof(noti_info->text), "%s", noti_text);
						g_free(noti_text);
					}

					char *prefix_subject = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_SUBJECT_COLON);
					if (prefix_subject) {
						snprintf(noti_info->active_subject, MSG_NOTI_TEXT_LEN_S, "%s%s", prefix_subject, noti_info->text);
						g_free(prefix_subject);
					} else {
						snprintf(noti_info->active_subject, MSG_NOTI_TEXT_LEN_S, "%s", noti_info->text);
					}

					if (msgSize > -1) {
						int kb_msg_size = msgSize / 1024;
						if (kb_msg_size == 0 && msgSize > 0)
							kb_msg_size = 1;
						else if (msgSize % 1024 >= 512)
							kb_msg_size++;

						char *msg_size_string = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MESSAGE_SIZE_STRING);
						char *msg_size_unit_kb = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MESSAGE_SIZE_UNIT_KB);

						snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s : %d%s", msg_size_string, kb_msg_size, msg_size_unit_kb);

						g_free(msg_size_string);
						g_free(msg_size_unit_kb);
					}

				} else {
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", db_res[col_cnt+4]);
				}

				if (noti_info->active_text[0] == '\0')
					snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s", db_res[col_cnt+4]);

				MSG_MGR_DEBUG("unread message ID [%d].", noti_info->msg_id);

				MSG_MGR_DEBUG("active sender [%s]", noti_info->active_sender);
				MSG_MGR_DEBUG("active subject [%s]", noti_info->active_subject);
				MSG_MGR_DEBUG("active text [%s]", noti_info->active_text);

				if (!isForInstantMessage) {
					if (noti_info->id > 0 && noti_info->count == 1) {
						noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}

						noti_info->id = 0;
						if (vconf_set_int(NOTIFICATION_PRIV_ID, noti_info->id) != 0)
							MSG_MGR_DEBUG("vconf_set_int fail : NOTIFICATION_PRIV_ID");
					}

					vconf_set_int(VCONFKEY_MESSAGE_RECV_SMS_STATE, smsUnreadCnt);
					vconf_set_int(VCONFKEY_MESSAGE_RECV_MMS_STATE, mmsUnreadCnt);
					MsgMgrInsertBadge(noti_info->count);
					MsgMgrSoundSetRepeatAlarm();
				}
			} else {
				MSG_MGR_DEBUG("No unread message.");
				MSG_MGR_DEBUG("notiPrivId [%d]", noti_info->id);

				msg_db_free(msg_handle, db_res);

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (vconf_set_int(NOTIFICATION_PRIV_ID, noti_info->id) != 0)
						MSG_MGR_DEBUG("vconf_set_int fail : NOTIFICATION_PRIV_ID");

					vconf_set_int(VCONFKEY_MESSAGE_RECV_SMS_STATE, 0);
					vconf_set_int(VCONFKEY_MESSAGE_RECV_MMS_STATE, 0);
					MsgMgrInsertBadge(0);
					MsgMgrSoundSetRepeatAlarm();
				}

				return -1;
			}
		} else {
			MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);
			msg_db_free(msg_handle, db_res);
			return -1;
		}

		msg_db_free(msg_handle, db_res);
		break;
	}

#ifndef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_CB: {
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"A.ADDRESS_VAL, "
				"B.DISPLAY_TIME, "
				"B.MSG_ID, "
				"B.MSG_TEXT "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 "
				"AND B.FOLDER_ID=%d "
				"AND B.STORAGE_ID = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_CBMSGBOX_ID,
				MSG_STORAGE_PHONE);

		if (dbhandler->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbhandler->stepQuery() == MSG_ERR_DB_ROW) {
			MSG_ADDRESS_INFO_S addrInfo;
			memset(&addrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

			if (dbhandler->columnText(0) != NULL)
				snprintf(addrInfo.addressVal, sizeof(addrInfo.addressVal), "%s", (char*)dbhandler->columnText(0));

			MSG_CONTACT_INFO_S tmpContact;
			memset(&tmpContact, 0x00, sizeof(MSG_CONTACT_INFO_S));

			MsgGetContactInfo(&addrInfo, &tmpContact);

			if (tmpContact.firstName[0] != '\0') {
				snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", tmpContact.firstName);
			} else if (addrInfo.addressVal[0] == '\0') {
				char *senderStr = NULL;
				senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
				g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
				if (senderStr) {
					free(senderStr);
					senderStr = NULL;
				}
			} else {
				snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", addrInfo.addressVal);
			}

			snprintf(noti_info->number, sizeof(noti_info->number), "%s", addrInfo.addressVal);

			noti_info->time = (time_t)dbhandler->columnInt(1);

			noti_info->msg_id = (msg_message_id_t)dbhandler->columnInt(2);

			snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(3));

			MSG_MGR_DEBUG("unread CB message [%d].", noti_info->msg_id);
		} else {
			MSG_MGR_DEBUG("No unread CB message.");
			MSG_MGR_DEBUG("notiCbId [%d]", noti_info->id);

			dbhandler->finalizeQuery();

			if (!isForInstantMessage) {
				/* No unread message. */
				if (noti_info->id > 0) {
					noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
					if (noti_err != NOTIFICATION_ERROR_NONE) {
						MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
					}
				}

				noti_info->id = 0;

				if (MsgSettingSetInt(CB_NOTI_PRIV_ID, noti_info->id) != MSG_SUCCESS)
					MSG_MGR_DEBUG("MsgSettingSetInt fail : CB_NOTI_PRIV_ID");
			}
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();

		if (dbhandler->getTable(sqlQuery, &noti_info->count, NULL) != MSG_SUCCESS) {
			MSG_MGR_DEBUG("getTable is failed");
			dbhandler->freeTable();
			return MSG_ERR_DB_GETTABLE;
		}

		dbhandler->freeTable();
		MSG_MGR_DEBUG("notiCbId [%d], unreadCbMsgCnt [%d]", noti_info->id, noti_info->count);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM: {
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"A.ADDRESS_VAL, "
				"B.DISPLAY_TIME, "
				"B.MSG_ID, "
				"B.MSG_TEXT, "
				"(COUNT(CASE WHEN B.MAIN_TYPE = %d THEN 1 END)) AS SMS_UNREAD_CNT "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND B.FOLDER_ID=%d "
				"AND B.STORAGE_ID = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSG_SMS_TYPE,
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_INBOX_ID,
				MSG_STORAGE_SIM);

		MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);

		if (dbhandler->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbhandler->stepQuery() == MSG_ERR_DB_ROW) {
			noti_info->count = dbhandler->columnInt(4);

			if (noti_info->count > 0) {
				MSG_ADDRESS_INFO_S addrInfo;
				memset(&addrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				if (dbhandler->columnText(0) != NULL)
					snprintf(addrInfo.addressVal, sizeof(addrInfo.addressVal), "%s", (char*)dbhandler->columnText(0));

				MSG_CONTACT_INFO_S tmpContact;
				memset(&tmpContact, 0x00, sizeof(MSG_CONTACT_INFO_S));

				MsgGetContactInfo(&addrInfo, &tmpContact);

				if (tmpContact.firstName[0] != '\0') {
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", tmpContact.firstName);
				} else if (addrInfo.addressVal[0] == '\0') {
					char *senderStr = NULL;
					senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					if (senderStr) {
						free(senderStr);
						senderStr = NULL;
					}
				} else {
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", addrInfo.addressVal);
				}

				snprintf(noti_info->number, sizeof(noti_info->number), "%s", addrInfo.addressVal);

				noti_info->time = (time_t)dbhandler->columnInt(1);

				noti_info->msg_id = (msg_message_id_t)dbhandler->columnInt(2);

				snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(3));

				MSG_MGR_DEBUG("unread SIM message [%d].", noti_info->msg_id);
			} else {
				MSG_MGR_DEBUG("No unread SIM message.");
				MSG_MGR_DEBUG("notiPrivId [%d]", noti_info->id);

				dbhandler->finalizeQuery();

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (MsgSettingSetInt(SIM_MSG_NOTI_PRIV_ID, noti_info->id) != MSG_SUCCESS)
						MSG_MGR_DEBUG("MsgSettingSetInt fail : SIM_MSG_NOTI_PRIV_ID");
				}

				return MSG_ERR_DB_STEP;
			}
		} else {
			MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);
			dbhandler->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();
		break;
	}
#endif
	case MSG_MGR_NOTI_TYPE_FAILED: {
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"A.ADDRESS_VAL, "
				"B.DISPLAY_TIME, "
				"B.MSG_ID, "
				"B.MSG_TEXT, "
				"B.SUBJECT, "
				"B.MAIN_TYPE, "
				"(COUNT(CASE WHEN B.NETWORK_STATUS = %d THEN 1 END)) AS SENT_FAILED_CNT "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID=B.CONV_ID "
				"AND B.READ_STATUS=0 AND B.FOLDER_ID=%d "
				"AND B.STORAGE_ID = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSG_NETWORK_SEND_FAIL,
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_OUTBOX_ID,
				MSG_STORAGE_PHONE);

		MSG_MGR_DEBUG("sqlQuery [%s]", sqlQuery);

		row_cnt = 0, col_cnt = 0;
		msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &row_cnt, &col_cnt);
		if (msg_err != MSG_SUCCESS) {
			MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
			return -1;
		}

		if (row_cnt > 0) {
			noti_info->count = atoi(db_res[col_cnt+6]);

			if (noti_info->count > 0) {
				MSG_MGR_ADDRESS_INFO_S addrInfo = {0, };

				snprintf(addrInfo.addressVal, MAX_ADDRESS_VAL_LEN, "%s", db_res[col_cnt]);

				MSG_MGR_CONTACT_INFO_S tmpContact = {0, };

				MsgMgrGetContactInfo(&addrInfo, &tmpContact);

				if (tmpContact.firstName[0] != '\0') {
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", tmpContact.firstName);
				} else if (addrInfo.addressVal[0] == '\0') {
					char *senderStr = NULL;
					senderStr = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					if (senderStr) {
						free(senderStr);
						senderStr = NULL;
					}
				} else {
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", addrInfo.addressVal);
				}

				snprintf(noti_info->number, sizeof(noti_info->number), "%s", addrInfo.addressVal);

				noti_info->time = (time_t)atoi(db_res[col_cnt+1]);

				noti_info->msg_id = (msg_message_id_t)atoi(db_res[col_cnt+2]);

				unsigned char mainType = (unsigned char)atoi(db_res[col_cnt+5]);

				if (mainType == MSG_TYPE_MMS)
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", db_res[col_cnt+4]);
				else
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", db_res[col_cnt+3]);

				MSG_MGR_DEBUG("Sent failed message ID [%d].", noti_info->msg_id);

				if (!isForInstantMessage) {
					if (noti_info->id > 0 && noti_info->count == 1) {
						noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
						noti_info->id = 0;
						if (vconf_set_int(MSG_SENTFAIL_NOTI_ID, noti_info->id) != 0)
							MSG_MGR_DEBUG("vconf_set_int fail : MSG_SENTFAIL_NOTI_ID");
					}
				}
			} else {
				MSG_MGR_DEBUG("No sent failed message.");
				MSG_MGR_DEBUG("failedNotiId [%d]", noti_info->id);

				msg_db_free(msg_handle, db_res);

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_MGR_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (vconf_set_int(MSG_SENTFAIL_NOTI_ID, noti_info->id) != 0)
						MSG_MGR_DEBUG("vconf_set_int fail : MSG_SENTFAIL_NOTI_ID");
				}

				return -1;
			}
		} else {
			msg_db_free(msg_handle, db_res);
			return -1;
		}

		msg_db_free(msg_handle, db_res);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL:
		break;
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_info->type);
		return -1;
	}

	MSG_MGR_END();

	return 0;
}


notification_h getHandle(int *noti_id)
{
	notification_h noti_h = NULL;

	if (*noti_id > 0) {
		MSG_MGR_DEBUG("Notification load");
		noti_h = notification_load(NULL, *noti_id);
		if (noti_h == NULL)
			MSG_MGR_DEBUG("notification_load is failed.");
	}

	if (noti_h == NULL) {
		MSG_MGR_DEBUG("Notification create");
		noti_h = notification_create(NOTIFICATION_TYPE_NOTI);
		if (noti_h == NULL) {
			MSG_MGR_DEBUG("notification_create is failed.");
			return NULL;
		}

		*noti_id = 0;
	}

	return noti_h;
}


int getPrivId(msg_mgr_notification_type_t noti_type, int sim_idx)
{
	MSG_MGR_BEGIN();

	int noti_id = 0;

	switch (noti_type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_SIM:
	case MSG_MGR_NOTI_TYPE_CB:
		vconf_get_int(NOTIFICATION_PRIV_ID, &noti_id);
		break;
#else
	case MSG_MGR_NOTI_TYPE_NORMAL:
		vconf_get_int(NOTIFICATION_PRIV_ID, &noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_SIM:
		vconf_get_int(SIM_MSG_NOTI_PRIV_ID, &noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_CB:
		vconf_get_int(CB_NOTI_PRIV_ID, &noti_id);
		break;
#endif
	case MSG_MGR_NOTI_TYPE_FAILED:
		vconf_get_int(MSG_SENTFAIL_NOTI_ID, &noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_VOICE_1: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		vconf_get_int(keyName, &noti_id);
		break;
	}
	case MSG_MGR_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_2, sim_idx);
		vconf_get_int(keyName, &noti_id);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL:
		vconf_get_int(SIM_FULL_NOTI_PRIV_ID, &noti_id);
		break;
	default:
		MSG_MGR_DEBUG("No matching noti type [%d]", noti_type);
		break;
	}

	MSG_MGR_DEBUG("Get noti type = %d, id = %d, sim_idx:%d", noti_type, noti_id, sim_idx);

	MSG_MGR_END();

	return noti_id;
}


void updatePrivId(msg_mgr_notification_type_t noti_type, int noti_id, int sim_idx)
{
	MSG_MGR_BEGIN();

	int err = 0;

	MSG_MGR_DEBUG("Update noti type = %d, id = %d, sim_idx = %d", noti_type, noti_id, sim_idx);

	switch (noti_type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_SIM:
	case MSG_MGR_NOTI_TYPE_CB:
		err = vconf_set_int(NOTIFICATION_PRIV_ID, noti_id);
		break;
#else
	case MSG_MGR_NOTI_TYPE_NORMAL:
		err = vconf_set_int(NOTIFICATION_PRIV_ID, noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_SIM:
		err = vconf_set_int(SIM_MSG_NOTI_PRIV_ID, noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_CB:
		err = vconf_set_int(CB_NOTI_PRIV_ID, noti_id);
		break;
#endif
	case MSG_MGR_NOTI_TYPE_FAILED:
		err = vconf_set_int(MSG_SENTFAIL_NOTI_ID, noti_id);
		break;
	case MSG_MGR_NOTI_TYPE_VOICE_1: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		err = vconf_set_int(keyName, noti_id);
		break;
	}
	case MSG_MGR_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_2, sim_idx);
		err = vconf_set_int(keyName, noti_id);
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL:
		err = vconf_set_int(SIM_FULL_NOTI_PRIV_ID, noti_id);
		break;
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_type);
		break;
	}

	if (err != 0)
		MSG_MGR_INFO("vconf_set_int fail : noti type = %d, id = %d, sim_idx = %d", noti_type, noti_id, sim_idx);

	MSG_MGR_END();
}


void createInfoData(MSG_MGR_NOTI_INFO_S *noti_info, MSG_MGR_MESSAGE_INFO_S *msg_info)
{
	MSG_MGR_BEGIN();

	if (msg_info) {
		noti_info->id = getPrivId(noti_info->type, msg_info->sim_idx);
		noti_info->msg_id = msg_info->msgId;
	} else {
		MSG_MGR_DEBUG("msg_info is NULL");
		return;
	}

	noti_info->sim_idx = msg_info->sim_idx;

	createServiceHandle(&noti_info->svc_h);
	char keyName[MAX_VCONFKEY_NAME_LEN];

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_VOICE_1:
	case MSG_MGR_NOTI_TYPE_VOICE_2: {
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, msg_info->sim_idx);
		vconf_get_int(keyName, &noti_info->count);
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, msg_info->sim_idx);
		char *voiceNumber = vconf_get_str(keyName);
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, msg_info->sim_idx);
		char *voiceAlphaId = vconf_get_str(keyName);
		char *dialNumber = NULL;

		MSG_MGR_SEC_DEBUG("Voice mail server - alpha id = [%s], default num = [%s]", voiceAlphaId, voiceNumber);

		if (voiceNumber && strlen(voiceNumber))
			dialNumber = voiceNumber;

		if (voiceAlphaId && strlen(voiceAlphaId) > 0) {
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", voiceAlphaId);
		} else if (dialNumber && strlen(dialNumber) > 0) {
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", dialNumber);
		}

		if (dialNumber && strlen(dialNumber) > 0)
			snprintf(noti_info->number, sizeof(noti_info->number), "%s", dialNumber);

		if (noti_info->svc_h) {
			setServiceOperation(noti_info->svc_h, APP_CONTROL_OPERATION_CALL);
			setServiceUri(noti_info->svc_h, MSG_TEL_URI_VOICEMAIL);

			char slot_id[5] = {0, };
			snprintf(slot_id, sizeof(slot_id), "%d", msg_info->sim_idx - 1);
			addServiceExtraData(noti_info->svc_h, "slot_id", slot_id);
		}

		if (voiceNumber)	g_free(voiceNumber);
		if (voiceAlphaId) g_free(voiceAlphaId);
		break;
	}
	case MSG_MGR_NOTI_TYPE_MWI:
	case MSG_MGR_NOTI_TYPE_CLASS0: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

		if (msg_info->displayName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->displayName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s", msg_info->addressVal);

		snprintf(noti_info->text, sizeof(noti_info->text), "%s", msg_info->msgText);

		if (noti_info->type == MSG_MGR_NOTI_TYPE_MWI) {
			if (noti_info->svc_h) {
				setServiceOperation(noti_info->svc_h, APP_CONTROL_OPERATION_CALL);
				setServiceUri(noti_info->svc_h, MSG_TEL_URI_VOICEMAIL);

				char slot_id[5] = {0, };
				snprintf(slot_id, sizeof(slot_id), "%d", msg_info->sim_idx - 1);
				addServiceExtraData(noti_info->svc_h, "slot_id", slot_id);
			}

		} else {
			setServiceAppId(noti_info->svc_h, "org.tizen.msg-ui-class0");
			addServiceExtraData(noti_info->svc_h, "type", "new_msg");
			addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_SMS_DELIVERY_REPORT: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;
		noti_info->extra_data = msg_info->networkStatus;

		MSG_MGR_CONTACT_INFO_S contactInfo = {0, };
		MSG_MGR_ADDRESS_INFO_S tmpAddressInfo = {0, };
		if (msg_info->addressVal[0] != '\0') {
			snprintf(tmpAddressInfo.addressVal, MAX_ADDRESS_VAL_LEN, "%s", msg_info->addressVal);
			if (_is_valid_email(msg_info->addressVal)) {
				tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_EMAIL;
			} else {
				tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_UNKNOWN;
			}
		}

		if (MsgMgrGetContactInfo(&tmpAddressInfo, &contactInfo) != 0) {
			MSG_MGR_WARN("MsgMgrGetContactInfo() fail.");
		}

		if (contactInfo.firstName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", contactInfo.firstName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s", msg_info->addressVal);

		if (noti_info->msg_id > 0) {
			setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
			addServiceExtraData(noti_info->svc_h, "type", "new_msg");
			addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
			addServiceExtraData(noti_info->svc_h, "address", msg_info->addressVal);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_MMS_READ_REPORT:
	case MSG_MGR_NOTI_TYPE_MMS_DELIVERY_REPORT: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

		MSG_MGR_CONTACT_INFO_S contactInfo = {0, };
		MSG_MGR_ADDRESS_INFO_S tmpAddressInfo = {0, };
		if (msg_info->addressVal[0] != '\0') {
			snprintf(tmpAddressInfo.addressVal, MAX_ADDRESS_VAL_LEN, "%s", msg_info->addressVal);
			if (_is_valid_email(msg_info->addressVal)) {
				tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_EMAIL;
			} else {
				tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_UNKNOWN;
			}
		}

		if (MsgMgrGetContactInfo(&tmpAddressInfo, &contactInfo) != 0) {
			MSG_MGR_WARN("MsgMgrGetContactInfo() fail.");
		}
		if (contactInfo.firstName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", contactInfo.firstName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s", msg_info->addressVal);

		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		int report_status_type;
		int report_status_value;

		if (noti_info->type == MSG_MGR_NOTI_TYPE_MMS_READ_REPORT) {
			report_status_type = MSG_REPORT_TYPE_READ;
		} else {
			report_status_type = MSG_REPORT_TYPE_DELIVERY;
		}

		snprintf(sqlQuery, sizeof(sqlQuery),
				"STATUS "
				"FROM %s "
				"WHERE MSG_ID=%d AND STATUS_TYPE=%d AND ADDRESS_VAL LIKE '%%%s';",
				MSGFW_REPORT_TABLE_NAME, msg_info->msgId, report_status_type, msg_mgr_normalize_number(msg_info->addressVal));

		MSG_MGR_DEBUG("sqlQuery = [%s]", sqlQuery);

		char **db_res = NULL;
		int row_cnt = 0, col_cnt = 0;

		int msg_err = msg_db_select_with_query(msg_handle, sqlQuery, &db_res, &row_cnt, &col_cnt);
		if (msg_err != MSG_SUCCESS || row_cnt <= 0) {
			MSG_MGR_ERR("msg_db_select_with_query() failed [%d]", msg_err);
			return;
		}

		report_status_value = atoi(db_res[col_cnt]);

		MSG_MGR_DEBUG("report status [type = %d, value = %d]", report_status_type, report_status_value);

		msg_err = msg_db_free(msg_handle, db_res);
		if (msg_err != MSG_SUCCESS) {
			MSG_MGR_ERR("msg_db_free() failed [%d]", msg_err);
			return;
		}

		if (noti_info->msg_id > 0) {
			setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
			addServiceExtraData(noti_info->svc_h, "type", "new_msg");
			addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
			addServiceExtraData(noti_info->svc_h, "address", msg_info->addressVal);
		}

		noti_info->extra_data = (unsigned char)report_status_value;
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
	MSG_MGR_END();
}


void createInfoData(MSG_MGR_NOTI_INFO_S *noti_info, msg_mgr_active_notification_type_t active_noti)
{
	MSG_MGR_BEGIN();

	createServiceHandle(&noti_info->svc_h);

	switch (noti_info->type) {
	case MSG_MGR_NOTI_TYPE_NORMAL: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
		addServiceExtraData(noti_info->svc_h, "http://tizen.org/appcontrol/data/notification", "new_message");

		noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER;

		if (noti_info->active_noti_button_num == 0)
			noti_info->active_noti_button_num = 3;
		break;
	}
	case MSG_MGR_NOTI_TYPE_CB: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		if (active_noti == MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT)
			noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
		else
			noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;

		noti_info->active_noti_button_num = 1;
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		if (active_noti == MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT)
			noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
		else
			noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;

		if (noti_info->active_noti_button_num == 0)
			noti_info->active_noti_button_num = 3;
		break;
	}
	case MSG_MGR_NOTI_TYPE_FAILED: {
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "send_failed_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER^NOTIFICATION_DISPLAY_APP_LOCK;
		break;
	}
	case MSG_MGR_NOTI_TYPE_SIM_FULL: {
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "sim_list_show", "sim_setting");

		noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;
		break;
	}
	default:
		break;
	}

	if (active_noti != MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE)
		noti_info->active_noti_button_num = 0;

	MSG_MGR_END();
}


void createServiceHandle(app_control_h *svc_h)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_create(svc_h);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_create() is failed, [%d]", svc_err);
}


void setServiceAppId(app_control_h svc_h, const char* app_id)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_app_id(svc_h, app_id);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_set_app_id() was failed, [%d]", svc_err);
}


void setServiceUri(app_control_h svc_h, const char* uri)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_uri(svc_h, uri);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_set_uri() was failed, [%d]", svc_err);
}


void setServiceOperation(app_control_h svc_h, const char* operation)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_operation(svc_h, operation);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_set_operation() was failed, [%d]", svc_err);
}


void addServiceExtraData(app_control_h svc_h, const char* bundle_key, const char* bundle_val)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_add_extra_data(svc_h, bundle_key, bundle_val);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_add_extra_data() was failed, [%d]", svc_err);
}


void addServiceExtraData(app_control_h svc_h, const char* bundle_key, int bundle_val)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	char tempId[10];
	memset(&tempId, 0x00, sizeof(tempId));
	snprintf(tempId, sizeof(tempId), "%d", bundle_val);

	svc_err = app_control_add_extra_data(svc_h, bundle_key, (const char *)tempId);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_add_extra_data() was failed, [%d]", svc_err);
}


void setServicePackageName(app_control_h svc_h, const char* pkg_name)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_app_id(svc_h, pkg_name);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_set_app_id() was failed, [%d]", svc_err);
}


void sendServicelaunchRequest(app_control_h svc_h, app_control_reply_cb callback, void *user_data)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_send_launch_request(svc_h, callback, user_data);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_MGR_DEBUG("app_control_send_launch_request() is failed : %d", svc_err);
}


void setNotiTextDomain(notification_h noti_h, const char *pkg_name, const char *loc_dir)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_text_domain(noti_h, pkg_name, loc_dir);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_text_domain() was failed. [%d]", noti_err);
}


void setNotiText(notification_h noti_h, notification_text_type_e type, const char *text, const char *key)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_text(noti_h, type, text, key, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_text() was failed. [%d]", noti_err);
}


void setNotiTimeToText(notification_h noti_h, notification_text_type_e type, time_t time)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_time_to_text(noti_h, type, time);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_time_to_text() was failed. [%d]", noti_err);
}


void setNotiTime(notification_h noti_h, time_t time)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_time(noti_h, time);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_time() was failed. [%d]", noti_err);
}



void setNotiImage(notification_h noti_h, notification_image_type_e type, const char *image_path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_image(noti_h, type, image_path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_image() was failed. [%d]", noti_err);
}


void setNotiSound(notification_h noti_h, notification_sound_type_e type, const char *path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_sound(noti_h, type, path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_sound() was failed. [%d]", noti_err);
}


void setNotiVibration(notification_h noti_h, notification_vibration_type_e type, const char *path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_vibration(noti_h, type, path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_vibration() was failed. [%d]", noti_err);
}


void setNotiEventHandler(notification_h noti_h, notification_event_type_e type, app_control_h event_handler)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_event_handler(noti_h, type, event_handler);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("notification_set_event_handler() was failed. [%d]", noti_err);
}


int MsgMgrInsertInstantMessage(msg_mgr_notification_type_t noti_type)
{
	MSG_MGR_BEGIN();

	char *notiMsg = NULL;

	notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);

	switch (noti_type) {
	case MSG_MGR_NOTI_TYPE_NORMAL:
	case MSG_MGR_NOTI_TYPE_SIM:
	case MSG_MGR_NOTI_TYPE_CB: {
		MSG_MGR_NOTI_INFO_S noti_info;
		memset(&noti_info, 0x00, sizeof(MSG_MGR_NOTI_INFO_S));

		noti_info.type = noti_type;
		int err = getLatestMsgInfo(&noti_info, true);

		if (err == 0) {
			MSG_MGR_DEBUG("Unread count [%d]", noti_info.count);
			if (noti_info.count == 1) {
				MSG_MGR_SEC_DEBUG("noti_info.sender [%s]", noti_info.sender);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_TITLE, noti_info.sender, NULL);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info.text, NULL);
			} else if (noti_info.count > 1) {
				gchar *cnt_string = g_strdup_printf("%i", noti_info.count);

				notiMsg = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, NEW_MESSAGES);
				gchar *outString = g_strconcat(cnt_string, " ", notiMsg, NULL);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_TITLE, outString, NULL);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info.sender, NULL);
				g_free(outString);
				g_free(cnt_string);
			}

			setNotiImage(noti, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		}
		break;
	}
	case MSG_MGR_NOTI_TYPE_FAILED: {
		notiMsg = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, FAILED_TO_SEND_MESSAGE);
		setNotiText(noti, NOTIFICATION_TEXT_TYPE_TITLE, notiMsg, NULL);
		setNotiImage(noti, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SMS_SENDING_FAILED_ICON_PATH);
		break;
	}
	default:
		MSG_MGR_DEBUG("No matching type for MsgNotiType%d]", noti_type);
		goto _END_OF_INSTANT_NOTI;
		break;
	}

	if (notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_TICKER) != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("Fail to notification_set_display_applist");

	if (notification_post(noti) != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("Fail to notification_post");

_END_OF_INSTANT_NOTI:

	if (notification_delete(noti) != NOTIFICATION_ERROR_NONE)
		MSG_MGR_DEBUG("Fail to notification_delete");
	if (notiMsg) {
		free(notiMsg);
		notiMsg = NULL;
	}

	if (noti) {
		if (notification_free(noti) != NOTIFICATION_ERROR_NONE)
			MSG_MGR_DEBUG("Fail to notification_free");
		noti = NULL;
	}

	MSG_MGR_END();
	return 0;
}


bool MsgMgrCheckNotificationSettingEnable()
{
	bool msg_noti_enabled = false;
	notification_system_setting_h system_setting = NULL;
	notification_setting_h setting = NULL;

	int err = NOTIFICATION_ERROR_NONE;

	err = notification_setting_get_setting_by_package_name(MSG_DEFAULT_APP_ID, &setting);

	if (err != NOTIFICATION_ERROR_NONE || setting == NULL) {
		MSG_MGR_ERR("getting setting handle for [%s] is failed. err = %d", MSG_DEFAULT_APP_ID, err);
	} else {
		msg_noti_enabled = true;

		bool allow_to_notify = false;
		err = notification_setting_get_allow_to_notify(setting, &allow_to_notify);

		if (err != NOTIFICATION_ERROR_NONE) {
			MSG_MGR_ERR("getting do not disturb setting is failed. err = %d", err);
			goto EXIT;
		}

		if (allow_to_notify) {
			MSG_MGR_DEBUG("message notification is allowed");

			/* check do not disturb mode */
			err = notification_system_setting_load_system_setting(&system_setting);

			if (err != NOTIFICATION_ERROR_NONE || system_setting == NULL) {
				MSG_MGR_ERR("getting system setting is failed. err = %d", err);
				goto EXIT;
			}

			bool do_not_disturb_mode = false;
			err = notification_system_setting_get_do_not_disturb(system_setting, &do_not_disturb_mode);

			if (err != NOTIFICATION_ERROR_NONE) {
				MSG_MGR_ERR("getting do not disturb setting is failed. err = %d", err);
				goto EXIT;
			}

			if (do_not_disturb_mode) {
				bool is_msg_excepted = false;
				err = notification_setting_get_do_not_disturb_except(setting, &is_msg_excepted);
				if (err != NOTIFICATION_ERROR_NONE) {
					MSG_MGR_ERR("getting do not disturb except status for [%s] is failed. err = %d", MSG_DEFAULT_APP_ID, err);
					msg_noti_enabled = false;
				} else {
					MSG_MGR_INFO("do not disturb mode status for [%s] : %d", MSG_DEFAULT_APP_ID, is_msg_excepted);
					msg_noti_enabled = (is_msg_excepted) ? true : false;
				}
			} else {
				MSG_MGR_DEBUG("do not disturb mode is off");
			}
		} else {
			MSG_MGR_INFO("message notification is not allowed");
			msg_noti_enabled = false;
		}
	}

EXIT:
	if (system_setting)
		notification_system_setting_free_system_setting(system_setting);

	if (setting)
		notification_setting_free_notification(setting);

	return msg_noti_enabled;
}


int MsgMgrInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId)
{
	MSG_MGR_DEBUG("pTickerMsg=[%s], pLocaleTickerMsg=[%s]", pTickerMsg, pLocaleTickerMsg);
	MSG_MGR_DEBUG("play feedback=[%d], msgId=[%d]", bPlayFeedback, msgId);

	MsgMgrChangePmState();

	char *notiMsg = NULL;
	msg_mgr_active_notification_type_t active_type = MSG_MGR_ACTIVE_NOTI_TYPE_NONE;
	int err = 0;

	notiMsg = get_translate_text(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, pLocaleTickerMsg);
	MSG_MGR_DEBUG("notiMsg %s", notiMsg);

	if (g_strcmp0(pLocaleTickerMsg, SMS_MESSAGE_SENDING_FAIL) != 0 &&
		g_strcmp0(pLocaleTickerMsg, SENDING_MULTIMEDIA_MESSAGE_FAILED) != 0 &&
		g_strcmp0(pLocaleTickerMsg, MESSAGE_RETRIEVED) != 0) {
		if (g_strcmp0(pLocaleTickerMsg, notiMsg) == 0) {
			notification_status_message_post(pTickerMsg);
		} else {
			notification_status_message_post(notiMsg);
		}
	} else {
		/* Show ticker popup for sending failed msg. */
		active_type = MSG_MGR_ACTIVE_NOTI_TYPE_INSTANT;
	}

	if (notiMsg) {
		free(notiMsg);
		notiMsg = NULL;
	}

	if (bPlayFeedback) {
		if (msgId > 0 &&
			(g_strcmp0(pLocaleTickerMsg, SMS_MESSAGE_SENDING_FAIL) == 0 || g_strcmp0(pLocaleTickerMsg, SENDING_MULTIMEDIA_MESSAGE_FAILED) == 0)) {
			err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_FAILED, true, active_type);
			if (err != 0) {
				MSG_MGR_DEBUG("MsgRefreshFailedNoti err=[%d]", err);
			}
		} else if (g_strcmp0(pLocaleTickerMsg, SMS_MESSAGE_SIM_MESSAGE_FULL) == 0) {
			err = MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_SIM_FULL, true, MSG_MGR_ACTIVE_NOTI_TYPE_NONE);
			if (err != 0) {
				MSG_MGR_DEBUG("MsgRefreshSimFullNoti err=[%d]", err);
			}
		} else {
			MsgMgrSoundPlayStart(NULL, MSG_MGR_SOUND_PLAY_DEFAULT);
		}
	}

	return err;
}


int MsgMgrInsertBadge(unsigned int unreadMsgCnt)
{
	MSG_MGR_DEBUG("Start to set badge to [%d].", unreadMsgCnt);

	int err = BADGE_ERROR_NONE;
	bool exist = false;

	err = badge_is_existing(MSG_DEFAULT_APP_ID, &exist);

	if (err != BADGE_ERROR_NONE) {
		MSG_MGR_ERR("Fail to badge_is_existing : %d", err);
		return -1;
	}

	if (!exist) {
		/* create badge */
		err = badge_add(MSG_DEFAULT_APP_ID);
		if (err != BADGE_ERROR_NONE) {
			MSG_MGR_ERR("Fail to badge_add : %d", err);
			return -1;
		}
	}

	err = badge_set_count(MSG_DEFAULT_APP_ID, unreadMsgCnt);

	if (err != BADGE_ERROR_NONE) {
		MSG_MGR_ERR("Fail to badge_set_count : %d", err);
		return -1;
	}

	return 0;
}


void MsgMgrNotiSoundRepeatAlarmCB(int alarmId)
{
	MSG_MGR_BEGIN();

	MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_NORMAL, true, MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE);

#ifndef MSG_NOTI_INTEGRATION
	MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_SIM, true, MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE);
	MsgMgrRefreshNotification(MSG_MGR_NOTI_TYPE_CB, true, MSG_MGR_ACTIVE_NOTI_TYPE_ACTIVE);
#endif

	MSG_MGR_END();
	return;
}


void MsgMgrSoundCreateRepeatAlarm(int RepeatTime)
{
	MSG_MGR_BEGIN();

	int tmpAlarmId = 0;
	time_t tmp_time;
	struct tm repeat_tm;

	time(&tmp_time);

	tmp_time += (RepeatTime*60);
	tzset();
	localtime_r(&tmp_time, &repeat_tm);

	if (MsgMgrAlarmRegistration(&repeat_tm, MsgMgrNotiSoundRepeatAlarmCB, &tmpAlarmId) != 0) {
		MSG_MGR_DEBUG("MsgAlarmRegistration fail.");
		return;
	}

	g_alarmId = tmpAlarmId;
	MSG_MGR_DEBUG("Set alarmId to [%d]", g_alarmId);

	MSG_MGR_END();

	return;
}


void MsgMgrSoundSetRepeatAlarm()
{
	int nRepeatValue = 0;
	long nRepeatTime = 0;

	if (vconf_get_int(MSG_ALERT_REP_TYPE, &nRepeatValue) != 0) {
		MSG_MGR_INFO("vconf_get_int() is failed");
	}

	switch (nRepeatValue) {
	case MSG_ALERT_TONE_ONCE:
		nRepeatTime = 0;
		break;
	case MSG_ALERT_TONE_2MINS:
		nRepeatTime = 2;
		break;
	case MSG_ALERT_TONE_5MINS:
		nRepeatTime = 5;
		break;
	case MSG_ALERT_TONE_10MINS:
		nRepeatTime = 10;
		break;
	default:
		MSG_MGR_DEBUG("Invalid Repetition time");
		break;
	}

	MSG_MGR_DEBUG("nRepeatTime = %d", nRepeatTime);

	if (nRepeatTime > 0) {
		if (g_alarmId > 0) {
			if (MsgMgrAlarmRemove(g_alarmId) != 0) {
				MSG_MGR_FATAL("MsgAlarmRemove fail.");
			}
			g_alarmId = 0;
		}
		MsgMgrSoundCreateRepeatAlarm(nRepeatTime);
	}

	return;
}


char *get_translate_text(const char *pkg_name, const char *locale_dir, const char *text)
{
	char *notiMsg = NULL;
	char *lang = NULL;

	lang = vconf_get_str(VCONFKEY_LANGSET);

	setlocale(LC_MESSAGES, lang);

	bindtextdomain(pkg_name, locale_dir);

	notiMsg = dgettext(pkg_name, text);

	if (lang) {
		free(lang);
		lang = NULL;
	}

	return g_strdup(notiMsg);
}

