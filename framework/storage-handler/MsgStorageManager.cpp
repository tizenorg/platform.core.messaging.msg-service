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
MSG_ERROR_T MsgStoConnectDB()
{
	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDisconnectDB()
{
	if (dbHandle.disconnect() != MSG_SUCCESS)
	{
		MSG_DEBUG("DB Disconnect Fail");
		return MSG_ERR_DB_DISCONNECT;
	}

	MSG_DEBUG("DB Disconnect Success");

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoInitDB(bool bSimChanged)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

#ifdef MSG_DB_CREATE
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
	if (MsgCreateScheduledMessageTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateSmsSendOptTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateFilterTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateMmsMsgTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;
	if (MsgCreateMmsAttributeTable() != MSG_SUCCESS)
		return MSG_ERR_DB_STORAGE_INIT;

	// Add Default Folders
	if (MsgAddDefaultFolders() != MSG_SUCCESS)
	{
		MSG_DEBUG("Add Default Folders Fail");
		return MSG_ERR_DB_STORAGE_INIT;
	}

	// Add Default Address
	if (MsgAddDefaultAddress() != MSG_SUCCESS)
	{
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


MSG_ERROR_T MsgCreateAddressTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_ADDRESS_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  ADDRESS_ID INTEGER PRIMARY KEY, \
						  ADDRESS_TYPE INTEGER, \
						  RECIPIENT_TYPE INTEGER, \
						  ADDRESS_VAL TEXT, \
						  CONTACT_ID INTEGER, \
						  DISPLAY_NAME TEXT, \
						  FIRST_NAME TEXT, \
						  LAST_NAME TEXT, \
						  IMAGE_PATH TEXT, \
						  SYNC_TIME DATETIME, \
						  UNREAD_CNT INTEGER DEFAULT 0, \
						  SMS_CNT INTEGER DEFAULT 0, \
						  MMS_CNT INTEGER DEFAULT 0, \
						  MAIN_TYPE INTEGER NOT NULL, \
						  SUB_TYPE INTEGER NOT NULL, \
						  MSG_DIRECTION INTEGER NOT NULL, \
						  MSG_TIME DATETIME, \
  						  MSG_TEXT TEXT);",
						MSGFW_ADDRESS_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_ADDRESS_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_ADDRESS_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateFolderTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_FOLDER_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  FOLDER_ID INTEGER PRIMARY KEY, \
						  FOLDER_NAME TEXT NOT NULL, \
						  FOLDER_TYPE INTEGER DEFAULT 0);",
						  MSGFW_FOLDER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_FOLDER_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_FOLDER_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateMsgTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_MESSAGE_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER PRIMARY KEY, \
						  ADDRESS_ID INTEGER, \
						  FOLDER_ID INTEGER, \
						  REFERENCE_ID INTEGER, \
						  STORAGE_ID INTEGER NOT NULL, \
						  MAIN_TYPE INTEGER NOT NULL, \
						  SUB_TYPE INTEGER NOT NULL, \
						  DISPLAY_TIME DATETIME, \
						  DATA_SIZE INTEGER DEFAULT 0, \
						  NETWORK_STATUS INTEGER DEFAULT 0, \
						  READ_STATUS INTEGER DEFAULT 0, \
						  PROTECTED INTEGER DEFAULT 0, \
						  PRIORITY INTEGER DEFAULT 0, \
						  MSG_DIRECTION INTEGER NOT NULL, \
						  SCHEDULED_TIME DATETIME, \
						  BACKUP INTEGER DEFAULT 0, \
						  SUBJECT TEXT, \
						  MSG_DATA TEXT, \
						  THUMB_PATH TEXT, \
						  MSG_TEXT TEXT, \
						  DELIVERY_REPORT_STATUS INTEGER DEFAULT 0, \
						  DELIVERY_REPORT_TIME DATETIME, \
						  READ_REPORT_STATUS INTEGER DEFAULT 0, \
						  READ_REPORT_TIME DATETIME, \
						  ATTACHMENT_COUNT INTEGER DEFAULT 0, \
						  FOREIGN KEY(ADDRESS_ID) REFERENCES %s(ADDRESS_ID), \
						  FOREIGN KEY(FOLDER_ID) REFERENCES %s(FOLDER_ID) \
						  );",
						  MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MSGFW_FOLDER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_MESSAGE_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_MESSAGE_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateSimMessageTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SIM_MSG_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  SIM_ID INTEGER NOT NULL, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_SIM_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_SIM_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SIM_MSG_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateWAPMessageTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_PUSH_MSG_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  ACTION INTEGER, \
						  CREATED INTEGER, \
						  EXPIRES INTEGER, \
						  ID TEXT, \
						  HREF TEXT, \
						  CONTENT TEXT, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_PUSH_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_PUSH_MSG_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateCBMessageTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_CB_MSG_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  CB_MSG_ID INTEGER NOT NULL, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_CB_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_CB_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_CB_MSG_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateSyncMLMessageTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SYNCML_MSG_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  EXT_ID INTEGER NOT NULL, \
						  PINCODE INTEGER NOT NULL, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_SYNCML_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SYNCML_MSG_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateScheduledMessageTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SCHEDULED_MSG_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  ALARM_ID INTEGER NOT NULL, \
						  LISTENER_FD INTEGER NOT NULL, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_SCHEDULED_MSG_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MSGFW_SCHEDULED_MSG_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SCHEDULED_MSG_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateSmsSendOptTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_SMS_SENDOPT_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  DELREP_REQ INTEGER NOT NULL, \
						  KEEP_COPY INTEGER NOT NULL, \
						  REPLY_PATH INTEGER NOT NULL, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MSGFW_SMS_SENDOPT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
				MSG_DEBUG("SUCCESS : create %s.", MSGFW_SMS_SENDOPT_TABLE_NAME);
		else
				MSG_DEBUG("FAIL : create %s [%d].", MSGFW_SMS_SENDOPT_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateFilterTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MSGFW_FILTER_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  FILTER_ID INTEGER PRIMARY KEY, \
						  FILTER_TYPE INTEGER NOT NULL, \
						  FILTER_VALUE TEXT NOT NULL);",
						  MSGFW_FILTER_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
				MSG_DEBUG("SUCCESS : create %s.", MSGFW_FILTER_TABLE_NAME);
		else
				MSG_DEBUG("FAIL : create %s [%d].", MSGFW_FILTER_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateMmsMsgTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MMS_PLUGIN_MESSAGE_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
						  MSG_ID INTEGER, \
						  TRANSACTION_ID TEXT, \
						  MESSAGE_ID TEXT, \
						  FWD_MESSAGE_ID TEXT, \
						  CONTENTS_LOCATION TEXT, \
						  FILE_PATH TEXT, \
						  FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						  MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MMS_PLUGIN_MESSAGE_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MMS_PLUGIN_MESSAGE_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgCreateMmsAttributeTable()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle.checkTableExist(MMS_PLUGIN_ATTRIBUTE_TABLE_NAME))
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				  		"CREATE TABLE %s ( \
				  		 MSG_ID INTEGER, \
				  	 	 VERSION INTEGER NOT NULL, \
						 DATA_TYPE INTEGER DEFAULT -1, \
						 DATE DATETIME, \
						 HIDE_ADDRESS INTEGER DEFAULT 0, \
						 ASK_DELIVERY_REPORT INTEGER DEFAULT 0, \
						 REPORT_ALLOWED INTEGER DEFAULT 0, \
						 READ_REPORT_ALLOWED_TYPE INTEGER DEFAULT 0, \
						 ASK_READ_REPLY INTEGER DEFAULT 0, \
						 READ INTEGER DEFAULT 0, \
						 READ_REPORT_SEND_STATUS	INTEGER DEFAULT 0, \
						 READ_REPORT_SENT INTEGER DEFAULT 0, \
						 PRIORITY INTEGER DEFAULT 0, \
						 KEEP_COPY INTEGER DEFAULT 0, \
					 	 MSG_SIZE INTEGER NOT NULL, \
						 MSG_CLASS INTEGER DEFAULT -1, \
						 EXPIRY_TIME DATETIME, \
					 	 CUSTOM_DELIVERY_TIME INTEGER DEFAULT 0, \
						 DELIVERY_TIME DATETIME, \
						 MSG_STATUS INTEGER DEFAULT -1, \
						 FOREIGN KEY(MSG_ID) REFERENCES %s(MSG_ID));",
						 MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

		err = dbHandle.execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("SUCCESS : create %s.", MMS_PLUGIN_ATTRIBUTE_TABLE_NAME);
		else
			MSG_DEBUG("FAIL : create %s [%d].", MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, err);
	}

	return err;
}


