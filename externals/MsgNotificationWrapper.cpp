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
#ifndef MSG_WEARABLE_PROFILE
#include <bundle.h>
#endif /* MSG_WEARABLE_PROFILE */
}

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

	msg_error_t err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "insert_only_active_noti");
	if (noti_type == MSG_NOTI_TYPE_NORMAL)
		bundle_add_str(bundle_data, "type", "normal");
	else if (noti_type == MSG_NOTI_TYPE_CLASS0)
		bundle_add_str(bundle_data, "type", "class0");

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
	return err;
}


msg_error_t MsgDeleteReportNotification(const char *addr)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "del_report_noti");
	bundle_add_str(bundle_data, "address", addr);

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();

	return err;
}

msg_error_t MsgAddReportNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
	msg_error_t err = MSG_SUCCESS;

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

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
}


msg_error_t MsgRefreshNotification(msg_notification_type_t noti_type, bool bFeedback, msg_active_notification_type_t active_type)
{
	msg_error_t err = MSG_SUCCESS;

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

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
}


msg_error_t MsgAddNotification(msg_notification_type_t noti_type, MSG_MESSAGE_INFO_S *msg_info)
{
	msg_error_t err = MSG_SUCCESS;

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

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

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


msg_error_t MsgDeleteNoti(msg_notification_type_t noti_type, int simIndex)
{
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "del_noti");
	switch (noti_type) {
	case MSG_NOTI_TYPE_ALL:
		bundle_add_str(bundle_data, "type", "all");
		break;
	case MSG_NOTI_TYPE_NORMAL:
		bundle_add_str(bundle_data, "type", "normal");
		break;
	case MSG_NOTI_TYPE_SIM:
		bundle_add_str(bundle_data, "type", "sim");
		break;
	case MSG_NOTI_TYPE_VOICE_1:
		bundle_add_str(bundle_data, "type", "voice1");
		break;
	case MSG_NOTI_TYPE_VOICE_2:
		bundle_add_str(bundle_data, "type", "voice2");
		break;
	default:
		break;
	}

	char *sim_index = g_strdup_printf("%d", simIndex);
	if (sim_index) {
		bundle_add_str(bundle_data, "sim_index", sim_index);
		g_free(sim_index);
	}

	msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
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

	if (pAddrInfo && pAddrInfo->addressVal[0] != '\0')
		bundle_add_str(bundle_data, "address", pAddrInfo->addressVal);

	msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}


msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg, bool bPlayFeedback, int msgId)
{
	msg_error_t err = MSG_SUCCESS;
#ifndef MSG_WEARABLE_PROFILE
	MSG_DEBUG("pTickerMsg=[%s], pLocaleTickerMsg=[%s]", pTickerMsg, pLocaleTickerMsg);
	MSG_DEBUG("play feedback=[%d], msgId=[%d]", bPlayFeedback, msgId);

	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "insert_ticker");
	bundle_add_str(bundle_data, "ticker_msg", pTickerMsg);
	bundle_add_str(bundle_data, "locale_ticker_msg", pLocaleTickerMsg);

	if (bPlayFeedback)
		bundle_add_str(bundle_data, "feedback", "true");
	else
		bundle_add_str(bundle_data, "feedback", "false");

	char *msg_id = g_strdup_printf("%d", msgId);
	if (msg_id) {
		bundle_add_str(bundle_data, "msg_id", msg_id);
		g_free(msg_id);
	}

	err = msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	return err;
}


void MsgInitMsgMgr()
{
	msg_launch_app(MSG_MGR_APP_ID, NULL);
}
