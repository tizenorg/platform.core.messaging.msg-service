/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
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


#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgDrmWrapper.h"
#include "MsgContact.h"
#include "MsgStorageTypes.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"
#include "MsgUtilStorage.h"
#include "MsgAlarm.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgSoundPlayer.h"
#include "MsgDevicedWrapper.h"
#include <libintl.h>
#include <locale.h>
#include <stdarg.h>

extern "C"
{
#include <bundle_internal.h>
#include <app_control_internal.h>
#ifndef MSG_WEARABLE_PROFILE
#include <notification_list.h>
#include <notification_text_domain.h>
#include <notification_internal.h>
#include <notification_status.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#include <feedback.h>
#include <badge_internal.h>
#endif /* MSG_WEARABLE_PROFILE */
}

#ifndef MSG_WEARABLE_PROFILE

/*======================================================================================*/
/*								VARIABLES AND STRUCTURES								*/
/*======================================================================================*/

int g_alarmId = 0;
bool bFeedbackInit;

GList *msg_report_notification_list;


typedef struct _msg_noti_info_s
{
	msg_notification_type_t			type;
	int			id;
	int			layout;
	int			count;
	int			senderCount;
	time_t		time;
	char		sender[MSG_NOTI_TEXT_LEN_S];
	char		text[MSG_NOTI_TEXT_LEN];
	char		number[MSG_NOTI_TEXT_LEN_S];
	char 		imagePath[MAX_IMAGE_PATH_LEN + 1];		/**< Indicates the image path of contact. */
	int			applist;
	app_control_h		svc_h;
	app_control_h		active_noti_svc_h[MSG_ACTIVE_NOTI_BUTTON_NUM];
	msg_message_id_t	msg_id;
	unsigned char		extra_data;
	int 		sim_idx;
	int			active_noti_button_num;
	int 		active_media_cnt;
	int 		active_media_size;
	MSG_SUB_TYPE_T	active_subtype;		/**< to distinguish cb, push message */
	char		active_sender[MSG_NOTI_TEXT_LEN_S];
	char		active_subject[MSG_NOTI_TEXT_LEN_S];
	char		active_text[MSG_NOTI_TEXT_LEN];
}MSG_NOTI_INFO_S;

typedef struct _report_notification_s
{
	int priv_id;
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
}report_notification_s;

/*======================================================================================*/
/*									FUNCTION DEFINE										*/
/*======================================================================================*/

notification_h getHandle(int *noti_id);

int getPrivId(msg_notification_type_t noti_type, int sim_idx);
void updatePrivId(msg_notification_type_t noti_type, int noti_id, int sim_idx);

void createInfoData(MSG_NOTI_INFO_S *noti_info, MSG_MESSAGE_INFO_S *msg_info); /* For addNoti() */
void createInfoData(MSG_NOTI_INFO_S *noti_info, msg_active_notification_type_t active_noti); /* For refreshNoti() */
void createActiveInfoData(MSG_NOTI_INFO_S *noti_info, MSG_MESSAGE_INFO_S *msg_info);
void clearInfoData(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);

msg_error_t getLatestMsgInfo(MSG_NOTI_INFO_S *noti_info, bool isForInstantMessage);

void setProperty(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setTextDomain(notification_h noti_h, msg_notification_type_t noti_type);
void setText(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setIcon(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setPkgName(notification_h noti_h, msg_notification_type_t noti_type);
void setSoundAndVibration(notification_h noti_h, char *addressVal, bool bVoiceMail);
void setActiveNotification(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setActiveProperty(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setActiveText(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);
void setActiveIcon(notification_h noti_h, MSG_NOTI_INFO_S *noti_info);

void setNotification(notification_h noti_h, MSG_NOTI_INFO_S *noti_info, bool bFeedback);

void MsgDeleteNotiCb(void *data);
void MsgRefreshNotiCb(void *data);

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
void MsgNotiSoundRepeatAlarmCB(int alarmId);
void MsgSoundCreateRepeatAlarm(int RepeatTime);
void MsgSoundSetRepeatAlarm();

void sendMsgReplyPopup(MSG_NOTI_INFO_S *noti_info);

#endif /* MSG_WEARABLE_PROFILE */

/*======================================================================================*/
/*								FUNCTION IMPLEMENTATION									*/
/*======================================================================================*/


msg_error_t MsgInsertNotification(MSG_MESSAGE_INFO_S *msg_info)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE

	msg_notification_type_t noti_type;

	switch (msg_info->msgType.subType) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NORMAL_SMS:
	case MSG_CB_SMS: {
		if (msg_info->msgType.classType == MSG_CLASS_0) {
			noti_type = MSG_NOTI_TYPE_CLASS0;
		} else {
			noti_type = MSG_NOTI_TYPE_NORMAL;
		}
		break;
	}
#else
	case MSG_NORMAL_SMS: {
		if (msg_info->msgType.classType == MSG_CLASS_0) {
			noti_type = MSG_NOTI_TYPE_CLASS0;
		} else if (msg_info->msgType.classType == MSG_CLASS_2) {
			noti_type = MSG_NOTI_TYPE_SIM;
		} else {
			noti_type = MSG_NOTI_TYPE_NORMAL;
		}
		break;
	}
	case MSG_CB_SMS:
		noti_type = MSG_NOTI_TYPE_CB;
		break;
#endif
	case MSG_MWI_FAX_SMS:
	case MSG_MWI_EMAIL_SMS:
	case MSG_MWI_OTHER_SMS:
		noti_type = MSG_NOTI_TYPE_MWI;
		break;
	case MSG_MWI_VOICE_SMS:
		noti_type = MSG_NOTI_TYPE_VOICE_1;
		break;
	case MSG_MWI_VOICE2_SMS:
		noti_type = MSG_NOTI_TYPE_VOICE_2;
		break;
	case MSG_STATUS_REPORT_SMS:
		noti_type = MSG_NOTI_TYPE_SMS_DELIVERY_REPORT;
		break;
	case MSG_DELIVERYIND_MMS:
		noti_type = MSG_NOTI_TYPE_MMS_DELIVERY_REPORT;
		break;
	case MSG_READORGIND_MMS:
		noti_type = MSG_NOTI_TYPE_MMS_READ_REPORT;
		break;
	default:
#ifdef MSG_NOTI_INTEGRATION
		noti_type = MSG_NOTI_TYPE_NORMAL;
#else
		if (msg_info->msgType.classType == MSG_CLASS_2) {
			noti_type = MSG_NOTI_TYPE_SIM;
		} else {
			noti_type = MSG_NOTI_TYPE_NORMAL;
		}
#endif
		break;
	}

	MSG_DEBUG("Notification type = [%d]", noti_type);

	switch (noti_type) {
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_CB:
		err = MsgRefreshNotification(noti_type, true, MSG_ACTIVE_NOTI_TYPE_ACTIVE);
		break;
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT:
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT:
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
		err = MsgAddReportNotification(noti_type, msg_info);
		break;
	case MSG_NOTI_TYPE_VOICE_1:
	case MSG_NOTI_TYPE_VOICE_2:
	case MSG_NOTI_TYPE_MWI:
	case MSG_NOTI_TYPE_CLASS0:
		err = MsgAddNotification(noti_type, msg_info);
		break;
	default:
		MSG_DEBUG("No matching type [%d]");
		break;
	}

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();

	return err;

}

msg_error_t MsgInsertOnlyActiveNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
	MSG_BEGIN();

	msg_error_t msg_err = MSG_SUCCESS;
#ifndef MSG_WEARABLE_PROFILE
	notification_h noti_h = NULL;

	MSG_NOTI_INFO_S noti_info = {0,};

	noti_info.type = noti_type;
	noti_info.active_noti_button_num = 1;

	createActiveInfoData(&noti_info, msg_info);

	noti_h = notification_create(NOTIFICATION_TYPE_NOTI);

	setActiveNotification(noti_h, &noti_info);

	clearInfoData(noti_h, &noti_info);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
	return msg_err;
}


msg_error_t MsgDeleteReportNotification(const char *addr)
{
	MSG_BEGIN();

	msg_error_t msg_err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE

	notification_h noti_h = NULL;
	bool bNotification = true;

	MSG_NOTI_INFO_S noti_info;
	memset(&noti_info, 0x00, sizeof(MSG_NOTI_INFO_S));

	if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bNotification) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingGetBool is failed.");
	}

	if (bNotification == false) {
		MSG_DEBUG("Msg Alert notification is off.");
		return msg_err;
	}

	char normalAddr[MAX_ADDRESS_VAL_LEN+1];
	unsigned int list_length = g_list_length(msg_report_notification_list);
	bool isDelete = false;

	MSG_DEBUG("list length [%d]", list_length);

	if (list_length > 0) {
		GList *iter = g_list_first(msg_report_notification_list);

		while (iter != NULL) {
			isDelete = false;
			report_notification_s *info = (report_notification_s*)(iter->data);
			if (info == NULL) {
				MSG_DEBUG("info is NULL!");
				return MSG_ERR_UNKNOWN;
			}

			MSG_SEC_DEBUG("list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);

			noti_h = notification_load(NULL, info->priv_id);
			if (noti_h == NULL) {
				MSG_DEBUG("notification with priv_id [%d] is NULL", info->priv_id);
				isDelete = true;
			} else {
				memset(normalAddr, 0x00, sizeof(normalAddr));
				MsgConvertNumber(info->addressVal, normalAddr, sizeof(normalAddr));
				MSG_SEC_DEBUG("normalized number = %s", normalAddr);

				if (g_str_has_suffix(addr, normalAddr)) {
					if (notification_delete(noti_h) == NOTIFICATION_ERROR_NONE) {
						MSG_SEC_DEBUG("delete report notification address [%s]", info->addressVal);
						isDelete = true;
					} else {
						MSG_DEBUG("delete notification failed");
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

#endif /* MSG_WEARABLE_PROFILE */

	MSG_END();

	return msg_err;
}

msg_error_t MsgAddReportNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
	msg_error_t msg_err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE
	notification_h noti_h = NULL;

	MSG_NOTI_INFO_S noti_info;
	memset(&noti_info, 0x00, sizeof(MSG_NOTI_INFO_S));

	report_notification_s *info = new report_notification_s;
	memset(info, 0x00, sizeof(report_notification_s));

	noti_info.type = noti_type;

	createInfoData(&noti_info, msg_info);

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_DEBUG("Notification handle is NULL");
		msg_err = MSG_ERR_NULL_POINTER;
		goto __END_OF_REFRESH_NOTI;
	}

	setNotification(noti_h, &noti_info, true);

	info->priv_id = noti_info.id;
	snprintf(info->addressVal, sizeof(info->addressVal), "%s", msg_info->addressList->addressVal);
	msg_report_notification_list = g_list_append(msg_report_notification_list, (void *)info);
	MSG_SEC_DEBUG("appended list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);

__END_OF_REFRESH_NOTI :
	clearInfoData(noti_h, &noti_info);

#endif /* MSG_WEARABLE_PROFILE */
	return msg_err;
}

msg_error_t MsgRefreshNotification(msg_notification_type_t noti_type, bool bFeedback, msg_active_notification_type_t active_type)
{

	msg_error_t msg_err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE
	notification_h noti_h = NULL;
	bool bNotification = true;
/*	bool bReplyPopup = false; */

	MSG_NOTI_INFO_S noti_info;
	memset(&noti_info, 0x00, sizeof(MSG_NOTI_INFO_S));

	noti_info.type = noti_type;

	noti_info.id = getPrivId(noti_info.type, -1);

	msg_err = getLatestMsgInfo(&noti_info, false);

	if (msg_err != MSG_SUCCESS) {
		MSG_DEBUG("getLatestMsgInfo() err = [%d]", msg_err);
		goto __END_OF_REFRESH_NOTI;
	}

	if (active_type == MSG_ACTIVE_NOTI_TYPE_INSTANT) {
		msg_err = MsgInsertInstantMessage(noti_type);

		if (msg_err != MSG_SUCCESS) {
			MSG_DEBUG(" MsgInsertInstantMessage() err = [%d]", msg_err);
			goto __END_OF_REFRESH_NOTI;
		}
	}

	if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bNotification) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingGetBool is failed.");
	}

	if (bNotification == false) {
		MSG_DEBUG("Msg Alert notification is off.");
		goto __END_OF_REFRESH_NOTI;
	}

	createInfoData(&noti_info, active_type);

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_DEBUG("Notification handle is NULL");
		msg_err = MSG_ERR_NULL_POINTER;
		goto __END_OF_REFRESH_NOTI;
	}

	setNotification(noti_h, &noti_info, bFeedback);

#if 0
	if (MsgSettingGetBool(VCONFKEY_MESSAGE_POPUP_DISPLAY_ENABLE , &bReplyPopup) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingGetBool is failed.");
	}

	if (bReplyPopup == true && bFeedback == true && noti_info.type == MSG_NOTI_TYPE_NORMAL) {
		MSG_DEBUG("Msg reply popup is on.");
		sendMsgReplyPopup(&noti_info);
	}
#endif

__END_OF_REFRESH_NOTI :
	clearInfoData(noti_h, &noti_info);

#endif /* MSG_WEARABLE_PROFILE */
	return msg_err;
}


msg_error_t MsgAddNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
	msg_error_t msg_err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE
	notification_h noti_h = NULL;

	MSG_NOTI_INFO_S noti_info = {0,};

	noti_info.type = noti_type;

	createInfoData(&noti_info, msg_info);

	/* check mwi or voicemail count is 0 then skip add notification */
	if (noti_info.count == 0) {
		MSG_DEBUG("Notification count is 0");
		msg_err = MSG_ERR_INVALID_MESSAGE;
		goto __END_OF_ADD_NOTI;
	}

	noti_h = getHandle(&noti_info.id);

	if (noti_h == NULL) {
		MSG_DEBUG("Notification handle is NULL");
		msg_err = MSG_ERR_NULL_POINTER;
		goto __END_OF_ADD_NOTI;
	}

	setNotification(noti_h, &noti_info, true);

__END_OF_ADD_NOTI :
	clearInfoData(noti_h, &noti_info);

#endif /* MSG_WEARABLE_PROFILE */
	return msg_err;
}


void MsgRefreshAllNotification(bool bWithSimNoti, bool bFeedback, msg_active_notification_type_t active_type)
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE
	msg_error_t err = MSG_SUCCESS;

/*	MsgDeleteNotification(MSG_NOTI_TYPE_SIM); */

#ifdef MSG_NOTI_INTEGRATION
	err = MsgRefreshNotification(MSG_NOTI_TYPE_NORMAL, bFeedback, active_type);
	if (err != MSG_SUCCESS)
		MSG_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_NOTI_TYPE_NORMAL, err);
