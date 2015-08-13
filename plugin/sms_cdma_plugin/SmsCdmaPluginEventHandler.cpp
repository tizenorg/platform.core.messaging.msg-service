/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"
#include "MsgUtilStorage.h"
#include "MsgCppTypes.h"
#include "MsgContact.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"

#include "SmsCdmaPluginTransport.h"
#include "SmsCdmaPluginStorage.h"
#include "SmsCdmaPluginSetting.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginWapPushHandler.h"



SmsPluginEventHandler* SmsPluginEventHandler::pInstance = NULL;


SmsPluginEventHandler::SmsPluginEventHandler()
{
	/**  Initialize global parameters */
	memset(&listener, 0x00, sizeof(MSG_PLUGIN_LISTENER_S));
	memset(&sentInfo, 0x00, sizeof(sms_sent_info_s));
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


void SmsPluginEventHandler::convertTpduToMsginfo(sms_trans_p2p_msg_s *p_p2p_msg, MSG_MESSAGE_INFO_S *p_msg_info)
{
	MSG_BEGIN();

	// Address
	if (p_p2p_msg->telesvc_msg.data.deliver.callback_number.szData[0] != '\0')
		p_msg_info->nAddressCnt = 2;
	else
		p_msg_info->nAddressCnt = 1;

	p_msg_info->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * p_msg_info->nAddressCnt];

	if (p_msg_info->addressList) {
		switch (p_msg_info->nAddressCnt) {
			case 2 :
				memset(p_msg_info->addressList[1].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
			case 1 :
				memset(p_msg_info->addressList[0].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
				break;
			default :
				MSG_ERR("Invalid case");
				memset(p_msg_info->addressList[0].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
		}
	}

	if (p_p2p_msg->address.szData[0] != '\0') {
		p_msg_info->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
		memcpy(p_msg_info->addressList[0].addressVal, p_p2p_msg->address.szData, MAX_ADDRESS_VAL_LEN);
		p_msg_info->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;
	}


	// Teleservice message
	switch(p_p2p_msg->telesvc_msg.type)
	{
		case SMS_TYPE_DELIVER :
			p_msg_info->msgType.subType = MSG_NORMAL_SMS;
			p_msg_info->folderId = MSG_INBOX_ID;

			convertDeliverMsgToMsgInfo(&(p_p2p_msg->telesvc_msg.data.deliver), p_msg_info);
			break;
		case SMS_TYPE_DELIVERY_ACK :
			convertAckMsgToMsgInfo(&(p_p2p_msg->telesvc_msg.data.delivery_ack), p_msg_info);
			break;
		case SMS_TYPE_SUBMIT_REPORT :
			convertReportMsgToMsgInfo(&(p_p2p_msg->telesvc_msg.data.report), p_msg_info);
			break;
		default :
			MSG_DEBUG("No matching type = [%d]", p_p2p_msg->telesvc_msg.type);
			break;
	}

	MSG_END();
}


void SmsPluginEventHandler::convertTpduToMsginfo(sms_trans_broadcast_msg_s *p_cb_msg, MSG_MESSAGE_INFO_S *p_msg_info)
{
	MSG_BEGIN();

	// Address
	p_msg_info->nAddressCnt = 0;
	p_msg_info->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];

	if (p_msg_info->addressList) {
		memset(p_msg_info->addressList[0].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
	}

	// Bearer Data
	if(p_cb_msg->telesvc_msg.type == SMS_TYPE_DELIVER) {
		p_msg_info->msgType.subType = MSG_CB_SMS;
		p_msg_info->folderId = MSG_CBMSGBOX_ID;

		if (p_cb_msg->svc_ctg >= SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL && p_cb_msg->svc_ctg <= SMS_TRANS_SVC_CTG_CMAS_TEST)
			convertCMASMsgToMsgInfo(&(p_cb_msg->telesvc_msg.data.deliver), p_msg_info);
		else
			convertDeliverMsgToMsgInfo(&(p_cb_msg->telesvc_msg.data.deliver), p_msg_info);
	}
	else {
		MSG_DEBUG("No matching type = [%d]", p_cb_msg->telesvc_msg.type);
	}

	if (p_msg_info->nAddressCnt == 0) {
		p_msg_info->nAddressCnt = 1;
		p_msg_info->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
		p_msg_info->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;

		if (p_cb_msg->svc_ctg == SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL)
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "Presidential Alert");
		else if (p_cb_msg->svc_ctg == SMS_TRANS_SVC_CTG_CMAS_EXTREME)
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "Extreme Alert");
		else if (p_cb_msg->svc_ctg == SMS_TRANS_SVC_CTG_CMAS_SEVERE)
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "Severe Alert");
		else if (p_cb_msg->svc_ctg == SMS_TRANS_SVC_CTG_CMAS_AMBER)
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "AMBER Alert");
		else if (p_cb_msg->svc_ctg == SMS_TRANS_SVC_CTG_CMAS_TEST)
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "Emergency Alert");
		else
			snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "CB Message");
	}

	MSG_END();
}


