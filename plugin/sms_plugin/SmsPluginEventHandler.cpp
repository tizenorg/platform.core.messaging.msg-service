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

#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"
#include "SmsPluginTransport.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginStorage.h"
#include "SmsPluginSetting.h"
#include "SmsPluginConcatHandler.h"
#include "SmsPluginEventHandler.h"

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginEventHandler - Member Functions
==================================================================================================*/
SmsPluginEventHandler* SmsPluginEventHandler::pInstance = NULL;


SmsPluginEventHandler::SmsPluginEventHandler()
{
	/**  Initialize global parameters */
	memset(&listener, 0x00, sizeof(MSG_PLUGIN_LISTENER_S));

	pSimCnt = NULL;
	devStatus = false;
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

	SmsPluginTransport::instance()->setNetStatus(NetStatus);

	if (sentInfo.bLast == true || NetStatus != MSG_NETWORK_SEND_SUCCESS) {
		/** Update Msg Status */
		if (sentInfo.reqInfo.msgInfo.msgPort.valid == false){
			SmsPluginStorage::instance()->updateSentMsg(&(sentInfo.reqInfo.msgInfo), NetStatus);
			sentInfo.reqInfo.msgInfo.networkStatus = NetStatus;
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


void SmsPluginEventHandler::handleMsgIncoming(SMS_TPDU_S *pTpdu)
{
	/** Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;

	/** initialize msgInfo */
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	/** check unsupported message */
	if (pTpdu->tpduType == SMS_TPDU_DELIVER) {
		if (pTpdu->data.deliver.dcs.codingScheme == SMS_CHARSET_8BIT && pTpdu->data.deliver.pid == 0x11) {
			MSG_DEBUG("Unsupported message!!");
			SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS);
			return;
		}
	}

	/** convert to msgInfo */
	convertTpduToMsginfo(pTpdu, &msgInfo);

	/** Short Message Type 0 - Just Send Deliver Report */
	if (msgInfo.msgType.subType == MSG_TYPE0_SMS) {
		SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS);
		return;
	}

	/** Print MSG_MESSAGE_INFO_S */
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.nAddressCnt : %d", msgInfo.nAddressCnt);
	MSG_DEBUG("msgInfo.addressList[0].addressType : %d", msgInfo.addressList[0].addressType);
	MSG_DEBUG("msgInfo.addressList[0].addressVal : %s", msgInfo.addressList[0].addressVal);
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.direction : %d", msgInfo.direction);
	MSG_DEBUG("msgInfo.msgType.mainType : %d", msgInfo.msgType.mainType);
	MSG_DEBUG("msgInfo.msgType.subType : %d", msgInfo.msgType.subType);
	MSG_DEBUG("msgInfo.msgType.classType : %d", msgInfo.msgType.classType);
	MSG_DEBUG("msgInfo.displayTime : %s", ctime(&msgInfo.displayTime));
	MSG_DEBUG("msgInfo.msgPort.valid : %d", msgInfo.msgPort.valid);
	MSG_DEBUG("msgInfo.encodeType : %d", msgInfo.encodeType);
	MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);

	if (msgInfo.bTextSms == true) {
		MSG_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
	} else {
		MSG_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
	}

	MSG_DEBUG("###############################################################");

	msg_error_t err = MSG_SUCCESS;

	/** Update Status in Report Table */
	if (msgInfo.msgType.subType == MSG_STATUS_REPORT_SMS) {
		err = SmsPluginStorage::instance()->addMessage(&msgInfo);

		if (err == MSG_SUCCESS) {
			MSG_DEBUG("callback to msg fw");
			err = listener.pfMsgIncomingCb(&msgInfo);
		}

		/** Handling of Fail Case ?? */
		SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS);
	} else { /** SMS Deliver */
		/** Class 2 Msg */
		if (msgInfo.msgType.classType == MSG_CLASS_2) {
			if (msgInfo.bTextSms == false) { /** Concat Msg cannot be saved in SIM */
				msgInfo.msgType.classType = MSG_CLASS_NONE;
				msgInfo.storageId = MSG_STORAGE_PHONE;
			}
		}

		if (msgInfo.msgPort.valid == false) {
			err = SmsPluginStorage::instance()->addMessage(&msgInfo);
		}

		/** Callback to MSG FW */
		if (msgInfo.msgType.classType != MSG_CLASS_2) {
			if (err == MSG_SUCCESS) {
				MSG_DEBUG("callback to msg fw");
				err = listener.pfMsgIncomingCb(&msgInfo);
			}

			/** Send Deliver Report */
			SmsPluginTransport::instance()->sendDeliverReport(err);
		}
	}
}