#else
	err = MsgRefreshNotification(MSG_NOTI_TYPE_NORMAL, bFeedback, active_type);
	if (err != MSG_SUCCESS)
		MSG_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_NOTI_TYPE_NORMAL, err);

	err = MsgRefreshNotification(MSG_NOTI_TYPE_CB, bFeedback, active_type);
	if (err != MSG_SUCCESS)
		MSG_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_NOTI_TYPE_CB, err);

	if (bWithSimNoti) {
		err = MsgRefreshNotification(MSG_NOTI_TYPE_SIM, bFeedback, active_type);
		if (err != MSG_SUCCESS)
			MSG_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_NOTI_TYPE_SIM, err);
	}
#endif

	err = MsgRefreshNotification(MSG_NOTI_TYPE_FAILED, bFeedback, active_type);
	if (err != MSG_SUCCESS)
		MSG_DEBUG("refreshNoti is failed, [type=%d, err=%d]", MSG_NOTI_TYPE_FAILED, err);

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


void MsgDeleteNotification(msg_notification_type_t noti_type, int simIndex)
{
#ifndef MSG_WEARABLE_PROFILE
	int noti_err = NOTIFICATION_ERROR_NONE;

	if (noti_type == MSG_NOTI_TYPE_ALL) {

		noti_err = notification_delete_all_by_type(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI);

	} else if (noti_type == MSG_NOTI_TYPE_VOICE_1 || noti_type == MSG_NOTI_TYPE_VOICE_2 || noti_type == MSG_NOTI_TYPE_SIM) {
		int notiId = 0;

		notiId = getPrivId(noti_type, simIndex);
		MSG_DEBUG("deleted notification ID = [%d] Type = [%d]", notiId, noti_type);

		if (notiId > 0)
			noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, notiId);

	} else {
		MSG_DEBUG("No matching type [%d]", noti_type);
	}

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_delete_all_by_type noti_err [%d]", noti_err);
	}

	updatePrivId(noti_type, 0, simIndex);
#endif /* MSG_WEARABLE_PROFILE */
}

bool MsgCheckNotificationSettingEnable(void)
{
	bool msg_noti_enabled = false;
#ifndef MSG_WEARABLE_PROFILE
	notification_system_setting_h system_setting = NULL;
	notification_setting_h setting = NULL;

	int err = NOTIFICATION_ERROR_NONE;

	err = notification_setting_get_setting_by_package_name(MSG_DEFAULT_APP_ID, &setting);

	if (err != NOTIFICATION_ERROR_NONE || setting == NULL) {
		MSG_ERR("getting setting handle for [%s] is failed. err = %d", MSG_DEFAULT_APP_ID, err);
	} else {
		msg_noti_enabled = true;

		bool allow_to_notify = false;
		err = notification_setting_get_allow_to_notify(setting, &allow_to_notify);

		if (err != NOTIFICATION_ERROR_NONE) {
			MSG_ERR("getting do not disturb setting is failed. err = %d", err);
			goto EXIT;
		}

		if (allow_to_notify) {
			MSG_DEBUG("message notification is allowed");

			// check do not disturb mode
			err = notification_system_setting_load_system_setting(&system_setting);

			if (err != NOTIFICATION_ERROR_NONE || system_setting == NULL) {
				MSG_ERR("getting system setting is failed. err = %d", err);
				goto EXIT;
			}

			bool do_not_disturb_mode = false;
			err = notification_system_setting_get_do_not_disturb(system_setting, &do_not_disturb_mode);

			if (err != NOTIFICATION_ERROR_NONE) {
				MSG_ERR("getting do not disturb setting is failed. err = %d", err);
				goto EXIT;
			}

			if (do_not_disturb_mode) {
				bool is_msg_excepted = false;
				err = notification_setting_get_do_not_disturb_except(setting, &is_msg_excepted);
				if (err != NOTIFICATION_ERROR_NONE) {
					MSG_ERR("getting do not disturb except status for [%s] is failed. err = %d", MSG_DEFAULT_APP_ID, err);
					msg_noti_enabled = false;
				} else {
					MSG_INFO("do not disturb mode status for [%s] : %d", MSG_DEFAULT_APP_ID, is_msg_excepted);
					msg_noti_enabled = (is_msg_excepted) ? true : false;
				}
			} else {
				MSG_DEBUG("do not disturb mode is off");
			}
		} else {
			MSG_INFO("message notification is not allowed");
			msg_noti_enabled = false;
		}
	}

EXIT:
	if (system_setting)
		notification_system_setting_free_system_setting(system_setting);

	if (setting)
		notification_setting_free_notification(setting);

#endif /* MSG_WEARABLE_PROFILE */
	return msg_noti_enabled;
}

