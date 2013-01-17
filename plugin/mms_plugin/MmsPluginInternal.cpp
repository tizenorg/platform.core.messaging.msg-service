/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgSettingTypes.h"
#include "MsgMmsMessage.h"
#include "MsgGconfWrapper.h"
#include "MsgStorageHandler.h"
#include "MmsPluginDebug.h"
#include "MmsPluginTypes.h"
#include "MmsPluginCodec.h"
#include "MmsPluginInternal.h"
#include "MmsPluginStorage.h"
#include "MmsPluginSmil.h"

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
	char fileName[MSG_FILENAME_LEN_MAX] = {0,};

	if (pMsgInfo->bTextSms == true) {
		char fullPath[MAX_FULL_PATH_SIZE+1] = {0,};

		if(MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

		MSG_DEBUG("File name = %s", fileName);

		if(MsgWriteIpcFile(fileName, pMsgInfo->msgText, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

		snprintf(fullPath, MAX_FULL_PATH_SIZE+1, MSG_IPC_DATA_PATH"%s", fileName);

		memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
		memcpy(pMsgInfo->msgData, fullPath, strlen(fullPath));
		pMsgInfo->bTextSms = false;
	}

	MSG_DEBUG("MMS File Path = %s", pMsgInfo->msgData);

	MmsInitHeader();
	MmsRegisterDecodeBuffer();

	if ((pFile = MsgOpenFile(pMsgInfo->msgData, "rb+")) == NULL) {
		MSG_DEBUG("File Open Error: %s", pMsgInfo->msgData);
	} else {
		//Decode Header
		if (!MmsBinaryDecodeMsgHeader(pFile, pMsgInfo->dataSize))
			MSG_DEBUG("Decoding Header Failed \r\n");

		MsgDeleteFile(pMsgInfo->msgData + strlen(MSG_IPC_DATA_PATH));

		switch (mmsHeader.type) {
		case MMS_MSGTYPE_NOTIFICATION_IND:
			MSG_DEBUG("process noti.ind\n");
			// For Set Value pMsgInfo
			if (processNotiInd(pMsgInfo, pRequest) == false)
				*bReject = true;
			else
				*bReject = false;
			break;

		case MMS_MSGTYPE_DELIVERY_IND:
			MSG_DEBUG("process delivery.ind\n");
			// For Set Value pMsgInfo
			processDeliveryInd(pMsgInfo);
			break;

		case MMS_MSGTYPE_READORG_IND:
			MSG_DEBUG("process readorig.ind\n");
			processReadOrgInd(pMsgInfo);
			break;
		default:
			break;
		}

		MsgCloseFile(pFile);
	}

	MSG_END();
}

bool MmsPluginInternal::processNotiInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest)
{
	MSG_DEBUG("MmsProcessNotiInd");
	msg_error_t	err = MSG_SUCCESS;

	MSG_MMS_HOME_RETRIEVE_TYPE_T retrieveType;
	bool bReportAllowed;

	MmsAttrib attrib;

	MmsInitMsgAttrib(&attrib);

	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS;
	pMsgInfo->priority = mmsHeader.priority;
	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);

	MSG_DEBUG("pMsgInfo->subject [%s]", pMsgInfo->subject);

	if (strlen(pMsgInfo->subject) < 1)
		snprintf(pMsgInfo->subject, MAX_SUBJECT_LEN, "MMS Notification Message.");

	attrib.expiryTime = mmsHeader.expiryTime;

	MmsPluginStorage *pStorage = MmsPluginStorage::instance();
	err = pStorage->updateMmsAttrib(pMsgInfo->msgId, &attrib, pMsgInfo->msgType.subType);

	if (mmsHeader.pFrom) {
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
		// From
		strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pFrom->szAddr, MAX_ADDRESS_VAL_LEN);
	}

	int roamState = 0;

	roamState = MsgSettingGetInt(VCONFKEY_TELEPHONY_SVC_ROAM);
	MsgSettingGetBool(MMS_SEND_REPORT_ALLOWED, &bReportAllowed);

	if (checkRejectNotiInd(roamState, bReportAllowed, pMsgInfo->msgData)) {
		MSG_DEBUG("MMS Message Rejected......");

		pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
		pMsgInfo->bTextSms = true;
		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;

		return false;
	}

	if (roamState == VCONFKEY_TELEPHONY_SVC_ROAM_OFF) {
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_HOME_NETWORK);
		MSG_DEBUG("$$$$$$$$$$ MMS_RECV_HOME_NETWORK = %d $$$$$$$$$$$$$", retrieveType);
	} else {
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK);
		MSG_DEBUG("$$$$$$$$$$ MMS_RECV_ABROAD_NETWORK = %d $$$$$$$$$$$$$", retrieveType);

		if (retrieveType == MSG_ABROAD_RESTRICTED) {
			MSG_DEBUG("MMS Receiving Setting Restricted was selected.");
			// m-notify-resp-ind encoding process
			memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);

			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pMsgInfo->msgData);

			pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
			pMsgInfo->bTextSms = true;
			memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

			pRequest->msgInfo.msgType.subType = MSG_NOTIFYRESPIND_MMS;

			return true;
		}
	}

	// should send http 'GET'
	if (retrieveType == MSG_HOME_AUTO_DOWNLOAD || retrieveType == MSG_ABROAD_AUTO_DOWNLOAD) {
		MSG_DEBUG("=========== START AUTO RETRIEVE MODE ============");
		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);

		memcpy(pMsgInfo->msgData, mmsHeader.szContentLocation, strlen(mmsHeader.szContentLocation)) ;

		pMsgInfo->dataSize = strlen(pMsgInfo->msgData);

		pMsgInfo->bTextSms = true;

		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		pRequest->msgInfo.msgType.subType = MSG_GET_MMS;

		MSG_DEBUG("MSG SUBTYPE = %d msg data %s bTextsms %d", pRequest->msgInfo.msgType.subType, pRequest->msgInfo.msgData, pRequest->msgInfo.bTextSms);
	} else {
	// should send m-notify-resp-ind
		MSG_DEBUG("=========== START MANUAL RETRIEVE MODE ===========");
		// m-notify-resp-ind encoding process
		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);

		if (retrieveType == MSG_HOME_MANUAL || retrieveType == MSG_ABROAD_MANUAL) {
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_DEFERRED, bReportAllowed, pMsgInfo->msgData);
		}

		pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
		pMsgInfo->bTextSms = true;
		memcpy(&pRequest->msgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

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
	status.bDeliveyrReportIsLast= true;

	MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);
	MSG_DEBUG("[INFO] [ADDR: %s, MMSID: %s]",mmsHeader.pTo->szAddr, mmsHeader.szMsgID);

	pMsgInfo->msgType.mainType	= MSG_MMS_TYPE;
	pMsgInfo->msgType.subType	= MSG_DELIVERYIND_MMS;
	pMsgInfo->bTextSms = true;
	pMsgInfo->dataSize = 0;
	memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);

	strncpy(pMsgInfo->msgData, getMmsDeliveryStatus(status.msgStatus), MAX_MSG_DATA_LEN);
	pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);
	MSG_DEBUG("Delivery Status = %s", pMsgInfo->msgData);

	strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);

	int tmpId = (msg_message_id_t)MmsSearchMsgId(mmsHeader.pTo->szAddr, mmsHeader.szMsgID);
	if (tmpId > 0) {
		MSG_DEBUG("Found MSG_ID = %d", tmpId);

		//Insert to Delievery DB
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

	MSG_DEBUG("read Status = %s", pMsgInfo->msgData);
	strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pFrom->szAddr, MAX_ADDRESS_VAL_LEN);

	int tmpId = MmsSearchMsgId(mmsHeader.pFrom->szAddr, mmsHeader.szMsgID);
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

	//Set only changed members
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

	MSG_ADDRESS_INFO_S addressinfo = {0,};
	char *msisdn = NULL;
	msisdn = MsgSettingGetString(MSG_SIM_MSISDN);

	MmsPluginStorage::instance()->getAddressInfo(pMsgInfo->msgId, &addressinfo);

	MSG_MMS_VLD_INFO("%d, MMS Send End %s->%s %s", pMsgInfo->msgId
													, (msisdn == NULL)?"ME":msisdn
													, addressinfo.addressVal
													, (pMsgInfo->networkStatus == MSG_NETWORK_SEND_SUCCESS)?"Success":"Fail");

	// set message-id from mmsc
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
#ifdef __SUPPORT_DRM__
	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
