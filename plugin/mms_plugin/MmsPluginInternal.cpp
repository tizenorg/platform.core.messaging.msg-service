/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgMmsMessage.h"
#include "MsgTransportTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgSoundPlayer.h"
#include "MsgStorageHandler.h"
#include "MmsPluginTypes.h"
#include "MmsPluginCodec.h"
#include "MmsPluginSetup.h"
#include "MmsPluginInternal.h"
#include "MmsPluginStorage.h"
#include "MmsPluginHttp.h"
#include "MmsPluginCodec.h"

#include "MsgNotificationWrapper.h"
#include "MmsPluginSmil.h"

MmsSetup gMmsSetup;

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
	MSG_DEBUG("processReceivedInd \r\n");

	FILE *pFile = NULL;
	char fileName[MSG_FILENAME_LEN_MAX] = {0,};

	if (pMsgInfo->bTextSms == true) {
		char fullPath[MAX_FULL_PATH_SIZE+1] = {0,};

		if(MsgCreateFileName(fileName) == false)
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

		MSG_DEBUG(" File name = %s", fileName);

		if(MsgWriteIpcFile(fileName, pMsgInfo->msgText, pMsgInfo->dataSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

		snprintf(fullPath, MAX_FULL_PATH_SIZE+1, MSG_IPC_DATA_PATH"%s", fileName);

		memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
		memcpy(pMsgInfo->msgData, fullPath, strlen(fullPath));
		pMsgInfo->bTextSms = false;
	}

	MSG_DEBUG("MMS File Path = %s", pMsgInfo->msgData);

	_MmsInitHeader();
	_MmsRegisterDecodeBuffer(gszMmsLoadBuf1,  gszMmsLoadBuf2, MSG_MMS_DECODE_BUFFER_MAX);

	if ((pFile = MsgOpenFile(pMsgInfo->msgData, "rb+")) == NULL) {
		MSG_DEBUG("File Open Error: %s", pMsgInfo->msgData);
	} else {
		//Decode Header
		if (!MmsBinaryDecodeMsgHeader(pFile, pMsgInfo->dataSize))
			MSG_DEBUG("Decoding Header Failed \r\n");

		MsgDeleteFile(pMsgInfo->msgData + strlen(MSG_IPC_DATA_PATH));

		switch (mmsHeader.type) {
		case MMS_MSGTYPE_NOTIFICATION_IND:
			MSG_DEBUG("MmsProcessNewMsgInd: process noti.ind\n");
			// For Set Value pMsgInfo
			if (processNotiInd(pMsgInfo, pRequest) == false)
				*bReject = true;
			else
				*bReject = false;
			break;

		case MMS_MSGTYPE_DELIVERY_IND:
			MSG_DEBUG("MmsProcessNewMsgInd: process delivery.ind\n");
			// For Set Value pMsgInfo
			processDeliveryInd(pMsgInfo);
			break;

		case MMS_MSGTYPE_READORG_IND:
			MSG_DEBUG("MmsProcessNewMsgInd: process readorig.ind\n");
			processReadOrgInd(pMsgInfo);
			break;
		default:
			break;
		}

		MsgCloseFile(pFile);
	}
	//Check Msg Type & Process(Save ...)
}

bool MmsPluginInternal::processNotiInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest)
{
	MSG_DEBUG("MmsProcessNotiInd");
	MSG_ERROR_T	err = MSG_SUCCESS;

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

	MmsMsg *pMsg = NULL;
	bool bFound = false;
	MmsMsgMultiStatus *pStatus = NULL;
	MmsMsgMultiStatus *pLastStatus = NULL;

	pMsg = (MmsMsg *)malloc(sizeof(MmsMsg));

	if (pMsg == NULL) {
		MSG_DEBUG("fail to allocation memory.");
		return;
	}

	MmsInitMsgAttrib(&pMsg->mmsAttrib);

	pMsgInfo->msgType.mainType 	= MSG_MMS_TYPE;
	pMsgInfo->msgType.subType 	= MSG_DELIVERYIND_MMS;
	pMsgInfo->bTextSms = true;

	MSG_DEBUG("#### mmsHeader.szMsgID = %s : when received delivery ind####", mmsHeader.szMsgID);

	int tmpId = (MSG_MESSAGE_ID_T)MmsSearchMsgId(mmsHeader.pTo->szAddr, mmsHeader.szMsgID);


	MSG_DEBUG("tmpId [%d]", tmpId);
	MSG_DEBUG("mmsHeader.pTo->szAddr [%s]", mmsHeader.pTo->szAddr);

	if (tmpId > 0) {
		pMsgInfo->msgId = (MSG_MESSAGE_ID_T)tmpId;

		pMsg->mmsAttrib.pMultiStatus = MmsGetMultiStatus(pMsgInfo->msgId);

		pStatus = pMsg->mmsAttrib.pMultiStatus;

		MSG_DEBUG("### pStatus->szTo = %s ###", pStatus->szTo);

		while (pStatus && !bFound) {
			MSG_DEBUG("### MmsAddrUtilCompareAddr ###");
			MSG_DEBUG("### mmsHeader.pTo->szAddr = %s ###", mmsHeader.pTo->szAddr);
			if (MmsAddrUtilCompareAddr( pStatus->szTo, mmsHeader.pTo->szAddr)) {
				bFound = true;
				break;
			}

			pStatus = pStatus->pNext;
		}

		if (bFound == false) {
			MSG_DEBUG("### bFound == false ###");
			/* Queue the delivery report  --------------------------- */

			pStatus = (MmsMsgMultiStatus *)malloc(sizeof(MmsMsgMultiStatus));
			memset(pStatus, 0, sizeof(MmsMsgMultiStatus));

			pStatus->readStatus = MMS_READSTATUS_NONE;
			memset(pStatus->szTo, 0, MSG_ADDR_LEN + 1);
			strncpy(pStatus->szTo, mmsHeader.pTo->szAddr, MSG_ADDR_LEN);

			if (pMsg->mmsAttrib.pMultiStatus == NULL) {
				/* first delivery report */
				pMsg->mmsAttrib.pMultiStatus = pStatus;
			} else {
				pLastStatus = pMsg->mmsAttrib.pMultiStatus;

				while (pLastStatus->pNext) {
					pLastStatus = pLastStatus->pNext;
				}

				pLastStatus->pNext = pStatus;
				pLastStatus = pStatus;
			}
		}

		pStatus->handledTime = mmsHeader.date;
		pStatus->msgStatus = mmsHeader.msgStatus;

		memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
		pMsgInfo->dataSize = 0;
		strncpy(pMsgInfo->msgData, getMmsDeliveryStatus(pStatus->msgStatus), MAX_MSG_DATA_LEN);
		pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);
		pMsgInfo->bTextSms = true;
		MSG_DEBUG("Delivery Status = %s", pMsgInfo->msgData);

		strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);

		pStatus->bDeliveryReportIsRead = false;

		_MmsDataUpdateLastStatus(pMsg);

		pStatus->bDeliveyrReportIsLast= true;

		MmsUpdateDeliveryReport(pMsgInfo->msgId, pStatus);

		MmsPluginStorage::instance()->addMmsNoti(pMsgInfo);
	} else {
		MSG_DEBUG("Can't not find Message!");
		memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
		pMsgInfo->dataSize = 0;
		strncpy(pMsgInfo->msgData, getMmsDeliveryStatus(mmsHeader.msgStatus), MAX_MSG_DATA_LEN);

		MSG_DEBUG("Delivery Status = %s", pMsgInfo->msgData);

		pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);

		MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);

		strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);
	}

	MsgFreeAttrib(&pMsg->mmsAttrib);

	free(pMsg);

	MSG_END();
}

