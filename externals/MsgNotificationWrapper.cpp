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
#include "MsgUtilFunction.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgDevicedWrapper.h"

extern "C"
{
#include <bundle.h>
#ifndef MSG_WEARABLE_PROFILE
#include <notification_internal.h>
#include <notification_status.h>
#include <notification_setting.h>
#include <notification_setting_internal.h>
#endif /* MSG_WEARABLE_PROFILE */
}

#ifndef MSG_WEARABLE_PROFILE

/*======================================================================================*/
/*								VARIABLES AND STRUCTURES								*/
/*======================================================================================*/

typedef struct _del_noti_info_s
{
	msg_notification_type_t			type;
	int 		sim_idx;
}DEL_NOTI_INFO_S;


/*======================================================================================*/
/*									FUNCTION DEFINE										*/
/*======================================================================================*/

int getPrivId(msg_notification_type_t noti_type, int sim_idx);
void updatePrivId(msg_notification_type_t noti_type, int noti_id, int sim_idx);

void MsgDeleteNotiCb(void *data);

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
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "insert_only_active_noti");
	if (noti_type == MSG_NOTI_TYPE_NORMAL)
		bundle_add_str(bundle_data, "type", "normal");
	else if (noti_type == MSG_NOTI_TYPE_CLASS0)
		bundle_add_str(bundle_data, "type", "class0");

	msg_error_t err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
	return err;
}


msg_error_t MsgDeleteReportNotification(const char *addr)
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "del_report_noti");
	bundle_add_str(bundle_data, "address", addr);

	msg_error_t err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();

	return err;
}

msg_error_t MsgAddReportNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "add_report_noti");
	switch (noti_type) {
	case MSG_NOTI_TYPE_SMS_DELIVERY_REPORT:
		bundle_add_str(bundle_data, "type", "sms_delivery");
		break;
	case MSG_NOTI_TYPE_MMS_DELIVERY_REPORT:
		bundle_add_str(bundle_data, "type", "mms_delivery");
		break;
	case MSG_NOTI_TYPE_MMS_READ_REPORT:
		bundle_add_str(bundle_data, "type", "mms_read");
		break;
	default:
		break;
	}

	char *msg_id = g_strdup_printf("%d", msg_info->msgId);
	if (msg_id) {
		bundle_add_str(bundle_data, "msg_id", msg_id);
		g_free(msg_id);
	}

	msg_error_t err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
}


msg_error_t MsgRefreshNotification(msg_notification_type_t noti_type, bool bFeedback, msg_active_notification_type_t active_type)
{
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "refresh_noti");
	switch (noti_type) {
	case MSG_NOTI_TYPE_NORMAL:
		bundle_add_str(bundle_data, "type", "normal");
		break;
	case MSG_NOTI_TYPE_CB:
		bundle_add_str(bundle_data, "type", "cb");
		break;
	case MSG_NOTI_TYPE_SIM:
		bundle_add_str(bundle_data, "type", "sim");
		break;
	case MSG_NOTI_TYPE_FAILED:
		bundle_add_str(bundle_data, "type", "failed");
		break;
	default:
		break;
	}

	switch (active_type) {
	case MSG_ACTIVE_NOTI_TYPE_NONE:
		bundle_add_str(bundle_data, "active_type", "none");
		break;
	case MSG_ACTIVE_NOTI_TYPE_ACTIVE:
		bundle_add_str(bundle_data, "active_type", "active");
		break;
	case MSG_ACTIVE_NOTI_TYPE_INSTANT:
		bundle_add_str(bundle_data, "active_type", "instant");
		break;
	default:
		break;
	}

	if (bFeedback)
		bundle_add_str(bundle_data, "feedback", "true");
	else
		bundle_add_str(bundle_data, "feedback", "false");

	msg_error_t err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
}


msg_error_t MsgAddNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "add_noti");
	switch (noti_type) {
	case MSG_NOTI_TYPE_VOICE_1:
		bundle_add_str(bundle_data, "type", "voice1");
		break;
	case MSG_NOTI_TYPE_VOICE_2:
		bundle_add_str(bundle_data, "type", "voice2");
		break;
	case MSG_NOTI_TYPE_MWI:
		bundle_add_str(bundle_data, "type", "mwi");
		break;
	case MSG_NOTI_TYPE_CLASS0:
		bundle_add_str(bundle_data, "type", "class0");
		break;
	default:
		break;
	}

	char *msg_id = g_strdup_printf("%d", msg_info->msgId);
	if (msg_id) {
		bundle_add_str(bundle_data, "msg_id", msg_id);
		g_free(msg_id);
	}

	msg_error_t err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
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
		noti_err = notification_delete_all_by_type(NULL, NOTIFICATION_TYPE_NOTI);
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

			/* check do not disturb mode */
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


msg_error_t MsgDeleteNoti(msg_notification_type_t noti_type, int simIndex)
{
#ifndef MSG_WEARABLE_PROFILE
	bool bNotiSvcReady = false;

	DEL_NOTI_INFO_S *delNotiInfo = (DEL_NOTI_INFO_S *)calloc(1, sizeof(DEL_NOTI_INFO_S));

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


void MsgSoundPlayStart(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_SOUND_TYPE_T soundType)
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "sound_play_start");
	switch (soundType) {
	case MSG_SOUND_PLAY_DEFAULT:
		bundle_add_str(bundle_data, "type", "default");
		break;
	case MSG_SOUND_PLAY_USER:
		bundle_add_str(bundle_data, "type", "user");
		break;
	case MSG_SOUND_PLAY_EMERGENCY:
		bundle_add_str(bundle_data, "type", "emergency");
		break;
	case MSG_SOUND_PLAY_VOICEMAIL:
		bundle_add_str(bundle_data, "type", "voicemail");
		break;
	default:
		break;
	}

	if (pAddrInfo && pAddrInfo->addressVal != '\0')
		bundle_add_str(bundle_data, "address", pAddrInfo->addressVal);

	msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
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
			MsgSoundPlayStart(NULL, MSG_SOUND_PLAY_DEFAULT);
		}
	}

#endif /* MSG_WEARABLE_PROFILE */
	return MSG_SUCCESS;
}


#ifndef MSG_WEARABLE_PROFILE
void MsgDeleteNotiCb(void *data)
{
	if (data) {
		DEL_NOTI_INFO_S *delNotiInfo = (DEL_NOTI_INFO_S *)data;

		MsgDeleteNotification(delNotiInfo->type, delNotiInfo->sim_idx);

		free(data);
		data = NULL;
	}

	return;
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
		if (MsgSettingGetInt(NOTIFICATION_PRIV_ID, &noti_id) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
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
		if (MsgSettingGetInt(MSG_SENTFAIL_NOTI_ID, &noti_id) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		break;
	case MSG_NOTI_TYPE_VOICE_1: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		if (MsgSettingGetInt(keyName, &noti_id) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		break;
	}
	case MSG_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_2, sim_idx);
		if (MsgSettingGetInt(keyName, &noti_id) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		break;
	}
	case MSG_NOTI_TYPE_SIM_FULL:
		if (MsgSettingGetInt(SIM_FULL_NOTI_PRIV_ID, &noti_id) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
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
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICE_NOTI_ID_1, sim_idx);
		err = MsgSettingSetInt(keyName, noti_id);
		break;
	}
	case MSG_NOTI_TYPE_VOICE_2: {
		char keyName[MAX_VCONFKEY_NAME_LEN] = {0, };
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

#endif /* MSG_WEARABLE_PROFILE */
