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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"
#include "MsgContact.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgDevicedWrapper.h"
#include "SmsPluginTransport.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginStorage.h"
#include "SmsPluginSetting.h"
#include "SmsPluginConcatHandler.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginDSHandler.h"
#include "SmsPluginParamCodec.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginEventHandler - Member Functions
==================================================================================================*/
SmsPluginEventHandler* SmsPluginEventHandler::pInstance = NULL;


SmsPluginEventHandler::SmsPluginEventHandler()
{
	/**  Initialize global parameters */
	memset(&listener, 0x00, sizeof(MSG_PLUGIN_LISTENER_S));
	memset(&sentInfo, 0x00, sizeof(SMS_SENT_INFO_S));

	pSimCnt = NULL;
	devStatus = false;
	bUdhMwiMethod = false;
	udhMwiCnt = 0;
	devHandle = NULL;
}


SmsPluginEventHandler::~SmsPluginEventHandler()
{
}


SmsPluginEventHandler* SmsPluginEventHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginEventHandler();

	return pInstance;
}


void SmsPluginEventHandler::registerListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	listener = *pListener;
}


void SmsPluginEventHandler::handleSentStatus(msg_network_status_t NetStatus)
{
	MSG_DEBUG("NetStatus[%d]", NetStatus);

	if (sentInfo.bLast == true || NetStatus != MSG_NETWORK_SEND_SUCCESS) {
		/** Update Msg Status */
		if (sentInfo.reqInfo.msgInfo.msgPort.valid == false) {
			/*SmsPluginStorage::instance()->updateSentMsg(&(sentInfo.reqInfo.msgInfo), NetStatus); */

			sentInfo.reqInfo.msgInfo.networkStatus = NetStatus;

			if (NetStatus == MSG_NETWORK_SEND_SUCCESS) {
				/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
				MSG_DEBUG("Add phone log");
				MsgAddPhoneLog(&(sentInfo.reqInfo.msgInfo));
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
				sentInfo.reqInfo.msgInfo.folderId = MSG_SENTBOX_ID; /* It should be set after adding phone log. */
			} else {
				sentInfo.reqInfo.msgInfo.bRead = false;
			}

			SmsPluginStorage::instance()->updateSmsMessage(&(sentInfo.reqInfo.msgInfo));

			callbackStorageChange(MSG_STORAGE_CHANGE_UPDATE, &(sentInfo.reqInfo.msgInfo));
		}

		MSG_DEBUG("sentInfo.reqInfo.sendOptInfo.bSetting [%d]", sentInfo.reqInfo.sendOptInfo.bSetting);
		MSG_DEBUG("sentInfo.reqInfo.sendOptInfo.bKeepCopy [%d]", sentInfo.reqInfo.sendOptInfo.bKeepCopy);
		/** Check sending options */
		if (sentInfo.reqInfo.sendOptInfo.bSetting && !sentInfo.reqInfo.sendOptInfo.bKeepCopy && NetStatus == MSG_NETWORK_SEND_SUCCESS) {
			SmsPluginStorage::instance()->deleteSmsMessage(sentInfo.reqInfo.msgInfo.msgId);
			callbackStorageChange(MSG_STORAGE_CHANGE_DELETE, &(sentInfo.reqInfo.msgInfo));
		}

		/** Callback to MSG FW */
		MSG_SENT_STATUS_S msgStatus;

		msgStatus.reqId = sentInfo.reqInfo.reqId;
		msgStatus.status = NetStatus;

		MSG_DEBUG("sentStatus.reqId : %d", msgStatus.reqId);
		MSG_DEBUG("sentStatus.status : %d", msgStatus.status);

		listener.pfSentStatusCb(&msgStatus);
	}
}


