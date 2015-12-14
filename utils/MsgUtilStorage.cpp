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
#include <string.h>
#include <stdlib.h>
#include <tr1/unordered_set>
#include <queue>
#include <glib.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFunction.h"
#include "MsgUtilStorage.h"

#include <storage.h>

static int msgCntLimit[MSG_COUNT_LIMIT_MAILBOX_TYPE_MAX][MSG_COUNT_LIMIT_MSG_TYPE_MAX] = {{10, 10, 0, 10, 10}, {5, 10, 0, 0, 0}, {10, 10, 0, 0, 0}, {10, 10, 0, 0, 0}, {0, 0, 10, 0, 0}};

using namespace std;

#define ITERATION_SIZE 200
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

unsigned int MsgStoAddMessageTable(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	msg_message_id_t msgId = 0;

	err = pDbHandle->getRowId(MSGFW_MESSAGE_TABLE_NAME, &msgId);

	if (err != MSG_SUCCESS)
		return 0;

	int fileSize = 0;

	char* pFileData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

	/* Get File Data */
	if (pMsgInfo->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
			return 0;

		MSG_DEBUG("file size [%d]", fileSize);
	}

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SUBS_ID, pMsgInfo->sim_idx);

	char *imsi = MsgSettingGetString(keyName);

	/* Add Message */
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %u, %d, %d, %d, %d, %d, %d, %d, %d, ?, '', '', ?, 0, %d, '%s');",
			MSGFW_MESSAGE_TABLE_NAME, msgId, pMsgInfo->threadId, pMsgInfo->folderId, pMsgInfo->storageId, pMsgInfo->msgType.mainType,
			pMsgInfo->msgType.subType, (unsigned int)pMsgInfo->displayTime, pMsgInfo->dataSize, pMsgInfo->networkStatus, pMsgInfo->bRead, pMsgInfo->bProtected,
			pMsgInfo->priority, pMsgInfo->direction, 0, pMsgInfo->bBackup, pMsgInfo->sim_idx, imsi);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	g_free(imsi);
	imsi = NULL;

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return 0;

	pDbHandle->bindText(pMsgInfo->subject, 1);

	if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		pDbHandle->bindText("", 2);
	} else {
		if (pMsgInfo->bTextSms == false)
			pDbHandle->bindText(pFileData, 2);
		else
			pDbHandle->bindText(pMsgInfo->msgText, 2);
	}

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		return 0;
	}

	pDbHandle->finalizeQuery();

	return msgId;
}


msg_error_t MsgStoSetReadStatus(MsgDbHandler *pDbHandle, msg_message_id_t msgId, bool bRead)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_STATUS = %d WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, (int)bRead, msgId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	/* Get MAIN_TYPE, SUB_TYPE, STORAGE_ID */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MAIN_TYPE, A.SUB_TYPE, B.CONV_ID \
			FROM %s A, %s B WHERE A.MSG_ID = %d AND A.CONV_ID = B.CONV_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	MSG_MESSAGE_TYPE_S msgType;
	msg_thread_id_t convId;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = pDbHandle->columnInt(0);
		msgType.subType = pDbHandle->columnInt(1);
		convId = pDbHandle->columnInt(2);
	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	MSG_DEBUG("Main Type:[%d] SubType:[%d] ConvId:[%d]", msgType.mainType, msgType.subType, convId);

	if (MsgStoUpdateConversation(pDbHandle, convId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoUpdateConversation() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetOldestMessage(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t *pMsgId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s \
			WHERE SUB_TYPE = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND PROTECTED = 0 \
			ORDER BY DISPLAY_TIME ASC",
			MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgType.subType, pMsgInfo->folderId, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		*pMsgId = pDbHandle->columnInt(0);
	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoCheckMsgCntFull(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t folderId)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	struct statvfs s;
	double freeSpace;
	int r;
	r = storage_get_internal_memory_size(&s);
	if (r < 0)
		return MSG_ERR_STORAGE_ERROR;
	else {
		freeSpace = (double)s.f_bsize*s.f_bavail;
		MSG_DEBUG("Free space of storage is [%lu] MB.", freeSpace);

		if (freeSpace < SMS_MINIMUM_SPACE && pMsgType->mainType == MSG_SMS_TYPE)
			err = MSG_ERR_MESSAGE_COUNT_FULL;
		else if (freeSpace < MMS_MINIMUM_SPACE && pMsgType->mainType == MSG_MMS_TYPE)
			err = MSG_ERR_MESSAGE_COUNT_FULL;
	}

	MSG_END();

	return err;
}


msg_error_t MsgStoCountMsgByLimitCategory(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount, msg_folder_id_t folderId)
{
	if (pMsgType == NULL) {
		MSG_DEBUG("pMsgType is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	*pMsgCount = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_WAP_SI_SMS ||pMsgType->subType == MSG_WAP_SL_SMS)) { /* PUSH */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_INBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_CB_SMS)) { /* CB */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_CB_SMS, MSG_CBMSGBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_SYNCML_CP)) { /* Provision */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SYNCML_CP, MSG_INBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE)) { /* SMS */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_CB_SMS, MSG_SYNCML_CP, MSG_INBOX_ID); /* etc SMS */
	} else if ((pMsgType->mainType == MSG_MMS_TYPE) &&
			(pMsgType->subType == MSG_SENDREQ_MMS || pMsgType->subType == MSG_SENDCONF_MMS || pMsgType->subType == MSG_RETRIEVE_AUTOCONF_MMS ||
					pMsgType->subType == MSG_RETRIEVE_MANUALCONF_MMS || pMsgType->subType == MSG_NOTIFICATIONIND_MMS)) { /* MMS */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SENDREQ_MMS, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS, MSG_NOTIFICATIONIND_MMS, folderId);
	} else {
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		*pMsgCount = pDbHandle->columnInt(0);
	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


int MsgStoCheckMsgCntLimit(const MSG_MESSAGE_TYPE_S* pMsgType, msg_folder_id_t folderId)
{
	int msgboxType = -1;
	int msgType = -1;

	switch (folderId) {
		case MSG_INBOX_ID :
			msgboxType = MSG_COUNT_LIMIT_INBOX_TYPE;
		break;

		case MSG_OUTBOX_ID :
			msgboxType = MSG_COUNT_LIMIT_OUTBOX_TYPE;
		break;

		case MSG_SENTBOX_ID :
			msgboxType = MSG_COUNT_LIMIT_SENTBOX_TYPE;
		break;

		case MSG_DRAFT_ID :
			msgboxType = MSG_COUNT_LIMIT_DRAFTBOX_TYPE;
		break;

		case MSG_CBMSGBOX_ID :
			msgboxType = MSG_COUNT_LIMIT_CBMSGBOX_TYPE;
		break;

		default:
			MSG_DEBUG("Unknown mailbox Type [%d]", folderId);
		return -1;
	}

	switch (pMsgType->subType) {
		case MSG_NORMAL_SMS:
		case MSG_REPLACE_TYPE1_SMS:
		case MSG_REPLACE_TYPE2_SMS:
		case MSG_REPLACE_TYPE3_SMS:
		case MSG_REPLACE_TYPE4_SMS:
		case MSG_REPLACE_TYPE5_SMS:
		case MSG_REPLACE_TYPE6_SMS:
		case MSG_REPLACE_TYPE7_SMS:
		case MSG_MWI_VOICE_SMS:
		case MSG_MWI_FAX_SMS:
		case MSG_MWI_EMAIL_SMS:
		case MSG_MWI_OTHER_SMS:
		case MSG_STATUS_REPORT_SMS:
			msgType = MSG_COUNT_LIMIT_SMS_TYPE;
		break;

		case MSG_CB_SMS:
			msgType = MSG_COUNT_LIMIT_CB_TYPE;
		break;

		case MSG_WAP_SI_SMS:
		case MSG_WAP_SL_SMS:
			msgType = MSG_COUNT_LIMIT_WAPPUSH_TYPE;
		break;

		case MSG_SYNCML_CP:
			msgType = MSG_COUNT_LIMIT_PROVISION_TYPE;
		break;

		case MSG_SENDREQ_MMS:
		case MSG_SENDCONF_MMS:
		case MSG_NOTIFICATIONIND_MMS:
		case MSG_RETRIEVE_AUTOCONF_MMS:
		case MSG_RETRIEVE_MANUALCONF_MMS:
			msgType = MSG_COUNT_LIMIT_MMS_TYPE;
		break;

		default:
			MSG_DEBUG("Unknown Message Type [%d]", pMsgType->subType);
		return -1;
	}

	return msgCntLimit[msgboxType][msgType];
}


msg_error_t MsgStocheckMemoryStatus()
{
	msg_error_t err = MSG_SUCCESS;
	struct statvfs s;
	double freeSpace;
	int r;
	r = storage_get_internal_memory_size(&s);
	if (r < 0)
		return MSG_ERR_STORAGE_ERROR;
	else {
		freeSpace = (double)s.f_bsize*s.f_bavail;
	    MSG_DEBUG("Free space of storage is [%ul] MB.", freeSpace);
		if (freeSpace < SMS_MINIMUM_SPACE)
			err = MSG_ERR_MESSAGE_COUNT_FULL;
	}

	MSG_DEBUG("Memory status =[%d]", err);

	return err;
}


msg_error_t MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	/* Check if new address or not */
	if (MsgExistAddress(pDbHandle, pMsg, pConvId) == true) {
		MSG_DEBUG("The address already exists. Conversation ID : [%d]", *pConvId);
		MsgStoUpdateAddress(pDbHandle, pMsg, *pConvId);
	} else {
		*pConvId = 0;

		if (pMsg->threadId)
			*pConvId = pMsg->threadId;

		/* conversation insert */
		err = MsgStoAddConversation(pDbHandle, pConvId);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddConversation() fail [%d]", err);
			return err;
		}

		/* insert address in loop */
		for (int i = 0; i < pMsg->nAddressCnt; i++) {
			unsigned int addrId;
			MSG_CONTACT_INFO_S contactInfo;
			memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

			/* Get Contact Info */
#if 0
			if (MsgGetContactInfo(&(pMsg->addressList[i]), &contactInfo) != MSG_SUCCESS) {
				MSG_DEBUG("MsgGetContactInfo() fail.");
			}
#endif

			err = pDbHandle->getRowId(MSGFW_ADDRESS_TABLE_NAME, &addrId);
			if (err != MSG_SUCCESS) {
				MSG_DEBUG("pDbHandle->getRowId fail. [%d]", err);
				return err;
			}

			/* Add Address */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, '%s', %d, %d, ?, ?, ?, ?, ?, '%s', 0);",
						MSGFW_ADDRESS_TABLE_NAME, addrId, *pConvId, pMsg->addressList[i].addressType, pMsg->addressList[i].recipientType, pMsg->addressList[i].addressVal,
						contactInfo.contactId, contactInfo.addrbookId, contactInfo.imagePath);

			MSG_SEC_DEBUG("Add Address Info. [%s]", sqlQuery);

			if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
				return MSG_ERR_DB_PREPARE;

			pDbHandle->bindText(contactInfo.firstName, 1);
			pDbHandle->bindText(contactInfo.lastName, 2);
			pDbHandle->bindText(contactInfo.middleName, 3);
			pDbHandle->bindText(contactInfo.prefix, 4);
			pDbHandle->bindText(contactInfo.suffix, 5);

			if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
				pDbHandle->finalizeQuery();
				return MSG_ERR_DB_STEP;
			}

			pDbHandle->finalizeQuery();
		}
	}

	/* set conversation display name by conv id */
	MsgStoSetConversationDisplayName(pDbHandle, *pConvId);

	return err;
}

msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, int *nAddressCnt, MSG_ADDRESS_INFO_S **pAddress)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0, index = 0;

	*nAddressCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"A.ADDRESS_TYPE, "
			"A.RECIPIENT_TYPE, "
			"A.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.CONV_ID = B.CONV_ID "
			"AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
			msgId);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return err;
	}

	*nAddressCnt = rowCnt;

	MSG_DEBUG("*nAddressCnt [%d]", *nAddressCnt);

	MSG_ADDRESS_INFO_S *tmpAddressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * rowCnt];
	memset(tmpAddressList, 0x00, sizeof(MSG_ADDRESS_INFO_S) * rowCnt);
	*pAddress = tmpAddressList;

	for (int i = 0; i < rowCnt; i++) {
		tmpAddressList[i].addressType = pDbHandle->getColumnToInt(index++);
		tmpAddressList[i].recipientType = pDbHandle->getColumnToInt(index++);
		pDbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, tmpAddressList[i].addressVal);

		strncpy(tmpAddressList[i].displayName, tmpAddressList[i].addressVal, MAX_DISPLAY_NAME_LEN);
	}
	pDbHandle->freeTable();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, msg_struct_list_s *pAddress)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	int rowCnt = 0, index = 0;

	pAddress->nCount = 0;
	pAddress->msg_struct_info = NULL;

	msg_struct_s *pTmp = NULL;
	MSG_ADDRESS_INFO_S *pAddr = NULL;

	pAddress->msg_struct_info = (msg_struct_t *)calloc(MAX_TO_ADDRESS_CNT, sizeof(msg_struct_t));
	if (pAddress->msg_struct_info == NULL)
		return MSG_ERR_MEMORY_ERROR;

	for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
		pAddress->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
		pTmp = (msg_struct_s *)pAddress->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_ADDRESS_INFO;
		pTmp->data = new MSG_ADDRESS_INFO_S;
		memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"A.ADDRESS_TYPE, "
			"A.RECIPIENT_TYPE, "
			"A.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.CONV_ID = B.CONV_ID "
			"AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
			msgId);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return err;
	}

	rowCnt = (rowCnt > 10)? MAX_TO_ADDRESS_CNT: rowCnt;
	pAddress->nCount = rowCnt;

	for (int i = 0; i < rowCnt; i++) {
		pTmp = (msg_struct_s *)pAddress->msg_struct_info[i];
		pAddr = (MSG_ADDRESS_INFO_S *)pTmp->data;

		pAddr->addressType = pDbHandle->getColumnToInt(index++);
		pAddr->recipientType = pDbHandle->getColumnToInt(index++);

		pDbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pAddr->addressVal);

		strncpy(pAddr->displayName, pAddr->addressVal, MAX_DISPLAY_NAME_LEN);
	}

	pDbHandle->freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddressByConvId(MsgDbHandler *pDbHandle, msg_thread_id_t convId, int contactNameOrder, msg_struct_list_s *pAddrlist)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	int rowCnt = 0, index = 0;

	pAddrlist->nCount = 0;
	pAddrlist->msg_struct_info = NULL;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"ADDRESS_TYPE, "
			"RECIPIENT_TYPE, "
			"ADDRESS_VAL "
			"FROM %s WHERE CONV_ID  = %d;",
			MSGFW_ADDRESS_TABLE_NAME, convId);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return err;
	}

	pAddrlist->nCount = rowCnt;

	MSG_DEBUG("pAddrlist->nCount [%d]", pAddrlist->nCount);

	msg_struct_s *pTmp = NULL;
	MSG_ADDRESS_INFO_S *pAddr = NULL;

	pAddrlist->msg_struct_info = (msg_struct_t *)calloc(rowCnt, sizeof(msg_struct_t));

	for (int i = 0; i < rowCnt; i++) {
		pAddrlist->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
		pTmp = (msg_struct_s *)pAddrlist->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_ADDRESS_INFO;
		pTmp->data = new MSG_ADDRESS_INFO_S;
		memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
	}

/*	rowCnt = (rowCnt > 10)? MAX_TO_ADDRESS_CNT: rowCnt; */

	for (int i = 0; i < rowCnt; i++) {
		pTmp = (msg_struct_s *)pAddrlist->msg_struct_info[i];
		pAddr = (MSG_ADDRESS_INFO_S *)pTmp->data;

		pAddr->addressType = pDbHandle->getColumnToInt(index++);
		pAddr->recipientType = pDbHandle->getColumnToInt(index++);
		pDbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pAddr->addressVal);

		strncpy(pAddr->displayName, pAddr->addressVal, MAX_DISPLAY_NAME_LEN);
	}
	pDbHandle->freeTable();

	return MSG_SUCCESS;
}

/* Have to use trigger for this function. */
msg_error_t MsgStoUpdateConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN];
	unsigned int tmpSize = 0;

#ifdef MSG_NOTI_INTEGRATION
	memset(sqlQuery, 0x00, MAX_QUERY_LEN);
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT * "
			"FROM %s "
			"WHERE CONV_ID = %d "
			"AND FOLDER_ID > %d AND FOLDER_ID < %d "
			"AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME,
			convId,
			MSG_ALLBOX_ID, MSG_SPAMBOX_ID,
			MSG_STORAGE_PHONE);
#else
	memset(sqlQuery, 0x00, MAX_QUERY_LEN);
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT * "
			"FROM %s "
			"WHERE CONV_ID = %d "
			"AND FOLDER_ID > %d AND FOLDER_ID < %d "
			"AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME,
			convId,
			MSG_ALLBOX_ID, MSG_CBMSGBOX_ID,
			MSG_STORAGE_PHONE);
#endif

	msg_error_t err = pDbHandle->prepareQuery(sqlQuery);
	if (err != MSG_SUCCESS) {
			MSG_DEBUG("Fail to prepareQuery().");
			pDbHandle->finalizeQuery();
			return err;
	}

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pDbHandle->finalizeQuery();

		memset(sqlQuery, 0x00, MAX_QUERY_LEN);
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT MAIN_TYPE, SUB_TYPE, MSG_DIRECTION, DISPLAY_TIME, LENGTH(SUBJECT), SUBJECT, MSG_TEXT "
				"FROM %s "
				"WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME,
				convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);

		err = pDbHandle->prepareQuery(sqlQuery);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Fail to prepareQuery().");
			return err;
		}

		err = pDbHandle->stepQuery();
		if (err != MSG_ERR_DB_ROW) {
			MSG_DEBUG("Fail to stepQuery().");
			pDbHandle->finalizeQuery();
			return err;
		}

		int main_type = pDbHandle->columnInt(0);
		int sub_type = pDbHandle->columnInt(1);
		int msg_direction = pDbHandle->columnInt(2);
		time_t disp_time = (time_t)pDbHandle->columnInt(3);
		int subject_length = pDbHandle->columnInt(4);
		char subject[MAX_SUBJECT_LEN+1] = {0, };
		snprintf(subject, sizeof(subject), "%s", pDbHandle->columnText(5));
		char msg_text[MAX_MSG_TEXT_LEN+1] = {0, };
		snprintf(msg_text, sizeof(msg_text), "%s", pDbHandle->columnText(6));

		pDbHandle->finalizeQuery();
		memset(sqlQuery, 0x00, MAX_QUERY_LEN);
		snprintf(sqlQuery, sizeof(sqlQuery),
				"UPDATE %s SET ",
				MSGFW_CONVERSATION_TABLE_NAME);

		tmpSize = strlen(sqlQuery);
#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"UNREAD_CNT = (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND (FOLDER_ID = %d OR FOLDER_ID = %d) AND STORAGE_ID = %d AND READ_STATUS = 0), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_INBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);
#else
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"UNREAD_CNT = (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND READ_STATUS = 0), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_INBOX_ID, MSG_STORAGE_PHONE);
#endif

		tmpSize = strlen(sqlQuery);
#ifdef MSG_NOTI_INTEGRATION
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"SMS_CNT = (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND MAIN_TYPE = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_SMS_TYPE, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
#else
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"SMS_CNT = (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND MAIN_TYPE = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_SMS_TYPE, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);
#endif

		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MMS_CNT = (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d) AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);

#if 0
		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MAIN_TYPE = (SELECT MAIN_TYPE FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);

		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"SUB_TYPE = (SELECT SUB_TYPE FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);

		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MSG_DIRECTION = (SELECT MSG_DIRECTION FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC), ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);

		tmpSize = strlen(sqlQuery);
#if 1
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"DISPLAY_TIME = CASE "
				"WHEN (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) > 0 "
				"THEN (SELECT DISPLAY_TIME FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) "
				"ELSE 0 "
				"END, ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
#else
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"DISPLAY_TIME = CASE "
				"WHEN (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND READ_STATUS = 0) > 0 "
				"THEN (SELECT DISPLAY_TIME FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC) "
				"WHEN (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d) > 0 "
				"THEN (SELECT DISPLAY_TIME FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC) "
				"WHEN (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND NETWORK_STATUS = %d) > 0 "
				"THEN (SELECT DISPLAY_TIME FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND NETWORK_STATUS = %d ORDER BY DISPLAY_TIME DESC) "
				"ELSE (SELECT DISPLAY_TIME FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC) "
				"END, ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_INBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_INBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_DRAFT_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_DRAFT_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_OUTBOX_ID, MSG_STORAGE_PHONE, MSG_NETWORK_SEND_FAIL,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_OUTBOX_ID, MSG_STORAGE_PHONE, MSG_NETWORK_SEND_FAIL,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
