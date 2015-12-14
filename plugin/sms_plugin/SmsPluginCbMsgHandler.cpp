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

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "SmsPluginUDCodec.h"
#include "SmsPluginStorage.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginCbMsgHandler.h"
#include "SmsPluginDSHandler.h"

/*==================================================================================================
                  IMPLEMENTATION OF SmsPluginCbMsgHandler - Member Functions
==================================================================================================*/
SmsPluginCbMsgHandler* SmsPluginCbMsgHandler::pInstance = NULL;


SmsPluginCbMsgHandler::SmsPluginCbMsgHandler()
{
	pageList.clear();
}


SmsPluginCbMsgHandler::~SmsPluginCbMsgHandler()
{
}


SmsPluginCbMsgHandler* SmsPluginCbMsgHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginCbMsgHandler();

	return pInstance;
}


void SmsPluginCbMsgHandler::handleCbMsg(TapiHandle *handle, TelSmsCbMsg_t *pCbMsg)
{
	MSG_BEGIN();

#if 0
	char temp[1000]= {0x01, 0xa4, 0x1f, 0x51, 0x10, 0x11, 0x02, 0xea, 0x30, 0x30, 0xa8, 0x30, 0xea, 0x30, 0xa2, 0x30, 0xe1, 0x30, 0xfc, 0x30, 0xeb, 0x91, 0x4d, 0x4f, 0xe1, 0x30, 0xc6, 0x30, 0xb9, 0x30, 0xc8, 0x00, 0x0d, 0x00, 0x0a, 0x30, 0x53, 0x30, 0x8c, 0x30, 0x6f, 0x8a, 0x66, 0x9a, 0x13, 0x75, 0x28, 0x30, 0x6e, 0x30, 0xe1, 0x30, 0xc3, 0x30, 0xbb, 0x30, 0xfc, 0x30, 0xb8, 0x30, 0x67, 0x30, 0x59, 0x30, 0x02, 0x00, 0x0d, 0x00, 0x0a, 0xff, 0x08, 0x00, 0x32, 0x00, 0x30, 0x00, 0x31, 0x00, 0x33, 0x00, 0x2f, 0x00, 0x31, 0x00, 0x31, 0x00, 0x2f, 0x00, 0x32, 0x52, 0xea, 0x30, 0x00, 0x37, 0x00, 0x20, 0x00, 0x31, 0x00, 0x35, 0x00, 0x3a, 0x00, 0x34, 0x00, 0x34, 0xff, 0x09, 0x00, 0x0d, 0x00, 0x0a, 0xff, 0x08, 0x30, 0xa8, 0x30, 0xea, 0x30, 0xa2, 0x5e, 0x02, 0xff, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22};
	TelSmsEtwsMsg_t *pCbMsg = (TelSmsEtwsMsg_t *)calloc(1, sizeof(TelSmsEtwsMsg_t));
	pCbMsg->Length = 173;
	pCbMsg->EtwsMsgType = (TelSmsEtwsMsgType_t)2;
	memcpy(pCbMsg->szMsgData, temp, pCbMsg->Length);

	SMS_CB_NETWORK_TYPE_T type = pCbMsg->EtwsMsgType;
	SMS_CBMSG_PAGE_S CbMsgPage = {0};

	switch (type) {
	case SMS_CB_NETWORK_TYPE_2G_GSM :
		Decode2gCbMsg((TelSmsCbMsg_t *)pCbMsg, &CbMsgPage);
		break;

	case SMS_CB_NETWORK_TYPE_3G_UMTS :
		Decode3gCbMsg((TelSmsCbMsg_t *)pCbMsg, &CbMsgPage);
		break;
	}
#else
	SMS_CB_NETWORK_TYPE_T type = pCbMsg->CbMsgType;
	SMS_CBMSG_PAGE_S CbMsgPage = {0};

	switch (type) {
	case SMS_CB_NETWORK_TYPE_2G_GSM :
		Decode2gCbMsg(pCbMsg, &CbMsgPage);
		break;

	case SMS_CB_NETWORK_TYPE_3G_UMTS :
		Decode3gCbMsg(pCbMsg, &CbMsgPage);
		break;
	}
#endif
	/* Check CB Msg Options */
	bool bJavaMsg = false;

	int simIndex = SmsPluginDSHandler::instance()->getSimIndex(handle);

	if (!checkCbOpt(&CbMsgPage, &bJavaMsg, simIndex)) {
		MSG_DEBUG("The CB Msg is not supported by option.");
		return;
	}

	if (bJavaMsg == true) {
		MSG_DEBUG("JAVA CB Msg.");
		CbMsgPage.cbMsgType = SMS_CBMSG_TYPE_JAVACBS;
	}


	/* Check CB Pages */
	unsigned char pageCnt = checkCbPage(&CbMsgPage);

	if (pageCnt == CbMsgPage.pageHeader.totalPages) {
		MSG_DEBUG("RECEIVED LAST MSG : %d", pageCnt);

		SMS_CBMSG_S *cbMsg = NULL;
		unique_ptr<SMS_CBMSG_S*, void(*)(SMS_CBMSG_S**)> buf(&cbMsg, unique_ptr_deleter);
		cbMsg = (SMS_CBMSG_S *)new char[sizeof(SMS_CBMSG_S)];
		memset(cbMsg, 0x00, sizeof(SMS_CBMSG_S));

		MSG_MESSAGE_INFO_S msgInfo;

		/** initialize msgInfo */
		memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

		msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

		/* Make CB Msg Structure */
		MakeCbMsg(&CbMsgPage, cbMsg);

		/* Convert to MSG_MESSAGE_INFO_S */
		convertCbMsgToMsginfo(cbMsg, &msgInfo, simIndex);

		/* Add CB Msg into DB */
		msg_error_t err = MSG_SUCCESS;

		err = SmsPluginStorage::instance()->checkMessage(&msgInfo);

		if (err == MSG_SUCCESS) {
			MSG_CB_MSG_S cbOutMsg = {0, };

			/*cbOutMsg.type = MSG_CB_SMS; */
			cbOutMsg.type = msgInfo.msgType.subType;
			cbOutMsg.receivedTime = cbMsg->recvTime;
			cbOutMsg.serialNum = encodeCbSerialNum(CbMsgPage.pageHeader.serialNum);
			cbOutMsg.messageId = cbMsg->msgId;
			cbOutMsg.dcs = CbMsgPage.pageHeader.dcs.rawData;
			memset (cbOutMsg.cbText, 0x00, sizeof(cbOutMsg.cbText));

			cbOutMsg.cbTextLen = convertTextToUtf8((unsigned char*)cbOutMsg.cbText, sizeof(cbOutMsg.cbText), cbMsg);
			memset(cbOutMsg.language_type, 0x00, sizeof(cbOutMsg.language_type));
			memcpy(cbOutMsg.language_type, CbMsgPage.pageHeader.dcs.iso639Lang, 3);
			err = SmsPluginEventHandler::instance()->callbackCBMsgIncoming(&cbOutMsg, &msgInfo);
			if (err != MSG_SUCCESS)
				MSG_WARN("callbackMsgIncoming() Error !! [%d]", err);
		} else {
			MSG_WARN("checkMessage() Error !! [%d]", err);
		}

#if 0
		/* insert message-id to internal CB message table */
		SmsPluginStorage *storageHandler = SmsPluginStorage::instance();
		err = storageHandler->insertReceivedCBMessage(CbMsgPage);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("insertReceivedCBMessage() Error !! [%d]", err);
		}
#endif
		/* Remove From List */
		removeFromPageList(&CbMsgPage);
	}
	MSG_END();
}


