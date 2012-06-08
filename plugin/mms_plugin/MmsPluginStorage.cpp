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

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include <mm_file.h>
#include <mm_util_jpeg.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgMmsMessage.h"
#include "MsgNotificationWrapper.h"
#include "MsgUtilStorage.h"

#include "MmsPluginStorage.h"
#include "MmsPluginSetup.h"
#include "MmsPluginStorage.h"
#include "MmsPluginCodec.h"
#include "MmsPluginSmil.h"
#include "MmsPluginDrm.h"

#include "media-thumbnail.h"
#include <mm_util_imgp.h>
#include <mm_util_jpeg.h>

#include "MsgHelper.h"


static void __MmsReleaseMmsLists(MMS_MESSAGE_DATA_S *mms_data)
{
	_MsgMmsReleasePageList(mms_data);
	_MsgMmsReleaseRegionList(mms_data);
	_MsgMmsReleaseAttachList(mms_data);
	_MsgMmsReleaseTransitionList(mms_data);
	_MsgMmsReleaseMetaList(mms_data);
}

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginStorage - Member Functions
==================================================================================================*/
MmsPluginStorage *MmsPluginStorage::pInstance = NULL;


MmsPluginStorage::MmsPluginStorage()
{
	memset(&mmsMsg, 0, sizeof(MmsMsg));
}


MmsPluginStorage::~MmsPluginStorage()
{
	if (dbHandle.disconnect() != MSG_SUCCESS) {
		MSG_DEBUG("DB Disconnect Fail");
	}
}


MmsPluginStorage *MmsPluginStorage::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginStorage();

	return pInstance;
}


void MmsPluginStorage::getMmsMessage(MmsMsg **pMmsMsg)
{
	*pMmsMsg = &mmsMsg;
}


