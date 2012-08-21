/*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
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


#define DEF_BUF_LEN	128

/*==================================================================================================
								STATIC FUNCTION PROTOTYPES
==================================================================================================*/
static char* msg_clean_country_code(char *src);
static char* msg_normalize_number(char *src);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgInitSimConfig(MSG_SIM_STATUS_T SimStatus)
{
	MSG_DEBUG("Start to initialize SIM Configuration");

	msg_error_t err = MSG_SUCCESS;

	if (SimStatus != MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
		MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

		if (plg == NULL)
		{
			MSG_DEBUG("No plugin for %d type", mainType);
			return MSG_ERR_INVALID_PLUGIN_HANDLE;
		}

		// Check SIM Status
		MSG_DEBUG(" ** SIM is available - status : [%d] ** ", SimStatus);

		err = plg->initConfigData(SimStatus);
	}

	return err;
}


msg_error_t MsgSetConfigData(const MSG_SETTING_S *pSetting)
{
	msg_error_t err = MSG_SUCCESS;

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
		case MSG_SMSC_LIST :
			err = MsgSetSMSCList(pSetting, true);
			break;
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
			err = MsgSetCBMsgOpt(pSetting, true);
			break;
		case MSG_VOICEMAIL_OPT :
			err = MsgSetVoiceMailOpt(pSetting);
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

	// Check SIM is present or not
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(MSG_SIM_CHANGED);

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
			if (simStatus == MSG_SIM_STATUS_NOT_FOUND) {
				MSG_DEBUG("SIM is not present..");
				return MSG_ERR_NO_SIM;
			}
			MsgGetCBMsgOpt(pSetting);
		}
		break;
		case MSG_VOICEMAIL_OPT :
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

	for (int index = 0; index < pSetting->option.smscList.totalCnt; index++)
	{
		if(strlen(pSetting->option.smscList.smscData[index].smscAddr.address) > SMSC_ADDR_MAX)
		{
			MSG_DEBUG("SMSC address is too long [%d]", strlen(pSetting->option.smscList.smscData[index].smscAddr.address));
			return MSG_ERR_SET_SIM_SET;
		}
	}

	if (bSetSim == true)
	{
		err = MsgSetConfigInSim(pSetting);

		if (err != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data in sim [%d]", err);
			return err;
		}
	}

	MSG_SMSC_LIST_S smscList;

	memcpy(&smscList, &(pSetting->option.smscList), sizeof(MSG_SMSC_LIST_S));

	char keyName[DEF_BUF_LEN] = {0, };

	// No selected SMSC Info. in SIM.
	if (bSetSim == true)
	{
		if (MsgSettingSetInt(SMSC_SELECTED, smscList.selected) != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data [%s]", SMSC_SELECTED);
			return MSG_ERR_SET_SETTING;
		}
	}

	if (MsgSettingSetInt(SMSC_TOTAL_COUNT, smscList.totalCnt) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SMSC_TOTAL_COUNT);
		return MSG_ERR_SET_SETTING;
	}

	for (int i = 0; i < smscList.totalCnt; i++)
	{
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_PID, i);

		if ((err = MsgSettingSetInt(keyName, (int)smscList.smscData[i].pid)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_VAL_PERIOD, i);

		if ((err = MsgSettingSetInt(keyName, (int)smscList.smscData[i].valPeriod)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_NAME, i);

		if ((err = MsgSettingSetString(keyName, smscList.smscData[i].name)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_TON, i);

		if (smscList.smscData[i].smscAddr.address[0] == '+')
			smscList.smscData[i].smscAddr.ton = MSG_TON_INTERNATIONAL;
		else
			smscList.smscData[i].smscAddr.ton = MSG_TON_NATIONAL;

		if ((err = MsgSettingSetInt(keyName, (int)smscList.smscData[i].smscAddr.ton)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_NPI, i);

		smscList.smscData[i].smscAddr.npi = MSG_NPI_ISDN; // app cannot set this value

		if ((err = MsgSettingSetInt(keyName, (int)smscList.smscData[i].smscAddr.npi)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_ADDRESS, i);

		if ((err = MsgSettingSetString(keyName, smscList.smscData[i].smscAddr.address)) != MSG_SUCCESS)
			break;
	}

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", keyName);
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

	memcpy(&cbOpt, &(pSetting->option.cbMsgOpt), sizeof(MSG_CBMSG_OPT_S));

	if (bSetSim == true) {
		cbOpt.maxSimCnt = MsgSettingGetInt(CB_MAX_SIM_COUNT);

		if (cbOpt.channelData.channelCnt > cbOpt.maxSimCnt) {
			MSG_DEBUG("Channel Count is over Max SIM Count [%d]", cbOpt.channelData.channelCnt);
			return MSG_ERR_SET_SIM_SET;
		}

		err = MsgSetConfigInSim(pSetting);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data in sim [%d]", err);
			return err;
		}
	}

	MsgSettingGetBool(CB_RECEIVE, &bValue);
	if (bValue != cbOpt.bReceive) {
		if (MsgSettingSetBool(CB_RECEIVE, cbOpt.bReceive) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", CB_RECEIVE);
			return MSG_ERR_SET_SETTING;
		}
	}

	MsgSettingGetBool(CB_ALL_CHANNEL, &bValue);
	if (bValue != cbOpt.bAllChannel) {
		if (MsgSettingSetBool(CB_ALL_CHANNEL, cbOpt.bAllChannel) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", CB_ALL_CHANNEL);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(CB_MAX_SIM_COUNT);
	if (iValue != cbOpt.maxSimCnt) {
		if (MsgSettingSetInt(CB_MAX_SIM_COUNT, cbOpt.maxSimCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", CB_MAX_SIM_COUNT);
			return MSG_ERR_SET_SETTING;
		}
	}

	iValue = MsgSettingGetInt(CB_CHANNEL_COUNT);
	if (iValue != cbOpt.channelData.channelCnt) {
		if (MsgSettingSetInt(CB_CHANNEL_COUNT, cbOpt.channelData.channelCnt) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", CB_CHANNEL_COUNT);
			return MSG_ERR_SET_SETTING;
		}
	}

	char keyName[DEF_BUF_LEN] = {0, };

	for (int i = 0; i < cbOpt.channelData.channelCnt; i++)
	{
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		if ((err = MsgSettingSetBool(keyName, cbOpt.channelData.channelInfo[i].bActivate)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_ID, i);

		if ((err = MsgSettingSetInt(keyName, cbOpt.channelData.channelInfo[i].id)) != MSG_SUCCESS)
			break;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_NAME, i);

		if ((err = MsgSettingSetString(keyName, cbOpt.channelData.channelInfo[i].name)) != MSG_SUCCESS)
			break;
	}

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

	return err;
}

msg_error_t MsgSetVoiceMailOpt(const MSG_SETTING_S *pSetting)
{
	MSG_VOICEMAIL_OPT_S voiceMailOpt;
	char *pValue = NULL;

	memcpy(&voiceMailOpt, &(pSetting->option.voiceMailOpt), sizeof(MSG_VOICEMAIL_OPT_S));

	pValue = MsgSettingGetString(VOICEMAIL_NUMBER);
	if (pValue != NULL && strcmp(pValue, voiceMailOpt.mailNumber) == 0) {
		/* Value is same with previous one. Therefore, we don't need to save it. */
	} else {
		if (MsgSettingSetString(VOICEMAIL_NUMBER, voiceMailOpt.mailNumber) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", VOICEMAIL_NUMBER);

			if (pValue != NULL) {
				free(pValue);
				pValue = NULL;
			}

			return MSG_ERR_SET_SETTING;
		}
	}

	if (pValue != NULL) {
		free(pValue);
		pValue = NULL;
	}

	return MSG_SUCCESS;
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
	memset(&(pSetting->option.generalOpt), 0x00, sizeof(MSG_GENERAL_OPT_S));

	MsgSettingGetBool(MSG_KEEP_COPY, &pSetting->option.generalOpt.bKeepCopy);
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
	char keyName[DEF_BUF_LEN] = {0, };
	char *tmpValue = NULL;

	memset(&(pSetting->option.smscList), 0x00, sizeof(MSG_SMSC_LIST_S));

	pSetting->option.smscList.selected = MsgSettingGetInt(SMSC_SELECTED);

	pSetting->option.smscList.totalCnt = MsgSettingGetInt(SMSC_TOTAL_COUNT);

	for (int i = 0; i < pSetting->option.smscList.totalCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_PID, i);

		pSetting->option.smscList.smscData[i].pid = (MSG_SMS_PID_T)MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_VAL_PERIOD, i);

		pSetting->option.smscList.smscData[i].valPeriod = (MSG_VAL_PERIOD_T)MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_NAME, i);

		memset(pSetting->option.smscList.smscData[i].name, 0x00, SMSC_NAME_MAX+1);

		tmpValue = MsgSettingGetString(keyName);
		if (tmpValue != NULL) {
			strncpy(pSetting->option.smscList.smscData[i].name, tmpValue, SMSC_NAME_MAX);
			free(tmpValue);
			tmpValue = NULL;
		}

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_TON, i);

		pSetting->option.smscList.smscData[i].smscAddr.ton = (MSG_SMS_TON_T)MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_NPI, i);

		pSetting->option.smscList.smscData[i].smscAddr.npi = (MSG_SMS_NPI_T)MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", SMSC_ADDRESS, i);

		memset(pSetting->option.smscList.smscData[i].smscAddr.address, 0x00, sizeof(pSetting->option.smscList.smscData[i].smscAddr.address));

		tmpValue = MsgSettingGetString(keyName);
		if (tmpValue != NULL) {
			strncpy(pSetting->option.smscList.smscData[i].smscAddr.address, tmpValue, SMSC_ADDR_MAX);
			free(tmpValue);
			tmpValue = NULL;
		}
	}
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
	char keyName[DEF_BUF_LEN] = {0, };
	char *tmpValue = NULL;

	memset(&(pSetting->option.cbMsgOpt), 0x00, sizeof(MSG_CBMSG_OPT_S));

	MsgSettingGetBool(CB_RECEIVE, &pSetting->option.cbMsgOpt.bReceive);

	MsgSettingGetBool(CB_ALL_CHANNEL, &pSetting->option.cbMsgOpt.bAllChannel);

	pSetting->option.cbMsgOpt.maxSimCnt = MsgSettingGetInt(CB_MAX_SIM_COUNT);

	pSetting->option.cbMsgOpt.channelData.channelCnt = MsgSettingGetInt(CB_CHANNEL_COUNT);

	for (int i = 0; i < pSetting->option.cbMsgOpt.channelData.channelCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.channelData.channelInfo[i].bActivate);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_ID, i);

		pSetting->option.cbMsgOpt.channelData.channelInfo[i].id = MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_CHANNEL_NAME, i);

		tmpValue = MsgSettingGetString(keyName);
		if (tmpValue != NULL) {
			strncpy(pSetting->option.cbMsgOpt.channelData.channelInfo[i].name, tmpValue, CB_CHANNEL_NAME_MAX);
			free(tmpValue);
			tmpValue = NULL;
		}
	}

	for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, DEF_BUF_LEN, "%s/%d", CB_LANGUAGE, i);

		MsgSettingGetBool(keyName, &pSetting->option.cbMsgOpt.bLanguage[i]);
	}
}

