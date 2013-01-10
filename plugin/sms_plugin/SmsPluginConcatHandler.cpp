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
#include "MsgException.h"
#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "SmsPluginStorage.h"
#include "SmsPluginTransport.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginWapPushHandler.h"
#include "SmsPluginConcatHandler.h"

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginConcatHandler - Member Functions
==================================================================================================*/
SmsPluginConcatHandler* SmsPluginConcatHandler::pInstance = NULL;


SmsPluginConcatHandler::SmsPluginConcatHandler()
{
	concatList.clear();
}


SmsPluginConcatHandler::~SmsPluginConcatHandler()
{
	concatList.clear();
}


SmsPluginConcatHandler* SmsPluginConcatHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginConcatHandler();

	return pInstance;
}


bool SmsPluginConcatHandler::IsConcatMsg(SMS_USERDATA_S *pUserData)
{
	MSG_BEGIN();

	MSG_DEBUG("headerCnt [%d]", pUserData->headerCnt);

	for (int i = 0; i < pUserData->headerCnt; i++) {
		/**  Handler Concatenated Message */
		if (pUserData->header[i].udhType == SMS_UDH_CONCAT_8BIT) {
			return true;
		} else if (pUserData->header[i].udhType == SMS_UDH_CONCAT_16BIT) {
			return true;
		}
	}

	MSG_END();

	return false;
}