void SmsPluginEventHandler::convertCMASMsgToMsgInfo(sms_telesvc_deliver_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info)
{
	MSG_BEGIN();

	p_msg_info->msgType.mainType = MSG_SMS_TYPE;

	p_msg_info->msgType.classType = MSG_CLASS_NONE;
	p_msg_info->networkStatus = MSG_NETWORK_RECEIVED;
	p_msg_info->bRead = false;
	p_msg_info->bProtected = false;
	p_msg_info->direction = MSG_DIRECTION_TYPE_MT;
	p_msg_info->bTextSms = true;

	if (p_deliver->callback_number.szData[0] != '\0') {			// If callback number is in received pdu, replace the address value.
		memset(p_msg_info->addressList[0].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
		p_msg_info->nAddressCnt = 1;
		p_msg_info->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
		snprintf(p_msg_info->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN, "%s", p_deliver->callback_number.szData);
		p_msg_info->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;
	}

	time_t rawtime = 0;
	p_msg_info->storageId = MSG_STORAGE_PHONE;

#if 0 // Save Timestamp of message center.
		char displayTime[32];
		struct tm * timeTM;

		struct tm timeinfo;
		memset(&timeinfo, 0x00, sizeof(tm));

		if (p_deliver->time_stamp.year > 95)
			timeinfo.tm_year = (p_deliver->time_stamp.year + 1900);
		else
			timeinfo.tm_year = (p_deliver->time_stamp.year + 2000);

		timeinfo.tm_mon = (p_deliver->time_stamp.month - 1);
		timeinfo.tm_mday = p_deliver->time_stamp.day;
		timeinfo.tm_hour = p_deliver->time_stamp.hour;
		timeinfo.tm_min = p_deliver->time_stamp.minute;
		timeinfo.tm_sec = p_deliver->time_stamp.second;
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

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);

		rawtime -= timezone;

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);
	}
#else
	rawtime = time(NULL);
#endif

	p_msg_info->displayTime = rawtime;


	if (p_deliver->priority == SMS_PRIORITY_URGENT || p_deliver->priority == SMS_PRIORITY_EMERGENCY)
		p_msg_info->priority = MSG_MESSAGE_PRIORITY_HIGH;
	else
		p_msg_info->priority = MSG_MESSAGE_PRIORITY_NORMAL;


	memset(p_msg_info->subject, 0x00, MAX_SUBJECT_LEN+1);

	p_msg_info->msgPort.valid = false;
	p_msg_info->msgPort.dstPort = 0;
	p_msg_info->msgPort.srcPort = 0;

	p_msg_info->encodeType = getEncodeType(p_deliver->cmas_data.encode_type);

	if (p_deliver->cmas_data.data_len <= 0) {
		memset(p_msg_info->msgText, 0x00, sizeof(p_msg_info->msgText));
		p_msg_info->dataSize = 0;
	} else if (p_deliver->cmas_data.data_len > MAX_MSG_TEXT_LEN) {
		sprintf(p_msg_info->msgText, "[Broken Message]");
		p_msg_info->dataSize = strlen(p_msg_info->msgData);
		return;
	} else {
		if(p_msg_info->encodeType == MSG_ENCODE_UCS2) {
			MSG_DEBUG("Encode Type = UCS2");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertUCS2ToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->cmas_data.alert_text, p_deliver->cmas_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_EUCKR) {
			MSG_DEBUG("Encode Type = EUCKR");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertEUCKRToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->cmas_data.alert_text, p_deliver->cmas_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_SHIFT_JIS) {
			MSG_DEBUG("Encode Type = Shift-JIS");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertSHIFTJISToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->cmas_data.alert_text, p_deliver->cmas_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_GSM7BIT) {
			MSG_DEBUG("Encode Type = GSM7BIT");
			MSG_LANG_INFO_S langinfo = {0,};
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertGSM7bitToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->cmas_data.alert_text, p_deliver->cmas_data.data_len, &langinfo);
		}
		else {
			snprintf(p_msg_info->msgText, sizeof(p_msg_info->msgText), "%s", p_deliver->cmas_data.alert_text);
			p_msg_info->dataSize = p_deliver->cmas_data.data_len;
		}
	}

	MSG_END();
}


