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
#include <sys/stat.h>
#include <errno.h>

#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgMmsMessage.h"
#include "MsgStorageTypes.h"
#include "MsgSmil.h"
#include "MsgSerialize.h"
#include "MsgUtilMime.h"

#include "MmsPluginDebug.h"
#include "MmsPluginStorage.h"
#include "MmsPluginMessage.h"
#include "MmsPluginDrm.h"
#include "MmsPluginUtil.h"
#include "MmsPluginComposer.h"
#include "MmsPluginAppBase.h"

MmsPluginStorage *MmsPluginStorage::pInstance = NULL;


MmsPluginStorage::MmsPluginStorage()
{
	memset(&mmsMsg, 0, sizeof(MmsMsg));
}


MmsPluginStorage::~MmsPluginStorage()
{
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

void MmsPluginStorage::composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	FILE *pFile = NULL;

	msg_read_report_status_t readStatus;
	int	version;

	memcpy(&readStatus, pMsgInfo->msgData, sizeof(msg_read_report_status_t));

	MSG_DEBUG("pMsgInfo->msgId = %d", pMsgInfo->msgId);

	version = MmsPluginStorage::instance()->getMmsVersion(pMsgInfo->msgId);

	snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, "%s%d-Read-Rec.ind", MSG_DATA_PATH, pMsgInfo->msgId);

	if (version == 0x90)
		pMsgInfo->msgType.subType = MSG_READREPLY_MMS;
	else
		pMsgInfo->msgType.subType = MSG_READRECIND_MMS;

	MmsComposeReadReportMessage(&mmsMsg, pMsgInfo, pMsgInfo->msgId);

	pFile = MsgOpenFile(pMsgInfo->msgData, "wb+");
	if (!pFile)
		THROW(MsgException::MMS_PLG_ERROR, "MsgOpenMMSFile Error");

	if (version == 0x90) {
		MSG_DEBUG("### version 1.0 ###");
		if (MmsEncodeReadReport10(pFile, &mmsMsg, readStatus) != true) {
			MsgCloseFile(pFile);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.0 Error");
		}
	} else {
		MSG_DEBUG("### version 1.1 ###");
		if (MmsEncodeReadReport11(pFile, &mmsMsg, readStatus) != true) {
			MsgCloseFile(pFile);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Encode Read Report 1.1 Error");
		}
	}

	MsgCloseFile(pFile);
}

msg_error_t MmsPluginStorage::addMmsMsgToDB(MmsMsg *pMmsMsg, const char *raw_filepath)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	/* Add Message */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsMsg->msgID, pMmsMsg->szTrID, pMmsMsg->szMsgID, pMmsMsg->szForwardMsgID, pMmsMsg->szContentLocation,
			raw_filepath, pMmsMsg->mmsAttrib.version, pMmsMsg->mmsAttrib.dataType, pMmsMsg->mmsAttrib.date, pMmsMsg->mmsAttrib.bHideAddress,
			pMmsMsg->mmsAttrib.bAskDeliveryReport, pMmsMsg->mmsAttrib.bReportAllowed, pMmsMsg->mmsAttrib.readReportAllowedType,
			pMmsMsg->mmsAttrib.bAskReadReply, pMmsMsg->mmsAttrib.bRead, pMmsMsg->mmsAttrib.readReportSendStatus, pMmsMsg->mmsAttrib.bReadReportSent,
			pMmsMsg->mmsAttrib.priority, pMmsMsg->mmsAttrib.bLeaveCopy, pMmsMsg->mmsAttrib.msgSize, pMmsMsg->mmsAttrib.msgClass,
			pMmsMsg->mmsAttrib.expiryTime.time,	0 /*pMmsMsg->mmsAttrib.bUseDeliveryCustomTime*/, pMmsMsg->mmsAttrib.deliveryTime.time, pMmsMsg->mmsAttrib.msgStatus);

	MSG_DEBUG("\n!!!!!!!!! QUERY : %s\n", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

#if 1 /* old */
msg_error_t MmsPluginStorage::updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	MSG_SEC_DEBUG("###### pMsgInfo->msgData = %s #######", pMmsRecvData->retrievedFilePath);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s', FILE_PATH = '%s' WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID, pMmsRecvData->retrievedFilePath, pMsgInfo->msgId);

	MSG_SEC_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

#else /* new */

msg_error_t MmsPluginStorage::updateRetriveConf(msg_message_id_t msgId, MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery),
			"UPDATE %s "
			"SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d, VERSION = %d, MESSAGE_ID = '%s' "
			"WHERE MSG_ID = %d;"
			, MMS_PLUGIN_MESSAGE_TABLE_NAME
			, pMmsData->header->bDeliveryReport
			, pMmsData->header->bReadReport
			, pMmsData->header->mmsPriority
			, pMmsData->header->mmsVersion
			, pMmsData->header->messageID
			, msgId);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();
	int err;

	char *pSerializedMms = NULL;
	gsize pSerializedMmsSize = 0;

	char working_dir[MSG_FILENAME_LEN_MAX+1] = {0, };
	MMS_DATA_S *pMmsData = NULL;

	/* pMsgInfo->msgData is Filepath of json */
	g_file_get_contents((gchar*)pMsgInfo->msgData, (gchar**)&pSerializedMms, (gsize*)&pSerializedMmsSize, NULL);

	if (MsgDeserializeMmsData(pSerializedMms, strlen(pSerializedMms), &pMmsData) != 0) {
		MSG_DEBUG("Fail to Deserialize Message Data");
		return MSG_ERR_NULL_MSGHANDLE;
	}

	updateRetriveConf(pMsgInfo->msgId, pMmsData);

	snprintf(working_dir, sizeof(working_dir), MSG_DATA_PATH"%d.mms.dir", pMsgInfo->msgId);

	MsgMmsSetMultipartListData(pMmsData); /* app file -> data */

	MsgMmsSetMultipartListFilePath(working_dir, pMmsData); /* data -> svc file */

	MMS_MULTIPART_DATA_S *pSmilMultipart = pMmsData->smil;
	if (pSmilMultipart) {
		MmsPluginStorage::instance()->insertMultipart(pMsgInfo->msgId, pSmilMultipart);
	}

	MMSList *multipart_list = pMmsData->multipartlist;
	for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
		MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);

		MmsPluginStorage::instance()->insertMultipart(pMsgInfo->msgId, pMultipart);
	}

	/* make Preview info for APP */
	MmsPluginAppBase appBase(pMmsData);
	appBase.makePreviewInfo(pMsgInfo->msgId, true, NULL);
	appBase.getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));

	MsgMmsRelease(&pMmsData);
	MSG_END();

	return MSG_SUCCESS;
}
#endif