#endif
	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);


	MSG_END();
}


void MmsPluginInternal::processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_BEGIN();

	int partCnt = 0;
	int attachCount = 0;
	MsgType partHeader;
	bool bMultipartRelated = false;

	msg_error_t err = MSG_SUCCESS;
	MMS_RECV_DATA_S recvData = {{0}, };

	MmsAttrib attrib;

	MmsInitMsgAttrib(&attrib);

	attrib.priority = mmsHeader.priority;
	attrib.bAskDeliveryReport = getMmsReport(mmsHeader.deliveryReport);
	attrib.bAskReadReply = getMmsReport(mmsHeader.readReply);

	//Set only changed members
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
		// If failed MMS Retrieve, then saved as MMS Noti Ind Message.
		pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS;
	}

	char *msisdn = NULL;
	msisdn = MsgSettingGetString(MSG_SIM_MSISDN);

	if (mmsHeader.pFrom)
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);

	MSG_MMS_VLD_INFO("%d, MMS Receive %s End %s->%s %s", pMsgInfo->msgId
														, (pRequest->eMmsPduType == eMMS_RETRIEVE_AUTO_CONF)?"Auto":"Manual"
														, (mmsHeader.pFrom)?mmsHeader.pFrom->szAddr:"YOU"
														, (msisdn == NULL)?"ME":msisdn
														, (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS)?"Success":"Fail");
	pMsgInfo->dataSize = pRequest->getDataLen;

	// set message-id & MMS TPDU file path
	strcpy(recvData.szMsgID, mmsHeader.szMsgID);
	if (pRetrievedFilePath)
		strncpy(recvData.retrievedFilePath, pRetrievedFilePath, sizeof(recvData.retrievedFilePath));

