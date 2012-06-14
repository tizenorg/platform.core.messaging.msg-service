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
#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "SmsPluginTextConvert.h"
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

	MSG_ERROR_T err = MSG_SUCCESS;
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

			if (msg.totalSeg > MAX_SEGMENT_NUM) {
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}

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

			if (msg.totalSeg > MAX_SEGMENT_NUM) {
				MSG_DEBUG("Total Segment Count is over Maximum [%d]", msg.totalSeg);
				return;
			}

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

	/** set folder id (temporary) */
	pMsgInfo->folderId = MSG_INBOX_ID;

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
	}

	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;


	time_t rawtime = time(NULL);

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

	int bufSize = (MAX_MSG_DATA_LEN*MAX_SEGMENT_NUM) + 1;

	char tmpBuf[bufSize];
	memset(tmpBuf, 0x00, sizeof(tmpBuf));

	/** Convert Data values */
	if (pTpdu->dcs.codingScheme == SMS_CHARSET_7BIT) {
		SMS_LANG_INFO_S langInfo = {0};

		langInfo.bSingleShift = false;
		langInfo.bLockingShift = false;

		pMsgInfo->encodeType = MSG_ENCODE_GSM7BIT;
		pMsgInfo->dataSize = SmsPluginTextConvert::instance()->convertGSM7bitToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize, &langInfo);
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_8BIT) {
		pMsgInfo->encodeType = MSG_ENCODE_8BIT;
		memcpy(tmpBuf, pUserData, DataSize);
		pMsgInfo->dataSize = DataSize;
	} else if (pTpdu->dcs.codingScheme == SMS_CHARSET_UCS2) {
		pMsgInfo->encodeType = MSG_ENCODE_UCS2;
		pMsgInfo->dataSize = SmsPluginTextConvert::instance()->convertUCS2ToUTF8((unsigned char*)tmpBuf, bufSize, (unsigned char*)pUserData, DataSize);
	}

	MSG_DEBUG("Data Size [%d]", pMsgInfo->dataSize);
	MSG_DEBUG("Data [%s]", tmpBuf);

	if (pMsgInfo->dataSize > MAX_MSG_TEXT_LEN) {
		pMsgInfo->bTextSms = false;

		/** Save Message Data into File */
		char fileName[MAX_COMMON_INFO_SIZE+1];
		memset(fileName, 0x00, sizeof(fileName));

		MsgCreateFileName(fileName);

		MSG_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsgInfo->dataSize, fileName);
		MsgWriteIpcFile(fileName, tmpBuf, pMsgInfo->dataSize);

		strncpy(pMsgInfo->msgData, fileName, MAX_MSG_DATA_LEN);
	} else {
		pMsgInfo->bTextSms = true;

		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		memcpy(pMsgInfo->msgText, tmpBuf, pMsgInfo->dataSize);
	}
}


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
