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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <VMessage.h>
#include <VCard.h>
#include "MsgVMessage.h"

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"

#define MSG_DB_VERSION 1

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoConnectDB()
{
	return MSG_SUCCESS;
}


msg_error_t MsgStoDisconnectDB()
{
	MsgDbHandler *dbHandle = getDbHandle();
	if (dbHandle->disconnect() != MSG_SUCCESS) {
		MSG_DEBUG("DB Disconnect Fail");
		return MSG_ERR_DB_DISCONNECT;
	}

	MSG_DEBUG("DB Disconnect Success");

	return MSG_SUCCESS;
}


void MsgUpdateDBtoVer1()
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];

	if (!dbHandle->checkTableExist(MSGFW_MMS_MULTIPART_TABLE_NAME)) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery),
				"CREATE TABLE %s ( "
				"_ID INTEGER PRIMARY KEY AUTOINCREMENT, "
				"MSG_ID INTEGER NOT NULL , "
				"SEQ INTEGER DEFAULT 0, "
				"CONTENT_TYPE TEXT, "
				"NAME TEXT, "
				"CHARSET INTEGER, "
				"CONTENT_ID TEXT, "
				"CONTENT_LOCATION TEXT, "
				"FILE_PATH TEXT, "
				"TEXT TEXT, "
				"TCS_LEVEL INTEGER DEFAULT -1, "
				"MALWARE_ALLOW INTEGER DEFAULT 0, "
				"THUMB_FILE_PATH TEXT, "
				"FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID));",
				MSGFW_MMS_MULTIPART_TABLE_NAME);

		err = dbHandle->execQuery(sqlQuery);

		if (err == MSG_SUCCESS)
			MSG_SEC_DEBUG("SUCCESS : create %s.", MSGFW_MMS_MULTIPART_TABLE_NAME);
		else
			MSG_SEC_DEBUG("FAIL : create %s [%d].", MSGFW_MMS_MULTIPART_TABLE_NAME, err);
	}
}


void MsgStoUpdateDBVersion()
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	snprintf(sqlQuery, sizeof(sqlQuery), "PRAGMA user_version=%d;", MSG_DB_VERSION);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to prepareQuery.");
		return;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_STEP) {
		MSG_DEBUG("Fail to stepQuery.");
		dbHandle->finalizeQuery();
		return;
	}

	dbHandle->finalizeQuery();
}

msg_error_t MsgStoDBVerCheck()
{
	MsgDbHandler *dbHandle = getDbHandle();
	int dbVersion = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	snprintf(sqlQuery, sizeof(sqlQuery), "PRAGMA user_version;");

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to prepareQuery.");
		return MSG_ERR_DB_EXEC;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_STEP) {
		MSG_DEBUG("Fail to stepQuery.");
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_EXEC;
	}

	dbVersion = dbHandle->columnInt(0);

	dbHandle->finalizeQuery();

	MSG_DEBUG("dbVersion [%d]", dbVersion);

	switch (dbVersion)
	{
	case 0 :
		MsgUpdateDBtoVer1();
		/* no break */
	default :
		MsgStoUpdateDBVersion();
		/* no break */
	}

	return MSG_SUCCESS;
}

void MsgInitMmapMutex(const char *shm_file_name)
{
	MSG_BEGIN();

	if(!shm_file_name) {
		MSG_FATAL("EMAIL_ERROR_INVALID_PARAM");
		return;
	}

	int fd = shm_open (shm_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); /*  note: permission is not working */

	if (fd < 0) {
		MSG_FATAL("shm_open errno [%d]", errno);
		return;
	}

	fchmod(fd, 0666);
	MSG_DEBUG("** Create SHM FILE **");
	if (ftruncate(fd, sizeof(pthread_mutex_t)) != 0) {
		MSG_FATAL("ftruncate errno [%d]", errno);
		return;
	}

	pthread_mutex_t *mx = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mx == MAP_FAILED) {
		MSG_FATAL("mmap errno [%d]", errno);
		return ;
	}

	// initialize the data on mmap
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST_NP);
	pthread_mutex_init(mx, &mattr);
	pthread_mutexattr_destroy(&mattr);

	close (fd);

	if (munmap((void *)mx, sizeof(pthread_mutex_t)) != 0) {
		MSG_FATAL("munmap() failed! (errno: %d)", errno);
		return;
	}

	MSG_END();
}

