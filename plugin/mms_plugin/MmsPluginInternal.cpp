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

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgSettingTypes.h"
#include "MsgMmsMessage.h"
#include "MsgGconfWrapper.h"
#include "MsgStorageHandler.h"
#include "MsgSerialize.h"
#include "MsgSpamFilter.h"
#include "MsgUtilMime.h"
#include "MsgUtilFunction.h"

#include "MmsPluginDebug.h"
#include "MmsPluginTypes.h"
#include "MmsPluginCodec.h"
#include "MmsPluginInternal.h"
#include "MmsPluginStorage.h"
#include "MmsPluginAppBase.h"

/*==================================================================================================
                                     IMPLEMENTATION OF MmsPluginInternal - Member Functions
==================================================================================================*/
MmsPluginInternal *MmsPluginInternal::pInstance = NULL;


MmsPluginInternal::MmsPluginInternal()
{
}

MmsPluginInternal::~MmsPluginInternal()
{
}

MmsPluginInternal *MmsPluginInternal::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginInternal();

	return pInstance;
}

void MmsPluginInternal::processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject)
{
	MSG_BEGIN();

	FILE *pFile = NULL;
	char fileName[MSG_FILENAME_LEN_MAX] = {0, };

	if (pMsgInfo->bTextSms == true) {
		char fullPath[MAX_FULL_PATH_SIZE+1] = {0, };

		if (MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

		MSG_SEC_DEBUG("File name = %s", fileName);

		if (MsgWriteIpcFile(fileName, pMsgInfo->msgText, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

		snprintf(fullPath, MAX_FULL_PATH_SIZE+1, "%s%s", MSG_IPC_DATA_PATH, fileName);

		memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
		memcpy(pMsgInfo->msgData, fullPath, strlen(fullPath));
		pMsgInfo->bTextSms = false;
	}

	MSG_SEC_DEBUG("MMS File Path = %s", pMsgInfo->msgData);

	MmsInitHeader();
	MmsRegisterDecodeBuffer();

	if ((pFile = MsgOpenFile(pMsgInfo->msgData, "rb+")) == NULL) {
		MSG_DEBUG("File Open Error: %s", pMsgInfo->msgData);
	} else {
		/* Decode Header */
		if (!MmsBinaryDecodeMsgHeader(pFile, pMsgInfo->dataSize))
			MSG_DEBUG("Decoding Header Failed \r\n");

		MsgCloseFile(pFile);

		if (remove(pMsgInfo->msgData) != 0)
			MSG_DEBUG("Fail remove");

		switch (mmsHeader.type) {
		case MMS_MSGTYPE_NOTIFICATION_IND:
			MSG_DEBUG("process noti.ind\n");
			/* For Set Value pMsgInfo */
			if (processNotiInd(pMsgInfo, pRequest) == false)
				*bReject = true;
			else
				*bReject = false;
			break;

		case MMS_MSGTYPE_DELIVERY_IND:
			MSG_DEBUG("process delivery.ind\n");
			/* For Set Value pMsgInfo */
			processDeliveryInd(pMsgInfo);
			break;

		case MMS_MSGTYPE_READORG_IND:
			MSG_DEBUG("process readorig.ind\n");
			processReadOrgInd(pMsgInfo);
			break;
		default:
			break;
		}
	}

	MSG_END();
}

bool MmsPluginInternal::processNotiInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest)
{
	MSG_DEBUG("MmsProcessNotiInd");

	MSG_MMS_HOME_RETRIEVE_TYPE_T retrieveType;
	bool bReportAllowed;

	MmsAttrib attrib;
	MsgDbHandler *dbHandle = getDbHandle();

	MmsInitMsgAttrib(&attrib);

	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS;
	pMsgInfo->priority = mmsHeader.priority;
	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);

	MSG_DEBUG("pMsgInfo->subject [%s]", pMsgInfo->subject);

#if 0	/* we do not need to add empty subject text any more. */
	if (strlen(pMsgInfo->subject) < 1)
		snprintf(pMsgInfo->subject, MAX_SUBJECT_LEN, "MMS Notification Message.");
#endif

	attrib.expiryTime = mmsHeader.expiryTime;

	if (mmsHeader.pFrom) {
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
		/* From */
		strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pFrom->szAddr, MAX_ADDRESS_VAL_LEN);
		if (MmsAddrUtilCheckEmailAddress(pMsgInfo->addressList[0].addressVal)) {
			pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_EMAIL;
		}
	}

	MMS_DATA_S *mms_data = MsgMmsCreate();
	if (mms_data == NULL)
		return false;

	mms_data->header = MsgMmsCreateHeader();

	if (mms_data->header == NULL) {
		MsgMmsRelease(&mms_data);
		return false;
	}

	MMS_HEADER_DATA_S *pHeader = mms_data->header;

	pHeader->messageType = mmsHeader.type;

	snprintf(pHeader->trID, sizeof(pHeader->trID), "%s", mmsHeader.szTrID);

	pHeader->mmsVersion = mmsHeader.version;

	/* From */
	if (mmsHeader.pFrom) {
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
		snprintf(pHeader->szFrom, sizeof(pHeader->szFrom), "%s", mmsHeader.pFrom->szAddr);
	}

	/* Subject */
	snprintf(pHeader->szSubject, sizeof(pHeader->szSubject), "%s", mmsHeader.szSubject);
	/* Delivery Report */
	pHeader->bDeliveryReport = (mmsHeader.deliveryReport != MMS_REPORT_YES);
	/* Message Class */
	pHeader->messageClass = mmsHeader.msgClass;

	/* Priority */
	pHeader->mmsPriority = mmsHeader.priority;

	/* Message Size : pMmsMsg->mmsAttrib.msgSize = mmsHeader.msgSize; */
	/* Expiry */
	pHeader->expiry.type = mmsHeader.expiryTime.type;
	pHeader->expiry.time = mmsHeader.expiryTime.time;

	time_t curTime = time(NULL);

	if (pHeader->expiry.type == MMS_TIMETYPE_RELATIVE) {
		pHeader->expiry.type = MMS_TIMETYPE_ABSOLUTE;
		pHeader->expiry.time += curTime;
	}

	/* Charge */
	/* contentclass */
	/* int contentClass; */ /*text | image-basic| image-rich | video-basic | video-rich | megapixel | content-basic | content-rich */
	strncpy(pHeader->contentLocation, mmsHeader.szContentLocation, MMS_LOCATION_LEN);

	pHeader->messageSize = mmsHeader.msgSize;

	MSG_DEBUG("Message size = [%d]", pHeader->messageSize);

	char *pSerializedMms = NULL;
	int serializeDataSize = 0;

	char pTempFileName[MSG_FILENAME_LEN_MAX+1] = {0};
	char pTempFilePath[MSG_FILEPATH_LEN_MAX+1] = {0};

	serializeDataSize = MsgSerializeMms(mms_data, &pSerializedMms);

	if (pSerializedMms) {
		if (MsgCreateFileName(pTempFileName) == true) {
			pMsgInfo->bTextSms = false;

			snprintf(pTempFilePath, sizeof(pTempFilePath), "%s%s", MSG_IPC_DATA_PATH, pTempFileName);

			MsgOpenCreateAndOverwriteFile(pTempFilePath, pSerializedMms, serializeDataSize);

			/* set file name */
			snprintf(pMsgInfo->msgData, sizeof(pMsgInfo->msgData), "%s", pTempFileName);
		}

		free(pSerializedMms);
	}

		MsgMmsRelease(&mms_data);

	/* Check contents-location is in noti.ind */
	if (mmsHeader.szContentLocation[0] == '\0') {
		THROW(MsgException::INCOMING_MSG_ERROR, "######## Contents-location is empty in MMS-Noti-Ind  #######");
		return false;
	}

	int roamState = 0;
	char pPduFilePath[MAX_FULL_PATH_SIZE] = {0};

	if (MsgSettingGetInt(VCONFKEY_TELEPHONY_SVC_ROAM, &roamState) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetInt() is failed");

	if (MsgSettingGetBool(MMS_SEND_REPORT_ALLOWED, &bReportAllowed) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	if (checkRejectNotiInd(roamState, bReportAllowed, pPduFilePath)) {
		MSG_DEBUG("MMS Message Rejected......");

		pMsgInfo->dataSize = strlen(pPduFilePath);
		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
		snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", pPduFilePath);
		pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;

		return false;
	}

	if (MsgCheckFilter(dbHandle, pMsgInfo)) {
		encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pPduFilePath);

		pMsgInfo->dataSize = strlen(pPduFilePath);

		pRequest->msgInfo.bTextSms = false;

		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", pPduFilePath);
		MSG_SEC_DEBUG("pRequest->msgInfo.msgData = %s", pRequest->msgInfo.msgData);
		pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;
		pRequest->msgInfo.folderId = MSG_SPAMBOX_ID;

		return true;
	}

	int tmpVal = 0;
	if (roamState == VCONFKEY_TELEPHONY_SVC_ROAM_OFF) {
		if (MsgSettingGetInt(MMS_RECV_HOME_NETWORK, &tmpVal) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)tmpVal;
		MSG_DEBUG("$$$$$$$$$$ MMS_RECV_HOME_NETWORK = %d $$$$$$$$$$$$$", retrieveType);
	} else {
		if (MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK, &tmpVal) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)tmpVal;
		MSG_DEBUG("$$$$$$$$$$ MMS_RECV_ABROAD_NETWORK = %d $$$$$$$$$$$$$", retrieveType);

		if (retrieveType == MSG_ABROAD_RESTRICTED) {
			MSG_DEBUG("MMS Receiving Setting Restricted was selected.");
			/* m-notify-resp-ind encoding process */
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pPduFilePath);

			pMsgInfo->dataSize = strlen(pPduFilePath);

			pRequest->msgInfo.bTextSms = false;

			memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

			snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", pPduFilePath);

			pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;

			return true;
		}
	}

	/* should send http 'GET' */
	if (retrieveType == MSG_HOME_AUTO_DOWNLOAD || retrieveType == MSG_ABROAD_AUTO_DOWNLOAD) {
		/* Check if current request sim index is different from default network SIM */
		/* Convert auto-retrieve to manual retrieve in case sim indexes are different */
		int default_sim = 0;
		if (MsgSettingGetInt(MSG_NETWORK_SIM, &default_sim) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}

		MSG_DEBUG("default_sim = %d, pMsgInfo->sim_idx = %d", default_sim, pMsgInfo->sim_idx);

		if (default_sim == (int)pMsgInfo->sim_idx) {
			MSG_DEBUG("=========== START AUTO RETRIEVE MODE ============");

			pMsgInfo->dataSize = strlen(mmsHeader.szContentLocation);

			pRequest->msgInfo.bTextSms = true;

			memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

			snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", mmsHeader.szContentLocation);

			pRequest->msgInfo.msgType.subType = MSG_GET_MMS;

			MSG_SEC_DEBUG("MSG SUBTYPE = %d msg data %s bTextsms %d", pRequest->msgInfo.msgType.subType, pRequest->msgInfo.msgData, pRequest->msgInfo.bTextSms);
		} else {
			/* should send m-notify-resp-ind */
			MSG_DEBUG("=========== START MANUAL RETRIEVE MODE ===========");
			/* m-notify-resp-ind encoding process */
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pPduFilePath);

			pMsgInfo->dataSize = strlen(pPduFilePath);

			pRequest->msgInfo.bTextSms = false;

			memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

			snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", pPduFilePath);
			MSG_SEC_DEBUG("pRequest->msgInfo.msgData = %s", pRequest->msgInfo.msgData);
			snprintf(pRequest->msgInfo.msgURL, sizeof(pRequest->msgInfo.msgURL), "%s", mmsHeader.szContentLocation);
			pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;
		}
	} else {
		/* should send m-notify-resp-ind */
		MSG_DEBUG("=========== START MANUAL RETRIEVE MODE ===========");
		/* m-notify-resp-ind encoding process */

		if (retrieveType == MSG_HOME_MANUAL || retrieveType == MSG_ABROAD_MANUAL) {
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pPduFilePath);
		}

		pMsgInfo->dataSize = strlen(pPduFilePath);

		pRequest->msgInfo.bTextSms = false;

		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		snprintf(pRequest->msgInfo.msgData, sizeof(pRequest->msgInfo.msgData), "%s", pPduFilePath);
		MSG_SEC_DEBUG("pRequest->msgInfo.msgData = %s", pRequest->msgInfo.msgData);
		snprintf(pRequest->msgInfo.msgURL, sizeof(pRequest->msgInfo.msgURL), "%s", mmsHeader.szContentLocation);
		pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;
	}

	return true;
}

