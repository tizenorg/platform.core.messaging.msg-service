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

#include <queue>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"

using namespace std;


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoClearSimMessageInDB()
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<msg_thread_id_t> threadList;

	// Get Address ID of SIM Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE STORAGE_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

	int rowCnt = 0;

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++)
	{
		MSG_DEBUG("thread ID : %d", dbHandle->getColumnToInt(i));
		threadList.push((msg_thread_id_t)dbHandle->getColumnToInt(i));
	}

	dbHandle->freeTable();

	const char* tableList[] = {MSGFW_SMS_SENDOPT_TABLE_NAME, MSGFW_SIM_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char*);

	dbHandle->beginTrans();

	for (int i = 0; i < listCnt; i++)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
				(SELECT MSG_ID FROM %s WHERE STORAGE_ID = %d);",
				tableList[i], MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

		// Delete SIM Message in tables
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Update Address
	while (!threadList.empty())
	{
		err = MsgStoUpdateConversation(dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}
	}

	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}