void MmsPluginStorage::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	MSG_BEGIN();

	MSG_ERROR_T	err;

	MmsMsg mmsMsg;

	bzero(&mmsMsg, sizeof(mmsMsg));

	mode_t file_mode = (S_IRUSR | S_IWUSR);

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS) {

		char szTemp[MAX_MSG_DATA_LEN + 1];

		MMS_MESSAGE_DATA_S mmsMsgData;

		if (MmsComposeMessage(&mmsMsg, pMsgInfo, pSendOptInfo, &mmsMsgData, pFileData) != true) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);

			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Message Compose Error");
		}

		if (pMsgInfo->msgId == pMsgInfo->referenceId) {
			char fileName[MSG_FILENAME_LEN_MAX+1] = {0,};

			FILE *pFile = NULL;

			strcpy(szTemp,pMsgInfo->msgData);

			snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, MSG_DATA_PATH"%d.mms", pMsgInfo->msgId);

			if (addMmsMsgToDB(&mmsMsg, pMsgInfo, _MsgMmsGetAttachCount(&mmsMsgData)) != MSG_SUCCESS) {
				_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				__MmsReleaseMmsLists(&mmsMsgData);

				THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
			}

			strcpy((char *)pMsgInfo->msgData,szTemp);

			snprintf(fileName, MSG_FILENAME_LEN_MAX+1, MSG_DATA_PATH"%d", mmsMsg.msgID);

			pFile = MsgOpenMMSFile(fileName);
			if (!pFile) {
				_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				__MmsReleaseMmsLists(&mmsMsgData);
				THROW(MsgException::MMS_PLG_ERROR, "MMS File open Error");
			}

			if (fchmod(fileno(pFile), file_mode) < 0) {
				_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				__MmsReleaseMmsLists(&mmsMsgData);
				MsgCloseFile(pFile);

				THROW(MsgException::MMS_PLG_ERROR, "chmod() error: %s", strerror(errno));
			}

			if (_MmsEncodeSendReq(pFile, &mmsMsg) != true) {
				_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				__MmsReleaseMmsLists(&mmsMsgData);
				MsgCloseFile(pFile);

				THROW(MsgException::MMS_PLG_ERROR, "MMS Message Encode Send Req Error");
			}

			MsgFsync(pFile);	//file is written to device immediately, it prevents missing file data from unexpected power off
			MsgCloseFile(pFile);

			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);
		} else {
			char szTemp[MAX_MSG_DATA_LEN + 1];

			strcpy(szTemp,pMsgInfo->msgData);

			snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, MSG_DATA_PATH"%d.mms", pMsgInfo->referenceId);

			if (addMmsMsgToDB(&mmsMsg, pMsgInfo, _MsgMmsGetAttachCount(&mmsMsgData)) != MSG_SUCCESS) {
				_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				__MmsReleaseMmsLists(&mmsMsgData);

				THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
			}

			strcpy((char *)pMsgInfo->msgData,szTemp);

			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);

			__MmsReleaseMmsLists(&mmsMsgData);
		}
	} else if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		MSG_DEBUG("######## MmsPlgAddMessage -> MSG_NOTIFICATIONIND_MMS ###########");

		MmsComposeNotiMessage(&mmsMsg, pMsgInfo->msgId);

		//Need to store mms specific data (contents location, TrID, ExpiryTime, Delivery Report, message ID)
		if (addMmsMsgToDB(&mmsMsg, pMsgInfo) != MSG_SUCCESS) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
		}
	} else if (pMsgInfo->msgType.subType == MSG_SENDCONF_MMS || pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS) {
		MmsMsg *pMsg = NULL;
		char szTemp[MAX_MSG_DATA_LEN + 1]= {0, };

		if (!_MmsReadMsgBody(pMsgInfo->msgId, true, true, pFileData))
			THROW(MsgException::MMS_PLG_ERROR, "_MmsReadMsgBody Error");

		MmsPluginStorage::instance()->getMmsMessage(&pMsg);

		if (pMsgInfo->msgType.subType == MSG_SENDCONF_MMS)
			pMsgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
		else
			pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;

		strcpy(szTemp,pMsgInfo->msgData);
		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);
		strncpy(pMsgInfo->msgData, pFileData, MAX_MSG_DATA_LEN);

		MmsPluginStorage *pStorage = MmsPluginStorage::instance();

		MMS_MESSAGE_DATA_S mmsMsgData;
		if (mmsHeader.msgType.type == MIME_MULTIPART_RELATED || mmsHeader.msgType.type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
			char *pSmilDoc;
			MmsMsg *pMsg = NULL;
			char szFileName[MSG_FILENAME_LEN_MAX] = {0, };

			mmsMsgData.regionCnt = 0;
			mmsMsgData.pageCnt = 0;
			mmsMsgData.attachCnt = 0;
			mmsMsgData.transitionCnt = 0;
			mmsMsgData.metaCnt = 0;
			memset(mmsMsgData.szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

			pSmilDoc = MmsSmilGetPresentationData(pMsgInfo->msgId);
			MmsSmilParseSmilDoc(&mmsMsgData, pSmilDoc);
			MmsPluginStorage::instance()->getMmsMessage(&pMsg);
			strcpy(szFileName, pMsg->szFileName);

			err = pStorage->getMsgText(&mmsMsgData, pMsgInfo->msgText);
			err = pStorage->makeThumbnail(&mmsMsgData, pMsgInfo->thumbPath, szFileName);

			__MmsReleaseMmsLists(&mmsMsgData);
		}

		if (addMmsMsgToDB(pMsg, pMsgInfo) != MSG_SUCCESS) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
		}
		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);
		strcpy((char *)pMsgInfo->msgData,szTemp);

		_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);

	} else if (pMsgInfo->msgType.subType == MSG_READREPLY_MMS || pMsgInfo->msgType.subType == MSG_READRECIND_MMS) {
		MSG_DEBUG("######## MmsPlgAddMessage -> MSG_READREPLY_MMS || MSG_READRECIND_MMS ###########");

		char filePath[MAX_FULL_PATH_SIZE+1] = {0, };
		FILE *pFile = NULL;

		MSG_READ_REPORT_STATUS_T readStatus;
		MSG_MESSAGE_ID_T selectedMsgId;
		int	version;

		memcpy(&readStatus, pMsgInfo->msgData, sizeof(MSG_READ_REPORT_STATUS_T));
		memcpy(&selectedMsgId, pMsgInfo->msgData + sizeof(MSG_READ_REPORT_STATUS_T), sizeof(MSG_MESSAGE_ID_T));

		version = MmsPluginStorage::instance()->getMmsVersion(selectedMsgId);

		snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, MSG_DATA_PATH"%d.mms", pMsgInfo->msgId);

		MmsComposeReadReportMessage(&mmsMsg, pMsgInfo, selectedMsgId);

		if (addMmsMsgToDB(&mmsMsg, pMsgInfo) != MSG_SUCCESS) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
		}

		snprintf(filePath, MAX_FULL_PATH_SIZE+1, MSG_DATA_PATH"%d", mmsMsg.msgID);
		pFile = MsgOpenMMSFile(filePath);
		if (!pFile) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			MsgCloseFile(pFile);
			pFile = NULL;

			THROW(MsgException::MMS_PLG_ERROR, "MsgOpenMMSFile error");
		}

		if (fchmod(fileno(pFile), file_mode) < 0) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			MsgCloseFile(pFile);
			pFile = NULL;

			THROW(MsgException::MMS_PLG_ERROR, "chmod() error: %s", strerror(errno));
		}

		if (version == 0x90) {
			MSG_DEBUG("### version 1.0 ###");
			if (_MmsEncodeReadReport10(pFile, &mmsMsg, readStatus) != true) {
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				MsgCloseFile(pFile);
				pFile = NULL;

				THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.0 Error");
			}
		} else {
			MSG_DEBUG("### version 1.1 ###");
			if (_MmsEncodeReadReport11(pFile, &mmsMsg, readStatus) != true) {
				MsgFreeAttrib(&mmsMsg.mmsAttrib);
				MsgCloseFile(pFile);
				pFile = NULL;

				THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.1 Error");
			}
		}

		MsgFsync(pFile);
		MsgCloseFile(pFile);
		pFile = NULL;

		_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);

		MsgFreeAttrib(&mmsMsg.mmsAttrib);

	} else if (pMsgInfo->msgType.subType == MSG_FORWARD_MMS) {
		MSG_DEBUG("######## MmsPlgAddMessage -> MSG_FORWARD_MMS ###########");

		char filePath[MAX_FULL_PATH_SIZE + 1] = {0, };
		char szTemp[MAX_MSG_DATA_LEN + 1] = {0, };
		FILE *pFile = NULL;
		MMS_MESSAGE_DATA_S mmsMsgData;

		if (MmsComposeMessage(&mmsMsg, pMsgInfo, pSendOptInfo, &mmsMsgData, pFileData) != true) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Message Compose Error");
		}

		strcpy(szTemp,pMsgInfo->msgData);

		snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN + 1, MSG_DATA_PATH"%d.mms", pMsgInfo->msgId);

		if (addMmsMsgToDB(&mmsMsg, pMsgInfo, _MsgMmsGetAttachCount(&mmsMsgData)) != MSG_SUCCESS) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
		}

		strcpy((char *)pMsgInfo->msgData,szTemp);

		snprintf(filePath, MAX_FULL_PATH_SIZE + 1 , MSG_DATA_PATH"%d", mmsMsg.msgID);

		pFile = MsgOpenMMSFile(filePath);
		if (!pFile) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);
			MsgCloseFile(pFile);
			pFile = NULL;

			THROW(MsgException::MMS_PLG_ERROR, "MsgOpenMMSFile error");
		}

		if (fchmod(fileno(pFile), file_mode) < 0) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);
			MsgCloseFile(pFile);
			pFile = NULL;

			THROW(MsgException::MMS_PLG_ERROR, "chmod() error: %s", strerror(errno));
		}

		if (_MmsEncodeSendReq(pFile, &mmsMsg) != true) {
			_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
			MsgFreeAttrib(&mmsMsg.mmsAttrib);
			__MmsReleaseMmsLists(&mmsMsgData);
			MsgCloseFile(pFile);
			pFile = NULL;

			THROW(MsgException::MMS_PLG_ERROR, "MMS Message Encode Send Req Error");
		}
		MsgFsync(pFile);
		MsgCloseFile(pFile);
		pFile = NULL;

		_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
		MsgFreeAttrib(&mmsMsg.mmsAttrib);
		__MmsReleaseMmsLists(&mmsMsgData);
	}

	MSG_END();
}


