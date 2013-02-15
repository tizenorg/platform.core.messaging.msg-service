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
msg_error_t MsgStoAddFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
{
	msg_error_t err = MSG_SUCCESS;

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


msg_error_t MsgStoUpdateFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
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


msg_error_t MsgStoDeleteFolder(const msg_folder_id_t folderId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, folderId);

	// Delete Message in the folder from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, folderId);

	// Delete Message in the folder from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetFolderList(msg_struct_list_s *pFolderList)
{
	if (pFolderList == NULL) {
		MSG_DEBUG("pFolderList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0;
	int index = 3;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FOLDER_ID, FOLDER_TYPE, FOLDER_NAME FROM %s;",
			MSGFW_FOLDER_TABLE_NAME);

	if (dbHandle.getTable(sqlQuery, &rowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	pFolderList->nCount = rowCnt;

	MSG_DEBUG("pFolderList->nCount [%d]", pFolderList->nCount);

	pFolderList->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_FOLDER_INFO_S *)*rowCnt];

	msg_struct_s* pTmp = NULL;

	for (int i = 0; i < rowCnt; i++)
	{
		pFolderList->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
		pTmp = (msg_struct_s *)pFolderList->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_FOLDER_INFO;
		pTmp->data = new char[sizeof(MSG_FOLDER_INFO_S)];
		MSG_FOLDER_INFO_S * pFolder = (MSG_FOLDER_INFO_S *)pTmp->data;
		memset(pFolder, 0x00, sizeof(MSG_FOLDER_INFO_S));

		pFolder->folderId = dbHandle.getColumnToInt(index++);

		pFolder->folderType = dbHandle.getColumnToInt(index++);

		memset(pFolder->folderName, 0x00, sizeof(pFolder->folderName));
		dbHandle.getColumnToString(index++, MAX_FOLDER_NAME_SIZE, pFolder->folderName);

	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}
