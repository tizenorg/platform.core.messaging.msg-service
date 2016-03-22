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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <queue>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgSoundPlayer.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgUtilStorage.h"
#include "MsgStorageHandler.h"

using namespace std;

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoGetSmsSendOpt(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S* pSendOpt)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DELREP_REQ, KEEP_COPY, REPLY_PATH, ENCODE_TYPE FROM %s WHERE MSG_ID = %d;",
			MSGFW_SMS_SENDOPT_TABLE_NAME, pMsg->msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pSendOpt->bSetting = true;
		pSendOpt->bDeliverReq = (bool)dbHandle->columnInt(0);
		pSendOpt->bKeepCopy = (bool)dbHandle->columnInt(1);
		pSendOpt->option.smsSendOptInfo.bReplyPath = (bool)dbHandle->columnInt(2);
		pMsg->encodeType = dbHandle->columnInt(3);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMmsSendOpt(msg_message_id_t msgId, MSG_SENDINGOPT_INFO_S* pSendOpt)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ASK_DELIVERY_REPORT, KEEP_COPY, ASK_READ_REPLY, EXPIRY_TIME, PRIORITY FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pSendOpt->bSetting = true;
		pSendOpt->bDeliverReq = (bool)dbHandle->columnInt(0);
		pSendOpt->bKeepCopy = (bool)dbHandle->columnInt(1);
		pSendOpt->option.mmsSendOptInfo.bReadReq = (bool)dbHandle->columnInt(2);
		pSendOpt->option.mmsSendOptInfo.expiryTime.time = (unsigned int)dbHandle->columnInt(3);
		pSendOpt->option.mmsSendOptInfo.priority = (msg_priority_type_t)dbHandle->columnInt(4);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


bool MsgStoCheckSyncMLMsgInThread(msg_thread_id_t threadId)
{
	int rowCnt = 0;
	bool isSyncMLMsg = false;
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE SUB_TYPE = %d AND CONV_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_SYNCML_CP, threadId);

	if (dbHandle->getTable(sqlQuery, &rowCnt, NULL) != MSG_SUCCESS) {
		MSG_DEBUG("getTable is failed!!!");
	}

	if (rowCnt > 0) isSyncMLMsg = true;

	MSG_DEBUG("rowCnt [%d]", rowCnt);

	dbHandle->freeTable();

	return isSyncMLMsg;
}


msg_error_t MsgStoResetNetworkStatus()
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery),
			"UPDATE %s SET NETWORK_STATUS = %d WHERE NETWORK_STATUS = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SEND_FAIL, MSG_NETWORK_SENDING);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		//continue reset process for MSG_NETWORK_RETRIEVE_FAIL case
		MSG_INFO("execQuery is failed");
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery),
			"UPDATE %s SET NETWORK_STATUS = %d WHERE NETWORK_STATUS = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_RETRIEVE_FAIL, MSG_NETWORK_RETRIEVING);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}

#if 0
msg_error_t MsgStoResetCBMessage()
{
	MSG_BEGIN();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", MSGFW_RECEIVED_CB_MSG_TABLE_NAME);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Delete Received CB Msg Fail!!");
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();
	return MSG_SUCCESS;
}
#endif