void SmsPluginEventHandler::handleMsgIncoming(TapiHandle *handle, SMS_TPDU_S *pTpdu)
{
	/** Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;
	MSG_MESSAGE_INFO_S stored_msgInfo;

	/** initialize msgInfo */
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);


	if (pTpdu->tpduType == SMS_TPDU_DELIVER) {
		/** check unsupported message */
		if (pTpdu->data.deliver.dcs.codingScheme == SMS_CHARSET_8BIT && pTpdu->data.deliver.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_SUCCESS);
			return;
		}
	}

	bUdhMwiMethod = false;
	udhMwiCnt = 0;

	if (pTpdu->data.deliver.dcs.msgClass == SMS_MSG_CLASS_2)
		msgInfo.storageId = MSG_STORAGE_UNKNOWN;
	else
		msgInfo.storageId = MSG_STORAGE_PHONE;

	msgInfo.sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);

	/** convert to msgInfo */
	convertTpduToMsginfo(pTpdu, &msgInfo);

	if (msgInfo.msgPort.valid == true) {
		if ((msgInfo.msgPort.dstPort >= 0x23F4 && msgInfo.msgPort.dstPort <= 0x23F7) || /** Check unsupported message (Vcard WAP push) **/
			(msgInfo.msgPort.dstPort == 0x1581)) { /** Check unsupported message (ringtone smart message) **/
			memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));
			snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "<Unsupported message>");
			msgInfo.dataSize = strlen(msgInfo.msgText);
			msgInfo.msgPort.valid = false;
		}
	}

	bool bStoreVoiceMsg = false;

	if (bUdhMwiMethod == false) {
		/** check MWI and set info to SIM for DCS & Address method */
		if (pTpdu->tpduType == SMS_TPDU_DELIVER && pTpdu->data.deliver.dcs.bMWI == true) {
			int MwiCnt = 0;
			MSG_DEBUG("MWI message - DCS method");

			if (pTpdu->data.deliver.dcs.bIndActive == false) {
				SmsPluginSetting::instance()->setMwiInfo(msgInfo.sim_idx, msgInfo.msgType.subType, 0);
				MwiCnt = 0;
			} else {
				SmsPluginSetting::instance()->setMwiInfo(msgInfo.sim_idx, msgInfo.msgType.subType, 1);
				MwiCnt = 1;

				/* For address method */
				if (pTpdu->data.deliver.pid == 0x20 && pTpdu->data.deliver.originAddress.ton == SMS_TON_ALPHANUMERIC) {
					MSG_DEBUG("MWI message - Address method");
					char *voiceNumber = NULL;
					char *voiceAlphaId = NULL;
					char keyName[MAX_VCONFKEY_NAME_LEN];

					memset(keyName, 0x00, sizeof(keyName));
					snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, msgInfo.sim_idx);
					voiceNumber = MsgSettingGetString(keyName);

					memset(keyName, 0x00, sizeof(keyName));
					snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, msgInfo.sim_idx);
					voiceAlphaId = MsgSettingGetString(keyName);

					memset(msgInfo.addressList[0].addressVal, 0x00, sizeof(msgInfo.addressList[0].addressVal));
					memset(msgInfo.addressList[0].displayName, 0x00, sizeof(msgInfo.addressList[0].displayName));

					if (voiceNumber) {
						snprintf(msgInfo.addressList[0].addressVal, sizeof(msgInfo.addressList[0].addressVal), "%s", voiceNumber);
						free(voiceNumber);
						voiceNumber = NULL;
					}

					if (voiceAlphaId) {
						snprintf(msgInfo.addressList[0].displayName, sizeof(msgInfo.addressList[0].displayName), "%s", voiceAlphaId);
						free(voiceAlphaId);
						voiceAlphaId = NULL;
					}
				}
			}

			if (pTpdu->data.deliver.dcs.codingGroup == SMS_GROUP_STORE) {
				bStoreVoiceMsg = true;
				memset(&stored_msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
				memcpy(&stored_msgInfo, &msgInfo, sizeof(MSG_MESSAGE_INFO_S));
				stored_msgInfo.msgType.subType = MSG_NORMAL_SMS;
			}

			memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));
			switch (msgInfo.msgType.subType) {
			case MSG_MWI_VOICE_SMS :
				snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d", MwiCnt);
				break;
			case MSG_MWI_FAX_SMS :
				snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d new fax message", MwiCnt);
				break;
			case MSG_MWI_EMAIL_SMS :
				snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d new email message", MwiCnt);
				break;
			default :
				snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d new special message", MwiCnt);
				break;
			}
			msgInfo.dataSize = strlen(msgInfo.msgText);

			if (pTpdu->data.deliver.dcs.codingGroup == SMS_GROUP_DISCARD)
				msgInfo.bStore = false;
		}
	}

	/** Short Message Type 0 - Just Send Deliver Report */
	if (msgInfo.msgType.subType == MSG_TYPE0_SMS) {
		SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_SUCCESS);
		return;
	}

	/** Print MSG_MESSAGE_INFO_S */
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.nAddressCnt : %d", msgInfo.nAddressCnt);
	if (msgInfo.nAddressCnt > 0) {
		MSG_DEBUG("msgInfo.addressList[0].addressType : %d", msgInfo.addressList[0].addressType);
		MSG_SEC_DEBUG("msgInfo.addressList[0].addressVal : %s", msgInfo.addressList[0].addressVal);
		MSG_SEC_DEBUG("msgInfo.addressList[0].displayName : %s", msgInfo.addressList[0].displayName);
	}
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.bStore : %d", msgInfo.bStore);
	MSG_DEBUG("msgInfo.direction : %d", msgInfo.direction);
	MSG_DEBUG("msgInfo.msgType.mainType : %d", msgInfo.msgType.mainType);
	MSG_DEBUG("msgInfo.msgType.subType : %d", msgInfo.msgType.subType);
	MSG_DEBUG("msgInfo.msgType.classType : %d", msgInfo.msgType.classType);
	MSG_DEBUG("msgInfo.displayTime : %d", msgInfo.displayTime);
	MSG_DEBUG("msgInfo.msgPort.valid : %d", msgInfo.msgPort.valid);
	MSG_DEBUG("msgInfo.encodeType : %d", msgInfo.encodeType);
	MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);
	MSG_DEBUG("msgInfo.sim_idx : %d", msgInfo.sim_idx);

	if (msgInfo.bTextSms == true) {
		MSG_SEC_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
	} else {
		MSG_SEC_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
	}

	MSG_DEBUG("###############################################################");

	msg_error_t err = MSG_SUCCESS;

	if (msgInfo.msgType.subType == MSG_STATUS_REPORT_SMS) {
		/** Status Report Message */
		err = SmsPluginStorage::instance()->updateMsgDeliverStatus(&msgInfo, pTpdu->data.statusRep.msgRef);

		if (err == MSG_SUCCESS)
			err = listener.pfMsgIncomingCb(&msgInfo);
		else
			MSG_DEBUG("updateMsgDeliverStatus is failed [%d]", err);

		/** Handling of Fail Case ?? */
		SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_SUCCESS);
	} else { /** SMS Deliver */
		/** Class 2 Msg */
		if (msgInfo.msgType.classType == MSG_CLASS_2) {
			if (msgInfo.bTextSms == false) { /** Concat Msg cannot be saved in SIM */
				msgInfo.msgType.classType = MSG_CLASS_NONE;
				msgInfo.storageId = MSG_STORAGE_PHONE;
			} else {
				/** set total segment of Class2 message as 1 */
				SmsPluginSimMsg::instance()->setSmsTpduTotalSegCount(1);
			}
		}

		/** Add message to DB */
		if (msgInfo.msgPort.valid == false) {
			err = SmsPluginStorage::instance()->checkMessage(&msgInfo);
		}

		/** Callback to MSG FW */
		if (msgInfo.msgType.classType == MSG_CLASS_2) {
			if (((msgInfo.msgType.subType >= MSG_MWI_VOICE_SMS) && (msgInfo.msgType.subType <= MSG_MWI_OTHER_SMS)) &&
					(msgInfo.bStore == false)) {
				if (listener.pfMsgIncomingCb(&msgInfo) != MSG_SUCCESS)
					MSG_DEBUG("listener.pfMsgIncomingCb is failed!");

				SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_SUCCESS);
			}
		} else {
			if (err == MSG_SUCCESS) {
				MSG_DEBUG("callback to msg fw");
				err = listener.pfMsgIncomingCb(&msgInfo);
				if (bStoreVoiceMsg) {
					err = listener.pfMsgIncomingCb(&stored_msgInfo);
				}
			} else {
				if (msgInfo.msgType.classType == MSG_CLASS_0) {
					MSG_DEBUG("callback for class0 message to msg fw");
					if (listener.pfMsgIncomingCb(&msgInfo) != MSG_SUCCESS)
						MSG_DEBUG("listener.pfMsgIncomingCb is failed!");
				}
			}

			/** Send Deliver Report */
			if (msgInfo.msgType.classType == MSG_CLASS_0)
				SmsPluginTransport::instance()->sendClass0DeliverReport(handle, err);
			else
				SmsPluginTransport::instance()->sendDeliverReport(handle, err);
		}

		/* Tizen Validation System */
		char *msisdn = NULL;
		char keyName[MAX_VCONFKEY_NAME_LEN];
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, msgInfo.sim_idx);
		msisdn = MsgSettingGetString(keyName);

		MSG_SMS_VLD_INFO("%d, SMS Receive, %s->%s, %s",  msgInfo.msgId, \
																msgInfo.addressList[0].addressVal, \
																(msisdn == NULL)?"ME":msisdn, \
																(err == MSG_SUCCESS)?"Success":"Fail");

		MSG_SMS_VLD_TXT("%d, [%s]", msgInfo.msgId, msgInfo.msgText);

		if (msisdn) {
			free(msisdn);
			msisdn = NULL;
		}
	}
}