void SmsPluginCbMsgHandler::handleEtwsMsg(TapiHandle *handle, TelSmsEtwsMsg_t *pEtwsMsg)
{
	MSG_BEGIN();
	msg_error_t err = MSG_SUCCESS;
	TelSmsEtwsMsgType_t type = pEtwsMsg->EtwsMsgType;
	SMS_ETWS_PRIMARY_S		etwsPn = {0, };
	MSG_CB_MSG_S			cbOutMsg = {0, };

	if (type != TAPI_NETTEXT_ETWS_PRIMARY) {
		MSG_DEBUG("The Etws secondary Message");
		handleCbMsg(handle, (TelSmsCbMsg_t *)pEtwsMsg);
		return;
	}
	DecodeEtwsMsg(pEtwsMsg, &etwsPn);

	cbOutMsg.type = MSG_ETWS_SMS;
	cbOutMsg.receivedTime = etwsPn.recvTime;
	cbOutMsg.serialNum = encodeCbSerialNum(etwsPn.serialNum);
	cbOutMsg.messageId = etwsPn.msgId;
	cbOutMsg.etwsWarningType = etwsPn.warningType;
	memcpy (cbOutMsg.etwsWarningSecurityInfo, etwsPn.warningSecurityInfo, sizeof(cbOutMsg.etwsWarningSecurityInfo));

	err = SmsPluginEventHandler::instance()->callbackCBMsgIncoming(&cbOutMsg, NULL);
	if (err != MSG_SUCCESS)
		MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);

	MSG_END();
}