msg_error_t MsgStoCleanAbnormalMmsData()
{
	MSG_BEGIN();

	int rowCnt = 0, index = 0; // numbers of index
	MsgDbHandler *dbHandle = getDbHandle();
	msg_message_id_t msgId;

	char sqlQuery[MAX_QUERY_LEN+1];
	char	filePath[MSG_FILEPATH_LEN_MAX];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.FILE_PATH FROM %s A, %s B WHERE A.MSG_ID = B.MSG_ID AND (B.SUB_TYPE = %d OR B.SUB_TYPE = %d OR B.SUB_TYPE = %d);",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);

	msg_error_t  err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}


	for (int i = 0; i < rowCnt; i++) {
		memset(filePath, 0x00, sizeof(filePath));

		msgId = dbHandle->getColumnToInt(index++);

		dbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, filePath);

		if(strlen(filePath) > 1) {
			MSG_DEBUG("strlen(filePath) [%d]", strlen(filePath));
			MSG_SEC_DEBUG("filePath [%s]", filePath);

			if(MsgGetFileSize(filePath) < 0) {
				// abnormal mms message
				MSG_DEBUG("abnormal mms message [%d]", msgId);

				// delete mms message
				MsgStoDeleteMessage(msgId, false);
			}
		}
	}
	dbHandle->freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoCheckReadReportStatus(msg_message_id_t msgId)
{
	MSG_BEGIN();

	bool	bReadReportRequested;
	bool	bReadReportIsSent;
	MsgDbHandler *dbHandle = getDbHandle();
	bReadReportRequested = MsgStoCheckReadReportRequested(dbHandle, msgId);
	if(bReadReportRequested == false)
		return MSG_ERR_READREPORT_NOT_REQUESTED;

	bReadReportIsSent = MsgStoCheckReadReportIsSent(dbHandle, msgId);
	if(bReadReportIsSent == true)
		return MSG_ERR_READREPORT_ALEADY_SENT;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoAutoDeleteConversation(msg_thread_id_t threadId, msg_id_list_s *msgIdList)
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	bool bAutoErase = false;
	if (MsgSettingGetBool(MSG_AUTO_ERASE, &bAutoErase) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	if (bAutoErase) {
		MSG_DEBUG("threadId [%d]", threadId);

		//msg_id_list_s msgIdList;
		int currentSmsCnt = 0;
		int currentMmsCnt = 0;
		int limitSmsCnt = 0;
		int limitMmsCnt = 0;

		char sqlQuery[MAX_QUERY_LEN+1];

		int rowCnt = 0, index = 0;

		//memset(msgIdList, 0x00, sizeof(msg_id_list_s));

		// Get current count of messages
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT COUNT(MSG_ID) FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND MAIN_TYPE = %d "
				"UNION ALL "
				"SELECT COUNT(MSG_ID) FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE, MSG_SMS_TYPE,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE, MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS);
#else
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT COUNT(MSG_ID) FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND MAIN_TYPE = %d "
				"UNION ALL "
				"SELECT COUNT(MSG_ID) FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE, MSG_SMS_TYPE,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE, MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS);
#endif

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			return MSG_ERR_DB_PREPARE;
		}

		if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			currentSmsCnt = dbHandle->columnInt(0);
		} else {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			currentMmsCnt = dbHandle->columnInt(0);
		} else {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbHandle->finalizeQuery();

		MSG_DEBUG("currentSmsCnt [%d], currentMmsCnt [%d]", currentSmsCnt, currentMmsCnt);

		if (currentSmsCnt > 0 || currentMmsCnt > 0) {
			if (MsgSettingGetInt(MSG_SMS_LIMIT, &limitSmsCnt) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetInt() is failed");
			}

			if (MsgSettingGetInt(MSG_MMS_LIMIT, &limitMmsCnt) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetInt() is failed");
			}

			if (limitSmsCnt < 0 || limitMmsCnt < 0) {
				MSG_DEBUG("limitSmsCnt [%d], limitMmsCnt [%d]", limitSmsCnt, limitMmsCnt);
				return MSG_SUCCESS;
			}
		} else { // !(currentSmsCnt > 0 || currentMmsCnt > 0)
			return MSG_SUCCESS;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT DISTINCT(MSG_ID) FROM %s "
				"WHERE MSG_ID IN "
				"(SELECT MSG_ID FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0 "
				"AND MAIN_TYPE = %d ORDER BY MSG_ID ASC LIMIT %d) "
				"OR MSG_ID IN "
				"(SELECT MSG_ID FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0 "
				"AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d) ORDER BY MSG_ID ASC LIMIT %d);",
				MSGFW_MESSAGE_TABLE_NAME,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSG_SMS_TYPE, ((currentSmsCnt-limitSmsCnt) > 0)?(currentSmsCnt-limitSmsCnt):0,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS, ((currentMmsCnt-limitMmsCnt) > 0)?(currentMmsCnt-limitMmsCnt):0);
#else
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT DISTINCT(MSG_ID) FROM %s "
				"WHERE MSG_ID IN "
				"(SELECT MSG_ID FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0 "
				"AND MAIN_TYPE = %d ORDER BY MSG_ID ASC LIMIT %d) "
				"OR MSG_ID IN "
				"(SELECT MSG_ID FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0 "
				"AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d) ORDER BY MSG_ID ASC LIMIT %d);",
				MSGFW_MESSAGE_TABLE_NAME,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE,
				MSG_SMS_TYPE, ((currentSmsCnt-limitSmsCnt) > 0)?(currentSmsCnt-limitSmsCnt):0,
				MSGFW_MESSAGE_TABLE_NAME,
				threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE,
				MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS, ((currentMmsCnt-limitMmsCnt) > 0)?(currentMmsCnt-limitMmsCnt):0);
#endif
		err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
			MSG_DEBUG("Fail to getTable().");
			dbHandle->freeTable();
			return err;
		}

		if (rowCnt <= 0) {
			MSG_DEBUG("rowCnt <= 0");
			dbHandle->freeTable();
			return MSG_SUCCESS;
		}

		msgIdList->nCount = rowCnt;

		MSG_DEBUG("msgIdList.nCount [%d]", msgIdList->nCount);

		msgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

		for (int i = 0; i < rowCnt; i++) {
			msgIdList->msgIdList[i] = dbHandle->getColumnToInt(index++);
		}

		dbHandle->freeTable();

		// delete message
		err = MsgStoDeleteMessageByList(msgIdList);

		//delete [] (char*)msgIdList.msgIdList;

	} else {
		MSG_DEBUG("bAutoErase [%d]", bAutoErase);
	}

	MSG_END();

	return err;
}