void MmsPluginInternal::processDeliveryInd(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MmsMsgMultiStatus status;
	memset(&status, 0x00, sizeof(MmsMsgMultiStatus));

	status.msgStatus = mmsHeader.msgStatus;
	status.handledTime = mmsHeader.date;
	status.bDeliveryReportIsRead = false;
	status.bDeliveyrReportIsLast = true;

	MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);
	MSG_SEC_DEBUG("[INFO] [ADDR: %s, MMSID: %s]", mmsHeader.pTo->szAddr, mmsHeader.szMsgID);

	pMsgInfo->msgType.mainType	= MSG_MMS_TYPE;
	pMsgInfo->msgType.subType	= MSG_DELIVERYIND_MMS;
	pMsgInfo->bTextSms = true;
	pMsgInfo->dataSize = 0;
	memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);

	strncpy(pMsgInfo->msgData, getMmsDeliveryStatus(status.msgStatus), MAX_MSG_DATA_LEN);
	pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);
	MSG_DEBUG("Delivery Status = %s", pMsgInfo->msgData);

	strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);

	int tmpId = (msg_message_id_t)MmsPluginStorage::instance()->searchMsgId(mmsHeader.pTo->szAddr, mmsHeader.szMsgID);
	if (tmpId > 0) {
		MSG_DEBUG("Found MSG_ID = %d", tmpId);

		/* Insert to Delievery DB */
		MmsPluginStorage::instance()->insertDeliveryReport(tmpId, mmsHeader.pTo->szAddr, &status);

		pMsgInfo->msgId = (msg_message_id_t)tmpId;

	} else {
		MSG_DEBUG("Can not find MMS message in DB");
	}

	MSG_END();
}

