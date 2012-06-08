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


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <iostream>
#include <string>
#include <stdlib.h>
using namespace std;

#include "MapiSetting.h"
#include "MsgTestSetting.h"
#include "main.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgTestSettingMain(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return;
	}

	char menu[2];

	do
	{
		system ("clear");

		print("======================================");
		print("============ Setting Menu ============");
		print("======================================");
		print("[1] General Options");
		print("[2] SMS Send Options");
		print("[3] SMSC List");
		print("[4] MMS Send Options");
		print("[5] MMS Recv Options");
		print("[6] MMS Style Options");
		print("[7] Push Msg Options");
		print("[8] CB Msg Options");
		print("[0] Voice Mail Option");
		print("[A] Msg Size Option");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		MsgSelectMenu(hMsgHandle, menu);
	}
	while (strcmp(menu, "B"));
}


void MsgSelectMenu(MSG_HANDLE_T hMsgHandle, char *pMenu)
{
	if (!strcmp(pMenu, "1"))
	{
		MsgTestGeneralOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "2"))
	{
		MsgTestSMSSendOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "3"))
	{
		MsgTestSMSCList(hMsgHandle);
	}
	else if (!strcmp(pMenu, "4"))
	{
		MsgTestMMSSendOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "5"))
	{
		MsgTestMMSRecvOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "6"))
	{
		MsgTestMMSStyleOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "7"))
	{
		MsgTestPushMsgOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "8"))
	{
		MsgTestCBMsgOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "0"))
	{
		MsgTestVoiceMailOpt(hMsgHandle);
	}
	else if (!strcmp(pMenu, "A"))
	{
		MsgTestMsgSizeOpt(hMsgHandle);
	}
}


void MsgTestGeneralOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setting;

	do
	{
		memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_GENERAL_OPT_S));

		setting.type = MSG_GENERAL_OPT;

		err = msg_get_config(hMsgHandle, &setting);

		system ("clear");

		print("======================================");
		print("=========== General Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Keep a Copy : [%d]", setting.option.generalOpt.bKeepCopy);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Alert Tone : [%d]", setting.option.generalOpt.alertTone);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_GENERAL_OPT_S));

			setting.type = MSG_GENERAL_OPT;

			setting.option.generalOpt.bKeepCopy = true;
			setting.option.generalOpt.alertTone = MSG_ALERT_TONE_ONCE;
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}

		err = msg_set_config(hMsgHandle, &setting);

		if (err == MSG_SUCCESS)
			print("Setting Config Data is OK!");
		else
			print("Setting Config Data is failed!");
	}
	while (1);
}


void MsgTestSMSSendOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setting;

	do
	{
		memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_SMS_SENDOPT_S));

		setting.type = MSG_SMS_SENDOPT;

		err = msg_get_config(hMsgHandle, &setting);

		system ("clear");

		print("======================================");
		print("=========== SMS Send Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "DCS : [%d]", setting.option.smsSendOpt.dcs);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Network Selection : [%d]", setting.option.smsSendOpt.netMode);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reply Path : [%d]", setting.option.smsSendOpt.bReplyPath);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Delivery Report : [%d]", setting.option.smsSendOpt.bDeliveryReport);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Save Storage : [%d]", setting.option.smsSendOpt.saveStorage);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_SMS_SENDOPT_S));

			setting.type = MSG_SMS_SENDOPT;

			setting.option.smsSendOpt.dcs = MSG_ENCODE_AUTO;
			setting.option.smsSendOpt.netMode = MSG_SMS_NETWORK_CS_ONLY;
			setting.option.smsSendOpt.bReplyPath = false;
			setting.option.smsSendOpt.bDeliveryReport = true;
			setting.option.smsSendOpt.saveStorage = MSG_SMS_SAVE_STORAGE_PHONE;
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}

		err = msg_set_config(hMsgHandle, &setting);

		if (err == MSG_SUCCESS)
			print("Setting Config Data is OK!");
		else
			print("Setting Config Data is failed!");
	}
	while (1);
}