void SmsPluginCbMsgHandler::Decode2gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage)
{
	unsigned char cbData[pCbMsg->Length+1];

	memset(cbData, 0x00, sizeof(cbData));
	memcpy(cbData, pCbMsg->szMsgData, pCbMsg->Length);
	cbData[pCbMsg->Length] = '\0';

	/* print cb data */
	MSG_INFO("Received CB length:%d", pCbMsg->Length);
	char cbDataTmp[(pCbMsg->Length*2)+1];
	memset(cbDataTmp, 0x00, sizeof(cbDataTmp));

	for (int i = 0; i < pCbMsg->Length; i++) {
		snprintf(cbDataTmp+(i*2), sizeof(cbDataTmp)-(i*2), "%02X", cbData[i]);
	}
	MSG_INFO("[%s]", cbDataTmp);

	pCbPage->cbMsgType = SMS_CBMSG_TYPE_CBS;

	/* Serial Number */
	pCbPage->pageHeader.serialNum.geoScope = (cbData[0] & 0xC0) >> 6;

	pCbPage->pageHeader.serialNum.msgCode = (cbData[0] & 0x3F) << 4;
	pCbPage->pageHeader.serialNum.msgCode |= (cbData[1] & 0xF0) >> 4;

	pCbPage->pageHeader.serialNum.updateNum = cbData[1] & 0x0F;

	MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pCbPage->pageHeader.serialNum.geoScope, pCbPage->pageHeader.serialNum.msgCode, pCbPage->pageHeader.serialNum.updateNum);

	pCbPage->pageHeader.msgId = (cbData[2] << 8) | cbData[3];

	MSG_DEBUG("MSG ID : [%d]", pCbPage->pageHeader.msgId);

	/* DCS */
	decodeCbMsgDCS(cbData[4], (unsigned char*)cbData + 6, &(pCbPage->pageHeader.dcs));

	/* Page Parameter */
	pCbPage->pageHeader.totalPages = cbData[5] & 0x0F;
	pCbPage->pageHeader.page = (cbData[5] & 0xF0) >> 4;

	if (pCbPage->pageHeader.totalPages > MAX_CBMSG_PAGE_NUM)
		THROW(MsgException::SMS_PLG_ERROR, "CB Page Count is over MAX[%d]", pCbPage->pageHeader.totalPages);

	MSG_DEBUG("Total Page : [%d], Page : [%d]", pCbPage->pageHeader.totalPages, pCbPage->pageHeader.page);

	/* Convert Language Type */
	convertLangType(pCbPage->pageHeader.dcs.langType, &(pCbPage->pageHeader.langType));

	MSG_DEBUG("In Language Type : [%d], Out Language Type : [%d]", pCbPage->pageHeader.dcs.langType, pCbPage->pageHeader.langType);
	MSG_DEBUG("iso639Lang : [%s]", pCbPage->pageHeader.dcs.iso639Lang);
	/* Get Receive Time */
	pCbPage->pageHeader.recvTime = getRecvTime();

	/* Decode CB Data */
	int dataLen = pCbMsg->Length - 6;

	MSG_DEBUG("codingScheme:[%d]", pCbPage->pageHeader.dcs.codingScheme);

	switch (pCbPage->pageHeader.dcs.codingScheme) {
	case SMS_CHARSET_7BIT: {
		MSG_DEBUG("GSM 7 BIT");

		dataLen = (dataLen*8) / 7;

		SmsPluginUDCodec udCodec;
		char pageData[MAX_CBMSG_PAGE_SIZE+1];
		int unpackLen = udCodec.unpack7bitChar(&cbData[6], dataLen, 0, pageData);

		if (pCbPage->pageHeader.dcs.iso639Lang[0]) {
			unpackLen = unpackLen - 3;
			if (unpackLen > 0)
				memcpy(pCbPage->pageData, &pageData[3], unpackLen);
			else
				unpackLen = 0;
		} else {
			memcpy(pCbPage->pageData, &pageData, unpackLen);
		}

		MSG_DEBUG("unpackLen : [%d]", unpackLen);

		pCbPage->pageLength = unpackLen;
		pCbPage->pageData[unpackLen] = '\0';
	}
	break;

	case SMS_CHARSET_8BIT:
	case SMS_CHARSET_UCS2: {
		MSG_DEBUG("UCS2 or 8BIT");

		if (pCbPage->pageHeader.dcs.iso639Lang[0]) {
			memcpy(pCbPage->pageData, &cbData[8], dataLen - 2);
			pCbPage->pageLength = dataLen - 2;
		} else {
			memcpy(pCbPage->pageData, &cbData[6], dataLen);
			pCbPage->pageLength = dataLen;
		}
	}
	break;
	}

	MSG_DEBUG("Page Length : [%d], Page Data : [%s]", pCbPage->pageLength, pCbPage->pageData);
}


void SmsPluginCbMsgHandler::DecodeEtwsMsg(TelSmsEtwsMsg_t *pEtwsMsg, SMS_ETWS_PRIMARY_S *pEtwsPn)
{
	if ( !pEtwsMsg || !pEtwsPn )
		return;

	if (pEtwsMsg->Length > MAX_ETWS_SIZE)
		THROW(MsgException::SMS_PLG_ERROR, "ETWS Msg Size is over MAX [%d]", pEtwsMsg->Length);

	unsigned char EtwsData[pEtwsMsg->Length+1];

	memset(EtwsData, 0x00, sizeof(EtwsData));
	memcpy(EtwsData, pEtwsMsg->szMsgData, pEtwsMsg->Length);
	EtwsData[pEtwsMsg->Length] = '\0';

	/* print received msg data */
	MSG_INFO("Received Etws length:%d", pEtwsMsg->Length);
	char EtwsDataTmp[(pEtwsMsg->Length*2)+1];
	memset(EtwsDataTmp, 0x00, sizeof(EtwsDataTmp));

	for (int i = 0; i < pEtwsMsg->Length; i++) {
		snprintf(EtwsDataTmp+(i*2), sizeof(EtwsDataTmp)-(i*2), "%02X", EtwsData[i]);
	}
	MSG_INFO("[%s]", EtwsDataTmp);

	/* received time */
	pEtwsPn->recvTime = getRecvTime();

	/* Serial Number */
	pEtwsPn->serialNum.geoScope = (EtwsData[0] & 0xC0) >> 6;
	pEtwsPn->serialNum.msgCode = (EtwsData[0] & 0x3F) << 4;
	pEtwsPn->serialNum.msgCode |= (EtwsData[1] & 0xF0) >> 4;
	pEtwsPn->serialNum.updateNum = EtwsData[1] & 0x0F;

	MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pEtwsPn->serialNum.geoScope, pEtwsPn->serialNum.msgCode, pEtwsPn->serialNum.updateNum);

	/* Message Identifier */
	pEtwsPn->msgId = (EtwsData[2] << 8) | EtwsData[3];
	MSG_DEBUG("MSG ID : [%d]", pEtwsPn->msgId);

	/* warning type */
	pEtwsPn->warningType = (EtwsData[4] << 8) | EtwsData[5];
	MSG_DEBUG("warningType : [0x%04x]", pEtwsPn->msgId);

	/* warning security information */
	memcpy(pEtwsPn->warningSecurityInfo, &EtwsData[6], sizeof(pEtwsPn->warningSecurityInfo));	/* 50bytes */
	for (unsigned int i = 0; i < sizeof(pEtwsPn->warningSecurityInfo); i++) {
		MSG_DEBUG("warning secu info [%02x]", pEtwsPn->warningSecurityInfo[i]);
	}
}