void SmsPluginConcatHandler::handleConcatMsg(SMS_TPDU_S *pTpdu)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	bool noneConcatTypeHeader = true;

	if (pTpdu->tpduType != SMS_TPDU_DELIVER) {
		MSG_DEBUG("The TPDU type is not deliver [%d]", pTpdu->tpduType);
		return;
	}

	SMS_CONCAT_MSG_S msg = {0};

	for (int i = 0; i < pTpdu->data.deliver.userData.headerCnt; i++) {
		if (pTpdu->data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT) {
			msg.msgRef = (unsigned short)pTpdu->data.deliver.userData.header[i].udh.concat8bit.msgRef;
			msg.totalSeg = pTpdu->data.deliver.userData.header[i].udh.concat8bit.totalSeg;
			msg.seqNum = pTpdu->data.deliver.userData.header[i].udh.concat8bit.seqNum;

			memcpy(&(msg.timeStamp.time.absolute), &(pTpdu->data.deliver.timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
			memcpy(&(msg.originAddress), &(pTpdu->data.deliver.originAddress), sizeof(SMS_ADDRESS_S));
			memcpy(&(msg.dcs), &(pTpdu->data.deliver.dcs), sizeof(SMS_DCS_S));

#if 0
			if (msg.totalSeg > MAX_SEGMENT_NUM) {
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}
#endif
			/**  check noneConcatTypeHeader */
			noneConcatTypeHeader = false;

			break;
		} else if (pTpdu->data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT) {
			msg.msgRef = (unsigned short)pTpdu->data.deliver.userData.header[i].udh.concat16bit.msgRef;
			msg.totalSeg = pTpdu->data.deliver.userData.header[i].udh.concat16bit.totalSeg;
			msg.seqNum = pTpdu->data.deliver.userData.header[i].udh.concat16bit.seqNum;

			memcpy(&(msg.timeStamp.time.absolute), &(pTpdu->data.deliver.timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
			memcpy(&(msg.originAddress), &(pTpdu->data.deliver.originAddress), sizeof(SMS_ADDRESS_S));
			memcpy(&(msg.dcs), &(pTpdu->data.deliver.dcs), sizeof(SMS_DCS_S));
#if 0
			if (msg.totalSeg > MAX_SEGMENT_NUM) {
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}
#endif

			/**  check noneConcatTypeHeader */
			noneConcatTypeHeader = false;

			break;
		}
	}

	unsigned char segCnt = checkConcatMsg(&msg, &(pTpdu->data.deliver.userData));

	MSG_DEBUG("segCnt [%d]", segCnt);
	MSG_DEBUG("msg.totalSeg [%d]", msg.totalSeg);

	if ((segCnt == msg.totalSeg) || noneConcatTypeHeader) {
		MSG_DEBUG("RECEIVED LAST CONCAT : %d", segCnt);

		int dataSize = 0;
		char* pUserData = NULL;
		AutoPtr<char> dataBuf(&pUserData);

		MSG_MESSAGE_INFO_S msgInfo = {0};

		dataSize = makeConcatUserData(msg.msgRef, &pUserData);

		if (dataSize > 0) {
			if (SmsPluginWapPushHandler::instance()->IsWapPushMsg(&(pTpdu->data.deliver.userData)) == true) {
				SmsPluginWapPushHandler::instance()->copyDeliverData(&(pTpdu->data.deliver));
				SmsPluginWapPushHandler::instance()->handleWapPushMsg(pUserData, dataSize);
			} else {
				convertConcatToMsginfo(&(pTpdu->data.deliver), pUserData, dataSize, &msgInfo);

				if (msgInfo.msgPort.valid == false) {
					/** Add Concat Msg into DB */
					err = SmsPluginStorage::instance()->addMessage(&msgInfo);
				}

				if (err == MSG_SUCCESS) {
					/** Callback */
					err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo);

					if (err != MSG_SUCCESS) {
						MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
					}
				} else {
					MSG_DEBUG("addMessage() Error !! [%d]", err);
				}
			}
		}

		removeFromConcatList(msg.msgRef);
	}

	/** Send Deliver Report */
	SmsPluginTransport::instance()->sendDeliverReport(err);

	MSG_END();
}

#ifdef CONCAT_SIM_MSG_OPERATION
void SmsPluginConcatHandler::handleConcatMsg(SMS_TPDU_S *pTpdu, msg_sim_id_t SimMsgId, bool bRead)
{
	MSG_BEGIN();

	if (pTpdu->tpduType != SMS_TPDU_DELIVER)
	{
		MSG_DEBUG("The TPDU type is not deliver [%d]", pTpdu->tpduType);
		return;
	}

	SMS_CONCAT_MSG_S msg;
	memset(&msg, 0x00, sizeof(SMS_CONCAT_MSG_S));

	for (int i = 0; i < pTpdu->data.deliver.userData.headerCnt; i++)
	{
		if (pTpdu->data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_8BIT)
		{
			msg.msgRef = (unsigned short)pTpdu->data.deliver.userData.header[i].udh.concat8bit.msgRef;
			msg.totalSeg = pTpdu->data.deliver.userData.header[i].udh.concat8bit.totalSeg;
			msg.seqNum = pTpdu->data.deliver.userData.header[i].udh.concat8bit.seqNum;

			memcpy(&(msg.timeStamp.time.absolute), &(pTpdu->data.deliver.timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
			memcpy(&(msg.originAddress), &(pTpdu->data.deliver.originAddress), sizeof(SMS_ADDRESS_S));
			memcpy(&(msg.dcs), &(pTpdu->data.deliver.dcs), sizeof(SMS_DCS_S));

			msg.bRead = bRead;

			if (msg.totalSeg > MAX_SEGMENT_NUM)
			{
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}

			break;
		}
		else if (pTpdu->data.deliver.userData.header[i].udhType == SMS_UDH_CONCAT_16BIT)
		{
			msg.msgRef = (unsigned short)pTpdu->data.deliver.userData.header[i].udh.concat16bit.msgRef;
			msg.totalSeg = pTpdu->data.deliver.userData.header[i].udh.concat16bit.totalSeg;
			msg.seqNum = pTpdu->data.deliver.userData.header[i].udh.concat16bit.seqNum;

			memcpy(&(msg.timeStamp.time.absolute), &(pTpdu->data.deliver.timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
			memcpy(&(msg.originAddress), &(pTpdu->data.deliver.originAddress), sizeof(SMS_ADDRESS_S));
			memcpy(&(msg.dcs), &(pTpdu->data.deliver.dcs), sizeof(SMS_DCS_S));

			msg.bRead = bRead;

			if (msg.totalSeg > MAX_SEGMENT_NUM)
			{
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}

			break;
		}
	}

	unsigned char segCnt = checkConcatMsg(&msg, &(pTpdu->data.deliver.userData));

	addToSimIdList(msg.msgRef, SimMsgId);

	if (segCnt == msg.totalSeg)
	{
		MSG_DEBUG("RECEIVED LAST CONCAT : %d", segCnt);

		int dataSize = 0;
		char* pUserData = NULL;
		AutoPtr<char> dataBuf(&pUserData);

		MSG_MESSAGE_INFO_S msgInfo = {0};

		dataSize = makeConcatUserData(msg.msgRef, &pUserData);

		if (dataSize >= 0)
		{
			MSG_DEBUG("TOTAL DATA : %s", pUserData);

			convertSimMsgToMsginfo(&msg, pUserData, dataSize, &msgInfo);

			// set Sim Message ID
			msgInfo.msgId = SimMsgId;

			// set read status
			msgInfo.bRead = bRead;

			/// Print MSG_MESSAGE_INFO_S
			MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");

			MSG_DEBUG("msgInfo.msgId : %d", msgInfo.msgId);
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
			MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);

			if (msgInfo.bTextSms == true)
				MSG_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
			else
				MSG_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);

			MSG_DEBUG("###############################################################");

			// Remove from List
			removeFromConcatList(msg.msgRef);
			removeFromSimIdList(msg.msgRef);

			//add msgInfo to msg list
			SmsPluginStorage::instance()->addSimMsgToList(&msgInfo, true);

			// Callback to MSG FW
			SmsPluginEventHandler::instance()->callbackGetSimMsg();
		}
	}
	else
	{
		//add index count to msg list
		SmsPluginStorage::instance()->addSimMsgToList(NULL, false);

		// Callback to MSG FW
		SmsPluginEventHandler::instance()->callbackGetSimMsg();
	}

	MSG_END();
}


void SmsPluginConcatHandler::handleBrokenMsg()
{
	if (concatList.size() <= 0 || simIdList.size() <= 0)
	{
		MSG_DEBUG("No Broken Concatenated Message");
		return;
	}

	do
	{
		int index = 0, dataSize = 0;
		char* pUserData = NULL;
		AutoPtr<char> dataBuf(&pUserData);

		MSG_MESSAGE_INFO_S msgInfo = {0};

		dataSize = makeConcatUserData(concatList[index].msgRef, &pUserData);

		if (dataSize > 0)
		{
			MSG_DEBUG("TOTAL DATA : %s", pUserData);

			SMS_CONCAT_MSG_S msg;
			memset(&msg, 0x00, sizeof(SMS_CONCAT_MSG_S));

			msg.msgRef = concatList[index].msgRef;
			msg.totalSeg = concatList[index].totalSeg;

			memcpy(&(msg.timeStamp.time.absolute), &(concatList[index].timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
			memcpy(&(msg.originAddress), &(concatList[index].originAddress), sizeof(SMS_ADDRESS_S));
			memcpy(&(msg.dcs), &(concatList[index].dcs), sizeof(SMS_DCS_S));

			convertSimMsgToMsginfo(&msg, pUserData, dataSize, &msgInfo);

			// set Sim Message ID
			msgInfo.msgId = 0;

			// set read status
			msgInfo.bRead = concatList[index].bRead;

			/// Print MSG_MESSAGE_INFO_S
			MSG_DEBUG("############# Convert  tpdu values to Message Info values ####################");
			MSG_DEBUG("msgInfo.msgId : %d", msgInfo.msgId);
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
			MSG_DEBUG("msgInfo.dataSize : %d", msgInfo.dataSize);
			if (msgInfo.bTextSms == true)
				MSG_DEBUG("msgInfo.msgText : %s", msgInfo.msgText);
			else
			MSG_DEBUG("msgInfo.msgData : %s", msgInfo.msgData);
			MSG_DEBUG("###############################################################");

			//add msgInfo to msg list
			SmsPluginStorage::instance()->addSimMsgToList(&msgInfo, true);
		}

		removeFromConcatList(concatList[index].msgRef);
		removeFromSimIdList(concatList[index].msgRef);
	}while (concatList.size() > 0);
}
#endif


unsigned char SmsPluginConcatHandler::checkConcatMsg(SMS_CONCAT_MSG_S *pConcatMsg, SMS_USERDATA_S *pUserData)
{
	if (pConcatMsg == NULL || pUserData == NULL) {
		MSG_DEBUG("In Parameter is NULL");
		return 0;
	}

	unsigned char currSegCnt = 0;

	bool bFind = false;

	for (unsigned int i = 0; i < concatList.size(); i++) {
		if (concatList[i].msgRef == pConcatMsg->msgRef) {
			if (concatList[i].data.count(pConcatMsg->seqNum) != 0) {
				MSG_DEBUG("The Sequence Number already exists [%d]", pConcatMsg->seqNum);
				return 0;
			}

			CONCAT_DATA_S concatData = {0};

			memcpy(concatData.data, pUserData->data, pUserData->length);
			concatData.length = pUserData->length;

			pair<unsigned char, CONCAT_DATA_S> newData(pConcatMsg->seqNum, concatData);
			concatList[i].data.insert(newData);

			MSG_DEBUG("MSG DATA : %s", pUserData->data);
			MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.data);

			concatList[i].segCnt++;
			concatList[i].totalSize += pUserData->length;

			currSegCnt = concatList[i].segCnt;

			bFind = true;

			break;
		}
	}

	/** New Concat Msg */
	if (bFind == false) {
		SMS_CONCAT_INFO_S tmpInfo;

		tmpInfo.msgRef = pConcatMsg->msgRef;
		tmpInfo.totalSeg = pConcatMsg->totalSeg;
		tmpInfo.segCnt = 1;

		memcpy(&(tmpInfo.timeStamp.time.absolute), &(pConcatMsg->timeStamp.time.absolute), sizeof(SMS_TIME_ABS_S));
		memcpy(&(tmpInfo.originAddress), &(pConcatMsg->originAddress), sizeof(SMS_ADDRESS_S));
		memcpy(&(tmpInfo.dcs), &(pConcatMsg->dcs), sizeof(SMS_DCS_S));

		tmpInfo.totalSize = pUserData->length;

		CONCAT_DATA_S concatData = {0};

		memcpy(concatData.data, pUserData->data, pUserData->length);
		concatData.length = pUserData->length;

		pair<unsigned char, CONCAT_DATA_S> newData(pConcatMsg->seqNum, concatData);
		tmpInfo.data.insert(newData);

		MSG_DEBUG("MSG DATA : %s", pUserData->data);
		MSG_DEBUG("PAIR DATA [%d] : %s", newData.first, newData.second.data);

		concatList.push_back(tmpInfo);

		currSegCnt = tmpInfo.segCnt;
	}

	return currSegCnt;
}


int SmsPluginConcatHandler::makeConcatUserData(unsigned short MsgRef, char **ppTotalData)
{
	concatDataMap::iterator it;

	int totalSize = 0, offset = 0;

	for (unsigned int i = 0; i < concatList.size(); i++) {
		if (concatList[i].msgRef == MsgRef) {
			totalSize = concatList[i].totalSize;

			if (totalSize <= 0) {
				MSG_DEBUG("Size Error : totalSize <= 0");
				return 0;
			}

			MSG_DEBUG("totalSize [%d]", totalSize);

			*ppTotalData = new char[totalSize];

			for (it = concatList[i].data.begin(); it != concatList[i].data.end(); it++) {
				memcpy(*ppTotalData+offset, it->second.data, it->second.length);
				offset += it->second.length;
			}
		}
	}

	return totalSize;
}


void SmsPluginConcatHandler::convertConcatToMsginfo(const SMS_DELIVER_S *pTpdu, const char *pUserData, int DataSize, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	/** Convert Type  values */
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;
	pMsgInfo->msgType.subType = MSG_NORMAL_SMS;

	/** set folder id */
	pMsgInfo->folderId = MSG_INBOX_ID;

	/** set storage id */
	pMsgInfo->storageId = MSG_STORAGE_PHONE;

	switch(pTpdu->dcs.msgClass)
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
			break;
	}

	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;


	time_t rawtime = time(NULL);

/*** Comment below lines to save local UTC time..... (it could be used later.)

	if (pTpdu->timeStamp.format == SMS_TIME_ABSOLUTE) {

		MSG_DEBUG("year : %d", pTpdu->timeStamp.time.absolute.year);
		MSG_DEBUG("month : %d", pTpdu->timeStamp.time.absolute.month);
		MSG_DEBUG("day : %d", pTpdu->timeStamp.time.absolute.day);
		MSG_DEBUG("hour : %d", pTpdu->timeStamp.time.absolute.hour);
		MSG_DEBUG("minute : %d", pTpdu->timeStamp.time.absolute.minute);
		MSG_DEBUG("second : %d", pTpdu->timeStamp.time.absolute.second);
		MSG_DEBUG("timezone : %d", pTpdu->timeStamp.time.absolute.timeZone);

		char displayTime[32];
		struct tm * timeTM;

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

***/

	pMsgInfo->displayTime = rawtime;

	/** Convert Address values */
	pMsgInfo->nAddressCnt = 1;
	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(pMsgInfo->addressList[0].addressVal, pTpdu->originAddress.address, MAX_ADDRESS_VAL_LEN);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	for (int i = 0; i < pTpdu->userData.headerCnt; i++) {
		/** Convert UDH values - Port Number */
		if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_8BIT) {
			pMsgInfo->msgPort.valid = true;
			pMsgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort8bit.destPort;
			pMsgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort8bit.originPort;
		} else if (pTpdu->userData.header[i].udhType == SMS_UDH_APP_PORT_16BIT) {
			pMsgInfo->msgPort.valid = true;
			pMsgInfo->msgPort.dstPort = pTpdu->userData.header[i].udh.appPort16bit.destPort;
			pMsgInfo->msgPort.srcPort = pTpdu->userData.header[i].udh.appPort16bit.originPort;
		}
	}

	//int bufSize = (MAX_MSG_DATA_LEN*MAX_SEGMENT_NUM) + 1;
	int bufSize = (DataSize*4) + 1; // For UTF8

	char tmpBuf[bufSize];
	memset(tmpBuf, 0x00, sizeof(tmpBuf));

	/** Convert Data values */
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		MSG_LANG_INFO_S langInfo = {0,};

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		pMsgInfo->encodeType = MSG_ENCODE_GSM7BIT;
		pMsgInfo->dataSize = textCvt.convertGSM7bitToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_8BIT) {
		pMsgInfo->encodeType = MSG_ENCODE_8BIT;
		memcpy(tmpBuf, pUserData, DataSize);
		pMsgInfo->dataSize = DataSize;
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		pMsgInfo->encodeType = MSG_ENCODE_UCS2;
		pMsgInfo->dataSize = textCvt.convertUCS2ToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize);
	}

	MSG_DEBUG("Data Size [%d]", pMsgInfo->dataSize);
	MSG_DEBUG("Data [%s]", tmpBuf);

#ifdef MSG_FW_FOR_DEBUG
printf("\n");

for (int i = 0; i < pMsgInfo->dataSize; i++)
{
	printf("[%02x]", tmpBuf[i]);
}

printf("\n");
#endif

	if (pMsgInfo->dataSize > MAX_MSG_TEXT_LEN) {
		pMsgInfo->bTextSms = false;

		/** Save Message Data into File */
		char fileName[MSG_FILENAME_LEN_MAX+1];
		memset(fileName, 0x00, sizeof(fileName));

		if (MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "########  MsgCreateFileName Fail !!! #######");

		MSG_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsgInfo->dataSize, fileName);
		if (MsgWriteIpcFile(fileName, tmpBuf, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "########  MsgWriteIpcFile Fail !!! #######");

		strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);
	} else {
		pMsgInfo->bTextSms = true;

		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		memcpy(pMsgInfo->msgText, tmpBuf, pMsgInfo->dataSize);
	}
}


