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

#include <time.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFile.h"
#include "SmsPluginUDCodec.h"
#include "SmsPluginStorage.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginCbMsgHandler.h"


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


void SmsPluginCbMsgHandler::handleCbMsg(TelSmsCbMsg_t *pCbMsg)
{
	MSG_BEGIN();

	SMS_CB_NETWORK_TYPE_T type = pCbMsg->CbMsgType;

	SMS_CBMSG_PAGE_S CbMsgPage = {0};

	switch (type)
	{
		case SMS_CB_NETWORK_TYPE_2G_GSM :
			Decode2gCbMsg(pCbMsg, &CbMsgPage);
		break;

		case SMS_CB_NETWORK_TYPE_3G_UMTS :
			Decode3gCbMsg(pCbMsg, &CbMsgPage);
		break;
	}

	// Check CB Msg Options
	bool bJavaMsg = false;

	if (!checkCbOpt(CbMsgPage, &bJavaMsg))
	{
		MSG_DEBUG("The CB Msg is not supported by option.");
		return;
	}

	if (bJavaMsg == true)
	{
		MSG_DEBUG("JAVA CB Msg.");
		CbMsgPage.cbMsgType = SMS_CBMSG_TYPE_JAVACBS;
	}

	// Check CB Pages
	unsigned char pageCnt = checkCbPage(CbMsgPage);

	if (pageCnt == CbMsgPage.pageHeader.totalPages)
	{
		MSG_DEBUG("RECEIVED LAST MSG : %d", pageCnt);

		SMS_CBMSG_S cbMsg = {};
		MSG_MESSAGE_INFO_S msgInfo = {};

		// Make CB Msg Structure
		MakeCbMsg(CbMsgPage, &cbMsg);

		// Convert to MSG_MESSAGE_INFO_S
		convertCbMsgToMsginfo(cbMsg, &msgInfo);

		// Add CB Msg into DB
		msg_error_t err = MSG_SUCCESS;

		err = SmsPluginStorage::instance()->addMessage(&msgInfo);

		if (err == MSG_SUCCESS)
		{
			MSG_CB_MSG_S cbOutMsg = {0, };

			cbOutMsg.type = MSG_CB_SMS;
			cbOutMsg.receivedTime = cbMsg.recvTime;
			cbOutMsg.serialNum = encodeCbSerialNum (CbMsgPage.pageHeader.serialNum);
			cbOutMsg.messageId = cbMsg.msgId;
			cbOutMsg.dcs = CbMsgPage.pageHeader.dcs.rawData;
			memset (cbOutMsg.cbText, 0x00, sizeof(cbOutMsg.cbText));

			cbOutMsg.cbTextLen= convertTextToUtf8((unsigned char*)cbOutMsg.cbText, sizeof(cbOutMsg.cbText), &cbMsg);
			memset(cbOutMsg.language_type, 0x00, sizeof(cbOutMsg.language_type));
			memcpy(cbOutMsg.language_type, CbMsgPage.pageHeader.dcs.iso639Lang, 3);
			err = SmsPluginEventHandler::instance()->callbackCBMsgIncoming(&cbOutMsg);
			if (err != MSG_SUCCESS)
			{
				MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
			}
		}
		else
		{
			MSG_DEBUG("addMessage() Error !! [%d]", err);
		}

		// Remove From List
		removeFromPageList(CbMsgPage);
	}
	MSG_END();
}


void SmsPluginCbMsgHandler::handleEtwsMsg(TelSmsEtwsMsg_t *pEtwsMsg)
{
	MSG_BEGIN();
	msg_error_t err = MSG_SUCCESS;
	SMS_ETWS_NETWORK_TYPE_T type = pEtwsMsg->EtwsMsgType;
	//MSG_MESSAGE_INFO_S msgInfo = {};
	SMS_ETWS_PRIMARY_S		etwsPn = {0, };
	MSG_CB_MSG_S			cbOutMsg = {0, };

	if(type != TAPI_NETTEXT_ETWS_PRIMARY)
	{
		MSG_DEBUG("The Etws Msg is not supported");
		return;
	}
	DecodeEtwsMsg(pEtwsMsg, &etwsPn);
	//convertEtwsMsgToMsginfo(CbMsgPage, &msgInfo);

	cbOutMsg.type = MSG_ETWS_SMS;
	cbOutMsg.receivedTime = etwsPn.recvTime;
	cbOutMsg.serialNum = encodeCbSerialNum (etwsPn.serialNum);
	cbOutMsg.messageId = etwsPn.msgId;
	cbOutMsg.etwsWarningType = etwsPn.warningType;
	memcpy (cbOutMsg.etwsWarningSecurityInfo, etwsPn.warningSecurityInfo, sizeof(cbOutMsg.etwsWarningSecurityInfo));

	err = SmsPluginEventHandler::instance()->callbackCBMsgIncoming(&cbOutMsg);
	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
	}

	MSG_END();
}