void SmsPluginEventHandler::handlePushMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen, char *application_id)
{
	MSG_PUSH_MESSAGE_DATA_S pushData;

	memset(&pushData, 0x00, sizeof(MSG_PUSH_MESSAGE_DATA_S));

	/** set PUSH data */
	memcpy(&pushData.pushHeader, pPushHeader, strlen(pPushHeader));

	pushData.pushBodyLen = pushBodyLen;
	memcpy(pushData.pushBody, pPushBody, pushBodyLen);

	memcpy(pushData.pushAppId, application_id, MAX_WAPPUSH_ID_LEN);

	/** Callback to MSG FW */
	listener.pfPushMsgIncomingCb(&pushData);
}


void SmsPluginEventHandler::handleSyncMLMsgIncoming(msg_syncml_message_type_t msgType, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen)
{
	MSG_SYNCML_MESSAGE_DATA_S syncMLData;

	memset(&syncMLData, 0x00, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	/** set syncML data */
	syncMLData.syncmlType = msgType;

	syncMLData.pushBodyLen = PushBodyLen;
	memcpy(syncMLData.pushBody, pPushBody, PushBodyLen);

	syncMLData.wspHeaderLen= WspHeaderLen;
	memcpy(syncMLData.wspHeader, pWspHeader, WspHeaderLen);

	/** Callback to MSG FW */
	listener.pfSyncMLMsgIncomingCb(&syncMLData);
}


void SmsPluginEventHandler::handleLBSMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen)
{
	MSG_LBS_MESSAGE_DATA_S lBSData;

	memset(&lBSData, 0x00, sizeof(MSG_LBS_MESSAGE_DATA_S));

	/** set LBA data */
	memcpy(&lBSData.pushHeader, pPushHeader, strlen(pPushHeader));

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

msg_error_t SmsPluginEventHandler::callbackCBMsgIncoming(MSG_CB_MSG_S *pCbMsg)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/** Callback to MSG FW */
	err = listener.pfCBMsgIncomingCb(pCbMsg);

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
	/** Callback to MSG FW */
	listener.pfStorageChangeCb(storageChangeType, pMsgInfo);

	return MSG_SUCCESS;
}


void SmsPluginEventHandler::convertTpduToMsginfo(SMS_TPDU_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	memset(msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	switch(pTpdu->tpduType)
	{
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
	msgInfo->folderId = MSG_OUTBOX_ID;

	switch(pTpdu->dcs.msgClass)
	{
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
	}

	msgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MT;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	/** What kind of time has to be saved?? (temporary store time) */

	time_t curTime;
	localtime(&curTime);

	msgInfo->displayTime = curTime;

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;
	msgInfo->addressList[addressListCnt].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(msgInfo->addressList[addressListCnt].addressVal, pTpdu->destAddress.address, MAX_ADDRESS_VAL_LEN);

	/**exception operation for none userdata */
	if (pTpdu->userData.length == 0) {
		sprintf(msgInfo->msgText, "[Broken Message]");
		msgInfo->dataSize = strlen(msgInfo->msgText);
		return;
	}

	/** Convert Data values */
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		MSG_LANG_INFO_S langInfo = {0,};

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		msgInfo->dataSize = textCvt.convertGSM7bitToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		msgInfo->dataSize = textCvt.convertUCS2ToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
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

	msgInfo->storageId = MSG_STORAGE_PHONE;

	switch(pTpdu->dcs.msgClass)
	{
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
		msgInfo->msgType.subType = (pTpdu->dcs.indType + MSG_MWI_VOICE_SMS);
	}

	msgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MT;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	time_t rawtime = time(NULL);

	msgInfo->displayTime = rawtime;

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;
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

			if (pTpdu->userData.header[i].udh.specialInd.waitMsgNum > 0) {
				SmsPluginSetting::instance()->setMwiInfo(msgInfo->msgType.subType, pTpdu->userData.header[i].udh.specialInd.waitMsgNum);
			}

			if (pTpdu->userData.length == 0) {
				switch (msgInfo->msgType.subType) {
				case MSG_MWI_VOICE_SMS :
					sprintf(msgInfo->msgText, "%d new voice message", pTpdu->userData.header[i].udh.specialInd.waitMsgNum);
					break;
				case MSG_MWI_FAX_SMS :
					sprintf(msgInfo->msgText, "%d new fax message", pTpdu->userData.header[i].udh.specialInd.waitMsgNum);
					break;
				case MSG_MWI_EMAIL_SMS :
					sprintf(msgInfo->msgText, "%d new email message", pTpdu->userData.header[i].udh.specialInd.waitMsgNum);
					break;
				default :
					sprintf(msgInfo->msgText, "%d new special message", pTpdu->userData.header[i].udh.specialInd.waitMsgNum);
					break;
				}
				msgInfo->dataSize = strlen(msgInfo->msgText);
				return;
			}
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_ALTERNATE_REPLY_ADDRESS) {
			strncpy(msgInfo->addressList[0].addressVal, pTpdu->userData.header[i].udh.alternateAddress.address, MAX_ADDRESS_VAL_LEN);
		}
	}

	/**length 0 - no user data - msg should be received */
	if (pTpdu->userData.length <= 0) {
		memset(msgInfo->msgText, 0x00, sizeof(msgInfo->msgText));
		msgInfo->dataSize = 0;

		switch(pTpdu->dcs.codingScheme)
		{
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
		}

		return;
	} else if (pTpdu->userData.length > MAX_MSG_TEXT_LEN) {
		sprintf(msgInfo->msgText, "[Broken Message]");
		msgInfo->dataSize = strlen(msgInfo->msgData);
		return;
	}

	/** Convert Data values */
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		MSG_LANG_INFO_S langInfo = {0,};

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		for (int i = 0; i < pTpdu->userData.headerCnt; i++) 	{
			if (pTpdu->userData.header[i].udhType == SMS_UDH_SINGLE_SHIFT) {
				langInfo.bSingleShift = true;
				langInfo.singleLang = pTpdu->userData.header[i].udh.singleShift.langId;
			} else if (pTpdu->userData.header[i].udhType == SMS_UDH_LOCKING_SHIFT) {
				langInfo.bLockingShift = true;
				langInfo.lockingLang = pTpdu->userData.header[i].udh.lockingShift.langId;
			}
		}

		msgInfo->encodeType = MSG_ENCODE_GSM7BIT;
		msgInfo->dataSize = textCvt.convertGSM7bitToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_8BIT) {
		msgInfo->encodeType = MSG_ENCODE_8BIT;
		memcpy(msgInfo->msgText, pTpdu->userData.data, pTpdu->userData.length);
		msgInfo->dataSize = pTpdu->userData.length;
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		msgInfo->encodeType = MSG_ENCODE_UCS2;
		msgInfo->dataSize = textCvt.convertUCS2ToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_EUCKR) {
		msgInfo->encodeType = MSG_ENCODE_8BIT;
		msgInfo->dataSize = textCvt.convertEUCKRToUTF8((unsigned char*)msgInfo->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)pTpdu->userData.data, pTpdu->userData.length);
		return;
	}

	MSG_END();
}