void SmsPluginCbMsgHandler::Decode3gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage)
{
	unsigned char cbData[(MAX_CBMSG_PAGE_SIZE*MAX_CBMSG_PAGE_NUM)+1];

	memset(cbData, 0x00, sizeof(cbData));
	memcpy(cbData, pCbMsg->szMsgData, pCbMsg->Length);
	cbData[pCbMsg->Length] = '\0';

	/* print cb data */
	MSG_INFO("Received CB length:%d", pCbMsg->Length);
	char cbDataTmp[(pCbMsg->Length*2)+1];
	memset(cbDataTmp, 0x00, sizeof(cbDataTmp));

	for (int i = 0; i < pCbMsg->Length; i++) {
		snprintf(cbDataTmp+(i*2), sizeof(cbDataTmp)-(i*2), "%02X", cbData[i]);
	}
	MSG_INFO("[%s]", cbDataTmp);

	pCbPage->cbMsgType = (SMS_CBMSG_TYPE_T)cbData[0];

	pCbPage->pageHeader.msgId = (cbData[1] << 8) | cbData[2];

	MSG_DEBUG("MSG ID : [%d]", pCbPage->pageHeader.msgId);

	/* Serial Number */
	pCbPage->pageHeader.serialNum.geoScope = (cbData[3] & 0xC0) >> 6;

	pCbPage->pageHeader.serialNum.msgCode = (cbData[3] & 0x3F) << 4;
	pCbPage->pageHeader.serialNum.msgCode |= (cbData[4] & 0xF0) >> 4;

	pCbPage->pageHeader.serialNum.updateNum = cbData[4] & 0x0F;

	MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pCbPage->pageHeader.serialNum.geoScope, pCbPage->pageHeader.serialNum.msgCode, pCbPage->pageHeader.serialNum.updateNum);

	/* DCS */
	decodeCbMsgDCS(cbData[5], (unsigned char*)cbData + 6, &(pCbPage->pageHeader.dcs));

	/* Convert Language Type */
	convertLangType(pCbPage->pageHeader.dcs.langType, &(pCbPage->pageHeader.langType));

	MSG_DEBUG("In Language Type : [%d], Out Language Type : [%d]", pCbPage->pageHeader.dcs.langType, pCbPage->pageHeader.langType);

	/* Get Receive Time */
	pCbPage->pageHeader.recvTime = getRecvTime();
	pCbPage->pageHeader.totalPages = cbData[6];

	/* Decode CB Data */
	int dataLen = 0;
	int offset = 0;

	switch (pCbPage->pageHeader.dcs.codingScheme) {
	case SMS_CHARSET_7BIT: {
		for (int i = 0; i < pCbPage->pageHeader.totalPages; ++i) {
			char cbMessage[MAX_CBMSG_PAGE_SIZE] = {0, };
			dataLen = cbData[7+(i+1)*82 + i];
			memcpy(cbMessage, &cbData[7+(i*82)+ i], dataLen);

			dataLen = (dataLen*8) / 7;

			if (dataLen > MAX_CBMSG_PAGE_SIZE)
				THROW(MsgException::SMS_PLG_ERROR, "CB Msg Size is over MAX [%d]", dataLen);

			SmsPluginUDCodec udCodec;
			int unpackLen = udCodec.unpack7bitChar((const unsigned char *)cbMessage, dataLen, 0, pCbPage->pageData + offset);
			offset += unpackLen;
		}
		pCbPage->pageLength = offset;
	}
	break;

	case SMS_CHARSET_8BIT:
	case SMS_CHARSET_UCS2: {
#if 0
		char cbMessage[MAX_CBMSG_PAGE_SIZE]= {0, };

		for (int i = 0; i < pCbPage->pageHeader.totalPages; ++i) {
			dataLen = cbData[7+(i+1)*82 + i];
			memcpy(cbMessage + offset, &cbData[7+(i*82)+ i], dataLen);
			offset += dataLen;
		}
		dataLen = offset;

		if (pCbPage->pageHeader.dcs.iso639Lang[0]) {
			int tmpDataLen = (dataLen > 2)?(dataLen - 2):0;
			memcpy(pCbPage->pageData, cbMessage + 2, tmpDataLen);
			pCbPage->pageLength = tmpDataLen;
		} else {
			memcpy(pCbPage->pageData, cbMessage, dataLen);
			pCbPage->pageLength = dataLen;
		}
#else
		char cbMessage[MAX_CBMSG_PAGE_SIZE] = {0, };

		for (int i = 0; i < pCbPage->pageHeader.totalPages; ++i) {
			if (pCbPage->pageHeader.dcs.iso639Lang[0]) {
				dataLen = cbData[7+(i+1)*82 + i] - 2;
				memcpy(cbMessage + offset, &cbData[7+(i*82)+ i + 2], dataLen);
				offset += dataLen;
			} else {
				dataLen = cbData[7+(i+1)*82 + i];
				memcpy(cbMessage + offset, &cbData[7+(i*82)+ i], dataLen);
				offset += dataLen;
			}
		}
		dataLen = offset;
		memcpy(pCbPage->pageData, cbMessage, dataLen);
		pCbPage->pageLength = dataLen;
#endif
	}
	break;
	}
	pCbPage->pageHeader.totalPages = 1;

	MSG_DEBUG("Page Length : [%d], Page Data : [%s]", pCbPage->pageLength, pCbPage->pageData);
}

unsigned short SmsPluginCbMsgHandler::encodeCbSerialNum(SMS_CBMSG_SERIAL_NUM_S snFields)
{
	unsigned short serialNum = 0;

	serialNum = ((snFields.geoScope & 0x03) << 14) | ((snFields.msgCode&0x03FF) << 4) | (snFields.updateNum&0x0F);
	MSG_DEBUG("serialNum (%x), geo(%x), mc(%x), un(%x)\n", serialNum, snFields.geoScope, snFields.msgCode, snFields.updateNum);

	return serialNum;
}