#endif

#endif
		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MSG_TEXT = CASE "
				"WHEN %d > 0 THEN ? ELSE ? "
				"END, ",
				subject_length);
#if 0
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MSG_TEXT = CASE "
				"WHEN (SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) > 0 "
				"THEN CASE "
				"WHEN (SELECT LENGTH(SUBJECT) FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) > 0 "
				"THEN (SELECT SUBJECT FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) "
				"ELSE (SELECT MSG_TEXT FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND SCHEDULED_TIME = 0 ORDER BY DISPLAY_TIME DESC) "
				"END ELSE '' "
				"END ",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE,
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
#endif
		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"MAIN_TYPE = %d, SUB_TYPE = %d, MSG_DIRECTION = %d, DISPLAY_TIME = %lu ",
				main_type, sub_type, msg_direction, disp_time);

		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"WHERE CONV_ID = %d;",
				convId);
		if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			return MSG_ERR_DB_PREPARE;
		}

		pDbHandle->bindText(subject, 1);
		pDbHandle->bindText(msg_text, 2);

		if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			MSG_DEBUG("stepQuery() Failed");
			pDbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		pDbHandle->finalizeQuery();
	} else {
		pDbHandle->finalizeQuery();

		memset(sqlQuery, 0x00, MAX_QUERY_LEN);
		snprintf(sqlQuery, sizeof(sqlQuery),
				"UPDATE %s SET UNREAD_CNT = 0, SMS_CNT = 0, MMS_CNT = 0, MAIN_TYPE = 0, SUB_TYPE = 0, MSG_DIRECTION = 0, DISPLAY_TIME = 0, MSG_TEXT = '' "
				"WHERE CONV_ID = %d;",
				MSGFW_CONVERSATION_TABLE_NAME, convId);

		if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


/* consider to replcae this function to trigger. */
msg_error_t MsgStoClearConversationTable(MsgDbHandler *pDbHandle)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s \
			WHERE CONV_ID NOT IN (SELECT CONV_ID FROM %s) AND CONV_ID <> 0;",
			MSGFW_CONVERSATION_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

	err = pDbHandle->execQuery(sqlQuery);

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE CONV_ID NOT IN (SELECT CONV_ID FROM %s);",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME);

	err = pDbHandle->execQuery(sqlQuery);

	return err;
}


msg_thread_id_t MsgGetThreadId(MsgDbHandler *pDbHandle, msg_message_id_t msgId)
{
	msg_thread_id_t conv_id = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return 0;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		conv_id = pDbHandle->columnInt(0);
	}

	pDbHandle->finalizeQuery();

	return conv_id;
}

/* Change the function name to conversation related. */
bool MsgExistAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	unsigned int tmpSize = 0;

	*pConvId = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT CONV_ID FROM ( SELECT CONV_ID FROM %s WHERE ( ",
			MSGFW_ADDRESS_TABLE_NAME);

	for (int i = 0; i < pMsg->nAddressCnt; i++) {
		if (strlen(pMsg->addressList[i].addressVal) >= (unsigned int)MsgContactGetMinMatchDigit()
				&& pMsg->addressList[i].addressType != MSG_ADDRESS_TYPE_EMAIL
				&& MsgIsNumber(pMsg->addressList[i].addressVal)) {
			int addrSize = strlen(pMsg->addressList[i].addressVal);
			char newPhoneNum[addrSize+1];
			memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
			MsgConvertNumber(pMsg->addressList[i].addressVal, newPhoneNum, addrSize);

			tmpSize = strlen(sqlQuery);
			snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
					"ADDRESS_VAL LIKE '%%%%%s' ",
					newPhoneNum);

			if ((pMsg->nAddressCnt-1) == i) break;

			tmpSize = strlen(sqlQuery);
			snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize, " OR ");

		} else {
			tmpSize = strlen(sqlQuery);
			snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
					"ADDRESS_VAL LIKE '%s' ",
					pMsg->addressList[i].addressVal);

			if ((pMsg->nAddressCnt-1) == i) break;

			tmpSize = strlen(sqlQuery);
			snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize, " OR ");
		}
	}

	tmpSize = strlen(sqlQuery);
	snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
			") AND CONV_ID IN (SELECT CONV_ID FROM %s GROUP BY CONV_ID HAVING COUNT(CONV_ID)=%d) ",
			MSGFW_ADDRESS_TABLE_NAME, pMsg->nAddressCnt);


	tmpSize = strlen(sqlQuery);
	snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
			") GROUP BY CONV_ID HAVING COUNT(CONV_ID)=%d;",
			pMsg->nAddressCnt);

	int rowCnt = 0;
	int convId = 0;

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	/* No record or other error */
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return false;
	}

	convId = pDbHandle->getColumnToInt(1);

	if (convId > 0) {
		MSG_DEBUG("Success  to get convId [%d]", convId);
		*pConvId = convId;
		pDbHandle->freeTable();
		return true;
	}

	pDbHandle->freeTable();

	return false;
}


int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T msgType)
{
	int msgCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgType == MSG_SMS_TYPE) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s "
				"WHERE MAIN_TYPE = %d "
				"AND (SUB_TYPE IN (%d, %d, %d, %d, %d, %d, %d) OR (SUB_TYPE >= %d AND SUB_TYPE <= %d)) "
				"AND FOLDER_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME,
				MSG_SMS_TYPE,
				MSG_NORMAL_SMS, MSG_STATUS_REPORT_SMS, MSG_CONCAT_SIM_SMS, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_MWI_VOICE_SMS, MSG_SYNCML_CP,
				MSG_REPLACE_TYPE1_SMS, MSG_REPLACE_TYPE7_SMS,
				MSG_INBOX_ID, MSG_STORAGE_PHONE);
	} else if (msgType == MSG_MMS_TYPE) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s "
				"WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d) "
				"AND FOLDER_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME,
				MSG_MMS_TYPE,
				MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS, MSG_NOTIFICATIONIND_MMS,
				MSG_INBOX_ID, MSG_STORAGE_PHONE);
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return 0;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgCnt = pDbHandle->columnInt(0);
	} else {
		pDbHandle->finalizeQuery();
		return 0;
	}

	pDbHandle->finalizeQuery();

	return msgCnt;
}


msg_error_t MsgStoGetMmsRawFilePath(MsgDbHandler *pDbHandle, msg_message_id_t msgId, char *pFilePath)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.FILE_PATH FROM %s A, %s B \
			WHERE A.MSG_ID = B.MSG_ID AND B.MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (pDbHandle->columnText(0) != NULL)
			strncpy(pFilePath, (char*)pDbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


bool MsgStoCheckReadReportRequested(MsgDbHandler *pDbHandle, msg_message_id_t msgId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0;
	bool bReadReportRequested = false;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ASK_READ_REPLY FROM %s A, %s B \
			WHERE A.MSG_ID = B.MSG_ID AND B.MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return bReadReportRequested;
	}

	if (rowCnt != 1) {
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return bReadReportRequested;
	}

	bReadReportRequested = pDbHandle->getColumnToInt(1);

	pDbHandle->freeTable();

	return bReadReportRequested;
}


bool MsgStoCheckReadReportIsSent(MsgDbHandler *pDbHandle, msg_message_id_t msgId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	int rowCnt = 0;
	bool bReadReportIsSent = true;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.READ_REPORT_SENT FROM %s A, %s B \
			WHERE A.MSG_ID = B.MSG_ID AND B.MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return bReadReportIsSent;
	}

	if (rowCnt != 1) {
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return bReadReportIsSent;
	}

	bReadReportIsSent = (bool)pDbHandle->getColumnToInt(1);

	pDbHandle->freeTable();

	return bReadReportIsSent;
}


msg_error_t MsgStoAddConversation(MsgDbHandler *pDbHandle, msg_thread_id_t *pConvId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	if(*pConvId == 0) {
		if (pDbHandle->getRowId(MSGFW_CONVERSATION_TABLE_NAME, pConvId) != MSG_SUCCESS) {
			return MSG_ERR_DB_EXEC;
		}
	}
	/* Add Conversation */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, 0, 0, 0, 0, 0, 0, 0, '', '');",
			MSGFW_CONVERSATION_TABLE_NAME, *pConvId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed. [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, int contactId)
{
	msg_error_t err = MSG_SUCCESS;
	int rowCnt = 0;
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	MSG_DEBUG("contactId [%d]", contactId);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE CONTACT_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, contactId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		MSG_DEBUG("Fail to getTable().");
		return err;
	}

	/*contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	int order = MsgGetContactNameOrder();
#else /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	int order = 0;
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	msg_struct_s *pAddrInfo = NULL;
	MSG_ADDRESS_INFO_S *address = NULL;

	for (int i = 1; i <= rowCnt; i++) {
		memset(displayName, 0x00, sizeof(displayName));
		MsgDbHandler tmpDbHandle;
		msg_struct_list_s addressList = {0, };
		MsgStoGetAddressByConvId(&tmpDbHandle, (msg_thread_id_t)pDbHandle->getColumnToInt(i), order, &addressList);

		for (int j = 0; j < addressList.nCount; j++) {
			if (j >0)
				strncat(displayName, ", ", MAX_DISPLAY_NAME_LEN-strlen(displayName));

			pAddrInfo = (msg_struct_s *)addressList.msg_struct_info[j];
			address = (MSG_ADDRESS_INFO_S *)pAddrInfo->data;

			if (address->displayName[0] == '\0')
				strncat(displayName, address->addressVal, MAX_DISPLAY_NAME_LEN-strlen(displayName));
			else
				strncat(displayName, address->displayName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET DISPLAY_NAME = ? WHERE CONV_ID = %d;",
				MSGFW_CONVERSATION_TABLE_NAME, pDbHandle->getColumnToInt(i));

		if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			pDbHandle->freeTable();
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			return MSG_ERR_DB_PREPARE;
		}

		pDbHandle->bindText(displayName, 1);

		if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			pDbHandle->freeTable();
			pDbHandle->finalizeQuery();
			MSG_SEC_DEBUG("Update Conversation disply name. Fail [%s]", sqlQuery);
			return MSG_ERR_DB_STEP;
		}

		pDbHandle->finalizeQuery();

		/* free address list */
		for (int j = 0; j < addressList.nCount; j++) {
			msg_struct_s *pStruct = (msg_struct_s *)addressList.msg_struct_info[j];
			delete (MSG_ADDRESS_INFO_S *)pStruct->data;
			delete (msg_struct_s *)pStruct;
		}

		if (addressList.msg_struct_info != NULL) {
			g_free((msg_struct_t *)addressList.msg_struct_info);
		}
	}

	pDbHandle->freeTable();

	return err;
}