#ifdef FEATURE_JAVA_MMS
	if (mmsHeader.msgType.param.szApplicationID || mmsHeader.msgType.param.szReplyToApplicationID) {
		recvData.msgAppId.valid = true;
		if (mmsHeader.msgType.param.szApplicationID)
			strncpy(recvData.msgAppId.appId, mmsHeader.msgType.param.szApplicationID, sizeof(recvData.msgAppId.appId));
		if (mmsHeader.msgType.param.szReplyToApplicationID)
			strncpy(recvData.msgAppId.replyToAppId, mmsHeader.msgType.param.szReplyToApplicationID, sizeof(recvData.msgAppId.replyToAppId));

		char fullPath[MAX_FULL_PATH_SIZE+1] = {0, };

		char *filename = NULL;
		filename = strrchr(pRetrievedFilePath, '/');

		snprintf(fullPath, MAX_FULL_PATH_SIZE+1, "%s%s", MSG_IPC_DATA_PATH, filename + 1);

		int ret  = rename(pRetrievedFilePath, fullPath);
		if (ret != 0) {
			MSG_DEBUG("File rename Error: %s", strerror(errno));
		}

		if (chmod(fullPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) != 0) {
			MSG_DEBUG("File Write Error: %s", strerror(errno));
		}

		if (chown(fullPath, 0, 6502 ) != 0) {
			MSG_DEBUG("File Write Error: %s", strerror(errno));
		}
	}
#endif
	memcpy(pMsgInfo->msgData, &recvData, sizeof(MMS_RECV_DATA_S));

	MSG_DEBUG("@@@@@ MsgData = %s @@@@@", pMsgInfo->msgData);
	MSG_DEBUG("@@@@@ retrievedFilePath = %s @@@@@", recvData.retrievedFilePath);
	MSG_DEBUG("@@@@@ szMsgID = %s @@@@@", recvData.szMsgID);
	//update delivery report, read reply

	MmsPluginStorage *pStorage = MmsPluginStorage::instance();

	MMS_MESSAGE_DATA_S msgData;
	memset(&msgData, 0, sizeof(MMS_MESSAGE_DATA_S));

	// Conversation is supported only Multipart Related message, Presentation info should be provided
	if (mmsHeader.msgType.type == MIME_MULTIPART_RELATED || mmsHeader.msgType.type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		char *pSmilDoc = NULL;
		MmsMsg *pMsg = NULL;
		char szFileName[MSG_FILENAME_LEN_MAX] = {0, };

		msgData.regionCnt = 0;
		msgData.pageCnt = 0;
		msgData.attachCnt = 0;
		msgData.transitionCnt = 0;
		msgData.metaCnt = 0;
		memset(msgData.szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

		pSmilDoc = MmsSmilGetPresentationData(pMsgInfo->msgId);
		MmsSmilParseSmilDoc(&msgData, pSmilDoc);
		MmsRemovePims(&msgData);

		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		strcpy(szFileName, pMsg->szFileName);

		err = pStorage->getMsgText(&msgData, pMsgInfo->msgText);
		bMultipartRelated = true;
	} else {
		MSG_DEBUG("Multipart mixed message doesn't support mms conversation");
	}

	err = pStorage->updateMmsAttrib(pMsgInfo->msgId, &attrib, pMsgInfo->msgType.subType);

	partCnt = MmsGetMediaPartCount(pMsgInfo->msgId);
	MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt );

	if (partCnt < 0) {
		MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt );
	} else {
		for (int i = 0; i < partCnt; ++i) {
			if (!MmsGetMediaPartHeader(i, &partHeader)) {
				MSG_DEBUG("MmsUiGetMediaAttachInfo: MmsGetMediaPartHeader failed\n" );
				break;
			}

			if (partHeader.contentSize > 0) {
				char szBuf[MSG_FILEPATH_LEN_MAX + 1];

				strcpy((char *)szBuf, partHeader.param.szFileName);
				sprintf(partHeader.param.szFileName, MSG_DATA_PATH"%s", szBuf);
				if (!bMultipartRelated || MmsCheckAdditionalMedia(&msgData, &partHeader)) {
					MMS_ATTACH_S *attachment = NULL;
					int tempType;

					attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

					MsgGetTypeByFileName(&tempType, partHeader.param.szFileName);
					attachment->mediatype = (MimeType)tempType;

					strcpy(attachment->szFilePath, partHeader.param.szFileName);

					strncpy(attachment->szFileName, partHeader.param.szName, MSG_FILENAME_LEN_MAX - 1);

					attachment->fileSize = partHeader.contentSize;

					_MsgMmsAddAttachment(&msgData, attachment);
					attachCount++;

				}

			}
		}
	}

	MmsMakePreviewInfo(pMsgInfo->msgId, &msgData);
	MSG_DEBUG("attachCount [%d]", attachCount);
	err = pStorage->updateMmsAttachCount(pMsgInfo->msgId, attachCount);

	if (bMultipartRelated) {
		_MsgMmsReleasePageList(&msgData);
		_MsgMmsReleaseRegionList(&msgData);
		_MsgMmsReleaseAttachList(&msgData);
		_MsgMmsReleaseTransitionList(&msgData);
		_MsgMmsReleaseMetaList(&msgData);
	}

	MmsMsg *pMsg = NULL;
	pStorage->getMmsMessage(&pMsg);
	MmsInitHeader();
