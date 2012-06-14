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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgMakeSortRule(const MSG_SORT_RULE_S *pSortRule, char *pSqlSort)
{
	char sql[128];
	char order[6];

	memset(sql, 0x00, sizeof(sql));
	memset(order, 0x00, sizeof(order));

	if (pSortRule->bAscending == true)
		strncpy(order, "ASC", 5);
	else
		strncpy(order, "DESC", 5);

	int nameOrder = MsgGetContactNameOrder();

	switch (pSortRule->sortType)
	{
		case MSG_SORT_BY_DISPLAY_FROM :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY B.FIRST_NAME %s, B.LAST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY B.LAST_NAME %s, B.FIRST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			break;
		case MSG_SORT_BY_DISPLAY_TO :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY B.FIRST_NAME %s, B.LAST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY B.LAST_NAME %s, B.FIRST_NAME %s, B.ADDRESS_VAL, A.DISPLAY_TIME DESC;", order, order);
			break;
		case MSG_SORT_BY_DISPLAY_TIME :
			snprintf(sql, sizeof(sql), "ORDER BY A.DISPLAY_TIME %s;", order);
			break;
		case MSG_SORT_BY_MSG_TYPE :
			snprintf(sql, sizeof(sql), "ORDER BY A.MAIN_TYPE %s, A.DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_READ_STATUS :
			snprintf(sql, sizeof(sql), "ORDER BY A.READ_STATUS %s, A.DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_STORAGE_TYPE :
			snprintf(sql, sizeof(sql), "ORDER BY A.STORAGE_ID %s, A.DISPLAY_TIME DESC;", order);
			break;
		case MSG_SORT_BY_THREAD_NAME :
			if (nameOrder == 0)
				snprintf(sql, sizeof(sql), "ORDER BY FIRST_NAME %s, LAST_NAME %s;", order, order);
			else
				snprintf(sql, sizeof(sql), "ORDER BY LAST_NAME %s, FIRST_NAME %s;", order, order);
			break;
		case MSG_SORT_BY_THREAD_DATE :
			snprintf(sql, sizeof(sql), "ORDER BY MSG_TIME %s;", order);
			break;
		case MSG_SORT_BY_THREAD_COUNT :
			snprintf(sql, sizeof(sql), "ORDER BY UNREAD_CNT %s;", order);
			break;
		default :
			snprintf(sql, sizeof(sql), "ORDER BY A.DISPLAY_TIME %s;", order);
			break;
	}

	memcpy(pSqlSort, sql, strlen(sql));

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetSmsSendOpt(MSG_MESSAGE_ID_T MsgId, MSG_SENDINGOPT_INFO_S* pSendOpt)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DELREP_REQ, KEEP_COPY, REPLY_PATH FROM %s WHERE MSG_ID = %d;",
				MSGFW_SMS_SENDOPT_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW)
	{
		pSendOpt->bSetting = true;
		pSendOpt->bDeliverReq = (bool)dbHandle.columnInt(0);
		pSendOpt->bKeepCopy = (bool)dbHandle.columnInt(1);
		pSendOpt->option.smsSendOptInfo.bReplyPath = (bool)dbHandle.columnInt(2);
	}
	else
	{
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetMmsSendOpt(MSG_MESSAGE_ID_T MsgId, MSG_SENDINGOPT_INFO_S* pSendOpt)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ASK_DELIVERY_REPORT, KEEP_COPY, ASK_READ_REPLY, EXPIRY_TIME, PRIORITY FROM %s WHERE MSG_ID = %d;",
				MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW)
	{
		pSendOpt->bSetting = true;
		pSendOpt->bDeliverReq = (bool)dbHandle.columnInt(0);
		pSendOpt->bKeepCopy = (bool)dbHandle.columnInt(1);
		pSendOpt->option.mmsSendOptInfo.bReadReq = (bool)dbHandle.columnInt(2);
		pSendOpt->option.mmsSendOptInfo.expiryTime.time = (unsigned int)dbHandle.columnInt(3);
		pSendOpt->option.mmsSendOptInfo.priority = (MSG_PRIORITY_TYPE_T)dbHandle.columnInt(4);
	}
	else
	{
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoAddScheduledMessage(MSG_MESSAGE_ID_T MsgID, int AlarmId, int ListenerFd)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
				MSGFW_SCHEDULED_MSG_TABLE_NAME, MsgID, AlarmId, ListenerFd);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
	{
		return MSG_ERR_DB_EXEC;
	}

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE)
	{
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDeleteScheduledMessage(MSG_MESSAGE_ID_T MsgId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_SCHEDULED_MSG_TABLE_NAME, MsgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
	{
		MSG_DEBUG("Delete Scheduled Msg Fail!!");
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetScheduledMessage(int AlarmId, MSG_REQUEST_INFO_S *pReqInfo, int *pListenerFd)
{
	MSG_BEGIN();

	MSG_ERROR_T	err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, LISTENER_FD FROM %s WHERE ALARM_ID = %d;",
				MSGFW_SCHEDULED_MSG_TABLE_NAME, AlarmId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW)
	{
		pReqInfo->msgInfo.msgId = dbHandle.columnInt(0);
		*pListenerFd = dbHandle.columnInt(1);
	}

	dbHandle.finalizeQuery();

	err = MsgStoGetMessage(pReqInfo->msgInfo.msgId, &(pReqInfo->msgInfo), &(pReqInfo->sendOptInfo));

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoGetMessage() Error!! [%d]", err);
		return err;
	}

	MSG_END();

	return MSG_SUCCESS;
}


bool MsgStoCheckSyncMLMsgInThread(MSG_THREAD_ID_T threadId)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	int rowCnt = 0;
	bool isSyncMLMsg = false;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE SUB_TYPE = %d AND ADDRESS_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_SYNCML_CP, threadId);

	MSG_DEBUG("sqlQuery [%s]", sqlQuery);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (rowCnt > 0) isSyncMLMsg = true;

	MSG_DEBUG("rowCnt [%d]", rowCnt);

	dbHandle.freeTable();

	return isSyncMLMsg;
}


