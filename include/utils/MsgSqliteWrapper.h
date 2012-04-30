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

#ifndef MSG_SQLITE_WRAPPER_H
#define MSG_SQLITE_WRAPPER_H

/*==================================================================================================
                                    INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"


/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MSGFW_DB_NAME 			"/opt/dbspace/.msg_service.db"

#define MSGFW_MESSAGE_TABLE_NAME    		"MSG_MESSAGE_TABLE"
#define MSGFW_FOLDER_TABLE_NAME			"MSG_FOLDER_TABLE"
#define MSGFW_ADDRESS_TABLE_NAME    		"MSG_ADDRESS_TABLE"
#define MSGFW_SIM_MSG_TABLE_NAME    		"MSG_SIM_TABLE"
#define MSGFW_FILTER_TABLE_NAME			"MSG_FILTER_TABLE"
#define MSGFW_PUSH_MSG_TABLE_NAME		"MSG_PUSH_TABLE"
#define MSGFW_CB_MSG_TABLE_NAME			"MSG_CBMSG_TABLE"
#define MMS_PLUGIN_MESSAGE_TABLE_NAME		"MSG_MMS_MESSAGE_TABLE"
#define MMS_PLUGIN_ATTRIBUTE_TABLE_NAME	"MSG_MMS_ATTR_TABLE"
#define MSGFW_SYNCML_MSG_TABLE_NAME    	"MSG_SYNCML_TABLE"
#define MSGFW_SCHEDULED_MSG_TABLE_NAME 	"MSG_SCHEDULED_TABLE"
#define MSGFW_SMS_SENDOPT_TABLE_NAME 		"MSG_SMS_SENDOPT_TABLE"
#define MSGFW_REPORT_TABLE_NAME			"MSG_REPORT_TABLE"

#define MAX_QUERY_LEN					3072
#define MAX_FOLDER_NAME_LEN			20
#define MAX_ACCOUNT_NAME_LEN			51


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void	MsgReleaseMemoryDB();


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgDbHandler
{
public:
	MsgDbHandler();
	~MsgDbHandler();

	MSG_ERROR_T connect();
	MSG_ERROR_T disconnect();

	bool checkTableExist(const char *pTableName);
	MSG_ERROR_T execQuery(const char *pQuery);
	MSG_ERROR_T getTable(const char *pQuery, int *pRowCnt);
	void freeTable();
	MSG_ERROR_T bindText(const char *pBindStr, int index);
	MSG_ERROR_T bindBlob(const void * pBindBlob, int size, int index);
	MSG_ERROR_T prepareQuery(const char *pQuery);
	MSG_ERROR_T stepQuery();
	void finalizeQuery();
	int columnInt(int ColumnIndex);
	const unsigned char* columnText(int ColumnIndex);
	const void* columnBlob(int ColumnIndex);
	MSG_ERROR_T beginTrans();
	MSG_ERROR_T endTrans(bool Success);
	int getColumnToInt(int RowIndex);
	char getColumnToChar(int RowIndex);
	void getColumnToString(int RowIndex, int Length, char *pString);
	MSG_ERROR_T getRowId(const char *pTableName, unsigned int *pRowId);

private:


};

#endif // MSG_SQLITE_WRAPPER_H

