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

#ifndef MSG_SQLITE_WRAPPER_H
#define MSG_SQLITE_WRAPPER_H

/*==================================================================================================
                                    INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"
#include <db-util.h>
#include "MsgMutex.h"

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MSGFW_DB_NAME 			"/opt/usr/dbspace/.msg_service.db"

#define MSGFW_MESSAGE_TABLE_NAME				"MSG_MESSAGE_TABLE"
#define MSGFW_FOLDER_TABLE_NAME					"MSG_FOLDER_TABLE"
#define MSGFW_ADDRESS_TABLE_NAME    			"MSG_ADDRESS_TABLE"
#define MSGFW_CONVERSATION_TABLE_NAME		"MSG_CONVERSATION_TABLE"
#define MSGFW_SIM_MSG_TABLE_NAME				"MSG_SIM_TABLE"
#define MSGFW_FILTER_TABLE_NAME					"MSG_FILTER_TABLE"
#define MSGFW_PUSH_MSG_TABLE_NAME			"MSG_PUSH_TABLE"
#define MSGFW_CB_MSG_TABLE_NAME				"MSG_CBMSG_TABLE"
#define MMS_PLUGIN_MESSAGE_TABLE_NAME		"MSG_MMS_MESSAGE_TABLE"
#define MSGFW_SYNCML_MSG_TABLE_NAME		"MSG_SYNCML_TABLE"

#if 0
#define MSGFW_RECEIVED_CB_MSG_TABLE_NAME	"MSG_CB_MESSAGE_TABLE"
#endif

#define MSGFW_CB_CHANNEL_INFO_TABLE_NAME	"MSG_CB_CHANNEL_INFO_TABLE"
#ifdef FEATURE_SMS_CDMA
#define MSGFW_CDMA_CB_CHANNEL_INFO_TABLE_NAME		"MSG_CDMA_CB_CHANNEL_INFO_TABLE"
#endif

#define MSGFW_SMS_SENDOPT_TABLE_NAME		"MSG_SMS_SENDOPT_TABLE"
#define MSGFW_SMS_REPORT_TABLE_NAME		"MSG_SMS_REPORT_TABLE"
#define MSGFW_REPORT_TABLE_NAME					"MSG_REPORT_TABLE"
#define MSGFW_PUSH_CONFIG_TABLE_NAME		"MSG_PUSHCFG_TABLE"
#define MSGFW_MMS_PREVIEW_TABLE_NAME		"MSG_MMS_PREVIEW_INFO_TABLE"
#define MSGFW_MMS_MULTIPART_TABLE_NAME		"MSG_MULTIPART_TABLE"

#ifdef FEATURE_SMS_CDMA
#define MSGFW_UNIQUENESS_INFO_TABLE_NAME	"MSG_UNIQUENESS_INFO_TABLE"
#endif

#define MSGFW_TMP_MSGID_TABLE_NAME			"MSG_TMP_MSGID_TABLE"

#define MSGFW_ADDRESS_TEMP_TABLE_NAME		"MSG_ADDRESS_TEMP_TABLE"

#define MAX_QUERY_LEN					3072
#define MAX_FOLDER_NAME_LEN		20
#define MAX_ACCOUNT_NAME_LEN	51

#define MSGFW_DB_ESCAPE_CHAR		'\\'

#define SHM_FILE_FOR_DB_LOCK                "/.msg_shm_db_lock"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void	MsgReleaseMemoryDB();
msg_error_t MsgConvertStrWithEscape(const char *input, char **output);


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgDbHandler
{
public:
	MsgDbHandler();
	~MsgDbHandler();

	msg_error_t connect();
	msg_error_t disconnect();

	bool checkTableExist(const char *pTableName);
	msg_error_t execQuery(const char *pQuery);
	msg_error_t getTable(const char *pQuery, int *pRowCnt, int *pColumnCnt);
	void freeTable();
	msg_error_t bindText(const char *pBindStr, int index);
	msg_error_t bindInt(const int pBindint, int index);
	msg_error_t bindBlob(const void * pBindBlob, int size, int index);
	msg_error_t prepareQuery(const char *pQuery);
	msg_error_t stepQuery();
	void resetQuery();
	void finalizeQuery();
	int columnInt(int ColumnIndex);
	const unsigned char* columnText(int ColumnIndex);
	const void* columnBlob(int ColumnIndex);
	msg_error_t beginTrans();
	msg_error_t endTrans(bool Success);
	int getColumnToInt(int RowIndex);
	char getColumnToChar(int RowIndex);
	char* getColumnToString(int RowIndex);
	void getColumnToString(int RowIndex, int Length, char *pString);
	msg_error_t getRowId(const char *pTableName, unsigned int *pRowId);
	void getMmapMutex(const char *shm_file_name);
	void shm_mutex_timedlock (int sec);
	void shm_mutex_unlock();

private:
	char **result;
	sqlite3 *handle;
	sqlite3_stmt *stmt;
	pthread_mutex_t *mmapMx;
	int shm_fd;
	static Mutex mx;
};


MsgDbHandler *getDbHandle();
void removeDbHandle();
#endif // MSG_SQLITE_WRAPPER_H