void MmsPluginStorage::composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	char filePath[MAX_FULL_PATH_SIZE+1] = {0, };
	FILE *pFile = NULL;

	MSG_READ_REPORT_STATUS_T readStatus;
	int	version;

	memcpy(&readStatus, pMsgInfo->msgData, sizeof(MSG_READ_REPORT_STATUS_T));

	MSG_DEBUG("pMsgInfo->msgId = %d", pMsgInfo->msgId);

	version = MmsPluginStorage::instance()->getMmsVersion(pMsgInfo->msgId);

	snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, MSG_DATA_PATH"%d.mms", pMsgInfo->msgId);

	if (version == 0x90)
		pMsgInfo->msgType.subType = MSG_READREPLY_MMS;
	else
		pMsgInfo->msgType.subType = MSG_READRECIND_MMS;

	MmsComposeReadReportMessage(&mmsMsg, pMsgInfo, pMsgInfo->msgId);

	snprintf(filePath, MAX_FULL_PATH_SIZE+1, MSG_DATA_PATH"%d", mmsMsg.msgID);
	pFile = MsgOpenMMSFile(filePath);
	if (!pFile)
		THROW(MsgException::MMS_PLG_ERROR, "MsgOpenMMSFile Error");

	if (version == 0x90) {
		MSG_DEBUG("### version 1.0 ###");
		if (_MmsEncodeReadReport10(pFile, &mmsMsg, readStatus) != true) {
			MsgCloseFile(pFile);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.0 Error");
		}
	} else {
		MSG_DEBUG("### version 1.1 ###");
		if (_MmsEncodeReadReport11(pFile, &mmsMsg, readStatus) != true) {
			MsgCloseFile(pFile);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.1 Error");
		}
	}

	MsgCloseFile(pFile);
}