void SmsPluginCbMsgHandler::Decode2gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage)
{
	if (pCbMsg->Length > MAX_CBMSG_PAGE_SIZE)
		THROW(MsgException::SMS_PLG_ERROR, "CB Msg Size is over MAX [%d]", pCbMsg->Length);

	unsigned char cbData[pCbMsg->Length+1];

	memset(cbData, 0x00, sizeof(cbData));
	memcpy(cbData, pCbMsg->szMsgData, pCbMsg->Length);
	cbData[pCbMsg->Length] = '\0';

	pCbPage->cbMsgType = SMS_CBMSG_TYPE_CBS;

	// Serial Number
	pCbPage->pageHeader.serialNum.geoScope = (cbData[0] & 0xC0) >> 6;

	pCbPage->pageHeader.serialNum.msgCode = (cbData[0] & 0x3F) << 4;
	pCbPage->pageHeader.serialNum.msgCode |= (cbData[1] & 0xF0) >> 4;

	pCbPage->pageHeader.serialNum.updateNum = cbData[1] & 0x0F;

MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pCbPage->pageHeader.serialNum.geoScope, pCbPage->pageHeader.serialNum.msgCode, pCbPage->pageHeader.serialNum.updateNum);

	pCbPage->pageHeader.msgId = (cbData[2] << 8) | cbData[3];

MSG_DEBUG("MSG ID : [%d]", pCbPage->pageHeader.msgId);

	// DCS
	decodeCbMsgDCS(cbData[4], (unsigned char*)cbData[6], &(pCbPage->pageHeader.dcs));

	// Page Parameter
	pCbPage->pageHeader.totalPages = cbData[5] & 0x0F;
	pCbPage->pageHeader.page = (cbData[5] & 0xF0) >> 4;

	if (pCbPage->pageHeader.totalPages > MAX_CBMSG_PAGE_NUM)
		THROW(MsgException::SMS_PLG_ERROR, "CB Page Count is over MAX[%d]", pCbPage->pageHeader.totalPages);

MSG_DEBUG("Total Page : [%d], Page : [%d]", pCbPage->pageHeader.totalPages, pCbPage->pageHeader.page);

	// Convert Language Type
	convertLangType(pCbPage->pageHeader.dcs.langType, &(pCbPage->pageHeader.langType));