void MmsPluginInternal::processReadOrgInd(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	if (pMsgInfo == NULL) {
		MSG_DEBUG("parameter err");
		return;
	}

	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_READORGIND_MMS;
	pMsgInfo->bTextSms = true;

	MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
	MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);

	memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
	pMsgInfo->dataSize = 0;

	strncpy(pMsgInfo->msgData, getMmsReadStatus(mmsHeader.readStatus), MAX_MSG_DATA_LEN);
	pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);

	MSG_SEC_DEBUG("read Status = %s", pMsgInfo->msgData);
	strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pFrom->szAddr, MAX_ADDRESS_VAL_LEN);

	int tmpId = MmsPluginStorage::instance()->searchMsgId(mmsHeader.pFrom->szAddr, mmsHeader.szMsgID);
	if (tmpId > 0) {
		pMsgInfo->msgId = (msg_message_id_t)tmpId;

		MmsMsgMultiStatus Status;
		memset(&Status, 0x00, sizeof(MmsMsgMultiStatus));
		Status.readTime = mmsHeader.date;
		Status.readStatus = mmsHeader.readStatus;

		MmsPluginStorage::instance()->insertReadReport(pMsgInfo->msgId, mmsHeader.pFrom->szAddr, &Status);

	} else {
		MSG_DEBUG("Can't not find Message!");
	}

	MSG_END();
}