void SmsPluginEventHandler::handlePushMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen, char *application_id, char *content_type)
{
	MSG_PUSH_MESSAGE_DATA_S pushData;

	memset(&pushData, 0x00, sizeof(MSG_PUSH_MESSAGE_DATA_S));

	/** set PUSH data */
	memcpy(pushData.pushHeader, pPushHeader, strlen(pPushHeader));

	pushData.pushBodyLen = pushBodyLen;
	memcpy(pushData.pushBody, pPushBody, pushBodyLen);

	memcpy(pushData.pushAppId, application_id, MAX_WAPPUSH_ID_LEN);
	memcpy(pushData.pushContentType, content_type, MAX_WAPPUSH_CONTENT_TYPE_LEN);

	/** Callback to MSG FW */
	listener.pfPushMsgIncomingCb(&pushData);
}


void SmsPluginEventHandler::handleResendMessage(void)
{
	listener.pfResendMessageCb();
}


void SmsPluginEventHandler::handleSyncMLMsgIncoming(msg_syncml_message_type_t msgType, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen, int simIndex)
{
	MSG_SYNCML_MESSAGE_DATA_S syncMLData;

	memset(&syncMLData, 0x00, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	/** set syncML data */
	syncMLData.syncmlType = msgType;

	syncMLData.simIndex = simIndex;

	syncMLData.pushBodyLen = PushBodyLen;
	memcpy(syncMLData.pushBody, pPushBody, PushBodyLen);

	syncMLData.wspHeaderLen = WspHeaderLen;
	memcpy(syncMLData.wspHeader, pWspHeader, WspHeaderLen);

	/** Callback to MSG FW */
	listener.pfSyncMLMsgIncomingCb(&syncMLData);
}


void SmsPluginEventHandler::handleLBSMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen)
{
	MSG_LBS_MESSAGE_DATA_S lBSData;

	memset(&lBSData, 0x00, sizeof(MSG_LBS_MESSAGE_DATA_S));

	/** set LBA data */
	memcpy(lBSData.pushHeader, pPushHeader, strlen(pPushHeader));

	lBSData.pushBodyLen = pushBodyLen;
	memcpy(lBSData.pushBody, pPushBody, pushBodyLen);

	/** Callback to MSG FW */
	listener.pfLBSMsgIncomingCb(&lBSData);
}

