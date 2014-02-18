/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
#define MSGFW_SMS_SENDOPT_TABLE_NAME		"MSG_SMS_SENDOPT_TABLE"
#define MSGFW_REPORT_TABLE_NAME				"MSG_REPORT_TABLE"
#define MSGFW_PUSH_CONFIG_TABLE_NAME			"MSG_PUSHCFG_TABLE"
#define MSGFW_MMS_PREVIEW_TABLE_NAME			"MSG_MMS_PREVIEW_INFO_TABLE"

#define MSGFW_TMP_MSGID_TABLE_NAME			"MSG_TMP_MSGID_TABLE"

#define MAX_QUERY_LEN					3072
#define MAX_FOLDER_NAME_LEN		20
#define MAX_ACCOUNT_NAME_LEN	51

#define MSGFW_DB_ESCAPE_CHAR		'\\'

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
	msg_error_t getTable(const char *pQuery, int *pRowCnt);
	void freeTable();
	msg_error_t bindText(const char *pBindStr, int index);
	msg_error_t bindBlob(const void * pBindBlob, int size, int index);
	msg_error_t prepareQuery(const char *pQuery);
	msg_error_t stepQuery();
	void finalizeQuery();
	int columnInt(int ColumnIndex);
	const unsigned char* columnText(int ColumnIndex);
	const void* columnBlob(int ColumnIndex);
	msg_error_t beginTrans();
	msg_error_t endTrans(bool Success);
	int getColumnToInt(int RowIndex);
	char getColumnToChar(int RowIndex);
	void getColumnToString(int RowIndex, int Length, char *pString);
	msg_error_t getRowId(const char *pTableName, unsigned int *pRowId);

private:

	char **result;

};

#endif // MSG_SQLITE_WRAPPER_H