void MmsPluginInternal::processSendConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest)
{
	MSG_BEGIN();

	MMS_RECV_DATA_S	recvData = {{0}, };

	pMsgInfo->msgId = pRequest->msgId;

	/* Set only changed members */
	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_SENDCONF_MMS;

	pMsgInfo->folderId = MSG_OUTBOX_ID;

	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);

	if (mmsHeader.responseStatus == MMS_RESPSTATUS_OK) {
		pMsgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
		pMsgInfo->dataSize = pRequest->postDataLen;
	} else {
		pMsgInfo->networkStatus = MSG_NETWORK_SEND_FAIL;

		char responseText[MMS_LOCALE_RESP_TEXT_LEN];

		memset(responseText, 0x00, MMS_LOCALE_RESP_TEXT_LEN);
		snprintf(responseText, MMS_LOCALE_RESP_TEXT_LEN, " %s [%d]", mmsHeader.szResponseText, mmsHeader.responseStatus);

		memset(pMsgInfo->msgText, 0x00, MAX_MSG_TEXT_LEN + 1);
		strncpy(pMsgInfo->msgText, responseText, MMS_LOCALE_RESP_TEXT_LEN);
	}

	MSG_ADDRESS_INFO_S addressinfo = {0, };
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));

	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, pMsgInfo->sim_idx);
	char *msisdn = NULL;
	if (MsgSettingGetString(keyName, &msisdn) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	MmsPluginStorage::instance()->getAddressInfo(pMsgInfo->msgId, &addressinfo);

	MSG_SEC_DEBUG("%d, MMS Send End %s->%s %s", pMsgInfo->msgId
													, (msisdn == NULL)?"ME":msisdn
													, addressinfo.addressVal
													, (pMsgInfo->networkStatus == MSG_NETWORK_SEND_SUCCESS)?"Success":"Fail");

	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}

	/* set message-id from mmsc */
	strncpy(recvData.szMsgID, mmsHeader.szMsgID, MMS_MSG_ID_LEN);
	strncpy(recvData.szTrID, mmsHeader.szTrID, MMS_TR_ID_LEN);

	memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
	memcpy(pMsgInfo->msgData, &recvData, sizeof(MMS_RECV_DATA_S));

	time_t curTime;
	curTime = time(NULL);

	pMsgInfo->displayTime = curTime;

	MmsMsg *pMsg = NULL;
	MmsPluginStorage::instance()->getMmsMessage(&pMsg);
	MmsInitHeader();
	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

	MSG_END();
}

