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

#include <errno.h>

#include "MsgGconfWrapper.h"
#include "MsgException.h"

#include "MsgUtilStorage.h"
#include "MsgNotificationWrapper.h"

#include "SmsCdmaPluginTransport.h"
#include "SmsCdmaPluginCodec.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginCallback.h"

extern "C" {
#include "TapiUtility.h"
#include "TelSms.h"
#include "TelNetwork.h"
#include "ITapiNetText.h"
}

extern struct tapi_handle *pTapiHandle;
extern bool isMemAvailable;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

SmsPluginTransport* SmsPluginTransport::pInstance = NULL;


SmsPluginTransport::SmsPluginTransport()
{
	msgRef 		= 0x00;
	msgRef8bit 	= 0x00;
	msgRef16bit	= 0x0000;

	msgSeqNum   = 0x00;
	if (MsgSettingGetInt(MSG_MESSAGE_ID_COUNTER, (int *)&msgSubmitId) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetInt() is failed");
	}
}


SmsPluginTransport::~SmsPluginTransport()
{
}


SmsPluginTransport* SmsPluginTransport::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginTransport();

	return pInstance;
}


unsigned char SmsPluginTransport::getMsgRef()
{
	return msgRef++;
}


unsigned char SmsPluginTransport::getSeqNum()
{
	msgSeqNum = ((msgSeqNum + 1) % SMS_SEQ_NUM_MAX);

	return msgSeqNum;
}


unsigned char SmsPluginTransport::getSubmitMsgId()
{
	msgSubmitId = ((msgSubmitId + 1) % SMS_MAX_MESSAGE_ID);

	MsgSettingSetInt(MSG_MESSAGE_ID_COUNTER, msgSubmitId);

	return msgSubmitId;
}


void SmsPluginTransport::convertMsgInfoToTelesvcMsg(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_trans_msg_s *pTransMsg)
{
	switch (pTransMsg->type) {
	case SMS_TRANS_P2P_MSG: {
		MSG_DEBUG("Convert  MSG_MESSAGE_INFO_S data to SMS_TRANS_MSG_S data.");
		sms_trans_p2p_msg_s *pPtpMsg = (sms_trans_p2p_msg_s *)&(pTransMsg->data.p2p_msg);

		convertMsgInfoToPtp(pMsgInfo, pPtpMsg);
	}
		break;
	default:
		MSG_DEBUG("Error Unsupported Transport Type");
		break;
	}
}


void SmsPluginTransport::convertMsgInfoToPtp(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_trans_p2p_msg_s *pPtpMsg)
{
	/* 1. Set Teleservice ID */
	pPtpMsg->telesvc_id = SMS_TRANS_TELESVC_CMT_95;

	/* 2. Set Service category */
	pPtpMsg->svc_ctg = SMS_TRANS_SVC_CTG_UNDEFINED;

	/* 3. Convert Address values */
	pPtpMsg->address.digit_mode = SMS_DIGIT_4BIT_DTMF;
	pPtpMsg->address.number_mode = SMS_NUMBER_MODE_NONE_DATANETWORK;
	pPtpMsg->address.number_type = SMS_NUMBER_TYPE_UNKNOWN;
	pPtpMsg->address.number_plan = SMS_NPI_UNKNOWN;
	pPtpMsg->address.addr_len = strlen(pMsgInfo->addressList[0].addressVal);
	strncpy(pPtpMsg->address.szData, pMsgInfo->addressList[0].addressVal, pPtpMsg->address.addr_len);

	if (pMsgInfo->addressList[0].addressVal[0] == '+') {
		pPtpMsg->address.digit_mode = SMS_DIGIT_8BIT;
		pPtpMsg->address.number_type = SMS_NUMBER_TYPE_INTERNATIONAL;
	} else {
		pPtpMsg->address.number_type = SMS_NUMBER_TYPE_NATIONAL;
	}

	/* 4. Convert Sub-address values */
	/* TODO */

	/* 5. Set Reply sequence number. */
	pPtpMsg->reply_seq = getSeqNum();

	/* convert msgInfo to Teleservice Message */
	switch (pPtpMsg->telesvc_msg.type) {
	case SMS_TYPE_SUBMIT:
		convertMsgInfoToSubmit(pMsgInfo, &(pPtpMsg->telesvc_msg.data.submit));
		break;
	default:
		break;
	}
}