int SmsPluginCbMsgHandler::CMAS_class(unsigned short message_id)
{
	int ret = 0;

	switch (message_id) {
	case 4370:
	case 4383:
		ret = MSG_CMAS_PRESIDENTIAL;
		break;
	case 4371:
	case 4372:
	case 4384:
	case 4385:
		ret = MSG_CMAS_EXTREME;
		break;
	case 4373:
	case 4374:
	case 4375:
	case 4376:
	case 4377:
	case 4378:
	case 4386:
	case 4387:
	case 4388:
	case 4389:
	case 4390:
	case 4391:
		ret = MSG_CMAS_SEVERE;
		break;
	case 4379:
	case 4392:
		ret = MSG_CMAS_AMBER;
		break;
	case 4380:
	case 4393:
		ret = MSG_CMAS_TEST;
		break;
	case 4381:
	case 4394:
		ret = MSG_CMAS_EXERCISE;
		break;
	case 4382:
	case 4395:
		ret = MSG_CMAS_OPERATOR_DEFINED;
		break;
	default:
		break;
	}

	return ret;
}

bool SmsPluginCbMsgHandler::checkCbOpt(SMS_CBMSG_PAGE_S *CbPage, bool *pJavaMsg, msg_sim_slot_id_t simIndex)
{
	bool bReceive = false;
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", CB_RECEIVE, simIndex);
	MsgSettingGetBool(keyName, &bReceive);

	/* Receive CB Msg = FALSE */
	if (!bReceive) {
		MSG_DEBUG("RECEIVE CB = FALSE");
		return false;
	}

#if 0
	char keyname[128];
	/* check Language */
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyname), "%s/%d", CB_LANGUAGE, MSG_CBLANG_TYPE_ALL);

	bool bAllLang = false;
	MsgSettingGetBool(keyName, &bAllLang);

	if (!bAllLang) {
		MSG_DEBUG("ALL LANGUAGE = FALSE");

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyname), "%s/%d", CB_LANGUAGE, CbPage.pageHeader.langType);

		bool bLang = false;

		MsgSettingGetBool(keyName, &bLang);

		if (!bLang || CbPage.pageHeader.langType == MSG_CBLANG_TYPE_MAX) {
			MSG_DEBUG("LANGUAGE [%d] = FALSE", CbPage.pageHeader.langType);
			return false;
		}
	}
#endif

	bool bActivate = false;
	int MsgId_from = 0, MsgId_to = 0;
	MSG_CB_CHANNEL_S cbChannelInfo = {0, };
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	err = MsgStoGetCBChannelInfo(dbHandle, &cbChannelInfo, simIndex);
	if (err != MSG_SUCCESS)
		MSG_ERR("Error value of MsgStoGetCBChannelInfo [%d]", err);

	for (int i = 0; i < cbChannelInfo.channelCnt; i++) {
		bActivate = cbChannelInfo.channelInfo[i].bActivate;
		MsgId_from = cbChannelInfo.channelInfo[i].from;
		MsgId_to = cbChannelInfo.channelInfo[i].to;

		if (bActivate == true && CbPage->pageHeader.msgId >= MsgId_from && CbPage->pageHeader.msgId <= MsgId_to) {
			MSG_DEBUG("FIND CHANNEL = [%d]", CbPage->pageHeader.msgId);
			return true;
		}
	}

	return false;
}

unsigned char SmsPluginCbMsgHandler::checkCbPage(SMS_CBMSG_PAGE_S *CbPage)
{
	unsigned char currPageCnt = 0;

	bool bFind = false;

#if 0
	msg_error_t err = MSG_SUCCESS;

	SmsPluginStorage *storageHandler = SmsPluginStorage::instance();
	err = storageHandler->isReceivedCBMessage(CbPage);

	/* check existing message with cb internal table; */
	if (err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("already received message: [%d]", CbPage.pageHeader.msgId);
		return 0;
	}
#endif

	if (CbPage->pageHeader.totalPages > 0) {
		for (unsigned int i = 0; i < pageList.size(); i++) {
			if (pageList[i].geoScope == CbPage->pageHeader.serialNum.geoScope && pageList[i].msgCode == CbPage->pageHeader.serialNum.msgCode) {
				MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[i].geoScope, pageList[i].msgCode);

				if (pageList[i].msgId == CbPage->pageHeader.msgId) {
					int updateNum = CbPage->pageHeader.serialNum.updateNum - pageList[i].updateNum;

					if (updateNum > 0) { /* New Message Content */
						break;
					} else if (updateNum == 0) { /* Same Message Content */
						if (pageList[i].data.count(CbPage->pageHeader.page) != 0) {
							MSG_DEBUG("The Page Number already exists [%d]", CbPage->pageHeader.page);
							return 0;
						}

						pair<unsigned char, SMS_CBMSG_PAGE_S> newData(CbPage->pageHeader.page, *CbPage);
						pageList[i].data.insert(newData);

						MSG_DEBUG("PAGE DATA : %s", CbPage->pageData);
						MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.pageData);

						pageList[i].pageCnt++;
						pageList[i].totalSize += CbPage->pageLength;

						currPageCnt = pageList[i].pageCnt;

						bFind = true;

						break;
					} else { /* Old Message Content */
						return 0;
					}
				}
			}
		}
	}

	if (bFind == false || CbPage->pageHeader.totalPages == 1) {
		addToPageList(CbPage);
		return 1;
	}

	return currPageCnt;
}