#if 1
void MmsPluginInternal::processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	MMS_RECV_DATA_S recvData = {{0}, };

	MmsAttrib attrib;

	MmsInitMsgAttrib(&attrib);

	attrib.priority = mmsHeader.priority;
	attrib.bAskDeliveryReport = getMmsReport(mmsHeader.deliveryReport);
	attrib.bAskReadReply = getMmsReport(mmsHeader.readReply);

	/* Set only changed members */
	pMsgInfo->msgId = pRequest->msgId;
	MSG_DEBUG("@@@@@ msgId = %d @@@@@", pMsgInfo->msgId);
	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;

	if (pRequest->eMmsPduType == eMMS_RETRIEVE_AUTO_CONF)
		pMsgInfo->msgType.subType = MSG_RETRIEVE_AUTOCONF_MMS;
	else
		pMsgInfo->msgType.subType = MSG_RETRIEVE_MANUALCONF_MMS;

	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);

	strncpy(pRequest->transactionId, mmsHeader.szTrID, MMS_TR_ID_LEN);

	time_t curTime;
	curTime = time(NULL);

	pMsgInfo->displayTime = curTime;

	if (mmsHeader.retrieveStatus == MMS_RETRSTATUS_OK) {
		pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;
		pMsgInfo->folderId = MSG_INBOX_ID;
	} else {
		pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		pMsgInfo->folderId = MSG_INBOX_ID;
		/* If failed MMS Retrieve, then saved as MMS Noti Ind Message.
		 * It will changed in MsgHandleMmsConfIncomingMsg */
/*		pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS; */
	}

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));

	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, pMsgInfo->sim_idx);
	char *msisdn = NULL;
	if (MsgSettingGetString(keyName, &msisdn) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}
	char *normal_msisdn = NULL;
	if (msisdn)
		normal_msisdn = msg_normalize_number(msisdn);

	/* get setting value of group message */
	bool is_group_on = false;

	if (is_group_on) {
		int addr_cnt = 0;
		MsgHeaderAddress *iter = NULL;

		iter = mmsHeader.pFrom;
		while (iter) {
			addr_cnt++;
			iter = iter->pNext;
		}

		iter = mmsHeader.pTo;
		while (iter) {
			if (normal_msisdn == NULL || !g_str_has_suffix(iter->szAddr, normal_msisdn))
				addr_cnt++;
			iter = iter->pNext;
		}

		iter = mmsHeader.pCc;
		while (iter) {
			if (normal_msisdn == NULL || !g_str_has_suffix(iter->szAddr, normal_msisdn))
				addr_cnt++;
			iter = iter->pNext;
		}

		MSG_ADDRESS_INFO_S *tmp_addr_info = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)*addr_cnt];
		memset(tmp_addr_info, 0x00, sizeof(MSG_ADDRESS_INFO_S)*addr_cnt);
		if (mmsHeader.pFrom == NULL) {
			strncpy(tmp_addr_info[0].addressVal, pMsgInfo->addressList[0].addressVal, MAX_ADDRESS_VAL_LEN);
		}

		pMsgInfo->nAddressCnt = addr_cnt;
		pMsgInfo->addressList = tmp_addr_info;
	} else {
		pMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
		memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));
		pMsgInfo->nAddressCnt = 1;
	}

	if (mmsHeader.pFrom) {
		MSG_DEBUG("FROM : [%s]", mmsHeader.pFrom->szAddr);
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
		/* From */
		strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pFrom->szAddr, MAX_ADDRESS_VAL_LEN);
		if (MmsAddrUtilCheckEmailAddress(pMsgInfo->addressList[0].addressVal)) {
			pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_EMAIL;
		}
	}

	if (is_group_on) {
		int addr_idx = 0;
		if (mmsHeader.pTo) {
			MsgHeaderAddress *iter = mmsHeader.pTo;
			while (iter) {
				addr_idx++;
				MSG_DEBUG("TO : [%s]", mmsHeader.pTo->szAddr);
				MmsAddrUtilRemovePlmnString(iter->szAddr);
				/* To */
				if (normal_msisdn == NULL || !g_str_has_suffix(iter->szAddr, normal_msisdn)) {
					strncpy(pMsgInfo->addressList[addr_idx].addressVal, iter->szAddr, MAX_ADDRESS_VAL_LEN);
					pMsgInfo->addressList[addr_idx].recipientType = MSG_RECIPIENTS_TYPE_TO;
					if (MmsAddrUtilCheckEmailAddress(pMsgInfo->addressList[addr_idx].addressVal)) {
						pMsgInfo->addressList[addr_idx].addressType = MSG_ADDRESS_TYPE_EMAIL;
					}
				} else {
					addr_idx--;
				}

				iter = iter->pNext;
			}
		}

		if (mmsHeader.pCc) {
			MsgHeaderAddress *iter = mmsHeader.pCc;
			while (iter) {
				addr_idx++;
				MSG_DEBUG("CC : [%s]", mmsHeader.pCc->szAddr);
				MmsAddrUtilRemovePlmnString(iter->szAddr);
				/* Cc */
				if (normal_msisdn == NULL || !g_str_has_suffix(iter->szAddr, normal_msisdn)) {
					strncpy(pMsgInfo->addressList[addr_idx].addressVal, iter->szAddr, MAX_ADDRESS_VAL_LEN);
					pMsgInfo->addressList[addr_idx].recipientType = MSG_RECIPIENTS_TYPE_CC;
					if (MmsAddrUtilCheckEmailAddress(pMsgInfo->addressList[addr_idx].addressVal)) {
						pMsgInfo->addressList[addr_idx].addressType = MSG_ADDRESS_TYPE_EMAIL;
					}
				} else {
					addr_idx--;
				}

				iter = iter->pNext;
			}
		}
	}

	MSG_SEC_DEBUG("%d, MMS Receive %s End %s->%s %s", pMsgInfo->msgId
														, (pRequest->eMmsPduType == eMMS_RETRIEVE_AUTO_CONF)?"Auto":"Manual"
														, (mmsHeader.pFrom)?mmsHeader.pFrom->szAddr:"YOU"
														, (msisdn == NULL)?"ME":msisdn
														, (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS)?"Success":"Fail");

	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}

	pMsgInfo->dataSize = pRequest->getDataLen;

	/* set message-id & MMS TPDU file path */
	snprintf(recvData.szMsgID, sizeof(recvData.szMsgID), "%s", mmsHeader.szMsgID);

	if (pRetrievedFilePath)
		strncpy(recvData.retrievedFilePath, pRetrievedFilePath, sizeof(recvData.retrievedFilePath)-1);

	char *filename = NULL;

#ifdef FEATURE_JAVA_MMS
	if (mmsHeader.msgType.param.szApplicationID || mmsHeader.msgType.param.szReplyToApplicationID) {
		recvData.msgAppId.valid = true;
		if (mmsHeader.msgType.param.szApplicationID)
			strncpy(recvData.msgAppId.appId, mmsHeader.msgType.param.szApplicationID, sizeof(recvData.msgAppId.appId));
		if (mmsHeader.msgType.param.szReplyToApplicationID)
			strncpy(recvData.msgAppId.replyToAppId, mmsHeader.msgType.param.szReplyToApplicationID, sizeof(recvData.msgAppId.replyToAppId));

		char fullPath[MAX_FULL_PATH_SIZE+1] = {0, };

		filename = MsgGetFileName(pRetrievedFilePath);

		snprintf(fullPath, MAX_FULL_PATH_SIZE+1, "%s%s", MSG_IPC_DATA_PATH, filename);

		if (pRetrievedFilePath) {
			int ret  = rename(pRetrievedFilePath, fullPath);
			if (ret != 0) {
				MSG_DEBUG("File rename Error: %s", g_strerror(errno));
			}
		}
	}