bool isExistAddressInReportTable(const char *addr)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	char normalAddr[MAX_ADDRESS_VAL_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();
	int rowCnt = 0;

	memset(normalAddr, 0x00, sizeof(normalAddr));
	MsgConvertNumber(addr, normalAddr, sizeof(normalAddr));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s'", MSGFW_SMS_REPORT_TABLE_NAME, normalAddr);
	if (dbHandle->getTable(sqlQuery, &rowCnt, NULL) == MSG_SUCCESS) {
		dbHandle->freeTable();
		return true;
	}

	dbHandle->freeTable();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s'", MSGFW_REPORT_TABLE_NAME, normalAddr);
	if (dbHandle->getTable(sqlQuery, &rowCnt, NULL) == MSG_SUCCESS) {
		dbHandle->freeTable();
		return true;
	}

	dbHandle->freeTable();

	return false;
}


void MsgInitReportNotiList()
{
	MSG_BEGIN();

#ifndef MSG_WEARABLE_PROFILE
	msg_report_notification_list = NULL;

	notification_h noti = NULL;
	notification_list_h noti_list = NULL;
	notification_list_h head_noti_list = NULL;
	int noti_err = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;

	noti_err = notification_get_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("notification_get_list() is failed!!");
		return;
	}

	head_noti_list = noti_list;

	while (noti_list != NULL) {
		noti = notification_list_get_data(noti_list);

		char tempAddr[MAX_ADDRESS_VAL_LEN+1];
		memset(tempAddr, 0x00, sizeof(tempAddr));

		noti_err = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &b);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("notification_get_excute_option() failed!!");
			break;
		}

		const char *bundle_addr = bundle_get_val(b, "address");

		if (bundle_addr != NULL) {
			if (isExistAddressInReportTable(bundle_addr)) {
				report_notification_s *info = new report_notification_s;
				memset(info, 0x00, sizeof(report_notification_s));

				notification_get_id(noti, NULL, &(info->priv_id));
				snprintf(info->addressVal, sizeof(info->addressVal), "%s", bundle_addr);

				msg_report_notification_list = g_list_append(msg_report_notification_list, (void *)info);
				MSG_SEC_DEBUG("appended list data = [priv_id = %d address = %s]", info->priv_id, info->addressVal);
			}
		}

		noti_list = notification_list_get_next(noti_list);
	}

	if (head_noti_list)
		notification_free_list(head_noti_list);

#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


msg_error_t MsgInitNoti()
{
#ifndef MSG_WEARABLE_PROFILE
	bool bNotiSvcReady = false;

	bNotiSvcReady = notification_is_service_ready();

	if (bNotiSvcReady == true) {
		MSG_DEBUG("Notification server is available");
#ifndef MSG_NOTI_INTEGRATION
		MsgDeleteNotification(MSG_NOTI_TYPE_SIM, -1);
#endif
		MsgRefreshAllNotification(false, true, MSG_ACTIVE_NOTI_TYPE_INSTANT);		/* On Booting */
		MsgInitReportNotiList();
	} else {
		MSG_DEBUG("Notification server is not available. Init is defered");
#ifndef MSG_NOTI_INTEGRATION
		MSG_NOTI_INFO_S *delNotiInfo = (MSG_NOTI_INFO_S *)calloc(1, sizeof(MSG_NOTI_INFO_S));
		if (delNotiInfo) {
			delNotiInfo->type = MSG_NOTI_TYPE_SIM;
			delNotiInfo->sim_idx = -1;
		}
		notification_add_deferred_task(MsgDeleteNotiCb, (void *)delNotiInfo);
#endif
		notification_add_deferred_task(MsgRefreshNotiCb, (void *)NULL);
	}

#endif /* MSG_WEARABLE_PROFILE */
	return MSG_SUCCESS;
}


msg_error_t MsgDeleteNoti(msg_notification_type_t noti_type, int simIndex)
{
#ifndef MSG_WEARABLE_PROFILE
	bool bNotiSvcReady = false;

	MSG_NOTI_INFO_S *delNotiInfo = (MSG_NOTI_INFO_S *)calloc(1, sizeof(MSG_NOTI_INFO_S));

	if (delNotiInfo) {
		delNotiInfo->type = noti_type;
		delNotiInfo->sim_idx = simIndex;
	}

	bNotiSvcReady = notification_is_service_ready();

	if (bNotiSvcReady == true) {
		MSG_DEBUG("Notification server is available");
		MsgDeleteNotiCb((void *)delNotiInfo);
	} else {
		MSG_DEBUG("Notification server is not available. Delete is defered");
		notification_add_deferred_task(MsgDeleteNotiCb, (void *)delNotiInfo);
	}
#endif /* MSG_WEARABLE_PROFILE */
	return MSG_SUCCESS;
}


msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId)
{
#ifndef MSG_WEARABLE_PROFILE
	MSG_DEBUG("pTickerMsg=[%s], pLocaleTickerMsg=[%s]", pTickerMsg, pLocaleTickerMsg);
	MSG_DEBUG("play feedback=[%d], msgId=[%d]", bPlayFeedback, msgId);

	if (MsgCheckNotificationSettingEnable())
		MsgChangePmState();

	char *notiMsg = NULL;
	msg_active_notification_type_t active_type = MSG_ACTIVE_NOTI_TYPE_NONE;

	notiMsg = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, pLocaleTickerMsg);
	MSG_DEBUG("notiMsg %s", notiMsg);

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
		active_type = MSG_ACTIVE_NOTI_TYPE_INSTANT;
	}

	if (notiMsg) {
		free(notiMsg);
		notiMsg = NULL;
	}

	if (bPlayFeedback) {
		if (msgId > 0 &&
			(g_strcmp0(pLocaleTickerMsg, SMS_MESSAGE_SENDING_FAIL) == 0 || g_strcmp0(pLocaleTickerMsg, SENDING_MULTIMEDIA_MESSAGE_FAILED) == 0)) {
			msg_error_t err = MSG_SUCCESS;
			err = MsgRefreshNotification(MSG_NOTI_TYPE_FAILED, true, active_type);
			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgRefreshFailedNoti err=[%d]", err);
			}
		} else if (g_strcmp0(pLocaleTickerMsg, SMS_MESSAGE_SIM_MESSAGE_FULL) == 0) {
			msg_error_t err = MSG_SUCCESS;
			err = MsgRefreshNotification(MSG_NOTI_TYPE_SIM_FULL, true, MSG_ACTIVE_NOTI_TYPE_NONE);
			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgRefreshSimFullNoti err=[%d]", err);
			}
		} else {
			MsgSoundPlayer::instance()->MsgSoundPlayStart(NULL, MSG_SOUND_PLAY_DEFAULT);
		}
	}

#endif /* MSG_WEARABLE_PROFILE */
	return MSG_SUCCESS;
}


msg_error_t MsgInsertBadge(unsigned int unreadMsgCnt)
{
#ifndef MSG_WEARABLE_PROFILE
	MSG_DEBUG("Start to set badge to [%d].", unreadMsgCnt);

	int err = BADGE_ERROR_NONE;
	bool exist = false;

	err = badge_is_existing(MSG_DEFAULT_APP_ID, &exist);

	if (err != BADGE_ERROR_NONE) {
		MSG_ERR("Fail to badge_is_existing : %d", err);
		return MSG_ERR_UNKNOWN;
	}

	if (!exist) {
		/* create badge */
		err = badge_create(MSG_DEFAULT_APP_ID, "/usr/bin/msg-server");
		if (err != BADGE_ERROR_NONE) {
			MSG_ERR("Fail to badge_new : %d", err);
			return MSG_ERR_UNKNOWN;
		}
	}

	err = badge_set_count(MSG_DEFAULT_APP_ID, unreadMsgCnt);

	if (err != BADGE_ERROR_NONE) {
		MSG_ERR("Fail to badge_set_count : %d", err);
		return MSG_ERR_UNKNOWN;
	}

#endif /* MSG_WEARABLE_PROFILE */
	return MSG_SUCCESS;
}

#ifndef MSG_WEARABLE_PROFILE

void MsgRefreshNotiCb(void *data)
{
	MsgRefreshAllNotification(false, true, MSG_ACTIVE_NOTI_TYPE_INSTANT);
	MsgInitReportNotiList();

	if (data) {
		free(data);
		data = NULL;
	}

	return;
}


void MsgDeleteNotiCb(void *data)
{
	if (data) {
		MSG_NOTI_INFO_S *delNotiInfo = (MSG_NOTI_INFO_S *)data;

		MsgDeleteNotification(delNotiInfo->type, delNotiInfo->sim_idx);

		free(data);
		data = NULL;
	}

	return;
}


notification_h getHandle(int *noti_id)
{
	notification_h noti_h = NULL;

	if (*noti_id > 0) {
		MSG_DEBUG("Notification load");
		noti_h = notification_load(NULL, *noti_id);
		if (noti_h == NULL)
			MSG_DEBUG("notification_load is failed.");
	}

	if (noti_h == NULL) {
		MSG_DEBUG("Notification create");
		noti_h = notification_create(NOTIFICATION_TYPE_NOTI);
		if (noti_h == NULL) {
			MSG_DEBUG("notification_create is failed.");
			return NULL;
		}

		*noti_id = 0;
	}

	return noti_h;
}