#ifdef __SUPPORT_DRM__
	MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
#endif
	MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

	MSG_END();
}

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

	snprintf(pTempFilePath, MAX_FULL_PATH_SIZE, MSG_DATA_PATH"%s.noti.ind", pTempFileName);

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
		snprintf(pSendFilePath, MAX_MSG_DATA_LEN+1, "%s.mms", pTempFilePath);
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

	snprintf(pTempFilePath, MAX_FULL_PATH_SIZE, MSG_DATA_PATH"%s.ack.ind", pTempFileName);

	pFile = MsgOpenMMSFile(pTempFilePath);
	if (!pFile) {
		MSG_DEBUG("[ERROR] MsgOpenMMSFile fail \n" );
		return false;
	}

	if (MmsEncodeAckInd(pFile, szTrID, bReportAllowed) == false) {
		MSG_DEBUG("MmsEncodeAckInd: MmsEncodeAckInd fail \n" );
		MsgCloseFile(pFile);
		return false;
	}

	MsgCloseFile(pFile);

	if (pSendFilePath) {
		snprintf(pSendFilePath, MAX_MSG_DATA_LEN+1, "%s.mms", pTempFilePath);
	} else {
		MSG_DEBUG("[ERROR] pSendFilePath is NULL");
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

	MsgSettingGetBool(MMS_RECV_REJECT_UNKNOWN, &bRejectAnonymous);
	MsgSettingGetBool(MMS_RECV_REJECT_ADVERTISE, &bRejectAdvertisement);

	// Anonymous Reject
	if (bRejectAnonymous &&
		(mmsHeader.pFrom == NULL || mmsHeader.pFrom->szAddr[0] == '\0')) {
		MSG_DEBUG("Anonymous Reject... ");
		encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

		return true;
	}

	// Advertisement Reject
	if (bRejectAdvertisement && mmsHeader.msgClass == MMS_MSGCLASS_ADVERTISEMENT) {
		MSG_DEBUG("Advertisement Reject... ");
		encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

		return true;
	}

	// Message Reject - Roaming Case
	if (roamState == VCONFKEY_TELEPHONY_SVC_ROAM_ON) {
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_ABROAD_NETWORK);
		if (retrieveType == MSG_ABROAD_REJECT) {
			MSG_DEBUG("Abroad_Network : Notification Reject... ");
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

			return true;
		}
	} else {
		retrieveType = (MSG_MMS_HOME_RETRIEVE_TYPE_T)MsgSettingGetInt(MMS_RECV_HOME_NETWORK);
		if (retrieveType == MSG_HOME_REJECT) {
			MSG_DEBUG("Home_Network : Notification Reject... ");
			encodeNotifyRespInd(mmsHeader.szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pSendFilePath);

			return true;
		}
	}

	// Not Rejected
	MSG_END();
	return false;

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