#ifdef CONCAT_SIM_MSG_OPERATION
void SmsPluginConcatHandler::convertSimMsgToMsginfo(const SMS_CONCAT_MSG_S *pConcatMsg, const char *pUserData, int DataSize, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	// Convert Type  values
	pMsgInfo->msgType.mainType = MSG_SMS_TYPE;
	pMsgInfo->msgType.subType = MSG_CONCAT_SIM_SMS;

	// set folder id (temporary)
	pMsgInfo->folderId = MSG_INBOX_ID;

	pMsgInfo->storageId = MSG_STORAGE_SIM;

	switch (pConcatMsg->dcs.msgClass)
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

	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	time_t rawtime = time(NULL);

/*** Comment below lines to save local UTC time..... (it could be used later.)

	if (pTpdu->timeStamp.format == SMS_TIME_ABSOLUTE) {

		MSG_DEBUG("year : %d", pTpdu->timeStamp.time.absolute.year);
		MSG_DEBUG("month : %d", pTpdu->timeStamp.time.absolute.month);
		MSG_DEBUG("day : %d", pTpdu->timeStamp.time.absolute.day);
		MSG_DEBUG("hour : %d", pTpdu->timeStamp.time.absolute.hour);
		MSG_DEBUG("minute : %d", pTpdu->timeStamp.time.absolute.minute);
		MSG_DEBUG("second : %d", pTpdu->timeStamp.time.absolute.second);
		MSG_DEBUG("timezone : %d", pTpdu->timeStamp.time.absolute.timeZone);

		char displayTime[32];
		struct tm * timeTM;

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

***/

	pMsgInfo->displayTime = rawtime;

	// Convert Address values
	pMsgInfo->nAddressCnt = 1;
	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(pMsgInfo->addressList[0].addressVal, pConcatMsg->originAddress.address, MAX_ADDRESS_VAL_LEN);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;

	// Insert SMS_CONCAT_SIM_MSG_S into File
	SMS_CONCAT_SIM_MSG_S concatSimMsg = {0};

	for (unsigned int i = 0; i < simIdList.size(); i++)
	{
		if (simIdList[i].msgRef == pConcatMsg->msgRef)
		{
			MSG_DEBUG("Get SIM ID [%d] - List Index [%d]", simIdList[i].simId, concatSimMsg.simIdCnt);

			concatSimMsg.simIdList[concatSimMsg.simIdCnt] = simIdList[i].simId;
			concatSimMsg.simIdCnt++;
		}
	}

	int bufSize = (MAX_MSG_DATA_LEN*MAX_SEGMENT_NUM) + 1;

	char tmpBuf[bufSize];
	memset(tmpBuf, 0x00, sizeof(tmpBuf));

	// Convert Data values
	if (pConcatMsg->dcs.codingScheme == SMS_CHARSET_7BIT)
	{
		SMS_LANG_INFO_S langInfo = {0};

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		pMsgInfo->encodeType = MSG_ENCODE_GSM7BIT;
		pMsgInfo->dataSize = SmsPluginTextConvert::instance()->convertGSM7bitToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize, &langInfo);
	}
	else if (pConcatMsg->dcs.codingScheme == SMS_CHARSET_8BIT)
	{
		pMsgInfo->encodeType = MSG_ENCODE_8BIT;
		memcpy(tmpBuf, pUserData, DataSize);
		pMsgInfo->dataSize = DataSize;
	}
	else if (pConcatMsg->dcs.codingScheme == SMS_CHARSET_UCS2)
	{
		pMsgInfo->encodeType = MSG_ENCODE_UCS2;
		pMsgInfo->dataSize = SmsPluginTextConvert::instance()->convertUCS2ToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize);
	}

	MSG_DEBUG("Data Size [%d]", pMsgInfo->dataSize);

	pMsgInfo->bTextSms = false;

	if (pMsgInfo->dataSize > 0)
		memcpy(concatSimMsg.msgData, tmpBuf, pMsgInfo->dataSize);

	// Save Message Data into File
	char fileName[MAX_COMMON_INFO_SIZE+1];
	memset(fileName, 0x00, sizeof(fileName));

	if (MsgCreateFileName(fileName) == false)
		THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

	if (MsgWriteIpcFile(fileName, (char*)(&concatSimMsg), sizeof(SMS_CONCAT_SIM_MSG_S)) == false)
		THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

	memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
	strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);

	MSG_DEBUG("Save Message Data into file : size[%d] name[%s]", pMsgInfo->dataSize, fileName);
}
#endif


void SmsPluginConcatHandler::removeFromConcatList(unsigned short MsgRef)
{
	for (int index = concatList.size(); index >= 0 ; index--) {
		if (concatList[index].msgRef == MsgRef) {
			MSG_DEBUG("remove concatlist of the index [%d]", index);
			concatList.erase(concatList.begin()+index);
			break;
		}
	}
}

#ifdef CONCAT_SIM_MSG_OPERATION
void SmsPluginConcatHandler::addToSimIdList(unsigned short MsgRef, msg_sim_id_t SimMsgId)
{
	SMS_SIM_ID_S simIdStruct;

	simIdStruct.msgRef = MsgRef;
	simIdStruct.simId = SimMsgId;

	simIdList.push_back(simIdStruct);
}


void SmsPluginConcatHandler::removeFromSimIdList(unsigned short MsgRef)
{
	for (int index = simIdList.size()-1; index >= 0 ; index--)
	{
		if (simIdList[index].msgRef == MsgRef)
		{
			MSG_DEBUG("remove simIdList of the index [%d]", index);

			simIdList.erase(simIdList.begin()+index);
		}
	}
}
#endif