MSG_ERROR_T MsgAddDefaultFolders()
{
	int nRowCnt = 0;
	int nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	// INBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
					MSGFW_FOLDER_TABLE_NAME, MSG_INBOX_ID);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
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

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
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

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
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

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
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

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'CBMSGBOX', %d);",
					MSGFW_FOLDER_TABLE_NAME, MSG_CBMSGBOX_ID, MSG_FOLDER_TYPE_INBOX);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgAddDefaultAddress()
{
	MSG_BEGIN();

	int nRowCnt = 0, nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE ADDRESS_ID = 0;",
					MSGFW_ADDRESS_TABLE_NAME);

	if (dbHandle.getTable(sqlQuery, &nRowCnt) != MSG_SUCCESS)
	{
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (nResult == 0)
	{
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


MSG_ERROR_T MsgStoResetDatabase()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	const char* tableList[] = {MSGFW_FOLDER_TABLE_NAME, MSGFW_FILTER_TABLE_NAME,
						MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MMS_PLUGIN_ATTRIBUTE_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SCHEDULED_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char*);

	dbHandle.beginTrans();

	// Delete Database
	for (int i = 0; i < listCnt; i++)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", tableList[i]);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		{
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Delete Message Table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE STORAGE_ID <> %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Delete Address Table
	err = MsgStoClearAddressTable(&dbHandle);

	if (err != MSG_SUCCESS)
	{
		dbHandle.endTrans(false);
		return err;
	}

	// Add Default Folders
	if (MsgAddDefaultFolders() != MSG_SUCCESS)
	{
		MSG_DEBUG("Add Default Folders Fail");
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	// Add Default Address
	if (MsgAddDefaultAddress() != MSG_SUCCESS)
	{
		MSG_DEBUG("Add Default Address Fail");
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	dbHandle.endTrans(true);

	// Delete MMS Files
	MsgRmRf((char*)MSG_DATA_PATH);
	MsgRmRf((char*)MSG_SMIL_FILE_PATH);

	// Reset SMS Count
	if (MsgSettingSetIndicator(0, 0) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

	// Reset MMS Count
	if (MsgSettingSetIndicator(0, 0) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgSettingSetIndicator() FAILED");
		return MSG_ERR_SET_SETTING;
	}

	return MSG_SUCCESS;
}