msg_error_t MsgStoInitDB(bool bSimChanged)
{
	MSG_BEGIN();
//	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	// Init mmap mutex for DB access
	MsgInitMmapMutex(SHM_FILE_FOR_DB_LOCK);

	// Check DB version.
	MsgStoDBVerCheck();

	// Delete Msgs in Hidden Folder
	MsgStoDeleteAllMessageInFolder(0, true, NULL);

	// Reset network status
	MsgStoResetNetworkStatus();

#if 0
	// Reset Cb Message
	MsgStoResetCBMessage();
#endif

	//clear abnormal mms message
	MsgStoCleanAbnormalMmsData();

	// Clear all old Sim data
	MsgStoClearSimMessageInDB();

	//update sim index to 0 for all messages
	MsgStoUpdateIMSI(0);

	MSG_END();

	return err;
}


msg_error_t MsgAddDefaultFolders()
{
	int nRowCnt = 0;
	int nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();
	// INBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_INBOX_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'INBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_INBOX_ID, MSG_FOLDER_TYPE_INBOX);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// OUTBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_OUTBOX_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'OUTBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_OUTBOX_ID, MSG_FOLDER_TYPE_OUTBOX);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// SENTBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_SENTBOX_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'SENTBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_SENTBOX_ID, MSG_FOLDER_TYPE_OUTBOX);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// DRAFT
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_DRAFT_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'DRAFT', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_DRAFT_ID, MSG_FOLDER_TYPE_DRAFT);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// CBMSGBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_CBMSGBOX_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'CBMSGBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_CBMSGBOX_ID, MSG_FOLDER_TYPE_INBOX);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	// SPAMBOX
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE FOLDER_ID = %d;",
			MSGFW_FOLDER_TABLE_NAME, MSG_SPAMBOX_ID);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 'SPAMBOX', %d);",
				MSGFW_FOLDER_TABLE_NAME, MSG_SPAMBOX_ID, MSG_FOLDER_TYPE_SPAMBOX);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgAddDefaultAddress()
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();
	int nRowCnt = 0, nResult = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s WHERE ADDRESS_ID = 0;",
			MSGFW_ADDRESS_TABLE_NAME);

	if (dbHandle->getTable(sqlQuery, &nRowCnt, NULL) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	nResult = dbHandle->getColumnToInt(1);

	dbHandle->freeTable();

	if (nResult == 0) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (0, 0, 0, 0, '', 0, 0, '', '', '', '', '', '', 0);",
				MSGFW_ADDRESS_TABLE_NAME);

		MSG_DEBUG("%s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoResetDatabase()
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	const char* tableList[] = {MSGFW_FOLDER_TABLE_NAME, MSGFW_FILTER_TABLE_NAME,
			MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_SYNCML_MSG_TABLE_NAME,
			MSGFW_SMS_SENDOPT_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char*);

	dbHandle->beginTrans();

	// Delete Database
	for (int i = 0; i < listCnt; i++)
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", tableList[i]);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	// Delete Message Table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE STORAGE_ID <> %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Delete Conversation Table
	err = MsgStoClearConversationTable(dbHandle);

	if (err != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return err;
	}

	// Add Default Folders
	if (MsgAddDefaultFolders() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Folders Fail");
		dbHandle->endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	// Add Default Address
	if (MsgAddDefaultAddress() != MSG_SUCCESS) {
		MSG_DEBUG("Add Default Address Fail");
		dbHandle->endTrans(false);
		return MSG_ERR_DB_STORAGE_INIT;
	}

	dbHandle->endTrans(true);

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


msg_error_t MsgStoBackupMessage(msg_message_backup_type_t type, const char *filepath)
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t	err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0;
	MSG_MESSAGE_INFO_S msgInfo = {0, };
	char* encoded_data = NULL;

	char fileName[MSG_FILENAME_LEN_MAX+1];
	memset(fileName, 0x00, sizeof(fileName));
	strncpy(fileName, filepath, MSG_FILENAME_LEN_MAX);
	if (remove(fileName) != 0) {
		MSG_SEC_DEBUG("Fail to delete [%s].", fileName);
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MSG_SEC_DEBUG("backup type = %d, path = %s", type, filepath);

	if (type == MSG_BACKUP_TYPE_SMS) {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT MSG_ID FROM %s "
				"WHERE STORAGE_ID = %d AND MAIN_TYPE = %d;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_PHONE, MSG_SMS_TYPE);
	} else if (type == MSG_BACKUP_TYPE_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT MSG_ID FROM %s "
				"WHERE STORAGE_ID = %d AND MAIN_TYPE = %d;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_PHONE, MSG_MMS_TYPE);
	} else if (type == MSG_BACKUP_TYPE_ALL) {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT MSG_ID FROM %s "
				"WHERE STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_PHONE);

	}

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS) {
		dbHandle->freeTable();
		return err;
	}

	MSG_DEBUG("backup number = %d", rowCnt);

	int msg_id[rowCnt];
	for (int i = 0; i < rowCnt; i++) {
		msg_id[i] = dbHandle->getColumnToInt(i+1);
	}
	dbHandle->freeTable();

	for (int i = 0; i < rowCnt; i++) {
		msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

		err = MsgStoGetMessage(msg_id[i], &msgInfo, NULL);
		if(err != MSG_SUCCESS) {
			return err;
		}

		encoded_data = MsgVMessageAddRecord(dbHandle, &msgInfo);

		if (msgInfo.bTextSms == false)
		{
			MsgDeleteFile(msgInfo.msgData); //ipc
		}
		if (encoded_data != NULL) {
			if (MsgAppendFile(fileName, encoded_data, strlen(encoded_data)) == false) {
				free(encoded_data);
				return MSG_ERR_STORAGE_ERROR;
			}

			free(encoded_data);

			if (chmod(fileName, 0666) == -1) {
				MSG_FATAL("chmod: %s", g_strerror(errno));
				return MSG_ERR_UNKNOWN;
			}
		}

		memset(&msgInfo, 0, sizeof(MSG_MESSAGE_INFO_S));
	}

	MSG_END();
	return MSG_SUCCESS;

}

msg_error_t MsgStoUpdateMms(MSG_MESSAGE_INFO_S *pMsg)
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];
	unsigned int fileSize = 0;
	char *pFileData = NULL;

	if (pMsg->msgType.subType == MSG_SENDCONF_MMS) {

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		dbHandle->beginTrans();
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET THUMB_PATH = ?, MSG_TEXT = ? WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		dbHandle->bindText(pMsg->thumbPath, 1);

		if (pMsg->msgText[0] != '\0' && g_file_get_contents(pMsg->msgText, &pFileData, &fileSize, NULL) == true) {
			dbHandle->bindText(pFileData, 2);
		}

		MSG_SEC_DEBUG("thumbPath = %s , msgText = %s" , pMsg->thumbPath, pFileData);

		MSG_DEBUG("%s", sqlQuery);

		if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			if (pFileData) {
				free(pFileData);
				pFileData = NULL;
			}

			return MSG_ERR_DB_EXEC;
		}

		dbHandle->finalizeQuery();
		dbHandle->endTrans(true);

		if (pFileData) {
			free(pFileData);
			pFileData = NULL;
		}
	} else {

		err = MsgStoUpdateMMSMessage(pMsg);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoUpdateMMSMessage() error : %d", err);
		}

	}

	return err;
}