void SmsPluginEventHandler::convertDeliverMsgToMsgInfo(sms_telesvc_deliver_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info)
{
	MSG_BEGIN();

	p_msg_info->msgType.mainType = MSG_SMS_TYPE;
//	p_msg_info->msgType.subType = MSG_NORMAL_SMS;

//	p_msg_info->folderId = MSG_INBOX_ID;

	p_msg_info->msgType.classType = MSG_CLASS_NONE;
	p_msg_info->networkStatus = MSG_NETWORK_RECEIVED;
	p_msg_info->bRead = false;
	p_msg_info->bProtected = false;
	p_msg_info->direction = MSG_DIRECTION_TYPE_MT;
	p_msg_info->bTextSms = true;

	if (p_deliver->callback_number.szData[0] != '\0') {			// If callback number is in received pdu, replace the address value.
		memset(p_msg_info->addressList[1].addressVal, 0x00, MAX_ADDRESS_VAL_LEN+1);
		p_msg_info->addressList[1].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
		memcpy(p_msg_info->addressList[1].addressVal, p_deliver->callback_number.szData, MAX_ADDRESS_VAL_LEN);
		p_msg_info->addressList[1].recipientType = MSG_RECIPIENTS_TYPE_TO;
	}

	time_t rawtime = 0;
	p_msg_info->storageId = MSG_STORAGE_PHONE;

#if 0 // Save Timestamp of message center.
		char displayTime[32];
		struct tm * timeTM;

		struct tm timeinfo;
		memset(&timeinfo, 0x00, sizeof(tm));

		if (p_deliver->time_stamp.year > 95)
			timeinfo.tm_year = (p_deliver->time_stamp.year + 1900);
		else
			timeinfo.tm_year = (p_deliver->time_stamp.year + 2000);

		timeinfo.tm_mon = (p_deliver->time_stamp.month - 1);
		timeinfo.tm_mday = p_deliver->time_stamp.day;
		timeinfo.tm_hour = p_deliver->time_stamp.hour;
		timeinfo.tm_min = p_deliver->time_stamp.minute;
		timeinfo.tm_sec = p_deliver->time_stamp.second;
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

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);

		rawtime -= timezone;

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);
	}
#else
	rawtime = time(NULL);
#endif

	p_msg_info->displayTime = rawtime;


	if (p_deliver->priority == SMS_PRIORITY_URGENT || p_deliver->priority == SMS_PRIORITY_EMERGENCY)
		p_msg_info->priority = MSG_MESSAGE_PRIORITY_HIGH;
	else
		p_msg_info->priority = MSG_MESSAGE_PRIORITY_NORMAL;


	memset(p_msg_info->subject, 0x00, MAX_SUBJECT_LEN+1);

	p_msg_info->msgPort.valid = false;
	p_msg_info->msgPort.dstPort = 0;
	p_msg_info->msgPort.srcPort = 0;

	p_msg_info->encodeType = getEncodeType(p_deliver->user_data.encode_type);

	if (p_deliver->user_data.data_len <= 0) {
		memset(p_msg_info->msgText, 0x00, sizeof(p_msg_info->msgText));
		p_msg_info->dataSize = 0;
	} else if (p_deliver->user_data.data_len > MAX_MSG_TEXT_LEN) {
		sprintf(p_msg_info->msgText, "[Broken Message]");
		p_msg_info->dataSize = strlen(p_msg_info->msgData);
		return;
	} else {
		if(p_msg_info->encodeType == MSG_ENCODE_UCS2) {
			MSG_DEBUG("Encode Type = UCS2");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertUCS2ToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->user_data.user_data, p_deliver->user_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_EUCKR) {
			MSG_DEBUG("Encode Type = EUCKR");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertEUCKRToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->user_data.user_data, p_deliver->user_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_SHIFT_JIS) {
			MSG_DEBUG("Encode Type = Shift-JIS");
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertSHIFTJISToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->user_data.user_data, p_deliver->user_data.data_len);
		}
		else if(p_msg_info->encodeType == MSG_ENCODE_GSM7BIT) {
			MSG_DEBUG("Encode Type = GSM7BIT");
			MSG_LANG_INFO_S langinfo = {0,};
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			p_msg_info->dataSize = textCvt->convertGSM7bitToUTF8((unsigned char*)&p_msg_info->msgText, MAX_MSG_TEXT_LEN, (unsigned char*)&p_deliver->user_data.user_data, p_deliver->user_data.data_len, &langinfo);
		}
		else {
			snprintf(p_msg_info->msgText, sizeof(p_msg_info->msgText), "%s", p_deliver->user_data.user_data);
			p_msg_info->dataSize = p_deliver->user_data.data_len;
		}
	}

	MSG_END();
}


void SmsPluginEventHandler::convertAckMsgToMsgInfo(sms_telesvc_deliver_ack_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info)
{

}


void SmsPluginEventHandler::convertReportMsgToMsgInfo(sms_telesvc_report_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info)
{

}


void SmsPluginEventHandler::SetSentInfo(sms_sent_info_s *pSentInfo)
{
	memset(&sentInfo, 0x00, sizeof(sms_sent_info_s));
	memcpy(&sentInfo, pSentInfo, sizeof(sms_sent_info_s));

	MSG_DEBUG("sentInfo.reqId : %d", sentInfo.reqInfo.reqId);
	MSG_DEBUG("sentInfo.bLast : %d", sentInfo.bLast);
}