void MsgTestSMSCList(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setting;

	do
	{
		memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_SMSC_LIST_S));

		setting.type = MSG_SMSC_LIST;

		err = msg_get_config(hMsgHandle, &setting);

		system ("clear");

		print("======================================");
		print("============== SMSC List ==============");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Total Count : [%d]", setting.option.smscList.totalCnt);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Selected SMSC : [%d]", setting.option.smscList.selected);
		print(strPrint);

		for (int i = 0; i < setting.option.smscList.totalCnt; i++)
		{
			print("======================================");

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC Name : [%s]", setting.option.smscList.smscData[i].name);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC Address : [%s]", setting.option.smscList.smscData[i].smscAddr.address);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC PID : [%d]", setting.option.smscList.smscData[i].pid);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC VAL PERIOD : [%d]", setting.option.smscList.smscData[i].valPeriod);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC TON : [%d]", setting.option.smscList.smscData[i].smscAddr.ton);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "SMSC NPI : [%d]", setting.option.smscList.smscData[i].smscAddr.npi);
			print(strPrint);
		}

		print("======================================");
		print("================ Menu ================");
		print("[A] Add New SMSC");
		print("[U] Update SMSC");
		print("[D] Delete SMSC");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "A"))
		{
			continue;
		}
		else if (!strcmp(menu, "U"))
		{
			memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_SMSC_LIST_S));

			setting.type = MSG_SMSC_LIST;

			setting.option.smscList.selected = 0;
			setting.option.smscList.totalCnt = 1;
			setting.option.smscList.smscData[0].pid = MSG_PID_TEXT;
			setting.option.smscList.smscData[0].valPeriod = MSG_VAL_MAXIMUM;
			snprintf(setting.option.smscList.smscData[0].name, SMSC_NAME_MAX, "%s", "SMS Centre 1");

			setting.option.smscList.smscData[0].smscAddr.ton = MSG_TON_INTERNATIONAL;
			setting.option.smscList.smscData[0].smscAddr.npi = MSG_NPI_ISDN;
			snprintf(setting.option.smscList.smscData[0].smscAddr.address, SMSC_ADDR_MAX, "%s", "1111");
		}
		else if (!strcmp(menu, "D"))
		{
			continue;
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}

		err = msg_set_config(hMsgHandle, &setting);

		if (err == MSG_SUCCESS)
			print("Setting Config Data is OK!");
		else
			print("Setting Config Data is failed!");
	}
	while (1);
}


void MsgTestMMSSendOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setting;

	do
	{
		memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_SENDOPT_S));

		setting.type = MSG_MMS_SENDOPT;

		err = msg_get_config(hMsgHandle, &setting);

		system ("clear");

		print("======================================");
		print("=========== MMS Send Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Msg Class : [%d]", setting.option.mmsSendOpt.msgClass);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Priority : [%d]", setting.option.mmsSendOpt.priority);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Expiry Time : [%d]", setting.option.mmsSendOpt.expiryTime);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Delivery Time : [%d]", setting.option.mmsSendOpt.deliveryTime);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Custom Delivery Time : [%d]", setting.option.mmsSendOpt.customDeliveryTime);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Sender Visibility : [%d]", setting.option.mmsSendOpt.bSenderVisibility);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Delivery Report : [%d]", setting.option.mmsSendOpt.bDeliveryReport);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Read Reply : [%d]", setting.option.mmsSendOpt.bReadReply);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Keep Copy : [%d]", setting.option.mmsSendOpt.bKeepCopy);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Body Replying : [%d]", setting.option.mmsSendOpt.bBodyReplying);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Hide Recipients : [%d]", setting.option.mmsSendOpt.bHideRecipients);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reply Charging : [%d]", setting.option.mmsSendOpt.replyCharging);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reply Charging Deadline : [%d]", setting.option.mmsSendOpt.replyChargingDeadline);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reply Charging Size : [%d]", setting.option.mmsSendOpt.replyChargingSize);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Creation Mode : [%d]", setting.option.mmsSendOpt.creationMode);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setting, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_SENDOPT_S));

			setting.type = MSG_MMS_SENDOPT;

			setting.option.mmsSendOpt.msgClass = MSG_CLASS_AUTO;
			setting.option.mmsSendOpt.priority = MSG_MESSAGE_PRIORITY_NORMAL;
			setting.option.mmsSendOpt.expiryTime = MSG_EXPIRY_TIME_1DAY;
			setting.option.mmsSendOpt.deliveryTime = MSG_DELIVERY_TIME_IMMEDIATLY;
			setting.option.mmsSendOpt.customDeliveryTime = 0;
			setting.option.mmsSendOpt.bSenderVisibility = false;
			setting.option.mmsSendOpt.bDeliveryReport = true;
			setting.option.mmsSendOpt.bReadReply = false;
			setting.option.mmsSendOpt.bKeepCopy = false;
			setting.option.mmsSendOpt.bBodyReplying = false;
			setting.option.mmsSendOpt.bHideRecipients = false;
			setting.option.mmsSendOpt.replyCharging = MSG_REPLY_CHARGING_NONE;
			setting.option.mmsSendOpt.replyChargingDeadline = 0;
			setting.option.mmsSendOpt.replyChargingSize = 0;
			setting.option.mmsSendOpt.creationMode = MSG_CREATION_MODE_FREE;
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}

		err = msg_set_config(hMsgHandle, &setting);

		if (err == MSG_SUCCESS)
			print("Setting Config Data is OK!");
		else
			print("Setting Config Data is failed!");
	}
	while (1);
}


void MsgTestMMSRecvOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_RECVOPT_S));

		getOption.type = MSG_MMS_RECVOPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		print("======================================");
		print("=========== MMS Recv Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Home Network : [%d]", getOption.option.mmsRecvOpt.homeNetwork);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Abroad Network : [%d]", getOption.option.mmsRecvOpt.abroadNetwok);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Read Receipt : [%d]", getOption.option.mmsRecvOpt.readReceipt);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Delivery Receipt : [%d]", getOption.option.mmsRecvOpt.bDeliveryReceipt);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reject Unknown : [%d]", getOption.option.mmsRecvOpt.bRejectUnknown);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Reject Advertisement : [%d]", getOption.option.mmsRecvOpt.bRejectAdvertisement);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_RECVOPT_S));

			setOption.type = MSG_MMS_RECVOPT;

			setOption.option.mmsRecvOpt.homeNetwork = MSG_HOME_AUTO_DOWNLOAD;
			setOption.option.mmsRecvOpt.abroadNetwok = MSG_ABROAD_RESTRICTED;
			setOption.option.mmsRecvOpt.readReceipt = false;
			setOption.option.mmsRecvOpt.bDeliveryReceipt = true;
			setOption.option.mmsRecvOpt.bRejectUnknown = false;
			setOption.option.mmsRecvOpt.bRejectAdvertisement = false;

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}
	while (1);
}


void MsgTestMMSStyleOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_STYLEOPT_S));

		getOption.type = MSG_MMS_STYLEOPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		print("======================================");
		print("=========== MMS Style Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Size : [%d]", getOption.option.mmsStyleOpt.fontSize);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Style (Bold) : [%d]", getOption.option.mmsStyleOpt.bFontStyleBold);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Style (Italic) : [%d]", getOption.option.mmsStyleOpt.bFontStyleItalic);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Style (Underline) : [%d]", getOption.option.mmsStyleOpt.bFontStyleUnderline);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Color (Red) : [%d]", getOption.option.mmsStyleOpt.fontColorRed);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Color (Green) : [%d]", getOption.option.mmsStyleOpt.fontColorGreen);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Color (Blue) : [%d]", getOption.option.mmsStyleOpt.fontColorBlue);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Font Color (Hue) : [%d]", getOption.option.mmsStyleOpt.fontColorHue);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "BG Color (Red) : [%d]", getOption.option.mmsStyleOpt.bgColorRed);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "BG Color (Green) : [%d]", getOption.option.mmsStyleOpt.bgColorGreen);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "BG Color (Blue) : [%d]", getOption.option.mmsStyleOpt.bgColorBlue);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "BG Color (Hue) : [%d]", getOption.option.mmsStyleOpt.bgColorHue);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Page Duration : [%d]", getOption.option.mmsStyleOpt.pageDur);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Page Custom Duration : [%d]", getOption.option.mmsStyleOpt.pageCustomDur);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Page Duration Manual : [%d]", getOption.option.mmsStyleOpt.pageDurManual);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MMS_STYLEOPT_S));

			setOption.type = MSG_MMS_STYLEOPT;

			setOption.option.mmsStyleOpt.fontSize = 30;
			setOption.option.mmsStyleOpt.bFontStyleBold = true;
			setOption.option.mmsStyleOpt.bFontStyleItalic = true;
			setOption.option.mmsStyleOpt.bFontStyleUnderline = false;
			setOption.option.mmsStyleOpt.fontColorRed = 0;
			setOption.option.mmsStyleOpt.fontColorGreen = 0;
			setOption.option.mmsStyleOpt.fontColorBlue = 0;
			setOption.option.mmsStyleOpt.fontColorHue = 255;
			setOption.option.mmsStyleOpt.bgColorRed = 255;
			setOption.option.mmsStyleOpt.bgColorGreen = 255;
			setOption.option.mmsStyleOpt.bgColorBlue = 255;
			setOption.option.mmsStyleOpt.bgColorHue = 255;
			setOption.option.mmsStyleOpt.pageDur = 2;
			setOption.option.mmsStyleOpt.pageCustomDur = 0;
			setOption.option.mmsStyleOpt.pageDurManual = 0;

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}
	while (1);
}


void MsgTestPushMsgOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_PUSHMSG_OPT_S));

		getOption.type = MSG_PUSHMSG_OPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		print("======================================");
		print("=========== Push Msg Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Receive Option : [%d]", getOption.option.pushMsgOpt.bReceive);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Service Load : [%d]", getOption.option.pushMsgOpt.serviceType);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_PUSHMSG_OPT_S));

			setOption.type = MSG_PUSHMSG_OPT;

			setOption.option.pushMsgOpt.bReceive = false;
			setOption.option.pushMsgOpt.serviceType = MSG_PUSH_SERVICE_PROMPT;

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}while (1);
}


void MsgTestCBMsgOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_CBMSG_OPT_S));

		getOption.type = MSG_CBMSG_OPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		print("======================================");
		print("=========== CB Msg Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Receive : [%d]", getOption.option.cbMsgOpt.bReceive);
		print(strPrint);

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "All Channel : [%d]", getOption.option.cbMsgOpt.bAllChannel);
		print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Channel Count : [%d]", getOption.option.cbMsgOpt.channelData.channelCnt);
			print(strPrint);

		for (int i = 0; i < getOption.option.cbMsgOpt.channelData.channelCnt; i++)
		{
			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "Channel Activate : [%d]", getOption.option.cbMsgOpt.channelData.channelInfo[i].bActivate);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "Channel ID : [%d]", getOption.option.cbMsgOpt.channelData.channelInfo[i].id);
			print(strPrint);

			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "Channel Name : [%s]", getOption.option.cbMsgOpt.channelData.channelInfo[i].name);
			print(strPrint);
		}

		for (int i = MSG_CBLANG_TYPE_ALL; i < MSG_CBLANG_TYPE_MAX; i++)
		{
			memset(strPrint, 0x00, sizeof(strPrint));
			snprintf(strPrint, sizeof(strPrint), "Language[%d] : [%d]", i, getOption.option.cbMsgOpt.bLanguage[i]);
			print(strPrint);
		}

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_CBMSG_OPT_S));

			setOption.type = MSG_CBMSG_OPT;

			setOption.option.cbMsgOpt.bReceive = true;
			setOption.option.cbMsgOpt.bAllChannel = true;

			setOption.option.cbMsgOpt.channelData.channelCnt = 3;

			setOption.option.cbMsgOpt.channelData.channelInfo[0].bActivate = true;
			setOption.option.cbMsgOpt.channelData.channelInfo[0].id = 10;
			memset(setOption.option.cbMsgOpt.channelData.channelInfo[0].name, 0x00, CB_CHANNEL_NAME_MAX+1);
			strncpy(setOption.option.cbMsgOpt.channelData.channelInfo[0].name, "CB MSG", CB_CHANNEL_NAME_MAX);

			setOption.option.cbMsgOpt.channelData.channelInfo[1].bActivate = true;
			setOption.option.cbMsgOpt.channelData.channelInfo[1].id = 50;
			memset(setOption.option.cbMsgOpt.channelData.channelInfo[1].name, 0x00, CB_CHANNEL_NAME_MAX+1);
			strncpy(setOption.option.cbMsgOpt.channelData.channelInfo[1].name, "CB TEST", CB_CHANNEL_NAME_MAX);

			setOption.option.cbMsgOpt.channelData.channelInfo[2].bActivate = false;
			setOption.option.cbMsgOpt.channelData.channelInfo[2].id = 60;
			memset(setOption.option.cbMsgOpt.channelData.channelInfo[2].name, 0x00, CB_CHANNEL_NAME_MAX+1);