msg_error_t MmsPluginStorage::updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s' WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID, pMsgInfo->msgId);

	MSG_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	if (pSendOptInfo != NULL) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d, EXPIRY_TIME = %d \
				WHERE MSG_ID = %d;", MMS_PLUGIN_MESSAGE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
				pSendOptInfo->option.mmsSendOptInfo.priority, pSendOptInfo->option.mmsSendOptInfo.expiryTime.time, pMsgInfo->msgId);

		MSG_DEBUG("SQLQuery = %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			return MSG_ERR_DB_EXEC;
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertDeliveryReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	/* ( MSG_ID INTEGER , ADDRESS_VAL TEXT , STATUS_TYPE INTEGER , STATUS INTEGER DEFAULT 0 , TIME DATETIME); */
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
			"(MSG_ID, ADDRESS_VAL, STATUS_TYPE, STATUS, TIME) "
			"VALUES (%d, '%s', %d, %d, %d);",
			MSGFW_REPORT_TABLE_NAME, msgId, address, MSG_REPORT_TYPE_DELIVERY, pStatus->msgStatus, (int)pStatus->handledTime);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertReadReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	/* ( MSG_ID INTEGER , ADDRESS_VAL TEXT , STATUS_TYPE INTEGER , STATUS INTEGER DEFAULT 0 , TIME DATETIME); */
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
			"(MSG_ID, ADDRESS_VAL, STATUS_TYPE, STATUS, TIME) "
			"VALUES (%d, '%s', %d, %d, %d);",
			MSGFW_REPORT_TABLE_NAME, msgId, address, MSG_REPORT_TYPE_READ, pStatus->readStatus, (int)pStatus->readTime);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::updateMmsAttrib(msg_message_id_t msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgSubType == MSG_NOTIFICATIONIND_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET EXPIRY_TIME = %d WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, attrib->expiryTime.time, msgId);
	} else if (msgSubType == MSG_RETRIEVE_AUTOCONF_MMS || msgSubType == MSG_RETRIEVE_MANUALCONF_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d, VERSION = %d WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, attrib->bAskDeliveryReport, attrib->bAskReadReply, attrib->priority, attrib->version, msgId);
	}

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::updateMmsAttachCount(msg_message_id_t msgId, int count)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ATTACHMENT_COUNT = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, count, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to execute query [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}

void MmsPluginStorage::getMmsFromDB(msg_message_id_t msgId, MmsMsg *pMmsMsg)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION, DATA_TYPE,  DATE, HIDE_ADDRESS, ASK_DELIVERY_REPORT, REPORT_ALLOWED, \
			READ_REPORT_ALLOWED_TYPE, ASK_READ_REPLY, READ, READ_REPORT_SEND_STATUS, READ_REPORT_SENT, PRIORITY, \
			MSG_SIZE, MSG_CLASS, EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME, MSG_STATUS, \
			MESSAGE_ID, TRANSACTION_ID, CONTENTS_LOCATION, FILE_PATH \
			FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		MSG_DEBUG("MSG_ERR_DB_PREPARE");

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		int i = 0;
		pMmsMsg->mmsAttrib.version = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.dataType = (MmsDataType)dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.date = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bHideAddress = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bAskDeliveryReport = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bReportAllowed = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.readReportAllowedType = (MmsRecvReadReportType)dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bAskReadReply = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bRead = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.readReportSendStatus = (MmsRecvReadReportSendStatus)dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.bReadReportSent = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.priority = (MmsPriority)dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.msgSize = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.expiryTime.time = dbHandle->columnInt(i++);
		i++; /* CUSTOM_DELIVERY_TIME */
		pMmsMsg->mmsAttrib.deliveryTime.time = dbHandle->columnInt(i++);
		pMmsMsg->mmsAttrib.msgStatus = (msg_delivery_report_status_t)dbHandle->columnInt(i++);
		snprintf(pMmsMsg->szMsgID, sizeof(pMmsMsg->szMsgID), "%s", dbHandle->columnText(i++));
		snprintf(pMmsMsg->szTrID, sizeof(pMmsMsg->szTrID), "%s", dbHandle->columnText(i++));
		snprintf(pMmsMsg->szContentLocation, sizeof(pMmsMsg->szContentLocation), "%s", dbHandle->columnText(i++));
		snprintf(pMmsMsg->szFileName, sizeof(pMmsMsg->szFileName), "%s", dbHandle->columnText(i++));
	}


	dbHandle->finalizeQuery();
}

msg_error_t MmsPluginStorage::getMmsMessageId(msg_message_id_t selectedMsgId, MmsMsg *pMmsMsg)
{
	msg_error_t err = MSG_SUCCESS;

	int rowCnt = 0;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MESSAGE_ID FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, selectedMsgId);

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return MSG_ERR_DB_NORECORD;
	}

	if (rowCnt != 1) {
		dbHandle->freeTable();
		MSG_DEBUG("[Error] MSG_ERR_DB_NORECORD");
		return MSG_ERR_DB_NORECORD;
	}

	dbHandle->getColumnToString(1, MMS_MSG_ID_LEN + 1, pMmsMsg->szMsgID);

	dbHandle->freeTable();

	return MSG_SUCCESS;
}