msg_error_t MsgStoSetConversationDisplayName(MsgDbHandler *pDbHandle, msg_thread_id_t convId)
{
	msg_error_t err = MSG_SUCCESS;

	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_struct_list_s addressList = {0, };

	/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	int order = MsgGetContactNameOrder();
#else /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	int order = 0;
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

	msg_struct_s *pAddrInfo = NULL;
	MSG_ADDRESS_INFO_S *address = NULL;

	memset(displayName, 0x00, sizeof(displayName));

	MsgStoGetAddressByConvId(pDbHandle, convId, order, &addressList);

	for (int j = 0; j < addressList.nCount; j++) {
		if (j >0)
			strncat(displayName, ", ", MAX_DISPLAY_NAME_LEN-strlen(displayName));

		pAddrInfo = (msg_struct_s *)addressList.msg_struct_info[j];
		address = (MSG_ADDRESS_INFO_S *)pAddrInfo->data;

		if (address->displayName[0] == '\0')
			strncat(displayName, address->addressVal, MAX_DISPLAY_NAME_LEN-strlen(displayName));
		else
			strncat(displayName, address->displayName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET DISPLAY_NAME = ? WHERE CONV_ID = %d;",
			MSGFW_CONVERSATION_TABLE_NAME, convId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	pDbHandle->bindText(displayName, 1);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		MSG_SEC_DEBUG("Update Conversation disply name. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	for (int j = 0; j < addressList.nCount; j++) {
		msg_struct_s *pStruct = (msg_struct_s *)addressList.msg_struct_info[j];
		delete (MSG_ADDRESS_INFO_S *)pStruct->data;
		delete (msg_struct_s *)pStruct;
	}

	if (addressList.msg_struct_info != NULL) {
		g_free((msg_struct_t *)addressList.msg_struct_info);
	}

	return err;
}

msg_error_t MsgStoUpdateNetworkStatus(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t status)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, status, pMsgInfo->msgId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		err = MSG_ERR_DB_EXEC;

	pDbHandle->finalizeQuery();

	return err;
}

bool MsgExistConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	int rowCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID FROM %s WHERE CONV_ID = %d;",
			MSGFW_CONVERSATION_TABLE_NAME, convId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return false;
	} else if (err != MSG_SUCCESS) {
		pDbHandle->freeTable();
		return false;
	}
	pDbHandle->freeTable();

	return true;
}

bool MsgExistMessage(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsg)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];


	int rowCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE CONV_ID = %ud AND DISPLAY_TIME = %ud;",
			MSGFW_MESSAGE_TABLE_NAME, pMsg->threadId, (int)pMsg->displayTime);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);


	if (err != MSG_SUCCESS) {
		pDbHandle->freeTable();
		return false;
	}

	if(rowCnt > 0) {
		pMsg->msgId = pDbHandle->getColumnToInt(1);
	}
	pDbHandle->freeTable();

	return true;
}


bool MsgExistAddress(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsg,  msg_thread_id_t convId, int index)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];


	int rowCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (strlen(pMsg->addressList[index].addressVal) > (unsigned int)MsgContactGetMinMatchDigit()) {
		int addrSize = strlen(pMsg->addressList[index].addressVal);
		char newPhoneNum[addrSize+1];
		memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
		MsgConvertNumber(pMsg->addressList[index].addressVal, newPhoneNum, addrSize);

		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT ADDRESS_ID FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s' AND CONV_ID= %d;",
				MSGFW_ADDRESS_TABLE_NAME, newPhoneNum, convId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT ADDRESS_ID FROM %s WHERE ADDRESS_VAL = '%s' AND CONV_ID= %d;",
				MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList[index].addressVal, convId);
	}

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return false;
	}
	else if (err != MSG_SUCCESS) {
		pDbHandle->freeTable();
		return false;
	}
	pDbHandle->freeTable();

	return true;
}


void MsgStoUpdateAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t convId)
{
	MSG_BEGIN();
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	for (int i = 0; i < pMsg->nAddressCnt; i++) {
			if (strlen(pMsg->addressList[i].addressVal) > (unsigned int)MsgContactGetMinMatchDigit() && pMsg->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				int addrSize = strlen(pMsg->addressList[i].addressVal);
				char newPhoneNum[addrSize+1];
				memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
				MsgConvertNumber(pMsg->addressList[i].addressVal, newPhoneNum, addrSize);

				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery),
						"UPDATE %s SET "
						"ADDRESS_VAL = '%s', "
						"ADDRESS_TYPE = %d, "
						"RECIPIENT_TYPE = %d "
						"WHERE CONV_ID = %d "
						"AND ADDRESS_VAL LIKE '%%%%%s';",
						MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList[i].addressVal,
						pMsg->addressList[i].addressType, pMsg->addressList[i].recipientType, convId, newPhoneNum);

				err = pDbHandle->execQuery(sqlQuery);
				if (err != MSG_SUCCESS) MSG_DEBUG("Fail to execQuery(). [%s]", sqlQuery);

				pDbHandle->finalizeQuery();
			}
	}

	MSG_END();
}

msg_error_t MsgStoAddCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel, msg_sim_slot_id_t simIndex)
{
#ifndef FEATURE_SMS_CDMA
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN] = {0, };

	pDbHandle->beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE SIM_INDEX = %d;", MSGFW_CB_CHANNEL_INFO_TABLE_NAME, simIndex);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		pDbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	for (int i = 0; i < pCBChannel->channelCnt; i++) {
		int index = 1;
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s(CHANNEL_ACTIVATION, CHANNEL_FROM, CHANNEL_TO, CHANNEL_NAME, SIM_INDEX) VALUES (?, ?, ?, ?, ?);",
				MSGFW_CB_CHANNEL_INFO_TABLE_NAME);

		if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			pDbHandle->endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}
		pDbHandle->bindInt(pCBChannel->channelInfo[i].bActivate, index++);
		pDbHandle->bindInt(pCBChannel->channelInfo[i].from, index++);
		pDbHandle->bindInt(pCBChannel->channelInfo[i].to, index++);
		pDbHandle->bindText(pCBChannel->channelInfo[i].name, index++);
		pDbHandle->bindInt(simIndex, index++);

		if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			pDbHandle->finalizeQuery();
			pDbHandle->endTrans(false);
			return MSG_ERR_DB_STEP;
		}

		pDbHandle->finalizeQuery();
	}

	pDbHandle->endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
#else /* TODO: Add multisim for CDMA */
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN] = {0, };

	pDbHandle->beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", MSGFW_CDMA_CB_CHANNEL_INFO_TABLE_NAME);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		pDbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	for (int i = 0; i < pCBChannel->channelCnt; i++) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, '%s');", MSGFW_CDMA_CB_CHANNEL_INFO_TABLE_NAME,
				i, pCBChannel->channelInfo[i].bActivate, pCBChannel->channelInfo[i].ctg,
				pCBChannel->channelInfo[i].lang, pCBChannel->channelInfo[i].name);

		if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			pDbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	pDbHandle->endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
#endif
}


msg_error_t MsgStoGetCBChannelInfo(MsgDbHandler *pDbHandle, MSG_CB_CHANNEL_S *pCBChannel, msg_sim_slot_id_t simIndex)
{
#ifndef FEATURE_SMS_CDMA
	MSG_BEGIN();

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN] = {0, };

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CHANNEL_ACTIVATION, CHANNEL_FROM, CHANNEL_TO, CHANNEL_NAME FROM %s WHERE SIM_INDEX = %d;", MSGFW_CB_CHANNEL_INFO_TABLE_NAME, simIndex);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);

	pCBChannel->channelCnt = rowCnt;

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_ERR_DB_NORECORD;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	for (int i = 0; i < rowCnt; i++) {
		pCBChannel->channelInfo[i].bActivate = pDbHandle->getColumnToInt(index++);
		pCBChannel->channelInfo[i].from  = pDbHandle->getColumnToInt(index++);
		pCBChannel->channelInfo[i].to = pDbHandle->getColumnToInt(index++);
		pDbHandle->getColumnToString(index++, CB_CHANNEL_NAME_MAX, pCBChannel->channelInfo[i].name);

		MSG_DEBUG("CH_ACT = %d", pCBChannel->channelInfo[i].bActivate);
		MSG_DEBUG("CH_FROM = %d", pCBChannel->channelInfo[i].from);
		MSG_DEBUG("CH_TO = %d", pCBChannel->channelInfo[i].to);
		MSG_DEBUG("CH_NAME = %s", pCBChannel->channelInfo[i].name);
	}

	pDbHandle->freeTable();

	MSG_END();

	return MSG_SUCCESS;
#else /* TODO: Add multisim for CDMA */
	MSG_BEGIN();

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN] = {0, };

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CHANNEL_ACTIVATION, CHANNEL_CATEGORY, CHANNEL_LANGUAGE, CHANNEL_NAME FROM %s;", MSGFW_CDMA_CB_CHANNEL_INFO_TABLE_NAME);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);

	pCBChannel->channelCnt = rowCnt;

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_ERR_DB_NORECORD;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	for (int i = 0; i < rowCnt; i++) {
		pCBChannel->channelInfo[i].bActivate = pDbHandle->getColumnToInt(index++);
		pCBChannel->channelInfo[i].ctg  = pDbHandle->getColumnToInt(index++);
		pCBChannel->channelInfo[i].lang = pDbHandle->getColumnToInt(index++);
		pDbHandle->getColumnToString(index++, CB_CHANNEL_NAME_MAX, pCBChannel->channelInfo[i].name);

		MSG_DEBUG("CH_ACT = %d", pCBChannel->channelInfo[i].bActivate);
		MSG_DEBUG("CH_CTG = %d", pCBChannel->channelInfo[i].ctg);
		MSG_DEBUG("CH_LANG = %d", pCBChannel->channelInfo[i].lang);
		MSG_DEBUG("CH_NAME = %s", pCBChannel->channelInfo[i].name);
	}

	pDbHandle->freeTable();

	MSG_END();

	return MSG_SUCCESS;
#endif
}