msg_error_t SmsPluginEventHandler::callbackMsgIncoming(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/** Callback to MSG FW */
	err = listener.pfMsgIncomingCb(pMsgInfo);

	MSG_END();

	return err;
}

msg_error_t SmsPluginEventHandler::callbackCBMsgIncoming(MSG_CB_MSG_S *pCbMsg, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/** Callback to MSG FW */
	err = listener.pfCBMsgIncomingCb(pCbMsg, pMsgInfo);

	MSG_END();

	return err;
}


msg_error_t SmsPluginEventHandler::callbackInitSimBySat()
{
	/** Callback to MSG FW */
	return listener.pfInitSimBySatCb();
}


msg_error_t SmsPluginEventHandler::callbackStorageChange(msg_storage_change_type_t storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = pMsgInfo->msgId;
	msgIdList.msgIdList = msgIds;

	/* Callback to MSG FW */
	listener.pfStorageChangeCb(storageChangeType, &msgIdList);

	return MSG_SUCCESS;
}


void SmsPluginEventHandler::convertTpduToMsginfo(SMS_TPDU_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	switch (pTpdu->tpduType) {
	case SMS_TPDU_SUBMIT :
		convertSubmitTpduToMsginfo(&pTpdu->data.submit, msgInfo);
		break;
	case SMS_TPDU_DELIVER :
		convertDeliverTpduToMsginfo(&pTpdu->data.deliver, msgInfo);
		break;
	case SMS_TPDU_STATUS_REP :
		convertStatusRepTpduToMsginfo(&pTpdu->data.statusRep, msgInfo);
		break;
	}
}


void SmsPluginEventHandler::convertSubmitTpduToMsginfo(const SMS_SUBMIT_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	int addressListCnt = 0;

	/** Convert Type  values */
	msgInfo->msgType.mainType = MSG_SMS_TYPE;
	msgInfo->msgType.subType = convertMsgSubType(pTpdu->pid);

	/** set folder id (temporary) */
	msgInfo->folderId = MSG_SENTBOX_ID;

	switch (pTpdu->dcs.msgClass) {
	case SMS_MSG_CLASS_0:
		msgInfo->msgType.classType = MSG_CLASS_0;
		break;
	case SMS_MSG_CLASS_1:
		msgInfo->msgType.classType = MSG_CLASS_1;
		break;
	case SMS_MSG_CLASS_2:
		msgInfo->msgType.classType = MSG_CLASS_2;
		break;
	case SMS_MSG_CLASS_3:
		msgInfo->msgType.classType = MSG_CLASS_3;
		break;
	default:
		msgInfo->msgType.classType = MSG_CLASS_NONE;
		break;
	}

	msgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MO;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	/** What kind of time has to be saved?? (temporary store time) */
	msgInfo->displayTime = time(NULL);

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;

	msgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(msgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	msgInfo->addressList[addressListCnt].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(msgInfo->addressList[addressListCnt].addressVal, pTpdu->destAddress.address, MAX_ADDRESS_VAL_LEN);

	/**exception operation for none userdata */
	if (pTpdu->userData.length == 0) {
		snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "[Broken Message]");
		msgInfo->dataSize = strlen(msgInfo->msgText);
		return;
	}

	/** Convert Data values */
	MsgTextConvert *textCvt = MsgTextConvert::instance();
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		MSG_LANG_INFO_S langInfo = {0, };

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		msgInfo->dataSize = textCvt->convertGSM7bitToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		msgInfo->dataSize = textCvt->convertUCS2ToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
	}
}