msg_error_t MsgStoGetReplaceMsgId(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_thread_id_t convId = 0;

	/** Check if new address or not */
	if (MsgExistAddress(dbHandle, pMsgInfo, &convId) == true) {
		MSG_DEBUG("Address Info. exists [%d]", convId);

		/**  Find Replace Type Msg : Same Replace Type, Same Origin Address, Same Storage ID */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE SUB_TYPE = %d AND CONV_ID = %d AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgType.subType, convId, pMsgInfo->storageId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			pMsgInfo->msgId = dbHandle->columnInt(0);
		} else {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbHandle->finalizeQuery();
	}

	return err;
}


msg_error_t MsgStoAddWAPMsg(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	MSG_PUSH_MESSAGE_S pushMsg = {};

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	int fileSize = 0;

	char* pFileData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

	if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
		return MSG_ERR_STORAGE_ERROR;

	MSG_DEBUG("fileSize : %d", fileSize);

	memcpy(&pushMsg, pFileData, fileSize);

	/** Delete temporary file */
	MsgDeleteFile(pMsgInfo->msgData);

	/** check pPushMsg data */

	MSG_DEBUG("check pushMsg data");
	MSG_DEBUG("pushMsg.action : [%d]", pushMsg.action);
	MSG_DEBUG("pushMsg.received : [%d]", pushMsg.received);
	MSG_DEBUG("pushMsg.created : [%d]", pushMsg.created);
	MSG_DEBUG("pushMsg.expires : [%d]", pushMsg.expires);
	MSG_SEC_DEBUG("pushMsg.id : [%s]", pushMsg.id);
	MSG_DEBUG("pushMsg.href : [%s]", pushMsg.href);
	MSG_DEBUG("pushMsg.contents : [%s]", pushMsg.contents);

	bool bProceed = true;

	/**  check validation of contents */
	if (MsgStoCheckPushMsgValidation(&pushMsg, &bProceed) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to check Push Message validation.");
	}

	/**  if validation check value is false */
	/** return and drop message. */
	if (bProceed == false)
		return MSG_ERR_INVALID_MESSAGE;


	/**  Update Msg Text - remove */
	strncpy(pMsgInfo->msgText, pushMsg.href, MAX_MSG_TEXT_LEN);

	if (pushMsg.contents[0] != '\0') {
		strncat(pMsgInfo->msgText, " ", MAX_MSG_TEXT_LEN - strlen(pMsgInfo->msgText));
		strncat(pMsgInfo->msgText, pushMsg.contents, MAX_MSG_TEXT_LEN - strlen(pMsgInfo->msgText));
	}

	pMsgInfo->dataSize = strlen(pMsgInfo->msgText);

	pMsgInfo->bTextSms = true;
	pMsgInfo->folderId = MSG_INBOX_ID;
	pMsgInfo->storageId = MSG_STORAGE_PHONE;

	msg_thread_id_t convId = 0;

	dbHandle->beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(dbHandle, pMsgInfo, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}

		pMsgInfo->threadId = convId;
	}

	/**  get last row count for Message id */
	unsigned int rowId = 0;

	/** Add Message Table */
	rowId = MsgStoAddMessageTable(dbHandle, pMsgInfo);

	if (rowId <= 0) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	pMsgInfo->msgId = (msg_message_id_t)rowId;

	/**  add msg_push_table */
	snprintf (sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %lu, %lu, ?, ?, ?)",
			MSGFW_PUSH_MSG_TABLE_NAME, pMsgInfo->msgId, pushMsg.action, pushMsg.created, pushMsg.expires);

	if ((err = dbHandle->prepareQuery(sqlQuery)) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return err;
	}

	dbHandle->bindText(pushMsg.id, 1);

	dbHandle->bindText(pushMsg.href, 2);

	dbHandle->bindText(pushMsg.contents, 3);

	if ((err = dbHandle->stepQuery()) != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return err;
	}

	dbHandle->finalizeQuery();

	/** Update conversation table. */
	if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoAddCOWAPMsg(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	char href[MAX_PUSH_CACHEOP_MAX_URL_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	int fileSize = 0;

	char* pFileData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

	if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
		return MSG_ERR_STORAGE_ERROR;

	MSG_PUSH_CACHEOP_S *pPushMsg;

	pPushMsg = (MSG_PUSH_CACHEOP_S*)pFileData;

	for (int i = 0; i < pPushMsg->invalObjectCnt; i++) {
		int msgid = -1;

		memset(href, 0x00, sizeof(href));
		strncpy(href, &(pPushMsg->invalObjectUrl[i][0]), MAX_PUSH_CACHEOP_MAX_URL_LEN);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf (sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE HREF LIKE '%%%s%%';", MSGFW_PUSH_MSG_TABLE_NAME, href);

		dbHandle->beginTrans();

		err = dbHandle->prepareQuery(sqlQuery);

		if ((dbHandle->stepQuery() == MSG_ERR_DB_ROW) && err == MSG_SUCCESS) {
			msgid = dbHandle->columnInt(0);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID= %d;", MSGFW_PUSH_MSG_TABLE_NAME, msgid);

			/** Delete Message from Push table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgid);

			/** Delete Message from msg table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			/** Update all Address */
			if (MsgStoUpdateAllAddress() != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			/** Clear Conversation table */
			if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}
		}

		dbHandle->finalizeQuery();

		dbHandle->endTrans(true);
	}

	for (int i = 0; i < pPushMsg->invalServiceCnt; i++) {
		int msgid = -1;

		memset(href, 0x00, sizeof(href));
		strncpy(href, &(pPushMsg->invalObjectUrl[i][0]), MAX_PUSH_CACHEOP_MAX_URL_LEN);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE HREF LIKE '%%%s%%'", MSGFW_PUSH_MSG_TABLE_NAME, href);

		dbHandle->beginTrans();

		err = dbHandle->prepareQuery(sqlQuery);

		if ((dbHandle->stepQuery() == MSG_ERR_DB_ROW) && err == MSG_SUCCESS) {
			msgid = dbHandle->columnInt(0);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID='%d'", MSGFW_PUSH_MSG_TABLE_NAME, msgid);

			/** Delete Message from Push table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgid);

			/** Delete Message from msg table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			/**  Update all Address */
			if (MsgStoUpdateAllAddress() != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}

			/** Clear Address table */
			if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				continue;
			}
		}

		dbHandle->finalizeQuery();

		dbHandle->endTrans(true);
	}

	/** delete temporary file */
	MsgDeleteFile(pMsgInfo->msgData);

	return MSG_SUCCESS;
}


msg_error_t MsgStoAddCBMsg(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	unsigned int rowId = 0;
	msg_thread_id_t convId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle->beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(dbHandle, pMsgInfo, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}

		pMsgInfo->threadId = convId;
	}

	/**  Add Message Table */
	rowId = MsgStoAddMessageTable(dbHandle, pMsgInfo);

	if (rowId <= 0) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	/**  Get CB Msg ID */
	unsigned short cbMsgId = (unsigned short)pMsgInfo->msgId;

	/** Add CB Msg in MSG_CBMSG_TABLE */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d);",
			MSGFW_CB_MSG_TABLE_NAME, rowId, cbMsgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Update conversation table. */
	if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);

	/** Assign Msg ID */
	pMsgInfo->msgId = (msg_message_id_t)rowId;

	return err;
}


