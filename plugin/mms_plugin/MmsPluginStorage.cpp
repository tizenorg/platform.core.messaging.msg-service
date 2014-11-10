/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgMmsMessage.h"
#include "MsgStorageTypes.h"
#include "MmsPluginDebug.h"
#include "MmsPluginStorage.h"
#include "MmsPluginMessage.h"
#include "MmsPluginSmil.h"
#include "MmsPluginDrm.h"
#include "MmsPluginMIME.h"

#define MMS_FREE(obj)\
	if (obj){\
		free(obj);\
		obj = NULL;\
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

void MmsPluginStorage::composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	FILE *pFile = NULL;

	msg_read_report_status_t readStatus;
	int	version;

	memcpy(&readStatus, pMsgInfo->msgData, sizeof(msg_read_report_status_t));

	MSG_DEBUG("pMsgInfo->msgId = %d", pMsgInfo->msgId);

	version = MmsPluginStorage::instance()->getMmsVersion(pMsgInfo->msgId);

	snprintf((char *)pMsgInfo->msgData, MAX_MSG_DATA_LEN+1, "%s/%d-Read-Rec.ind", MSG_DATA_PATH, pMsgInfo->msgId);

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


msg_error_t MmsPluginStorage::addMmsMsgToDB(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, int attachCnt)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	// Add Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsMsg->msgID, pMmsMsg->szTrID, pMmsMsg->szMsgID, pMmsMsg->szForwardMsgID, pMmsMsg->szContentLocation,
			pMsgInfo->msgData, pMmsMsg->mmsAttrib.version, pMmsMsg->mmsAttrib.dataType, pMmsMsg->mmsAttrib.date, pMmsMsg->mmsAttrib.bHideAddress,
			pMmsMsg->mmsAttrib.bAskDeliveryReport, pMmsMsg->mmsAttrib.bReportAllowed, pMmsMsg->mmsAttrib.readReportAllowedType,
			pMmsMsg->mmsAttrib.bAskReadReply, pMmsMsg->mmsAttrib.bRead, pMmsMsg->mmsAttrib.readReportSendStatus, pMmsMsg->mmsAttrib.bReadReportSent,
			pMmsMsg->mmsAttrib.priority, pMmsMsg->mmsAttrib.bLeaveCopy, pMmsMsg->mmsAttrib.msgSize, pMmsMsg->mmsAttrib.msgClass,
			pMmsMsg->mmsAttrib.expiryTime.time,	pMmsMsg->mmsAttrib.bUseDeliveryCustomTime, pMmsMsg->mmsAttrib.deliveryTime.time, pMmsMsg->mmsAttrib.msgStatus);

	MSG_DEBUG("\n!!!!!!!!! QUERY : %s\n", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	if (updateMmsAttachCount(pMmsMsg->msgID, attachCnt) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::addMmsMsgToDB(MmsMsg *pMmsMsg, const char *raw_filepath)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	// Add Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, '%s', '%s', '%s', '%s', '%s', %d, %d, %ld, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsMsg->msgID, pMmsMsg->szTrID, pMmsMsg->szMsgID, pMmsMsg->szForwardMsgID, pMmsMsg->szContentLocation,
			raw_filepath, pMmsMsg->mmsAttrib.version, pMmsMsg->mmsAttrib.dataType, pMmsMsg->mmsAttrib.date, pMmsMsg->mmsAttrib.bHideAddress,
			pMmsMsg->mmsAttrib.bAskDeliveryReport, pMmsMsg->mmsAttrib.bReportAllowed, pMmsMsg->mmsAttrib.readReportAllowedType,
			pMmsMsg->mmsAttrib.bAskReadReply, pMmsMsg->mmsAttrib.bRead, pMmsMsg->mmsAttrib.readReportSendStatus, pMmsMsg->mmsAttrib.bReadReportSent,
			pMmsMsg->mmsAttrib.priority, pMmsMsg->mmsAttrib.bLeaveCopy, pMmsMsg->mmsAttrib.msgSize, pMmsMsg->mmsAttrib.msgClass,
			pMmsMsg->mmsAttrib.expiryTime.time,	pMmsMsg->mmsAttrib.bUseDeliveryCustomTime, pMmsMsg->mmsAttrib.deliveryTime.time, pMmsMsg->mmsAttrib.msgStatus);

	MSG_DEBUG("\n!!!!!!!!! QUERY : %s\n", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN + 1];
	MmsMsg mmsMsg;
	MMS_MESSAGE_DATA_S mmsMsgData = {0,};
	char raw_filepath[MSG_FILENAME_LEN_MAX+1] = {0,};
	char raw_filedir[MSG_FILENAME_LEN_MAX+1] = {0,};
	bzero(&mmsMsg, sizeof(mmsMsg));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, KEEP_COPY = %d, ASK_READ_REPLY = %d, EXPIRY_TIME = %d, CUSTOM_DELIVERY_TIME = %d, DELIVERY_TIME= %d, PRIORITY = %d \
			WHERE MSG_ID = %d;", MMS_PLUGIN_MESSAGE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->bKeepCopy, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
			pSendOptInfo->option.mmsSendOptInfo.expiryTime.time, pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime, pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time,
			pSendOptInfo->option.mmsSendOptInfo.priority, pMsgInfo->msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_DEBUG("msg sub type = [%d]", pMsgInfo->msgType.subType);

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS) {

		if (_MsgMmsDeserializeMessageData(&mmsMsgData, pFileData) == false) {
			MSG_DEBUG("Fail to Deserialize Message Data");
			THROW(MsgException::MMS_PLG_ERROR, "MMS Message Compose Error");
		}

		if (MmsComposeSendReq(&mmsMsg, pMsgInfo, pSendOptInfo, &mmsMsgData) != true) {
			MmsReleaseMmsMsg(&mmsMsg);
			MsgMmsReleaseMmsLists(&mmsMsgData);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Message MmsComposeSendReq Error");
		}

		//mms file
		snprintf(raw_filepath, sizeof(raw_filepath), "%s/%d.mms", MSG_DATA_PATH, pMsgInfo->msgId);

		//encode mms
		if (MmsEncodeMmsMessage(&mmsMsg, raw_filepath) == false) {
			MSG_DEBUG("Fail to Encode Message");
			MmsReleaseMmsMsg(&mmsMsg);
			MsgMmsReleaseMmsLists(&mmsMsgData);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Message Encode Error");
		}

		//preview data
		MmsPluginStorage::instance()->removePreviewInfo(pMsgInfo->msgId); //remove exist previnfo

		err = MmsMakePreviewInfo(pMsgInfo->msgId, &mmsMsgData);

		err = getMsgText(&mmsMsgData, pMsgInfo->msgText);

		if (mmsMsgData.attachCnt > 0) {
			if (updateMmsAttachCount(mmsMsg.msgID, mmsMsgData.attachCnt) != MSG_SUCCESS) {
				MSG_DEBUG("Fail to updateMmsAttachCount");
			}
		}

		snprintf(raw_filedir, sizeof(raw_filedir), "%s/%d.mms.dir", MSG_DATA_PATH, pMsgInfo->msgId);
		MsgRmRf(raw_filedir);
		rmdir(raw_filedir);

		int size = 0;

		if (MsgGetFileSize(raw_filepath, &size) == false) {
			MmsReleaseMmsMsg(&mmsMsg);
			MsgMmsReleaseMmsLists(&mmsMsgData);
			THROW(MsgException::MMS_PLG_ERROR, "MMS Message MsgGetFileSize Error");
		}

		pMsgInfo->dataSize = size;

		MmsReadMsgBody(pMsgInfo->msgId, true, false, NULL);
	}

	MmsReleaseMmsMsg(&mmsMsg);
	MsgMmsReleaseMmsLists(&mmsMsgData);

	MSG_END();

	return err;
}