void SmsPluginEventHandler::convertDeliverTpduToMsginfo(const SMS_DELIVER_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	MSG_BEGIN();

	/** Convert Type  values */
	msgInfo->msgType.mainType = MSG_SMS_TYPE;
	msgInfo->msgType.subType = convertMsgSubType(pTpdu->pid);

	/** set folder id (temporary) */
	msgInfo->folderId = MSG_INBOX_ID;

	time_t rawtime = 0;
	if (msgInfo->storageId == MSG_STORAGE_SIM) {
	/*** Comment below lines to save local UTC time..... (it could be used later.)
	***/
		if (pTpdu->timeStamp.format == SMS_TIME_ABSOLUTE) {
			MSG_DEBUG("year : %d", pTpdu->timeStamp.time.absolute.year);
			MSG_DEBUG("month : %d", pTpdu->timeStamp.time.absolute.month);
			MSG_DEBUG("day : %d", pTpdu->timeStamp.time.absolute.day);
			MSG_DEBUG("hour : %d", pTpdu->timeStamp.time.absolute.hour);
			MSG_DEBUG("minute : %d", pTpdu->timeStamp.time.absolute.minute);
			MSG_DEBUG("second : %d", pTpdu->timeStamp.time.absolute.second);
			MSG_DEBUG("timezone : %d", pTpdu->timeStamp.time.absolute.timeZone);

			char displayTime[32];
			struct tm timeTM;

			struct tm timeinfo;
			memset(&timeinfo, 0x00, sizeof(tm));

			timeinfo.tm_year = (pTpdu->timeStamp.time.absolute.year + 100);
			timeinfo.tm_mon = (pTpdu->timeStamp.time.absolute.month - 1);
			timeinfo.tm_mday = pTpdu->timeStamp.time.absolute.day;
			timeinfo.tm_hour = pTpdu->timeStamp.time.absolute.hour;
			timeinfo.tm_min = pTpdu->timeStamp.time.absolute.minute;
			timeinfo.tm_sec = pTpdu->timeStamp.time.absolute.second;
			timeinfo.tm_isdst = 0;

			rawtime = mktime(&timeinfo);

			MSG_DEBUG("tzname[0] [%s]", tzname[0]);
			MSG_DEBUG("tzname[1] [%s]", tzname[1]);
			MSG_DEBUG("timezone [%d]", timezone);
			MSG_DEBUG("daylight [%d]", daylight);

			memset(displayTime, 0x00, sizeof(displayTime));
			strftime(displayTime, 32, "%Y-%02m-%02d %T %z", &timeinfo);
			MSG_DEBUG("displayTime [%s]", displayTime);

			rawtime -= (pTpdu->timeStamp.time.absolute.timeZone * (3600/4));

			localtime_r(&rawtime, &timeTM);
			memset(displayTime, 0x00, sizeof(displayTime));
			strftime(displayTime, 32, "%Y-%02m-%02d %T %z", &timeTM);
			MSG_DEBUG("displayTime [%s]", displayTime);

			rawtime -= timezone;

			localtime_r(&rawtime, &timeTM);
			memset(displayTime, 0x00, sizeof(displayTime));
			strftime(displayTime, 32, "%Y-%02m-%02d %T %z", &timeTM);
			MSG_DEBUG("displayTime [%s]", displayTime);
		}
	}
	else
		rawtime = time(NULL);

	msgInfo->displayTime = rawtime;

	switch (pTpdu->dcs.msgClass) {
	case SMS_MSG_CLASS_0:
		msgInfo->msgType.classType = MSG_CLASS_0;
		break;
	case SMS_MSG_CLASS_1:
		msgInfo->msgType.classType = MSG_CLASS_1;
		break;
	case SMS_MSG_CLASS_2:
		msgInfo->msgType.classType = MSG_CLASS_2;
		msgInfo->storageId = MSG_STORAGE_SIM;
		break;
	case SMS_MSG_CLASS_3:
		msgInfo->msgType.classType = MSG_CLASS_3;
		break;
	default:
		msgInfo->msgType.classType = MSG_CLASS_NONE;
		break;
	}

	if (pTpdu->dcs.bMWI) {
		if (pTpdu->dcs.indType == SMS_VOICE_INDICATOR)
			msgInfo->msgType.subType = MSG_MWI_VOICE_SMS;
		else if (pTpdu->dcs.indType == SMS_VOICE2_INDICATOR)
			msgInfo->msgType.subType = MSG_MWI_VOICE2_SMS;
		else if (pTpdu->dcs.indType == SMS_FAX_INDICATOR)
			msgInfo->msgType.subType = MSG_MWI_FAX_SMS;
		else if (pTpdu->dcs.indType == SMS_EMAIL_INDICATOR)
			msgInfo->msgType.subType = MSG_MWI_EMAIL_SMS;
		else if (pTpdu->dcs.indType == SMS_OTHER_INDICATOR)
			msgInfo->msgType.subType = MSG_MWI_OTHER_SMS;
	}

	msgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MT;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;

	msgInfo->addressList =  (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(msgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	msgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(msgInfo->addressList[0].addressVal, pTpdu->originAddress.address, MAX_ADDRESS_VAL_LEN);

	msgInfo->msgPort.valid = false;
	msgInfo->msgPort.dstPort = 0;
	msgInfo->msgPort.srcPort = 0;

	for (int i = 0; i < pTpdu->userData.headerCnt; i++) {
		/** Convert UDH values - Port Number */
		if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_8BIT) {
			msgInfo->msgPort.valid = true;
			msgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort8bit.destPort;
			msgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort8bit.originPort;
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_16BIT) {
			msgInfo->msgPort.valid = true;
			msgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort16bit.destPort;
			msgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort16bit.originPort;
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_SPECIAL_SMS) {
			msgInfo->msgType.subType = (pTpdu->userData.header[i].udh.specialInd.msgInd+MSG_MWI_VOICE_SMS);
			msgInfo->bStore = pTpdu->userData.header[i].udh.specialInd.bStore;

			bUdhMwiMethod = true;

			if (pTpdu->dcs.codingGroup == SMS_GROUP_DISCARD)
				msgInfo->bStore = false;

			udhMwiCnt = pTpdu->userData.header[i].udh.specialInd.waitMsgNum;

			if (udhMwiCnt < 0) {
				MSG_DEBUG("Message waiting number is smaller than 0. It will be treated as 0. [%d]", udhMwiCnt);
				udhMwiCnt = 0;
			}

			MSG_DEBUG("Message waiting number : [%d]", udhMwiCnt);

			SmsPluginSetting::instance()->setMwiInfo(msgInfo->sim_idx, msgInfo->msgType.subType, udhMwiCnt);

			if (udhMwiCnt > 0 && (msgInfo->msgType.subType >= MSG_MWI_VOICE_SMS && msgInfo->msgType.subType <= MSG_MWI_OTHER_SMS)) {
				switch (msgInfo->msgType.subType) {
				case MSG_MWI_VOICE_SMS :
					snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%d", udhMwiCnt);
					break;
				case MSG_MWI_FAX_SMS :
					snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%d new fax message", udhMwiCnt);
					break;
				case MSG_MWI_EMAIL_SMS :
					snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%d new email message", udhMwiCnt);
					break;
				default :
					snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%d new special message", udhMwiCnt);
					break;
				}
				msgInfo->dataSize = strlen(msgInfo->msgText);
				return;
			}
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_ALTERNATE_REPLY_ADDRESS) {
			strncpy(msgInfo->addressList[0].addressVal, pTpdu->userData.header[i].udh.alternateAddress.address, MAX_ADDRESS_VAL_LEN);
		} else if (pTpdu->userData.header[i].udhType >= SMS_UDH_EMS_FIRST && pTpdu->userData.header[i].udhType <= SMS_UDH_EMS_LAST) {
			/* TODO: Raw text should be changed to string design id. Currently there's no design id in message-app-lite */
/*			char *msg_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, "IDS_MSGF_POP_ERROR_UNSUPPORTED_MSG");
			snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%s", msg_text);
*/
			snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "Unsupported Message");
			msgInfo->dataSize = strlen(msgInfo->msgText);
/*			if (msg_text) {
				free(msg_text);
				msg_text = NULL;
			}
*/
			return;
		}
	}

	/**length 0 - no user data - msg should be received */
	if (pTpdu->userData.length <= 0) {
		memset(msgInfo->msgText, 0x00, sizeof(msgInfo->msgText));
		msgInfo->dataSize = 0;

		switch (pTpdu->dcs.codingScheme) {
		case SMS_CHARSET_7BIT:
			msgInfo->encodeType = MSG_ENCODE_GSM7BIT;
			break;
		case SMS_CHARSET_8BIT:
			msgInfo->encodeType = MSG_ENCODE_8BIT;
			break;
		case SMS_CHARSET_UCS2:
			msgInfo->encodeType = MSG_ENCODE_UCS2;
			break;
		default:
			msgInfo->encodeType = MSG_ENCODE_8BIT;
			break;
		}

		return;
	} else if (pTpdu->userData.length > MAX_MSG_TEXT_LEN) {
		snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "[Broken Message]");
		msgInfo->dataSize = strlen(msgInfo->msgData);
		return;
	}

	/** Convert Data values */
	MsgTextConvert *textCvt = MsgTextConvert::instance();
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		MSG_LANG_INFO_S langInfo = {0, };

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		for (int i = 0; i < pTpdu->userData.headerCnt; i++) {
			if (pTpdu->userData.header[i].udhType == SMS_UDH_SINGLE_SHIFT) {
				langInfo.bSingleShift = true;
				langInfo.singleLang = pTpdu->userData.header[i].udh.singleShift.langId;
			} else if (pTpdu->userData.header[i].udhType == SMS_UDH_LOCKING_SHIFT) {
				langInfo.bLockingShift = true;
				langInfo.lockingLang = pTpdu->userData.header[i].udh.lockingShift.langId;
			}
		}

		msgInfo->encodeType = MSG_ENCODE_GSM7BIT;
		msgInfo->dataSize = textCvt->convertGSM7bitToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_8BIT) {
		msgInfo->encodeType = MSG_ENCODE_8BIT;
		memcpy(msgInfo->msgText, pTpdu->userData.data, sizeof(pTpdu->userData.data));
		msgInfo->dataSize = pTpdu->userData.length;
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		msgInfo->encodeType = MSG_ENCODE_UCS2;
		msgInfo->dataSize = textCvt->convertUCS2ToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_EUCKR) {
		msgInfo->encodeType = MSG_ENCODE_8BIT;
		msgInfo->dataSize = textCvt->convertEUCKRToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
		return;
	}

	MSG_END();
}