msg_error_t MsgStoCheckPushMsgValidation(MSG_PUSH_MESSAGE_S *pPushMsg, bool *pbProceed)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	unsigned long oldExpireTime = 0;
	int rowCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	/**  is push message is expired?? */
	if (pPushMsg->received > pPushMsg->expires) {
		MSG_DEBUG("Push Message is expired.");
		*pbProceed = false;
		return err;
	}

	if (pPushMsg->action == MSG_PUSH_SL_ACTION_EXECUTE_LOW) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXPIRES FROM %s WHERE ID = '%s' AND ACTION = %d",
				MSGFW_PUSH_MSG_TABLE_NAME, pPushMsg->id, pPushMsg->action);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXPIRES FROM %s WHERE ID = '%s'",
				MSGFW_PUSH_MSG_TABLE_NAME, pPushMsg->id);
	}

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (rowCnt < 1) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	oldExpireTime = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (pPushMsg->created < oldExpireTime) {
		MSG_DEBUG("Push Message is expired.");
		*pbProceed = false;
		return err;
	}

	return err;
}


msg_error_t MsgStoUpdateAllAddress()
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	int rowCnt = 0, index = 0;
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID FROM %s", MSGFW_ADDRESS_TABLE_NAME);

	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		dbHandle->freeTable();
		return err;
	}

	for (int i = 0; i < rowCnt; i++) {
		err = MsgStoUpdateConversation(dbHandle, index++);

		if (err != MSG_SUCCESS)
			break;
	}

	dbHandle->freeTable();

	return err;
}