void SmsPluginEventHandler::convertStatusRepTpduToMsginfo(const SMS_STATUS_REPORT_S *pTpdu, MSG_MESSAGE_INFO_S *msgInfo)
{
	/** Convert Type  values */
	msgInfo->msgType.mainType = MSG_SMS_TYPE;
	msgInfo->msgType.subType = MSG_STATUS_REPORT_SMS;

	/** set folder id (temporary) */
	msgInfo->folderId = MSG_INBOX_ID;

	switch(pTpdu->dcs.msgClass)
	{
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
	}

	MSG_DEBUG("delivery status : [%d]", pTpdu->status);

	if (pTpdu->status == SMS_STATUS_RECEIVE_SUCCESS) {
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_SUCCESS;
	} else {
		msgInfo->networkStatus = MSG_NETWORK_DELIVER_FAIL;
	}

	msgInfo->bRead = false;
	msgInfo->bProtected = false;
	msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msgInfo->direction = MSG_DIRECTION_TYPE_MT;
	msgInfo->bTextSms = true;

	memset(msgInfo->subject, 0x00, MAX_SUBJECT_LEN+1);

	time_t rawtime = time(NULL);

	msgInfo->displayTime = rawtime;

	/** Convert Address values */
	msgInfo->nAddressCnt = 1;
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

	memset(msgInfo->msgText, 0x00, sizeof(MAX_MSG_TEXT_LEN+1));
	msgInfo->dataSize = 0;

	if (pTpdu->status <= SMS_STATUS_SMSC_SPECIFIC_LAST) {
		strncpy(msgInfo->msgText, "Message delivered.", MAX_MSG_TEXT_LEN);
		msgInfo->dataSize = strlen(msgInfo->msgText);
	} else if (pTpdu->status == SMS_STATUS_TEMP_SERVICE_REJECTED) {
		strncpy(msgInfo->msgText, "Message delivery rejected.", MAX_MSG_TEXT_LEN);
		msgInfo->dataSize = strlen(msgInfo->msgText);
	} else if (pTpdu->status == SMS_STATUS_PERM_MSG_VAL_PERIOD_EXPIRED) {
		strncpy(msgInfo->msgText, "Message delivery expired.", MAX_MSG_TEXT_LEN);
		msgInfo->dataSize = strlen(msgInfo->msgText);
	} else {
		strncpy(msgInfo->msgText, "Message delivery failed.", MAX_MSG_TEXT_LEN);
		msgInfo->dataSize = strlen(msgInfo->msgText);
	}
}


MSG_SUB_TYPE_T SmsPluginEventHandler::convertMsgSubType(SMS_PID_T pid)
{
	switch (pid)
	{
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


void SmsPluginEventHandler::setDeviceStatus()
{
	mx.lock();
	devStatus = true;
	cv.signal();
	mx.unlock();
}


bool SmsPluginEventHandler::getDeviceStatus()
{
	int ret = 0;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 16);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: DEVICE STATUS TIME-OUT");
		devStatus = false;
	}

	return devStatus;
}