msg_error_t MsgStoRestoreMessage(const char *filepath, msg_id_list_s **result_id_list)
{
	if (result_id_list == NULL) {
		MSG_DEBUG("result_id_list is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	msg_error_t err = MSG_SUCCESS;
	MSG_MESSAGE_INFO_S msgInfo = {0,};
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	VTree* vMsg = NULL;
	VObject* pObject = NULL;

	msg_id_list_s *msgIdList = NULL;
	msgIdList = (msg_id_list_s *)calloc(1, sizeof(msg_id_list_s));

	int dataSize = 0;

	char fileName[MSG_FILENAME_LEN_MAX+1];
	char *pData = NULL;
	char *pCurrent = NULL;
	char *pTemp = NULL;

	*result_id_list = NULL;

#ifdef MSG_FOR_DEBUG
	char sample[10000] = "BEGIN:VMSG\r\nX-MESSAGE-TYPE:SMS\r\nX-IRMC-BOX:INBOX\r\nX-SS-DT:20100709T155811Z\r\nBEGIN:VBODY\r\nX-BODY-SUBJECT:hekseh\r\nX-BODY-CONTENTS;ENCODING=BASE64:aGVsbG93b3JsZA==\r\nEND:VBODY\r\nBEGIN:VCARD\r\nVERSION:2.1\r\nTEL:01736510664\r\nEND:VCARD\r\nEND:VMSG\r\n";
	vMsg = vmsg_decode(sample);
#else
	memset(fileName, 0x00, sizeof(fileName));
	strncpy(fileName, filepath, MSG_FILENAME_LEN_MAX);
	pData = MsgOpenAndReadMmsFile(fileName, 0, -1, &dataSize);
	if (pData == NULL) {
		if (msgIdList) {
			if (msgIdList->msgIdList)
				free(msgIdList->msgIdList);
			free(msgIdList);
		}
		return MSG_ERR_STORAGE_ERROR;
	}

	pCurrent = pData;

	while ((pTemp = strstr(pCurrent, "END:VMSG")) != NULL)
	{
		MSG_DEBUG("Start Position: %s", pCurrent);

		while (*pCurrent == '\r' || *pCurrent == '\n')
			pCurrent++;

		MSG_DEBUG("Start Position2: %s", pCurrent);

		vMsg = vmsg_decode(pCurrent);
		if (vMsg == NULL) {
			err = MSG_ERR_STORAGE_ERROR;
			goto __RETURN;
		}
#endif

		pObject = vMsg->pTop;

		memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

		while (1)
		{
			while (1)
			{
				MSG_DEBUG("pObject type [%d], pObject Value [%s]", pObject->property, pObject->pszValue[0]);

				switch (pObject->property)
				{
					case VMSG_TYPE_MSGTYPE :
					{
						if (!strncmp(pObject->pszValue[0], "SMS", strlen("SMS"))) {
							msgInfo.msgType.mainType = MSG_SMS_TYPE;
							msgInfo.msgType.subType = MSG_NORMAL_SMS;
						} else if (!strncmp(pObject->pszValue[0], "MMS RETRIEVED", strlen("MMS RETRIEVED"))) {
							msgInfo.msgType.mainType = MSG_MMS_TYPE;
							msgInfo.msgType.subType = MSG_RETRIEVE_AUTOCONF_MMS;
						} else if (!strncmp(pObject->pszValue[0], "MMS SEND", strlen("MMS SEND"))) {
							msgInfo.msgType.mainType = MSG_MMS_TYPE;
							msgInfo.msgType.subType = MSG_SENDCONF_MMS;
						} else if (!strncmp(pObject->pszValue[0], "MMS NOTIFICATION", strlen("MMS NOTIFICATION"))) {
							msgInfo.msgType.mainType = MSG_MMS_TYPE;
							msgInfo.msgType.subType = MSG_NOTIFICATIONIND_MMS;
						} else {
							vmsg_free_vtree_memory(vMsg);
							err = MSG_ERR_STORAGE_ERROR;
							goto __RETURN;
						}
					}
					break;

					case VMSG_TYPE_MSGBOX :
					{
						if(!strncmp(pObject->pszValue[0], "INBOX", strlen("INBOX"))) {
							msgInfo.folderId= MSG_INBOX_ID;
							msgInfo.direction=MSG_DIRECTION_TYPE_MT;

							if (msgInfo.msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS)
								msgInfo.networkStatus=MSG_NETWORK_RETRIEVE_SUCCESS;
							else if (msgInfo.msgType.subType == MSG_SENDCONF_MMS)
								msgInfo.networkStatus=MSG_NETWORK_SEND_SUCCESS;
							else
								msgInfo.networkStatus=MSG_NETWORK_RECEIVED;

						} else if(!strncmp(pObject->pszValue[0], "OUTBOX", strlen("OUTBOX"))) {
							msgInfo.folderId= MSG_OUTBOX_ID;
							msgInfo.direction=MSG_DIRECTION_TYPE_MO;

							msgInfo.networkStatus=MSG_NETWORK_SEND_FAIL;
						} else if(!strncmp(pObject->pszValue[0], "SENTBOX", strlen("SENTBOX"))) {
							msgInfo.folderId= MSG_SENTBOX_ID;
							msgInfo.direction=MSG_DIRECTION_TYPE_MO;

							msgInfo.networkStatus=MSG_NETWORK_SEND_SUCCESS;
						} else if(!strncmp(pObject->pszValue[0], "DRAFTBOX", strlen("DRAFTBOX"))) {
							msgInfo.folderId=MSG_DRAFT_ID;
							msgInfo.direction=MSG_DIRECTION_TYPE_MO;

							msgInfo.networkStatus=MSG_NETWORK_NOT_SEND;
						} else {
							vmsg_free_vtree_memory(vMsg);
							err = MSG_ERR_STORAGE_ERROR;
							goto __RETURN;
						}
					}
					break;

					case VMSG_TYPE_STATUS :
					{
						if(!strncmp(pObject->pszValue[0], "READ", strlen("READ"))) {
							msgInfo.bRead = true;
						} else if(!strncmp(pObject->pszValue[0], "UNREAD", strlen("UNREAD"))) {
							msgInfo.bRead = false;
						} else {
							vmsg_free_vtree_memory(vMsg);
							err = MSG_ERR_STORAGE_ERROR;
							goto __RETURN;
						}
					}
					break;

					case VMSG_TYPE_DATE :
					{
						struct tm	displayTime;

						if (!_convert_vdata_str_to_tm(pObject->pszValue[0], &displayTime)) {
							vmsg_free_vtree_memory( vMsg );
							err = MSG_ERR_STORAGE_ERROR;
							goto __RETURN;
						}

						msgInfo.displayTime = mktime(&displayTime);
					}
					break;

					case VMSG_TYPE_SUBJECT :
					{
						MSG_DEBUG("subject length is [%d].", strlen(pObject->pszValue[0]));

						if(strlen(pObject->pszValue[0]) > 0) {
							strncpy(msgInfo.subject, pObject->pszValue[0], MAX_SUBJECT_LEN);
							if ( msgInfo.subject[strlen(pObject->pszValue[0])-1] == '\r' )
								msgInfo.subject[strlen(pObject->pszValue[0])-1]= '\0';
						}
					}
					break;

					case VMSG_TYPE_BODY :
					{
						if (msgInfo.msgType.mainType == MSG_SMS_TYPE) {
							if (pObject->numOfBiData > MAX_MSG_DATA_LEN) {
								msgInfo.bTextSms = false;
								char fileName[MAX_COMMON_INFO_SIZE + 1];
								memset(fileName, 0x00, sizeof(fileName));

								if (MsgCreateFileName(fileName) == false) {
									vmsg_free_vtree_memory(vMsg);
									err = MSG_ERR_STORAGE_ERROR;
									goto __RETURN;
								}

								if (MsgWriteIpcFile(fileName, pObject->pszValue[0], pObject->numOfBiData) == false) {
									vmsg_free_vtree_memory(vMsg);
									err = MSG_ERR_STORAGE_ERROR;
									goto __RETURN;
								}

								strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);
								msgInfo.dataSize = pObject->numOfBiData;
							} else {
								msgInfo.bTextSms = true;

								if(pObject->numOfBiData > 0) {
									memset(msgInfo.msgText, 0x00, sizeof(msgInfo.msgText));
									memcpy(msgInfo.msgText, pObject->pszValue[0], pObject->numOfBiData);

									msgInfo.dataSize = pObject->numOfBiData;
								}
							}
						} else {
							msgInfo.bTextSms = true;
#if 0
							if(msgInfo.msgType.subType == MSG_NOTIFICATIONIND_MMS) {

									msgInfo.bTextSms = true;

								// Save Message Data into File
								char fileName[MAX_COMMON_INFO_SIZE+1];
								memset(fileName, 0x00, sizeof(fileName));

								if (MsgCreateFileName(fileName) == false) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_STORAGE_ERROR;
								}

								if (MsgWriteIpcFile(fileName, pObject->pszValue[0], pObject->numOfBiData) == false) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_STORAGE_ERROR;
								}
								strncpy(msgInfo.msgData, MSG_IPC_DATA_PATH, MAX_MSG_DATA_LEN);
								strncat(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN-strlen(msgInfo.msgData));
								msgInfo.dataSize = strlen(fileName);
								MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(msgInfo.msgType.mainType);
								if (plg == NULL) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_NULL_POINTER;
								}
								err =  plg->restoreMsg(&msgInfo, pObject->pszValue[0], pObject->numOfBiData, NULL);

							} else {
//////////////// From here was avaliable
								char	retrievedFilePath[MAX_FULL_PATH_SIZE] = {0,};
								MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(msgInfo.msgType.mainType);
								if (plg == NULL) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_NULL_POINTER;
								}
								err =  plg->restoreMsg(&msgInfo, pObject->pszValue[0], pObject->numOfBiData, retrievedFilePath);
								msgInfo.bTextSms = false;

								char fileName[MAX_COMMON_INFO_SIZE+1];
								memset(fileName, 0x00, sizeof(fileName));

								if (MsgCreateFileName(fileName) == false) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_STORAGE_ERROR;
								}
								MSG_SEC_DEBUG("fileName: %s, retrievedFilePath: %s (%d)", fileName, retrievedFilePath, strlen(retrievedFilePath));

								if (MsgWriteIpcFile(fileName, retrievedFilePath, strlen(retrievedFilePath)+ 1) == false) {
									vmsg_free_vtree_memory(vMsg);
									return MSG_ERR_STORAGE_ERROR;
								}
								strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);
								msgInfo.dataSize = strlen(retrievedFilePath) + 1;

								if (err != MSG_SUCCESS)
									return vmsg_free_vtree_memory(vMsg);
///////////////////////////
							}
#else

							msgInfo.bTextSms = false;

							char fileName[MAX_COMMON_INFO_SIZE+1];
							memset(fileName, 0x00, sizeof(fileName));

							if (MsgCreateFileName(fileName) == false) {
								vmsg_free_vtree_memory(vMsg);
								err = MSG_ERR_STORAGE_ERROR;
								goto __RETURN;
							}

							if (MsgWriteIpcFile(fileName, pObject->pszValue[0], pObject->numOfBiData) == false) {
								vmsg_free_vtree_memory(vMsg);
								err = MSG_ERR_STORAGE_ERROR;
								goto __RETURN;
							}

							//set serialized mms data ipcfilename
							strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);
#endif
						}
					}
					break;

					case VCARD_TYPE_TEL :
					{
						MSG_ADDRESS_INFO_S * addrInfo = NULL;

						msgInfo.nAddressCnt++;

						if (msgInfo.addressList == NULL) {
							addrInfo = (MSG_ADDRESS_INFO_S *)calloc(1, sizeof(MSG_ADDRESS_INFO_S));
						} else {
							addrInfo = (MSG_ADDRESS_INFO_S *)realloc(msgInfo.addressList, msgInfo.nAddressCnt * sizeof(MSG_ADDRESS_INFO_S));
						}

						if (addrInfo == NULL) {
							vmsg_free_vtree_memory(vMsg);
							err = MSG_ERR_STORAGE_ERROR;
							goto __RETURN;

						}

						msgInfo.addressList = addrInfo;

						msgInfo.addressList[msgInfo.nAddressCnt-1].addressType = MSG_ADDRESS_TYPE_PLMN;
						msgInfo.addressList[msgInfo.nAddressCnt-1].recipientType = MSG_RECIPIENTS_TYPE_TO;
						strncpy(msgInfo.addressList[msgInfo.nAddressCnt-1].addressVal, pObject->pszValue[0], MAX_ADDRESS_VAL_LEN);

					}
					break;
				}

				if (pObject->pSibling != NULL)
					pObject = pObject->pSibling;
				else
					break;
			}

			if (vMsg->pNext != NULL) {
				vMsg = vMsg->pNext;
				pObject = vMsg->pTop;
			} else {
				break;
			}
		}

		msgInfo.bBackup = true; // Set Backup Flag
		msgInfo.storageId = MSG_STORAGE_PHONE; // Set Storage Id
		msgInfo.priority = MSG_MESSAGE_PRIORITY_NORMAL; // Set Priority

		err = MsgStoAddMessage(&msgInfo, NULL);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddMessage() error : %d", err);
		}

		if (msgInfo.msgType.mainType == MSG_MMS_TYPE)
			MsgStoUpdateMms(&msgInfo);

		if (msgIdList->nCount == 0) {
			msgIdList->msgIdList = (msg_message_id_t*)calloc(1, sizeof(msg_message_id_t));
		} else {
			msg_message_id_t * msg_id_list;
			msg_id_list = (msg_message_id_t*)realloc(msgIdList->msgIdList, sizeof(msg_message_id_t)*(msgIdList->nCount+1));

			if (msg_id_list)
				msgIdList->msgIdList = msg_id_list;
			else {
				MSG_DEBUG("realloc failed");
				err = MSG_ERR_UNKNOWN;
				goto __RETURN;
			}
		}

		msgIdList->msgIdList[msgIdList->nCount] = msgInfo.msgId;
		msgIdList->nCount++;

		vmsg_free_vtree_memory(vMsg);

		vMsg = NULL;
		pCurrent = pTemp + strlen("END:VMSG");