void SmsPluginEventHandler::handleSentStatus(msg_network_status_t NetStatus)
{
	MSG_DEBUG("NetStatus[%d]", NetStatus);

	if (sentInfo.bLast == true || NetStatus != MSG_NETWORK_SEND_SUCCESS) {
		/** Update Msg Status */
		if (sentInfo.reqInfo.msgInfo.msgPort.valid == false) {

			sentInfo.reqInfo.msgInfo.networkStatus = NetStatus;

			if (NetStatus == MSG_NETWORK_SEND_SUCCESS) {
				MSG_DEBUG("Add phone log");
				MsgAddPhoneLog(&(sentInfo.reqInfo.msgInfo));
				sentInfo.reqInfo.msgInfo.folderId = MSG_SENTBOX_ID;
			} else {
				sentInfo.reqInfo.msgInfo.bRead = false;
			}

			callbackStorageChange(MSG_STORAGE_CHANGE_UPDATE, &(sentInfo.reqInfo.msgInfo));
		}

		MSG_DEBUG("sentInfo.reqInfo.sendOptInfo.bSetting [%d]", sentInfo.reqInfo.sendOptInfo.bSetting);
		MSG_DEBUG("sentInfo.reqInfo.sendOptInfo.bKeepCopy [%d]", sentInfo.reqInfo.sendOptInfo.bKeepCopy);
		/** Check sending options */
		if (sentInfo.reqInfo.sendOptInfo.bSetting && !sentInfo.reqInfo.sendOptInfo.bKeepCopy && NetStatus == MSG_NETWORK_SEND_SUCCESS) {
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


void SmsPluginEventHandler::handleMsgIncoming(sms_trans_p2p_msg_s *p_p2p_msg)
{

	/** Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;

	/** initialize msgInfo */
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	/** convert to msgInfo */
	convertTpduToMsginfo(p_p2p_msg, &msgInfo);

	if (p_p2p_msg->telesvc_id == SMS_TRANS_TELESVC_RESERVED) {
		MSG_DEBUG("This Incoming Message has Unknown Teleservice ID");
		SmsPluginTransport::instance()->sendDeliverReport(MSG_ERR_INVALID_MSG_TYPE, p_p2p_msg);
		return;
	}

	/** Check for Voice Mail Notification */
	if (p_p2p_msg->telesvc_id == SMS_TRANS_TELESVC_VMN_95) {
		if (p_p2p_msg->telesvc_msg.data.deliver.enhanced_vmn.fax_included)
			msgInfo.msgType.subType = MSG_MWI_FAX_SMS;
		else
			msgInfo.msgType.subType = MSG_MWI_VOICE_SMS;

		if (p_p2p_msg->telesvc_msg.data.deliver.num_msg < 0)
			p_p2p_msg->telesvc_msg.data.deliver.num_msg = 0;

		int voice_cnt = MsgSettingGetInt(VOICEMAIL_COUNT);

		// repeated msg check for voicemail
		if (voice_cnt == p_p2p_msg->telesvc_msg.data.deliver.num_msg) {
			SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS, p_p2p_msg);
			return;
		}

		SmsPluginSetting::instance()->setMwiInfo(msgInfo.msgType.subType, p_p2p_msg->telesvc_msg.data.deliver.num_msg);

		memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));

		snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%d", p_p2p_msg->telesvc_msg.data.deliver.num_msg);
		msgInfo.dataSize = strlen(msgInfo.msgText);
	}
	/** Check for EMS(Unsupported) */
	else if (p_p2p_msg->telesvc_id == SMS_TRANS_TELESVC_WEMT) {
		char *msg_text = getTranslateText(MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR, "IDS_MSGF_POP_ERROR_UNSUPPORTED_MSG");
		memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));
		snprintf(msgInfo.msgText, sizeof(msgInfo.msgText), "%s", msg_text);
		msgInfo.dataSize = strlen(msgInfo.msgText);

		if (msg_text) {
			free(msg_text);
			msg_text = NULL;
		}
	}

	/** Print MSG_MESSAGE_INFO_S */
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.nAddressCnt : %d", msgInfo.nAddressCnt);
	MSG_DEBUG("msgInfo.addressList[0].addressType : %d", msgInfo.addressList[0].addressType);
	MSG_DEBUG("msgInfo.addressList[0].addressVal : %s", msgInfo.addressList[0].addressVal);
	MSG_DEBUG("msgInfo.addressList[0].displayName : %s", msgInfo.addressList[0].displayName);
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.bStore : %d", msgInfo.bStore);
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
	bool isUnique = true;
	MSG_UNIQUE_INDEX_S unq_ind;
	memset(&unq_ind, 0x00, sizeof(MSG_UNIQUE_INDEX_S));

	if (msgInfo.msgType.subType == MSG_STATUS_REPORT_SMS) {
//		/** Status Report Message */
//		err = SmsPluginStorage::instance()->updateMsgDeliverStatus(&msgInfo, pTpdu->data.statusRep.msgRef);
//
//		if (err == MSG_SUCCESS)
//			err = listener.pfMsgIncomingCb(&msgInfo);
//		else
//			MSG_DEBUG("updateMsgDeliverStatus is failed [%d]", err);
//
//		/** Handling of Fail Case ?? */
//		SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS);
	} else { /** SMS Deliver */

		/** Add message to DB */
		if (msgInfo.msgPort.valid == false) {
			if (p_p2p_msg->telesvc_id != SMS_TRANS_TELESVC_VMN_95) {
				memcpy(unq_ind.address, p_p2p_msg->address.szData, sizeof(p_p2p_msg->address.szData));
				memcpy(unq_ind.sub_address, p_p2p_msg->sub_address.szData, sizeof(p_p2p_msg->sub_address.szData));
				unq_ind.tele_msgId = p_p2p_msg->telesvc_msg.data.deliver.msg_id.msg_id;
				snprintf(unq_ind.time_stamp, sizeof(unq_ind.time_stamp), "%02d%02d%02d%02d%02d%02d",
						p_p2p_msg->telesvc_msg.data.deliver.time_stamp.year, p_p2p_msg->telesvc_msg.data.deliver.time_stamp.month,
						p_p2p_msg->telesvc_msg.data.deliver.time_stamp.day, p_p2p_msg->telesvc_msg.data.deliver.time_stamp.hours,
						p_p2p_msg->telesvc_msg.data.deliver.time_stamp.minutes, p_p2p_msg->telesvc_msg.data.deliver.time_stamp.seconds);
				unq_ind.telesvc_id = p_p2p_msg->telesvc_id;
				isUnique = listener.pfCheckUniquenessCb(&unq_ind, 0, false);
			}
		}

		if (isUnique) {
			err = SmsPluginStorage::instance()->checkMessage(&msgInfo);
		}
		else {
			SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS, p_p2p_msg);
			return;
		}

		/** Callback to MSG FW */
		if (err == MSG_SUCCESS) {
			MSG_DEBUG("callback to msg fw");
			err = listener.pfMsgIncomingCb(&msgInfo);
		} else {
			if (msgInfo.msgType.classType == MSG_CLASS_0) {
				MSG_DEBUG("callback for class0 message to msg fw");
				if (listener.pfMsgIncomingCb(&msgInfo) != MSG_SUCCESS)
					MSG_DEBUG("listener.pfMsgIncomingCb is failed!");
			}
		}

		if (err == MSG_SUCCESS && p_p2p_msg->telesvc_id != SMS_TRANS_TELESVC_VMN_95)
			listener.pfCheckUniquenessCb(&unq_ind, msgInfo.msgId, true);

		/** Send Deliver Report */
		if (p_p2p_msg->telesvc_id == SMS_TRANS_TELESVC_WEMT) {
			SmsPluginTransport::instance()->sendDeliverReport(MSG_ERR_INVALID_MSG_TYPE, p_p2p_msg);
		} else {
			SmsPluginTransport::instance()->sendDeliverReport(err, p_p2p_msg);
		}

		// Tizen Validation System
		char *msisdn = NULL;
		char keyName[MAX_VCONFKEY_NAME_LEN];
		int simIndex = 1;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, simIndex);

		msisdn = MsgSettingGetString(keyName);

		MSG_SMS_VLD_INFO("%d, SMS Receive, %s->%s, %s", msgInfo.msgId, \
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


void SmsPluginEventHandler::handleCbMsgIncoming(sms_trans_broadcast_msg_s *p_cb_msg)
{
	MSG_BEGIN();

	if (p_cb_msg->telesvc_msg.data.deliver.cmas_data.encode_type == SMS_ENCODE_KOREAN
		|| p_cb_msg->telesvc_msg.data.deliver.cmas_data.encode_type == SMS_ENCODE_GSMDCS) {
		MSG_DEBUG("This encode type is not supported [%d]", p_cb_msg->telesvc_msg.data.deliver.cmas_data.encode_type);
		return;
	}

	/** Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;

	/** initialize msgInfo */
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	/** convert to msgInfo */
	convertTpduToMsginfo(p_cb_msg, &msgInfo);
	msgInfo.msgId = p_cb_msg->telesvc_msg.data.deliver.msg_id.msg_id;

	/** Print MSG_MESSAGE_INFO_S */
	MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
	MSG_DEBUG("msgInfo.priority : %d", msgInfo.priority);
	MSG_DEBUG("msgInfo.bProtected : %d", msgInfo.bProtected);
	MSG_DEBUG("msgInfo.bRead : %d", msgInfo.bRead);
	MSG_DEBUG("msgInfo.bTextSms : %d", msgInfo.bTextSms);
	MSG_DEBUG("msgInfo.bStore : %d", msgInfo.bStore);
	MSG_DEBUG("msgInfo.direction : %d", msgInfo.direction);
	MSG_DEBUG("msgInfo.msgType.mainType : %d", msgInfo.msgType.mainType);
	MSG_DEBUG("msgInfo.msgType.subType : %d", msgInfo.msgType.subType);
	MSG_DEBUG("msgInfo.msgType.classType : %d", msgInfo.msgType.classType);
	MSG_DEBUG("msgInfo.displayTime : %s", ctime(&msgInfo.displayTime));
	MSG_DEBUG("msgInfo.msgPort.valid : %d", msgInfo.msgPort.valid);
	MSG_DEBUG("msgInfo.encodeType : %d", msgInfo.encodeType);
	MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);
	MSG_DEBUG("msgInfo.msgId : %d", msgInfo.msgId);

	if (msgInfo.bTextSms == true) {
		MSG_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
	} else {
		MSG_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
	}

	MSG_DEBUG("###############################################################");

	msg_error_t err = MSG_SUCCESS;

	/** Add message to DB */
	if (msgInfo.msgPort.valid == false) {
		err = SmsPluginStorage::instance()->checkMessage(&msgInfo);
	}

	if (!checkCbOpt(p_cb_msg->svc_ctg))
	{
		MSG_DEBUG("The CB Msg is not supported by option.");
		return;
	}

	/** Callback to MSG FW */
	if (err == MSG_SUCCESS) {

#if 1
		MSG_CB_MSG_S cbOutMsg = {0, };
		bool is_duplicate = false;

		switch (p_cb_msg->svc_ctg) {
			case SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL :
				msgInfo.msgType.subType = MSG_CMAS_PRESIDENTIAL;
				break;
			case SMS_TRANS_SVC_CTG_CMAS_EXTREME :
				msgInfo.msgType.subType = MSG_CMAS_EXTREME;
				break;
			case SMS_TRANS_SVC_CTG_CMAS_SEVERE :
				msgInfo.msgType.subType = MSG_CMAS_SEVERE;
				break;
			case SMS_TRANS_SVC_CTG_CMAS_AMBER :
				msgInfo.msgType.subType = MSG_CMAS_AMBER;
				break;
			case SMS_TRANS_SVC_CTG_CMAS_TEST :
				msgInfo.msgType.subType = MSG_CMAS_TEST;
				break;
			default :
				msgInfo.msgType.subType = MSG_CB_SMS;
				break;
		}

		cbOutMsg.type = msgInfo.msgType.subType;
		cbOutMsg.receivedTime = msgInfo.displayTime;
		cbOutMsg.serialNum = 0;//encodeCbSerialNum (CbMsgPage.pageHeader.serialNum);
		cbOutMsg.messageId = msgInfo.msgId;
//		cbOutMsg.dcs = CbMsgPage.pageHeader.dcs.rawData;
		memset (cbOutMsg.cbText, 0x00, sizeof(cbOutMsg.cbText));

		cbOutMsg.cbTextLen= msgInfo.dataSize;
		memset(cbOutMsg.language_type, 0x00, sizeof(cbOutMsg.language_type));
//		memcpy(cbOutMsg.language_type, CbMsgPage.pageHeader.dcs.iso639Lang, 3);

		if (!is_duplicate)
		{
			MSG_DEBUG("callback to msg fw");
			err = listener.pfCBMsgIncomingCb(&cbOutMsg, &msgInfo);

			if (err != MSG_SUCCESS)
			{
				MSG_WARN("callbackMsgIncoming() Error !! [%d]", err);
			}
		}
		else
		{
			MSG_WARN("duplicate cb serialNum[%d] messageId[%d]", cbOutMsg.serialNum, cbOutMsg.messageId);
		}

#else
		err = listener.pfMsgIncomingCb(&msgInfo);
#endif
	}

	/** Send Deliver Report */
//	SmsPluginTransport::instance()->sendDeliverReport(err, p_cb_msg);

	// Tizen Validation System
	char *msisdn = NULL;
	char keyName[MAX_VCONFKEY_NAME_LEN];
	int simIndex = 1;

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, simIndex);

	msisdn = MsgSettingGetString(keyName);

	MSG_SMS_VLD_INFO("%d, SMS Receive, %s->%s, %s", msgInfo.msgId, \
													msgInfo.addressList[0].addressVal, \
													(msisdn == NULL)?"ME":msisdn, \
													(err == MSG_SUCCESS)?"Success":"Fail");

	MSG_SMS_VLD_TXT("%d, [%s]", msgInfo.msgId, msgInfo.msgText);

	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}

	MSG_END();
}