void SmsPluginCbMsgHandler::MakeCbMsg(SMS_CBMSG_PAGE_S *CbPage, SMS_CBMSG_S *pCbMsg)
{
	pCbMsg->cbMsgType = CbPage->cbMsgType;
	pCbMsg->msgId = CbPage->pageHeader.msgId;
	pCbMsg->classType = CbPage->pageHeader.dcs.classType;
	pCbMsg->codingScheme = CbPage->pageHeader.dcs.codingScheme;
	pCbMsg->recvTime = CbPage->pageHeader.recvTime;

	cbPageMap::iterator it;
	int offset = 0;

	for (unsigned int i = 0; i < pageList.size(); i++) {
		if (pageList[i].geoScope == CbPage->pageHeader.serialNum.geoScope && pageList[i].msgCode == CbPage->pageHeader.serialNum.msgCode) {
			MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[i].geoScope, pageList[i].msgCode);

			if (pageList[i].msgId == CbPage->pageHeader.msgId) {
				for (it = pageList[i].data.begin(); it != pageList[i].data.end(); it++) {
					memcpy(pCbMsg->msgData + offset, it->second.pageData, it->second.pageLength);
					pCbMsg->msgLength += it->second.pageLength;
					offset = pCbMsg->msgLength;
				}
			}
		}
	}

#if 0
	pCbMsg->msgLength = tmpStr.size();

	memcpy(pCbMsg->msgData, tmpStr.c_str(), tmpStr.size());
	pCbMsg->msgData[tmpStr.size()] = '\0';

	MSG_DEBUG("SIZE : [%d] TOTAL MSG : %s", tmpStr.size(), tmpStr.c_str());
#endif
}