int getPrivId(msg_notification_type_t noti_type, int sim_idx)
{
	MSG_BEGIN();

	int noti_id = 0;

	switch (noti_type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_CB:
		noti_id = MsgSettingGetInt(NOTIFICATION_PRIV_ID);
		break;
#else
	case MSG_NOTI_TYPE_NORMAL:
		noti_id = MsgSettingGetInt(NOTIFICATION_PRIV_ID);
		break;
	case MSG_NOTI_TYPE_SIM:
		noti_id = MsgSettingGetInt(SIM_MSG_NOTI_PRIV_ID);
		break;
	case MSG_NOTI_TYPE_CB:
		noti_id = MsgSettingGetInt(CB_NOTI_PRIV_ID);
		break;
#endif
	case MSG_NOTI_TYPE_FAILED:
		noti_id = MsgSettingGetInt(MSG_SENTFAIL_NOTI_ID);
		break;
	case MSG_NOTI_TYPE_VOICE_1: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		noti_id = MsgSettingGetInt(keyName);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_2, sim_idx);
		noti_id = MsgSettingGetInt(keyName);
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL:
		noti_id = MsgSettingGetInt(SIM_FULL_NOTI_PRIV_ID);
		break;
	default:
		MSG_DEBUG("No matching noti type [%d]", noti_type);
		break;
	}

	MSG_DEBUG("Get noti type = %d, id = %d, sim_idx:%d", noti_type, noti_id, sim_idx);

	MSG_END();

	return noti_id;
}


void updatePrivId(msg_notification_type_t noti_type, int noti_id, int sim_idx)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	MSG_DEBUG("Update noti type = %d, id = %d, sim_idx = %d", noti_type, noti_id, sim_idx);

	switch (noti_type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_CB:
		err = MsgSettingSetInt(NOTIFICATION_PRIV_ID, noti_id);
		break;
#else
	case MSG_NOTI_TYPE_NORMAL:
		err = MsgSettingSetInt(NOTIFICATION_PRIV_ID, noti_id);
		break;
	case MSG_NOTI_TYPE_SIM:
		err = MsgSettingSetInt(SIM_MSG_NOTI_PRIV_ID, noti_id);
		break;
	case MSG_NOTI_TYPE_CB:
		err = MsgSettingSetInt(CB_NOTI_PRIV_ID, noti_id);
		break;
#endif
	case MSG_NOTI_TYPE_FAILED:
		err = MsgSettingSetInt(MSG_SENTFAIL_NOTI_ID, noti_id);
		break;
	case MSG_NOTI_TYPE_VOICE_1: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		err = MsgSettingSetInt(keyName, noti_id);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_2, sim_idx);
		err = MsgSettingSetInt(keyName, noti_id);
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL:
		err = MsgSettingSetInt(SIM_FULL_NOTI_PRIV_ID, noti_id);
		break;
	default:
		MSG_DEBUG("No matching type [%d]", noti_type);
		break;
	}

	if (err != MSG_SUCCESS)
		MSG_INFO("MsgSettingSetInt fail : noti type = %d, id = %d, sim_idx = %d", noti_type, noti_id, sim_idx);

	MSG_END();
}


void createInfoData(MSG_NOTI_INFO_S *noti_info, msg_active_notification_type_t active_noti)
{
	MSG_BEGIN();

	createServiceHandle(&noti_info->svc_h);

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
		addServiceExtraData(noti_info->svc_h, "http://tizen.org/appcontrol/data/notification", "new_message");

#if 0
		bool bReplyPopup = false;
		if (MsgSettingGetBool(VCONFKEY_MESSAGE_POPUP_DISPLAY_ENABLE , &bReplyPopup) != MSG_SUCCESS) {
			MSG_DEBUG("MsgSettingGetBool is failed.");
		}
#endif
		noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER;

		if (noti_info->active_noti_button_num == 0)
			noti_info->active_noti_button_num = 3;
		break;
	}
	case MSG_NOTI_TYPE_CB: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		if (active_noti == MSG_ACTIVE_NOTI_TYPE_INSTANT)
			noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
		else
			noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;

		noti_info->active_noti_button_num = 1;
		break;
	}
	case MSG_NOTI_TYPE_SIM: {
		if (noti_info->count > 1) {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_MULTIPLE;
		} else {
			noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		}

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "new_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		if (active_noti == MSG_ACTIVE_NOTI_TYPE_INSTANT)
			noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
		else
			noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;

		if (noti_info->active_noti_button_num == 0)
			noti_info->active_noti_button_num = 3;
		break;
	}
	case MSG_NOTI_TYPE_FAILED: {
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "type", "send_failed_msg");
		addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);

		noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER^NOTIFICATION_DISPLAY_APP_LOCK;
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL: {
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;

		setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
		addServiceExtraData(noti_info->svc_h, "sim_list_show", "sim_setting");

		noti_info->applist = NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR;
		break;
	}
	default:
		break;
	}

	if (active_noti != MSG_ACTIVE_NOTI_TYPE_ACTIVE)
		noti_info->active_noti_button_num = 0;

	MSG_END();
}


void createInfoData(MSG_NOTI_INFO_S *noti_info, MSG_MESSAGE_INFO_S *msg_info)
{
	MSG_BEGIN();

	if (msg_info) {
		noti_info->id = getPrivId(noti_info->type, msg_info->sim_idx);
		noti_info->msg_id = msg_info->msgId;
	} else {
		MSG_DEBUG("msg_info is NULL");
		return;
	}

	noti_info->sim_idx = msg_info->sim_idx;

	createServiceHandle(&noti_info->svc_h);
	char keyName[MAX_VCONFKEY_NAME_LEN];

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_VOICE_1:
	case MSG_NOTI_TYPE_VOICE_2: {
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, msg_info->sim_idx);
		noti_info->count = MsgSettingGetInt(keyName);
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, msg_info->sim_idx);
		char *voiceNumber = MsgSettingGetString(keyName);
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, msg_info->sim_idx);
		char *voiceAlphaId = MsgSettingGetString(keyName);
		char *dialNumber = NULL;

		MSG_SEC_DEBUG("Voice mail server - alpha id = [%s], default num = [%s]", voiceAlphaId, voiceNumber);

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

			char slot_id[5] = {0,};
			snprintf(slot_id, sizeof(slot_id), "%d", msg_info->sim_idx - 1);
			addServiceExtraData(noti_info->svc_h, "slot_id", slot_id);
		}

		MSG_FREE(voiceNumber);
		MSG_FREE(voiceAlphaId);
		break;
	}
	case MSG_NOTI_TYPE_MWI:
	case MSG_NOTI_TYPE_CLASS0: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

		if (msg_info->addressList[0].displayName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->addressList[0].addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", msg_info->addressList[0].displayName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s", msg_info->addressList[0].addressVal);

		snprintf(noti_info->text, sizeof(noti_info->text), "%s", msg_info->msgText);

		if (noti_info->type == MSG_NOTI_TYPE_MWI) {
			if (noti_info->svc_h) {
				setServiceOperation(noti_info->svc_h, APP_CONTROL_OPERATION_CALL);
				setServiceUri(noti_info->svc_h, MSG_TEL_URI_VOICEMAIL);

				char slot_id[5] = {0,};
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
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;
		noti_info->extra_data = msg_info->networkStatus;

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
		MSG_CONTACT_INFO_S contactInfo;
		memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

		if (MsgGetContactInfo(&(msg_info->addressList[0]), &contactInfo) != MSG_SUCCESS) {
			MSG_WARN("MsgGetContactInfo() fail.");
		}

		snprintf(msg_info->addressList[0].displayName, sizeof(msg_info->addressList[0].displayName), "%s", contactInfo.firstName);
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED

		if (msg_info->addressList[0].displayName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].displayName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s", msg_info->addressList[0].addressVal);

		if (noti_info->msg_id > 0) {
			setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
			addServiceExtraData(noti_info->svc_h, "type", "new_msg");
			addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
			addServiceExtraData(noti_info->svc_h, "address", msg_info->addressList[0].addressVal);
		}
		break;
	}
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT: {
		noti_info->count = 1;
		noti_info->layout = NOTIFICATION_LY_NOTI_EVENT_SINGLE;
		noti_info->time = msg_info->displayTime;

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
		MSG_CONTACT_INFO_S contactInfo;
		memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

		if (MsgGetContactInfo(&(msg_info->addressList[0]), &contactInfo) != MSG_SUCCESS) {
			MSG_WARN("MsgGetContactInfo() fail.");
		}

		snprintf(msg_info->addressList[0].displayName, sizeof(msg_info->addressList[0].displayName), "%s", contactInfo.firstName);
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED
				if (msg_info->addressList[0].displayName[0] == '\0')
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].addressVal);
				else
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].displayName);

		if (msg_info->addressList[0].displayName[0] == '\0')
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].addressVal);
		else
			snprintf(noti_info->sender, sizeof(noti_info->sender), "%s",msg_info->addressList[0].displayName);

		snprintf(noti_info->number, sizeof(noti_info->number), "%s",msg_info->addressList[0].addressVal);

		MsgDbHandler *dbhandler = getDbHandle();
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		int report_status_type;
		int report_status_value;

		if (noti_info->type == MSG_NOTI_TYPE_MMS_READ_REPORT) {
			report_status_type = MSG_REPORT_TYPE_READ;
		} else {
			report_status_type = MSG_REPORT_TYPE_DELIVERY;
		}

		char *normalNum = NULL;
		if (msg_info->addressList[0].addressVal[0] != '\0') {
			normalNum = msg_normalize_number(msg_info->addressList[0].addressVal);
		}

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
				"STATUS "
				"FROM %s "
				"WHERE MSG_ID=%d AND STATUS_TYPE=%d AND ADDRESS_VAL LIKE '%%%s';",
				MSGFW_REPORT_TABLE_NAME, msg_info->msgId, report_status_type, normalNum);

		MSG_DEBUG("sqlQuery = [%s]", sqlQuery);

		if (dbhandler->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("prepareQuery is failed");
			return;
		}

		if (dbhandler->stepQuery() == MSG_ERR_DB_ROW) {
			report_status_value = dbhandler->columnInt(0);
			MSG_DEBUG("report status [type = %d, value = %d]", report_status_type, report_status_value);
		} else {
			MSG_DEBUG("DB Query Result Fail");
			dbhandler->finalizeQuery();
			return;
		}

		dbhandler->finalizeQuery();

		if (noti_info->msg_id > 0) {
			setServiceAppId(noti_info->svc_h, MSG_DEFAULT_APP_ID);
			addServiceExtraData(noti_info->svc_h, "type", "new_msg");
			addServiceExtraData(noti_info->svc_h, "msgId", noti_info->msg_id);
			addServiceExtraData(noti_info->svc_h, "address", msg_info->addressList[0].addressVal);
		}

		noti_info->extra_data = (unsigned char)report_status_value;
		break;
	}
	default:
		MSG_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	noti_info->applist = NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK;
	MSG_END();
}