void SmsPluginTransport::convertMsgInfoToSubmit(const MSG_MESSAGE_INFO_S *pMsgInfo, sms_telesvc_submit_s *pSubmit)
{
	if (pSubmit == NULL)
		return;

	/* 1. Set msg ID. */
	pSubmit->msg_id.msg_id = getSubmitMsgId();
	pSubmit->msg_id.header_ind = false;

	/* 2. Set User Data */
	unsigned char decodeData[SMS_MAX_USER_DATA_LEN + 1];
	memset(decodeData, 0x00, sizeof(decodeData));

	MsgTextConvert *textCvt = MsgTextConvert::instance();

	msg_encode_type_t encodeType = MSG_ENCODE_GSM7BIT;


	if (pMsgInfo->bTextSms == true) {
		if (pMsgInfo->encodeType == MSG_ENCODE_GSM7BIT) {
			pSubmit->user_data.encode_type = SMS_ENCODE_GSM7BIT;
		} else if (pMsgInfo->encodeType == MSG_ENCODE_8BIT) {
			pSubmit->user_data.encode_type = SMS_ENCODE_OCTET;
		} else if (pMsgInfo->encodeType == MSG_ENCODE_UCS2) {
			pSubmit->user_data.encode_type = SMS_ENCODE_UNICODE;
		} else if (pMsgInfo->encodeType == MSG_ENCODE_AUTO) {
			textCvt->convertUTF8ToAuto(decodeData, SMS_MAX_USER_DATA_LEN + 1, (unsigned char*)pMsgInfo->msgText, (int)pMsgInfo->dataSize, &encodeType);
			if (encodeType == MSG_ENCODE_ASCII7BIT) {
				pSubmit->user_data.encode_type = SMS_ENCODE_7BIT_ASCII;
			} else if (encodeType == MSG_ENCODE_8BIT) {
				pSubmit->user_data.encode_type = SMS_ENCODE_OCTET;
			} else if (encodeType == MSG_ENCODE_UCS2) {
				pSubmit->user_data.encode_type = SMS_ENCODE_UNICODE;
			}
		}
	}

	memset(pSubmit->user_data.user_data, 0x00, sizeof(pSubmit->user_data.user_data));
	snprintf((char *)pSubmit->user_data.user_data, sizeof(pSubmit->user_data.user_data), "%s", pMsgInfo->msgText);
	pSubmit->user_data.data_len = pMsgInfo->dataSize;

	MSG_DEBUG("encode type : [%d]", pSubmit->user_data.encode_type);

	/* 3. Set Valid period */
#if 0
	pSubmit->val_period.format = SMS_TIME_ABSOLUTE;
	pSubmit->val_period.time.abs_time.year = 0;
	pSubmit->val_period.time.abs_time.month = 0;
	pSubmit->val_period.time.abs_time.day = 0;
	pSubmit->val_period.time.abs_time.hours = 0;
	pSubmit->val_period.time.abs_time.minutes = 0;
	pSubmit->val_period.time.abs_time.seconds = 0;
#else
	pSubmit->val_period.format = SMS_TIME_RELATIVE;
	pSubmit->val_period.time.rel_time.rel_time = SMS_REL_TIME_INDEFINITE;
#endif

	/* 4. Set Defer valid period */
	/* TODO */

	/* 5. Set Priority */
	switch (pMsgInfo->priority) {
	case MSG_MESSAGE_PRIORITY_HIGH:
		pSubmit->priority = SMS_PRIORITY_URGENT;
		break;
	default:
		pSubmit->priority = SMS_PRIORITY_NORMAL;
		break;
	}

	/* 6. Set Privacy */
	pSubmit->privacy = SMS_PRIVACY_NOT_RESTRICTED;

	/* 7. Set Reply option */
	if (MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, (bool *)&(pSubmit->reply_opt.deliver_ack_req)) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	/* 8. Set Alert priority */
	pSubmit->alert_priority = SMS_ALERT_MOBILE_DEFAULT;

	/* 9. Set Language */
	pSubmit->language = SMS_LAN_UNKNOWN;

	/* 10. Set Callback number */
	/* TODO :: Set callback number to MSISDN */

	/* 11. Set Multi encode data */
	/* TODO */

	/* 12. Set Deposit id */
	/* TODO */

	/* 13. Set Service category program data */
	/* TODO */
}