#endif
	memcpy(pMsgInfo->msgData, &recvData, sizeof(MMS_RECV_DATA_S));

	MSG_SEC_DEBUG("@@@@@ MsgData = %s @@@@@", pMsgInfo->msgData);
	MSG_SEC_DEBUG("@@@@@ retrievedFilePath = %s @@@@@", recvData.retrievedFilePath);
	MSG_DEBUG("@@@@@ szMsgID = %s @@@@@", recvData.szMsgID);
	/* update delivery report, read reply */

	MmsPluginStorage *pStorage = MmsPluginStorage::instance();

	err = pStorage->updateMmsAttrib(pMsgInfo->msgId, &attrib, pMsgInfo->msgType.subType);

	MSG_DEBUG("Error value of updateMmsAttrib [%d]", err);

	/* make MmsData & insert multipart */
	MMSList *multipart_list = NULL;
	MMS_MULTIPART_DATA_S *pSmilMultipart = NULL;

	MmsMsg *pMmsMsg = NULL;
	MmsPluginStorage::instance()->getMmsMessage(&pMmsMsg);

	bool bFiltered;
	MmsPluginAppBase *appBase;

	MMS_DATA_S *pMmsData = MsgMmsCreate();
	if (pMmsData == NULL) {
		MSG_SEC_DEBUG("Fail to create mms");
		goto __CATCH;
	}

	pMmsData->header = MsgMmsCreateHeader();

	MmsConvertMmsData(pMmsMsg, pMmsData);
	/* CID 41996 : MmsConvertMmsData always returns true */
	/*
	if (MmsConvertMmsData(pMmsMsg, pMmsData) != true) {
		MSG_DEBUG("Fail to Compose MMS Message");
		goto __CATCH;
	} */

	bFiltered = checkFilterMmsBody(pMmsData);
	if (bFiltered == true) {
		pMsgInfo->folderId = MSG_SPAMBOX_ID;
	}

	pSmilMultipart = pMmsData->smil;
	if (pSmilMultipart) {
		MmsPluginStorage::instance()->insertMultipart(pMsgInfo->msgId, pSmilMultipart);
	}

	multipart_list = pMmsData->multipartlist;

	for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
		MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);
		MmsPluginStorage::instance()->insertMultipart(pMsgInfo->msgId, pMultipart);
	}

	/* make Preview info for APP */
	appBase = new MmsPluginAppBase(pMmsData);
	appBase->makePreviewInfo(pMsgInfo->msgId, false, pRetrievedFilePath);
	appBase->getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));
	delete appBase;

	MsgMmsRelease(&pMmsData);

	if (MsgGetFileSize(pRetrievedFilePath, (int *)&pMsgInfo->dataSize) == false) {
		MSG_SEC_DEBUG("Fail to get mms file size [%s]", pRetrievedFilePath);
		goto __CATCH;
	}

__CATCH: {
		MmsMsg *pMsg = NULL;
		pStorage->getMmsMessage(&pMsg);
		MmsInitHeader();
		MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);
		g_free(filename); filename = NULL;
	}
	MSG_END();
}
#else /* NEW process RetrieveConf */
void MmsPluginInternal::processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	pMsgInfo->msgId = pRequest->msgId;

	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	if (pRequest->eMmsPduType == eMMS_RETRIEVE_AUTO_CONF)
		pMsgInfo->msgType.subType = MSG_RETRIEVE_AUTOCONF_MMS;
	else
		pMsgInfo->msgType.subType = MSG_RETRIEVE_MANUALCONF_MMS;

	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);
	strncpy(pRequest->transactionId, mmsHeader.szTrID, MMS_TR_ID_LEN);

	time_t curTime;
	curTime = time(NULL);

	pMsgInfo->displayTime = curTime;

	if (mmsHeader.retrieveStatus == MMS_RETRSTATUS_OK) {
		pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;
		pMsgInfo->folderId = MSG_INBOX_ID;
	} else {
		pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		pMsgInfo->folderId = MSG_INBOX_ID;
		/* If failed MMS Retrieve, then saved as MMS Noti Ind Message.
		 * It will changed in MsgHandleMmsConfIncomingMsg */
/*		pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS; */
	}

	char *msisdn = NULL;
	if (MsgSettingGetString(MSG_SIM_MSISDN, &msisdn) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	if (mmsHeader.pFrom)
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);

	MSG_MMS_VLD_INFO("%d, MMS Receive %s End %s->%s %s", pMsgInfo->msgId
														, (pRequest->eMmsPduType == eMMS_RETRIEVE_AUTO_CONF)?"Auto":"Manual"
														, (mmsHeader.pFrom)?mmsHeader.pFrom->szAddr:"YOU"
														, (msisdn == NULL)?"ME":msisdn
														, (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS)?"Success":"Fail");

	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}

	pMsgInfo->dataSize = pRequest->getDataLen;

	MmsMsg *pMmsMsg = NULL;
	MmsPluginStorage::instance()->getMmsMessage(&pMmsMsg);

	MMS_DATA_S *pMmsData = MsgMmsCreate();
	pMmsData->header = MsgMmsCreateHeader();

	if (MmsConvertMmsData(pMmsMsg, pMmsData) != true) {
		MSG_DEBUG("Fail to Compose MMS Message");
		goto __CATCH;
	}

	char *pSerializedData = NULL;

	MsgSerializeMms(pMmsData, &pSerializedData);

	MsgMmsRelease(&pMmsData);


	char fileName[MSG_FILENAME_LEN_MAX] = {0};
	char fileFilePath[MSG_FILEPATH_LEN_MAX] = {0};

	if (MsgCreateFileName(fileName) == false)
		goto __CATCH;

	snprintf(fileFilePath, sizeof(fileFilePath), "%s%s", MSG_DATA_PATH, fileName);

	if (!MsgOpenCreateAndOverwriteFile(fileFilePath, (char *)pSerializedData, (int)strlen(pSerializedData))) {
		goto __CATCH;
	}

	snprintf(pMsgInfo->msgData, sizeof(pMsgInfo->msgData), "%s", fileFilePath);

	if (MsgGetFileSize(pRetrievedFilePath, (int *)&pMsgInfo->dataSize) == false) {
		MSG_SEC_DEBUG("Fail to get mms file size [%s]", pRetrievedFilePath);
		goto __CATCH;
	}