msg_error_t MmsPluginStorage::updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	MSG_DEBUG("###### pMsgInfo->msgData = %s #######", pMmsRecvData->retrievedFilePath);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s', FILE_PATH = '%s' WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID, pMmsRecvData->retrievedFilePath, pMsgInfo->msgId);

	MSG_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MMS_RECV_DATA_S *pMmsRecvData = (MMS_RECV_DATA_S *)pMsgInfo->msgData;

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MESSAGE_ID = '%s' WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMmsRecvData->szMsgID, pMsgInfo->msgId);

	MSG_DEBUG("SQLQuery = %s", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	if (pSendOptInfo != NULL) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ASK_DELIVERY_REPORT = %d, ASK_READ_REPLY = %d, PRIORITY = %d, EXPIRY_TIME = %d \
				WHERE MSG_ID = %d;", MMS_PLUGIN_MESSAGE_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->option.mmsSendOptInfo.bReadReq,
				pSendOptInfo->option.mmsSendOptInfo.priority, pSendOptInfo->option.mmsSendOptInfo.expiryTime.time, pMsgInfo->msgId);

		MSG_DEBUG("SQLQuery = %s", sqlQuery);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			return MSG_ERR_DB_EXEC;
		}
	}

	dbHandle.finalizeQuery();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::updateNetStatus(msg_message_id_t msgId, msg_network_status_t netStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, netStatus, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertDeliveryReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	//( MSG_ID INTEGER , ADDRESS_VAL TEXT , STATUS_TYPE INTEGER , STATUS INTEGER DEFAULT 0 , TIME DATETIME);
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
			"(MSG_ID, ADDRESS_VAL, STATUS_TYPE, STATUS, TIME) "
			"VALUES (%d, '%s', %d, %d, %d);",
			MSGFW_REPORT_TABLE_NAME, msgId, address, MSG_REPORT_TYPE_DELIVERY, pStatus->msgStatus, (int)pStatus->handledTime);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::insertReadReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	//( MSG_ID INTEGER , ADDRESS_VAL TEXT , STATUS_TYPE INTEGER , STATUS INTEGER DEFAULT 0 , TIME DATETIME);
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s "
			"(MSG_ID, ADDRESS_VAL, STATUS_TYPE, STATUS, TIME) "
			"VALUES (%d, '%s', %d, %d, %d);",
			MSGFW_REPORT_TABLE_NAME, msgId, address, MSG_REPORT_TYPE_READ, pStatus->readStatus, (int)pStatus->readTime);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::updateMmsAttrib(msg_message_id_t msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType)
{
	MSG_BEGIN();

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

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::updateMmsAttachCount(msg_message_id_t msgId, int count)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ATTACHMENT_COUNT = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, count, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to execute query [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}

void MmsPluginStorage::getMmsFromDB(msg_message_id_t msgId, MmsMsg *pMmsMsg)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION, DATA_TYPE,  DATE, HIDE_ADDRESS, ASK_DELIVERY_REPORT, REPORT_ALLOWED, \
			READ_REPORT_ALLOWED_TYPE, ASK_READ_REPLY, READ, READ_REPORT_SEND_STATUS, READ_REPORT_SENT, PRIORITY, \
			MSG_SIZE, MSG_CLASS, EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME, MSG_STATUS, \
			MESSAGE_ID, TRANSACTION_ID, CONTENTS_LOCATION, FILE_PATH \
			FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		MSG_DEBUG("MSG_ERR_DB_PREPARE");

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		int i = 0;
		pMmsMsg->mmsAttrib.version = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.dataType = (MmsDataType)dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.date = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bHideAddress = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bAskDeliveryReport = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bReportAllowed = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.readReportAllowedType = (MmsRecvReadReportType)dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bAskReadReply = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bRead = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.readReportSendStatus = (MmsRecvReadReportSendStatus)dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.bReadReportSent = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.priority = (MmsPriority)dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.msgSize = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.expiryTime.time = dbHandle.columnInt(i++);
		i++;//CUSTOM_DELIVERY_TIME
		pMmsMsg->mmsAttrib.deliveryTime.time = dbHandle.columnInt(i++);
		pMmsMsg->mmsAttrib.msgStatus = (msg_delivery_report_status_t)dbHandle.columnInt(i++);
		snprintf(pMmsMsg->szMsgID, sizeof(pMmsMsg->szMsgID), "%s", dbHandle.columnText(i++));
		snprintf(pMmsMsg->szTrID, sizeof(pMmsMsg->szTrID), "%s", dbHandle.columnText(i++));
		snprintf(pMmsMsg->szContentLocation, sizeof(pMmsMsg->szContentLocation), "%s", dbHandle.columnText(i++));
		snprintf(pMmsMsg->szFileName, sizeof(pMmsMsg->szFileName), "%s", dbHandle.columnText(i++));
	}


	dbHandle.finalizeQuery();
}