void createActiveInfoData(MSG_NOTI_INFO_S *noti_info, MSG_MESSAGE_INFO_S *msg_info)
{
	MSG_BEGIN();

	if (!msg_info) {
		MSG_DEBUG("msg_info is NULL");
		return;
	}

	noti_info->msg_id = msg_info->msgId;
	noti_info->sim_idx = msg_info->sim_idx;

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL: {
		char *senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, PUSH_MESSAGE);
		snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", senderStr);
		if (senderStr) {
			free(senderStr);
			senderStr = NULL;
		}
		break;
	}
	case MSG_NOTI_TYPE_CLASS0: {
		if (msg_info->addressList[0].displayName[0] == '\0')
			snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", msg_info->addressList[0].addressVal);
		else
			snprintf(noti_info->active_sender, MSG_NOTI_TEXT_LEN_S, "%s", msg_info->addressList[0].displayName);

		snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s", msg_info->msgText);
		break;
	}
	default:
		MSG_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_END();
}


void clearInfoData(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

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

	MSG_END();
}


void setProperty(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	/* set layout */
	noti_err = notification_set_layout(noti_h, (notification_ly_type_e)noti_info->layout);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	/* set led */
	noti_err = notification_set_led(noti_h, NOTIFICATION_LED_OP_ON, 0x00);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_led.");
	}

	/* set execute option */
	bundle *bundle_data = NULL;
	bundle *reply_msg = NULL;

	app_control_to_bundle(noti_info->svc_h, &bundle_data);

	if (bundle_data == NULL) {
		MSG_DEBUG("bundle is NULL");
	}

	/* set execute option and property */
	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL: {
		if (noti_info->count > 1) {
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, NULL, NULL, bundle_data);
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_RESPONDING, NULL, NULL, NULL);
		} else {
			if (noti_info->svc_h) { /* overwrite bundle key "type" */
				/* addServiceExtraData(noti_info->svc_h, "type", "reply"); */
				addServiceExtraData(noti_info->svc_h, "show_list", "list_show");

				app_control_to_bundle(noti_info->svc_h, &reply_msg);
			}
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, bundle_data);
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_RESPONDING, NULL, NULL, reply_msg);
		}

		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
		break;
	}
	case MSG_NOTI_TYPE_CB:
	case MSG_NOTI_TYPE_SIM: {
		if (noti_info->count > 1) {
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, NULL, NULL, bundle_data);
		} else {
			notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, bundle_data);
		}

		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE|NOTIFICATION_PROP_VOLATILE_DISPLAY);
		break;
	}
	case MSG_NOTI_TYPE_FAILED: {
		notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, bundle_data);
		notification_set_property(noti_h, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL: {
		notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, bundle_data);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_1:
	case MSG_NOTI_TYPE_VOICE_2:
	case MSG_NOTI_TYPE_MWI:
	case MSG_NOTI_TYPE_CLASS0:
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT:
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT: {
		notification_set_execute_option(noti_h, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, bundle_data);
		break;
	}
	default:
		MSG_DEBUG("No matching type for notification_set_execute_option() [%d]", noti_info->type);
		break;
	}

	/* set applist */
	noti_err = notification_set_display_applist(noti_h, noti_info->applist);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_display_applist");
	}


	MSG_END();
}


void setTextDomain(notification_h noti_h, msg_notification_type_t noti_type)
{
	MSG_BEGIN();

	setNotiTextDomain(noti_h, MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR);
	MSG_END();
}


void setText(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	char unreadMsgCntStr[10] = {0,};
	bool bPreview;

	if (MsgSettingGetBool(MSG_SETTING_PREVIEW, &bPreview) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingGetBool is failed.");
	}

	/* set title and content */
	switch (noti_info->type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_CB:
	case MSG_NOTI_TYPE_SIM: {
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
	case MSG_NOTI_TYPE_NORMAL: {
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
	case MSG_NOTI_TYPE_CB: {
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
	case MSG_NOTI_TYPE_SIM: {
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
	case MSG_NOTI_TYPE_FAILED: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Message", MSG_MESSAGE);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, "Failed to send message.", FAILED_TO_SEND_MESSAGE);
		if (noti_info->count > 1) {
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
		}
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_1:
	case MSG_NOTI_TYPE_VOICE_2: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "Voicemail", VOICE_MAIL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);

		if (noti_info->count == 1) {
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, "1", NULL);
		} else if (noti_info->count > 1) {
			snprintf(unreadMsgCntStr, sizeof(unreadMsgCntStr), "%d", noti_info->count);
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL);
		} else {
			MSG_DEBUG("Invalid notification count, [cnt = %d]", noti_info->count);
		}
		break;
	}
	case MSG_NOTI_TYPE_MWI: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "MWI Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_NOTI_TYPE_CLASS0: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "CLASS 0 Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->sender, NULL);
		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT: {
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
	case MSG_NOTI_TYPE_MMS_READ_REPORT: {
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
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT: {
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
		/* CID 45672: noti_info->extra_data in unsigned char but MSG_READ_REPORT_NONE is -1. So the expression is always false */
#if 0
		else if (noti_info->extra_data == MSG_DELIVERY_REPORT_NONE) {
			/* notification free */
		}
#endif
		else
			setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_INFO_1, "Message delivered", DELIVERED_MESSAGE);

		setNotiTime(noti_h, noti_info->time);
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "SIM card full", SMS_SIM_CARD_FULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, "Not enough memory. Delete some items.", SMS_MESSAGE_MEMORY_FULL);
		break;
	}
	default:
		MSG_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_END();

}

void setIcon(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	switch (noti_info->type) {
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_NORMAL :
	case MSG_NOTI_TYPE_CB :
	case MSG_NOTI_TYPE_SIM :
	{
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

				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_SUB, MSG_MESSAGE_APP_SUB_ICON);
			}
		}
		break;
	}
#else
	case MSG_NOTI_TYPE_NORMAL :
	{
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, MSG_NORMAL_ICON_PATH);

		if (noti_info->count > 1) {
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		} else {
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_REPLY_ICON_PATH);
		}
		break;
	}
	case MSG_NOTI_TYPE_CB: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_CB_ICON_PATH);
		break;
	}
	case MSG_NOTI_TYPE_SIM: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SIM_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_SIM_ICON_PATH);
		break;
	}
#endif
	case MSG_NOTI_TYPE_FAILED: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SMS_SENDING_FAILED_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_SMS_SENDING_FAILED_ICON_PATH);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_1:
	case MSG_NOTI_TYPE_VOICE_2:
	case MSG_NOTI_TYPE_MWI: {
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_VOICE_MSG_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_VOICE_ICON_PATH);
		break;
	}
	case MSG_NOTI_TYPE_CLASS0:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	case MSG_NOTI_TYPE_SIM_FULL:
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, MSG_NORMAL_STATUS_ICON);
		break;
	default:
		MSG_DEBUG("No matching type for MsgNotiSetImage [%d]", noti_info->type);
		break;
	}

	MSG_END();
}

void setActiveProperty(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	/* set layout */
	noti_err = notification_set_layout(noti_h, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	/* set led */
	noti_err = notification_set_led(noti_h, NOTIFICATION_LED_OP_ON, 0x00);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_led.");
	}

	/* set applist */
	noti_err = notification_set_display_applist(noti_h, NOTIFICATION_DISPLAY_APP_ACTIVE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_display_applist");
	}

	MSG_END();
}


void setActiveText(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_CB: {
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
	case MSG_NOTI_TYPE_CLASS0: {
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, "CLASS 0 Message", NULL);
		setNotiText(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info->active_sender, NULL);
		break;
	}
	default:
		MSG_DEBUG("No matching type [%d]", noti_info->type);
		break;
	}

	MSG_END();

}


