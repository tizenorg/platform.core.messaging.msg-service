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
MSG_ERROR_T MsgInitSimMessage(MSG_SIM_STATUS_T SimStatus)
{
	MSG_DEBUG("Start SIM Message Initialization");

	MSG_ERROR_T err = MSG_SUCCESS;

	// Set SIM count of vconf to 0
	if (MsgSettingSetInt(SIM_USED_COUNT, 0) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
	}

	if (MsgSettingSetInt(SIM_TOTAL_COUNT, 0) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_TOTAL_COUNT);
	}

	if (SimStatus != MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
		MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

		if (plg == NULL)
		{
			MSG_DEBUG("No plugin for %d type", mainType);
			return MSG_ERR_INVALID_PLUGIN_HANDLE;
		}

		// Check SIM Status
		MSG_DEBUG(" ** SIM is available - status : [%d] ** ", SimStatus);

		err = plg->initSimMessage();
	}

	return err;
}


MSG_ERROR_T MsgStoClearSimMessageInDB()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<MSG_THREAD_ID_T> threadList;

	// Get Address ID of SIM Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(B.ADDRESS_ID) \
					FROM %s A, %s B \
				     WHERE A.ADDRESS_ID = B.ADDRESS_ID \
				          AND A.STORAGE_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MSG_STORAGE_SIM);

	int rowCnt = 0;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
	{
		MSG_DEBUG("sql query is %s.", sqlQuery);

		dbHandle.freeTable();
		return err;
	}

	if (rowCnt <= 0)
	{
		dbHandle.freeTable();
		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++)
	{
		MSG_DEBUG("thread ID : %d", dbHandle.getColumnToInt(i));
		threadList.push((MSG_THREAD_ID_T)dbHandle.getColumnToInt(i));
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
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		{
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Clear Address table
	if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Update Address
	while (!threadList.empty())
	{
		err = MsgStoUpdateAddress(&dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS)
		{
			dbHandle.endTrans(false);
			return err;
		}
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}