int MmsPluginStorage::getMmsVersion(msg_message_id_t selectedMsgId)
{
	msg_error_t err = MSG_SUCCESS;

	int rowCnt = 0;
	int version = 0;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, selectedMsgId);

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return version;
	}

	if (rowCnt != 1) {
		dbHandle->freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return version;
	}

	version = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	return version;
}

/* reject_msg_support */
msg_error_t MmsPluginStorage::getTrID(MSG_MESSAGE_INFO_S *pMsgInfo, char *pszTrID, int nBufferLen)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT TRANSACTION_ID FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle->columnText(0) != NULL) {
			strncpy(pszTrID, (char *)dbHandle->columnText(0), nBufferLen - 1);
			pszTrID[nBufferLen-1] = '\0';
		}
	} else {
		dbHandle->finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}
/* reject_msg_support */

msg_error_t MmsPluginStorage::getAddressInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN+1];

	/* Add Address */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"A.ADDRESS_TYPE, "
			"A.RECIPIENT_TYPE, "
			"A.CONTACT_ID, "
			"A.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.CONV_ID = B.CONV_ID "
			"AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
			msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pAddrInfo->addressType = dbHandle->columnInt(0);
		pAddrInfo->recipientType = dbHandle->columnInt(1);
		pAddrInfo->contactId = dbHandle->columnInt(2);

		strncpy(pAddrInfo->addressVal, (char*)dbHandle->columnText(3), MAX_ADDRESS_VAL_LEN);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::getMmsRawFilePath(msg_message_id_t msgId, char *pFilepath, size_t filePathLen)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle->columnText(0) != NULL) {
			snprintf(pFilepath, filePathLen, "%s", (char *)dbHandle->columnText(0));
		}
	} else {
		dbHandle->finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


int MmsPluginStorage::searchMsgId(char *toNumber, char *szMsgID)
{
	int msgId = -1;

	msg_folder_id_t	folderId = MSG_SENTBOX_ID;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MmsAddrUtilRemovePlmnString(toNumber);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID FROM %s A, %s B \
			WHERE A.MSG_ID = B.MSG_ID AND A.FOLDER_ID = %d AND B.MESSAGE_ID LIKE '%%%s%%'",
			MSGFW_MESSAGE_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME, folderId, szMsgID);

	MSG_DEBUG("sqlQuery [%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgId = dbHandle->columnInt(0);
	}

	dbHandle->finalizeQuery();

	return msgId;
}

msg_error_t MmsPluginStorage::getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	if (pMmsMsg == NULL)
		return MSG_ERR_NULL_POINTER;

	/* Get the text data from the 1st slide. */
	if (pMmsMsg->pageCnt > 0) {
		pPage = _MsgMmsGetPage(pMmsMsg, 0);

		if (pPage) {
			for (int j = 0; j < pPage->mediaCnt; ++j) {
				pMedia = _MsgMmsGetMedia(pPage, j);

				if (pMedia && pMedia->mediatype == MMS_SMIL_MEDIA_TEXT) {
					MimeType mimeType = MIME_UNKNOWN;
					MsgGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, pMedia->szFilePath, &mimeType, NULL);

					if (mimeType == MIME_TEXT_X_VCALENDAR || mimeType == MIME_TEXT_X_VCARD || mimeType == MIME_TEXT_X_VTODO || mimeType == MIME_TEXT_X_VNOTE) {
						MSG_DEBUG("Media Type is Text, but Vobject file [%s]", pMedia->szFilePath);
					} else {
						strncpy(pMsgText, pMedia->szFilePath, MAX_MSG_TEXT_LEN);
					}
					break;
				}
			}
		}
	}

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertPreviewInfo(int msgId, int type, const char *value, int count)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	/* (MSG_ID INTEGER, TYPE INTEGER, INFO TEXT) */
	snprintf(sqlQuery, sizeof(sqlQuery),
			"INSERT INTO %s "
			"(MSG_ID, TYPE, VALUE, COUNT)"
			"VALUES (%d, %d, \"%s\", %d);",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId, type, value, count);

	MSG_SEC_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::removePreviewInfo(int msgId)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0, };

	/* remove thumbnail file */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT VALUE FROM %s "
			"WHERE MSG_ID = %d AND (TYPE=%d OR TYPE=%d);",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId, MSG_MMS_ITEM_TYPE_IMG, MSG_MMS_ITEM_TYPE_VIDEO);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_PREPARE;
	}

	while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		memset(filePath, 0x00, sizeof(filePath));
		strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
		if (remove(filePath) == -1)
			MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
		else
			MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
	}

	dbHandle->finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	/* (MSG_ID INTEGER, TYPE INTEGER, INFO TEXT) */
	snprintf(sqlQuery, sizeof(sqlQuery),
			"DELETE FROM %s WHERE MSG_ID= %d;",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::deleteMmsMessage(int msgId)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0, };
	char dirPath[MSG_FILEPATH_LEN_MAX + 1]= {0, };

	/* remove DB Preview */
	removePreviewInfo(msgId);

	deleteMultipartList(msgId);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_PREPARE;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);

		snprintf(dirPath, MSG_FILEPATH_LEN_MAX, "%s.dir", filePath);

		/* delete pdu file */
		if (remove(filePath) == -1)
			MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
		else
			MSG_SEC_DEBUG("Success to delete file [%s]", filePath);

		/* delete multipart files */
		MsgRmRf(dirPath);

		/* delete multipart dir */
		rmdir(dirPath);

	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();


	/* Delete Data from MMS table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_EXEC;
	}

	/* Delete Data from MMS STATUS table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
			MSGFW_REPORT_TABLE_NAME, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertMultipart(msg_message_id_t msgId, MMS_MULTIPART_DATA_S *pMultipart)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pMultipart == NULL) {
		MSG_ERR("NULL parameter");
		return MSG_ERR_INVALID_PARAMETER;
	}

#if 0
	/* ( MSG_ID INTEGER NOT NULL , SEQ INTEGER DEFAULT 0, CONTENT_TYPE TEXT, 	NAME TEXT, CHARSET INTEGER,	CONTENT_ID TEXT, CONTENT_LOCATION TEXT,	FILE_PATH TEXT, TEXT TEXT,); */
	if (pMultipart->type == MIME_APPLICATION_SMIL) { /* Smil */
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
				"(MSG_ID, SEQ, CONTENT_TYPE, NAME, CONTENT_ID, CONTENT_LOCATION, TEXT) "
				"VALUES (%d, -1, '%s', '%s', '%s', '%s' , '%s');",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->szContentID, pMultipart->szContentLocation, (char *)pMultipart->data);

	} else if (MmsIsText(pMultipart->type)) { /* Text */
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
				"(MSG_ID, CONTENT_TYPE, NAME, CHARSET, CONTENT_ID, CONTENT_LOCATION, TEXT) "
				"VALUES (%d, '%s', '%s', %d, '%s', '%s' ,'%s');",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->char_set, pMultipart->szContentID, pMultipart->szContentLocation, (char *)pMultipart->data);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
				"(MSG_ID, CONTENT_TYPE, NAME, CONTENT_ID, CONTENT_LOCATION, FILE_PATH) "
				"VALUES (%d, '%s', '%s', '%s', '%s' ,'%s');",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->szContentID, pMultipart->szContentLocation, (char *)pMultipart->szFilePath);
	}