void SmsPluginEventHandler::handleWapMsgIncoming(sms_trans_p2p_msg_s *p_p2p_msg)
{
	MSG_BEGIN();

	sms_wap_msg_s msg;
	memset(&msg, 0x00, sizeof(sms_wap_msg_s));

	msg.msgId = p_p2p_msg->telesvc_msg.data.deliver.msg_id.msg_id;
	msg.totalSeg = p_p2p_msg->telesvc_msg.data.deliver.user_data.user_data[1];
	msg.segNum = p_p2p_msg->telesvc_msg.data.deliver.user_data.user_data[2];

	char tmpUserText[SMS_MAX_USER_DATA_LEN+1] = {0,};
	sms_telesvc_userdata_s tmpUserData;
	memset(&tmpUserData, 0x00, sizeof(sms_telesvc_userdata_s));

	tmpUserData.data_len = p_p2p_msg->telesvc_msg.data.deliver.user_data.data_len - 3;
	memcpy(tmpUserText, &(p_p2p_msg->telesvc_msg.data.deliver.user_data.user_data[3]), tmpUserData.data_len);
	memcpy(tmpUserData.user_data, tmpUserText, sizeof(tmpUserData.user_data));

	unsigned char segCnt = checkWapMsg(&msg, &tmpUserData);

	MSG_DEBUG("segNum [%d]", msg.segNum);
	MSG_DEBUG("segCnt [%d]", segCnt);
	MSG_DEBUG("msg.totalSeg [%d]", msg.totalSeg);

	if (segCnt == msg.totalSeg) {
		MSG_DEBUG("RECEIVED LAST WAP : %d", segCnt);

		unsigned short srcPort = 0, dstPort = 0;
		int dataSize = 0;
		char* pUserData = NULL;
		char* pTmpUserData = NULL;
		unique_ptr<char*, void(*)(char**)> dataBuf(&pUserData, unique_ptr_deleter);
		unique_ptr<char*, void(*)(char**)> dataBuf1(&pTmpUserData, unique_ptr_deleter);

		MSG_MESSAGE_INFO_S msgInfo = {0,};

		msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

		dataSize = MakeWapUserData(msg.msgId, &pUserData);

		pTmpUserData = new char[dataSize];

		memcpy(pTmpUserData, pUserData, dataSize);
		memset(pUserData, 0x00, dataSize);

		srcPort = pTmpUserData[0] << 8	| pTmpUserData[1];
		dstPort = pTmpUserData[2] << 8 | pTmpUserData[3];

#ifndef FEATURE_OMADM_DUPLICATE_PORT_WAPPUSH
		dataSize -= 4;
		memcpy(pUserData, &pTmpUserData[4], dataSize);
#else
		if ((srcPort == 0x23f0 && dstPort == 0x0b84) &&
			((pTmpUserData[4] << 8 | pTmpUserData[5]) == 0x23f0) &&
			((pTmpUserData[6] << 8 | pTmpUserData[7]) == 0x0b84)) {
			dataSize -= 8;
			memcpy(pUserData, &pTmpUserData[8], dataSize);
		}
		else {
			dataSize -= 4;
			memcpy(pUserData, &pTmpUserData[4], dataSize);
		}
#endif

		if (dataSize > 0) {
			MSG_DEBUG("dataSize = %d", dataSize);
			for (int i = 0; i < dataSize; i++)
				MSG_DEBUG("UserData[%d] = [%c] [%02x]", i, pUserData[i], pUserData[i]);

			if	(SmsPluginWapPushHandler::instance()->IsWapPushMsg(dstPort) == true) {
				msgInfo.msgType.mainType = MSG_SMS_TYPE;
				SmsPluginWapPushHandler::instance()->copyDeliverData(&(p_p2p_msg->address));
				SmsPluginWapPushHandler::instance()->handleWapPushMsg(pUserData, dataSize);
			}
			else {
				MSG_DEBUG("not supported wap push port [%x]", dstPort);
			}
		}

		// remove from waplist
		for (int index = wapList.size()-1; index >= 0 ; index--) {
			if (wapList[index].msgId == msg.msgId) {
				MSG_DEBUG("remove waplist of the index [%d]", index);
				wapList.erase(wapList.begin()+index);
				break;
			}
		}
	}

	/** Send Deliver Report */
	SmsPluginTransport::instance()->sendDeliverReport(MSG_SUCCESS, p_p2p_msg);

	MSG_END();
}


