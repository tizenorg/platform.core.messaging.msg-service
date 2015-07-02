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
#include "MsgPluginManager.h"
#include "MsgSettingHandler.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFunction.h"
#include "MsgSqliteWrapper.h"
#include "MsgUtilStorage.h"


#define DEF_BUF_LEN	128

/*==================================================================================================
								STATIC FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgSetConfigData(const MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	MSG_SIM_STATUS_T simStatus = MSG_SIM_STATUS_NOT_FOUND;

#ifdef USE_GCONF
	err = MsgGconfGetClient();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Get GConf Client Error");
		return MSG_ERR_NULL_POINTER;
	}
#endif

	MSG_DEBUG("Setting Type : %d", pSetting->type);

	switch (pSetting->type)
	{
		case MSG_GENERAL_OPT :
			err = MsgSetGeneralOpt(pSetting);
			break;
		case MSG_SMS_SENDOPT :
			err = MsgSetSMSSendOpt(pSetting);
			break;
#ifndef FEATURE_SMS_CDMA
		case MSG_SMSC_LIST :
			// Check SIM is present or not
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.smscList.simIndex);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			err = MsgSetSMSCList(pSetting, true);
			break;
#endif
		case MSG_MMS_SENDOPT :
			err = MsgSetMMSSendOpt(pSetting);
			break;
		case MSG_MMS_RECVOPT :
			err = MsgSetMMSRecvOpt(pSetting);
			break;
		case MSG_MMS_STYLEOPT :
			err = MsgSetMMSStyleOpt(pSetting);
			break;
		case MSG_PUSHMSG_OPT :
			err = MsgSetPushMsgOpt(pSetting);
			break;
		case MSG_CBMSG_OPT :
			if (pSetting->option.cbMsgOpt.simIndex != 0) {
				// Check SIM is present or not
				snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.cbMsgOpt.simIndex);
				simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

				if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
					MSG_DEBUG("SIM is not present..");
					return MSG_ERR_NO_SIM;
				}
			}
			err = MsgSetCBMsgOpt(pSetting, true);
			break;
		case MSG_VOICEMAIL_OPT :
			// Check SIM is present or not
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.voiceMailOpt.simIndex);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			err = MsgSetVoiceMailOpt(pSetting, true);
			break;
		case MSG_MSGSIZE_OPT:
			err = MsgSetMsgSizeOpt(pSetting);
			break;
		default :
			break;
	}

#ifdef USE_GCONF
	MsgGconfUnrefClient();
#endif

	return err;
}


msg_error_t MsgGetConfigData(MSG_SETTING_S *pSetting)
{
#ifdef USE_GCONF
	msg_error_t err = MsgGconfGetClient();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Get GConf Client Error");
		return MSG_ERR_NULL_POINTER;
	}
#endif
	char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
	MSG_SIM_STATUS_T simStatus = MSG_SIM_STATUS_NOT_FOUND;

	switch (pSetting->type)
	{
		case MSG_GENERAL_OPT :
			MsgGetGeneralOpt(pSetting);
			break;
		case MSG_SMS_SENDOPT :
			MsgGetSMSSendOpt(pSetting);
			break;
		case MSG_SMSC_LIST :
		{
			// Check SIM is present or not
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.smscList.simIndex);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			MsgGetSMSCList(pSetting);
		}
		break;
		case MSG_MMS_SENDOPT :
			MsgGetMMSSendOpt(pSetting);
			break;
		case MSG_MMS_RECVOPT :
			MsgGetMMSRecvOpt(pSetting);
			break;
		case MSG_MMS_STYLEOPT :
			MsgGetMMSStyleOpt(pSetting);
			break;
		case MSG_PUSHMSG_OPT :
			MsgGetPushMsgOpt(pSetting);
			break;
		case MSG_CBMSG_OPT :
		{
			// Check SIM is present or not
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.cbMsgOpt.simIndex);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			MsgGetCBMsgOpt(pSetting);
		}
		break;
		case MSG_VOICEMAIL_OPT :
			// Check SIM is present or not
			if (pSetting->option.voiceMailOpt.simIndex == 0) {
				MSG_DEBUG("Invalid SIM Index [%d]", pSetting->option.voiceMailOpt.simIndex);
				return MSG_ERR_INVALID_PARAMETER;
			}

			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pSetting->option.voiceMailOpt.simIndex);
			simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			MsgGetVoiceMailOpt(pSetting);
			break;
		case MSG_MSGSIZE_OPT :
			MsgGetMsgSizeOpt(pSetting);
			break;

		default :
			break;
	}

#ifdef USE_GCONF
	MsgGconfUnrefClient();
#endif

	return MSG_SUCCESS;
}


msg_error_t MsgSetGeneralOpt(const MSG_SETTING_S *pSetting)
{
	MSG_GENERAL_OPT_S generalOpt;
	bool bValue = false;
	int iValue = 0;
	char *strValue = NULL;

	memcpy(&generalOpt, &(pSetting->option.generalOpt), sizeof(MSG_GENERAL_OPT_S));

	MsgSettingGetBool(MSG_KEEP_COPY, &bValue);
	if (bValue != generalOpt.bKeepCopy) {
		if (MsgSettingSetBool(MSG_KEEP_COPY, generalOpt.bKeepCopy) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_KEEP_COPY);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MSG_AUTO_ERASE, &bValue);
	if (bValue != generalOpt.bAutoErase) {
		if (MsgSettingSetBool(MSG_AUTO_ERASE, generalOpt.bAutoErase) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_AUTO_ERASE);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MSG_BLOCK_UNKNOWN_MSG, &bValue);
	if (bValue != generalOpt.bBlockUnknownMsg) {
		if (MsgSettingSetBool(MSG_BLOCK_UNKNOWN_MSG, generalOpt.bBlockUnknownMsg) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_BLOCK_UNKNOWN_MSG);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MSG_SMS_LIMIT);
	if (iValue != (int)generalOpt.smsLimitCnt) {
		if (MsgSettingSetInt(MSG_SMS_LIMIT, (int)generalOpt.smsLimitCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SMS_LIMIT);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MSG_MMS_LIMIT);
	if (iValue != (int)generalOpt.mmsLimitCnt) {
		if (MsgSettingSetInt(MSG_MMS_LIMIT, (int)generalOpt.mmsLimitCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_MMS_LIMIT);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bValue);
	if (bValue != generalOpt.bNotification) {
		if (MsgSettingSetBool(MSG_SETTING_NOTIFICATION, generalOpt.bNotification) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_NOTIFICATION);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MSG_SETTING_VIBRATION, &bValue);
	if (bValue != generalOpt.bVibration) {
		if (MsgSettingSetBool(MSG_SETTING_VIBRATION, generalOpt.bVibration) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_VIBRATION);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MSG_SETTING_PREVIEW, &bValue);
	if (bValue != generalOpt.bPreview) {
		if (MsgSettingSetBool(MSG_SETTING_PREVIEW, generalOpt.bPreview) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_PREVIEW);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE);
	if (iValue != generalOpt.ringtoneType) {
		if (MsgSettingSetInt(MSG_SETTING_RINGTONE_TYPE, generalOpt.ringtoneType) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_RINGTONE_TYPE);
			return MSG_ERR_SET_SETTING;
		}
	}

	if (generalOpt.ringtoneType == MSG_RINGTONE_TYPE_SILENT) {
		if (MsgSettingSetString(MSG_SETTING_RINGTONE_PATH, "") != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_RINGTONE_PATH);
			return MSG_ERR_SET_SETTING;
		}
	} else {
		strValue = MsgSettingGetString(MSG_SETTING_RINGTONE_PATH);
		MSG_DEBUG("strValue=[%s], ringtone=[%s]", strValue, generalOpt.ringtonePath);

		if (g_strcmp0(strValue, generalOpt.ringtonePath) != 0) {
			if (MsgSettingSetString(MSG_SETTING_RINGTONE_PATH, generalOpt.ringtonePath) != MSG_SUCCESS) {
				MSG_DEBUG("Error to set config data [%s]", MSG_SETTING_RINGTONE_PATH);
				return MSG_ERR_SET_SETTING;
			}
		}
	}

	if (strValue) {
		free(strValue);
		strValue = NULL;
	}

	iValue = MsgSettingGetInt(MSG_ALERT_REP_TYPE);
	if (iValue != (int)generalOpt.alertTone) {
		if (MsgSettingSetInt(MSG_ALERT_REP_TYPE, (int)generalOpt.alertTone) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_ALERT_REP_TYPE);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSetSMSSendOpt(const MSG_SETTING_S *pSetting)
{
	MSG_SMS_SENDOPT_S sendOpt;
	int iValue = 0;
	bool bValue = false;

	memcpy(&sendOpt, &(pSetting->option.smsSendOpt), sizeof(MSG_SMS_SENDOPT_S));

	iValue = MsgSettingGetInt(SMS_SEND_DCS);
	if (iValue != (int)sendOpt.dcs) {
		if (MsgSettingSetInt(SMS_SEND_DCS, (int)sendOpt.dcs) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", SMS_SEND_DCS);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(SMS_SEND_NETWORK_MODE);
	if (iValue != (int)sendOpt.netMode) {
		if (MsgSettingSetInt(SMS_SEND_NETWORK_MODE, (int)sendOpt.netMode) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", SMS_SEND_NETWORK_MODE);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(SMS_SEND_REPLY_PATH, &bValue);
	if (bValue != sendOpt.bReplyPath) {
		if (MsgSettingSetBool(SMS_SEND_REPLY_PATH, sendOpt.bReplyPath) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", SMS_SEND_REPLY_PATH);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &bValue);
	if (bValue != sendOpt.bDeliveryReport) {
		if (MsgSettingSetBool(SMS_SEND_DELIVERY_REPORT, sendOpt.bDeliveryReport) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", SMS_SEND_DELIVERY_REPORT);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(SMS_SEND_SAVE_STORAGE);
	if (iValue != (int)sendOpt.saveStorage) {
		if (MsgSettingSetInt(SMS_SEND_SAVE_STORAGE, (int)sendOpt.saveStorage) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", SMS_SEND_SAVE_STORAGE);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSetSMSCList(const MSG_SETTING_S *pSetting, bool bSetSim)
{
	msg_error_t err = MSG_SUCCESS;

	int addrLen = 0;
	int index = 0;

	MSG_SMSC_LIST_S smscList = {0,};
	memcpy(&smscList, &(pSetting->option.smscList), sizeof(MSG_SMSC_LIST_S));

//	int sel_id = smscList.selected;

	index = smscList.index;

	if (index < 0 || index >= smscList.totalCnt) {
		MSG_DEBUG("Update SMSC index is invalid [id=%d]", index);
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pSetting->option.smscList.smscData[index].smscAddr.address[0] == '+')
		addrLen = strlen(pSetting->option.smscList.smscData[index].smscAddr.address) - 1;
	else
		addrLen = strlen(pSetting->option.smscList.smscData[index].smscAddr.address);

	if(addrLen > SMSC_ADDR_MAX) {
		MSG_DEBUG("SMSC address is too long [%d]", strlen(pSetting->option.smscList.smscData[index].smscAddr.address));
		return MSG_ERR_SET_SIM_SET;
	} else if(addrLen < 2) {
		MSG_DEBUG("SMSC address is too short [%d]", addrLen);
		return MSG_ERR_SET_SIM_SET;
	}

	if (pSetting->option.smscList.simIndex == 0) {
		MSG_DEBUG("SIM Index for Setting SMSC List = 0");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (bSetSim == true) {
		err = MsgSetConfigInSim(pSetting);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data in sim [%d]", err);
			return err;
		}
	}

	return err;
}


msg_error_t MsgSetMMSSendOpt(const MSG_SETTING_S *pSetting)
{
	MSG_MMS_SENDOPT_S sendOpt;
	int iValue = 0;
	bool bValue = false;

	memcpy(&sendOpt, &(pSetting->option.mmsSendOpt), sizeof(MSG_MMS_SENDOPT_S));

	iValue = MsgSettingGetInt(MMS_SEND_MSG_CLASS);
	if (iValue != (int)sendOpt.msgClass) {
		if (MsgSettingSetInt(MMS_SEND_MSG_CLASS, (int)sendOpt.msgClass) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_MSG_CLASS);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_PRIORITY);
	if (iValue != (int)sendOpt.priority) {
		if (MsgSettingSetInt(MMS_SEND_PRIORITY, (int)sendOpt.priority) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_PRIORITY);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);
	if (iValue != (int)sendOpt.expiryTime) {
		if (MsgSettingSetInt(MMS_SEND_EXPIRY_TIME, (int)sendOpt.expiryTime) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_EXPIRY_TIME);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);
	if (iValue != (int)sendOpt.deliveryTime) {
		if (MsgSettingSetInt(MMS_SEND_DELIVERY_TIME, (int)sendOpt.deliveryTime) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_DELIVERY_TIME);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY);
	if (iValue != (int)sendOpt.customDeliveryTime) {
		if (MsgSettingSetInt(MMS_SEND_CUSTOM_DELIVERY, sendOpt.customDeliveryTime) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_CUSTOM_DELIVERY);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_SEND_SENDER_VISIBILITY, &bValue);
	if (bValue != sendOpt.bSenderVisibility) {
		if (MsgSettingSetBool(MMS_SEND_SENDER_VISIBILITY, sendOpt.bSenderVisibility) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_SENDER_VISIBILITY);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &bValue);
	if (bValue != sendOpt.bDeliveryReport) {
		if (MsgSettingSetBool(MMS_SEND_DELIVERY_REPORT, sendOpt.bDeliveryReport) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_DELIVERY_REPORT);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_SEND_READ_REPLY, &bValue);
	if (bValue != sendOpt.bReadReply) {
		if (MsgSettingSetBool(MMS_SEND_READ_REPLY, sendOpt.bReadReply) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_READ_REPLY);
			return MSG_ERR_SET_SETTING;
		}
	}
#ifdef	__NOT_USED_BY_DESIGN_CHANGE__
	if (MsgSettingSetBool(MMS_SEND_KEEP_COPY, sendOpt.bKeepCopy) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", MMS_SEND_KEEP_COPY);
		return MSG_ERR_SET_SETTING;
	}
#endif	/* __NOT_USED_BY_DESIGN_CHANGE__ */

	MsgSettingGetBool(MMS_SEND_BODY_REPLYING, &bValue);
	if (bValue != sendOpt.bBodyReplying) {
		if (MsgSettingSetBool(MMS_SEND_BODY_REPLYING, sendOpt.bBodyReplying) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_BODY_REPLYING);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_SEND_HIDE_RECIPIENTS, &bValue);
	if (bValue != sendOpt.bHideRecipients) {
		if (MsgSettingSetBool(MMS_SEND_HIDE_RECIPIENTS, sendOpt.bHideRecipients) != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_HIDE_RECIPIENTS);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING);
	if (iValue != sendOpt.replyCharging) {
		if (MsgSettingSetInt(MMS_SEND_REPLY_CHARGING, sendOpt.replyCharging) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_REPLY_CHARGING);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING_DEADLINE);
	if (iValue != (int)sendOpt.replyChargingDeadline) {
		if (MsgSettingSetInt(MMS_SEND_REPLY_CHARGING_DEADLINE, sendOpt.replyChargingDeadline) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_REPLY_CHARGING_DEADLINE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING_SIZE);
	if (iValue != (int)sendOpt.replyChargingSize) {
		if (MsgSettingSetInt(MMS_SEND_REPLY_CHARGING_SIZE, sendOpt.replyChargingSize) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_REPLY_CHARGING_SIZE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_SEND_CREATION_MODE);
	if (iValue != sendOpt.creationMode) {
		if (MsgSettingSetInt(MMS_SEND_CREATION_MODE, sendOpt.creationMode) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_SEND_CREATION_MODE);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSetMMSRecvOpt(const MSG_SETTING_S *pSetting)
{
	MSG_MMS_RECVOPT_S recvOpt;
	int iValue = 0;
	bool bValue = false;

	memcpy(&recvOpt, &(pSetting->option.mmsRecvOpt), sizeof(MSG_MMS_RECVOPT_S));

	iValue = MsgSettingGetInt(MMS_RECV_HOME_NETWORK);
	if (iValue != (int)recvOpt.homeNetwork) {
		if (MsgSettingSetInt(MMS_RECV_HOME_NETWORK, (int)recvOpt.homeNetwork) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_HOME_NETWORK);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK);
	if (iValue != (int)recvOpt.abroadNetwok) {
		if (MsgSettingSetInt(MMS_RECV_ABROAD_NETWORK, (int)recvOpt.abroadNetwok) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_ABROAD_NETWORK);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_RECV_READ_RECEIPT, &bValue);
	if (bValue != recvOpt.readReceipt) {
		if (MsgSettingSetBool(MMS_RECV_READ_RECEIPT, recvOpt.readReceipt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_READ_RECEIPT);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_RECV_DELIVERY_RECEIPT, &bValue);
	if (bValue != recvOpt.bDeliveryReceipt) {
		if (MsgSettingSetBool(MMS_RECV_DELIVERY_RECEIPT, recvOpt.bDeliveryReceipt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_DELIVERY_RECEIPT);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_RECV_REJECT_UNKNOWN, &bValue);
	if (bValue != recvOpt.bRejectUnknown) {
		if (MsgSettingSetBool(MMS_RECV_REJECT_UNKNOWN, recvOpt.bRejectUnknown) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_REJECT_UNKNOWN);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_RECV_REJECT_ADVERTISE, &bValue);
	if (bValue != recvOpt.bRejectAdvertisement) {
		if (MsgSettingSetBool(MMS_RECV_REJECT_ADVERTISE, recvOpt.bRejectAdvertisement) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_RECV_REJECT_ADVERTISE);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSetMMSStyleOpt(const MSG_SETTING_S *pSetting)
{
	MSG_MMS_STYLEOPT_S styleOpt;
	int iValue = 0;
	bool bValue = false;

	memcpy(&styleOpt, &(pSetting->option.mmsStyleOpt), sizeof(MSG_MMS_STYLEOPT_S));

	iValue = MsgSettingGetInt(MMS_STYLE_FONT_SIZE);
	if (iValue != (int)styleOpt.fontSize) {
		if (MsgSettingSetInt(MMS_STYLE_FONT_SIZE, styleOpt.fontSize) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_SIZE);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_BOLD, &bValue);
	if (bValue != styleOpt.bFontStyleBold) {
		if (MsgSettingSetBool(MMS_STYLE_FONT_STYLE_BOLD, styleOpt.bFontStyleBold) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_STYLE_BOLD);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_ITALIC, &bValue);
	if (bValue != styleOpt.bFontStyleItalic) {
		if (MsgSettingSetBool(MMS_STYLE_FONT_STYLE_ITALIC, styleOpt.bFontStyleItalic) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_STYLE_ITALIC);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_UNDERLINE, &bValue);
	if (bValue != styleOpt.bFontStyleUnderline) {
		if (MsgSettingSetBool(MMS_STYLE_FONT_STYLE_UNDERLINE, styleOpt.bFontStyleUnderline) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_STYLE_UNDERLINE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_RED);
	if (iValue != (int)styleOpt.fontColorRed) {
		if (MsgSettingSetInt(MMS_STYLE_FONT_COLOR_RED, styleOpt.fontColorRed) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_COLOR_RED);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_GREEN);
	if (iValue != (int)styleOpt.fontColorGreen) {
		if (MsgSettingSetInt(MMS_STYLE_FONT_COLOR_GREEN, styleOpt.fontColorGreen) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_COLOR_GREEN);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_BLUE);
	if (iValue != (int)styleOpt.fontColorBlue) {
		if (MsgSettingSetInt(MMS_STYLE_FONT_COLOR_BLUE, styleOpt.fontColorBlue) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_COLOR_BLUE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_HUE);
	if (iValue != (int)styleOpt.fontColorHue) {
		if (MsgSettingSetInt(MMS_STYLE_FONT_COLOR_HUE, styleOpt.fontColorHue) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_FONT_COLOR_HUE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_RED);
	if (iValue != (int)styleOpt.bgColorRed) {
		if (MsgSettingSetInt(MMS_STYLE_BG_COLOR_RED, styleOpt.bgColorRed) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_BG_COLOR_RED);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_GREEN);
	if (iValue != (int)styleOpt.bgColorGreen) {
		if (MsgSettingSetInt(MMS_STYLE_BG_COLOR_GREEN, styleOpt.bgColorGreen) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_BG_COLOR_GREEN);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_BLUE);
	if (iValue != (int)styleOpt.bgColorBlue) {
		if (MsgSettingSetInt(MMS_STYLE_BG_COLOR_BLUE, styleOpt.bgColorBlue) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_BG_COLOR_BLUE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_HUE);
	if (iValue != (int)styleOpt.bgColorHue) {
		if (MsgSettingSetInt(MMS_STYLE_BG_COLOR_HUE, styleOpt.bgColorHue) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_BG_COLOR_HUE);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_PAGE_DUR);
	if (iValue != (int)styleOpt.pageDur) {
		if (MsgSettingSetInt(MMS_STYLE_PAGE_DUR, styleOpt.pageDur) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_PAGE_DUR);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_PAGE_CUSTOM_DUR);
	if (iValue != (int)styleOpt.pageCustomDur) {
		if (MsgSettingSetInt(MMS_STYLE_PAGE_CUSTOM_DUR, styleOpt.pageCustomDur) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_PAGE_CUSTOM_DUR);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(MMS_STYLE_PAGE_DUR_MANUAL);
	if (iValue != (int)styleOpt.pageDurManual) {
		if (MsgSettingSetInt(MMS_STYLE_PAGE_DUR_MANUAL, styleOpt.pageDurManual) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MMS_STYLE_PAGE_DUR_MANUAL);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}

msg_error_t MsgSetPushMsgOpt(const MSG_SETTING_S *pSetting)
{
	MSG_PUSHMSG_OPT_S pushOpt;
	int iValue = 0;
	bool bValue = false;

	memcpy(&pushOpt, &(pSetting->option.pushMsgOpt), sizeof(MSG_PUSHMSG_OPT_S));

	MsgSettingGetBool(PUSH_RECV_OPTION, &bValue);
	if (bValue != pushOpt.bReceive) {
		if (MsgSettingSetBool(PUSH_RECV_OPTION, pushOpt.bReceive) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", PUSH_RECV_OPTION);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(PUSH_SERVICE_TYPE);
	if (iValue != (int)pushOpt.serviceType) {
		if (MsgSettingSetInt(PUSH_SERVICE_TYPE, (int)pushOpt.serviceType) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", PUSH_SERVICE_TYPE);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgSetCBMsgOpt(const MSG_SETTING_S *pSetting, bool bSetSim)
{
	msg_error_t err = MSG_SUCCESS;

	MSG_CBMSG_OPT_S cbOpt;
	int iValue = 0;
	bool bValue = false;
	char keyName[MAX_VCONFKEY_NAME_LEN];
	msg_sim_slot_id_t simIndex;

	memcpy(&cbOpt, &(pSetting->option.cbMsgOpt), sizeof(MSG_CBMSG_OPT_S));

	simIndex = cbOpt.simIndex;

	MSG_DEBUG("SIM Index = [%d]", simIndex);

	if (bSetSim == true) {//if (bSetSim == true && simIndex != 0) {
#ifndef FEATURE_SMS_CDMA
		if (simIndex != 0) {
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, simIndex);
			cbOpt.maxSimCnt = MsgSettingGetInt(keyName);

			if (cbOpt.channelData.channelCnt > cbOpt.maxSimCnt) {
				MSG_DEBUG("Channel Count [%d] is over Max SIM Count [%d]", cbOpt.channelData.channelCnt,cbOpt.maxSimCnt);
				return MSG_ERR_SET_SIM_SET;
			}
		}
#endif
		err = MsgSetConfigInSim(pSetting);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data in sim [%d]", err);
			return err;
		}
	}

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, simIndex);
	MsgSettingGetBool(keyName, &bValue);
	if (bValue != cbOpt.bReceive) {
		if (MsgSettingSetBool(keyName, cbOpt.bReceive) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", keyName);
			return MSG_ERR_SET_SETTING;
		}
	}

#ifndef FEATURE_SMS_CDMA
	if (simIndex == 0) {
		MSG_DEBUG("SIM Index for Setting CB Option = 0, Setting for CB_RECEIVE success");
		return MSG_SUCCESS;
	}

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, simIndex);
	iValue = MsgSettingGetInt(keyName);
	if (iValue != cbOpt.maxSimCnt) {
		if (MsgSettingSetInt(keyName, cbOpt.maxSimCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", keyName);
			return MSG_ERR_SET_SETTING;
		}
	}
#endif

	MsgDbHandler *dbHandle = getDbHandle();

#ifdef FEATURE_SMS_CDMA
	err = MsgStoAddCBChannelInfo(dbHandle, &cbOpt.channelData);
#else
	err = MsgStoAddCBChannelInfo(dbHandle, &cbOpt.channelData, simIndex);
#endif
	MSG_DEBUG("MsgStoAddCBChannelInfo : err=[%d]", err);

#ifndef FEATURE_SMS_CDMA
	if (bSetSim == true)
	{
		for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++)
		{
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_LANGUAGE, i);

			if (MsgSettingSetBool(keyName, cbOpt.bLanguage[i]) != MSG_SUCCESS)
			{
				MSG_DEBUG("Error to set config data [%s]", keyName);
				return MSG_ERR_SET_SETTING;
			}
		}
	}
#endif

	return err;
}


msg_error_t MsgSetVoiceMailOpt(const MSG_SETTING_S *pSetting, bool bSetSim)
{
	MSG_VOICEMAIL_OPT_S voiceMailOpt;
	char *pValue = NULL;
	char keyName[DEF_BUF_LEN];
	msg_sim_slot_id_t simIndex = 0;
	msg_error_t err = MSG_SUCCESS;

	memcpy(&voiceMailOpt, &(pSetting->option.voiceMailOpt), sizeof(MSG_VOICEMAIL_OPT_S));

	simIndex = pSetting->option.voiceMailOpt.simIndex;

	if (simIndex == 0) {
		MSG_DEBUG("SIM Index for Setting Voicemail Option = 0");
		return MSG_ERR_INVALID_PARAMETER;
	}

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, simIndex);

	pValue = MsgSettingGetString(keyName);

	if (pValue != NULL && strcmp(pValue, voiceMailOpt.mailNumber) == 0) {
		/* Value is same with previous one. Therefore, we don't need to save it. */
	} else {
		if (bSetSim == true) {
			err = MsgSetConfigInSim(pSetting);
			/* Even if err is not Success, no need to return error. */
			MSG_DEBUG("MsgSetConfigInSim return [%d]", err);
		}

		if (err != MSG_SUCCESS) {
			goto _END_OF_SET_VOICE_OPT;
		}

		err = MsgSettingSetString(keyName, voiceMailOpt.mailNumber);
		if (err != MSG_SUCCESS)
			MSG_ERR("Error to set config data [%s]", keyName);
	}

_END_OF_SET_VOICE_OPT:
	if (pValue != NULL) {
		free(pValue);
		pValue = NULL;
	}

	return err;
}


msg_error_t MsgSetMsgSizeOpt(const MSG_SETTING_S *pSetting)
{
	MSG_MSGSIZE_OPT_S msgSizeOpt;
	int iValue = 0;

	memcpy(&msgSizeOpt, &(pSetting->option.msgSizeOpt), sizeof(MSG_MSGSIZE_OPT_S));

	iValue = MsgSettingGetInt(MSGSIZE_OPTION);
	if (iValue != msgSizeOpt.nMsgSize) {
		if (MsgSettingSetInt(MSGSIZE_OPTION, msgSizeOpt.nMsgSize) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSGSIZE_OPTION);
			return MSG_ERR_SET_SETTING;
		}
	}

	return MSG_SUCCESS;
}


void MsgGetGeneralOpt(MSG_SETTING_S *pSetting)
{
	char *tmpValue = NULL;

	memset(&(pSetting->option.generalOpt), 0x00, sizeof(MSG_GENERAL_OPT_S));

	MsgSettingGetBool(MSG_KEEP_COPY, &pSetting->option.generalOpt.bKeepCopy);

	MsgSettingGetBool(MSG_BLOCK_UNKNOWN_MSG, &pSetting->option.generalOpt.bBlockUnknownMsg);

	pSetting->option.generalOpt.smsLimitCnt = MsgSettingGetInt(MSG_SMS_LIMIT);

	pSetting->option.generalOpt.mmsLimitCnt = MsgSettingGetInt(MSG_MMS_LIMIT);

	MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &pSetting->option.generalOpt.bNotification);

	MsgSettingGetBool(MSG_SETTING_VIBRATION, &pSetting->option.generalOpt.bVibration);

	MsgSettingGetBool(MSG_SETTING_PREVIEW, &pSetting->option.generalOpt.bPreview);

	MsgSettingGetBool(MSG_AUTO_ERASE, &pSetting->option.generalOpt.bAutoErase);

	pSetting->option.generalOpt.ringtoneType = MsgSettingGetInt(MSG_SETTING_RINGTONE_TYPE);

	tmpValue = MsgSettingGetString(MSG_SETTING_RINGTONE_PATH);
	if (tmpValue != NULL) {
		strncpy(pSetting->option.generalOpt.ringtonePath, tmpValue, MSG_FILEPATH_LEN_MAX);
		free(tmpValue);
		tmpValue = NULL;
	}

	pSetting->option.generalOpt.alertTone = (MSG_ALERT_TONE_T)MsgSettingGetInt(MSG_ALERT_REP_TYPE);
}


void MsgGetSMSSendOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.smsSendOpt), 0x00, sizeof(MSG_SMS_SENDOPT_S));

	pSetting->option.smsSendOpt.dcs = (msg_encode_type_t)MsgSettingGetInt(SMS_SEND_DCS);

	pSetting->option.smsSendOpt.netMode = (MSG_SMS_NETWORK_MODE_T)MsgSettingGetInt(SMS_SEND_NETWORK_MODE);

	MsgSettingGetBool(SMS_SEND_REPLY_PATH, &pSetting->option.smsSendOpt.bReplyPath);

	MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &pSetting->option.smsSendOpt.bDeliveryReport);

	pSetting->option.smsSendOpt.saveStorage = (MSG_SMS_SAVE_STORAGE_T)MsgSettingGetInt(SMS_SEND_SAVE_STORAGE);
}


void MsgGetSMSCList(MSG_SETTING_S *pSetting)
{
	MSG_BEGIN();

	MsgGetConfigInSim(pSetting);

	MSG_END();
}


void MsgGetMMSSendOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.mmsSendOpt), 0x00, sizeof(MSG_MMS_SENDOPT_S));

	pSetting->option.mmsSendOpt.msgClass = (MSG_MMS_MSG_CLASS_TYPE_T)MsgSettingGetInt(MMS_SEND_MSG_CLASS);

	pSetting->option.mmsSendOpt.priority = (msg_priority_type_t)MsgSettingGetInt(MMS_SEND_PRIORITY);

	pSetting->option.mmsSendOpt.expiryTime = (MSG_MMS_EXPIRY_TIME_T)MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);

	pSetting->option.mmsSendOpt.deliveryTime = (MSG_MMS_DELIVERY_TIME_T)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);

	pSetting->option.mmsSendOpt.customDeliveryTime = MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY);

	MsgSettingGetBool(MMS_SEND_SENDER_VISIBILITY, &pSetting->option.mmsSendOpt.bSenderVisibility);

	MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &pSetting->option.mmsSendOpt.bDeliveryReport);

	MsgSettingGetBool(MMS_SEND_READ_REPLY, &pSetting->option.mmsSendOpt.bReadReply);

