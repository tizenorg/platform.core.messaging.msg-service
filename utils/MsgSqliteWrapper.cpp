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

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "MsgDebug.h"
#include "MsgSqliteWrapper.h"

extern "C"
{
	#include <db-util.h>
}


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
__thread sqlite3 *handle = NULL;
__thread sqlite3_stmt *stmt = NULL;
//__thread char **result = NULL;


/*==================================================================================================
                                     IMPLEMENTATION OF MsgDbHandler - Member Functions
==================================================================================================*/
MsgDbHandler::MsgDbHandler()
{
	handle = NULL;
	stmt = NULL;
	result = NULL;
}


MsgDbHandler::~MsgDbHandler()
{
	if(handle != NULL) {
		if (disconnect() != MSG_SUCCESS)
			MSG_DEBUG("disconnect is failed!!");
	}

	if(stmt != NULL)
		finalizeQuery();

	if(result != NULL)
		freeTable();
}


msg_error_t MsgDbHandler::connect()
{
	int ret = 0;

	if (handle == NULL)
	{
		char strDBName[64];

		memset(strDBName, 0x00, sizeof(strDBName));
		snprintf(strDBName, 64, "%s", MSGFW_DB_NAME);

		ret = db_util_open(strDBName, &handle, DB_UTIL_REGISTER_HOOK_METHOD);

		if (ret == SQLITE_OK)
		{
			MSG_DEBUG("DB Connect Success : [%p]", handle);
			return MSG_SUCCESS;
		}
		else
		{
			MSG_DEBUG("DB Connect Fail [%d]", ret);
			return MSG_ERR_DB_CONNECT;
		}
	}
	else
	{
		MSG_DEBUG("DB Connect exist : [%p]", handle);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::disconnect()
{
	int ret = 0;

	if (handle != NULL)
	{
		ret = db_util_close(handle);

		if (ret == SQLITE_OK)
		{
			handle = NULL;
			MSG_DEBUG("DB Disconnect Success");
			return MSG_SUCCESS;
		}
		else
		{
			MSG_DEBUG("DB Disconnect Fail [%d]", ret);
			return MSG_ERR_DB_DISCONNECT;
		}
	}

	return MSG_SUCCESS;
}


bool MsgDbHandler::checkTableExist(const char *pTableName)
{
	char strQuery[256];
	int nRowCnt = 0, nResult = 0;

	/* Formulate the Query */
	memset(strQuery, 0x00, sizeof(strQuery));
	snprintf(strQuery, sizeof(strQuery), "select count(name) from sqlite_master where name='%s'", pTableName);

	if (getTable(strQuery, &nRowCnt) != MSG_SUCCESS)
	{
		freeTable();
		return false;
	}

	nResult = getColumnToInt(1);
	MSG_DEBUG("Result [%d]", nResult);

	freeTable();

	if (nResult > 0)
		return true;
	else
		return false;
}


msg_error_t MsgDbHandler::execQuery(const char *pQuery)
{
	int ret = 0;

    	if (!pQuery)
		return MSG_ERR_INVALID_PARAMETER;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;

	ret = sqlite3_exec(handle, pQuery, 0, 0, NULL);

	if (ret == SQLITE_OK)
	{
		MSG_DEBUG("Execute Query Success");
		return MSG_SUCCESS;
	}
	else
	{
		MSG_DEBUG("Execute Query Fail [%d]", ret);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::getTable(const char *pQuery, int *pRowCnt)
{
	int ret = 0;

	*pRowCnt = 0;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;

	freeTable();
	ret = sqlite3_get_table(handle, pQuery, &result, pRowCnt, 0, NULL);

	if (ret == SQLITE_OK)
	{
		if (*pRowCnt == 0)    // when the no record return 'MSG_ERR_DB_NORECORD'
		{
			MSG_DEBUG("No Query Result");
			return MSG_ERR_DB_NORECORD;
		}

		MSG_DEBUG("Get Table Success");
		return MSG_SUCCESS;
	}
	else
	{
		MSG_DEBUG("Get Table Fail [%d]", ret);
		return MSG_ERR_DB_GETTABLE;
	}

	return MSG_SUCCESS;
}


void MsgDbHandler::freeTable()
{
	if (result)
	{
		sqlite3_free_table(result);
		result = NULL;
	}
}


msg_error_t MsgDbHandler::bindText(const char *pBindStr, int index)
{
	int ret = 0;

	if (pBindStr != NULL)
		ret = sqlite3_bind_text(stmt, index, pBindStr, (strlen(pBindStr) + sizeof(unsigned char)), SQLITE_STATIC);

	return ret;
}


msg_error_t MsgDbHandler::bindBlob(const void * pBindBlob, int size, int index)
{
	int ret = 0;

	ret = sqlite3_bind_blob(stmt, index, pBindBlob, size, SQLITE_STATIC);

	return ret;
}


msg_error_t MsgDbHandler::prepareQuery(const char *pQuery)
{
	int ret = 0;

	stmt = NULL;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;


	if ((ret = sqlite3_prepare_v2(handle, pQuery, strlen(pQuery), &stmt, NULL)) == SQLITE_OK)
	{
		MSG_DEBUG("Prepare Query Success");
		return MSG_SUCCESS;
	}
	else
	{
		MSG_DEBUG("Prepare Query Fail [%d]", ret);
		return MSG_ERR_DB_PREPARE;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::stepQuery()
{
	int ret = 0;

	ret = sqlite3_step(stmt);

	if (ret == SQLITE_ROW)
	{
		MSG_DEBUG("MsgStepQuery() SQLITE_ROW");
		return MSG_ERR_DB_ROW;
	}
	else if (ret == SQLITE_DONE)
	{
		MSG_DEBUG("MsgStepQuery() SQLITE_DONE");
		return MSG_ERR_DB_DONE;
	}
	else
	{
		MSG_DEBUG("MsgStepQuery() Fail [%d]", ret);
		return MSG_ERR_DB_STEP;
	}

	return MSG_SUCCESS;
}


void MsgDbHandler::finalizeQuery()
{
	if(stmt != NULL)
		sqlite3_finalize(stmt);
	stmt = NULL;
}


int MsgDbHandler::columnInt(int ColumnIndex)
{
	return sqlite3_column_int(stmt, ColumnIndex);
}


const unsigned char* MsgDbHandler::columnText(int ColumnIndex)
{
	return sqlite3_column_text(stmt, ColumnIndex);
}


const void* MsgDbHandler::columnBlob(int ColumnIndex)
{
	return sqlite3_column_blob(stmt, ColumnIndex);
}


msg_error_t MsgDbHandler::beginTrans()
{
	int ret = 0;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;


	ret = sqlite3_exec(handle, "BEGIN deferred;", 0, 0, NULL);

	if (ret == SQLITE_OK)
	{
		MSG_DEBUG("Begin Transaction Success");
		return MSG_SUCCESS;
	}
	else
	{
		MSG_DEBUG("Begin Transaction Fail [%d]", ret);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::endTrans(bool Success)
{
	int ret = 0;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;


	if (Success == true)
	{
		ret = sqlite3_exec(handle, "END;", 0, 0, NULL);
	}
	else
	{
		ret = sqlite3_exec(handle, "rollback", 0, 0, NULL);
		ret = sqlite3_exec(handle, "END;", 0, 0, NULL);
	}

	if (ret == SQLITE_OK)
	{
		MSG_DEBUG("End Transaction Success");
		return MSG_SUCCESS;
	}
	else
	{
		MSG_DEBUG("End Transaction Fail [%d]", ret);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


int MsgDbHandler::getColumnToInt(int RowIndex)
{
	char* pTemp = result[RowIndex];

	int nTemp = 0;

	if (pTemp == NULL)
	{
		MSG_DEBUG("NULL");
		return nTemp;
	}

	nTemp = (int)strtol(pTemp, (char**)NULL, 10);

	return nTemp;
}


char MsgDbHandler::getColumnToChar(int RowIndex)
{
	char* pTemp = result[RowIndex];

	if (pTemp == NULL)
	{
		MSG_DEBUG("NULL");
		return '\0';
	}

	return *pTemp;
}


void MsgDbHandler::getColumnToString(int RowIndex, int Length, char *pString)
{
	char* pTemp = result[RowIndex];

	if (pTemp == NULL)
	{
		MSG_DEBUG("NULL");
		return;
	}

	strncpy(pString, pTemp, Length);
}


msg_error_t MsgDbHandler::getRowId(const char *pTableName, unsigned int *pRowId)
{
	int ret = 0, nRowId = 0, nRowCnt = 0;
	char strQuery[256];

	if (pTableName == NULL || pRowId == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MSG_DEBUG("Table Name [%s]", pTableName);

	memset(strQuery, 0x00, sizeof(strQuery));
	snprintf(strQuery, sizeof(strQuery), "select max(rowid) from %s", pTableName);

	ret = getTable(strQuery, &nRowCnt);

	if (ret == SQLITE_OK)
	{
		nRowId = getColumnToInt(1);

		if ((nRowCnt <= 1) && (nRowId == 0))
			*pRowId = 1;
		else
			*pRowId = nRowId + 1;
	}
	else
	{
		MSG_DEBUG("MsgGetRowId failed");
		*pRowId = 0;
		freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	freeTable();

	MSG_DEBUG("Row ID [%d]", *pRowId);

	return MSG_SUCCESS;
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgReleaseMemoryDB()
{
	int freeSize = 0;

	freeSize = sqlite3_release_memory(-1);

	MSG_DEBUG("freed memory size (bytes) : [%d]", freeSize);
}