#else

	if (pMultipart->type == MIME_APPLICATION_SMIL) { /* Smil */
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
						"(MSG_ID, SEQ, CONTENT_TYPE, NAME, CONTENT_ID, CONTENT_LOCATION, FILE_PATH) "
						"VALUES (%d, -1, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\");",
						MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->szContentID, pMultipart->szContentLocation, (char *)pMultipart->szFilePath);

		MSG_SEC_DEBUG("QUERY : [%s]", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	} else {
		const char *pszMimeType = NULL;

		if (strlen(pMultipart->szContentType) == 0) {
			MsgGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, pMultipart->szFilePath, &pMultipart->type, &pszMimeType);
			snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", pszMimeType);
			MSG_DEBUG("pMultipart->szContentType = %s", pMultipart->szContentType);
		}

		/* make tcs_bc_level & malware_allow value */
		pMultipart->tcs_bc_level = -1;
		/* CID 41991: pMultipart->szFilePath is an array, hence checking for null is not required */
		if (strlen(pMultipart->szFilePath) > 0 && MsgAccessFile(pMultipart->szFilePath, F_OK) == true) {
			int bc_level = -1;
			int tcs_ret = 0; /* MsgTcsScanFile(pMultipart->szFilePath, &bc_level); */
			if (tcs_ret == 0) {
				if (bc_level > -1) {
					MSG_DEBUG("This content is malware, level = %d", bc_level);
					pMultipart->tcs_bc_level = bc_level;
					pMultipart->malware_allow = 0;
				}
			}
		}

		if (pMultipart->tcs_bc_level < 0 || pMultipart->malware_allow) {
			if (g_str_has_prefix(pMultipart->szContentType, "video") || g_str_has_prefix(pMultipart->szContentType, "image")) {
				char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };

				if (MmsMakeMultipartThumbnailInfo(pMultipart, thumbPath) == MSG_SUCCESS)
					snprintf(pMultipart->szThumbFilePath, sizeof(pMultipart->szThumbFilePath), "%s", thumbPath);
			}
		}

		if (!g_strcmp0(pMultipart->szContentType, "text/plain")) {
			gchar *contents = NULL;
			gsize length = 0;

			if (MsgAccessFile(pMultipart->szFilePath, F_OK))
				g_file_get_contents((gchar*)pMultipart->szFilePath, (gchar**)&contents, (gsize*)&length, NULL);

			if (contents) {
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
							"(MSG_ID, CONTENT_TYPE, NAME, CONTENT_ID, CONTENT_LOCATION, FILE_PATH, TCS_LEVEL, MALWARE_ALLOW, TEXT) "
							"VALUES (%d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", '%d', '%d', ?);",
							MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->szContentID, pMultipart->szContentLocation, pMultipart->szFilePath, pMultipart->tcs_bc_level, pMultipart->malware_allow);

				MSG_SEC_DEBUG("QUERY : [%s]", sqlQuery);

				if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
					g_free(contents);
					return MSG_ERR_DB_EXEC;
				}

				dbHandle->bindText(contents, 1);

				if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
					dbHandle->finalizeQuery();
					g_free(contents);
					return MSG_ERR_DB_EXEC;
				}

				dbHandle->finalizeQuery();
				g_free(contents);
			} else {
				MSG_ERR("file contents is null!");
			}
		} else {
			snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
						"(MSG_ID, CONTENT_TYPE, NAME, CONTENT_ID, CONTENT_LOCATION, FILE_PATH, TCS_LEVEL, MALWARE_ALLOW, THUMB_FILE_PATH) "
						"VALUES (%d, \"%s\", \"%s\", \"%s\", \"%s\" ,\"%s\", '%d', '%d', \"%s\");",
						MSGFW_MMS_MULTIPART_TABLE_NAME, msgId, pMultipart->szContentType, pMultipart->szFileName, pMultipart->szContentID, pMultipart->szContentLocation, pMultipart->szFilePath, pMultipart->tcs_bc_level, pMultipart->malware_allow, pMultipart->szThumbFilePath);

			MSG_SEC_DEBUG("QUERY : [%s]", sqlQuery);

			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
				return MSG_ERR_DB_EXEC;
		}
	}