#ifdef	__NOT_USED_BY_DESIGN_CHANGE__
	MsgSettingGetBool(MMS_SEND_KEEP_COPY, &pSetting->option.mmsSendOpt.bKeepCopy);
#endif	/* __NOT_USED_BY_DESIGN_CHANGE__ */

	MsgSettingGetBool(MMS_SEND_BODY_REPLYING, &pSetting->option.mmsSendOpt.bBodyReplying);

	MsgSettingGetBool(MMS_SEND_HIDE_RECIPIENTS, &pSetting->option.mmsSendOpt.bHideRecipients);

	pSetting->option.mmsSendOpt.replyCharging = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING);

	pSetting->option.mmsSendOpt.replyChargingDeadline = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING_DEADLINE);

	pSetting->option.mmsSendOpt.replyChargingSize = MsgSettingGetInt(MMS_SEND_REPLY_CHARGING_SIZE);

	pSetting->option.mmsSendOpt.creationMode = MsgSettingGetInt(MMS_SEND_CREATION_MODE);
}


void MsgGetMMSRecvOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.mmsRecvOpt), 0x00, sizeof(MSG_MMS_RECVOPT_S));

	pSetting->option.mmsRecvOpt.homeNetwork = (MSG_MMS_HOME_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_HOME_NETWORK);

	pSetting->option.mmsRecvOpt.abroadNetwok = (MSG_MMS_ABROAD_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK);

	MsgSettingGetBool(MMS_RECV_READ_RECEIPT, &pSetting->option.mmsRecvOpt.readReceipt);

	MsgSettingGetBool(MMS_RECV_DELIVERY_RECEIPT, &pSetting->option.mmsRecvOpt.bDeliveryReceipt);

	MsgSettingGetBool(MMS_RECV_REJECT_UNKNOWN, &pSetting->option.mmsRecvOpt.bRejectUnknown);

	MsgSettingGetBool(MMS_RECV_REJECT_ADVERTISE, &pSetting->option.mmsRecvOpt.bRejectAdvertisement);
}


void MsgGetMMSStyleOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.mmsStyleOpt), 0x00, sizeof(MSG_MMS_STYLEOPT_S));

	pSetting->option.mmsStyleOpt.fontSize = MsgSettingGetInt(MMS_STYLE_FONT_SIZE);

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_BOLD, &pSetting->option.mmsStyleOpt.bFontStyleBold);

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_ITALIC, &pSetting->option.mmsStyleOpt.bFontStyleItalic);

	MsgSettingGetBool(MMS_STYLE_FONT_STYLE_UNDERLINE, &pSetting->option.mmsStyleOpt.bFontStyleUnderline);

	pSetting->option.mmsStyleOpt.fontColorRed = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_RED);

	pSetting->option.mmsStyleOpt.fontColorGreen = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_GREEN);

	pSetting->option.mmsStyleOpt.fontColorBlue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_BLUE);

	pSetting->option.mmsStyleOpt.fontColorHue = MsgSettingGetInt(MMS_STYLE_FONT_COLOR_HUE);

	pSetting->option.mmsStyleOpt.bgColorRed = MsgSettingGetInt(MMS_STYLE_BG_COLOR_RED);

	pSetting->option.mmsStyleOpt.bgColorGreen = MsgSettingGetInt(MMS_STYLE_BG_COLOR_GREEN);

	pSetting->option.mmsStyleOpt.bgColorBlue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_BLUE);

	pSetting->option.mmsStyleOpt.bgColorHue = MsgSettingGetInt(MMS_STYLE_BG_COLOR_HUE);

	pSetting->option.mmsStyleOpt.pageDur = MsgSettingGetInt(MMS_STYLE_PAGE_DUR);

	pSetting->option.mmsStyleOpt.pageCustomDur = MsgSettingGetInt(MMS_STYLE_PAGE_CUSTOM_DUR);

	pSetting->option.mmsStyleOpt.pageDurManual = MsgSettingGetInt(MMS_STYLE_PAGE_DUR_MANUAL);
}


void MsgGetPushMsgOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.pushMsgOpt), 0x00, sizeof(MSG_PUSHMSG_OPT_S));

	MsgSettingGetBool(PUSH_RECV_OPTION, &pSetting->option.pushMsgOpt.bReceive);

	pSetting->option.pushMsgOpt.serviceType = (MSG_PUSH_SERVICE_TYPE_T)MsgSettingGetInt(PUSH_SERVICE_TYPE);
}


void MsgGetCBMsgOpt(MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;

	char keyName[DEF_BUF_LEN] = {0, };
	MsgDbHandler *dbHandle = getDbHandle();
	msg_sim_slot_id_t simIndex = pSetting->option.cbMsgOpt.simIndex;

	memset(&(pSetting->option.cbMsgOpt), 0x00, sizeof(MSG_CBMSG_OPT_S));

	MSG_DEBUG("Sim index = [%d]", simIndex);

	/* Keep simIndex */
	pSetting->option.cbMsgOpt.simIndex = simIndex;

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, simIndex);
	MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bReceive);

	if (simIndex == 0) {
		MSG_DEBUG("SIM Index = 0, bReceive is gotten");
		return;
	}

#ifndef FEATURE_SMS_CDMA
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_MAX_SIM_COUNT, simIndex);
	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(keyName);