void MsgGetVoiceMailOpt(MSG_SETTING_S *pSetting)
{
        char *tmpValue = NULL;

        memset(&(pSetting->option.voiceMailOpt), 0x00, sizeof(MSG_VOICEMAIL_OPT_S));

        tmpValue = MsgSettingGetString(VOICEMAIL_NUMBER);
        if (tmpValue != NULL) {
                strncpy(pSetting->option.voiceMailOpt.mailNumber, tmpValue, MAX_PHONE_NUMBER_LEN);
                free(tmpValue);
                tmpValue = NULL;
        }
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

static char* msg_clean_country_code(char *src)
{
	int ret = 1;

	switch (src[ret++]-'0')
	{
		case 1:
		case 7:
			break;
		case 2:
			switch (src[ret++]-'0')
			{
				case 0:
				case 7:
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 3:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 9:
					break;
				case 5:
				case 7:
				case 8:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 4:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					break;
				case 2:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 5:
			switch (src[ret++]-'0')
			{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					break;
				case 0:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 6:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					break;
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 8:
			switch (src[ret++]-'0')
			{
				case 1:
				case 2:
				case 4:
				case 6:
					break;
				case 0:
				case 3:
				case 5:
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 9:
			switch (src[ret++]-'0')
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 8:
					break;
				case 6:
				case 7:
				case 9:
					ret += 1;
					break;
				default:
					MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 0:
		default:
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
			return src;
	}

	return &src[ret];
}

static char* msg_normalize_number(char *src)
{
	char *normalized_number;

	if ('+' == src[0])
		normalized_number = msg_clean_country_code(src);
	else if ('0' == src[0])
		normalized_number = src+1;
	else
		normalized_number = src;

	MSG_DEBUG("src = %s, normalized = %s", src, normalized_number);

	return normalized_number;
}