void MmsPluginStorage::getMmsAttrib(msg_message_id_t msgId, MmsMsg *pMmsMsg)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION, DATA_TYPE,  DATE, HIDE_ADDRESS, ASK_DELIVERY_REPORT, REPORT_ALLOWED, \
			READ_REPORT_ALLOWED_TYPE, ASK_READ_REPLY, READ, READ_REPORT_SEND_STATUS, READ_REPORT_SENT, PRIORITY, \
			MSG_SIZE, MSG_CLASS, EXPIRY_TIME, CUSTOM_DELIVERY_TIME, DELIVERY_TIME, MSG_STATUS FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

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
		pMmsMsg->mmsAttrib.msgStatus = (msg_delivery_report_status_t)dbHandle.columnInt(18);
	}

	dbHandle.finalizeQuery();
}


msg_error_t MmsPluginStorage::getMmsMessageId(msg_message_id_t selectedMsgId, MmsMsg *pMmsMsg)
{
	msg_error_t err = MSG_SUCCESS;

	int rowCnt = 0;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MESSAGE_ID FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, selectedMsgId);

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


int MmsPluginStorage::getMmsVersion(msg_message_id_t selectedMsgId)
{
	msg_error_t err = MSG_SUCCESS;
	int rowCnt = 0;

	int	version = 0;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VERSION FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, selectedMsgId);

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