#endif

#ifdef FEATURE_SMS_CDMA
	err = MsgStoGetCBChannelInfo(dbHandle, &pSetting->option.cbMsgOpt.channelData);
#else
	err = MsgStoGetCBChannelInfo(dbHandle, &pSetting->option.cbMsgOpt.channelData, simIndex);
#endif
	if (err != MSG_SUCCESS)
		MSG_ERR("MsgStoGetCBChannelInfo : err=[%d]", err);

#ifndef FEATURE_SMS_CDMA
	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_LANGUAGE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bLanguage[i]);
	}
#endif
}


void MsgGetVoiceMailOpt(MSG_SETTING_S *pSetting)
{
	char *tmpValue = NULL;
	char keyName[DEF_BUF_LEN];
	msg_sim_slot_id_t simIndex;

	simIndex = pSetting->option.voiceMailOpt.simIndex;
	MSG_DEBUG("sim index = [%d]", simIndex);

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, simIndex);
	tmpValue = MsgSettingGetString(keyName);
	memset(pSetting->option.voiceMailOpt.mailNumber, 0x00, sizeof(pSetting->option.voiceMailOpt.mailNumber));
	if (tmpValue != NULL) {
		strncpy(pSetting->option.voiceMailOpt.mailNumber, tmpValue, MAX_PHONE_NUMBER_LEN);
		MSG_SEC_DEBUG("Voicemail number = [%s]", pSetting->option.voiceMailOpt.mailNumber);
		free(tmpValue);
		tmpValue = NULL;
	}

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, simIndex);
	tmpValue = MsgSettingGetString(keyName);
	memset(pSetting->option.voiceMailOpt.alpahId, 0x00, sizeof(pSetting->option.voiceMailOpt.alpahId));
	if (tmpValue != NULL) {
		strncpy(pSetting->option.voiceMailOpt.alpahId, tmpValue, MAX_SIM_XDN_ALPHA_ID_LEN);
		MSG_SEC_DEBUG("Voicemail alpha ID = [%s]", pSetting->option.voiceMailOpt.alpahId);
		free(tmpValue);
		tmpValue = NULL;
	}

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, simIndex);
	pSetting->option.voiceMailOpt.voiceCnt = MsgSettingGetInt(keyName);
}


void MsgGetMsgSizeOpt(MSG_SETTING_S *pSetting)
{
	memset(&(pSetting->option.msgSizeOpt), 0x00, sizeof(MSG_MSGSIZE_OPT_S));

	pSetting->option.msgSizeOpt.nMsgSize = MsgSettingGetInt(MSGSIZE_OPTION);
}


msg_error_t MsgSetConfigInSim(const MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;

	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	// Get Setting Data from SIM
	if (plg != NULL)
		err = plg->setConfigData(pSetting);
	else
		err = MSG_ERR_NULL_POINTER;

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Error. Error code is %d.", err);
		return err;
	}

	return err;
}


msg_error_t MsgGetConfigInSim(MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;

	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	// Get Setting Data from SIM
	if (plg != NULL)
		err = plg->getConfigData(pSetting);
	else
		err = MSG_ERR_NULL_POINTER;

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Error. Error code is %d.", err);
		return err;
	}

	return err;
}