void setActiveIcon(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL :
	case MSG_NOTI_TYPE_SIM :
	{
		switch (noti_info->active_subtype) {
		case MSG_CB_SMS :
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
			break;
		case MSG_WAP_SI_SMS :
		case MSG_WAP_SL_SMS :
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_ACTIVE_PUSH_ICON_PATH);
		case MSG_SYNCML_CP :
			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_OTA_ICON_PATH);
		default :
			if (noti_info->imagePath[0] != '\0')
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, noti_info->imagePath);
			else
				setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NO_CONTACT_PROFILE_ICON_PATH);

			setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON_SUB, MSG_MESSAGE_APP_SUB_ICON);
		}

		break;
	}
	case MSG_NOTI_TYPE_CB :
	{
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_CB_ICON_PATH);
		break;
	}
	case MSG_NOTI_TYPE_CLASS0 :
		setNotiImage(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, MSG_NORMAL_ICON_PATH);
		break;
	default:
		MSG_DEBUG("No matching type for MsgNotiSetImage [%d]", noti_info->type);
		break;
	}

	MSG_END();
}

msg_error_t MsgInsertInstantMessage(msg_notification_type_t noti_type)
{
	MSG_BEGIN();

	msg_error_t msg_err = MSG_SUCCESS;
	char *notiMsg = NULL;

	notification_h noti = notification_create(NOTIFICATION_TYPE_NOTI);

	setPkgName(noti, noti_type);

	switch (noti_type) {
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_CB: {
		MSG_NOTI_INFO_S noti_info;
		memset(&noti_info, 0x00, sizeof(MSG_NOTI_INFO_S));

		noti_info.type = noti_type;
		msg_err = getLatestMsgInfo(&noti_info, true);

		if (msg_err == MSG_SUCCESS) {
			MSG_DEBUG("Unread count [%d]", noti_info.count);
			if (noti_info.count == 1) {
				MSG_SEC_DEBUG("noti_info.sender [%s]",noti_info.sender);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_TITLE, noti_info.sender, NULL);
				setNotiText(noti, NOTIFICATION_TEXT_TYPE_CONTENT, noti_info.text, NULL);
			} else if (noti_info.count > 1) {
				gchar *cnt_string = g_strdup_printf("%i", noti_info.count);

				notiMsg = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, NEW_MESSAGE);
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
	case MSG_NOTI_TYPE_FAILED: {
		notiMsg = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, FAILED_TO_SEND_MESSAGE);
		setNotiText(noti, NOTIFICATION_TEXT_TYPE_TITLE, notiMsg, NULL);
		setNotiImage(noti, NOTIFICATION_IMAGE_TYPE_ICON, MSG_SMS_SENDING_FAILED_ICON_PATH);
		break;
	}
	default:
		MSG_DEBUG("No matching type for MsgNotiType%d]", noti_type);
		goto _END_OF_INSTANT_NOTI;
		break;
	}

	if (notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_TICKER) != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_set_display_applist");

	if (notification_post(noti) != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_post");

_END_OF_INSTANT_NOTI:

	if (notification_delete(noti) != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_delete");
	if (notiMsg) {
		free(notiMsg);
		notiMsg = NULL;
	}

	if (noti) {
		if (notification_free(noti) != NOTIFICATION_ERROR_NONE)
			MSG_DEBUG("Fail to notification_free");
		noti = NULL;
	}

	MSG_END();
	return MSG_SUCCESS;
}

void setPkgName(notification_h noti_h, msg_notification_type_t noti_type)
{
	MSG_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	switch (noti_type) {
	case MSG_NOTI_TYPE_NORMAL:
	case MSG_NOTI_TYPE_CB:
	case MSG_NOTI_TYPE_SIM:
	case MSG_NOTI_TYPE_FAILED:
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT:
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT:
	case MSG_NOTI_TYPE_SIM_FULL: {
		noti_err = notification_set_pkgname(noti_h, MSG_DEFAULT_APP_ID);
		if (noti_err != NOTIFICATION_ERROR_NONE)
			MSG_ERR("notification_set_pkgname() is failed, [%d]", noti_err);
		break;
	}
	default:
		MSG_DEBUG("No matching type for notification_set_pkgname() [%d]", noti_type);
		break;
	}

	MSG_END();
}


void setSoundAndVibration(notification_h noti_h, char *addressVal, bool bVoiceMail)
{
	MSG_BEGIN();

	bool bBlockingMode = false;

	MSG_ADDRESS_INFO_S addrInfo;
	memset(&addrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	MSG_CONTACT_INFO_S contactInfo;
	memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	if (addressVal != NULL) {
		snprintf(addrInfo.addressVal, sizeof(addrInfo.addressVal), "%s", addressVal);
		/* Get Contact Info */
		if (MsgGetContactInfo(&addrInfo, &contactInfo) != MSG_SUCCESS) {
			MSG_DEBUG("MsgGetContactInfo() fail.");
		}
		bBlockingMode = checkBlockingMode(addressVal, NULL);
	} else {
		MSG_DEBUG("addressVal is NULL.");
	}

	char *msg_tone_file_path = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&msg_tone_file_path, unique_ptr_deleter);

	MsgSoundPlayer::instance()->MsgGetRingtonePath(contactInfo.alerttonePath, &msg_tone_file_path);

	MSG_SEC_DEBUG("Sound File [%s]", msg_tone_file_path);
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

	bool bPlaySound = false;
	bool bPlayVibration = false;
	bool bOnCall = false;

	MsgSoundPlayer::instance()->MsgGetPlayStatus(bVoiceMail, &bPlaySound, &bPlayVibration, &bOnCall);

	if (!bBlockingMode) { /* check blocking mode. */
		if (bPlaySound) {
/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			if (msg_tone_file_path)
				setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_USER_DATA, msg_tone_file_path);
			else {
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
				MSG_RINGTONE_TYPE_T ringtoneType = (MSG_RINGTONE_TYPE_T)MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE);
				if (ringtoneType == MSG_RINGTONE_TYPE_SILENT)
					setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
				else
					setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_DEFAULT, NULL);
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
		} else {
			setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
		}

		if (bPlayVibration) {
/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			if (contactInfo.vibrationPath[0] == '\0')
				setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_DEFAULT, NULL);
			else
				setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_USER_DATA, contactInfo.vibrationPath);
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
		} else {
			setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
		}
	} else {
		setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
		setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
	}
	MSG_END();
}


void setActiveNotification(notification_h noti_h, MSG_NOTI_INFO_S *noti_info)
{
	MSG_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	setPkgName(noti_h, noti_info->type);

	setActiveProperty(noti_h, noti_info);

	setTextDomain(noti_h, noti_info->type);

	setActiveText(noti_h, noti_info);

	setActiveIcon(noti_h, noti_info);

	if (noti_info->active_noti_button_num > 1) {
		createServiceHandle(&noti_info->active_noti_svc_h[0]);
		if (noti_info->active_noti_svc_h[0]) {
			setServicePackageName(noti_info->active_noti_svc_h[0], MSG_CALL_APP_ID);
			setServiceOperation(noti_info->active_noti_svc_h[0], APP_CONTROL_OPERATION_CALL);

			MSG_DEBUG("Active Notification button 1 - Msg Id = [%d]", noti_info->msg_id);

			char tel_num[MSG_NOTI_TEXT_LEN_S] = {0,};
			snprintf(tel_num, sizeof(tel_num), "tel:%s", noti_info->number);
			MSG_SEC_DEBUG("Active sender number [%s]", noti_info->number);
			setServiceUri(noti_info->active_noti_svc_h[0], tel_num);
		}

		createServiceHandle(&noti_info->active_noti_svc_h[1]);
		if (noti_info->active_noti_svc_h[1]) {
			setServicePackageName(noti_info->active_noti_svc_h[1], MSG_DEFAULT_APP_ID);

			MSG_DEBUG("Active Notification button 2 - Msg Id = [%d]", noti_info->msg_id);
			addServiceExtraData(noti_info->active_noti_svc_h[1], "type", "reply");
			addServiceExtraData(noti_info->active_noti_svc_h[1], "msgId", noti_info->msg_id);

			char slot_id[5] = {0,};
			snprintf(slot_id, sizeof(slot_id), "%d", noti_info->sim_idx - 1);
			addServiceExtraData(noti_info->active_noti_svc_h[1], "slot_id", slot_id);
		}
	}

	createServiceHandle(&noti_info->active_noti_svc_h[2]);
	if (noti_info->active_noti_svc_h[2]) {
		setServicePackageName(noti_info->active_noti_svc_h[2], MSG_DEFAULT_APP_ID);

		MSG_DEBUG("Active Notification button 3 - msgId = [%d]", noti_info->msg_id);
		addServiceExtraData(noti_info->active_noti_svc_h[2], "type", "new_msg");
		addServiceExtraData(noti_info->active_noti_svc_h[2], "msgId", noti_info->msg_id);
		addServiceExtraData(noti_info->active_noti_svc_h[2], "CALLER", "active_noti");

		char slot_id[5] = {0,};
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

	noti_err = notification_post(noti_h);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_post");
	}

	MSG_END();
}