#ifndef MSG_FOR_DEBUG
	}
#endif
	*result_id_list = msgIdList;

__RETURN:
	if(pData)
	{
		free( pData );
		pData = NULL;
		pCurrent = NULL;
	}

	if (*result_id_list == NULL && msgIdList) {
		if (msgIdList->msgIdList) {
			free(msgIdList->msgIdList);
		}
		free(msgIdList);
	}

	return err;
}

msg_error_t MsgStoAddPushEvent(MSG_PUSH_EVENT_INFO_S* pPushEvent)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];
	unsigned int rowId = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	// Check whether Same record exists or not.
#if 0
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE CONTENT_TYPE LIKE '%s' AND APP_ID LIKE '%s' AND (PKG_NAME LIKE '%s' OR SECURE = 1);",
			MSGFW_PUSH_CONFIG_TABLE_NAME, pPushEvent->contentType, pPushEvent->appId, pPushEvent->pkgName);
#else
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE CONTENT_TYPE LIKE '%s' AND APP_ID LIKE '%s';",
			MSGFW_PUSH_CONFIG_TABLE_NAME, pPushEvent->contentType, pPushEvent->appId);
#endif

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_ROW;
	}
	dbHandle->finalizeQuery();

	dbHandle->beginTrans();

	if (dbHandle->getRowId(MSGFW_PUSH_CONFIG_TABLE_NAME, &rowId) != MSG_SUCCESS) {
		MSG_DEBUG("getRowId is failed!!!");
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, ?, ?, ?, %d, 0, 0);",
			MSGFW_PUSH_CONFIG_TABLE_NAME, rowId, pPushEvent->bLaunch);


	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->bindText(pPushEvent->contentType, 1);
	dbHandle->bindText(pPushEvent->appId, 2);
	dbHandle->bindText(pPushEvent->pkgName, 3);

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();
	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeletePushEvent(MSG_PUSH_EVENT_INFO_S* pPushEvent)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];
	dbHandle->beginTrans();
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE CONTENT_TYPE LIKE '%s' AND APP_ID LIKE '%s' AND PKG_NAME LIKE '%s';",
			MSGFW_PUSH_CONFIG_TABLE_NAME, pPushEvent->contentType, pPushEvent->appId, pPushEvent->pkgName);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}
	dbHandle->endTrans(true);
	return MSG_SUCCESS;
}

msg_error_t MsgStoUpdatePushEvent(MSG_PUSH_EVENT_INFO_S* pSrc, MSG_PUSH_EVENT_INFO_S* pDst)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];
	dbHandle->beginTrans();
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET CONTENT_TYPE = ?, APP_ID = ?, PKG_NAME = ?, LAUNCH = %d WHERE CONTENT_TYPE LIKE '%s' AND APP_ID LIKE '%s' AND PKG_NAME LIKE '%s';",
			MSGFW_PUSH_CONFIG_TABLE_NAME, pDst->bLaunch, pSrc->contentType, pSrc->appId, pSrc->pkgName);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->bindText(pDst->contentType, 1);
	dbHandle->bindText(pDst->appId, 2);
	dbHandle->bindText(pDst->pkgName, 3);

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();
	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}