void SmsPluginCbMsgHandler::convertCbMsgToMsginfo(SMS_CBMSG_S *pCbMsg, MSG_MESSAGE_INFO_S *pMsgInfo, msg_sim_slot_id_t simIndex)
{
	pMsgInfo->msgId = (msg_message_id_t)pCbMsg->msgId;

	pMsgInfo->folderId = MSG_CBMSGBOX_ID;

	/* Convert Type values */
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;

	if (pCbMsg->cbMsgType == SMS_CBMSG_TYPE_CBS) {
		int cmas_class = CMAS_class(pCbMsg->msgId);

		if (cmas_class == 0)
			pMsgInfo->msgType.subType = MSG_CB_SMS;
		else
			pMsgInfo->msgType.subType = (MSG_SUB_TYPE_T)cmas_class;
	} else if (pCbMsg->cbMsgType == SMS_CBMSG_TYPE_JAVACBS) {
		pMsgInfo->msgType.subType = MSG_JAVACB_SMS;
	}

	switch (pCbMsg->classType) {
	case SMS_MSG_CLASS_0:
		pMsgInfo->msgType.classType = MSG_CLASS_0;
		break;
	case SMS_MSG_CLASS_1:
		pMsgInfo->msgType.classType = MSG_CLASS_1;
		break;
	case SMS_MSG_CLASS_2:
		pMsgInfo->msgType.classType = MSG_CLASS_2;
		break;
	case SMS_MSG_CLASS_3:
		pMsgInfo->msgType.classType = MSG_CLASS_3;
		break;
	default:
		pMsgInfo->msgType.classType = MSG_CLASS_NONE;
		break;
	}

	pMsgInfo->storageId = MSG_STORAGE_PHONE;
	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	/* Temporary */
	pMsgInfo->nAddressCnt = 1;

	pMsgInfo->addressList =  (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
	pMsgInfo->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_UNKNOWN;
	pMsgInfo->sim_idx = simIndex;

	/*	TODO :: MSG ID should be used to get CB message type */
	getDisplayName(pCbMsg->msgId, pMsgInfo->addressList[0].addressVal, simIndex);
	MSG_SEC_DEBUG("%s", pMsgInfo->addressList[0].addressVal);


	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	pMsgInfo->displayTime = pCbMsg->recvTime;
	MSG_DEBUG("recvTime is %d", pMsgInfo->displayTime);

	int bufSize = pCbMsg->msgLength*2;

	char tmpBuf[bufSize];
	memset(tmpBuf, 0x00, sizeof(tmpBuf));

	while (pCbMsg->msgLength > 0) {
		if (pCbMsg->msgData[pCbMsg->msgLength-1] == ' ' ||
				pCbMsg->msgData[pCbMsg->msgLength-1] == '\r' ||
				pCbMsg->msgData[pCbMsg->msgLength-1] == '\n') {
			pCbMsg->msgLength--;
		}
		else {
			break;
		}
	}
	pCbMsg->msgData[pCbMsg->msgLength] = '\0';

	MSG_DEBUG("LENGTH %d CB MSG %s", pCbMsg->msgLength, pCbMsg->msgData);

	/* Convert Data values */
	pMsgInfo->dataSize = convertTextToUtf8((unsigned char*)tmpBuf, bufSize, pCbMsg);

	if (pMsgInfo->dataSize > MAX_MSG_TEXT_LEN) {
		pMsgInfo->bTextSms = false;

		/* Save Message Data into File */
		char fileName[MSG_FILENAME_LEN_MAX+1];
		memset(fileName, 0x00, sizeof(fileName));

		if (MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

		MSG_SEC_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsgInfo->dataSize, fileName);
		if (MsgWriteIpcFile(fileName, tmpBuf, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

		strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);
	} else {
		pMsgInfo->bTextSms = true;

		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		memcpy(pMsgInfo->msgText, tmpBuf, pMsgInfo->dataSize);
		pMsgInfo->dataSize = strlen(pMsgInfo->msgText);
		MSG_SEC_DEBUG("CB MSG %s", pMsgInfo->msgText);
	}
}


void SmsPluginCbMsgHandler::convertEtwsMsgToMsginfo(SMS_CBMSG_PAGE_S *EtwsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, msg_sim_slot_id_t simIndex)
{
	pMsgInfo->msgId = (msg_message_id_t)EtwsMsg->pageHeader.msgId;

	pMsgInfo->folderId = MSG_CBMSGBOX_ID;

	/* Convert Type values */
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;

	if (EtwsMsg->cbMsgType == SMS_CBMSG_TYPE_ETWS)
		pMsgInfo->msgType.subType = MSG_ETWS_SMS;

	pMsgInfo->storageId = MSG_STORAGE_PHONE;
	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	/* Temporary */
	pMsgInfo->nAddressCnt = 1;

	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
	pMsgInfo->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_UNKNOWN;

	getDisplayName(EtwsMsg->pageHeader.msgId, pMsgInfo->addressList[0].addressVal, simIndex);
	MSG_SEC_DEBUG("%s", pMsgInfo->addressList[0].addressVal);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	pMsgInfo->displayTime = EtwsMsg->pageHeader.recvTime;
	MSG_DEBUG("recvTime is %d", pMsgInfo->displayTime);
	MSG_DEBUG("LENGTH %d", EtwsMsg->pageLength);
	pMsgInfo->bTextSms = true;
	pMsgInfo->dataSize = EtwsMsg->pageLength;
	memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
	memcpy(pMsgInfo->msgData, EtwsMsg->pageData, pMsgInfo->dataSize);
}

int SmsPluginCbMsgHandler::convertTextToUtf8(unsigned char* outBuf, int outBufSize, SMS_CBMSG_S* pCbMsg)
{
	int	convertedTextSize = 0;
	MSG_LANG_INFO_S langInfo = {0, };

	MsgTextConvert *textCvt = MsgTextConvert::instance();

	if (!outBuf || !pCbMsg) {
		MSG_DEBUG("invalid param.\n");
		return 0;
	}

	langInfo.bSingleShift = false;
	langInfo.bLockingShift = false;


	/* Convert Data values */
	if (pCbMsg->codingScheme == SMS_CHARSET_7BIT)
		convertedTextSize = textCvt->convertGSM7bitToUTF8(outBuf, outBufSize, (unsigned char*)pCbMsg->msgData, pCbMsg->msgLength, &langInfo);
	else if (pCbMsg->codingScheme == SMS_CHARSET_UCS2)
		convertedTextSize = textCvt->convertUCS2ToUTF8(outBuf, outBufSize, (unsigned char*)pCbMsg->msgData, pCbMsg->msgLength);

	return convertedTextSize;
}

void SmsPluginCbMsgHandler::addToPageList(SMS_CBMSG_PAGE_S *CbPage)
{
	CB_PAGE_INFO_S tmpInfo;

	tmpInfo.geoScope = CbPage->pageHeader.serialNum.geoScope;
	tmpInfo.msgCode = CbPage->pageHeader.serialNum.msgCode;
	tmpInfo.updateNum = CbPage->pageHeader.serialNum.updateNum;
	tmpInfo.msgId = CbPage->pageHeader.msgId;
	tmpInfo.totalPages = CbPage->pageHeader.totalPages;

	tmpInfo.pageCnt = 1;
	tmpInfo.totalSize = CbPage->pageLength;

	pair<unsigned char, SMS_CBMSG_PAGE_S> newData(CbPage->pageHeader.page, *CbPage);
	tmpInfo.data.insert(newData);

	MSG_DEBUG("MSG DATA : %s", CbPage->pageData);
	MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.pageData);

	pageList.push_back(tmpInfo);
}


void SmsPluginCbMsgHandler::removeFromPageList(SMS_CBMSG_PAGE_S *CbPage)
{
	unsigned int index;

	for (index = 0; index < pageList.size(); index++) {
		if (pageList[index].geoScope == CbPage->pageHeader.serialNum.geoScope && pageList[index].msgCode == CbPage->pageHeader.serialNum.msgCode) {
			MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[index].geoScope, pageList[index].msgCode);

			if (pageList[index].msgId == CbPage->pageHeader.msgId)
				break;
		}
	}

	MSG_DEBUG("remove index [%d]", index);

	pageList.erase(pageList.begin()+index);
}


