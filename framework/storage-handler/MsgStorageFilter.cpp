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

#include "MsgDebug.h"
#include "MsgSqliteWrapper.h"
#include "MsgStorageHandler.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoCheckDuplicatedFilter(const MSG_FILTER_S *pFilter)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char *filterStr = NULL;
	char sqlQuery[MAX_QUERY_LEN+1];

	MsgConvertStrWithEscape(pFilter->filterValue, &filterStr);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND FILTER_VALUE = ?;",
			MSGFW_FILTER_TABLE_NAME, pFilter->filterType);

	MSG_DEBUG("sql : %s", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
	{
	        if (filterStr)
        		free(filterStr);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.bindText(filterStr, 1);

	err = dbHandle.stepQuery();

	if (err == MSG_ERR_DB_ROW) {
		err = MSG_ERR_FILTER_DUPLICATED;
	} else if (err == MSG_ERR_DB_DONE) {
		err = MSG_SUCCESS;
	}

	dbHandle.finalizeQuery();

	if (filterStr)
		free(filterStr);

	MSG_END();

	return err;
}


msg_error_t MsgStoAddFilter(const MSG_FILTER_S *pFilter)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	//check duplication
	err = MsgStoCheckDuplicatedFilter(pFilter);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Filter is duplicated : [%d]", err);
		return err;
	}

	unsigned int rowId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	err = dbHandle.getRowId(MSGFW_FILTER_TABLE_NAME, &rowId);

	if (err != MSG_SUCCESS)
		return err;

	// Add Filter
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, ?, 1);",
			MSGFW_FILTER_TABLE_NAME, rowId, pFilter->filterType);

	MSG_DEBUG("sql : %s", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	dbHandle.bindText(pFilter->filterValue, 1);

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateFilter(const MSG_FILTER_S *pFilter)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	//check duplication
	err = MsgStoCheckDuplicatedFilter(pFilter);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Filter is duplicated : [%d]", err);
		return err;
	}

	char sqlQuery[MAX_QUERY_LEN+1];

	// Update Filter
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FILTER_TYPE = %d, FILTER_VALUE = ? WHERE FILTER_ID = %d;",
			MSGFW_FILTER_TABLE_NAME, pFilter->filterType, pFilter->filterId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	dbHandle.bindText(pFilter->filterValue, 1);

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteFilter(msg_filter_id_t filterId)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE FILTER_ID = %d;", MSGFW_FILTER_TABLE_NAME, filterId);

	// Delete Filter
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetFilterList(msg_struct_list_s *pFilterList)
{
	MSG_BEGIN();

	if (pFilterList == NULL) {
		MSG_DEBUG("pFilterList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0, index = 4;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Get filters from DB
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_ID, FILTER_TYPE, FILTER_VALUE, FILTER_ACTIVE FROM %s;", MSGFW_FILTER_TABLE_NAME);

	msg_error_t err = MSG_SUCCESS;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		pFilterList->nCount = 0;
		pFilterList->msg_struct_info = NULL;

		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		dbHandle.freeTable();
		return err;
	}

	pFilterList->nCount = rowCnt;

	MSG_DEBUG("pMsgCommInfoList->nCount [%d]", pFilterList->nCount);

	pFilterList->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_FILTER_S *)*rowCnt];

	msg_struct_s* pTmp = NULL;

	for (int i = 0; i < rowCnt; i++)
	{
		pFilterList->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];

		pTmp = (msg_struct_s *)pFilterList->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_FILTER;
		pTmp->data = new char [sizeof(MSG_FILTER_S)];
		MSG_FILTER_S *pFilter = (MSG_FILTER_S *)pTmp->data;
		memset(pFilter, 0x00, sizeof(MSG_FILTER_S));
		pFilter->filterId = dbHandle.getColumnToInt(index++);
		pFilter->filterType = dbHandle.getColumnToInt(index++);
		memset(pFilter->filterValue, 0x00, sizeof(pFilter->filterValue));
		dbHandle.getColumnToString(index++, MAX_FILTER_VALUE_LEN, pFilter->filterValue);
		pFilter->bActive = dbHandle.getColumnToInt(index++);
	}


	dbHandle.freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoSetFilterActivation(msg_filter_id_t filterId, bool bActive)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN+1];

	// Set Filter Activation
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FILTER_ACTIVE = %d WHERE FILTER_ID = %d;",
			MSGFW_FILTER_TABLE_NAME, bActive, filterId);

	MSG_DEBUG("sqlQuery [%s]", sqlQuery);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}