MSG_ERROR_T MmsPluginStorage::addMmsMsgToDB(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, int attachCnt)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	// Add Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (checkExistedMessage(pMmsMsg->msgID) == false) {
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s');",
					MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsMsg->msgID, pMmsMsg->szTrID, pMmsMsg->szMsgID, pMmsMsg->szForwardMsgID, pMmsMsg->szContentLocation, pMsgInfo->msgData);

		dbHandle.beginTrans();

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);",
					MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, pMmsMsg->msgID, pMmsMsg->mmsAttrib.version, pMmsMsg->mmsAttrib.dataType,
												pMmsMsg->mmsAttrib.date, pMmsMsg->mmsAttrib.bHideAddress, pMmsMsg->mmsAttrib.bAskDeliveryReport,
												pMmsMsg->mmsAttrib.bReportAllowed, pMmsMsg->mmsAttrib.readReportAllowedType, pMmsMsg->mmsAttrib.bAskReadReply,
												pMmsMsg->mmsAttrib.bRead, pMmsMsg->mmsAttrib.readReportSendStatus, pMmsMsg->mmsAttrib.bReadReportSent,
												pMmsMsg->mmsAttrib.priority, pMmsMsg->mmsAttrib.bLeaveCopy, pMmsMsg->mmsAttrib.msgSize, pMmsMsg->mmsAttrib.msgClass,
												pMmsMsg->mmsAttrib.expiryTime.time,	pMmsMsg->mmsAttrib.bUseDeliveryCustomTime, pMmsMsg->mmsAttrib.deliveryTime.time,
												pMmsMsg->mmsAttrib.msgStatus);

		MSG_DEBUG("\n!!!!!!!!! QUERY : %s\n", sqlQuery);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (updateMmsAttachCount(pMmsMsg->msgID, attachCnt) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		dbHandle.endTrans(true);
	} else {
		MSG_DEBUG("Already existed message");

		if (updateMmsAttachCount(pMmsMsg->msgID, attachCnt) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T	MmsPluginStorage::plgGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg)
{
	MSG_BEGIN();

	MSG_ERROR_T	err = MSG_SUCCESS;

	int partCnt = 0;
	unsigned int nSize = 0;

	MsgType partHeader;
	MmsAttrib pMmsAttrib;

	char szBuf[MSG_FILEPATH_LEN_MAX] = {0, };
	bool bMultipartRelated = false;

	if (pSendOptInfo != NULL) {
		char sqlQuery[MAX_QUERY_LEN + 1];

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ASK_DELIVERY_REPORT, KEEP_COPY, ASK_READ_REPLY, PRIORITY, EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME FROM %s WHERE REFERENCE_ID IN \
								(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
								MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

		MSG_DEBUG("### SQLQuery = %s ###", sqlQuery);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
			MSG_DEBUG("MSG_ERR_DB_PREPARE");

		if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
			pSendOptInfo->bDeliverReq = dbHandle.columnInt(0);
			MSG_DEBUG("## delivery = %d ##", pSendOptInfo->bDeliverReq);

			pSendOptInfo->bKeepCopy = dbHandle.columnInt(1);
			MSG_DEBUG("## bKeepCopy = %d ##", pSendOptInfo->bKeepCopy);

			pSendOptInfo->option.mmsSendOptInfo.bReadReq = dbHandle.columnInt(2);
			MSG_DEBUG("## bReadReq = %d ##", pSendOptInfo->option.mmsSendOptInfo.bReadReq);

			pSendOptInfo->option.mmsSendOptInfo.priority = dbHandle.columnInt(3);
			MSG_DEBUG("## priority = %d ##", pSendOptInfo->option.mmsSendOptInfo.priority);

			pSendOptInfo->option.mmsSendOptInfo.expiryTime.time = (unsigned int)dbHandle.columnInt(4);
			MSG_DEBUG("## expiryTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.expiryTime.time);

			pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime = (unsigned int)dbHandle.columnInt(5);
			MSG_DEBUG("## bUseDeliveryCustomTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime);

			pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = (unsigned int)dbHandle.columnInt(6);
			MSG_DEBUG("## deliveryTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time);
		} else {
			dbHandle.finalizeQuery();

			return MSG_ERR_DB_STEP;
		}

		dbHandle.finalizeQuery();
	}

	if (_MmsReadMsgBody(pMsg->msgId, true, false, NULL) == false) {
		MSG_DEBUG("The MMS Message might include drm contents!!!");

#ifdef __SUPPORT_DRM__
		if (MmsDrm2GetConvertState() == MMS_DRM2_CONVERT_REQUIRED) {
			bool bRetToConvert = true;

			bRetToConvert = MmsDrm2ConvertMsgBody(mmsHeader.msgType.szOrgFilePath);

			MmsDrm2SetConvertState(MMS_DRM2_CONVERT_FINISH);

			if (bRetToConvert) {
				remove(mmsHeader.msgType.szOrgFilePath);
				rename(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, mmsHeader.msgType.szOrgFilePath);

				if (MmsDrm2ReadMsgConvertedBody(pMsg, true, false, NULL) == false) {
					MSG_DEBUG("MmsLoadMsg:MmsDrm2ReadMsgConvertedBody() returns false\n");
					goto L_CATCH;
				}
			} else {
				goto L_CATCH;
			}
		}
#endif
	}

	MmsGetMsgAttrib(pMsg->msgId, &pMmsAttrib);

	pMmsMsg->regionCnt = 0;
	pMmsMsg->pageCnt = 0;
	pMmsMsg->attachCnt = 0;
	pMmsMsg->transitionCnt = 0;
	pMmsMsg->metaCnt = 0;
	memset(pMmsMsg->szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

	if (pMmsAttrib.contentType == MIME_MULTIPART_RELATED || pMmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		char *pSmilDoc = NULL;

		pSmilDoc = MmsSmilGetPresentationData(pMsg->msgId);
		if (!pSmilDoc) {
	 		goto L_CATCH;
		}

		MmsSmilParseSmilDoc(pMmsMsg, pSmilDoc);
		bMultipartRelated = true;
	}

	partCnt = MmsGetMediaPartCount(pMsg->msgId);
	MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt);

	if (partCnt < 0) {
		MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt);
 		goto FREE_CATCH;
	}

	for (int i = 0; i < partCnt; ++i) {
		if (!MmsGetMediaPartHeader(i, &partHeader)) {
			MSG_DEBUG("MmsUiGetMediaAttachInfo: MmsGetMediaPartHeader failed\n");
			goto FREE_CATCH;
		}

		if (partHeader.contentSize > 0) {
			if (!strcasecmp(partHeader.param.szFileName, "cid:")) {
				strncpy((char *)szBuf, &partHeader.param.szFileName[4], MSG_FILEPATH_LEN_MAX - 1);
			} else {
				strcpy((char *)szBuf, partHeader.param.szFileName);
			}
			sprintf(partHeader.param.szFileName, MSG_DATA_PATH"%s", szBuf);

			if (!bMultipartRelated || MmsCheckAdditionalMedia(pMmsMsg, &partHeader)) {

				MMS_ATTACH_S *attachment = NULL;
				int tempType;

				attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

				MsgGetTypeByFileName(&tempType, partHeader.param.szFileName);
				attachment->mediatype = (MimeType)tempType;

				strcpy(attachment->szFilePath, partHeader.param.szFileName);

				strncpy(attachment->szFileName, partHeader.param.szName, MSG_FILENAME_LEN_MAX -1);

				attachment->fileSize = partHeader.contentSize;

				_MsgMmsAddAttachment(pMmsMsg, attachment);
			}
		}
	}

	*pDestMsg = _MsgMmsSerializeMessageData(pMmsMsg, &nSize);

	__MmsReleaseMmsLists(pMmsMsg);


	MmsMsg *pStoMmsMsg;
	MmsPluginStorage::instance()->getMmsMessage(&pStoMmsMsg);
	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pStoMmsMsg->msgType.drmInfo);
#endif
	_MsgFreeBody(&pStoMmsMsg->msgBody, pStoMmsMsg->msgType.type);

	pMsg->dataSize = nSize;

	if (err != MSG_SUCCESS)
		MSG_DEBUG("MmsPlgUpdateMessage : Update MMS Message Failed");

	MSG_END();

	return err;

FREE_CATCH:
	if (bMultipartRelated) {
		__MmsReleaseMmsLists(pMmsMsg);
	}

L_CATCH:
	{
		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		_MmsInitHeader();

		_MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
		_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif
		_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);

		return MSG_ERR_STORAGE_ERROR;
	}
	MSG_END();
}


MSG_ERROR_T MmsPluginStorage::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	MSG_BEGIN();

	MSG_ERROR_T	err = MSG_SUCCESS;

	MmsMsg mmsMsg;
	bzero(&mmsMsg, sizeof(mmsMsg));

	char filePath[MAX_FULL_PATH_SIZE+1] = {0, };
	char sqlQuery[MAX_QUERY_LEN + 1];

	unsigned int addrId = 0;
	FILE *pFile = NULL;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, KEEP_COPY = %d, ASK_READ_REPLY = %d, EXPIRY_TIME = %d, CUSTOM_DELIVERY_TIME = %d, DELIVERY_TIME= %d, PRIORITY = %d \
									WHERE REFERENCE_ID IN \
									(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
				MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->bKeepCopy, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
												pSendOptInfo->option.mmsSendOptInfo.expiryTime.time, pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime,
												pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time, pSendOptInfo->option.mmsSendOptInfo.priority,
												MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MMS_MESSAGE_DATA_S mmsMsgData;

	if (MmsComposeMessage(&mmsMsg, pMsgInfo, pSendOptInfo, &mmsMsgData, pFileData) != true) {
		_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
		MsgFreeAttrib(&mmsMsg.mmsAttrib);
		__MmsReleaseMmsLists(&mmsMsgData);

		THROW(MsgException::MMS_PLG_ERROR, "MMS Message Compose Error");
	}

	snprintf(filePath, MAX_FULL_PATH_SIZE+1, MSG_DATA_PATH"%d", mmsMsg.msgID);

	pFile = MsgOpenMMSFile(filePath);

	if (_MmsEncodeSendReq(pFile, &mmsMsg) != true) {
		_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
		MsgFreeAttrib(&mmsMsg.mmsAttrib);
		__MmsReleaseMmsLists(&mmsMsgData);
		MsgCloseFile(pFile);

		THROW(MsgException::MMS_PLG_ERROR, "MMS Message Encode Send Req Error");
	}

	MsgCloseFile(pFile);

	_MsgFreeBody(&mmsMsg.msgBody, mmsMsg.msgType.type);
	MsgFreeAttrib(&mmsMsg.mmsAttrib);

	__MmsReleaseMmsLists(&mmsMsgData);

	MSG_END();

	return err;
}


MSG_ERROR_T MmsPluginStorage::updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	MSG_DEBUG("###### pMsgInfo->msgData = %s #######", pMmsRecvData->retrievedFilePath);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s', FILE_PATH = '%s' WHERE REFERENCE_ID IN \
					(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
					MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID, pMmsRecvData->retrievedFilePath,
					MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	MSG_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s' WHERE REFERENCE_ID IN \
					(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
					MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID,
					MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	MSG_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	if (pSendOptInfo != NULL) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d, EXPIRY_TIME = %d WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
						pSendOptInfo->option.mmsSendOptInfo.priority, pSendOptInfo->option.mmsSendOptInfo.expiryTime.time,
						MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

		MSG_DEBUG("SQLQuery = %s", sqlQuery);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			return MSG_ERR_DB_EXEC;
		}
	}

	dbHandle.finalizeQuery();

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateNetStatus(MSG_MESSAGE_ID_T msgId, MSG_NETWORK_STATUS_T netStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, netStatus, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateDeliveryReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET DELIVERY_REPORT_STATUS = %d, DELIVERY_REPORT_TIME = %ld \
				      WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pStatus->msgStatus, pStatus->handledTime, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateReadReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_REPORT_STATUS = %d, READ_REPORT_TIME = %lu \
				      WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, pStatus->readStatus, pStatus->readTime, msgId);


	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateMmsAttrib(MSG_MESSAGE_ID_T msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgSubType == MSG_NOTIFICATIONIND_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET EXPIRY_TIME = %d WHERE REFERENCE_ID IN \
								(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
								MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, attrib->expiryTime.time,
								MSGFW_MESSAGE_TABLE_NAME, msgId);
	} else if (msgSubType == MSG_RETRIEVE_AUTOCONF_MMS || msgSubType == MSG_RETRIEVE_MANUALCONF_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d WHERE REFERENCE_ID IN \
								(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
								MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, attrib->bAskDeliveryReport, attrib->bAskReadReply, attrib->priority,
								MSGFW_MESSAGE_TABLE_NAME, msgId);
	}

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::updateMmsAttachCount(MSG_MESSAGE_ID_T msgId, int count)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ATTACHMENT_COUNT = %d WHERE MSG_ID = %d;",
								MSGFW_MESSAGE_TABLE_NAME, count, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to execute query [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}