MSG_DEBUG("In Language Type : [%d], Out Language Type : [%d]", pCbPage->pageHeader.dcs.langType, pCbPage->pageHeader.langType);
MSG_DEBUG("iso639Lang : [%s]", pCbPage->pageHeader.dcs.iso639Lang);
	// Get Receive Time
	pCbPage->pageHeader.recvTime = getRecvTime();

	// Decode CB Data
	int dataLen = pCbMsg->Length - 6;

	switch (pCbPage->pageHeader.dcs.codingScheme)
	{
		case SMS_CHARSET_7BIT :
		{
			MSG_DEBUG("GSM 7 BIT");

			dataLen = (dataLen*8) / 7;

			SmsPluginUDCodec udCodec;
			char pageData[MAX_CBMSG_PAGE_SIZE+1];
			int unpackLen = udCodec.unpack7bitChar(&cbData[6], dataLen, 0, pageData);

			if(pCbPage->pageHeader.dcs.iso639Lang[0])
			{
				unpackLen = unpackLen - 3;
				memcpy(pCbPage->pageData, &pageData[3], unpackLen);
			} else {
				memcpy(pCbPage->pageData, &pageData, unpackLen);
			}

			MSG_DEBUG("unpackLen : [%d]", unpackLen);

			pCbPage->pageLength = unpackLen;
			pCbPage->pageData[unpackLen] = '\0';
		}
		break;

		case SMS_CHARSET_8BIT :
		case SMS_CHARSET_UCS2 :
		{
			MSG_DEBUG("UCS2");

			if(pCbPage->pageHeader.dcs.iso639Lang[0])
			{
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

	// received time
	pEtwsPn->recvTime = getRecvTime();

	// Serial Number
	pEtwsPn->serialNum.geoScope = (EtwsData[0] & 0xC0) >> 6;
	pEtwsPn->serialNum.msgCode = (EtwsData[0] & 0x3F) << 4;
	pEtwsPn->serialNum.msgCode |= (EtwsData[1] & 0xF0) >> 4;
	pEtwsPn->serialNum.updateNum = EtwsData[1] & 0x0F;

	MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pEtwsPn->serialNum.geoScope, pEtwsPn->serialNum.msgCode, pEtwsPn->serialNum.updateNum);

	// Message Identifier
	pEtwsPn->msgId = (EtwsData[2] << 8) | EtwsData[3];
	MSG_DEBUG("MSG ID : [%d]", pEtwsPn->msgId);

	// warning type
	pEtwsPn->warningType = (EtwsData[4] << 8) | EtwsData[5];
	MSG_DEBUG("warningType : [0x%04x]", pEtwsPn->msgId);

	// warning security information
	memcpy(pEtwsPn->warningSecurityInfo, &EtwsData[6], sizeof(pEtwsPn->warningSecurityInfo));	// 50bytes
	for (unsigned int i = 0; i < sizeof(pEtwsPn->warningSecurityInfo); i++)
	{
		MSG_DEBUG("warning secu info [%02x]", pEtwsPn->warningSecurityInfo[i] );
	}
}

void SmsPluginCbMsgHandler::Decode3gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage)
{
	if (pCbMsg->Length > (MAX_CBMSG_PAGE_SIZE*MAX_CBMSG_PAGE_NUM))
		THROW(MsgException::SMS_PLG_ERROR, "CB Msg Size is over MAX [%d]", pCbMsg->Length);

	char cbData[(MAX_CBMSG_PAGE_SIZE*MAX_CBMSG_PAGE_NUM)+1];

	memset(cbData, 0x00, sizeof(cbData));
	memcpy(cbData, pCbMsg->szMsgData, pCbMsg->Length);
	cbData[pCbMsg->Length] = '\0';

	pCbPage->cbMsgType = (SMS_CBMSG_TYPE_T)cbData[0];

	pCbPage->pageHeader.msgId = (cbData[1] << 8) | cbData[2];

MSG_DEBUG("MSG ID : [%d]", pCbPage->pageHeader.msgId);

	// Serial Number
	pCbPage->pageHeader.serialNum.geoScope = (cbData[3] & 0xC0) >> 6;

	pCbPage->pageHeader.serialNum.msgCode = (cbData[3] & 0x3F) << 4;
	pCbPage->pageHeader.serialNum.msgCode |= (cbData[4] & 0xF0) >> 4;

	pCbPage->pageHeader.serialNum.updateNum = cbData[4] & 0x0F;

MSG_DEBUG("geoScope : [%d], msgCode : [%d], updateNum : [%d]", pCbPage->pageHeader.serialNum.geoScope, pCbPage->pageHeader.serialNum.msgCode, pCbPage->pageHeader.serialNum.updateNum);

	// DCS
	decodeCbMsgDCS(cbData[5], (unsigned char*)cbData[6], &(pCbPage->pageHeader.dcs));

	// Convert Language Type
	convertLangType(pCbPage->pageHeader.dcs.langType, &(pCbPage->pageHeader.langType));

MSG_DEBUG("In Language Type : [%d], Out Language Type : [%d]", pCbPage->pageHeader.dcs.langType, pCbPage->pageHeader.langType);

	// Get Receive Time
	pCbPage->pageHeader.recvTime = getRecvTime();

#if 0
	// Decode CB Data
	int dataLen = pCbMsg->Length - 6;

	switch (pCbPage->pageHeader.dcs.codingScheme)
	{
		case SMS_CHARSET_7BIT :
		{
			dataLen = (dataLen*8) / 7;

			if (pCbPage->pageLength > MAX_CBMSG_PAGE_SIZE)
				THROW(MsgException::SMS_PLG_ERROR, "CB Msg Size is over MAX [%d]", pCbPage->pageLength);

			SmsPluginUDCodec udCodec;
			int unpackLen = udCodec.unpack7bitChar(&cbData[6], dataLen, 0, pCbPage->pageData);
			pCbPage->pageData[unpackLen] = '\0';

			pCbPage->pageLength = unpackLen;
		}
		break;

		case SMS_CHARSET_8BIT :
		case SMS_CHARSET_UCS2 :
		{
			pCbPage->pageLength = dataLen;

			memcpy(pCbPage->pageData, &cbData[6], pCbPage->pageLength);
			pCbPage->pageData[pCbPage->pageLength] = '\0';
		}
		break;
	}

MSG_DEBUG("Page Length : [%d], Page Data : [%s]", pCbPage->pageLength, pCbPage->pageData);
#endif
}