void SmsPluginEventHandler::handleResendMessage(void)
{
	listener.pfResendMessageCb();
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


msg_error_t SmsPluginEventHandler::callbackStorageChange(msg_storage_change_type_t storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	/** Callback to MSG FW */
	listener.pfStorageChangeCb(storageChangeType, pMsgInfo);

	return MSG_SUCCESS;
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

	ret = cv.timedwait(mx.pMutex(), 25);

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: DEVICE STATUS TIME-OUT");
		devStatus = false;
		bNeedInitConfig = true;
	}
	mx.unlock();
	return devStatus;
}

void SmsPluginEventHandler::setNeedInitConfig(bool bNeeded)
{
	bNeedInitConfig = bNeeded;
}

bool SmsPluginEventHandler::getNeedInitConfig()
{
	return bNeedInitConfig;
}

msg_encode_type_t SmsPluginEventHandler::getEncodeType(sms_encoding_type_t encode_type)
{
	switch(encode_type) {
		case SMS_ENCODE_IA5 :
		case SMS_ENCODE_GSM7BIT :
			return MSG_ENCODE_GSM7BIT;
		case SMS_ENCODE_KOREAN :
		case SMS_ENCODE_EUCKR :
			return MSG_ENCODE_EUCKR;
		case SMS_ENCODE_7BIT_ASCII :
		case SMS_ENCODE_LATIN_HEBREW :
		case SMS_ENCODE_LATIN :
		case SMS_ENCODE_OCTET :
			return MSG_ENCODE_8BIT;
		case SMS_ENCODE_SHIFT_JIS :
			return MSG_ENCODE_SHIFT_JIS;
//		case SMS_ENCODE_EPM :
//		case SMS_ENCODE_UNICODE :
//		case SMS_ENCODE_GSMDCS :
		default :
			return MSG_ENCODE_UCS2;
	}

	return MSG_ENCODE_UCS2;
}