void MmsPluginInternal::processReadOrgInd(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MmsMsg *pMsg = NULL;
	bool bFound = false;
	MmsMsgMultiStatus *pStatus = NULL;
	MmsMsgMultiStatus *pLastStatus = NULL;

	pMsg = (MmsMsg *)malloc(sizeof(MmsMsg));

	if (pMsg == NULL) {
		MSG_DEBUG("fail to allocation memory.");
		return;
	}

	MmsInitMsgAttrib(&pMsg->mmsAttrib);

	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_READORGIND_MMS;
	pMsgInfo->bTextSms = true;

	MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr);
	MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);

	if (mmsHeader.readStatus != MSG_READ_REPORT_IS_DELETED || (strcmp(mmsHeader.pFrom->szAddr, mmsHeader.pTo->szAddr))) {
		MSG_DEBUG("#### mmsHeader.szMsgID = %s : when received read orig ind####", mmsHeader.szMsgID);

		int tmpId = MmsSearchMsgId(mmsHeader.pFrom->szAddr, mmsHeader.szMsgID);

		if (tmpId > 0) {
			pMsgInfo->msgId = (MSG_MESSAGE_ID_T)tmpId;

			pMsg->mmsAttrib.pMultiStatus = MmsGetMultiStatus(pMsgInfo->msgId);

			pStatus = pMsg->mmsAttrib.pMultiStatus;

			MSG_DEBUG("### pStatus->szTo = %s ###", pStatus->szTo);

			while (pStatus && !bFound) {
				MSG_DEBUG("### MmsAddrUtilCompareAddr ###");
				MSG_DEBUG("### mmsHeader.pFrom->szAddr = %s ###", mmsHeader.pFrom->szAddr);
				if (MmsAddrUtilCompareAddr(pStatus->szTo, mmsHeader.pFrom->szAddr)) {
					bFound = true;
					break;
				}

				pStatus = pStatus->pNext;
			}

			if (bFound == false) {
				MSG_DEBUG("### bFound == false ###");
				/* Queue the delivery report  --------------------------- */

				pStatus = (MmsMsgMultiStatus *)malloc(sizeof(MmsMsgMultiStatus));
				memset(pStatus, 0, sizeof(MmsMsgMultiStatus));

				pStatus->msgStatus = MMS_MSGSTATUS_NONE;

				memset(pStatus->szTo, 0, MSG_ADDR_LEN + 1);
				strncpy(pStatus->szTo, mmsHeader.pFrom->szAddr, MSG_ADDR_LEN);

				if (pMsg->mmsAttrib.pMultiStatus == NULL) {
					/* first readOrg report */
					pMsg->mmsAttrib.pMultiStatus = pStatus;
				} else {
					pLastStatus = pMsg->mmsAttrib.pMultiStatus;

					while (pLastStatus->pNext) {
						pLastStatus = pLastStatus->pNext;
					}

					pLastStatus->pNext = pStatus;
					pLastStatus = pStatus;
				}
			}

			pStatus->readTime = mmsHeader.date;
			pStatus->readStatus = mmsHeader.readStatus;

			memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
			pMsgInfo->dataSize = 0;
			strncpy(pMsgInfo->msgData, getMmsReadStatus(pStatus->readStatus), MAX_MSG_DATA_LEN);
			pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);
			pMsgInfo->bTextSms = true;
			MSG_DEBUG("read Status = %s", pMsgInfo->msgData);

			strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);

			pStatus->bReadReplyIsRead = false;

			_MmsDataUpdateLastStatus(pMsg);

			pStatus->bReadReplyIsLast= true;

			MmsUpdateReadReport(pMsgInfo->msgId, pStatus);

			MmsPluginStorage::instance()->addMmsNoti(pMsgInfo);
		} else {
			MSG_DEBUG("Can't not find Message!");
			memset(pMsgInfo->msgData, 0x00, MAX_MSG_DATA_LEN + 1);
			pMsgInfo->dataSize = 0;
			strncpy(pMsgInfo->msgData, getMmsReadStatus(mmsHeader.readStatus), MAX_MSG_DATA_LEN);
			pMsgInfo->dataSize  = strlen(pMsgInfo->msgData);
			MSG_DEBUG("read Status = %s", pMsgInfo->msgData);

			MmsAddrUtilRemovePlmnString(mmsHeader.pTo->szAddr);

			strncpy(pMsgInfo->addressList[0].addressVal, mmsHeader.pTo->szAddr, MAX_ADDRESS_VAL_LEN);
		}
	}

	MsgFreeAttrib(&pMsg->mmsAttrib);

	free(pMsg);

	MSG_END();
}