unsigned short SmsPluginCbMsgHandler::encodeCbSerialNum ( SMS_CBMSG_SERIAL_NUM_S snFields )
{
	unsigned short serialNum = 0;

	serialNum = ((snFields.geoScope & 0x03) << 14) | ((snFields.msgCode&0x03FF) << 4) | (snFields.updateNum&0x0F);
	MSG_DEBUG ("serialNum (%x), geo(%x), mc(%x), un(%x)\n", serialNum, snFields.geoScope, snFields.msgCode, snFields.updateNum);

	return serialNum;
}

bool SmsPluginCbMsgHandler::checkCbOpt(SMS_CBMSG_PAGE_S CbPage, bool *pJavaMsg)
{
	bool bReceive = false;
	MsgSettingGetBool(CB_RECEIVE, &bReceive);

	// Receive CB Msg = FALSE
	if (!bReceive)
	{
		MSG_DEBUG("RECEIVE CB = FALSE");
		return false;
	}

	char keyName[128];

	// check Language
	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", CB_LANGUAGE, MSG_CBLANG_TYPE_ALL);

	bool bAllLang = false;
	MsgSettingGetBool(keyName, &bAllLang);

	if (!bAllLang)
	{
		MSG_DEBUG("ALL LANGUAGE = FALSE");

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_LANGUAGE, CbPage.pageHeader.langType);

		bool bLang = false;

		MsgSettingGetBool(keyName, &bLang);

		if (!bLang || CbPage.pageHeader.langType == MSG_CBLANG_TYPE_MAX)
		{
			MSG_DEBUG("LANGUAGE [%d] = FALSE", CbPage.pageHeader.langType);
			return false;
		}
	}

	int MsgIdCnt = MsgSettingGetInt(CB_CHANNEL_COUNT);

	bool bActivate = false;
	int MsgId_from = 0, MsgId_to = 0;

	for (int i = 0; i < MsgIdCnt; i++)
	{
		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ACTIVATE, i);

		MsgSettingGetBool(keyName, &bActivate);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_FROM, i);

		MsgId_from = MsgSettingGetInt(keyName);

		memset(keyName, 0x00, sizeof(keyName));
		sprintf(keyName, "%s/%d", CB_CHANNEL_ID_TO, i);

		MsgId_to = MsgSettingGetInt(keyName);

		if (bActivate == true && CbPage.pageHeader.msgId >= MsgId_from && CbPage.pageHeader.msgId <= MsgId_to)
		{
			MSG_DEBUG("FIND CHANNEL = [%d]", CbPage.pageHeader.msgId);
			return true;
		}
	}

	return false;
}


unsigned char SmsPluginCbMsgHandler::checkCbPage(SMS_CBMSG_PAGE_S CbPage)
{
	unsigned char currPageCnt = 0;

	bool bFind = false;

	if (CbPage.pageHeader.totalPages > 1)
	{
		for (unsigned int i = 0; i < pageList.size(); i++)
		{
			if (pageList[i].geoScope == CbPage.pageHeader.serialNum.geoScope && pageList[i].msgCode == CbPage.pageHeader.serialNum.msgCode)
			{
				MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[i].geoScope, pageList[i].msgCode);

				if (pageList[i].msgId == CbPage.pageHeader.msgId)
				{
					int updateNum = CbPage.pageHeader.serialNum.updateNum - pageList[i].updateNum;

					if (updateNum > 0) // New Message Content
					{
						break;
					}
					else if (updateNum == 0) // Same Message Content
					{
						if (pageList[i].data.count(CbPage.pageHeader.page) != 0)
						{
							MSG_DEBUG("The Page Number already exists [%d]", CbPage.pageHeader.page);
							return 0;
						}

						pair<unsigned char, string> newData(CbPage.pageHeader.page, CbPage.pageData);
						pageList[i].data.insert(newData);

						MSG_DEBUG("PAGE DATA : %s", CbPage.pageData);
						MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.c_str());

						pageList[i].pageCnt++;
						pageList[i].totalSize += CbPage.pageLength;

						currPageCnt = pageList[i].pageCnt;

						bFind = true;

						break;
					}
					else // Old Message Content
					{
						return 0;
					}
				}
			}
		}
	}

	if (bFind == false || CbPage.pageHeader.totalPages == 1)
	{
		addToPageLiat(CbPage);
		return 1;
	}

	return currPageCnt;
}