void setNotification(notification_h noti_h, MSG_NOTI_INFO_S *noti_info, bool bFeedback)
{
	MSG_BEGIN();

	int noti_err = NOTIFICATION_ERROR_NONE;

	if (bFeedback && noti_info->active_noti_button_num > 0 &&
		((noti_info->type >= MSG_NOTI_TYPE_NORMAL && noti_info->type <= MSG_NOTI_TYPE_SIM) || noti_info->type == MSG_NOTI_TYPE_CLASS0)) {
		notification_h active_noti_h = notification_create(NOTIFICATION_TYPE_NOTI);

		setActiveNotification(active_noti_h, noti_info);

		notification_free(active_noti_h);
		active_noti_h = NULL;
	}

	setPkgName(noti_h, noti_info->type);

	setProperty(noti_h, noti_info);

	setTextDomain(noti_h, noti_info->type);

	setText(noti_h, noti_info);

	setIcon(noti_h, noti_info);

	if (bFeedback) {
		if (noti_info->type == MSG_NOTI_TYPE_VOICE_1 || noti_info->type == MSG_NOTI_TYPE_VOICE_2)
			setSoundAndVibration(noti_h, noti_info->number, true);
		else
			setSoundAndVibration(noti_h, noti_info->number, false);

	} else {
		setNotiSound(noti_h, NOTIFICATION_SOUND_TYPE_NONE, NULL);
		setNotiVibration(noti_h, NOTIFICATION_VIBRATION_TYPE_NONE, NULL);
	}

	if (noti_info->id > 0) {
		MSG_DEBUG("Notification update");
		noti_err = notification_update(noti_h);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_update");
		}
	} else {
		MSG_DEBUG("Notification insert");
		noti_err = notification_insert(noti_h, &noti_info->id);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_insert");
		}

		updatePrivId(noti_info->type, noti_info->id, noti_info->sim_idx);
	}

	MSG_END();
}


msg_error_t getLatestMsgInfo(MSG_NOTI_INFO_S *noti_info, bool isForInstantMessage)
{
	MSG_BEGIN();

	MsgDbHandler *dbhandler = getDbHandle();
	int noti_err = NOTIFICATION_ERROR_NONE;
	msg_error_t msg_err = MSG_SUCCESS;

	switch (noti_info->type) {
	case MSG_NOTI_TYPE_NORMAL:
#ifdef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_CB:
	case MSG_NOTI_TYPE_SIM:
#endif
	{
		int smsUnreadCnt = 0;
		int mmsUnreadCnt = 0;

		char sqlQuery[MAX_QUERY_LEN+1];
		MSG_MAIN_TYPE_T mainType;
		MSG_SUB_TYPE_T subType;
		int msgSize;

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT "
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
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT "
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
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		int rowCnt = 0, index = 0;

		msg_err = dbhandler->getTable(sqlQuery, &rowCnt, &index);
		MSG_DEBUG("getTable() ret=[%d], rowCnt=[%d]", msg_err, rowCnt);

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
		MSG_ADDRESS_INFO_S tmpAddressInfo;
		int normalAddCnt = 0;

		for (int i = 1; i <= rowCnt; i++) {
			memset(&tmpAddressInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

			char *address = dbhandler->getColumnToString(index++);
			normalAddCnt++;
			if (address) {
				snprintf(tmpAddressInfo.addressVal, MAX_ADDRESS_VAL_LEN, "%s", address);
				if (msg_is_valid_email(address)) {
					tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_EMAIL;
				} else {
					tmpAddressInfo.addressType = MSG_ADDRESS_TYPE_UNKNOWN;
				}
			}
			subType = dbhandler->getColumnToInt(index++);

			MSG_CONTACT_INFO_S tmpContact;
			memset(&tmpContact, 0x00, sizeof(MSG_CONTACT_INFO_S));

			MsgGetContactInfo(&tmpAddressInfo, &tmpContact);

			if (rowCnt == 1) {
				snprintf(noti_info->imagePath, sizeof(noti_info->imagePath), "%s", tmpContact.imagePath);
			}

			if (normalAddCnt > 1) {
				g_strlcat(noti_info->sender, ", ", sizeof(noti_info->sender)-strlen(noti_info->sender));
			}

			if (tmpContact.firstName[0] != '\0') {
				g_strlcat(noti_info->sender, tmpContact.firstName, sizeof(noti_info->sender)-strlen(noti_info->sender));
			} else if (tmpAddressInfo.addressVal[0] == '\0') {
				char *senderStr = NULL;
				senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
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
					senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, CB_MESSAGE);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					noti_info->active_noti_button_num = 1;
				} else if (subType == MSG_SYNCML_CP) {
					senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, CP_MESSAGE);
					g_strlcat(noti_info->sender, senderStr, sizeof(noti_info->sender)-strlen(noti_info->sender));
					noti_info->active_noti_button_num = 1;
				} else if (subType == MSG_WAP_SI_SMS || subType == MSG_WAP_SL_SMS) {
					senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, PUSH_MESSAGE);
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
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
		dbhandler->freeTable();

		MSG_SEC_DEBUG("sender info = [%s]", noti_info->sender);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
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
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
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
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		if (dbhandler->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbhandler->stepQuery() == MSG_ERR_DB_ROW) {

			smsUnreadCnt = dbhandler->columnInt(6);
			mmsUnreadCnt = dbhandler->columnInt(7);
			msgSize = dbhandler->columnInt(8);

			noti_info->count = smsUnreadCnt + mmsUnreadCnt;

			if (noti_info->count > 0) {
				snprintf(noti_info->number, sizeof(noti_info->number), "%s", (char*)dbhandler->columnText(0));

				noti_info->time = (time_t)dbhandler->columnInt(1);

				noti_info->msg_id = (msg_message_id_t)dbhandler->columnInt(2);

				mainType = (MSG_MAIN_TYPE_T)dbhandler->columnInt(5);

				if (mainType == MSG_MMS_TYPE) {
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(3));
					if (noti_info->text[0] == '\0') {
						char *noti_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_NO_SUBJECT);
						snprintf(noti_info->text, sizeof(noti_info->text), "%s", noti_text);
						g_free(noti_text);
					}

					char *prefix_subject = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_SUBJECT_COLON);
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

						char *msg_size_string = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MESSAGE_SIZE_STRING);
						char *msg_size_unit_kb = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MESSAGE_SIZE_UNIT_KB);

						snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s : %d%s", msg_size_string, kb_msg_size, msg_size_unit_kb);

						g_free(msg_size_string);
						g_free(msg_size_unit_kb);
					}

				} else {
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(4));
				}

				if (noti_info->active_text[0] == '\0')
					snprintf(noti_info->active_text, MSG_NOTI_TEXT_LEN, "%s", (char*)dbhandler->columnText(4));

				MSG_DEBUG("unread message ID [%d].", noti_info->msg_id);

				MSG_DEBUG("active sender [%s]", noti_info->active_sender);
				MSG_DEBUG("active subject [%s]", noti_info->active_subject);
				MSG_DEBUG("active text [%s]", noti_info->active_text);

				if (!isForInstantMessage) {
					if (noti_info->id > 0 && noti_info->count == 1) {
						noti_err = notification_delete_by_priv_id(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}

						noti_info->id = 0;
						if (MsgSettingSetInt(NOTIFICATION_PRIV_ID, noti_info->id) != MSG_SUCCESS)
							MSG_DEBUG("MsgSettingSetInt fail : NOTIFICATION_PRIV_ID");
					}
					MsgSettingHandleNewMsg(smsUnreadCnt, mmsUnreadCnt);
					MsgInsertBadge(noti_info->count);
					MsgSoundSetRepeatAlarm();
				}
			} else {

				MSG_DEBUG("No unread message.");
				MSG_DEBUG("notiPrivId [%d]", noti_info->id);

				dbhandler->finalizeQuery();

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (MsgSettingSetInt(NOTIFICATION_PRIV_ID, noti_info->id) != MSG_SUCCESS)
						MSG_DEBUG("MsgSettingSetInt fail : NOTIFICATION_PRIV_ID");

					MsgSettingHandleNewMsg(0,0);
					MsgInsertBadge(0);
					MsgSoundSetRepeatAlarm();
				}

				return MSG_ERR_DB_STEP;
			}
		} else {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbhandler->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();
		break;
	}

#ifndef MSG_NOTI_INTEGRATION
	case MSG_NOTI_TYPE_CB: {
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
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
				senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
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

			MSG_DEBUG("unread CB message [%d].", noti_info->msg_id);
		} else {
			MSG_DEBUG("No unread CB message.");
			MSG_DEBUG("notiCbId [%d]", noti_info->id);

			dbhandler->finalizeQuery();

			if (!isForInstantMessage) {
				/* No unread message. */
				if (noti_info->id > 0) {
					noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, noti_info->id);
					if (noti_err != NOTIFICATION_ERROR_NONE) {
						MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
					}
				}

				noti_info->id = 0;

				if (MsgSettingSetInt(CB_NOTI_PRIV_ID, noti_info->id) != MSG_SUCCESS)
					MSG_DEBUG("MsgSettingSetInt fail : CB_NOTI_PRIV_ID");
			}
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();

		if (dbhandler->getTable(sqlQuery, &noti_info->count, NULL) != MSG_SUCCESS) {
			MSG_DEBUG("getTable is failed");
			dbhandler->freeTable();
			return MSG_ERR_DB_GETTABLE;
		}

		dbhandler->freeTable();
		MSG_DEBUG("notiCbId [%d], unreadCbMsgCnt [%d]", noti_info->id, noti_info->count);
		break;
	}
	case MSG_NOTI_TYPE_SIM: {
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
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

		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

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
					senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
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

				MSG_DEBUG("unread SIM message [%d].", noti_info->msg_id);
			} else {

				MSG_DEBUG("No unread SIM message.");
				MSG_DEBUG("notiPrivId [%d]", noti_info->id);

				dbhandler->finalizeQuery();

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (MsgSettingSetInt(SIM_MSG_NOTI_PRIV_ID, noti_info->id) != MSG_SUCCESS)
						MSG_DEBUG("MsgSettingSetInt fail : SIM_MSG_NOTI_PRIV_ID");
				}

				return MSG_ERR_DB_STEP;
			}
		} else {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbhandler->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();
		break;
	}