void MmsPluginInternal::processSendConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	MMS_RECV_DATA_S	recvData = {{0}, };

	pMsgInfo->msgId = pRequest->msgId;

	//Set only changed members
	pMsgInfo->msgType.mainType = MSG_MMS_TYPE;
	pMsgInfo->msgType.subType = MSG_SENDCONF_MMS;

	pMsgInfo->folderId = MSG_OUTBOX_ID;

	strncpy(pMsgInfo->subject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);

	if (mmsHeader.responseStatus == MMS_RESPSTATUS_OK) {
		pMsgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
	} else {
		pMsgInfo->networkStatus = MSG_NETWORK_SEND_FAIL;

		char responseText[MMS_LOCALE_RESP_TEXT_LEN];

		memset(responseText, 0x00, MMS_LOCALE_RESP_TEXT_LEN);
		snprintf(responseText, MMS_LOCALE_RESP_TEXT_LEN, " %s [%d]", mmsHeader.szResponseText, mmsHeader.responseStatus);

		memset(pMsgInfo->msgText, 0x00, MAX_MSG_TEXT_LEN + 1);
		strncpy(pMsgInfo->msgText, responseText, MMS_LOCALE_RESP_TEXT_LEN);
	}

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
	_MmsInitHeader();
#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif
	_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);


	MSG_END();
}