MSG_ERROR_T MsgStoResetNetworkStatus()
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery),
		"UPDATE %s SET NETWORK_STATUS = %d WHERE NETWORK_STATUS = %d; UPDATE %s SET NETWORK_STATUS = %d WHERE NETWORK_STATUS = %d;"
		, MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SEND_FAIL, MSG_NETWORK_SENDING
		, MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_RETRIEVE_FAIL, MSG_NETWORK_RETRIEVING);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoCleanAbnormalMmsData()
{
	MSG_BEGIN();

	int rowCnt = 0, index = 2; // numbers of index

	MSG_MESSAGE_ID_T msgId;

	char sqlQuery[MAX_QUERY_LEN+1];
	char	filePath[MSG_FILEPATH_LEN_MAX];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.FILE_PATH FROM %s A, %s B WHERE A.MSG_ID = B.MSG_ID AND (B.SUB_TYPE = %d OR B.SUB_TYPE = %d OR B.SUB_TYPE = %d);",
		MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);

	MSG_ERROR_T  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD)
	{
		dbHandle.freeTable();

		return MSG_SUCCESS;
	}
	else if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}


	for (int i = 0; i < rowCnt; i++)
	{
		memset(filePath, 0x00, sizeof(filePath));

		msgId = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, filePath);

		if(strlen(filePath) > 1)
		{
			MSG_DEBUG("strlen(filePath) [%d]", strlen(filePath));
			MSG_DEBUG("filePath [%s]", filePath);
			if(MsgGetFileSize(filePath) < 0)
			{
				// abnormal mms message
				MSG_DEBUG("abnormal mms message [%d]", msgId);

				// delete mms message
				MsgStoDeleteMessage(msgId, false);
			}
		}

	}
	dbHandle.freeTable();

	MSG_END();

	return MSG_SUCCESS;
}

MSG_ERROR_T MsgStoCheckReadReportStatus(MSG_MESSAGE_ID_T msgId)
{
	MSG_BEGIN();

	bool	bReadReportRequested;
	bool	bReadReportIsSent;

	bReadReportRequested = MsgStoCheckReadReportRequested(&dbHandle, msgId);
	if(bReadReportRequested == false)
		return MSG_ERR_READREPORT_NOT_REQUESTED;

	bReadReportIsSent = MsgStoCheckReadReportIsSent(&dbHandle, msgId);
	if(bReadReportIsSent == true)
		return MSG_ERR_READREPORT_ALEADY_SENT;

	MSG_END();

	return MSG_SUCCESS;
}