msg_error_t MsgStoGetThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList)
{
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	pThreadViewList->nCount = 0;
	pThreadViewList->msg_struct_info = NULL;

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.CONV_ID, A.UNREAD_CNT, A.SMS_CNT, A.MMS_CNT, A.MAIN_TYPE, A.SUB_TYPE, "
			"A.MSG_DIRECTION, A.DISPLAY_TIME, A.DISPLAY_NAME, A.MSG_TEXT, "
			"(COUNT(CASE WHEN B.PROTECTED = 1 THEN 1 END)) AS PROTECTED, "
			"(COUNT(CASE WHEN B.FOLDER_ID = %d THEN 1 END)) AS DRAFT, "
			"(COUNT(CASE WHEN B.NETWORK_STATUS = %d THEN 1 END)) AS FAILED, "
			"(COUNT(CASE WHEN B.NETWORK_STATUS = %d THEN 1 END)) AS SENDING "
			"FROM %s A, %s B ON A.SMS_CNT + A.MMS_CNT > 0 AND B.CONV_ID = A.CONV_ID "
			"GROUP BY A.CONV_ID ORDER BY A.DISPLAY_TIME DESC;",
			MSG_DRAFT_ID,
			MSG_NETWORK_SEND_FAIL,
			MSG_NETWORK_SENDING,
			MSGFW_CONVERSATION_TABLE_NAME,
			MSGFW_MESSAGE_TABLE_NAME);

	msg_error_t err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	if (rowCnt < 1) {
		MSG_DEBUG("rowCnt is %d", rowCnt);
		dbHandle->freeTable();
		return err;
	}

	pThreadViewList->nCount = rowCnt;

	MSG_DEBUG("pThreadViewList->nCount [%d]", pThreadViewList->nCount);

	pThreadViewList->msg_struct_info = (msg_struct_t *)calloc(rowCnt, sizeof(msg_struct_t));

	MSG_THREAD_VIEW_S *pTmp = NULL;
	msg_struct_s *thread_t = NULL;

	for (int i = 0; i < rowCnt; i++) {
		thread_t = (msg_struct_s *)new msg_struct_s;
		pThreadViewList->msg_struct_info[i] = (msg_struct_t)thread_t;

		thread_t->type = MSG_STRUCT_THREAD_INFO;
		thread_t->data = new MSG_THREAD_VIEW_S;

		pTmp = (MSG_THREAD_VIEW_S *)thread_t->data;
		memset(pTmp, 0x00, sizeof(MSG_THREAD_VIEW_S));

		pTmp->threadId = dbHandle->getColumnToInt(index++);

		pTmp->unreadCnt = dbHandle->getColumnToInt(index++);
		pTmp->smsCnt = dbHandle->getColumnToInt(index++);
		pTmp->mmsCnt = dbHandle->getColumnToInt(index++);

		pTmp->mainType = dbHandle->getColumnToInt(index++);
		pTmp->subType = dbHandle->getColumnToInt(index++);

		pTmp->direction = dbHandle->getColumnToInt(index++);
		pTmp->threadTime = (time_t)dbHandle->getColumnToInt(index++);

		memset(pTmp->threadName, 0x00, sizeof(pTmp->threadName));
		dbHandle->getColumnToString(index++, MAX_THREAD_NAME_LEN, pTmp->threadName);

		memset(pTmp->threadData, 0x00, sizeof(pTmp->threadData));
		dbHandle->getColumnToString(index++, MAX_THREAD_DATA_LEN, pTmp->threadData);

		int protectedCnt = dbHandle->getColumnToInt(index++);
		if (protectedCnt > 0)
			pTmp->bProtected = true;

		int draftCnt = dbHandle->getColumnToInt(index++);
		if (draftCnt > 0)
			pTmp->bDraft = true;

		int failedCnt = dbHandle->getColumnToInt(index++);
		if (failedCnt > 0)
			pTmp->bSendFailed = true;

		int sendingCnt = dbHandle->getColumnToInt(index++);
		if (sendingCnt > 0)
			pTmp->bSending = true;
	}

	dbHandle->freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetConversationPreview(MsgDbHandler *pDbHandle, MSG_CONVERSATION_VIEW_S *pConv)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	int rowCnt = 0, index = 0;
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pConv == NULL)
		return MSG_ERR_NULL_POINTER;

	pConv->tcs_bc_level = -1; /* init */

	/*(MSG_ID INTEGER, TYPE INTEGER, VALUE TEXT, COUNT INTEGER) */
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT TYPE, VALUE, COUNT "
			"FROM %s WHERE MSG_ID=%d;",
			MSGFW_MMS_PREVIEW_TABLE_NAME, pConv->msgId);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);
	if (err == MSG_SUCCESS) {
		for (int i = 0; i < rowCnt; i++) {
			int type = pDbHandle->getColumnToInt(index++);
			if (type == MSG_MMS_ITEM_TYPE_IMG) {
				pDbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pConv->imageThumbPath);
				pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_VIDEO) {
				pDbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pConv->videoThumbPath);
				pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_AUDIO) {
				pDbHandle->getColumnToString(index++, MSG_FILENAME_LEN_MAX, pConv->audioFileName);
				pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_ATTACH) {
				pDbHandle->getColumnToString(index++, MSG_FILENAME_LEN_MAX, pConv->attachFileName);
				pConv->attachCount = pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_PAGE) {
				index++;
				pConv->pageCount = pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_MALWARE) {
				index++;
				pConv->tcs_bc_level = pDbHandle->getColumnToInt(index++);
			} else if (type == MSG_MMS_ITEM_TYPE_1ST_MEDIA) {
				pDbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pConv->firstMediaPath);
				pDbHandle->getColumnToInt(index++);
			} else {
				MSG_DEBUG("Unknown item type [%d]", type);
				index+=2;
			}
		}
	}

	pDbHandle->freeTable();
	return MSG_SUCCESS;
}

msg_error_t MsgStoGetConversationMultipart(MsgDbHandler *pDbHandle, MSG_CONVERSATION_VIEW_S *pConv)
{
	char sqlQuery[MAX_QUERY_LEN + 1];
	int rowCnt = 0, index = 0;
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pConv == NULL)
		return MSG_ERR_NULL_POINTER;

	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT CONTENT_TYPE, NAME, FILE_PATH, CONTENT_ID, CONTENT_LOCATION, TCS_LEVEL, MALWARE_ALLOW, THUMB_FILE_PATH "
			"FROM %s WHERE MSG_ID=%d;",
			MSGFW_MMS_MULTIPART_TABLE_NAME, pConv->msgId);

	msg_error_t err = pDbHandle->getTable(sqlQuery, &rowCnt, &index);
	if (err == MSG_SUCCESS) {
		GList *multipart_list = NULL;
		for (int i = 0; i < rowCnt; i++) {
			msg_struct_s *multipart_struct_s = new msg_struct_s;
			multipart_struct_s->type = MSG_STRUCT_MULTIPART_INFO;
			multipart_struct_s->data = new MMS_MULTIPART_DATA_S;
			memset(multipart_struct_s->data, 0x00, sizeof(MMS_MULTIPART_DATA_S));

			MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)multipart_struct_s->data;

			pDbHandle->getColumnToString(index++, sizeof(multipart->szContentType), multipart->szContentType);
			pDbHandle->getColumnToString(index++, sizeof(multipart->szFileName), multipart->szFileName);
			pDbHandle->getColumnToString(index++, sizeof(multipart->szFilePath), multipart->szFilePath);
			pDbHandle->getColumnToString(index++, sizeof(multipart->szContentID), multipart->szContentID);
			pDbHandle->getColumnToString(index++, sizeof(multipart->szContentLocation), multipart->szContentLocation);

			multipart->tcs_bc_level = pDbHandle->getColumnToInt(index++);
			multipart->malware_allow = pDbHandle->getColumnToInt(index++);
			pDbHandle->getColumnToString(index++, sizeof(multipart->szThumbFilePath), multipart->szThumbFilePath);

			multipart_list = g_list_append(multipart_list, multipart_struct_s);
		}
		pConv->multipart_list = (msg_list_handle_t)multipart_list;
	}

	pDbHandle->freeTable();
	return MSG_SUCCESS;
}

