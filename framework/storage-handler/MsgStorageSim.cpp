/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
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
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgInitSimMessage(MSG_SIM_STATUS_T SimStatus)
{
	MSG_DEBUG("Start SIM Message Initialization");

	msg_error_t err = MSG_SUCCESS;

	// Set SIM count of vconf to 0
	if (MsgSettingSetInt(SIM_USED_COUNT, 0) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
	}

	if (MsgSettingSetInt(SIM_TOTAL_COUNT, 0) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", SIM_TOTAL_COUNT);
	}

	if (SimStatus != MSG_SIM_STATUS_NOT_FOUND) {
		MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
		MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

		if (plg == NULL) {
			MSG_DEBUG("No plugin for %d type", mainType);
			return MSG_ERR_INVALID_PLUGIN_HANDLE;
		}

		// Check SIM Status
		MSG_DEBUG(" ** SIM is available - status : [%d] ** ", SimStatus);

		err = plg->initSimMessage();
	}

	return err;
}


msg_error_t MsgStoClearSimMessageInDB()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<msg_thread_id_t> threadList;

	// Get Address ID of SIM Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE STORAGE_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

	int rowCnt = 0;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("sql query is %s.", sqlQuery);

		dbHandle.freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++)
	{
		MSG_DEBUG("thread ID : %d", dbHandle.getColumnToInt(i));
		threadList.push((msg_thread_id_t)dbHandle.getColumnToInt(i));
	}

	dbHandle.freeTable();

	const char* tableList[] = {MSGFW_SMS_SENDOPT_TABLE_NAME, MSGFW_SIM_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char*);

	dbHandle.beginTrans();

	for (int i = 0; i < listCnt; i++)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
				(SELECT MSG_ID FROM %s WHERE STORAGE_ID = %d);",
				tableList[i], MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

		// Delete SIM Message in tables
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Update Address
	while (!threadList.empty())
	{
		err = MsgStoUpdateConversation(&dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}