void SmsPluginCbMsgHandler::decodeCbMsgDCS(unsigned char dcsData, const unsigned char *pMsgData, SMS_CBMSG_DCS_S* pDcs)
{
	pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
	pDcs->classType = SMS_MSG_CLASS_NONE;
	pDcs->bCompressed = false;
	pDcs->codingScheme = SMS_CHARSET_7BIT;
	pDcs->langType = SMS_CBMSG_LANG_UNSPECIFIED;
	memset(pDcs->iso639Lang, 0x00, sizeof(pDcs->iso639Lang));
	pDcs->bUDH = false;
	pDcs->rawData = dcsData;

	unsigned char codingGrp = (dcsData & 0xF0) >> 4;

	switch (codingGrp) {
	case 0x00:
	case 0x02:
	case 0x03: {
		pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
		pDcs->langType = (SMS_CBMSG_LANG_TYPE_T)dcsData;
	}
	break;

	case 0x01: {
		if (dcsData == 0x10 || dcsData == 0x11) {
			pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
			pDcs->codingScheme = (dcsData & 0x01) ? SMS_CHARSET_UCS2 : SMS_CHARSET_7BIT;
			pDcs->langType = SMS_CBMSG_LANG_ISO639;
			MSG_DEBUG("codingScheme: [%d]", pDcs->codingScheme);
			if (pMsgData[0] && pMsgData[1]) {
				pDcs->iso639Lang[0] = pMsgData[0] & 0x7F;
				pDcs->iso639Lang[1] = (pMsgData[0] & 0X80) >> 7;
				pDcs->iso639Lang[1] |= (pMsgData[1] & 0X3F) << 1;
				pDcs->iso639Lang[2] = 0x13;  /* CR */
			} else {
				/* Default it to English if pMsgData is NULL */
				pDcs->iso639Lang[0] = 0x45;  /* E */
				pDcs->iso639Lang[1] = 0x4E;  /* N */
				pDcs->iso639Lang[2] = 0x13;  /* CR */
			}
		}
	}
	break;

	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07: {
		pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;

		pDcs->bCompressed = (dcsData & 0x20) ? true : false;

		if (dcsData & 0x10)
			pDcs->classType = (SMS_MSG_CLASS_T)(dcsData & 0x03);

		unsigned char tmpScheme = (dcsData & 0x0C) >> 2;

		switch (tmpScheme) {
		case 0x00:
			pDcs->codingScheme = SMS_CHARSET_7BIT;
			break;
		case 0x01:
			pDcs->codingScheme = SMS_CHARSET_8BIT;
			break;
		case 0x02:
			pDcs->codingScheme = SMS_CHARSET_UCS2;
			break;
		default:
			MSG_DEBUG("tmpScheme: [%d]", tmpScheme);
			break;
		}
	}
	break;

	case 0x09: {
		pDcs->bUDH = true;
		pDcs->classType = (MSG_CLASS_TYPE_T)(dcsData & 0x03);
		pDcs->codingScheme = (SMS_CODING_SCHEME_T)((dcsData & 0x0C) >> 2);
	}
	break;

	case 0x0E: {
		pDcs->codingGroup = SMS_CBMSG_CODGRP_WAP;
	}
	break;

	case 0x0F: {
		pDcs->codingGroup = SMS_CBMSG_CODGRP_CLASS_CODING;
		pDcs->codingScheme = (dcsData & 0x04) ? SMS_CHARSET_8BIT : SMS_CHARSET_7BIT;
		pDcs->classType = (MSG_CLASS_TYPE_T)(dcsData & 0x03);
	}
	break;
	default:
		MSG_DEBUG("codingGrp: [0x%x]", codingGrp);
		break;
	}
}


void SmsPluginCbMsgHandler::convertLangType(SMS_CBMSG_LANG_TYPE_T InType , MSG_CB_LANGUAGE_TYPE_T *pOutType)
{
	switch (InType) {
	case SMS_CBMSG_LANG_GERMAN :
		*pOutType = MSG_CBLANG_TYPE_GER;
		break;

	case SMS_CBMSG_LANG_ENGLISH :
		*pOutType = MSG_CBLANG_TYPE_ENG;
		break;

	case SMS_CBMSG_LANG_ITALIAN :
		*pOutType = MSG_CBLANG_TYPE_ITA;
		break;

	case SMS_CBMSG_LANG_FRENCH :
		*pOutType = MSG_CBLANG_TYPE_FRE;
		break;

	case SMS_CBMSG_LANG_SPANISH :
		*pOutType = MSG_CBLANG_TYPE_SPA;
		break;

	case SMS_CBMSG_LANG_DUTCH :
		*pOutType = MSG_CBLANG_TYPE_NED;
		break;

	case SMS_CBMSG_LANG_SWEDISH :
		*pOutType = MSG_CBLANG_TYPE_SWE;
		break;

	case SMS_CBMSG_LANG_PORTUGUESE :
		*pOutType = MSG_CBLANG_TYPE_POR;
		break;

	case SMS_CBMSG_LANG_TURKISH :
		*pOutType = MSG_CBLANG_TYPE_TUR;
		break;

	default :
		*pOutType = MSG_CBLANG_TYPE_MAX;
		break;
	}
}


unsigned long SmsPluginCbMsgHandler::getRecvTime()
{
	time_t recvTime;

	recvTime = time(NULL);

	return (unsigned long)recvTime;
}


void SmsPluginCbMsgHandler::getDisplayName(unsigned short	MsgId, char *pDisplayName, msg_sim_slot_id_t simIndex)
{
	MSG_CB_CHANNEL_S cbChannelInfo = {0, };
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	err = MsgStoGetCBChannelInfo(dbHandle, &cbChannelInfo, simIndex);
	MSG_DEBUG("MsgStoGetCBChannelInfo [err = %d]", err);

	for (int i = 0; i < cbChannelInfo.channelCnt; i++) {
		if (MsgId >= cbChannelInfo.channelInfo[i].from && MsgId <= cbChannelInfo.channelInfo[i].to) {
			MSG_DEBUG("FIND MSG ID = [%d]", MsgId);
#if 0
			char strTmp[CB_CHANNEL_NAME_MAX + 1];
			memset(strTmp, 0x00, sizeof(strTmp));

			strncpy(strTmp, cbChannelInfo.channelInfo[i].name, CB_CHANNEL_NAME_MAX);

			if (strlen(strTmp) > 0)
				snprintf(pDisplayName, sizeof(strTmp), "[%s]", strTmp);
			else
				snprintf(pDisplayName, sizeof(unsigned short), "[%d]", MsgId);
#else

#ifdef MSG_NOTI_INTEGRATION
			snprintf(pDisplayName, MAX_ADDRESS_VAL_LEN + 1, "CB message");
#else
			snprintf(pDisplayName, MAX_ADDRESS_VAL_LEN + 1, "[%d]", MsgId);
#endif

#endif
			return;
		}
	}

	snprintf(pDisplayName, MAX_ADDRESS_VAL_LEN + 1, "[%d]", MsgId);
}