void SmsPluginCbMsgHandler::MakeCbMsg(SMS_CBMSG_PAGE_S CbPage, SMS_CBMSG_S *pCbMsg)
{
	pCbMsg->cbMsgType = CbPage.cbMsgType;
	pCbMsg->msgId = CbPage.pageHeader.msgId;
	pCbMsg->classType = CbPage.pageHeader.dcs.classType;
	pCbMsg->codingScheme = CbPage.pageHeader.dcs.codingScheme;
	pCbMsg->recvTime = CbPage.pageHeader.recvTime;

	cbPageMap::iterator it;
	string tmpStr ("");

	for (unsigned int i = 0; i < pageList.size(); i++)
	{
		if (pageList[i].geoScope == CbPage.pageHeader.serialNum.geoScope && pageList[i].msgCode == CbPage.pageHeader.serialNum.msgCode)
		{
			MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[i].geoScope, pageList[i].msgCode);

			if (pageList[i].msgId == CbPage.pageHeader.msgId)
			{
				for (it = pageList[i].data.begin(); it != pageList[i].data.end(); it++)
				{
					tmpStr += it->second;
				}
			}
		}
	}

	pCbMsg->msgLength = tmpStr.size();

	memcpy(pCbMsg->msgData, tmpStr.c_str(), tmpStr.size());
	pCbMsg->msgData[tmpStr.size()] = '\0';

	MSG_DEBUG("SIZE : [%d] TOTAL MSG : %s", tmpStr.size(), tmpStr.c_str());
}