void SmsPluginTransport::submitRequest(sms_request_info_s *pReqInfo)
{
	int tapiRet = TAPI_API_SUCCESS;

	if (pReqInfo == NULL) {
		THROW(MsgException::SMS_PLG_ERROR, "pReqInfo is NULL");
	}

	/* Get address informations. */
	MsgDbHandler *dbHandle = getDbHandle();
	MsgStoGetAddressByMsgId(dbHandle, pReqInfo->msgInfo.msgId, &pReqInfo->msgInfo.nAddressCnt, &pReqInfo->msgInfo.addressList);

	MSG_DEBUG("pReqInfo->msgInfo.nAddressCnt [%d]", pReqInfo->msgInfo.nAddressCnt);

	/* Get MSISDN */
	char *msisdn = NULL;
	char keyName[MAX_VCONFKEY_NAME_LEN];
	int simIndex = 1;

	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, simIndex);

	if (MsgSettingGetString(keyName, &msisdn) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	/* Tapi Data Structure */
	TelSmsDatapackageInfo_t tapi_data_pkg;
	memset(&tapi_data_pkg, 0x00, sizeof(TelSmsDatapackageInfo_t));

	bool bMoreMsgToSend = false;

	tapi_data_pkg.format = (TelSmsNetType_t)TAPI_NETTEXT_NETTYPE_3GPP2;

	/* convert msg_info to trans_msg */
	sms_trans_msg_s  trans_msg;
	memset(&trans_msg, 0x00, sizeof(sms_trans_msg_s));

	trans_msg.type = (sms_trans_msg_type_t)SMS_TRANS_P2P_MSG;
	trans_msg.data.p2p_msg.telesvc_msg.type = (sms_message_type_t)SMS_TYPE_SUBMIT;

	convertMsgInfoToTelesvcMsg(&pReqInfo->msgInfo, &trans_msg);

	/* encode msg data */
	unsigned char tel_sms_data[TAPI_NETTEXT_SMDATA_SIZE_MAX+1] = {0, };

	tapi_data_pkg.MsgLength = SmsPluginMsgCodec::instance()->encodeMsg(&trans_msg, tel_sms_data);

	memcpy((void *)tapi_data_pkg.szData, (void *)tel_sms_data, sizeof(tapi_data_pkg.szData));

	MSG_DEBUG("Submit Request TPDU.");
	char pduDbg[TAPI_NETTEXT_SMDATA_SIZE_MAX*2];
	memset(pduDbg, 0x00, sizeof(pduDbg));

	for (int i = 0; i < tapi_data_pkg.MsgLength; i++) {
		snprintf(pduDbg+(i*2), sizeof(pduDbg)- (i*2), "%02x", tapi_data_pkg.szData[i]);
	}
	MSG_DEBUG("Encode PDU= [%s]", pduDbg);

	sms_network_status_t retStatus;

	for (int cnt = 0; cnt < MAX_SMS_SEND_RETRY; ++cnt) {
		/* send request */
		sms_sent_info_s sent_info;
		memset(&sent_info, 0x00, sizeof(sms_sent_info_s));
		memcpy(&sent_info.reqInfo, pReqInfo, sizeof(sent_info.reqInfo));

		sent_info.bLast = true;

		SmsPluginEventHandler::instance()->SetSentInfo(&sent_info);

		int svc_type;
		tel_get_property_int(pTapiHandle, TAPI_PROP_NETWORK_SERVICE_TYPE, &svc_type);

		if (svc_type < TAPI_NETWORK_SERVICE_TYPE_2G) {
			MSG_DEBUG("Network service is not available : [%d]", svc_type);
			SmsPluginEventHandler::instance()->handleSentStatus(MSG_NETWORK_SEND_PENDING);
			goto _RETURN_FUNC;
		}

		curStatus = SMS_NETWORK_SENDING;

		/* Send SMS */
		tapiRet = tel_send_sms(pTapiHandle, &tapi_data_pkg, bMoreMsgToSend, TapiEventSentStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS) {
			MSG_DEBUG("########  tel_send_sms Success !!! return : [%d] #######", tapiRet);
		} else {
			SmsPluginEventHandler::instance()->handleSentStatus(MSG_NETWORK_SEND_FAIL);
			THROW(MsgException::SMS_PLG_ERROR, "########  tel_send_sms Fail !!! return : [%d] #######", tapiRet);
		}

		/* Tizen Validation System */
		MSG_SMS_VLD_INFO("%d, SMS Send Start, %s->%s, %s",  pReqInfo->msgInfo.msgId, \
																	(msisdn == NULL)?"ME":msisdn, \
																	pReqInfo->msgInfo.addressList[0].addressVal, \
																	(tapiRet == TAPI_API_SUCCESS)?"Success":"Fail");

		MSG_SMS_VLD_TXT("%d, [%s]", pReqInfo->msgInfo.msgId, pReqInfo->msgInfo.msgText);

		retStatus = getNetStatus();

		if (retStatus != SMS_NETWORK_SEND_FAIL_TEMPORARY)
			break;
	}

	MSG_SMS_VLD_INFO("%d, SMS Send End, %s->%s, %s",  pReqInfo->msgInfo.msgId, \
															(msisdn == NULL)?"ME":msisdn, \
															pReqInfo->msgInfo.addressList[0].addressVal, \
															(retStatus == SMS_NETWORK_SEND_SUCCESS)?"Success":"Fail");

	if (retStatus == SMS_NETWORK_SEND_SUCCESS) {
		MSG_DEBUG("########  Msg Sent was Successful !!! #######");
	} else {
		if (retStatus == SMS_NETWORK_SEND_FAIL_TIMEOUT || retStatus == SMS_NETWORK_SEND_FAIL_TEMPORARY
			|| retStatus == SMS_NETWORK_SEND_FAIL_MANDATORY_INFO_MISSING || retStatus == SMS_NETWORK_SEND_FAIL_FDN_RESTRICED)
			SmsPluginEventHandler::instance()->handleSentStatus(MSG_NETWORK_SEND_FAIL);

		if (retStatus == SMS_NETWORK_SEND_FAIL_FDN_RESTRICED)
			MsgInsertTicker("Unable to send the message while Fixed dialling mode is enabled", SMS_FDN_RESTRICTED, true, 0);
		else if (retStatus == SMS_NETWORK_SEND_PENDING)
			MsgInsertTicker("Network not available. Message will be sent when connected to network.", SMS_MESSAGE_SENDING_PENDING, true, 0);
		else
			MsgInsertTicker("Sending SMS is failed", SMS_MESSAGE_SENDING_FAIL, true, pReqInfo->msgInfo.msgId);
	}

_RETURN_FUNC :
	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}

	MSG_END();

	return;
}