msg_error_t MmsPluginStorage::getContentLocation(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONTENTS_LOCATION FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

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
msg_error_t MmsPluginStorage::getTrID(MSG_MESSAGE_INFO_S *pMsgInfo,char *pszTrID,int nBufferLen)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT TRANSACTION_ID FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

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

msg_error_t MmsPluginStorage::getAddressInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	// Add Address
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_TYPE, A.RECIPIENT_TYPE, A.CONTACT_ID, A.ADDRESS_VAL \
			FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pAddrInfo->addressType = dbHandle.columnInt(0);
		pAddrInfo->recipientType = dbHandle.columnInt(1);
		pAddrInfo->contactId = dbHandle.columnInt(2);

		strncpy(pAddrInfo->addressVal, (char*)dbHandle.columnText(3), MAX_ADDRESS_VAL_LEN);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::getMmsRawFilePath(msg_message_id_t msgId, char *pFilepath)
{
	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

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

	msg_folder_id_t	folderId = MSG_SENTBOX_ID;

	char sqlQuery[MAX_QUERY_LEN + 1];

	MmsAddrUtilRemovePlmnString(toNumber);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID FROM %s A, %s B \
			WHERE A.MSG_ID = B.MSG_ID AND A.FOLDER_ID = %d AND B.MESSAGE_ID LIKE '%%%s%%'",
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


msg_error_t MmsPluginStorage::setReadReportSendStatus(msg_message_id_t msgId, int readReportSendStatus)
{
	bool bReadReportSent = false;

	if ((MmsRecvReadReportSendStatus)readReportSendStatus == MMS_RECEIVE_READ_REPORT_SENT)
		bReadReportSent = true;
	else
		bReadReportSent = false;

	char sqlQuery[MAX_QUERY_LEN + 1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_REPORT_SEND_STATUS = %d, READ_REPORT_SENT = %d WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, (MmsRecvReadReportSendStatus)readReportSendStatus, (int)bReadReportSent, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


msg_error_t MmsPluginStorage::getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;
	char *pMmsMsgText = NULL;
	int textLen = 0;
	bool bText = false;

	if (pMmsMsg == NULL)
		return MSG_ERR_NULL_POINTER;

	// Get the text data from the 1st slide.
	for (int i = 0; i< pMmsMsg->pageCnt; ++i) {
		pPage = _MsgMmsGetPage(pMmsMsg, i);

		for (int j = 0; j < pPage->mediaCnt; ++j) {
			pMedia = _MsgMmsGetMedia(pPage, j);

			if (pMedia->mediatype == MMS_SMIL_MEDIA_TEXT) {
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

msg_error_t MmsPluginStorage::insertPreviewInfo(int msgId, int type, char *value, int count)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	//(MSG_ID INTEGER, TYPE INTEGER, INFO TEXT)
	snprintf(sqlQuery, sizeof(sqlQuery),
			"INSERT INTO %s "
			"(MSG_ID, TYPE, VALUE, COUNT)"
			"VALUES (%d, %d, '%s', %d);",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId, type, value, count);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::removePreviewInfo(int msgId)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0,};

	// remove thumbnail file
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT VALUE FROM %s "
			"WHERE MSG_ID = %d AND (TYPE=%d OR TYPE=%d);",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId, MSG_MMS_ITEM_TYPE_IMG, MSG_MMS_ITEM_TYPE_VIDEO);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_PREPARE;
	}

	while (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {

		memset(filePath, 0x00, sizeof(filePath));
		strncpy(filePath, (char *)dbHandle.columnText(0), MSG_FILEPATH_LEN_MAX);
		if (remove(filePath) == -1)
			MSG_DEBUG("Fail to delete file [%s]", filePath);
		else
			MSG_DEBUG("Success to delete file [%s]", filePath);
	}

	dbHandle.finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	//(MSG_ID INTEGER, TYPE INTEGER, INFO TEXT)
	snprintf(sqlQuery, sizeof(sqlQuery),
			"DELETE FROM %s WHERE MSG_ID= %d;",
			MSGFW_MMS_PREVIEW_TABLE_NAME, msgId);

	MSG_DEBUG("QUERY : [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::deleteMmsMessage(int msgId)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0,};
	char dirPath[MSG_FILEPATH_LEN_MAX + 1]= {0,};

	//remove DB Preview
	removePreviewInfo(msgId);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		strncpy(filePath, (char *)dbHandle.columnText(0), MSG_FILEPATH_LEN_MAX);

		snprintf(dirPath, MSG_FILEPATH_LEN_MAX, "%s.dir", filePath);

		//delete pdu file
		if (remove(filePath) == -1)
			MSG_DEBUG("Fail to delete file [%s]", filePath);
		else
			MSG_DEBUG("Success to delete file [%s]", filePath);

		//delete multipart files
		MsgRmRf(dirPath);

		//delete multipart dir
		rmdir(dirPath);

	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
		dbHandle.finalizeQuery();
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();


	// Delete Data from MMS table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Delete Data from MMS STATUS table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
			MSGFW_REPORT_TABLE_NAME, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}

msg_error_t MmsPluginStorage::plgGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char **pDestMsg)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;
	MMS_MESSAGE_DATA_S tempMmsMsgData = {0,};
	MMS_MESSAGE_DATA_S *pMmsMsg = &tempMmsMsgData;

	size_t nSize = 0;

	bzero(pMmsMsg, sizeof(MMS_MESSAGE_DATA_S));
	pMmsMsg->regionCnt = 0;
	pMmsMsg->pageCnt = 0;
	pMmsMsg->attachCnt = 0;
	pMmsMsg->transitionCnt = 0;
	pMmsMsg->metaCnt = 0;
	memset(pMmsMsg->szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

	MmsMsg tempMmsMsg;
	memset(&tempMmsMsg, 0x00, sizeof(MmsMsg));

	getMmsFromDB(pMsg->msgId, &tempMmsMsg);

	snprintf(pMmsMsg->header.contentLocation, MSG_MSG_ID_LEN, "%s", tempMmsMsg.szContentLocation);
	pMmsMsg->header.mmsVersion = tempMmsMsg.mmsAttrib.version;
	pMmsMsg->header.messageClass = tempMmsMsg.mmsAttrib.msgClass;
	pMmsMsg->header.mmsPriority = tempMmsMsg.mmsAttrib.priority;
	snprintf(pMmsMsg->header.messageID, MSG_MSG_ID_LEN, "%s", tempMmsMsg.szMsgID);
	snprintf(pMmsMsg->header.trID, MSG_MSG_ID_LEN, "%s", tempMmsMsg.szTrID);

	if (pSendOptInfo != NULL) {

		pSendOptInfo->bDeliverReq = tempMmsMsg.mmsAttrib.bAskDeliveryReport;
		MSG_DEBUG("## delivery = %d ##", pSendOptInfo->bDeliverReq);

		pSendOptInfo->bKeepCopy = tempMmsMsg.mmsAttrib.bLeaveCopy;
		MSG_DEBUG("## bKeepCopy = %d ##", pSendOptInfo->bKeepCopy);

		pSendOptInfo->option.mmsSendOptInfo.bReadReq = tempMmsMsg.mmsAttrib.bAskReadReply;
		MSG_DEBUG("## bReadReq = %d ##", pSendOptInfo->option.mmsSendOptInfo.bReadReq);

		pSendOptInfo->option.mmsSendOptInfo.priority = tempMmsMsg.mmsAttrib.priority;
		MSG_DEBUG("## priority = %d ##", pSendOptInfo->option.mmsSendOptInfo.priority);

		pSendOptInfo->option.mmsSendOptInfo.expiryTime.time = tempMmsMsg.mmsAttrib.expiryTime.time;
		MSG_DEBUG("## expiryTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.expiryTime.time);

		pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = tempMmsMsg.mmsAttrib.deliveryTime.time;
		MSG_DEBUG("## deliveryTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time);
	}

	switch(pMsg->msgType.subType) {
		case MSG_SENDREQ_MMS:
		case MSG_SENDCONF_MMS:
		case MSG_RETRIEVE_MMS:
		case MSG_RETRIEVE_AUTOCONF_MMS:
		case MSG_RETRIEVE_MANUALCONF_MMS:
		{
			if (MmsReadMsgBody(pMsg->msgId, true, false, NULL) == false) {
				MSG_DEBUG("Fail to MmsReadMsgBody");
				goto FREE_CATCH;
			}

			//header value
			snprintf(pMmsMsg->header.szContentType, MSG_MSG_ID_LEN, "%s", MimeGetMimeStringFromMimeInt(mmsMsg.msgType.type));
			pMmsMsg->header.contentType = mmsMsg.msgType.type;
			pMmsMsg->header.messageType = mmsMsg.mmsAttrib.msgType;

			//body value
			if (MmsMakeMmsData(&mmsMsg, pMmsMsg) == false) {
				MSG_DEBUG("Fail to makeMmsMessageData");
				goto FREE_CATCH;
			}
		}
		break;

		case MSG_NOTIFICATIONIND_MMS:
			pMmsMsg->header.messageType = MMS_MSGTYPE_NOTIFICATION_IND;

		break;
		default:

		break;
	}



	*pDestMsg = _MsgMmsSerializeMessageData(pMmsMsg, &nSize);

	MsgMmsReleaseMmsLists(pMmsMsg);


	MmsMsg *pStoMmsMsg;
	MmsPluginStorage::instance()->getMmsMessage(&pStoMmsMsg);
	MmsInitHeader();
	MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
	MmsReleaseMsgDRMInfo(&pStoMmsMsg->msgType.drmInfo);
#endif
	MmsReleaseMsgBody(&pStoMmsMsg->msgBody, pStoMmsMsg->msgType.type);

	pMsg->dataSize = nSize;
	MSG_END();

	return err;

FREE_CATCH:

	MSG_DEBUG("MmsPlgUpdateMessage : Update MMS Message Failed");
	MSG_END();
	{
		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);
		MmsInitHeader();

		MmsUnregisterDecodeBuffer();
#ifdef __SUPPORT_DRM__
		MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
#endif
		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

		return MSG_ERR_STORAGE_ERROR;
	}
}

void MmsPluginStorage::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	MSG_BEGIN();

	msg_error_t	err;
	MmsMsg *pMmsMsg = NULL;
	MMS_MESSAGE_DATA_S *pMmsMsgData = NULL;

	char raw_filepath[MSG_FILENAME_LEN_MAX+1] = {0,};

	pMmsMsg = (MmsMsg *)calloc(1, sizeof(MmsMsg));
	if (pMmsMsg == NULL) {
		MSG_DEBUG("memory allocation error");
		goto __CATCH;
	}

	pMmsMsgData = (MMS_MESSAGE_DATA_S *)calloc(1, sizeof(MMS_MESSAGE_DATA_S));
	if (pMmsMsgData == NULL) {
		MSG_DEBUG("memory allocation error");
		goto __CATCH;
	}

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS) {

		if (_MsgMmsDeserializeMessageData(pMmsMsgData, pFileData) == false) {
			MSG_DEBUG("Fail to Deserialize Message Data");
			goto __CATCH;
		}

		if (MmsComposeSendReq(pMmsMsg, pMsgInfo, pSendOptInfo, pMmsMsgData) != true) {
			MSG_DEBUG("Fail to Compose Message");
			goto __CATCH;
		}

		//mms file
		snprintf(raw_filepath, sizeof(raw_filepath), "%s/%d.mms", MSG_DATA_PATH, pMsgInfo->msgId);

		//encode mms
		if (MmsEncodeMmsMessage(pMmsMsg, raw_filepath) == false) {
			MSG_DEBUG("Fail to Encode Message");
			goto __CATCH;
		}

		//add to db
		if (addMmsMsgToDB(pMmsMsg, raw_filepath) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to add db message");
			goto __CATCH;
		}

		//preview data
		err = MmsMakePreviewInfo(pMsgInfo->msgId, pMmsMsgData);
		err = getMsgText(pMmsMsgData, pMsgInfo->msgText);

		if (pMmsMsgData->attachCnt > 0) {
			if (updateMmsAttachCount(pMmsMsg->msgID, pMmsMsgData->attachCnt) != MSG_SUCCESS) {
				MSG_DEBUG("Fail to updateMmsAttachCount");
				goto __CATCH;
			}
		}

		if (MsgGetFileSize(raw_filepath, (int *)&pMsgInfo->dataSize) == false) {
			MSG_DEBUG("Fail to get mms file size [%s]", raw_filepath);
			goto __CATCH;
		}

		MmsReleaseMmsMsg(pMmsMsg);
		MMS_FREE(pMmsMsg);

		MsgMmsReleaseMmsLists(pMmsMsgData);
		MMS_FREE(pMmsMsgData);

	} else if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {

		MmsComposeNotiMessage(pMmsMsg, pMsgInfo->msgId);

		//add to db
		if (addMmsMsgToDB(pMmsMsg, "") != MSG_SUCCESS) {
			MSG_DEBUG("Fail to add db message");
			goto __CATCH;
		}

		MmsReleaseMmsMsg(pMmsMsg);
		MMS_FREE(pMmsMsg);
	} else if (pMsgInfo->msgType.subType == MSG_SENDCONF_MMS || pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS) {

		MmsMsg *pMsg = NULL;

		if (!MmsReadMsgBody(pMsgInfo->msgId, true, true, pFileData))
			THROW(MsgException::MMS_PLG_ERROR, "_MmsReadMsgBody Error");

		MmsPluginStorage::instance()->getMmsMessage(&pMsg);

		if (pMsgInfo->msgType.subType == MSG_SENDCONF_MMS)
			pMsgInfo->networkStatus = MSG_NETWORK_SEND_SUCCESS;
		else
			pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;


		{//preview data
			MMS_MESSAGE_DATA_S tempMmsMsgData = {0,};
			if (MmsMakeMmsData(pMsg, &tempMmsMsgData) == false) {
				MSG_DEBUG("Fail to makeMmsMessageData");
			} else {
				err = MmsMakePreviewInfo(pMsgInfo->msgId, &tempMmsMsgData);
				err = getMsgText(&tempMmsMsgData, pMsgInfo->msgText);
			}

			if (tempMmsMsgData.attachCnt > 0) {
				if (updateMmsAttachCount(pMsg->msgID, tempMmsMsgData.attachCnt) != MSG_SUCCESS) {
					MSG_DEBUG("Fail to updateMmsAttachCount");
					goto __CATCH;
				}
			}

			MsgMmsReleaseMmsLists(&tempMmsMsgData);
		}//end preview data

		if (addMmsMsgToDB(pMsg, pFileData) != MSG_SUCCESS) {
			MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

			THROW(MsgException::MMS_PLG_ERROR, "MMS Stroage Error");
		}

		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);
	} else {
		MSG_DEBUG("Not support msg sub type [%d]", pMsgInfo->msgType.subType);
		goto __CATCH;
	}

	MmsReleaseMmsMsg(pMmsMsg);
	MsgMmsReleaseMmsLists(pMmsMsgData);
	MMS_FREE(pMmsMsg);
	MMS_FREE(pMmsMsgData);

	MSG_END();
	return;

__CATCH:

	removePreviewInfo(pMsgInfo->msgId);

	MmsReleaseMmsMsg(pMmsMsg);
	MsgMmsReleaseMmsLists(pMmsMsgData);
	MMS_FREE(pMmsMsg);
	MMS_FREE(pMmsMsgData);

	THROW(MsgException::MMS_PLG_ERROR, "MMS add Error");
}