#endif
	case MSG_NOTI_TYPE_FAILED: {
		MSG_MAIN_TYPE_T mainType;
		char sqlQuery[MAX_QUERY_LEN+1];
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
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

		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		if (dbhandler->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbhandler->stepQuery() == MSG_ERR_DB_ROW) {

			noti_info->count = dbhandler->columnInt(6);

			if (noti_info->count > 0) {
				MSG_ADDRESS_INFO_S addrInfo;
				memset(&addrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				if (dbhandler->columnText(0) != NULL)
					snprintf(addrInfo.addressVal, sizeof(addrInfo.addressVal), "%s", (char*)dbhandler->columnText(0));

/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
				MSG_CONTACT_INFO_S tmpContact;
				memset(&tmpContact, 0x00, sizeof(MSG_CONTACT_INFO_S));

				MsgGetContactInfo(&addrInfo, &tmpContact);

				if (tmpContact.firstName[0] != '\0') {
					snprintf(noti_info->sender, sizeof(noti_info->sender), "%s", tmpContact.firstName);
				} else if (addrInfo.addressVal[0] == '\0') {
#else /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
						if (addrInfo.addressVal[0] == '\0') {
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
					char *senderStr = NULL;
					senderStr = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, MSG_UNKNOWN_SENDER);
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

				mainType = (MSG_MAIN_TYPE_T)dbhandler->columnInt(5);

				if (mainType == MSG_TYPE_MMS)
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(4));
				else
					snprintf(noti_info->text, sizeof(noti_info->text), "%s", (char*)dbhandler->columnText(3));

				MSG_DEBUG("Sent failed message ID [%d].", noti_info->msg_id);

				if (!isForInstantMessage) {
					if (noti_info->id > 0 && noti_info->count == 1) {
						noti_err = notification_delete_by_priv_id(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
						noti_info->id = 0;
						if (MsgSettingSetInt(MSG_SENTFAIL_NOTI_ID, noti_info->id) != MSG_SUCCESS)
							MSG_DEBUG("MsgSettingSetInt fail : MSG_SENTFAIL_NOTI_ID");
					}
				}
			} else {

				MSG_DEBUG("No sent failed message.");
				MSG_DEBUG("failedNotiId [%d]", noti_info->id);

				dbhandler->finalizeQuery();

				if (!isForInstantMessage) {
					/* No unread message. */
					if (noti_info->id > 0) {
						noti_err = notification_delete_by_priv_id(MSG_DEFAULT_APP_ID, NOTIFICATION_TYPE_NOTI, noti_info->id);
						if (noti_err != NOTIFICATION_ERROR_NONE) {
							MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
						}
					}

					noti_info->id = 0;

					if (MsgSettingSetInt(MSG_SENTFAIL_NOTI_ID, noti_info->id) != MSG_SUCCESS)
						MSG_DEBUG("MsgSettingSetInt fail : MSG_SENTFAIL_NOTI_ID");
				}

				return MSG_ERR_DB_STEP;
			}
		} else {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbhandler->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbhandler->finalizeQuery();
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL:
		break;
	default:
		MSG_DEBUG("No matching type [%d]", noti_info->type);
		return MSG_ERR_UNKNOWN;
	}

	MSG_END();

	return MSG_SUCCESS;
}


void createServiceHandle(app_control_h *svc_h)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_create(svc_h);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_create() is failed, [%d]", svc_err);
}


void setServiceAppId(app_control_h svc_h, const char* app_id)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_app_id(svc_h, app_id);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_set_app_id() was failed, [%d]", svc_err);
}


void setServiceUri(app_control_h svc_h, const char* uri)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_uri(svc_h, uri);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_set_uri() was failed, [%d]", svc_err);
}


void setServiceOperation(app_control_h svc_h, const char* operation)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_operation(svc_h, operation);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_set_operation() was failed, [%d]", svc_err);
}


void addServiceExtraData(app_control_h svc_h, const char* bundle_key, const char* bundle_val)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_add_extra_data(svc_h, bundle_key, bundle_val);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_add_extra_data() was failed, [%d]", svc_err);
}


void addServiceExtraData(app_control_h svc_h, const char* bundle_key, int bundle_val)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	char tempId[10];
	memset(&tempId, 0x00, sizeof(tempId));
	snprintf(tempId, sizeof(tempId), "%d", bundle_val);

	svc_err = app_control_add_extra_data(svc_h, bundle_key, (const char *)tempId);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_add_extra_data() was failed, [%d]", svc_err);
}


void setServicePackageName(app_control_h svc_h, const char* pkg_name)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_set_app_id(svc_h, pkg_name);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_set_app_id() was failed, [%d]", svc_err);
}


void sendServicelaunchRequest(app_control_h svc_h, app_control_reply_cb callback, void *user_data)
{
	int svc_err = APP_CONTROL_ERROR_NONE;

	svc_err = app_control_send_launch_request(svc_h, callback, user_data);

	if (svc_err != APP_CONTROL_ERROR_NONE)
		MSG_DEBUG("app_control_send_launch_request() is failed : %d", svc_err);
}


void setNotiTextDomain(notification_h noti_h, const char *pkg_name, const char *loc_dir)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_text_domain(noti_h, pkg_name, loc_dir);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("notification_set_text_domain() was failed. [%d]", noti_err);
	}
}


void setNotiText(notification_h noti_h, notification_text_type_e type, const char *text, const char *key)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_text(noti_h, type, text, key, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_text() was failed. [%d]", noti_err);
}


void setNotiTimeToText(notification_h noti_h, notification_text_type_e type, time_t time)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_time_to_text(noti_h, type, time);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_time_to_text() was failed. [%d]", noti_err);
}


void setNotiTime(notification_h noti_h, time_t time)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_time(noti_h, time);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_time() was failed. [%d]", noti_err);
}



void setNotiImage(notification_h noti_h, notification_image_type_e type, const char *image_path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_image(noti_h, type, image_path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_image() was failed. [%d]", noti_err);
}


void setNotiSound(notification_h noti_h, notification_sound_type_e type, const char *path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_sound(noti_h, type, path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_sound() was failed. [%d]", noti_err);
}


void setNotiVibration(notification_h noti_h, notification_vibration_type_e type, const char *path)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_vibration(noti_h, type, path);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_vibration() was failed. [%d]", noti_err);
}


void setNotiEventHandler(notification_h noti_h, notification_event_type_e type, app_control_h event_handler)
{
	int noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_event_handler(noti_h, type, event_handler);

	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("notification_set_event_handler() was failed. [%d]", noti_err);
}


char *getTranslateText(const char *pkg_name, const char *locale_dir, const char *text)
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

void MsgNotiSoundRepeatAlarmCB(int alarmId)
{
	MSG_BEGIN();

	MsgRefreshNotification(MSG_NOTI_TYPE_NORMAL, true, MSG_ACTIVE_NOTI_TYPE_ACTIVE);

#ifndef MSG_NOTI_INTEGRATION
	MsgRefreshNotification(MSG_NOTI_TYPE_SIM, true, MSG_ACTIVE_NOTI_TYPE_ACTIVE);
	MsgRefreshNotification(MSG_NOTI_TYPE_CB, true, MSG_ACTIVE_NOTI_TYPE_ACTIVE);
#endif

	MSG_END();
	return;
}


void MsgSoundCreateRepeatAlarm(int RepeatTime)
{
	MSG_BEGIN();

	int tmpAlarmId = 0;
	time_t tmp_time;
	struct tm repeat_tm;

	time(&tmp_time);

	tmp_time += (RepeatTime*60);
	tzset();
	localtime_r(&tmp_time, &repeat_tm);

	if (MsgAlarmRegistration(&repeat_tm, MsgNotiSoundRepeatAlarmCB, &tmpAlarmId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgAlarmRegistration fail.");
		return;
	}

	g_alarmId = tmpAlarmId;
	MSG_DEBUG("Set alarmId to [%d]", g_alarmId);

	MSG_END();

	return;
}


void MsgSoundSetRepeatAlarm()
{
	int nRepeatValue = 0;
	long	nRepeatTime = 0;

	nRepeatValue = MsgSettingGetInt(MSG_ALERT_REP_TYPE);

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
		MSG_DEBUG("Invalid Repetition time");
		break;
	}

	MSG_DEBUG("nRepeatTime = %d", nRepeatTime);

	if (nRepeatTime > 0) {
		if (g_alarmId > 0) {
			if (MsgAlarmRemove(g_alarmId) != MSG_SUCCESS) {
				MSG_FATAL("MsgAlarmRemove fail.");
			}
			g_alarmId = 0;
		}
		MsgSoundCreateRepeatAlarm(nRepeatTime);
	}

	return;
}


void sendMsgReplyPopup(MSG_NOTI_INFO_S *noti_info)
{
	app_control_h svc_h = NULL;

	createServiceHandle(&svc_h);

	if (svc_h) {
		setServicePackageName(svc_h, "org.tizen.msg-ui-reply");

		MSG_DEBUG("Reply-popup display - Msg Id = [%d]", noti_info->msg_id);
		addServiceExtraData(svc_h, "msgId", noti_info->msg_id);

		sendServicelaunchRequest(svc_h, NULL, NULL);

		app_control_destroy(svc_h);
	}
}

#endif /* MSG_WEARABLE_PROFILE */