void MmsPluginInternal::processRetrieveConf(MSG_MESSAGE_INFO_S *pMsgInfo, mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_BEGIN();

	int partCnt = 0;
	int attachCount = 0;
	MsgType partHeader;
	bool bMultipartRelated = false;

	MSG_ERROR_T err = MSG_SUCCESS;
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

		rename(pRetrievedFilePath, fullPath);

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

		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		strcpy(szFileName, pMsg->szFileName);

		err = pStorage->getMsgText(&msgData, pMsgInfo->msgText);
		err = pStorage->makeThumbnail(&msgData, pMsgInfo->thumbPath, szFileName);

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
				char szBuf[MSG_FILEPATH_LEN_MAX];

				strcpy((char *)szBuf, partHeader.param.szFileName);
				sprintf(partHeader.param.szFileName, MSG_DATA_PATH"%s", szBuf);
				if (!bMultipartRelated || MmsCheckAdditionalMedia(&msgData, &partHeader))
					attachCount++;
			}
		}
	}
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
	_MmsInitHeader();
#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif
	_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);

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
bool MmsPluginInternal::encodeNotifyRespInd(char *szTrID, MSG_DELIVERY_REPORT_STATUS_T iStatus, bool bReportAllowed, char *pSendFilePath)
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

	if (_MmsEncodeNotiRespInd(pFile, szTrID, iStatus, bReportAllowed) == false) {
		MSG_DEBUG("MmsEncodeNotifyRespInd: _MmsEncodeNotiRespInd fail");
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

	if (_MmsEncodeAckInd(pFile, szTrID, bReportAllowed) == false) {
		MSG_DEBUG("MmsEncodeAckInd: _MmsEncodeAckInd fail \n" );
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

const char *MmsPluginInternal::getMmsDeliveryStatus(MSG_DELIVERY_REPORT_STATUS_T deliveryStatus)
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

const char *MmsPluginInternal::getMmsReadStatus(MSG_READ_REPORT_STATUS_T readStatus)
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

