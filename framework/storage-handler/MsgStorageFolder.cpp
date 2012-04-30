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

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgSqliteWrapper.h"
#include "MsgStorageHandler.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgStoAddFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int rowId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	err = dbHandle.getRowId(MSGFW_FOLDER_TABLE_NAME, &rowId);

	if (err != MSG_SUCCESS)
		return err;

	// Add Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, '%s', %d);",
				MSGFW_FOLDER_TABLE_NAME, rowId, pFolderInfo->folderName, pFolderInfo->folderType);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoUpdateFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	// Update Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_NAME = '%s', FOLDER_TYPE = %d WHERE FOLDER_ID = %d;",
				MSGFW_FOLDER_TABLE_NAME, pFolderInfo->folderName, pFolderInfo->folderType, pFolderInfo->folderId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDeleteFolder(const MSG_FOLDER_ID_T FolderId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE FOLDER_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, FolderId);

	// Delete Message in the folder from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE FOLDER_ID = %d;",
					MSGFW_FOLDER_TABLE_NAME, FolderId);

	// Delete Message in the folder from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Clear Address table
	if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetFolderList(MSG_FOLDER_LIST_S *pFolderList)
{
	if (pFolderList == NULL)
	{
		MSG_DEBUG("pFolderList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0;
	int index = 3;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FOLDER_ID, FOLDER_TYPE, FOLDER_NAME FROM %s;",
		MSGFW_FOLDER_TABLE_NAME);

	if (dbHandle.getTable(sqlQuery, &rowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	pFolderList->nCount = rowCnt;

	MSG_DEBUG("pFolderList->nCount [%d]", pFolderList->nCount);

	pFolderList->folderInfo = (MSG_FOLDER_INFO_S*)new char[sizeof(MSG_FOLDER_INFO_S)*rowCnt];

	MSG_FOLDER_INFO_S* pTmp = pFolderList->folderInfo;

	for (int i = 0; i < rowCnt; i++)
	{
		pTmp->folderId = dbHandle.getColumnToInt(index++);

		pTmp->folderType = dbHandle.getColumnToInt(index++);

		memset(pTmp->folderName, 0x00, sizeof(pTmp->folderName));
		dbHandle.getColumnToString(index++, MAX_FOLDER_NAME_SIZE, pTmp->folderName);

		pTmp++;
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}