MmsMsgMultiStatus *MmsPluginStorage::getMultiStatus(MSG_MESSAGE_ID_T msgId)
{
	MSG_BEGIN();

	MmsMsgMultiStatus *pMultiStatus = NULL;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.ADDRESS_VAL, A.DELIVERY_REPORT_STATUS, A.DELIVERY_REPORT_TIME, \
						  A.READ_REPORT_STATUS, A.READ_REPORT_TIME \
					FROM %s A, %s B \
				     WHERE A.MSG_ID = %d AND A.ADDRESS_ID = B.ADDRESS_ID;",
				MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		MSG_DEBUG("MSG_ERR_DB_PREPARE");

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pMultiStatus = (MmsMsgMultiStatus *)malloc(sizeof(MmsMsgMultiStatus));

		memset(pMultiStatus, 0, sizeof(MmsMsgMultiStatus));

		if (dbHandle.columnText(0) != NULL) {
			strncpy(pMultiStatus->szTo, (char *)dbHandle.columnText(0), (strlen((char *)dbHandle.columnText(0)) > MAX_ADDRESS_VAL_LEN ? MAX_ADDRESS_VAL_LEN : strlen((char *)dbHandle.columnText(0))));
			MSG_DEBUG("### szTo = %s ###", pMultiStatus->szTo);
		}

		pMultiStatus->msgStatus = (MSG_DELIVERY_REPORT_STATUS_T)dbHandle.columnInt(1);
		pMultiStatus->handledTime = dbHandle.columnInt(2);
		pMultiStatus->readStatus = (MSG_READ_REPORT_STATUS_T)dbHandle.columnInt(3);
		pMultiStatus->readTime = dbHandle.columnInt(4);
	} else {
		MSG_DEBUG("MSG_ERR_DB_STEP");
	}

	/* possible NULL pointer dereference*/
	if (pMultiStatus != NULL)
		pMultiStatus->pNext = NULL;

	dbHandle.finalizeQuery();

	return pMultiStatus;
}