unsigned short SmsPluginEventHandler::checkWapMsg(sms_wap_msg_s *pMsg, sms_telesvc_userdata_s *pUserdata)
{
	unsigned char currSegNum = 0;

	bool bFind = false;

	for (unsigned int i = 0; i < wapList.size(); i++) {
		if (wapList[i].msgId == pMsg->msgId) {
			if (wapList[i].data.count(pMsg->segNum) != 0) {
				MSG_DEBUG("The Segment Number already exists [%d]", pMsg->segNum);
				return 0;
			}
			wap_data_s wapData = {0};

			memcpy(wapData.data, pUserdata->user_data, pUserdata->data_len);
			wapData.length = pUserdata->data_len;

			pair<unsigned char, wap_data_s> newData(pMsg->segNum, wapData);
			wapList[i].data.insert(newData);

			MSG_DEBUG("MSG DATA : %s", pUserdata->user_data);
			MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.data);
			MSG_DEBUG("DATA SIZE [%d]", pUserdata->data_len);

			wapList[i].segNum++;
			wapList[i].totalSize += pUserdata->data_len;
			currSegNum = wapList[i].segNum;

			bFind = true;

			break;
		}
	}

	/** New Wap Push Msg */
	if (bFind == false) {
		sms_wap_info_s tmpInfo;
		tmpInfo.msgId = pMsg->msgId;
		tmpInfo.totalSeg = pMsg->totalSeg;
		tmpInfo.segNum = 1;

		tmpInfo.totalSize = pUserdata->data_len;

		wap_data_s wapData = {0};

		memcpy(wapData.data, pUserdata->user_data, pUserdata->data_len);
		wapData.length = pUserdata->data_len;

		pair<unsigned char, wap_data_s> newData(pMsg->segNum, wapData);
		tmpInfo.data.insert(newData);

		MSG_DEBUG("MSG DATA : %s", pUserdata->user_data);
		MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.data);
		MSG_DEBUG("DATA SIZE [%d]", pUserdata->data_len);

		wapList.push_back(tmpInfo);

		currSegNum = tmpInfo.segNum;
	}

	return currSegNum;
}