#endif

	MSG_END();
	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::updateMultipart(msg_message_id_t msgId, int allow_malware, MMS_MULTIPART_DATA_S *pMultipart)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pMultipart == NULL) {
		MSG_ERR("NULL parameter");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pMultipart->type != MIME_APPLICATION_SMIL) {
		/* make tcs_bc_level & malware_allow value */
		pMultipart->malware_allow = allow_malware;

		if (pMultipart->tcs_bc_level >= 0 || pMultipart->malware_allow) {
			if (!strcasecmp(pMultipart->szContentType, "video") || !strcasecmp(pMultipart->szContentType, "image")) {
				char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };

				if (MmsMakeMultipartThumbnailInfo(pMultipart, thumbPath) == MSG_SUCCESS)
					snprintf(pMultipart->szThumbFilePath, sizeof(pMultipart->szThumbFilePath), "%s", thumbPath);
			}
		}

		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MALWARE_ALLOW = %d, THUMB_FILE_PATH = '%s' WHERE MSG_ID = %d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, pMultipart->malware_allow, (char *)pMultipart->szThumbFilePath, msgId);

		MSG_DEBUG("QUERY : [%s]", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	MSG_END();
	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::deleteMultipartList(int msgId)
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
/*	char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0,}; */
	const char *filePath = NULL;
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_PREPARE;
	}

	while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		filePath = (const char *)dbHandle->columnText(0);
		if (filePath) {
			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
		}
	}

	dbHandle->finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT THUMB_FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_PREPARE;
	}

	while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		filePath = (const char *)dbHandle->columnText(0);
		if (filePath) {
			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
		}
	}

	dbHandle->finalizeQuery();

	/* Delete Data from Multipart table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();
	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::getMultipartList(msg_message_id_t msgId, MMSList **multipart_list)
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];
	int rowCnt = 0, index = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT CONTENT_TYPE, NAME, FILE_PATH, CONTENT_ID, CONTENT_LOCATION, TCS_LEVEL, MALWARE_ALLOW, THUMB_FILE_PATH "
			"FROM %s WHERE MSG_ID=%d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

	msg_error_t err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_SUCCESS) {
		for (int i = 0; i < rowCnt; i++) {
			MMS_MULTIPART_DATA_S *multipart = MsgMmsCreateMultipart();

			dbHandle->getColumnToString(index++, sizeof(multipart->szContentType), multipart->szContentType);

			dbHandle->getColumnToString(index++, sizeof(multipart->szFileName), multipart->szFileName);

			dbHandle->getColumnToString(index++, sizeof(multipart->szFilePath), multipart->szFilePath);

			dbHandle->getColumnToString(index++, sizeof(multipart->szContentID), multipart->szContentID);

			dbHandle->getColumnToString(index++, sizeof(multipart->szContentLocation), multipart->szContentLocation);

			multipart->tcs_bc_level = dbHandle->getColumnToInt(index++);

			multipart->malware_allow = dbHandle->getColumnToInt(index++);

			dbHandle->getColumnToString(index++, sizeof(multipart->szThumbFilePath), multipart->szThumbFilePath);

			multipart->type = MimeGetMimeIntFromMimeString(multipart->szContentType);

			*multipart_list = g_list_append(*multipart_list, multipart);
		}
	}

	dbHandle->freeTable();

	MSG_END();
	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::addMmsData(msg_message_id_t msgId, const char *raw_filepath, MMS_DATA_S *pMmsData)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_HEADER_DATA_S *pHeader = pMmsData->header;

	if (pHeader == NULL) {
		MSG_ERR("Header is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery)
			, "INSERT INTO %s "
			"(MSG_ID, TRANSACTION_ID, MESSAGE_ID, FWD_MESSAGE_ID, CONTENTS_LOCATION"
			", FILE_PATH, VERSION, DATA_TYPE, DATE, HIDE_ADDRESS"
			", ASK_DELIVERY_REPORT, REPORT_ALLOWED, READ_REPORT_ALLOWED_TYPE"
			", ASK_READ_REPLY, READ, READ_REPORT_SEND_STATUS, READ_REPORT_SENT"
			", PRIORITY, KEEP_COPY, MSG_SIZE, MSG_CLASS"
			", EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME, MSG_STATUS) "
			"VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);"
			, MMS_PLUGIN_MESSAGE_TABLE_NAME
			, msgId, pHeader->trID, pHeader->messageID, "", pHeader->contentLocation
			, raw_filepath, pHeader->mmsVersion, MMS_DATATYPE_NONE, pHeader->date, pHeader->bHideAddress
			, pHeader->bDeliveryReport, 0 /*pMmsMsg->mmsAttrib.bReportAllowed*/, 0 /*pMmsMsg->mmsAttrib.readReportAllowedType*/
			, pHeader->bReadReport, 0 /*pMmsMsg->mmsAttrib.bRead*/, 0 /*pMmsMsg->mmsAttrib.readReportSendStatus*/, 0 /*pMmsMsg->mmsAttrib.bReadReportSent*/
			, pHeader->mmsPriority, true /*pMmsMsg->mmsAttrib.bLeaveCopy*/, pHeader->messageSize, pHeader->messageClass
			, pHeader->expiry.time, 0, pHeader->delivery.time, pHeader->mmsStatus);

	msg_error_t db_err = dbHandle->execQuery(sqlQuery);
	if (db_err != MSG_SUCCESS) {
		MSG_SEC_ERR("execute query fail [%s], err = [%d]", sqlQuery, db_err);
		return MSG_ERR_DB_EXEC;
	} else {
		MSG_SEC_DEBUG("execute query success [%s]", sqlQuery);
	}

	MMS_MULTIPART_DATA_S *smil = pMmsData->smil;
	if (smil) {
		MSG_DEBUG("insert smil multipart to db");
		smil->type = MIME_APPLICATION_SMIL;
		this->insertMultipart(msgId, smil);
	}

	MMSList *multipart_list = pMmsData->multipartlist;
	if (multipart_list) {
		MSG_DEBUG("insert multipart to db");

		for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
			MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);

			if (pMultipart) {
				this->insertMultipart(msgId, pMultipart);
			}
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::getMmsData(msg_message_id_t msgId, MMS_DATA_S *pMmsData)
{
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_HEADER_DATA_S *pHeader = pMmsData->header;

	if (pHeader == NULL) {
		MSG_ERR("Header is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery)
			, "SELECT "
			"VERSION, DATA_TYPE,  DATE, HIDE_ADDRESS, ASK_DELIVERY_REPORT, REPORT_ALLOWED"
			", READ_REPORT_ALLOWED_TYPE, ASK_READ_REPLY, READ, READ_REPORT_SEND_STATUS, READ_REPORT_SENT"
			", PRIORITY,  MSG_SIZE, MSG_CLASS, EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME, MSG_STATUS"
			", MESSAGE_ID, TRANSACTION_ID, CONTENTS_LOCATION, FILE_PATH "
			"FROM %s "
			"WHERE MSG_ID = %d;"
			, MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	msg_error_t db_err = dbHandle->prepareQuery(sqlQuery);

	if (db_err != MSG_SUCCESS) {
		MSG_ERR("prepare query fail [%s], err = [%d]", sqlQuery, db_err);
		return MSG_ERR_PLUGIN_STORAGE;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		int i = 0;
		pHeader->mmsVersion = dbHandle->columnInt(i++);
		i++; /* pMmsMsg->mmsAttrib.dataType = (MmsDataType)dbHandle->columnInt(i++); */
		pHeader->date = dbHandle->columnInt(i++);
		pHeader->bHideAddress = dbHandle->columnInt(i++);
		pHeader->bDeliveryReport = dbHandle->columnInt(i++);
		i++; /* pMmsMsg->mmsAttrib.bReportAllowed = dbHandle->columnInt(i++); */
		i++; /* pMmsMsg->mmsAttrib.readReportAllowedType = (MmsRecvReadReportType)dbHandle->columnInt(i++); */
		pHeader->bReadReport = dbHandle->columnInt(i++);
		i++; /* pMmsMsg->mmsAttrib.bRead = dbHandle->columnInt(i++); */
		i++; /* pMmsMsg->mmsAttrib.readReportSendStatus = (MmsRecvReadReportSendStatus)dbHandle->columnInt(i++); */
		i++; /* pMmsMsg->mmsAttrib.bReadReportSent = dbHandle->columnInt(i++); */
		pHeader->mmsPriority = (MmsPriority)dbHandle->columnInt(i++);
		pHeader->messageSize = dbHandle->columnInt(i++);
		pHeader->messageClass = (MmsMsgClass)dbHandle->columnInt(i++);
		pHeader->expiry.time = dbHandle->columnInt(i++);
		i++; /* CUSTOM_DELIVERY_TIME */
		pHeader->delivery.time = dbHandle->columnInt(i++);
		i++; /* pMmsMsg->mmsAttrib.msgStatus = (msg_delivery_report_status_t)dbHandle->columnInt(i++); */
		snprintf(pHeader->messageID, sizeof(pHeader->messageID), "%s", dbHandle->columnText(i++));
		snprintf(pHeader->trID, sizeof(pHeader->trID), "%s", dbHandle->columnText(i++));
		snprintf(pHeader->contentLocation, sizeof(pHeader->contentLocation), "%s", dbHandle->columnText(i++));
		i++; /* snprintf(pMmsMsg->szFileName, sizeof(pMmsMsg->szFileName), "%s", dbHandle->columnText(i++)); */
	}

	dbHandle->finalizeQuery();


	MMSList *pMultipartList = NULL;

	if (this->getMultipartList(msgId, &pMultipartList) == MSG_SUCCESS) {
		if (pMultipartList) {
			MMS_MULTIPART_DATA_S *smil_multipart = NULL;
			char *content_type = NULL;

			MsgMmsGetSmilMultipart(pMultipartList, &smil_multipart);

			if (smil_multipart) {
				pMmsData->smil = smil_multipart ;
				pMultipartList = g_list_remove(pMultipartList, smil_multipart);
				pHeader->contentType = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;

				if (MsgAccessFile(smil_multipart->szFilePath, F_OK)) {
					gchar *contents = NULL;
					gsize length = 0;

					g_file_get_contents((gchar*)smil_multipart->szFilePath, (gchar**)&contents, (gsize*)&length, NULL);

					smil_multipart->pMultipartData = contents;
					smil_multipart->nMultipartDataLen = length;

					memset(smil_multipart->szFilePath, 0x00, sizeof(smil_multipart->szFilePath));
				}

			} else {
				pHeader->contentType = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
			}

			content_type = MimeGetMimeStringFromMimeInt(pHeader->contentType);
			if (content_type) {
				snprintf(pHeader->szContentType, sizeof(pHeader->szContentType), "%s", content_type);
			}

			pMmsData->multipartlist = pMultipartList;
		}
	}

	MSG_END();
	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pSerializedMms)
{ /* send request or notification ind */
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;

	MMS_DATA_S *pMmsData = NULL;

	if (MsgDeserializeMmsData(pSerializedMms, strlen(pSerializedMms), &pMmsData) != 0) {
		MSG_DEBUG("Fail to Deserialize Message Data");
		goto __CATCH;
	}

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS || pMsgInfo->msgType.subType == MSG_SENDCONF_MMS) {
		char raw_filepath[MSG_FILENAME_LEN_MAX+1] = {0, };
		char working_dir[MSG_FILENAME_LEN_MAX+1] = {0, };

		/* compose */
		MmsPluginComposer::instance()->composeSendReq(pMsgInfo, pSendOptInfo, pMmsData);

		/* encode */
		snprintf(working_dir, sizeof(working_dir), MSG_DATA_PATH"%d.mms.dir", pMsgInfo->msgId);

		MsgMmsSetMultipartListData(pMmsData); /* app file -> data */

		MsgMmsSetMultipartListFilePath(working_dir, pMmsData); /* data -> svc file */

		snprintf(raw_filepath, sizeof(raw_filepath), "%s%d.mms", MSG_DATA_PATH, pMsgInfo->msgId);

		MmsPluginEncoder::instance()->encodeMmsPdu(pMmsData, pMsgInfo->msgId, raw_filepath);

		/* add to db */
		if (addMmsData(pMsgInfo->msgId, raw_filepath, pMmsData) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to add db message");
			goto __CATCH;
		}

		/* set */
		if (MsgGetFileSize(raw_filepath, (int *)&pMsgInfo->dataSize) == false) {
			MSG_SEC_DEBUG("Fail to get mms file size [%s]", raw_filepath);
			goto __CATCH;
		}

		/* make Preview info for APP */
		MmsPluginAppBase appBase(pMmsData);
		appBase.makePreviewInfo(pMsgInfo->msgId, true, raw_filepath);
		appBase.getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));

	} else if (pMsgInfo->msgType.subType == MSG_RETRIEVE_MMS
			|| pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS
			|| pMsgInfo->msgType.subType ==MSG_RETRIEVE_MANUALCONF_MMS) {
		char raw_filepath[MSG_FILENAME_LEN_MAX+1] = {0, };
		char working_dir[MSG_FILENAME_LEN_MAX+1] = {0, };

		/* compose */
		MmsPluginComposer::instance()->composeRetrieveConf(pMsgInfo, pSendOptInfo, pMmsData);

		/* encode */
		snprintf(working_dir, sizeof(working_dir), MSG_DATA_PATH"%d.mms.dir", pMsgInfo->msgId);

		MsgMmsSetMultipartListData(pMmsData); /* app file -> data */

		MsgMmsSetMultipartListFilePath(working_dir, pMmsData); /* data -> svc file */

		snprintf(raw_filepath, sizeof(raw_filepath), "%s%d.mms", MSG_DATA_PATH, pMsgInfo->msgId);

		MmsPluginEncoder::instance()->encodeMmsPdu(pMmsData, pMsgInfo->msgId, raw_filepath);

		/* add to db */
		if (addMmsData(pMsgInfo->msgId, raw_filepath, pMmsData) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to add db message");
			goto __CATCH;
		}

		/* set */
		if (MsgGetFileSize(raw_filepath, (int *)&pMsgInfo->dataSize) == false) {
			MSG_SEC_DEBUG("Fail to get mms file size [%s]", raw_filepath);
			goto __CATCH;
		}

		MmsPluginAppBase *appBase;
		appBase = new MmsPluginAppBase(pMmsData);
		appBase->makePreviewInfo(pMsgInfo->msgId, true, raw_filepath);
		appBase->getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));
		delete appBase;

	} else if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		if (pMmsData->header == NULL || pMmsData->header->messageType != MMS_MSGTYPE_NOTIFICATION_IND) {
			goto __CATCH;
		}

		if (addMmsData(pMsgInfo->msgId, "", pMmsData) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to add db message");
			goto __CATCH;
		}

	} else {
		MSG_DEBUG("Not support msg sub type [%d]", pMsgInfo->msgType.subType);
		goto __CATCH;
	}

	MsgMmsRelease(&pMmsData);
	MSG_END();
	return MSG_SUCCESS;

