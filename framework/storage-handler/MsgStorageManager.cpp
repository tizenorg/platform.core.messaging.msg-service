/*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoConnectDB()
{
	return MSG_SUCCESS;
}


msg_error_t MsgStoDisconnectDB()
{
	if (dbHandle.disconnect() != MSG_SUCCESS) {
		MSG_DEBUG("DB Disconnect Fail");
		return MSG_ERR_DB_DISCONNECT;
	}

	MSG_DEBUG("DB Disconnect Success");

	return MSG_SUCCESS;
}


msg_error_t MsgStoInitDB(bool bSimChanged)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

#ifdef MSG_DB_CREATE
	if (MsgCreateConversationTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateAddressTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateFolderTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateMsgTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateSimMessageTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateWAPMessageTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateCBMessageTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateSyncMLMessageTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateSmsSendOptTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateFilterTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateMmsTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;

	// Add Default Folders
	if (MsgAddDefaultFolders() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Folders Fail");
		return MSG_ERR_DB_STORAGE_INIT;
	}

	// Add Default Address
	if (MsgAddDefaultAddress() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Address Fail");
		return MSG_ERR_DB_STORAGE_INIT;
	}
#endif

	// Delete Msgs in Hidden Folder
	MsgStoDeleteAllMessageInFolder(0, true, NULL);

	// Reset network status
	MsgStoResetNetworkStatus();

	//clear abnormal mms message
	MsgStoCleanAbnormalMmsData();

	// Clear all old Sim data
	MsgStoClearSimMessageInDB();

	int smsCnt = 0, mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

	// Set Indicator
	MsgSettingSetIndicator(smsCnt, mmsCnt);

	MSG_END();

	return err;
}


msg_error_t MsgCreateConversationTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_ADDRESS_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				CONV_ID INTEGER NOT NULL , \
				UNREAD_CNT INTEGER DEFAULT 0 , \
				SMS_CNT INTEGER DEFAULT 0 , \
				MMS_CNT INTEGER DEFAULT 0 , \
				MAIN_TYPE INTEGER NOT NULL , \
				SUB_TYPE INTEGER NOT NULL , \
				MSG_DIRECTION INTEGER NOT NULL , \
				DISPLAY_TIME INTEGER , \
				DISPLAY_NAME TEXT , \
				MSG_TEXT TEXT );",
				MSGFW_CONVERSATION_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_CONVERSATION_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_CONVERSATION_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateAddressTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_ADDRESS_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				ADDRESS_ID INTEGER PRIMARY KEY , \
				CONV_ID INTEGER  NOT NULL , \
				ADDRESS_TYPE INTEGER , \
				RECIPIENT_TYPE INTEGER , \
				ADDRESS_VAL TEXT , \
				CONTACT_ID INTEGER , \
				DISPLAY_NAME TEXT , \
				FIRST_NAME TEXT , \
				LAST_NAME TEXT , \
				IMAGE_PATH TEXT , \
				SYNC_TIME DATETIME , \
				FOREIGN KEY(CONV_ID) REFERENCES %s (CONV_ID) );",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_ADDRESS_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_ADDRESS_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateFolderTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_FOLDER_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				FOLDER_ID INTEGER PRIMARY KEY, \
				FOLDER_NAME TEXT NOT NULL, \
				FOLDER_TYPE INTEGER DEFAULT 0 );",
				MSGFW_FOLDER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_FOLDER_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_FOLDER_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateMsgTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_MESSAGE_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER PRIMARY KEY , \
				CONV_ID INTEGER NOT NULL , \
				FOLDER_ID INTEGER NOT NULL , \
				STORAGE_ID INTEGER NOT NULL , \
				MAIN_TYPE INTEGER NOT NULL , \
				SUB_TYPE INTEGER NOT NULL , \
				DISPLAY_TIME DATETIME , \
				DATA_SIZE INTEGER DEFAULT 0 , \
				NETWORK_STATUS INTEGER DEFAULT 0 , \
				READ_STATUS INTEGER DEFAULT 0 , \
				PROTECTED INTEGER DEFAULT 0 , \
				PRIORITY INTEGER DEFAULT 0 , \
				MSG_DIRECTION INTEGER NOT NULL , \
				SCHEDULED_TIME DATETIME , \
				BACKUP INTEGER DEFAULT 0 , \
				SUBJECT TEXT , \
				MSG_DATA TEXT , \
				THUMB_PATH TEXT , \
				MSG_TEXT TEXT , \
				DELIVERY_REPORT_STATUS INTEGER DEFAULT 0 , \
				DELIVERY_REPORT_TIME DATETIME , \
				READ_REPORT_STATUS INTEGER DEFAULT 0 , \
				READ_REPORT_TIME DATETIME , \
				ATTACHMENT_COUNT INTEGER DEFAULT 0 , \
				FOREIGN KEY(CONV_ID) REFERENCES %s (CONV_ID) , \
				FOREIGN KEY(FOLDER_ID) REFERENCES %s (FOLDER_ID) );",
				MSGFW_MESSAGE_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME, MSGFW_FOLDER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_MESSAGE_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_MESSAGE_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateSimMessageTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SIM_MSG_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				SIM_ID INTEGER NOT NULL , \
				FOREIGN KEY(MSG_ID) REFERENCES %s (MSG_ID) );",
				MSGFW_SIM_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_SIM_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SIM_MSG_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateWAPMessageTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_PUSH_MSG_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				ACTION INTEGER , \
				CREATED INTEGER , \
				EXPIRES INTEGER , \
				ID TEXT , \
				HREF TEXT , \
				CONTENT TEXT , \
				FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID) );",
				MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_PUSH_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_PUSH_MSG_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateCBMessageTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_CB_MSG_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				CB_MSG_ID INTEGER NOT NULL , \
				FOREIGN KEY(MSG_ID) REFERENCES %s (MSG_ID) );",
				MSGFW_CB_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_CB_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_CB_MSG_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateSyncMLMessageTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SYNCML_MSG_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				EXT_ID INTEGER NOT NULL , \
				PINCODE INTEGER NOT NULL , \
				FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID) );",
				MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_SYNCML_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SYNCML_MSG_TABLE_NAME, err);
	}

	return err;
}

msg_error_t MsgCreateSmsSendOptTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SMS_SENDOPT_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				DELREP_REQ INTEGER NOT NULL , \
				KEEP_COPY INTEGER NOT NULL , \
				REPLY_PATH INTEGER NOT NULL , \
				FOREIGN KEY(MSG_ID) REFERENCES %s (MSG_ID) );",
				MSGFW_SMS_SENDOPT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
				MSG_DEBUG("SUCCESS : create %s.", MSGFW_SMS_SENDOPT_TABLE_NAME);
		else
				MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SMS_SENDOPT_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateFilterTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_FILTER_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				FILTER_ID INTEGER PRIMARY KEY , \
				FILTER_TYPE INTEGER NOT NULL , \
				FILTER_VALUE TEXT NOT NULL );",
				MSGFW_FILTER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
				MSG_DEBUG("SUCCESS : create %s.", MSGFW_FILTER_TABLE_NAME);
		else
				MSG_DEBUG("FAIL : create %s [%d].", MSGFW_FILTER_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgCreateMmsTable()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MMS_PLUGIN_MESSAGE_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( \
				MSG_ID INTEGER , \
				TRANSACTION_ID TEXT , \
				MESSAGE_ID TEXT , \
				FWD_MESSAGE_ID TEXT , \
				CONTENTS_LOCATION TEXT , \
				FILE_PATH TEXT , \
				VERSION INTEGER NOT NULL , \
				DATA_TYPE INTEGER DEFAULT -1 , \
				DATE DATETIME , \
				HIDE_ADDRESS INTEGER DEFAULT 0 , \
				ASK_DELIVERY_REPORT INTEGER DEFAULT 0 , \
				REPORT_ALLOWED INTEGER DEFAULT 0 , \
				READ_REPORT_ALLOWED_TYPE INTEGER DEFAULT 0 , \
				ASK_READ_REPLY INTEGER DEFAULT 0 , \
				READ INTEGER DEFAULT 0 , \
				READ_REPORT_SEND_STATUS INTEGER DEFAULT 0 , \
				READ_REPORT_SENT INTEGER DEFAULT 0 , \
				PRIORITY INTEGER DEFAULT 0 , \
				KEEP_COPY INTEGER DEFAULT 0 , \
				MSG_SIZE INTEGER NOT NULL , \
				MSG_CLASS INTEGER DEFAULT -1 , \
				EXPIRY_TIME DATETIME , \
				CUSTOM_DELIVERY_TIME INTEGER DEFAULT 0 , \
				DELIVERY_TIME DATETIME , \
				MSG_STATUS INTEGER DEFAULT -1 , \
				FOREIGN KEY(MSG_ID) REFERENCES %s (MSG_ID) );",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MMS_PLUGIN_MESSAGE_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MMS_PLUGIN_MESSAGE_TABLE_NAME, err);
	}

	return err;
}


msg_error_t MsgAddDefaultFolders()
{
	int nRowCnt = 0;
	int nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	// INBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_INBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'INBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_INBOX_ID, MSG_FOLDER_TYPE_INBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// OUTBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_OUTBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'OUTBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_OUTBOX_ID, MSG_FOLDER_TYPE_OUTBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// SENTBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_SENTBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'SENTBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_SENTBOX_ID, MSG_FOLDER_TYPE_OUTBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// DRAFT
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_DRAFT_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'DRAFT', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_DRAFT_ID, MSG_FOLDER_TYPE_DRAFT);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// CBMSGBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_CBMSGBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'CBMSGBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_CBMSGBOX_ID, MSG_FOLDER_TYPE_INBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// SPAMBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_SPAMBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'SPAMBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_SPAMBOX_ID, MSG_FOLDER_TYPE_SPAMBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// SMS TEMPLATE
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_SMS_TEMPLATE_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'SMS TEMPLATE', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_SMS_TEMPLATE_ID, MSG_FOLDER_TYPE_TEMPLATE);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// MMS TEMPLATE
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_MMS_TEMPLATE_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'MMS TEMPLATE', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_MMS_TEMPLATE_ID, MSG_FOLDER_TYPE_TEMPLATE);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgAddDefaultAddress()
{
	MSG_BEGIN();

	int nRowCnt = 0, nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE ADDRESS_ID = 0;",
			MSGFW_ADDRESS_TABLE_NAME);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (0, 0, 0, '', 0, '', '', '', '', 0, 0, 0, 0, 0, 0, 0, 0, '');",
				MSGFW_ADDRESS_TABLE_NAME);

		MSG_DEBUG("%s", sqlQuery);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoResetDatabase()
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	const char* tableList[] = {MSGFW_FOLDER_TABLE_NAME, MSGFW_FILTER_TABLE_NAME,
			MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_SYNCML_MSG_TABLE_NAME,
			MSGFW_SMS_SENDOPT_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char*);

	dbHandle.beginTrans();

	// Delete Database
	for (int i = 0; i < listCnt; i++)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", tableList[i]);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Delete Message Table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE STORAGE_ID <> %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Delete Conversation Table
	err = MsgStoClearConversationTable(&dbHandle);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	// Add Default Folders
	if (MsgAddDefaultFolders() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Folders Fail");
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	// Add Default Address
	if (MsgAddDefaultAddress() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Address Fail");
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	dbHandle.endTrans(true);

	// Delete MMS Files
	MsgRmRf((char*)MSG_DATA_PATH);
	MsgRmRf((char*)MSG_SMIL_FILE_PATH);

	// Reset SMS Count
	if (MsgSettingSetIndicator(0, 0) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

	// Reset MMS Count
	if (MsgSettingSetIndicator(0, 0) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoBackupMessage()
{
	return MSG_SUCCESS;
}

msg_error_t MsgStoUpdateMms(MSG_MESSAGE_INFO_S *pMsg)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	if (pMsg->msgType.subType == MSG_SENDCONF_MMS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		dbHandle.beginTrans();
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET THUMB_PATH = ?, MSG_TEXT = ? WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}


		dbHandle.bindText(pMsg->thumbPath, 1);
		dbHandle.bindText(pMsg->msgText, 2);
		MSG_DEBUG("thumbPath = %s , msgText = %s" , pMsg->thumbPath, pMsg->msgText);
		MSG_DEBUG("%s", sqlQuery);

		if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle.finalizeQuery();
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		dbHandle.finalizeQuery();
		dbHandle.endTrans(true);
	} else {
		MsgStoUpdateMMSMessage(pMsg);
	}
	return MSG_SUCCESS;
}

msg_error_t MsgStoRestoreMessage()
{
	return MSG_SUCCESS;
}