void SmsPluginEventHandler::convertStatusRepTpduToMsginfo(const SMS_STATUS_REPORT_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	/** Convert Type  values */
	msgInfo->msgType.mainType = MSG_SMS_TYPE;
	msgInfo->msgType.subType = MSG_STATUS_REPORT_SMS;

	/** set folder id */
	msgInfo->folderId = MSG_INBOX_ID;

	/** set storage id */
	if (msgInfo->storageId == MSG_STORAGE_UNKNOWN) {
		msgInfo->storageId = MSG_STORAGE_PHONE;
	}

	switch (pTpdu->dcs.msgClass) {
	case SMS_MSG_CLASS_0:
		msgInfo->msgType.classType = MSG_CLASS_0;
		break;
	case SMS_MSG_CLASS_1:
		msgInfo->msgType.classType = MSG_CLASS_1;
		break;
	case SMS_MSG_CLASS_2:
		msgInfo->msgType.classType = MSG_CLASS_2;
		break;
	case SMS_MSG_CLASS_3:
		msgInfo->msgType.classType = MSG_CLASS_3;
		break;
	default:
		msgInfo->msgType.classType = MSG_CLASS_NONE;
		break;
	}

	MSG_DEBUG("delivery status : [%d]", pTpdu->status);

	if (pTpdu->status == SMS_STATUS_RECEIVE_SUCCESS)
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_SUCCESS;
	else if (pTpdu->status == SMS_STATUS_TRY_REQUEST_PENDING)
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_PENDING;
	else if (pTpdu->status == SMS_STATUS_PERM_MSG_VAL_PERIOD_EXPIRED)
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_EXPIRED;
	else
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_FAIL;

	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MT;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	msgInfo->displayTime = time(NULL);

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;

	msgInfo->addressList =  (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(msgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	msgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(msgInfo->addressList[0].addressVal, pTpdu->recipAddress.address, MAX_ADDRESS_VAL_LEN);

	msgInfo->msgPort.valid = false;
	msgInfo->msgPort.dstPort = 0;
	msgInfo->msgPort.srcPort = 0;

	for (int i = 0; i < pTpdu->userData.headerCnt; i++) {
		/** Convert UDH values - Port Number */
		if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_8BIT) {
			msgInfo->msgPort.valid = true;
			msgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort8bit.destPort;
			msgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort8bit.originPort;
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_16BIT) {
			msgInfo->msgPort.valid = true;
			msgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort16bit.destPort;
			msgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort16bit.originPort;
		}
	}

	memset(msgInfo->msgText, 0x00, sizeof(msgInfo->msgText));
	msgInfo->dataSize = 0;

	if (pTpdu->status <= SMS_STATUS_SMSC_SPECIFIC_LAST) {
		char *msg_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, "IDS_MSGF_BODY_MESSAGE_DELIVERED");
		snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%s", msg_text);
		msgInfo->dataSize = strlen(msgInfo->msgText);
		if (msg_text) {
			free(msg_text);
			msg_text = NULL;
		}
	} else if (pTpdu->status == SMS_STATUS_TEMP_SERVICE_REJECTED) {
		char *msg_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, "IDS_MSGF_BODY_MMSDELIVERYMSGREJECTED");
		snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%s", msg_text);
		msgInfo->dataSize = strlen(msgInfo->msgText);
		if (msg_text) {
			free(msg_text);
			msg_text = NULL;
		}
	} else if (pTpdu->status == SMS_STATUS_PERM_MSG_VAL_PERIOD_EXPIRED) {
		char *msg_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, "IDS_MSGF_BODY_MESSAGE_HAS_EXPIRED");
		snprintf(msgInfo->msgText, sizeof(msgInfo->msgText), "%s", msg_text);
		msgInfo->dataSize = strlen(msgInfo->msgText);
		if (msg_text) {
			free(msg_text);
			msg_text = NULL;
		}
	} else {
		strncpy(msgInfo->msgText, "Message delivery failed.", MAX_MSG_TEXT_LEN);
		msgInfo->dataSize = strlen(msgInfo->msgText);
	}
}