void MmsPluginStorage::getMmsAttrib(MSG_MESSAGE_ID_T msgId, MmsMsg *pMmsMsg)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		MSG_DEBUG("MSG_ERR_DB_PREPARE");

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pMmsMsg->mmsAttrib.version = dbHandle.columnInt(1);
		pMmsMsg->mmsAttrib.dataType = (MmsDataType)dbHandle.columnInt(2);
		pMmsMsg->mmsAttrib.date = dbHandle.columnInt(3);
		pMmsMsg->mmsAttrib.bHideAddress = dbHandle.columnInt(4);
		pMmsMsg->mmsAttrib.bAskDeliveryReport = dbHandle.columnInt(5);
		pMmsMsg->mmsAttrib.bReportAllowed = dbHandle.columnInt(6);
		pMmsMsg->mmsAttrib.readReportAllowedType = (MmsRecvReadReportType)dbHandle.columnInt(7);
		pMmsMsg->mmsAttrib.bAskReadReply = dbHandle.columnInt(8);
		pMmsMsg->mmsAttrib.bRead = dbHandle.columnInt(9);
		pMmsMsg->mmsAttrib.readReportSendStatus = (MmsRecvReadReportSendStatus)dbHandle.columnInt(10);
		pMmsMsg->mmsAttrib.bReadReportSent = dbHandle.columnInt(11);
		pMmsMsg->mmsAttrib.priority = (MmsPriority)dbHandle.columnInt(12);
		pMmsMsg->mmsAttrib.msgSize = dbHandle.columnInt(13);
		pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)dbHandle.columnInt(14);
		pMmsMsg->mmsAttrib.expiryTime.time = dbHandle.columnInt(15);
		pMmsMsg->mmsAttrib.deliveryTime.time = dbHandle.columnInt(17);
		pMmsMsg->mmsAttrib.msgStatus = (MSG_DELIVERY_REPORT_STATUS_T)dbHandle.columnInt(18);
	}

	dbHandle.finalizeQuery();
}


MSG_ERROR_T MmsPluginStorage::getMmsMessageId(MSG_MESSAGE_ID_T selectedMsgId, MmsMsg *pMmsMsg)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int rowCnt = 0;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MESSAGE_ID FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, selectedMsgId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return MSG_ERR_DB_NORECORD;
	}

	if (rowCnt != 1) {
		dbHandle.freeTable();
		MSG_DEBUG("[Error] MSG_ERR_DB_NORECORD");
		return MSG_ERR_DB_NORECORD;
	}

	dbHandle.getColumnToString(1, MMS_MSG_ID_LEN + 1, pMmsMsg->szMsgID);

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


int MmsPluginStorage::getMmsVersion(MSG_MESSAGE_ID_T selectedMsgId)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	int rowCnt = 0;

	int	version = 0;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, selectedMsgId);

	MSG_DEBUG("SqlQuery = %s", sqlQuery);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return version;
	}

	if (rowCnt != 1) {
		dbHandle.freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return version;
	}

	version = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	return version;
}


MSG_ERROR_T MmsPluginStorage::getContentLocation(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONTENTS_LOCATION FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle.columnText(0) != NULL) {
			strncpy(pMsgInfo->msgData, (char *)dbHandle.columnText(0), (strlen((char *)dbHandle.columnText(0)) > MAX_MSG_DATA_LEN ? MAX_MSG_DATA_LEN : strlen((char *)dbHandle.columnText(0))));
			pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
		}
	} else {
		dbHandle.finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


/* reject_msg_support */
MSG_ERROR_T MmsPluginStorage::getTrID(MSG_MESSAGE_INFO_S *pMsgInfo,char *pszTrID,int nBufferLen)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT TRANSACTION_ID FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle.columnText(0) != NULL) {
			strncpy(pszTrID, (char *)dbHandle.columnText(0), nBufferLen - 1);
			pszTrID[nBufferLen-1] = '\0';
		}
	} else {
		dbHandle.finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}
/* reject_msg_support */


MSG_ERROR_T MmsPluginStorage::getMmsRawFilePath(MSG_MESSAGE_ID_T msgId, char *pFilepath)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE REFERENCE_ID IN \
						(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle.columnText(0) != NULL) {
			strcpy(pFilepath, (char *)dbHandle.columnText(0));
		}
	} else {
		dbHandle.finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


int MmsPluginStorage::searchMsgId(char *toNumber, char *szMsgID)
{
	int msgId = -1;

	MSG_FOLDER_ID_T	folderId = MSG_SENTBOX_ID;

	char sqlQuery[MAX_QUERY_LEN + 1];

	MmsAddrUtilRemovePlmnString(toNumber);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID FROM %s A, %s B \
				 WHERE A.REFERENCE_ID = B.REFERENCE_ID AND A.FOLDER_ID = %d AND B.MESSAGE_ID LIKE '%%%s%%'",
				MSGFW_MESSAGE_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME, folderId, szMsgID);

	MSG_DEBUG("sqlQuery [%s]", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgId = dbHandle.columnInt(0);
	}

	dbHandle.finalizeQuery();

	return msgId;
}