__CATCH: {
		MmsMsg *pMsg = NULL;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		MmsInitHeader();
		MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);
	}
	MSG_END();
}
#endif
void MmsPluginInternal::processForwardConf(MSG_MESSAGE_INFO_S *msgInfo, mmsTranQEntity *pRequest)
{
}

/* This function Send NotifyRespInd Msg
 *
 * @param	pTrID [in] Specifies Transaction ID
 * @param	iStatus [in] Specifies Msg Status
 * @param	iReportAllowed [in] Specifies whether to send deliveryReport to sender or not
 * @return	This function returns true on success, or false on failure.
 */
bool MmsPluginInternal::encodeNotifyRespInd(char *szTrID, msg_delivery_report_status_t iStatus, bool bReportAllowed, char *pSendFilePath)
{
	MSG_BEGIN();

	FILE *pFile = NULL;
	char pTempFileName[MSG_FILENAME_LEN_MAX+1] = {0};
	char pTempFilePath[MAX_FULL_PATH_SIZE] = {0};

	if (MsgCreateFileName(pTempFileName) == false)
		return false;

	snprintf(pTempFilePath, MAX_FULL_PATH_SIZE, "%s%s.noti.ind", MSG_DATA_PATH, pTempFileName);

	pFile = MsgOpenMMSFile(pTempFilePath);

	if (!pFile) {
		MSG_DEBUG("[ERROR] MsgOpenMMSFile fail");
		return false;
	}

	if (MmsEncodeNotiRespInd(pFile, szTrID, iStatus, bReportAllowed) == false) {
		MSG_DEBUG("MmsEncodeNotifyRespInd: MmsEncodeNotiRespInd fail");
		MsgCloseFile(pFile);
		return false;
	}

	MsgCloseFile(pFile);

	if (pSendFilePath) {
		/* CID 41993: replaced size 'MAX_MSG_DATA_LEN+1' with MAX_FULL_PATH_SIZE */
		snprintf(pSendFilePath, MAX_FULL_PATH_SIZE, "%s.mms", pTempFilePath);
	} else {
		MSG_DEBUG("[ERROR] pSendFilePath is NULL");
		return false;
	}

	MSG_END();

	return true;
}

/* This function Send AcknowledgeInd Msg
 *
 * @param	pTrID [in] Specifies Transaction ID
 * @param	iReportAllowed [in] Specifies whether to send deliveryReport to sender or not
 * @return	This function returns true on success, or false on failure.
 */
bool MmsPluginInternal::encodeAckInd(char *szTrID, bool bReportAllowed, char *pSendFilePath)
{
	MSG_BEGIN();
	FILE *pFile = NULL;
	char pTempFileName[MSG_FILENAME_LEN_MAX+1] = {0};
	char pTempFilePath[MAX_FULL_PATH_SIZE] = {0};

	if (MsgCreateFileName(pTempFileName) == false)
		return false;

	snprintf(pTempFilePath, MAX_FULL_PATH_SIZE, "%s%s.ack.ind", MSG_DATA_PATH, pTempFileName);

	pFile = MsgOpenMMSFile(pTempFilePath);
	if (!pFile) {
		MSG_ERR("MsgOpenMMSFile fail \n");
		return false;
	}

	if (MmsEncodeAckInd(pFile, szTrID, bReportAllowed) == false) {
		MSG_ERR("MmsEncodeAckInd: MmsEncodeAckInd fail \n");
		MsgCloseFile(pFile);
		return false;
	}

	MsgCloseFile(pFile);

	if (pSendFilePath) {
		snprintf(pSendFilePath, MAX_MSG_DATA_LEN+1, "%s.mms", pTempFilePath);
	} else {
		MSG_ERR("pSendFilePath is NULL");
		return false;
	}

	MSG_END();

	return true;
}