//			memcpy(&setOption.option.cbMsgOpt.channelData, &getOption.option.cbMsgOpt.channelData, sizeof(MSG_CB_CHANNEL_S));

			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_ALL] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_ENG] = true;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_GER] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_FRE] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_ITA] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_NED] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_SPA] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_POR] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_SWE] = false;
			setOption.option.cbMsgOpt.bLanguage[MSG_CBLANG_TYPE_TUR] = false;

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}while (1);
}


void MsgTestVoiceMailOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_VOICEMAIL_OPT_S));

		getOption.type = MSG_VOICEMAIL_OPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		print("======================================");
		print("=========== Voice Mail Option ===========");
		print("======================================");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Voice Mail Number : [%s]", getOption.option.voiceMailOpt.mailNumber);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_VOICEMAIL_OPT_S));

			setOption.type = MSG_VOICEMAIL_OPT;

			memset(setOption.option.voiceMailOpt.mailNumber, 0x00, sizeof(MAX_PHONE_NUMBER_LEN));
			strncpy(setOption.option.voiceMailOpt.mailNumber, "11111", MAX_PHONE_NUMBER_LEN);

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}while (1);
}


void MsgTestMsgSizeOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char menu[2];
	char strPrint [512];

	MSG_SETTING_S setOption;
	MSG_SETTING_S getOption;

	do
	{
		memset(&getOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MSGSIZE_OPT_S));

		getOption.type = MSG_MSGSIZE_OPT;

		err = msg_get_config(hMsgHandle, &getOption);

		system ("clear");

		memset(strPrint, 0x00, sizeof(strPrint));
		snprintf(strPrint, sizeof(strPrint), "Message Size : [%d]", getOption.option.msgSizeOpt.nMsgSize);
		print(strPrint);

		print("======================================");
		print("================ Menu ================");
		print("[U] Update Options");
		print("[B] Back");
		print("======================================");

		print("Input : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 2);

		if (!strcmp(menu, "U"))
		{
			memset(&setOption, 0x00, sizeof(MSG_OPTION_TYPE_T)+sizeof(MSG_MSGSIZE_OPT_S));

			setOption.type = MSG_MSGSIZE_OPT;

			setOption.option.msgSizeOpt.nMsgSize = 100;

			err = msg_set_config(hMsgHandle, &setOption);

			if (err == MSG_SUCCESS)
				print("Setting Config Data is OK!");
			else
				print("Setting Config Data is failed!");
		}
		else if (!strcmp(menu, "B"))
		{
			break;
		}
		else
		{
			continue;
		}
	}while (1);
}