msg_error_t MsgStoGetConversationViewItem(msg_message_id_t msgId, MSG_CONVERSATION_VIEW_S *pConv)
{
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
			DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, \
			MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, SIM_INDEX\
			FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	msg_error_t err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	memset(pConv, 0x00, sizeof(MSG_CONVERSATION_VIEW_S));
	pConv->pText = NULL;

	pConv->msgId = dbHandle->getColumnToInt(index++);
	pConv->threadId = dbHandle->getColumnToInt(index++);
	pConv->folderId = dbHandle->getColumnToInt(index++);
	pConv->storageId = dbHandle->getColumnToInt(index++);
	pConv->mainType = dbHandle->getColumnToInt(index++);
	pConv->subType = dbHandle->getColumnToInt(index++);
	pConv->displayTime = (time_t)dbHandle->getColumnToInt(index++);
	pConv->textSize = dbHandle->getColumnToInt(index++);
	pConv->networkStatus = dbHandle->getColumnToInt(index++);
	pConv->bRead = dbHandle->getColumnToInt(index++);
	pConv->bProtected = dbHandle->getColumnToInt(index++);
	pConv->direction = dbHandle->getColumnToInt(index++);
	pConv->scheduledTime = (time_t)dbHandle->getColumnToInt(index++);

	dbHandle->getColumnToString(index++, MAX_SUBJECT_LEN, pConv->subject);
	char *tmpText = g_strdup(dbHandle->getColumnToString(index++));

	/*It does Not need to Get attach count in MSG_MESSAGE_TABLE. see MsgStoGetConversationPreview */
	/*pConv->attachCount = dbHandle->getColumnToInt(index++); */
	index++;
	pConv->simIndex = dbHandle->getColumnToInt(index++);

	dbHandle->freeTable();

	if (pConv->mainType == MSG_MMS_TYPE &&
		(pConv->networkStatus == MSG_NETWORK_RETRIEVING || pConv->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pConv->subType == MSG_NOTIFICATIONIND_MMS)) {
		pConv->pText = NULL;
		pConv->textSize = 0;
	} else {
		if (pConv->mainType == MSG_SMS_TYPE) {
			pConv->pText = new char[pConv->textSize+2];
			memset(pConv->pText, 0x00, pConv->textSize+2);
			snprintf(pConv->pText, pConv->textSize+1, "%s", tmpText);
		} else if (pConv->mainType == MSG_MMS_TYPE) {
			if (tmpText) {
				pConv->textSize = strlen(tmpText);

				pConv->pText = new char[pConv->textSize+1];
				memset(pConv->pText, 0x00, pConv->textSize+1);

				strncpy(pConv->pText, tmpText, pConv->textSize);
			}

			MsgStoGetConversationPreview(dbHandle, pConv);
			MsgStoGetConversationMultipart(dbHandle, pConv);
		}
	}

	if (tmpText) {
		g_free(tmpText);
		tmpText = NULL;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetConversationViewList(msg_thread_id_t threadId, msg_struct_list_s *pConvViewList)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	pConvViewList->nCount = 0;
	pConvViewList->msg_struct_info = NULL;

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
			DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, \
			MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, SIM_INDEX  \
			FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME, MSG_ID ASC;",
			MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
#else
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
			DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, \
			MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT \
			FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME, MSG_ID ASC;",
			MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);
#endif

	msg_error_t err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	pConvViewList->nCount = rowCnt;
	char *tmpText[rowCnt] = {NULL};

	MSG_DEBUG("pConvViewList->nCount [%d]", pConvViewList->nCount);

	pConvViewList->msg_struct_info = (msg_struct_t *)calloc(rowCnt, sizeof(msg_struct_t));
	memset(pConvViewList->msg_struct_info, 0x00, sizeof(msg_struct_t) * rowCnt);

	msg_struct_s *conv = NULL;
	MSG_CONVERSATION_VIEW_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pConvViewList->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
		memset(pConvViewList->msg_struct_info[i], 0x00, sizeof(msg_struct_s));

		conv = (msg_struct_s *)pConvViewList->msg_struct_info[i];

		conv->type = MSG_STRUCT_CONV_INFO;
		conv->data = new MSG_CONVERSATION_VIEW_S;
		memset(conv->data, 0x00, sizeof(MSG_CONVERSATION_VIEW_S));

		pTmp = (MSG_CONVERSATION_VIEW_S *)conv->data;

		pTmp->pText = NULL;

		pTmp->msgId = dbHandle->getColumnToInt(index++);
		pTmp->threadId = dbHandle->getColumnToInt(index++);
		pTmp->folderId = dbHandle->getColumnToInt(index++);
		pTmp->storageId = dbHandle->getColumnToInt(index++);
		pTmp->mainType = dbHandle->getColumnToInt(index++);
		pTmp->subType = dbHandle->getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle->getColumnToInt(index++);
		pTmp->textSize = dbHandle->getColumnToInt(index++);
		pTmp->networkStatus = dbHandle->getColumnToInt(index++);
		pTmp->bRead = dbHandle->getColumnToInt(index++);
		pTmp->bProtected = dbHandle->getColumnToInt(index++);
		pTmp->direction = dbHandle->getColumnToInt(index++);
		index++; /* This field is reserved. */

		dbHandle->getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);
		tmpText[i] = g_strdup(dbHandle->getColumnToString(index++));

		/*It does Not need to Get attach count in MSG_MESSAGE_TABLE. see MsgStoGetConversationPreview */
		/*pTmp->attachCount = dbHandle->getColumnToInt(index++); */
		index++;
		pTmp->simIndex = dbHandle->getColumnToInt(index++);
	}
	dbHandle->freeTable();

	for (int i = 0; i < pConvViewList->nCount; i++) {
		conv = (msg_struct_s *)pConvViewList->msg_struct_info[i];
		pTmp = (MSG_CONVERSATION_VIEW_S *)conv->data;

		if (pTmp->mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pText = NULL;
			pTmp->textSize = 0;
		} else {
			if (pTmp->mainType == MSG_SMS_TYPE) {
				pTmp->pText = new char[pTmp->textSize+2];
				memset(pTmp->pText, 0x00, pTmp->textSize+2);
				snprintf(pTmp->pText, pTmp->textSize+1, "%s", tmpText[i]);
			} else if (pTmp->mainType == MSG_MMS_TYPE) {
				if (tmpText[i]) {
					pTmp->textSize = strlen(tmpText[i]);

					pTmp->pText = new char[pTmp->textSize+1];
					memset(pTmp->pText, 0x00, pTmp->textSize+1);

					strncpy(pTmp->pText, tmpText[i], pTmp->textSize);
				}

				MsgStoGetConversationPreview(dbHandle, pTmp);
				MsgStoGetConversationMultipart(dbHandle, pTmp);
			}
		}
		if (tmpText[i]) {
			g_free(tmpText[i]);
			tmpText[i] = NULL;
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoSearchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList, int contactCount)
{
	if (!pSearchString)
		return MSG_ERR_NULL_POINTER;

	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();
	char *escapeAddressStr = NULL;

	/* Clear Out Parameter */
	pThreadViewList->nCount = 0;
	pThreadViewList->msg_struct_info = NULL;

	tr1::unordered_set<msg_thread_id_t> IdList;
	queue<MSG_THREAD_VIEW_S> searchList;

	MSG_THREAD_VIEW_S threadView;

	char sqlQuery[MAX_QUERY_LEN+1];

	/* Search - Address, Name */
	memset(sqlQuery, 0x00, MAX_QUERY_LEN+1);
	snprintf(sqlQuery, MAX_QUERY_LEN, "SELECT A.CONV_ID, A.UNREAD_CNT, A.SMS_CNT, A.MMS_CNT, A.DISPLAY_NAME, "
			"A.MAIN_TYPE, A.SUB_TYPE, A.MSG_DIRECTION, A.DISPLAY_TIME, A.MSG_TEXT, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.PROTECTED = 1) AS PROTECTED, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.FOLDER_ID = %d) AS DRAFT, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.NETWORK_STATUS = %d) AS FAILED, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.NETWORK_STATUS = %d) AS SENDING "
			"FROM %s A WHERE (A.SMS_CNT > 0 OR A.MMS_CNT > 0) "
			"AND A.CONV_ID IN "
			"(SELECT DISTINCT(CONV_ID) FROM %s WHERE "
			"ADDRESS_VAL LIKE ? ESCAPE '%c' ",
			MSGFW_MESSAGE_TABLE_NAME,
			MSGFW_MESSAGE_TABLE_NAME, MSG_DRAFT_ID,
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SEND_FAIL,
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SENDING,
			MSGFW_CONVERSATION_TABLE_NAME,
			MSGFW_ADDRESS_TABLE_NAME,
			MSGFW_DB_ESCAPE_CHAR);

	unsigned int tmpSize = 0;
	if (contactCount > 0) {
		tmpSize = strlen(sqlQuery);
		snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
				"OR ADDRESS_VAL IN (SELECT C.ADDRESS_VAL FROM %s C JOIN %s D ON (C.ADDRESS_VAL LIKE D.ADDRESS_VAL))"
				, MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TEMP_TABLE_NAME);
	}

	tmpSize = strlen(sqlQuery);
	snprintf(sqlQuery+tmpSize, MAX_QUERY_LEN-tmpSize,
			") ORDER BY A.DISPLAY_TIME DESC;");

	MSG_DEBUG("sqlQuery=[%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Prepare query fail.");
		return MSG_ERR_DB_PREPARE;
	}

	MsgConvertStrWithEscape(pSearchString, &escapeAddressStr);
	MSG_DEBUG("escapeAddressStr [%s]", escapeAddressStr);
	dbHandle->bindText(escapeAddressStr, 1);
	/*dbHandle->bindText(escapeAddressStr, 2); */
	/*dbHandle->bindText(escapeAddressStr, 3); */
	/*dbHandle->bindText(escapeAddressStr, 4); */

	while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		memset(&threadView, 0x00, sizeof(threadView));

		threadView.threadId = dbHandle->columnInt(0);
		threadView.unreadCnt = dbHandle->columnInt(1);
		threadView.smsCnt = dbHandle->columnInt(2);
		threadView.mmsCnt = dbHandle->columnInt(3);

		if (dbHandle->columnText(4))
			strncpy(threadView.threadName, (char *)dbHandle->columnText(4), MAX_THREAD_NAME_LEN);

		threadView.mainType = dbHandle->columnInt(5);
		threadView.subType = dbHandle->columnInt(6);

		threadView.direction = dbHandle->columnInt(7);
		threadView.threadTime = (time_t)dbHandle->columnInt(8);

		if (dbHandle->columnText(9))
			strncpy(threadView.threadData, (char *)dbHandle->columnText(9), MAX_THREAD_DATA_LEN);

		int protectedCnt = dbHandle->columnInt(10);
		if (protectedCnt > 0)
			threadView.bProtected = true;

		int draftCnt = dbHandle->columnInt(11);
		if (draftCnt > 0)
			threadView.bDraft = true;

		int failedCnt = dbHandle->columnInt(12);
		if (failedCnt > 0)
			threadView.bSendFailed = true;

		int sendingCnt = dbHandle->columnInt(13);
		if (sendingCnt > 0)
			threadView.bSending = true;

		tr1::unordered_set<msg_thread_id_t>::iterator it;

		it = IdList.find(threadView.threadId);

		if (it == IdList.end()) {
			IdList.insert(threadView.threadId);
			searchList.push(threadView);
		}
	}

	dbHandle->finalizeQuery();

	if (escapeAddressStr) {
		free(escapeAddressStr);
		escapeAddressStr = NULL;
	}


	/* Add data to Out Parameter */
	pThreadViewList->nCount = searchList.size();
	pThreadViewList->msg_struct_info = (msg_struct_t *)calloc(searchList.size(), sizeof(msg_struct_t));
	if (pThreadViewList->msg_struct_info == NULL)
		return MSG_ERR_MEMORY_ERROR;

	MSG_THREAD_VIEW_S *pTmp = NULL;
	msg_struct_s *thread_t = NULL;

	int index = 0;

	while (!searchList.empty()) {
		thread_t = (msg_struct_s *)new msg_struct_s;
		pThreadViewList->msg_struct_info[index] = (msg_struct_t)thread_t;

		thread_t->type = MSG_STRUCT_THREAD_INFO;
		thread_t->data = new MSG_THREAD_VIEW_S;

		pTmp = (MSG_THREAD_VIEW_S *)thread_t->data;
		memset(pTmp, 0x00, sizeof(MSG_THREAD_VIEW_S));

		memcpy(pTmp, &(searchList.front()), sizeof(MSG_THREAD_VIEW_S));

		searchList.pop();

		index++;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList)
{
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	/* Clear Out Parameter */
	pRejectMsgList->nCount = 0;
	pRejectMsgList->msg_struct_info = NULL;

	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	/* Search Reject Msg */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pNumber != NULL) {
		int addrSize = strlen(pNumber);
		char phoneNumber[addrSize+1];
		memset(phoneNumber, 0x00, sizeof(phoneNumber));

		if (addrSize > MsgContactGetMinMatchDigit())
			MsgConvertNumber(pNumber, phoneNumber, addrSize);
		else
			strncpy(phoneNumber, pNumber, addrSize);

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
				"B.MSG_ID, "
				"B.MSG_TEXT, "
				"B.DISPLAY_TIME "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID = B.CONV_ID "
				"AND B.MAIN_TYPE = %d "
				"AND B.SUB_TYPE = %d "
				"AND A.ADDRESS_VAL LIKE '%%%s' "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_SMS_TYPE,
				MSG_REJECT_SMS,
				phoneNumber);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
				"B.MSG_ID, "
				"B.MSG_TEXT, "
				"B.DISPLAY_TIME "
				"FROM %s A, %s B "
				"WHERE A.CONV_ID = B.CONV_ID "
				"AND B.MAIN_TYPE = %d "
				"AND B.SUB_TYPE = %d "
				"ORDER BY B.DISPLAY_TIME DESC;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_SMS_TYPE,
				MSG_REJECT_SMS);
	}

	msg_error_t err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	pRejectMsgList->nCount = rowCnt;

	MSG_DEBUG("pRejectMsgList->nCount [%d]", pRejectMsgList->nCount);

	pRejectMsgList->msg_struct_info = (msg_struct_t *)calloc(rowCnt, sizeof(MSG_REJECT_MSG_INFO_S *));

	msg_struct_s* pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pRejectMsgList->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;

		pTmp = (msg_struct_s *)pRejectMsgList->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_REJECT_MSG_INFO;
		pTmp->data = new MSG_REJECT_MSG_INFO_S;
		MSG_REJECT_MSG_INFO_S * pMsg = (MSG_REJECT_MSG_INFO_S *)pTmp->data;
		memset(pMsg, 0x00, sizeof(MSG_REJECT_MSG_INFO_S));

		pMsg->msgId = dbHandle->getColumnToInt(index++);
		memset(pMsg->msgText, 0x00, sizeof(pMsg->msgText));
		dbHandle->getColumnToString(index++, MAX_MSG_TEXT_LEN, pMsg->msgText);

		pMsg->displayTime = (time_t)dbHandle->getColumnToInt(index++);
	}

	dbHandle->freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList)
{
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	msg_error_t err = MSG_SUCCESS;

	/* contacts-service is not used for gear */
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	int order = MsgGetContactNameOrder();
#else /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	int order = 0;
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */


	err = MsgStoGetAddressByConvId(dbHandle, threadId, order, pAddrList);

	return err;
}