int SmsPluginEventHandler::MakeWapUserData(unsigned short msgId, char **ppTotalData)
{
	wapDataMap::iterator it;

	int totalSize = 0, offset = 0;

	for (unsigned int i = 0; i < wapList.size(); i++) {
		if (wapList[i].msgId == msgId) {
			totalSize = wapList[i].totalSize;

			if (totalSize <= 0) {
				MSG_DEBUG("Size Error : totalSize <= 0");
				return 0;
			}

			MSG_DEBUG("totalSize [%d]", totalSize);

			if (*ppTotalData == NULL)
				*ppTotalData = new char[totalSize];

			for (it = wapList[i].data.begin(); it != wapList[i].data.end(); it++) {
				memcpy(*ppTotalData+offset, it->second.data, it->second.length);
				offset += it->second.length;
			}
		}
	}

	return totalSize;

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
	memcpy(lBSData.pushHeader, pPushHeader, strlen(pPushHeader));

	lBSData.pushBodyLen = pushBodyLen;
	memcpy(lBSData.pushBody, pPushBody, pushBodyLen);

	/** Callback to MSG FW */
	listener.pfLBSMsgIncomingCb(&lBSData);
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


bool SmsPluginEventHandler::checkCbOpt(sms_trans_svc_ctg_t svc_ctg)
{
	bool bReceive = false;
	MsgSettingGetBool(CB_RECEIVE, &bReceive);

	// Receive CB Msg = FALSE
	if (!bReceive)
	{
		MSG_DEBUG("RECEIVE CB = FALSE");
		return false;
	}

	if (svc_ctg >= SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL && svc_ctg <= SMS_TRANS_SVC_CTG_CMAS_TEST) {
		bool bActivate = false;
		short Category = 0;
		MSG_CB_CHANNEL_S cbChannelInfo = {0,};
		msg_error_t err = MSG_SUCCESS;
		MsgDbHandler *dbHandle = getDbHandle();

		err = MsgStoGetCBChannelInfo(dbHandle, &cbChannelInfo);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Error value of MsgStoGetCBChannelInfo [%d]", err);
			return false;
		}

		for (int i = 0; i < cbChannelInfo.channelCnt; i++)
		{
			bActivate = cbChannelInfo.channelInfo[i].bActivate;
			Category = cbChannelInfo.channelInfo[i].ctg;

			if (bActivate == true && svc_ctg == Category)
			{
				MSG_DEBUG("FIND CHANNEL = [%d]", svc_ctg);
				return true;
			}
		}

		return false;
	}

	return true;
}