__CATCH:
	MsgMmsRelease(&pMmsData);
	deleteMmsMessage(pMsgInfo->msgId);

	return err;
}

msg_error_t MmsPluginStorage::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pSerializedMms)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_DATA_S *pMmsData = NULL;

	/* CID 12917: strlen will give exception in case of NULL, hence the check of pSerializedMms */
	if (pSerializedMms == NULL || MsgDeserializeMmsData(pSerializedMms, strlen(pSerializedMms), &pMmsData) != 0) {
		MSG_DEBUG("Fail to Deserialize Message Data");
		goto __CATCH;
	}

	MSG_DEBUG("msg sub type = [%d]", pMsgInfo->msgType.subType);

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS) {
		char raw_filepath[MSG_FILENAME_LEN_MAX+1] = {0, };
		char working_dir[MSG_FILENAME_LEN_MAX+1] = {0, };

		/* compose */
		MmsPluginComposer::instance()->composeSendReq(pMsgInfo, pSendOptInfo, pMmsData);

		/* encode */
		snprintf(raw_filepath, sizeof(raw_filepath), "%s%d.mms", MSG_DATA_PATH, pMsgInfo->msgId);

		MmsPluginEncoder::instance()->encodeMmsPdu(pMmsData, pMsgInfo->msgId, raw_filepath);

		/* add to db */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, KEEP_COPY = %d, ASK_READ_REPLY = %d, EXPIRY_TIME = %d, CUSTOM_DELIVERY_TIME = %d, DELIVERY_TIME = %d, PRIORITY = %d \
				WHERE MSG_ID = %d;", MMS_PLUGIN_MESSAGE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->bKeepCopy, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
				pSendOptInfo->option.mmsSendOptInfo.expiryTime.time, pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime, pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time,
				pSendOptInfo->option.mmsSendOptInfo.priority, pMsgInfo->msgId);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			/* CID 41997: Releasing pMmsDta in case of failure */
			MsgMmsRelease(&pMmsData);
			return MSG_ERR_DB_EXEC;
		}

		snprintf(working_dir, sizeof(working_dir), MSG_DATA_PATH"%d.mms.dir", pMsgInfo->msgId);

		MsgMmsSetMultipartListData(pMmsData); /* app file -> data */

		MmsPluginStorage::instance()->deleteMultipartList(pMsgInfo->msgId); /* remove exist multipart */

		MsgMmsSetMultipartListFilePath(working_dir, pMmsData); /* data -> svc file */

		/* add multipart list */
		if (pMmsData->smil) {
			pMmsData->smil->type = MIME_APPLICATION_SMIL;
			insertMultipart(pMsgInfo->msgId, pMmsData->smil);
		}

		MMSList *multipart_list = pMmsData->multipartlist;

		if (multipart_list) {
			for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
				MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);

				if (pMultipart) {
					insertMultipart(pMsgInfo->msgId, pMultipart);
				}
			}
		}

		/* set */
		if (MsgGetFileSize(raw_filepath, (int *)&pMsgInfo->dataSize) == false) {
			MSG_SEC_DEBUG("Fail to get mms file size [%s]", raw_filepath);
			/* CID 41997: Releasing pMmsDta in case of failure */
			MsgMmsRelease(&pMmsData);
			goto __CATCH;
		}

		/* make Preview info for APP */
		MmsPluginAppBase *appBase;
		appBase = new MmsPluginAppBase(pMmsData);
		appBase->makePreviewInfo(pMsgInfo->msgId, true, raw_filepath);
		appBase->getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));
		delete appBase;
	}

	MsgMmsRelease(&pMmsData);
	MSG_END();

	return err;