void SmsPluginCbMsgHandler::convertCbMsgToMsginfo(SMS_CBMSG_S cbMsg, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	pMsgInfo->msgId = (msg_message_id_t)cbMsg.msgId;

	pMsgInfo->folderId = MSG_CBMSGBOX_ID;

	// Convert Type values
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;

	if (cbMsg.cbMsgType == SMS_CBMSG_TYPE_CBS)
		pMsgInfo->msgType.subType = MSG_CB_SMS;
	else if (cbMsg.cbMsgType == SMS_CBMSG_TYPE_JAVACBS)
		pMsgInfo->msgType.subType = MSG_JAVACB_SMS;

	switch(cbMsg.classType)
	{
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
	}

	pMsgInfo->storageId = MSG_STORAGE_PHONE;
	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	// Temporary
	pMsgInfo->nAddressCnt = 1;

	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
	pMsgInfo->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_UNKNOWN;

	getDisplayName(cbMsg.msgId, pMsgInfo->addressList[0].addressVal);
	MSG_DEBUG("%s", pMsgInfo->addressList[0].addressVal);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	pMsgInfo->displayTime = cbMsg.recvTime;
	MSG_DEBUG("recvTime is %s", ctime(&pMsgInfo->displayTime));

	int bufSize = cbMsg.msgLength*2;

	char tmpBuf[bufSize];
	memset(tmpBuf, 0x00, sizeof(tmpBuf));

	MSG_DEBUG("LENGTH %d CB MSG %s", cbMsg.msgLength, cbMsg.msgData);

	// Convert Data values
	pMsgInfo->dataSize = convertTextToUtf8((unsigned char*)tmpBuf, bufSize, &cbMsg);

	if (pMsgInfo->dataSize > MAX_MSG_TEXT_LEN)
	{
		pMsgInfo->bTextSms = false;

		// Save Message Data into File
		char fileName[MAX_COMMON_INFO_SIZE+1];
		memset(fileName, 0x00, sizeof(fileName));

		if (MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

		MSG_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsgInfo->dataSize, fileName);
		if (MsgWriteIpcFile(fileName, tmpBuf, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

		strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);
	}
	else
	{
		pMsgInfo->bTextSms = true;

		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		memcpy(pMsgInfo->msgText, tmpBuf, pMsgInfo->dataSize);
	}
}


void SmsPluginCbMsgHandler::convertEtwsMsgToMsginfo(SMS_CBMSG_PAGE_S EtwsMsg, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	pMsgInfo->msgId = (msg_message_id_t)EtwsMsg.pageHeader.msgId;

	pMsgInfo->folderId = MSG_CBMSGBOX_ID;

	// Convert Type values
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;

	if (EtwsMsg.cbMsgType == SMS_CBMSG_TYPE_ETWS)
		pMsgInfo->msgType.subType = MSG_ETWS_SMS;

	pMsgInfo->storageId = MSG_STORAGE_PHONE;
	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	// Temporary
	pMsgInfo->nAddressCnt = 1;

	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
	pMsgInfo->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_UNKNOWN;

	getDisplayName(EtwsMsg.pageHeader.msgId, pMsgInfo->addressList[0].addressVal);
	MSG_DEBUG("%s", pMsgInfo->addressList[0].addressVal);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	pMsgInfo->displayTime = EtwsMsg.pageHeader.recvTime;
	MSG_DEBUG("recvTime is %s", ctime(&pMsgInfo->displayTime));
	MSG_DEBUG("LENGTH %d", EtwsMsg.pageLength);
	pMsgInfo->bTextSms = true;
	pMsgInfo->dataSize = EtwsMsg.pageLength;
	memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
	memcpy(pMsgInfo->msgData, EtwsMsg.pageData, pMsgInfo->dataSize);
}

int SmsPluginCbMsgHandler::convertTextToUtf8 (unsigned char* outBuf, int outBufSize, SMS_CBMSG_S* pCbMsg)
{
	int	convertedTextSize = 0;
	MSG_LANG_INFO_S langInfo = {0,};

	if (!outBuf || !pCbMsg)
	{
		MSG_DEBUG ("invalid param.\n");
		return 0;
	}

	langInfo.bSingleShift = false;
	langInfo.bLockingShift = false;


	// Convert Data values
	if (pCbMsg->codingScheme == SMS_CHARSET_7BIT)
		convertedTextSize = textCvt.convertGSM7bitToUTF8(outBuf, outBufSize, (unsigned char*)pCbMsg->msgData, pCbMsg->msgLength, &langInfo);
	else if (pCbMsg->codingScheme == SMS_CHARSET_UCS2)
		convertedTextSize = textCvt.convertUCS2ToUTF8(outBuf, outBufSize, (unsigned char*)pCbMsg->msgData, pCbMsg->msgLength);

	return convertedTextSize;
}

void SmsPluginCbMsgHandler::addToPageLiat(SMS_CBMSG_PAGE_S CbPage)
{
	CB_PAGE_INFO_S tmpInfo;

	tmpInfo.geoScope = CbPage.pageHeader.serialNum.geoScope;
	tmpInfo.msgCode = CbPage.pageHeader.serialNum.msgCode;
	tmpInfo.updateNum = CbPage.pageHeader.serialNum.updateNum;
	tmpInfo.msgId = CbPage.pageHeader.msgId;
	tmpInfo.totalPages = CbPage.pageHeader.totalPages;

	tmpInfo.pageCnt = 1;
	tmpInfo.totalSize = CbPage.pageLength;

	pair<unsigned char, string> newData(CbPage.pageHeader.page, CbPage.pageData);
	tmpInfo.data.insert(newData);

	MSG_DEBUG("MSG DATA : %s", CbPage.pageData);
	MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.c_str());

	pageList.push_back(tmpInfo);
}