bool MmsPluginInternal::checkRejectNotiInd(int roamState, bool bReportAllowed, char *pSendFilePath)
{
	MSG_BEGIN();
	MSG_MMS_HOME_RETRIEVE_TYPE_T retrieveType;
	bool bRejectAnonymous;
	bool bRejectAdvertisement;

	if (MsgSettingGetBool(MMS_RECV_REJECT_UNKNOWN, &bRejectAnonymous) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	if (MsgSettingGetBool(MMS_RECV_REJECT_ADVERTISE, &bRejectAdvertisement) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	/* Anonymous Reject */
	if (bRejectAnonymous &&
		(mmsHeader.pFrom == NULL || mmsHeader.pFrom->szAddr[0] == '\0')) {
		MSG_DEBUG("Anonymous Reject... ");
		encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

		return true;
	}

	/* Advertisement Reject */
	if (bRejectAdvertisement && mmsHeader.msgClass == MMS_MSGCLASS_ADVERTISEMENT) {
		MSG_DEBUG("Advertisement Reject... ");
		encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

		return true;
	}

	/* Message Reject - Roaming Case */
	int tmpVal = 0;
	if (roamState == VCONFKEY_TELEPHONY_SVC_ROAM_ON) {
		if (MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK, &tmpVal) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)tmpVal;
		if (retrieveType == MSG_ABROAD_REJECT) {
			MSG_DEBUG("Abroad_Network : Notification Reject... ");
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

			return true;
		}
	} else {
		if (MsgSettingGetInt(MMS_RECV_HOME_NETWORK, &tmpVal) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetInt() is failed");
		}
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)tmpVal;
		if (retrieveType == MSG_HOME_REJECT) {
			MSG_DEBUG("Home_Network : Notification Reject... ");
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

			return true;
		}
	}

	/* Duplicate MMS notification */
	int msgId = 0;

	msgId = MmsPluginStorage::instance()->checkDuplicateNotification(mmsHeader.szTrID, mmsHeader.szContentLocation);

	MSG_DEBUG("Msg Id = %d", msgId);
	if (msgId > 0)
		return true;

	/* Not Rejected */
	MSG_END();
	return false;
}


bool MmsPluginInternal::checkFilterMmsBody(MMS_DATA_S *pMmsData)
{
	if (pMmsData == NULL)
		return false;

	bool bFiltered = false;
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;
	char filePath[MSG_FILEPATH_LEN_MAX + 1];
	gchar *fileContent = NULL;
	MsgDbHandler *dbHandle = getDbHandle();
	MimeType mimeType = MIME_UNKNOWN;

	MMS_MESSAGE_DATA_S *mmsMsg = NULL;
	unique_ptr<MMS_MESSAGE_DATA_S*, void(*)(MMS_MESSAGE_DATA_S**)> buf(&mmsMsg, unique_ptr_deleter);
	mmsMsg = (MMS_MESSAGE_DATA_S *)new char[sizeof(MMS_MESSAGE_DATA_S)];
	memset(mmsMsg, 0x00, sizeof(MMS_MESSAGE_DATA_S));

	MsgMmsConvertMmsDataToMmsMessageData(pMmsData, mmsMsg);

	/* Get the text data from the 1st slide. */
	if (mmsMsg->pageCnt <= 0) {
		MSG_WARN("pageCnt : %d", mmsMsg->pageCnt);
		MsgMmsReleaseMmsLists(mmsMsg);
		return false;
	}

	pPage = _MsgMmsGetPage(mmsMsg, 0);

	if (!pPage) {
		MSG_WARN("page is NULL");
		MsgMmsReleaseMmsLists(mmsMsg);
		return false;
	}

	for (int j = 0; j < pPage->mediaCnt; ++j) {
		pMedia = _MsgMmsGetMedia(pPage, j);

		if (pMedia && pMedia->mediatype == MMS_SMIL_MEDIA_TEXT) {
			MsgGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, pMedia->szFilePath, &mimeType, NULL);

			if (mimeType == MIME_TEXT_X_VCALENDAR || mimeType == MIME_TEXT_X_VCARD || mimeType == MIME_TEXT_X_VTODO || mimeType == MIME_TEXT_X_VNOTE) {
				MSG_SEC_DEBUG("Media Type is Text, but Vobject file [%s]", pMedia->szFilePath);
			} else {
				strncpy(filePath, pMedia->szFilePath, MSG_FILEPATH_LEN_MAX);

				g_file_get_contents((const gchar*)filePath, (gchar**)&fileContent, NULL, NULL);

				bFiltered = MsgCheckFilterByWord(dbHandle, (const char *)fileContent);

				g_free(fileContent);
				fileContent = NULL;

				if (bFiltered == true)
					break;
			}
		}
	}

	MsgMmsReleaseMmsLists(mmsMsg);

	return bFiltered;
}

bool MmsPluginInternal::getMmsReport(MmsReport mmsReport)
{
	bool result = false;

	if (mmsReport == MMS_REPORT_YES)
		result = true;
	else if (mmsReport == MMS_REPORT_NO)
		result = false;

	return result;
}

const char *MmsPluginInternal::getMmsDeliveryStatus(msg_delivery_report_status_t deliveryStatus)
{
	MSG_DEBUG("msgStatus= %d", deliveryStatus);

	switch (deliveryStatus) {
	case MSG_DELIVERY_REPORT_EXPIRED:
		return "expired.";
	case MSG_DELIVERY_REPORT_REJECTED:
		return "rejected.";
	case MSG_DELIVERY_REPORT_UNREACHABLE:
		return "unreachable.";
	case MSG_DELIVERY_REPORT_UNRECOGNISED:
		return "unrecognised.";
	case MSG_DELIVERY_REPORT_SUCCESS:
		return "delivered.";
	default:
		return "delivery failed.";
	}
}

const char *MmsPluginInternal::getMmsReadStatus(msg_read_report_status_t readStatus)
{
	switch (readStatus) {
	case MSG_READ_REPORT_IS_READ:
		return "message is read.";
	case MSG_READ_REPORT_IS_DELETED:
		return "message is deleted.";
	default:
		return "read status is none.";
	}
}