__CATCH:
	removePreviewInfo(pMsgInfo->msgId);

	THROW(MsgException::MMS_PLG_ERROR, "MMS add Error");
}

msg_error_t MmsPluginStorage::getMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char **pSerializedMms)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;

	MMS_DATA_S *pMmsData = MsgMmsCreate();

	if (pMmsData == NULL) {
		MSG_ERR("MsgMmsCreate Fail");
		return MSG_ERR_NULL_POINTER;
	}

	MMS_HEADER_DATA_S *pHeader = MsgMmsCreateHeader();

	if (pHeader == NULL) {
		MsgMmsRelease(&pMmsData);
		MSG_ERR("MsgMmsCreateHeader Fail");
		return MSG_ERR_NULL_POINTER;
	}

	pMmsData->header = pHeader;

	switch(pMsg->msgType.subType) {
	case MSG_SENDREQ_MMS:
		pHeader->messageType = MMS_MSGTYPE_SEND_REQ;
		break;
	case MSG_SENDCONF_MMS:
		pHeader->messageType = MMS_MSGTYPE_SEND_CONF;
		break;
	case MSG_RETRIEVE_MMS:
		pHeader->messageType = MMS_MSGTYPE_RETRIEVE_CONF;
		break;
	case MSG_RETRIEVE_AUTOCONF_MMS:
		pHeader->messageType = MMS_MSGTYPE_RETRIEVE_CONF;
		break;
	case MSG_RETRIEVE_MANUALCONF_MMS:
		pHeader->messageType = MMS_MSGTYPE_RETRIEVE_CONF;
		break;
	case MSG_NOTIFICATIONIND_MMS:
		pHeader->messageType = MMS_MSGTYPE_NOTIFICATION_IND;
		break;
	default:
		break;
	}

	err = getMmsData(pMsg->msgId, pMmsData); /* get MmsData Info from DB */

	if (err == MSG_SUCCESS) {
		if (pSendOptInfo) {
			pSendOptInfo->bDeliverReq = pHeader->bDeliveryReport;
			pSendOptInfo->option.mmsSendOptInfo.bReadReq = pHeader->bReadReport;
			pSendOptInfo->option.mmsSendOptInfo.priority = pHeader->mmsPriority;
			pSendOptInfo->option.mmsSendOptInfo.expiryTime.type = pHeader->expiry.type;
			pSendOptInfo->option.mmsSendOptInfo.expiryTime.time = pHeader->expiry.time;
			pSendOptInfo->option.mmsSendOptInfo.deliveryTime.type = pHeader->delivery.type;
			pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = pHeader->delivery.time;
		}

		/* MsgMmsSetMultipartListData(pMmsData); */
		int mmsDataSize = MsgSerializeMms(pMmsData, pSerializedMms);
		if (mmsDataSize > 0) {
			pMsg->dataSize = mmsDataSize;
		} else {
			MSG_ERR("MsgSerializeMms fail");
		}

	} else {
		MSG_ERR("getMmsData fail, err = [%d]",  err);
	}

	/* Release pMmsData */
	MsgMmsRelease(&pMmsData);
	MSG_END();

	return err;
}