MSG_ERROR_T MmsPluginStorage::setReadReportSendStatus(MSG_MESSAGE_ID_T msgId, int readReportSendStatus)
{
	bool bReadReportSent = false;

	if ((MmsRecvReadReportSendStatus)readReportSendStatus == MMS_RECEIVE_READ_REPORT_SENT)
		bReadReportSent = true;
	else
		bReadReportSent = false;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_REPORT_SEND_STATUS = %d, READ_REPORT_SENT = %d WHERE REFERENCE_ID IN \
					(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
					MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, (MmsRecvReadReportSendStatus)readReportSendStatus, (int)bReadReportSent,
					MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


bool MmsPluginStorage::checkExistedMessage(MSG_MESSAGE_ID_T msgId)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT REFERENCE_ID FROM %s WHERE REFERENCE_ID IN \
					(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
					MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	int rowCnt = 0;
	int refId = 0;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	// No record or other error
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("result error");
		dbHandle.freeTable();
		return false;
	}

	refId = dbHandle.getColumnToInt(1);

	MSG_DEBUG("refId : [%d]", refId);

	if (rowCnt > 0) {
		dbHandle.freeTable();
		return true;
	} else {
		dbHandle.freeTable();
		return false;
	}
}


MSG_ERROR_T MmsPluginStorage::getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;
	char *pMmsMsgText = NULL;
	int textLen = 0;
	bool bText = false;

	// Get the text data from the 1st slide.
	for (int i = 0; i< pMmsMsg->pageCnt; ++i) {
		pPage = _MsgMmsGetPage(pMmsMsg, i);

		for (int j = 0; j < pPage->mediaCnt; ++j) {
			pMedia = _MsgMmsGetMedia(pPage, j);

			if (pMedia->mediatype == MMS_SMIL_MEDIA_TEXT && pMedia->szFilePath != NULL) {
				pMmsMsgText = MsgOpenAndReadMmsFile(pMedia->szFilePath, 0, -1, &textLen);
				if (pMmsMsgText)
					strncpy(pMsgText, pMmsMsgText, MAX_MSG_TEXT_LEN);

				// for avoiding break character end of the string.
				if ((textLen >= MAX_MSG_TEXT_LEN) && pMsgText[MAX_MSG_TEXT_LEN - 1] >= 0x80) { // if it is multibyte chars by UTF8, it would be presendted by 1xxx xxxx
					for (int k = 1; k < 5; k++) {
						// the first byte of multi-byte chars of UTF8, should be larger than 1100 0000
						// (two byte chars start with 110x xxxx, and three byte chars start with 1110 xxxx,
						// four byte chars start with 1111 0xxx)
						if ((pMsgText[MAX_MSG_TEXT_LEN - k] >= 0xC0)) {
							pMsgText[MAX_MSG_TEXT_LEN - k] = '\0';
							break;
						}
					}
				}

				if (pMmsMsgText) {
					free(pMmsMsgText);
					pMmsMsgText = NULL;
				}
				bText = true;
				break;
			}
		}

		if (bText)
			break;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::makeThumbnail(MMS_MESSAGE_DATA_S *pMmsMsg, char *pThumbnailPath, char *szFileName)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	bool bThumbnail = false;

	for (int i = 0; i < pMmsMsg->pageCnt; i++) {
		pPage = _MsgMmsGetPage(pMmsMsg, i);
		for (int j = 0; j < pPage->mediaCnt; j++) {
			pMedia = _MsgMmsGetMedia(pPage, j);

			MSG_DEBUG("pMedia's Name: %s", pMedia->szFilePath);

			if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG) {
				MSG_DEBUG("Make thumbnail: image");

				char thumbPath[MSG_FILEPATH_LEN_MAX] = {0, };

				snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"/%s.jpg", szFileName);
				int err = -1;
				err = thumbnail_request_save_to_file(pMedia->szFilePath, MEDIA_THUMB_LARGE, thumbPath);
				if (err < 0) {
					MSG_DEBUG("Make thumbnail: image failed");
					continue;
				}

				strncpy(pThumbnailPath, thumbPath, MSG_FILEPATH_LEN_MAX);

				bThumbnail = true;

				break;
			} else if (pMedia->mediatype == MMS_SMIL_MEDIA_VIDEO) {
				MSG_DEBUG("Make thumbnail: video");

				MMHandleType content_attrs = (MMHandleType)NULL;
				char *err_attr_name = NULL;

				int fileRet = 0;

				int trackCount = 0;

				mm_file_create_content_attrs(&content_attrs, pMedia->szFilePath);

				fileRet = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_VIDEO_TRACK_COUNT, &trackCount, NULL);

				if (fileRet != 0) {
					MSG_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);
					if (err_attr_name) {
						free(err_attr_name);
						err_attr_name = NULL;
					}

					mm_file_destroy_content_attrs(content_attrs);

					continue;
				}

				MSG_DEBUG("video track num: %d", trackCount);

				if (trackCount > 0) {
					int thumbnailWidth = 0;
					int thumbnailHeight = 0;
					int thumbnailSize = 0;

					void *thumbnail = NULL;

					fileRet = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_VIDEO_WIDTH, &thumbnailWidth,
															MM_FILE_CONTENT_VIDEO_HEIGHT, &thumbnailHeight,
															MM_FILE_CONTENT_VIDEO_THUMBNAIL, &thumbnail, &thumbnailSize,
															NULL);

					if (fileRet != 0) {
						MSG_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);
						if (err_attr_name) {
							free(err_attr_name);
							err_attr_name = NULL;
						}

						mm_file_destroy_content_attrs(content_attrs);

						continue;
					}

					MSG_DEBUG("video width: %d", thumbnailWidth);
					MSG_DEBUG("video height: %d", thumbnailHeight);
					MSG_DEBUG("video thumbnail: %p", thumbnail);

					if (thumbnail) {
						char thumbPath[MSG_FILEPATH_LEN_MAX] = {0, };

						snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"%s.jpg", szFileName);

						fileRet = mm_util_jpeg_encode_to_file (thumbPath, thumbnail, thumbnailWidth, thumbnailHeight, MM_UTIL_JPEG_FMT_RGB888, 70);

						if (fileRet != 0) {
							MSG_DEBUG("mm_util_jpeg_encode_to_file fails [%d]", fileRet);

							mm_file_destroy_content_attrs(content_attrs);

							continue;
						}

						memset(pThumbnailPath, 0x00, MSG_FILEPATH_LEN_MAX);

						strncpy(pThumbnailPath, thumbPath, MSG_FILEPATH_LEN_MAX);

						bThumbnail = true;

					}

					mm_file_destroy_content_attrs(content_attrs);

					break;
				}

				mm_file_destroy_content_attrs(content_attrs);
			} else if (pMedia->mediatype == MMS_SMIL_MEDIA_AUDIO) {
				MSG_DEBUG("Make thumbnail: %s", "audio");

				MMHandleType tag_attrs = (MMHandleType)NULL;
				char *err_attr_name = NULL;
				void *artwork = NULL;
				int artworkSize = 0;
				int tmpLen = 0;
				int fileRet = 0;

				if (mm_file_create_tag_attrs(&tag_attrs, pMedia->szFilePath) == 0) {


					fileRet = mm_file_get_attrs(tag_attrs, &err_attr_name, MM_FILE_TAG_ARTWORK, &artwork, &tmpLen,
														MM_FILE_TAG_ARTWORK_SIZE, &artworkSize,
														NULL);

					mm_file_destroy_tag_attrs(tag_attrs);

					if (fileRet != 0) {
						MSG_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);
						if (err_attr_name) {
							free(err_attr_name);
							err_attr_name = NULL;
						}

						continue;
					}
				} else {
					MSG_DEBUG("mm_file_create_tag_attrs fails");
					continue;
				}

				MSG_DEBUG("artwork: %p", artwork);
				MSG_DEBUG("artwork_size: %d", artworkSize);

				if (artwork) {
					char thumbPath[MSG_FILEPATH_LEN_MAX] = {0, };

					snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"%s.jpg", szFileName);

					FILE *tmp = MsgOpenFile(thumbPath, "wb+");

					if(!tmp) {
						MSG_DEBUG("MsgOpenFile file error");
						return MSG_ERR_STORAGE_ERROR;
					}

					if (MsgWriteFile((char*)artwork, 1, artworkSize, tmp) != (size_t)artworkSize) {
						MSG_DEBUG("MsgWriteFile error");
						MsgCloseFile(tmp);
						return MSG_ERR_STORAGE_ERROR;
					}
					MsgFsync(tmp);
					MsgCloseFile(tmp);

					strncpy(pThumbnailPath, thumbPath, MSG_FILEPATH_LEN_MAX);

					bThumbnail = true;

					break;
				}
			}
		}

		if (bThumbnail == true)
			break;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MmsPluginStorage::addMmsNoti(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MSG_ERROR_T	err = MSG_SUCCESS;

	err = MsgInsertMmsReportToNoti(&dbHandle, pMsgInfo);

	MSG_END();

	return err;
}