void SmsPluginCbMsgHandler::removeFromPageList(SMS_CBMSG_PAGE_S CbPage)
{
	unsigned int index;

	for (index = 0; index < pageList.size(); index++)
	{
		if (pageList[index].geoScope == CbPage.pageHeader.serialNum.geoScope && pageList[index].msgCode == CbPage.pageHeader.serialNum.msgCode)
		{
			MSG_DEBUG("geoScope [%d], msgCode [%d]", pageList[index].geoScope, pageList[index].msgCode);

			if (pageList[index].msgId == CbPage.pageHeader.msgId) break;
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

	switch (codingGrp)
	{
		case 0x00 :
		case 0x02 :
		case 0x03 :
		{
			pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
			pDcs->langType = (SMS_CBMSG_LANG_TYPE_T)dcsData;
		}
		break;

		case 0x01 :
		{
			if (dcsData == 0x10 || dcsData == 0x11)
			{
				pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;
				pDcs->codingScheme = (dcsData & 0x01) ? SMS_CHARSET_UCS2 : SMS_CHARSET_7BIT;
				pDcs->langType = SMS_CBMSG_LANG_ISO639;

				if (pMsgData != NULL)
				{
					pDcs->iso639Lang[0] = pMsgData[0] & 0x7F;
					pDcs->iso639Lang[1] = (pMsgData[0] & 0x80) >> 7;
					pDcs->iso639Lang[1] |= (pMsgData[1] & 0x3F) << 1;
					pDcs->iso639Lang[2]  = 0x13; /* CR char in GSM 7-bit Alphabet */
				}
				else
				{
					/* Default it to English if pMsgData is NULL */
					pDcs->iso639Lang[0] = 0x45;  /* E */
					pDcs->iso639Lang[1] = 0x4E;  /* N */
					pDcs->iso639Lang[2] = 0x13;  /* CR */
				}
			}
		}
		break;

		case 0x04 :
		case 0x05 :
		case 0x06 :
		case 0x07 :
		{
			pDcs->codingGroup = SMS_CBMSG_CODGRP_GENERAL_DCS;

			pDcs->bCompressed = (dcsData & 0x20) ? true : false;

			if (dcsData & 0x10)
				pDcs->classType = (SMS_MSG_CLASS_T)(dcsData & 0x03);

			pDcs->codingScheme = (SMS_CODING_SCHEME_T)(dcsData & 0x0C);
		}
		break;

		case 0x09 :
		{
			pDcs->bUDH = true;
		}
		break;

		case 0x14 :
		{
			pDcs->codingGroup = SMS_CBMSG_CODGRP_WAP;
		}
		break;

		case 0x15 :
		{
			pDcs->codingGroup = SMS_CBMSG_CODGRP_CLASS_CODING;
			pDcs->codingScheme = (dcsData & 0x04) ? SMS_CHARSET_UCS2 : SMS_CHARSET_7BIT;
			pDcs->classType = (MSG_CLASS_TYPE_T)(dcsData & 0x03);
		}
		break;
	}
}


void SmsPluginCbMsgHandler::convertLangType(SMS_CBMSG_LANG_TYPE_T InType , MSG_CB_LANGUAGE_TYPE_T *pOutType)
{
	switch (InType)
	{
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


void SmsPluginCbMsgHandler::getDisplayName(unsigned short	MsgId, char *pDisplayName)
{
	int MsgIdCnt = MsgSettingGetInt(CB_CHANNEL_COUNT);

	char from[128];
	char to[128];

	for (int i = 0; i < MsgIdCnt; i++)
	{
		memset(from, 0x00, sizeof(from));
		sprintf(from, "%s/%d", CB_CHANNEL_ID_FROM, i);

		memset(to, 0x00, sizeof(to));
		sprintf(to, "%s/%d", CB_CHANNEL_ID_TO, i);

		if (MsgId >= MsgSettingGetInt(from) && MsgId <= MsgSettingGetInt(to))
		{
			MSG_DEBUG("FIND MSG ID = [%d]", MsgId);
#if 0
			memset(keyName, 0x00, sizeof(keyName));
			sprintf(keyName, "%s/%d", CB_CHANNEL_NAME, i);

			memset(strTmp, 0x00, sizeof(strTmp));

			channelName = MsgSettingGetString(keyName);

			strncpy(strTmp, channelName, CB_CHANNEL_NAME_MAX);

			if (channelName) {
				free(channelName);
				channelName = NULL;
			}

			if (strlen(strTmp) > 0)
				sprintf(pDisplayName, "[%s]", strTmp);
			else
				sprintf(pDisplayName, "[%d]", MsgId);
#else
			sprintf(pDisplayName, "[%d]", MsgId);
#endif

			return;
		}
	}

	sprintf(pDisplayName, "[%d]", MsgId);
}