msg_error_t MmsPluginStorage::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	/* update multipart list */
	MMSList *multipart_list = NULL;
	if (MmsPluginStorage::instance()->getMultipartList(pMsgInfo->msgId, &multipart_list) != MSG_SUCCESS)
		return -1;

	for (int i = 0; i < (int)g_list_length(multipart_list); i++) {
		MMS_MULTIPART_DATA_S *pMultipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(multipart_list, i);
		MmsPluginStorage::instance()->updateMultipart(pMsgInfo->msgId, true, pMultipart);
	}

	/* update preview */
	char szFullPath[MSG_FILEPATH_LEN_MAX] = {0, };
	MmsMsg *mmsMsg = NULL;
	unique_ptr<MmsMsg*, void(*)(MmsMsg**)> buf(&mmsMsg, unique_ptr_deleter);
	mmsMsg = (MmsMsg *)new char[sizeof(MmsMsg)];
	memset(mmsMsg, 0x00, sizeof(MmsMsg));

	MmsPluginStorage::instance()->getMmsRawFilePath(pMsgInfo->msgId, szFullPath, sizeof(szFullPath));
	MmsPluginDecoder::instance()->decodeMmsPdu(mmsMsg, pMsgInfo->msgId, szFullPath);

	/* make Preview info for APP */
	MmsPluginAppBase *appBase;
	appBase = new MmsPluginAppBase(mmsMsg);
	appBase->makePreviewInfo(pMsgInfo->msgId, true, szFullPath);
	appBase->getFirstPageTextFilePath(pMsgInfo->msgText, sizeof(pMsgInfo->msgText));
	delete appBase;

	MmsReleaseMmsMsg(mmsMsg);

	MSG_END();
	return 0;
}

int MmsPluginStorage::checkDuplicateNotification(char* pszTrID, char* pszContentLocation)
{
	MSG_BEGIN();

	if (!pszTrID || strlen(pszTrID) == 0)
		return 0;

	if (!pszContentLocation || strlen(pszContentLocation) == 0)
		return 0;

	MSG_SEC_DEBUG("Trans Id = %s, Content Loc = %s", pszTrID, pszContentLocation);

	int msgId = 0;
	msg_error_t err = MSG_SUCCESS;
	int rowCnt = 0;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s \
			WHERE TRANSACTION_ID LIKE '%s' AND CONTENTS_LOCATION LIKE '%s'",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pszTrID, pszContentLocation);

	MSG_SEC_DEBUG("sqlQuery [%s]", sqlQuery);

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return 0;
	}

	if (rowCnt > 0)
		msgId = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	MSG_END();

	return msgId;
}