/* This API is not used anywhere now */
MSG_ERROR_T	MmsPluginStorage::plgGetRestoreMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg, char *filePath)
{
	MSG_BEGIN();

	MSG_ERROR_T	err = MSG_SUCCESS;
	int partCnt = 0;
	MsgType partHeader;
	MmsAttrib pMmsAttrib;
	char szBuf[MSG_FILEPATH_LEN_MAX] = {0, };
	unsigned int nSize;
	bool bMultipartRelated = false;

	if (_MmsReadMsgBody(pMsg->msgId, true, true, filePath) == false) {
		MSG_DEBUG("The MMS Message might include drm contents!!!");

#ifdef __SUPPORT_DRM__
		if (MmsDrm2GetConvertState() == MMS_DRM2_CONVERT_REQUIRED) {
			bool bRetToConvert = true;

			bRetToConvert = MmsDrm2ConvertMsgBody(mmsHeader.msgType.szOrgFilePath);

			MmsDrm2SetConvertState(MMS_DRM2_CONVERT_FINISH);

			if (bRetToConvert) {
				remove(mmsHeader.msgType.szOrgFilePath);
				rename(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, mmsHeader.msgType.szOrgFilePath);

				if (MmsDrm2ReadMsgConvertedBody(pMsg, true, false, NULL) == false) {
					MSG_DEBUG("MmsLoadMsg:MmsDrm2ReadMsgConvertedBody() returns false\n");
					goto L_CATCH;
				}
			} else {
				goto L_CATCH;
			}
		}
#endif
	}

	MmsGetMsgAttrib(pMsg->msgId, &pMmsAttrib);

	pMmsMsg->regionCnt = 0;
	pMmsMsg->pageCnt = 0;
	pMmsMsg->attachCnt = 0;
	pMmsMsg->transitionCnt = 0;
	pMmsMsg->metaCnt = 0;
	memset(pMmsMsg->szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

	if (pMmsAttrib.contentType == MIME_MULTIPART_RELATED || pMmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		char *pSmilDoc = NULL;

		pSmilDoc = MmsSmilGetPresentationData(pMsg->msgId);
		if (!pSmilDoc) {
			goto L_CATCH;
		}

		MmsSmilParseSmilDoc(pMmsMsg, pSmilDoc);
		bMultipartRelated = true;
	}

	partCnt = MmsGetMediaPartCount(pMsg->msgId);
	MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt);

	if (partCnt < 0) {
		MSG_DEBUG("MmsUiGetMediaAttachInfo: partCnt=%d\n", partCnt);
		goto FREE_CATCH;
	}

	for (int i = 0; i < partCnt; ++i) {
		if (!MmsGetMediaPartHeader(i, &partHeader)) {
			MSG_DEBUG("MmsUiGetMediaAttachInfo: MmsGetMediaPartHeader failed\n");
			goto FREE_CATCH;
		}

		if (partHeader.contentSize > 0) {
			if (!strcasecmp(partHeader.param.szFileName, "cid:")) {
				strncpy((char *)szBuf, &partHeader.param.szFileName[4], MSG_FILEPATH_LEN_MAX - 1);
			} else {
				strcpy((char *)szBuf, partHeader.param.szFileName);
			}
			sprintf(partHeader.param.szFileName, MSG_DATA_PATH"%s", szBuf);

			if (!bMultipartRelated || MmsCheckAdditionalMedia(pMmsMsg, &partHeader)) {
				MMS_ATTACH_S *attachment = NULL;
				int tempType;

				attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

				MsgGetTypeByFileName(&tempType, partHeader.param.szFileName);
				attachment->mediatype = (MimeType)tempType;

				strcpy(attachment->szFilePath, partHeader.param.szFileName);

				strncpy(attachment->szFileName, partHeader.param.szName, MSG_FILENAME_LEN_MAX - 1);

				attachment->fileSize = partHeader.contentSize;

				_MsgMmsAddAttachment(pMmsMsg, attachment);
			}
		}
	}

	*pDestMsg = _MsgMmsSerializeMessageData(pMmsMsg, &nSize);

	__MmsReleaseMmsLists(pMmsMsg);

	MmsMsg *pStoMmsMsg;
	MmsPluginStorage::instance()->getMmsMessage(&pStoMmsMsg);
	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
	_MsgFreeDRMInfo(&pStoMmsMsg->msgType.drmInfo);
#endif
	_MsgFreeBody(&pStoMmsMsg->msgBody, pStoMmsMsg->msgType.type);


	pMsg->dataSize = nSize;

	if (err != MSG_SUCCESS)
		MSG_DEBUG("MmsPlgUpdateMessage : Update MMS Message Failed");

	MSG_END();

	return err;

FREE_CATCH:
	if (bMultipartRelated) {
		__MmsReleaseMmsLists(pMmsMsg);

	}

L_CATCH:
	{
		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		_MmsInitHeader();

		_MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
		_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif
		_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);

		return MSG_ERR_STORAGE_ERROR;
	}
	MSG_END();
}
/* This API is not used anywhere now */