void SmsPluginTransport::sendDeliverReport(msg_error_t err, sms_trans_p2p_msg_s *p_p2p_msg)
{
	MSG_BEGIN();

	int tapiRet = TAPI_API_SUCCESS;
	TelSmsResponse_t response;

	sms_trans_msg_s  trans_msg;
	memset(&trans_msg, 0x00, sizeof(sms_trans_msg_s));

	trans_msg.type = (sms_trans_msg_type_t)SMS_TRANS_ACK_MSG;

	if (p_p2p_msg)
		memcpy(&(trans_msg.data.ack_msg.address), &(p_p2p_msg->address), sizeof(sms_trans_addr_s));

	sms_trans_cause_code_s	cause_code;
	memset(&cause_code, 0x00, sizeof(sms_trans_cause_code_s));

	if (p_p2p_msg->reply_seq > 0)
		cause_code.reply_seq = p_p2p_msg->reply_seq;


	if (err == MSG_SUCCESS) {
		cause_code.error_class = SMS_TRANS_ERR_CLASS_NONE;

		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

		if (isMemAvailable == false) {
			tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, TapiEventMemoryStatus, NULL);

			if (tapiRet == TAPI_API_SUCCESS)
				MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
			else
				MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}

	} else if (err == MSG_ERR_MESSAGE_COUNT_FULL) {
		cause_code.error_class = SMS_TRANS_ERR_CLASS_TEMPORARY;

		response = TAPI_NETTEXT_ME_FULL;
		/* MsgInsertTicker("Not enough memory. Delete some items.", SMS_MESSAGE_MEMORY_FULL, true, 0); */

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		else
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);

	} else if (err == MSG_ERR_UNKNOWN) {
		cause_code.error_class = SMS_TRANS_ERR_CLASS_TEMPORARY;
		cause_code.cause_code = SMS_CAUSE_CODE_SERVICE_TERMINATION_DENIED;

		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

	} else if (err == MSG_ERR_INVALID_MSG_TYPE) {
		cause_code.error_class = SMS_TRANS_ERR_CLASS_PERMANENT;
		cause_code.cause_code = SMS_CAUSE_CODE_INVAILD_TELESERVICE_ID;

		response = TAPI_NETTEXT_INVALID_MSG;
	} else {
		cause_code.error_class = SMS_TRANS_ERR_CLASS_TEMPORARY;
		response = TAPI_NETTEXT_SIM_FULL;
	}

	memcpy(&(trans_msg.data.ack_msg.cause_code), &(cause_code), sizeof(sms_trans_cause_code_s));

	MSG_DEBUG("err : [%d], response : [%02x]", err, response);

	int bufLen = 0;

	unsigned char buf[512];
	memset(buf, 0x00, sizeof(buf));
	bufLen = SmsPluginMsgCodec::encodeMsg(&trans_msg, buf);


	MSG_DEBUG("######## DeliverReport tpdu #########");
	for (int i=0; i < bufLen; i++) {
		printf("[%02x] ", buf[i]);
	}
	MSG_DEBUG("#################################");

	/* Make Telephony Structure */
	TelSmsDatapackageInfo_t pkgInfo;

	pkgInfo.format = TAPI_NETTEXT_NETTYPE_3GPP2;

	/* Set TPDU data */
	memset((void*)pkgInfo.szData, 0x00, sizeof(pkgInfo.szData));
	memcpy((void*)pkgInfo.szData, buf, bufLen);

	pkgInfo.szData[bufLen] = 0;
	pkgInfo.MsgLength = bufLen;


	/* Send Deliver Report */
	tapiRet = tel_send_sms_deliver_report(pTapiHandle, &pkgInfo, response, TapiEventDeliveryReportCNF, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
		MSG_DEBUG("########  tel_send_sms_deliver_report() Success !!! #######");
	else
		MSG_DEBUG("########  tel_send_sms_deliver_report() Fail !!! return : [%d] #######", tapiRet);

	MSG_END();
}


void SmsPluginTransport::setNetStatus(sms_network_status_t sentStatus)
{
	mx.lock();
	curStatus = sentStatus;
	cv.signal();
	mx.unlock();
}


sms_network_status_t SmsPluginTransport::getNetStatus()
{
	mx.lock();

	int ret = 0;

	if (curStatus == SMS_NETWORK_SENDING)
		ret = cv.timedwait(mx.pMsgMutex(), 125);

	mx.unlock();

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("WARNING: SENT STATUS TIME-OUT");
		curStatus = SMS_NETWORK_SEND_FAIL_TIMEOUT;
	}

	return curStatus;
}