MSG_SUB_TYPE_T SmsPluginEventHandler::convertMsgSubType(SMS_PID_T pid)
{
	switch (pid) {
	case SMS_PID_TYPE0 :
		return MSG_TYPE0_SMS;
	case SMS_PID_REPLACE_TYPE1 :
		return MSG_REPLACE_TYPE1_SMS;
	case SMS_PID_REPLACE_TYPE2 :
		return MSG_REPLACE_TYPE2_SMS;
	case SMS_PID_REPLACE_TYPE3 :
		return MSG_REPLACE_TYPE3_SMS;
	case SMS_PID_REPLACE_TYPE4 :
		return MSG_REPLACE_TYPE4_SMS;
	case SMS_PID_REPLACE_TYPE5 :
		return MSG_REPLACE_TYPE5_SMS;
	case SMS_PID_REPLACE_TYPE6 :
		return MSG_REPLACE_TYPE6_SMS;
	case SMS_PID_REPLACE_TYPE7 :
		return MSG_REPLACE_TYPE7_SMS;
	case SMS_PID_RETURN_CALL :
		return MSG_MWI_OTHER_SMS;
	default :
		return MSG_NORMAL_SMS;
	}
}


void SmsPluginEventHandler::SetSentInfo(SMS_SENT_INFO_S *pSentInfo)
{
	memset(&sentInfo, 0x00, sizeof(SMS_SENT_INFO_S));
	memcpy(&sentInfo, pSentInfo, sizeof(SMS_SENT_INFO_S));

	MSG_DEBUG("sentInfo.reqId : %d", sentInfo.reqInfo.reqId);
	MSG_DEBUG("sentInfo.bLast : %d", sentInfo.bLast);
}