msg_error_t MsgStoGetMessageList(const MSG_LIST_CONDITION_S *pListCond, msg_struct_list_s *pMsgList, int contactCount)
{
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();

	/* Clear Out Parameter */
	pMsgList->nCount = 0;
	pMsgList->msg_struct_info = NULL;

	int index = 0;
	int multipartCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlQuerySubset[(MAX_QUERY_LEN/5)+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) FROM %s;", MSGFW_MMS_MULTIPART_TABLE_NAME);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		multipartCnt = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT "
			"A.MSG_ID, "
			"A.CONV_ID, "
			"A.FOLDER_ID, "
			"A.STORAGE_ID, "
			"A.MAIN_TYPE, "
			"A.SUB_TYPE, "
			"A.DISPLAY_TIME, "
			"A.DATA_SIZE, "
			"A.NETWORK_STATUS, "
			"A.READ_STATUS, "
			"A.PROTECTED, "
			"A.BACKUP, "
			"A.PRIORITY, "
			"A.MSG_DIRECTION, "
			"A.SCHEDULED_TIME, "
			"A.SUBJECT, "
			"A.MSG_TEXT, "
			"A.ATTACHMENT_COUNT, "
			"A.THUMB_PATH, "
			"A.SIM_INDEX, "
			"B.ADDRESS_TYPE, "
			"B.RECIPIENT_TYPE, "
			"B.ADDRESS_VAL ");

	if (pListCond->pTextVal != NULL && multipartCnt > 0) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "FROM %s C, %s B, %s A WHERE A.CONV_ID > 0 AND A.CONV_ID = B.CONV_ID AND ",
			MSGFW_MMS_MULTIPART_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);
	}
	else {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),	"FROM %s B, %s A WHERE A.CONV_ID > 0 AND A.CONV_ID = B.CONV_ID AND ",
		MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);
	}

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

	/* folder */
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (pListCond->folderId == MSG_ALLBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "A.FOLDER_ID > 0 AND A.FOLDER_ID < %d ", MSG_SPAMBOX_ID);
	else if (pListCond->folderId == MSG_IOSBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "A.FOLDER_ID > 0 AND A.FOLDER_ID < %d ", MSG_DRAFT_ID);
	else
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "A.FOLDER_ID = %d ", pListCond->folderId);

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	/* thread */
	if (pListCond->threadId > 0) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.CONV_ID = %d ", pListCond->threadId);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}


	/* msg type */
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	switch (pListCond->msgType) {
		case MSG_TYPE_SMS:
			if (pListCond->pAddressVal == NULL)
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d ", MSG_SMS_TYPE);
			else
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d AND A.SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_NORMAL_SMS);
			break;

		case MSG_TYPE_MMS:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d ", MSG_MMS_TYPE);
			break;

		case MSG_TYPE_MMS_JAVA:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d AND A.SUB_TYPE = %d ", MSG_MMS_TYPE, MSG_SENDREQ_JAVA_MMS);
			break;

		case MSG_TYPE_SMS_SYNCML:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d AND A.SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_SYNCML_CP);
			break;
		case MSG_TYPE_SMS_REJECT:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.MAIN_TYPE = %d AND A.SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_REJECT_SMS);
			break;

		default:
			MSG_DEBUG("msg type is not set.");
			break;
	}

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	/* storage */
	if (pListCond->storageId > MSG_STORAGE_UNKNOWN) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.STORAGE_ID = %d ", pListCond->storageId);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}


	/* protected */
	if (pListCond->bProtected) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.PROTECTED = %d ", pListCond->bProtected);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}


	/* scheduled */
	if (pListCond->bScheduled) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.SCHEDULED_TIME > 0 ");
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}


	/* sim index */
	if (pListCond->simIndex > 0) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.SIM_INDEX = %d ", pListCond->simIndex);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}


	/* time range */
	if (pListCond->fromTime > 0) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.DISPLAY_TIME >= %u ", (unsigned int)pListCond->fromTime);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}

	if (pListCond->toTime > 0) {
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.DISPLAY_TIME <= %u ", (unsigned int)pListCond->toTime);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}

	if (pListCond->pAddressVal == NULL) {
		/* Text */
		if (pListCond->pTextVal != NULL) {
			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			if (multipartCnt > 0) {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"AND ((A.MSG_TEXT LIKE ? ESCAPE '%c' OR A.SUBJECT LIKE ? ESCAPE '%c' OR (C.NAME LIKE ? ESCAPE '%c' AND A.MSG_ID = C.MSG_ID AND C.CONTENT_TYPE <> 'application/smil'))) ",
						MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR);
			}
			else {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"AND ((A.MSG_TEXT LIKE ? ESCAPE '%c' OR A.SUBJECT LIKE ? ESCAPE '%c')) ",
						MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR);
			}
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
		}
	} else {
		/* Text */
		if (pListCond->pTextVal != NULL) {
			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			if (multipartCnt > 0) {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"AND ((A.MSG_TEXT LIKE ? ESCAPE '%c' OR A.SUBJECT LIKE ? ESCAPE '%c' OR (C.NAME LIKE ? ESCAPE '%c' AND A.MSG_ID = C.MSG_ID AND C.CONTENT_TYPE <> 'application/smil')) ",
						MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR);
			}
			else {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"AND ((A.MSG_TEXT LIKE ? ESCAPE '%c' OR A.SUBJECT LIKE ? ESCAPE '%c') ",
						MSGFW_DB_ESCAPE_CHAR, MSGFW_DB_ESCAPE_CHAR);
			}
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			if (pListCond->bAnd) {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND ");
			} else {
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "OR ");
			}
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

			/* Address */
			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
					"(B.ADDRESS_VAL LIKE ? ESCAPE '%c' ", MSGFW_DB_ESCAPE_CHAR);
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

			if (contactCount > 0) {
				memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"OR B.ADDRESS_VAL IN (SELECT D.ADDRESS_VAL FROM %s D JOIN %s E ON (D.ADDRESS_VAL LIKE E.ADDRESS_VAL)) "
						, MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TEMP_TABLE_NAME);
				strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
			}

			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), ")) ");
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
		} else {
			/* Address */
			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
					"AND (B.ADDRESS_VAL LIKE ? ESCAPE '%c' ", MSGFW_DB_ESCAPE_CHAR);
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

			if (contactCount > 0) {
				memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
				snprintf(sqlQuerySubset, sizeof(sqlQuerySubset),
						"OR B.ADDRESS_VAL IN (SELECT D.ADDRESS_VAL FROM %s D JOIN %s E ON (D.ADDRESS_VAL LIKE E.ADDRESS_VAL)) "
						, MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TEMP_TABLE_NAME);
				strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
			}

			memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), ") ");
			strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
		}
	}

	msg_struct_s *pSortRule = (msg_struct_s *)pListCond->sortRule;

	if (pSortRule->type != MSG_STRUCT_SORT_RULE) {
		/* Order */
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.DISPLAY_TIME ");

		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

		/* Sorting type */
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "DESC ");

		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	} else {
		MSG_SORT_RULE_S *pTmp = (MSG_SORT_RULE_S *)pSortRule->data;
		/* order : TODO: have to finish this */
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		switch (pTmp->sortType) {
		case MSG_SORT_BY_MSG_TYPE:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.MAIN_TYPE ");
			break;
		case MSG_SORT_BY_READ_STATUS:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.READ_STATUS ");
			break;
		case MSG_SORT_BY_STORAGE_TYPE:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.STORAGE_ID ");
			break;
		default:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.DISPLAY_TIME ");
			break;
		}
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

		/* Sorting type */
		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		if (pTmp->bAscending)
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ASC ");
		else
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "DESC ");

		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));
	}

	/* offset & limit */
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
	if (pListCond->offset >= 0 && pListCond->limit > 0)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "LIMIT %d OFFSET %d;", pListCond->limit, pListCond->offset);
	else
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), ";");

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	/* 'til here sqlQuery is complete. */

	queue<MSG_MESSAGE_HIDDEN_S*> searchList;

	MSG_DEBUG("[%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Prepare query fail.");
		return MSG_ERR_DB_PREPARE;
	}

	char *escapeTextStr = NULL;
	char *escapeAddressStr = NULL;

	if (pListCond->pAddressVal == NULL) {
		/* Text */
		if (pListCond->pTextVal != NULL) {
			MsgConvertStrWithEscape(pListCond->pTextVal, &escapeTextStr);
			MSG_DEBUG("escapeTextStr [%s]", escapeTextStr);
			dbHandle->bindText(escapeTextStr, 1);
			dbHandle->bindText(escapeTextStr, 2);
			if (multipartCnt > 0) dbHandle->bindText(escapeTextStr, 3);
		}
	} else {
		/* Text */
		if (pListCond->pTextVal != NULL) {
			MsgConvertStrWithEscape(pListCond->pTextVal, &escapeTextStr);
			MSG_DEBUG("escapeTestStr [%s]", escapeTextStr);

			/* Address */
			MsgConvertStrWithEscape(pListCond->pAddressVal, &escapeAddressStr);
			MSG_DEBUG("escapeAddressStr [%s]", escapeAddressStr);

			dbHandle->bindText(escapeTextStr, 1);
			dbHandle->bindText(escapeTextStr, 2);
			if (multipartCnt > 0) {
				dbHandle->bindText(escapeTextStr, 3);
				dbHandle->bindText(escapeAddressStr, 4);
			} else {
				dbHandle->bindText(escapeAddressStr, 3);
			}

		} else {
			/* Address */
			MsgConvertStrWithEscape(pListCond->pAddressVal, &escapeAddressStr);
			MSG_DEBUG("escapeAddressStr [%s]", escapeAddressStr);
			dbHandle->bindText(escapeAddressStr, 1);
		}
	}


	MSG_MESSAGE_HIDDEN_S *pTmp = NULL;
	int lastMsgId = 0;  /* for comparing same msg id. */

	while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		index = 0;

		int msgid = dbHandle->columnInt(index++);
		MSG_DEBUG("msgid [%d]", msgid);

		if (lastMsgId != msgid) {
			MSG_DEBUG("lastMsgId != msgid");

			lastMsgId = msgid;

			pTmp = new MSG_MESSAGE_HIDDEN_S;

			if(pTmp) {
				memset(pTmp, 0x00, sizeof(MSG_MESSAGE_HIDDEN_S));

				pTmp->pData = NULL;
				pTmp->pMmsData = NULL;
				pTmp->addressList = NULL;

				pTmp->msgId = msgid;

				pTmp->threadId = dbHandle->columnInt(index++);
				pTmp->folderId = dbHandle->columnInt(index++);
				pTmp->storageId = dbHandle->columnInt(index++);
				pTmp->mainType = dbHandle->columnInt(index++);
				pTmp->subType = dbHandle->columnInt(index++);
				pTmp->displayTime = (time_t)dbHandle->columnInt(index++);
				pTmp->dataSize = dbHandle->columnInt(index++);
				pTmp->networkStatus = dbHandle->columnInt(index++);
				pTmp->bRead = dbHandle->columnInt(index++);
				pTmp->bProtected = dbHandle->columnInt(index++);
				pTmp->bBackup = dbHandle->columnInt(index++);
				pTmp->priority = dbHandle->columnInt(index++);
				pTmp->direction = dbHandle->columnInt(index++);
				index++; /* This field is reserved. */

				strncpy(pTmp->subject, (char *)dbHandle->columnText(index++), MAX_SUBJECT_LEN);

				if (pTmp->mainType == MSG_MMS_TYPE &&
					(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
					pTmp->pData = NULL;
					index++;
				} else {
					pTmp->pData = (void *)new char[pTmp->dataSize+2];
					memset(pTmp->pData, 0x00, pTmp->dataSize+2);

					strncpy((char *)pTmp->pData, (char *)dbHandle->columnText(index++), pTmp->dataSize+1);
				}

				pTmp->attachCount = dbHandle->columnInt(index++);

				strncpy(pTmp->thumbPath, (char *)dbHandle->columnText(index++), MSG_FILEPATH_LEN_MAX);

				pTmp->simIndex = dbHandle->columnInt(index++);

				pTmp->addr_list = (msg_struct_list_s *)new msg_struct_list_s;
				pTmp->addr_list->nCount = 0;
				pTmp->addr_list->msg_struct_info = (msg_struct_t *)calloc(MAX_TO_ADDRESS_CNT, sizeof(msg_struct_t));
				for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
					pTmp->addr_list->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
					memset(pTmp->addr_list->msg_struct_info[i], 0x00, sizeof(msg_struct_s));
				}

				searchList.push(pTmp);
			}

		} else {
			MSG_DEBUG("lastMsgId == msgid");
			index += 19;
		}

		if(pTmp) {
			MSG_ADDRESS_INFO_S *pAddr = new MSG_ADDRESS_INFO_S;
			memset(pAddr, 0x00, sizeof(MSG_ADDRESS_INFO_S));

			pAddr->addressType = dbHandle->columnInt(index++);
			pAddr->recipientType = dbHandle->columnInt(index++);

			strncpy(pAddr->addressVal, (char *)dbHandle->columnText(index++), MAX_ADDRESS_VAL_LEN);

			strncpy(pAddr->displayName, pAddr->addressVal, MAX_DISPLAY_NAME_LEN);

			/* For GList *addressList */
			msg_struct_s *addr_info_s = new msg_struct_s;
			memset(addr_info_s, 0x00, sizeof(msg_struct_s));
			addr_info_s->type = MSG_STRUCT_ADDRESS_INFO;
			addr_info_s->data = new MSG_ADDRESS_INFO_S;
			memset(addr_info_s->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
			MSG_ADDRESS_INFO_S *addr_info = (MSG_ADDRESS_INFO_S *)addr_info_s->data;
			addr_info->addressType = pAddr->addressType;
			addr_info->recipientType = pAddr->recipientType;
			addr_info->contactId = pAddr->contactId;
			strncpy(addr_info->addressVal, pAddr->addressVal, MAX_ADDRESS_VAL_LEN);
			strncpy(addr_info->displayName, pAddr->displayName, MAX_DISPLAY_NAME_LEN);
			addr_info->displayName[MAX_DISPLAY_NAME_LEN] = '\0';

			pTmp->addressList = g_list_append(pTmp->addressList, addr_info_s);

			if (pTmp->addr_list->nCount >= MAX_TO_ADDRESS_CNT) {
				delete pAddr;

			} else {
				msg_struct_s *pStruct = (msg_struct_s *)pTmp->addr_list->msg_struct_info[pTmp->addr_list->nCount];
				pTmp->addr_list->nCount++;
				pStruct->type = MSG_STRUCT_ADDRESS_INFO;
				pStruct->data = pAddr;
			}
		}
	}

	dbHandle->finalizeQuery();

	pMsgList->nCount = searchList.size();
	MSG_DEBUG("pMsgList->nCount [%d]", pMsgList->nCount);

	pMsgList->msg_struct_info = (msg_struct_t *)calloc(pMsgList->nCount, sizeof(msg_struct_t));
	if (pMsgList->msg_struct_info == NULL)
		return MSG_ERR_MEMORY_ERROR;

	int offset = 0;
	while (!searchList.empty()) {
		msg_struct_s *msg = new msg_struct_s;

		pMsgList->msg_struct_info[offset++] = (msg_struct_t)msg;

		msg->type = MSG_STRUCT_MESSAGE_INFO;
		msg->data = searchList.front();

		searchList.pop();
	}


	if (escapeTextStr)
		free(escapeTextStr);

	if (escapeAddressStr)
		free(escapeAddressStr);

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMediaList(const msg_thread_id_t threadId, msg_list_handle_t *pMediaList)
{
	MSG_BEGIN();
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->connectReadOnly();
	char sqlQuery[MAX_QUERY_LEN+1];
	int msgIdCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE MAIN_TYPE = %d AND CONV_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_MMS_TYPE, threadId);

	MSG_DEBUG("sqlQuery = [%s]", sqlQuery);

	err = dbHandle->getTable(sqlQuery, &msgIdCnt, NULL);
	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return err;
	} else if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	msg_message_id_t msgIds[msgIdCnt];

	for (int i = 1; i <= msgIdCnt; i++) {
		msgIds[i-1] = dbHandle->getColumnToInt(i);
	}

	dbHandle->freeTable();

	GList *media_list = NULL;

	for (int i = 0; i < msgIdCnt; i++) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONTENT_TYPE, FILE_PATH, THUMB_FILE_PATH "
				"FROM %s WHERE MSG_ID = %d AND SEQ <> -1 AND (TCS_LEVEL = -1 OR MALWARE_ALLOW = 1);",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgIds[i]);

		int rowCnt = 0, msg_id = 0, index = 0;

		err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
			dbHandle->freeTable();
			return err;
		}

		MSG_MEDIA_INFO_S *pMedia = NULL;
		char mime_type[MAX_MIME_TYPE_LEN+1], media_item[MSG_FILEPATH_LEN_MAX+1], thumb_path[MSG_FILEPATH_LEN_MAX+1];

		for (int j = 0; j < rowCnt; j++) {
			msg_id = dbHandle->getColumnToInt(index++);
			memset(mime_type, 0x00, sizeof(mime_type));
			dbHandle->getColumnToString(index++, MAX_MIME_TYPE_LEN, mime_type);
			memset(media_item, 0x00, sizeof(media_item));
			dbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, media_item);
			memset(thumb_path, 0x00, sizeof(thumb_path));
			dbHandle->getColumnToString(index++, MSG_FILEPATH_LEN_MAX, thumb_path);

			if (strstr(mime_type, "image") || strstr(mime_type, "video")) {
				msg_struct_s *media_struct_s = new msg_struct_s;
				media_struct_s->type = MSG_STRUCT_MEDIA_INFO;
				media_struct_s->data = new MSG_MEDIA_INFO_S;
				memset(media_struct_s->data, 0x00, sizeof(MSG_MEDIA_INFO_S));

				pMedia = (MSG_MEDIA_INFO_S *)media_struct_s->data;

				pMedia->msg_id = msg_id;
				snprintf(pMedia->mime_type, MAX_MIME_TYPE_LEN, "%s", mime_type);
				snprintf(pMedia->media_item, MSG_FILEPATH_LEN_MAX, "%s", media_item);
				snprintf(pMedia->thumb_path, MSG_FILEPATH_LEN_MAX, "%s", thumb_path);

				media_list = g_list_append(media_list, media_struct_s);
			}
		}

		dbHandle->freeTable();

		*pMediaList = (msg_list_handle_t)media_list;
	}

	MSG_END();
	return MSG_SUCCESS;
}

#ifdef FEATURE_SMS_CDMA
msg_error_t MsgStoClearUniquenessTable()
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN+1] = {0, };
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = 0", MSGFW_UNIQUENESS_INFO_TABLE_NAME);

	err = dbHandle->execQuery(sqlQuery);

	MSG_END();

	return err;
}
#endif
