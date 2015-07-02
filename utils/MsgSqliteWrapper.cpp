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
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "MsgDebug.h"
#include "MsgSqliteWrapper.h"

extern "C"
{
	#include <db-util.h>
}

Mutex MsgDbHandler::mx;

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
//__thread sqlite3 *handle = NULL;
//__thread sqlite3_stmt *stmt = NULL;

/*==================================================================================================
                                     IMPLEMENTATION OF MsgDbHandler - Member Functions
==================================================================================================*/
MsgDbHandler::MsgDbHandler()
{
	result = NULL;
	handle = NULL;
	stmt   = NULL;
	mmapMx = NULL;
	shm_fd = -1;
}


MsgDbHandler::~MsgDbHandler()
{
	freeTable();
	finalizeQuery();
	if (disconnect() != MSG_SUCCESS)
		MSG_DEBUG("DB disconnect is failed.");
}

msg_error_t MsgDbHandler::connect()
{
	int ret = 0;

	if (handle == NULL) {
		char strDBName[64];

		memset(strDBName, 0x00, sizeof(strDBName));
		snprintf(strDBName, 64, "%s", MSGFW_DB_NAME);

		ret = db_util_open(strDBName, &handle, DB_UTIL_REGISTER_HOOK_METHOD);

		if (ret == SQLITE_OK) {
			MSG_DEBUG("DB Connect Success : [%p]", handle);
			return MSG_SUCCESS;
		} else {
			MSG_DEBUG("DB Connect Fail [%d]", ret);
			return MSG_ERR_DB_CONNECT;
		}
	} else {
		MSG_DEBUG("DB Connect exist : [%p]", handle);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::disconnect()
{
	int ret = 0;

	if (handle != NULL) {
		ret = db_util_close(handle);

		if (ret == SQLITE_OK) {
			handle = NULL;
			MSG_DEBUG("DB Disconnect Success");
			return MSG_SUCCESS;
		} else {
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

	if (getTable(strQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
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
//	int ret = 0;

	if (!pQuery)
		return MSG_ERR_INVALID_PARAMETER;

	MutexLocker lock(mx);

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;

	if (prepareQuery(pQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to prepareQuery.");
		return MSG_ERR_DB_EXEC;
	}

	if (stepQuery() == MSG_ERR_DB_STEP) {
		MSG_DEBUG("Fail to stepQuery.");
		finalizeQuery();
		return MSG_ERR_DB_EXEC;
	}

	finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::getTable(const char *pQuery, int *pRowCnt, int *pColumnCnt)
{
	int ret = 0;

	*pRowCnt = 0;
	if (pColumnCnt)
		*pColumnCnt = 0;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;

	freeTable();
	MSG_DEBUG("[%s]", pQuery);
	ret = sqlite3_get_table(handle, pQuery, &result, pRowCnt, pColumnCnt, NULL);

	if (ret == SQLITE_OK) {
		if (*pRowCnt == 0) {// when the no record return 'MSG_ERR_DB_NORECORD'
			MSG_DEBUG("No Query Result");
			return MSG_ERR_DB_NORECORD;
		}

		MSG_DEBUG("Get Table Success");
		return MSG_SUCCESS;
	} else if (ret == SQLITE_BUSY) {
		MSG_DEBUG("The database file is locked [%d]", ret);
		return MSG_ERR_DB_BUSY;
	} else {
		MSG_DEBUG("Get Table Fail [%d]", ret);
		return MSG_ERR_DB_GETTABLE;
	}

	return MSG_SUCCESS;
}


void MsgDbHandler::freeTable()
{
	if (result) {
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


msg_error_t MsgDbHandler::bindInt(const int pBindint, int index)
{
	int ret = 0;

	ret = sqlite3_bind_int(stmt, index, pBindint);

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

	if ((ret = sqlite3_prepare_v2(handle, pQuery, strlen(pQuery), &stmt, NULL)) == SQLITE_OK) {
		MSG_DEBUG("Prepare Query Success");
		return MSG_SUCCESS;
	} else {
		MSG_ERR("Prepare Query Fail [%d]", ret);
		finalizeQuery();
		return MSG_ERR_DB_PREPARE;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgDbHandler::stepQuery()
{
	int ret = 0;

	ret = sqlite3_step(stmt);

	if (ret == SQLITE_ROW) {
		MSG_DEBUG("MsgStepQuery() SQLITE_ROW");
		return MSG_ERR_DB_ROW;
	} else if (ret == SQLITE_DONE) {
		MSG_DEBUG("MsgStepQuery() SQLITE_DONE");
		return MSG_ERR_DB_DONE;
	} else {
		MSG_ERR("MsgStepQuery() Fail [%d]", ret);
		finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	return MSG_SUCCESS;
}


void MsgDbHandler::resetQuery()
{
	if (stmt)
		sqlite3_reset(stmt);
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

void MsgDbHandler::getMmapMutex(const char *shm_file_name)
{
	MSG_BEGIN();

	if(!shm_file_name) {
		MSG_FATAL ("NULL INPUT_PARAM");
		return;
	}

	MSG_DEBUG("** mapping begin **");

	 /*  open shm_file_name at first. Otherwise, the num of files in /proc/pid/fd will be increasing  */
	shm_fd = shm_open(shm_file_name, O_RDWR, 0);
	if (shm_fd == -1) {
		MSG_FATAL("shm_open error [%d]", errno);
		return;
	}

	pthread_mutex_t *tmp = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (tmp == MAP_FAILED) {
		MSG_FATAL("mmap error [%d]", errno);
		return;
	}
	mmapMx = tmp;
}

void MsgDbHandler::shm_mutex_timedlock (int sec)
{
	MSG_BEGIN();
	if (!mmapMx) {
		MSG_DEBUG("mmapMx not initd");
		return;
	}

	struct timespec abs_time;
	clock_gettime(CLOCK_REALTIME, &abs_time);
	abs_time.tv_sec += sec;

	int err = pthread_mutex_timedlock(mmapMx, &abs_time);

	if (err == EOWNERDEAD) {
		err = pthread_mutex_consistent(mmapMx);
		MSG_DEBUG("Previous owner is dead with lock. Fix mutex");
	}
	else if (err != 0) {
		MSG_FATAL("pthread_mutex_timedlock error [%d]", errno);
		return;
	}

	MSG_END();
}

void MsgDbHandler::shm_mutex_unlock()
{
	MSG_BEGIN();
	if(!mmapMx) {
		MSG_DEBUG("mmapMx not initd");
		return;
	}

	pthread_mutex_unlock(mmapMx);
	MSG_END();
}

msg_error_t MsgDbHandler::beginTrans()
{
	int ret = 0;

	if(connect() != MSG_SUCCESS)
		return MSG_ERR_DB_DISCONNECT;

	if (!mmapMx) {
		getMmapMutex(SHM_FILE_FOR_DB_LOCK);
	}
	shm_mutex_timedlock(2);

	ret = sqlite3_exec(handle, "BEGIN DEFERRED TRANSACTION;", 0, 0, NULL);

	if (ret == SQLITE_OK) {
		MSG_DEBUG("Begin Transaction Success");
		return MSG_SUCCESS;
	} else {
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


	if (Success) {
		ret = sqlite3_exec(handle, "COMMIT TRANSACTION;", 0, 0, NULL);
	} else {
		ret = sqlite3_exec(handle, "ROLLBACK TRANSACTION;", 0, 0, NULL);
	}

	shm_mutex_unlock();

	if (ret == SQLITE_OK) {
		MSG_DEBUG("End Transaction Success");
		return MSG_SUCCESS;
	} else {
		MSG_DEBUG("End Transaction Fail [%d]", ret);
		if (Success) endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


int MsgDbHandler::getColumnToInt(int RowIndex)
{
	char* pTemp = result[RowIndex];

	int nTemp = 0;

	if (pTemp == NULL) {
		MSG_DEBUG("NULL");
		return nTemp;
	}

	nTemp = (int)strtol(pTemp, (char**)NULL, 10);

	return nTemp;
}


char MsgDbHandler::getColumnToChar(int RowIndex)
{
	char* pTemp = result[RowIndex];

	if (pTemp == NULL) {
		MSG_DEBUG("NULL");
		return '\0';
	}

	return *pTemp;
}


char *MsgDbHandler::getColumnToString(int RowIndex)
{
	char* pTemp = result[RowIndex];

	if (pTemp == NULL) {
		MSG_DEBUG("NULL");
		return NULL;
	}

	return pTemp;
}


void MsgDbHandler::getColumnToString(int RowIndex, int Length, char *pString)
{
	char* pTemp = result[RowIndex];

	if (pTemp == NULL) {
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

	MSG_SEC_DEBUG("Table Name [%s]", pTableName);

	memset(strQuery, 0x00, sizeof(strQuery));
	snprintf(strQuery, sizeof(strQuery), "select max(rowid) from %s", pTableName);

	ret = getTable(strQuery, &nRowCnt, NULL);

	if (ret == SQLITE_OK) {
		nRowId = getColumnToInt(1);

		if ((nRowCnt <= 1) && (nRowId == 0))
			*pRowId = 1;
		else
			*pRowId = nRowId + 1;
	} else {
		MSG_DEBUG("MsgGetRowId failed");
		*pRowId = 0;
		freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	freeTable();

	MSG_DEBUG("Row ID [%d]", *pRowId);

	return MSG_SUCCESS;
}


void MsgReleaseMemoryDB()
{
	int freeSize = 0;

	freeSize = sqlite3_release_memory(-1);

	MSG_DEBUG("freed memory size (bytes) : [%d]", freeSize);
}


msg_error_t MsgConvertStrWithEscape(const char *input, char **output)
{
	if (input == NULL || output == NULL || strlen(input) == 0) {
		MSG_DEBUG("MSG_ERR_INVALID_PARAMETER");
		return MSG_ERR_INVALID_PARAMETER;
	}

	int inputSize = 0;
	int i = 0;
	int j = 0;

	inputSize = strlen(input);
	MSG_DEBUG("Size of input string [%d]", inputSize);

	char tmpStr[(inputSize*2)+3];
	memset(tmpStr, 0x00, sizeof(tmpStr));

	tmpStr[j++] = '%';

	for(i=0;i<inputSize;i++) {
		if (input[i] == '\'' || input[i] == '_' || input[i] == '%' || input[i] == '\\') {
			tmpStr[j++] = MSGFW_DB_ESCAPE_CHAR;
		}
		tmpStr[j++] = input[i];
	}
	tmpStr[j++] = '%';
	tmpStr[j] = '\0';

	*output = strdup(tmpStr);

	return MSG_SUCCESS;
}

typedef std::map<pthread_t, MsgDbHandler*> dbMap_t;
dbMap_t gDbHandles;
pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

MsgDbHandler *getDbHandle()
{
	pthread_t self = pthread_self();
	MsgDbHandler *tmp = NULL;

	pthread_mutex_lock(&mu);
	dbMap_t::iterator it = gDbHandles.find(self);
	if (it == gDbHandles.end()) {
		MSG_DEBUG("New DB handle added.");
		tmp = new MsgDbHandler();
		gDbHandles.insert ( std::pair<pthread_t,MsgDbHandler*>(self,tmp));

	} else {
		tmp = it->second;
	}
	pthread_mutex_unlock(&mu);

	return tmp;
}

void removeDbHandle()
{
	pthread_t self = pthread_self();
	MsgDbHandler *tmp = NULL;

	pthread_mutex_lock(&mu);
	dbMap_t::iterator it = gDbHandles.find(self);
	if (it != gDbHandles.end()) {
		tmp = it->second;
		delete tmp;
		gDbHandles.erase (it);
	}
	pthread_mutex_unlock(&mu);
}