void SmsPluginEventHandler::setDeviceStatus(TapiHandle *handle)
{
	if (handle == devHandle) {
		mx.lock();
		devStatus = true;
		cv.signal();
		mx.unlock();
	}
}


bool SmsPluginEventHandler::getDeviceStatus(TapiHandle *handle)
{
	int ret = 0;

	mx.lock();
	devHandle = handle;
	ret = cv.timedwait(mx.pMutex(), MAX_TAPI_SIM_API_TIMEOUT);

	if (ret == ETIMEDOUT) {
		MSG_WARN("WARNING: DEVICE STATUS TIME-OUT");
		devStatus = false;
	}
	devHandle = NULL;
	mx.unlock();
	return devStatus;
}


msg_error_t SmsPluginEventHandler::handleSimMsg(MSG_MESSAGE_INFO_S *pMsgInfo, int *simIdList, msg_message_id_t *retMsgId, int listSize)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/** Callback to MSG FW */
	err = listener.pfSimMsgIncomingCb(pMsgInfo, simIdList, retMsgId, listSize);

	MSG_END();

	return err;
}

msg_error_t SmsPluginEventHandler::updateIMSI(int sim_idx)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/** Callback to MSG FW */
	err = listener.pfSimInitImsiCb(sim_idx);

	MSG_END();

	return err;
}

void SmsPluginEventHandler::handleSimMemoryFull(int simIndex)
{
	char keyName[MAX_VCONFKEY_NAME_LEN];
	bool bSimSst = true;
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SERVICE_TABLE, simIndex);
	if (MsgSettingGetBool(keyName, &bSimSst) != MSG_SUCCESS)
		MSG_ERR("MsgSettingGetBool [%s] failed", keyName);

	if (bSimSst == true)
		MsgInsertTicker("Sim memory full. Delete some items", SMS_MESSAGE_SIM_MESSAGE_FULL, true, 0);
}
