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
#include "MsgCppTypes.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgSqliteWrapper.h"
#include "MsgStorageHandler.h"
#include "MsgContact.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoGetText(msg_message_id_t msgId, char *pSubject, char *pMsgText)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SUBJECT, MSG_TEXT FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if(dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if(dbHandle.stepQuery() == MSG_ERR_DB_ROW) {

		char *subject = (char*)dbHandle.columnText(0);
		char *text = (char*)dbHandle.columnText(1);

		if(subject)
			strncpy(pSubject, subject, MAX_SUBJECT_LEN);
		if(text)
			strncpy(pMsgText, text, MAX_MSG_TEXT_LEN);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;

}



msg_error_t MsgStoUpdateMMSMessage(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	dbHandle.beginTrans();

	if(pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		if( pMsg->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS ) {
			snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, DISPLAY_TIME = %lu, SUBJECT = ?, NETWORK_STATUS = %d, MSG_TEXT = ?, THUMB_PATH = '%s', DATA_SIZE = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->displayTime, pMsg->networkStatus, pMsg->thumbPath,  pMsg->dataSize, pMsg->msgId);

			if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_PREPARE;
			}

			dbHandle.bindText(pMsg->subject, 1);
			dbHandle.bindText(pMsg->msgText, 2);
		} else if( pMsg->networkStatus == MSG_NETWORK_RETRIEVE_FAIL) {
			snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, SUBJECT = ?, NETWORK_STATUS = %d, MSG_TEXT = ?, THUMB_PATH = '%s' WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->networkStatus, pMsg->thumbPath,  pMsg->msgId);

			if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_PREPARE;
			}

			dbHandle.bindText(pMsg->subject, 1);
			dbHandle.bindText(pMsg->msgText, 2);
		}
	} else if (pMsg->msgType.subType == MSG_SENDREQ_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"UPDATE %s SET MSG_DATA = '%s', MSG_TEXT = ?, THUMB_PATH = '%s', DATA_SIZE = %d WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsg->msgData, pMsg->thumbPath, pMsg->dataSize, pMsg->msgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		dbHandle.bindText(pMsg->msgText, 1);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, FOLDER_ID = %d, NETWORK_STATUS = %d WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->folderId, pMsg->networkStatus, pMsg->msgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}
	}

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.finalizeQuery();
		dbHandle.endTrans(false);
		MSG_DEBUG("Update MMS Message. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	msg_thread_id_t convId = 0;
	int row = 0;

	// Get SUB_TYPE, STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	if (dbHandle.getTable(sqlQuery, &row) != MSG_SUCCESS) {
		dbHandle.freeTable();
		dbHandle.endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	if (row > 0) {
		convId = dbHandle.getColumnToInt(1);

		MSG_DEBUG("Conversation id:[%d]", convId);

		if (MsgStoUpdateConversation(&dbHandle, convId) != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoUpdateConversation() Error");
			dbHandle.freeTable();
			dbHandle.endTrans(false);

			return MSG_ERR_STORAGE_ERROR;
		}

	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
		dbHandle.freeTable();
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.freeTable();
	dbHandle.endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetContentLocation(MSG_MESSAGE_INFO_S* pMsgInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONTENTS_LOCATION FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle.columnText(0) != NULL) {
			strncpy(pMsgInfo->msgData, (char*)dbHandle.columnText(0), MAX_MSG_DATA_LEN);
			pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
		} else {
			dbHandle.finalizeQuery();
			return MSG_ERR_DB_NORECORD;
		}
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoSetReadReportSendStatus(msg_message_id_t msgId, int readReportSendStatus)
{
	bool bReadReportSent = false;

	if((MmsRecvReadReportSendStatus)readReportSendStatus == MMS_RECEIVE_READ_REPORT_SENT)
		bReadReportSent = true;
	else
		bReadReportSent = false;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_REPORT_SEND_STATUS = %d, READ_REPORT_SENT = %d WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, (MmsRecvReadReportSendStatus)readReportSendStatus, (int)bReadReportSent, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetOrgAddressList(MSG_MESSAGE_INFO_S *pMsg)
{
	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0;
	int index = 3;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_TYPE, A.RECIPIENT_TYPE, A.ADDRESS_VAL FROM %s A, %s B \
			WHERE A.CONV_ID = B.CONV_ID AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return err;
	}

	for(int i = 0; i < rowCnt; i++)
	{
		pMsg->addressList[i].addressType = dbHandle.getColumnToInt(index++);
		pMsg->addressList[i].recipientType = dbHandle.getColumnToInt(index++);
		dbHandle.getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pMsg->addressList[i].addressVal);
	}

	pMsg->nAddressCnt = rowCnt;

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetSubject(msg_message_id_t msgId, char *pSubject)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SUBJECT FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		strncpy(pSubject, (char*)dbHandle.columnText(0), MAX_SUBJECT_LEN);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetRecipientList(msg_message_id_t msgId, MSG_RECIPIENTS_LIST_S *pRecipientList)
{
	if (pRecipientList == NULL) {
		MSG_DEBUG("pRecipientList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0, index = 7;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.ADDRESS_TYPE, B.RECIPIENT_TYPE, \
			B.ADDRESS_VAL, B.CONTACT_ID, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME \
			FROM %s A, %s B WHERE A.MSG_ID = %d AND A.CONV_ID = B.CONV_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, msgId);

	if (dbHandle.getTable(sqlQuery, &rowCnt) != MSG_SUCCESS) {
		dbHandle.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	pRecipientList->recipientCnt= rowCnt;

	MSG_DEBUG("pRecipientList->recipientCnt [%d]", pRecipientList->recipientCnt);

	pRecipientList->recipientAddr= (MSG_ADDRESS_INFO_S*)new char[sizeof(MSG_ADDRESS_INFO_S)*rowCnt];

	MSG_ADDRESS_INFO_S* pTmp = pRecipientList->recipientAddr;

	char firstName[MAX_THREAD_NAME_LEN+1], lastName[MAX_THREAD_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];

	int order = MsgGetContactNameOrder();

	for (int i = 0; i < rowCnt; i++)
	{
		pTmp->addressType = dbHandle.getColumnToInt(index++);
		pTmp->recipientType= dbHandle.getColumnToInt(index++);

		memset(pTmp->addressVal, 0x00, sizeof(pTmp->addressVal));
		dbHandle.getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pTmp->addressVal);

		pTmp->contactId= dbHandle.getColumnToInt(index++);

		memset(displayName, 0x00, sizeof(displayName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, displayName);

		memset(firstName, 0x00, sizeof(firstName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, firstName);

		memset(lastName, 0x00, sizeof(lastName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, lastName);

		if (strlen(displayName) <= 0) {
			if (order == 0) {
				if (firstName[0] != '\0')
					strncpy(displayName, firstName, MAX_DISPLAY_NAME_LEN);

				if (lastName[0] != '\0') {
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
					strncat(displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			} else if (order == 1) {
				if (lastName[0] != '\0') {
					strncpy(displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}

				if (firstName[0] != '\0')
					strncat(displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
			}
		}

		memset(pTmp->displayName, 0x00, sizeof(pTmp->displayName));
		strncpy(pTmp->displayName, displayName, MAX_DISPLAY_NAME_LEN);

		pTmp++;
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetReadStatus(msg_message_id_t msgId, bool *pReadStatus)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT READ_STATUS FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		*pReadStatus = (bool)dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddrInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	// Add Address
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_TYPE, A.RECIPIENT_TYPE, A.CONTACT_ID, A.ADDRESS_VAL \
			FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pAddrInfo->addressType = dbHandle.columnInt(0);
		pAddrInfo->recipientType = dbHandle.columnInt(1);
		pAddrInfo->contactId = dbHandle.columnInt(2);

		strncpy(pAddrInfo->addressVal, (char*)dbHandle.columnText(3), MAX_ADDRESS_VAL_LEN);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}
