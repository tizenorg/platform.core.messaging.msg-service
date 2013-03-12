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
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgUtilStorage.h"

#include <sys/stat.h>
#include <sys/vfs.h>


static int msgCntLimit[MSG_COUNT_LIMIT_MAILBOX_TYPE_MAX][MSG_COUNT_LIMIT_MSG_TYPE_MAX] = {{10, 10, 0, 10, 10}, {5, 10, 0, 0, 0}, {10, 10, 0, 0, 0}, {10, 10, 0, 0, 0}, {0, 0, 10, 0, 0}};


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
	AutoPtr<char> buf(&pFileData);

	// Get File Data
	if (pMsgInfo->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
			return 0;

		MSG_DEBUG("file size [%d]", fileSize);
	}

	// Add Message
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %ld, %d, ?, ?, ?, ?, 0);",
			MSGFW_MESSAGE_TABLE_NAME, msgId, pMsgInfo->threadId, pMsgInfo->folderId, pMsgInfo->storageId, pMsgInfo->msgType.mainType,
			pMsgInfo->msgType.subType, pMsgInfo->displayTime, pMsgInfo->dataSize, pMsgInfo->networkStatus, pMsgInfo->bRead, pMsgInfo->bProtected,
			pMsgInfo->priority, pMsgInfo->direction, 0, pMsgInfo->bBackup);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return 0;

	pDbHandle->bindText(pMsgInfo->subject, 1);

	pDbHandle->bindText(pMsgInfo->msgData, 2);

	pDbHandle->bindText(pMsgInfo->thumbPath, 3);

	if (pMsgInfo->bTextSms == false)
		pDbHandle->bindText(pFileData, 4);
	else
		pDbHandle->bindText(pMsgInfo->msgText, 4);

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

	// Get MAIN_TYPE, SUB_TYPE, STORAGE_ID
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

	int smsCnt = 0, mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(pDbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(pDbHandle, MSG_MMS_TYPE);

	MsgSettingSetIndicator(smsCnt, mmsCnt);

	MsgRefreshNoti(false);

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
		struct statfs buf = {0};

		if (statfs(MSG_DATA_ROOT_PATH, &buf) == -1) {
			MSG_DEBUG("statfs(\"%s\") failed - %d", MSG_DATA_ROOT_PATH);
			return MSG_ERR_STORAGE_ERROR;
		}

		unsigned long freeSpace = (buf.f_bfree * buf.f_bsize);

		MSG_DEBUG("f_bfree [%d] f_bsize [%d]", buf.f_bfree, buf.f_bsize);
		MSG_DEBUG("Free space of storage is [%ul] MB.", freeSpace);

		if (freeSpace < SMS_MINIMUM_SPACE && pMsgType->mainType == MSG_SMS_TYPE)
			err = MSG_ERR_MESSAGE_COUNT_FULL;
		else if(freeSpace < MMS_MINIMUM_SPACE && pMsgType->mainType == MSG_MMS_TYPE)
			err = MSG_ERR_MESSAGE_COUNT_FULL;

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

	if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_WAP_SI_SMS ||pMsgType->subType == MSG_WAP_SL_SMS)) { // PUSH
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_INBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_CB_SMS)) { // CB
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_CB_SMS, MSG_CBMSGBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_SYNCML_CP)) { // Provision
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SYNCML_CP, MSG_INBOX_ID);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE)) { // SMS
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_CB_SMS, MSG_SYNCML_CP, MSG_INBOX_ID); // etc SMS
	} else if ((pMsgType->mainType == MSG_MMS_TYPE) &&
			(pMsgType->subType == MSG_SENDREQ_MMS || pMsgType->subType == MSG_SENDCONF_MMS || pMsgType->subType == MSG_RETRIEVE_AUTOCONF_MMS ||
					pMsgType->subType == MSG_RETRIEVE_MANUALCONF_MMS || pMsgType->subType == MSG_NOTIFICATIONIND_MMS)) { // MMS
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
	int msgType= -1;

	switch (folderId)
	{
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

	switch (pMsgType->subType)
	{
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


msg_error_t MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Check if new address or not
	if (MsgExistAddress(pDbHandle, pMsg, pConvId) == true) {
		MSG_DEBUG("The address already exists. Conversation ID : [%d]", *pConvId);
		return err;
	}

	MSG_DEBUG("Conversation ID : [%d]", *pConvId);

	/* conversation insert */
	err = MsgStoAddConversation(pDbHandle, pConvId);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoAddConversation() fail [%d]", err);
		return err;
	}

	/* insert address in loop */
	for (int i=0; i<pMsg->nAddressCnt; i++) {

		unsigned int addrId;
		MSG_CONTACT_INFO_S contactInfo;
		memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

		// Get Contact Info
		if (MsgGetContactInfo(&(pMsg->addressList[i]), &contactInfo) != MSG_SUCCESS) {
			MSG_DEBUG("MsgGetContactInfo() fail.");
		}

		err = pDbHandle->getRowId(MSGFW_ADDRESS_TABLE_NAME, &addrId);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("pDbHandle->getRowId fail. [%d]", err);
			return err;
		}

		// Add Address
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, '%s', %d, '', ?, ?, '%s', 0);",
					MSGFW_ADDRESS_TABLE_NAME, addrId, *pConvId, pMsg->addressList[i].addressType, pMsg->addressList[i].recipientType, pMsg->addressList[i].addressVal,
					contactInfo.contactId, contactInfo.imagePath);

		MSG_DEBUG("Add Address Info. [%s]", sqlQuery);

		if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		pDbHandle->bindText(contactInfo.firstName, 1);
		pDbHandle->bindText(contactInfo.lastName, 2);

		if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			pDbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		pDbHandle->finalizeQuery();

		// set conversation display name by conv id
		MsgStoSetConversationDisplayName(pDbHandle, *pConvId);

	}

	return err;
}

msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, int *nAddressCnt, MSG_ADDRESS_INFO_S *pAddress)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1];
	char lastName[MAX_DISPLAY_NAME_LEN+1];

	*nAddressCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_TYPE, A.RECIPIENT_TYPE, \
				A.CONTACT_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME \
				FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND B.MSG_ID = %d;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	while (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		memset(firstName, 0x00, sizeof(firstName));
		memset(lastName, 0x00, sizeof(lastName));

		pAddress[*nAddressCnt].addressType = pDbHandle->columnInt(0);
		pAddress[*nAddressCnt].recipientType = pDbHandle->columnInt(1);
		pAddress[*nAddressCnt].contactId = pDbHandle->columnInt(2);

		if (pDbHandle->columnText(3) != NULL)
			strncpy(pAddress[*nAddressCnt].addressVal, (char *)pDbHandle->columnText(3), MAX_ADDRESS_VAL_LEN);

		if (pDbHandle->columnText(4) != NULL && ((char *)pDbHandle->columnText(4))[0]!='\0') {
			MSG_DEBUG("displayName  : [%s]", pDbHandle->columnText(4));
			strncpy(pAddress[*nAddressCnt].displayName, (char *)pDbHandle->columnText(4), MAX_DISPLAY_NAME_LEN);
		} else {
			if (pDbHandle->columnText(5) != NULL)
					strncpy(firstName, (char *)pDbHandle->columnText(5), MAX_DISPLAY_NAME_LEN);

			if (pDbHandle->columnText(6) != NULL)
					strncpy(lastName, (char *)pDbHandle->columnText(6), MAX_DISPLAY_NAME_LEN);

			if (contactNameOrder == 0) {
				if (strlen(firstName) > 0) {
					strncpy(pAddress[*nAddressCnt].displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}
				if (strlen(lastName) > 0) {
					strncat(pAddress[*nAddressCnt].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddress[*nAddressCnt].displayName));
					strncat(pAddress[*nAddressCnt].displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(pAddress[*nAddressCnt].displayName));
				}
			} else if (contactNameOrder == 1) {
				if (strlen(lastName) > 0) {
					strncpy(pAddress[*nAddressCnt].displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(pAddress[*nAddressCnt].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddress[*nAddressCnt].displayName));
				}

				if (strlen(firstName) > 0) {
					strncat(pAddress[*nAddressCnt].displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(pAddress[*nAddressCnt].displayName));
				}
			}
		}

		(*nAddressCnt)++;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetAddressByMsgId(MsgDbHandler *pDbHandle, msg_message_id_t msgId, int contactNameOrder, msg_struct_list_s *pAddress)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1];
	char lastName[MAX_DISPLAY_NAME_LEN+1];

	pAddress->nCount = 0;
	pAddress->msg_struct_info = NULL;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_TYPE, A.RECIPIENT_TYPE, \
			A.CONTACT_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME \
			FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	msg_struct_s *pTmp = NULL;
	MSG_ADDRESS_INFO_S *pAddr = NULL;

	pAddress->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * MAX_TO_ADDRESS_CNT];

	for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
		pAddress->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
		pTmp = (msg_struct_s *)pAddress->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_ADDRESS_INFO;
		pTmp->data = new char[sizeof(MSG_ADDRESS_INFO_S)];
		memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
	}


	while (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		memset(firstName, 0x00, sizeof(firstName));
		memset(lastName, 0x00, sizeof(lastName));

		pTmp = (msg_struct_s *)pAddress->msg_struct_info[pAddress->nCount];
		pAddr = (MSG_ADDRESS_INFO_S *)pTmp->data;

		pAddr->addressType = pDbHandle->columnInt(0);
		pAddr->recipientType = pDbHandle->columnInt(1);
		pAddr->contactId = pDbHandle->columnInt(2);

		if (pDbHandle->columnText(3) != NULL)
					strncpy(pAddr->addressVal, (char *)pDbHandle->columnText(3), MAX_ADDRESS_VAL_LEN);

		if (pDbHandle->columnText(4) != NULL && ((char *)pDbHandle->columnText(4))[0]!='\0') {
					MSG_DEBUG("displayName  : [%s]", pDbHandle->columnText(4));
					strncpy(pAddr->displayName, (char *)pDbHandle->columnText(4), MAX_DISPLAY_NAME_LEN);
		} else {
			if (pDbHandle->columnText(5) != NULL)
				strncpy(firstName, (char *)pDbHandle->columnText(5), MAX_DISPLAY_NAME_LEN);

			if (pDbHandle->columnText(6) != NULL)
				strncpy(lastName, (char *)pDbHandle->columnText(6), MAX_DISPLAY_NAME_LEN);

			if (contactNameOrder == 0) {
				if (strlen(firstName) > 0) {
					strncpy(pAddr->displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (strlen(lastName) > 0) {
					strncat(pAddr->displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
					strncat(pAddr->displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}
			} else if (contactNameOrder == 1) {
				if (strlen(lastName) > 0) {
					strncpy(pAddr->displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(pAddr->displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}

				if (strlen(firstName) > 0) {
					strncat(pAddr->displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}
			}
		}

		pAddress->nCount++;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddressByConvId(MsgDbHandler *pDbHandle, msg_thread_id_t convId, int contactNameOrder, msg_struct_list_s *pAddrlist)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1];
	char lastName[MAX_DISPLAY_NAME_LEN+1];
	int index = 7;
	int rowCnt = 0;

	pAddrlist->nCount = 0;
	pAddrlist->msg_struct_info = NULL;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_TYPE, RECIPIENT_TYPE, \
			CONTACT_ID, ADDRESS_VAL, DISPLAY_NAME, FIRST_NAME, LAST_NAME \
			FROM %s WHERE CONV_ID  = %d;",
			MSGFW_ADDRESS_TABLE_NAME, convId);

	msg_error_t  err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("sqlQuery is - %s", sqlQuery);
		pDbHandle->freeTable();
		return err;
	}

	pAddrlist->nCount = rowCnt;

	MSG_DEBUG("pAddrlist->nCount [%d]", pAddrlist->nCount);

	msg_struct_s *pTmp = NULL;
	MSG_ADDRESS_INFO_S *pAddr = NULL;

	pAddrlist->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * MAX_TO_ADDRESS_CNT];

	for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
		pAddrlist->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
		pTmp = (msg_struct_s *)pAddrlist->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_ADDRESS_INFO;
		pTmp->data = new char[sizeof(MSG_ADDRESS_INFO_S)];
		memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
	}

	for (int i = 0; i < rowCnt; i++) {
		pTmp = (msg_struct_s *)pAddrlist->msg_struct_info[i];
		pAddr = (MSG_ADDRESS_INFO_S *)pTmp->data;

		pAddr->addressType = pDbHandle->getColumnToInt(index++);
		pAddr->recipientType = pDbHandle->getColumnToInt(index++);
		pAddr->contactId = pDbHandle->getColumnToInt(index++);
		pDbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pAddr->addressVal);
		pDbHandle->getColumnToString(index++, MAX_DISPLAY_NAME_LEN, pAddr->displayName);
		if(!strlen(pAddr->displayName))
		{
			pDbHandle->getColumnToString(index++,MAX_DISPLAY_NAME_LEN, firstName);
			pDbHandle->getColumnToString(index++,MAX_DISPLAY_NAME_LEN, lastName);

			if (contactNameOrder == 0) {
				if (strlen(firstName) > 0) {
					strncpy(pAddr->displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (strlen(lastName) > 0) {
					strncat(pAddr->displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
					strncat(pAddr->displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}
			} else if (contactNameOrder == 1) {
				if (strlen(lastName) > 0) {
					strncpy(pAddr->displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(pAddr->displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}

				if (strlen(firstName) > 0) {
					strncat(pAddr->displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(pAddr->displayName));
				}
			}
		}

	}
	pDbHandle->freeTable();

	return MSG_SUCCESS;
}

/* Have to use trigger for this function. */
msg_error_t MsgStoUpdateConversation(MsgDbHandler *pDbHandle, msg_thread_id_t convId)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	int unreadCnt = 0;
	int smsCnt = 0;
	int mmsCnt = 0;

	char msgText[MAX_THREAD_DATA_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(msgText, 0x00, sizeof(msgText));

	// Get Unread Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND READ_STATUS = 0;",
			MSGFW_MESSAGE_TABLE_NAME, convId, MSG_INBOX_ID, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW) {
		unreadCnt = pDbHandle->columnInt(0);
	} else if (err != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get SMS Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE CONV_ID = %d AND MAIN_TYPE = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, convId, MSG_SMS_TYPE, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW) {
		smsCnt = pDbHandle->columnInt(0);
	}
	else if (err != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get MMS Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
			WHERE CONV_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d) AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, convId, MSG_MMS_TYPE, MSG_DELIVERYIND_MMS, MSG_READRECIND_MMS, MSG_READORGIND_MMS,
			MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW) {
		mmsCnt = pDbHandle->columnInt(0);
	} else if (err != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get Latest Msg Data
	MSG_MAIN_TYPE_T mainType = MSG_UNKNOWN_TYPE;
	MSG_SUB_TYPE_T subType = MSG_NORMAL_SMS;
	msg_direction_type_t direction = MSG_DIRECTION_TYPE_MO;
	time_t msgTime = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE, MSG_DIRECTION, DISPLAY_TIME, SUBJECT, MSG_TEXT FROM %s \
			WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, convId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW) {
		mainType = pDbHandle->columnInt(0);
		subType = pDbHandle->columnInt(1);
		direction = pDbHandle->columnInt(2);

		msgTime = (time_t)pDbHandle->columnInt(3);

		memset(msgText, 0x00, sizeof(msgText));

		if (mainType == MSG_SMS_TYPE) {
			if (pDbHandle->columnText(5) != NULL)
				strncpy(msgText, (char*)pDbHandle->columnText(5), MAX_THREAD_DATA_LEN);
		} else if (mainType == MSG_MMS_TYPE) {
			if (pDbHandle->columnText(4) != NULL) {
				strncpy(msgText, (char*)pDbHandle->columnText(4), MAX_THREAD_DATA_LEN);
			}

			if ((strlen(msgText) <= 0) && (pDbHandle->columnText(5) != NULL) && (subType != MSG_NOTIFICATIONIND_MMS)) {
				memset(msgText, 0x00, sizeof(msgText));
				strncpy(msgText, (char*)pDbHandle->columnText(5), MAX_THREAD_DATA_LEN);
			}
		}
	} else if (err != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Update Address Table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
			UNREAD_CNT = %d, SMS_CNT = %d, MMS_CNT = %d, MAIN_TYPE = %d, SUB_TYPE = %d, MSG_DIRECTION = %d, DISPLAY_TIME = %ld, MSG_TEXT = ? \
			WHERE CONV_ID = %d;", MSGFW_CONVERSATION_TABLE_NAME, unreadCnt, smsCnt, mmsCnt, mainType, subType, direction, msgTime, convId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	pDbHandle->bindText(msgText, 1);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		MSG_DEBUG("Update Address Info. Fail [%d] [%s]", err, sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

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


void MsgConvertNumber(const char* pSrcNum, char* pDestNum)
{
	int overLen = 0;
	int i = 0;

	overLen = strlen(pSrcNum) - MAX_PRECONFIG_NUM;

	for (i = 0; i < MAX_PRECONFIG_NUM; i++)
		pDestNum[i] = pSrcNum[i+overLen];

	pDestNum[i] = '\0';
}

/* Change the function name to conversation related. */
bool MsgExistAddress(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pConvId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	*pConvId = 0;

	if(pMsg->nAddressCnt == 1) {
		if (strlen(pMsg->addressList[0].addressVal) > MAX_PRECONFIG_NUM) {
			char newPhoneNum[MAX_PRECONFIG_NUM+1];

			memset(newPhoneNum, 0x00, sizeof(newPhoneNum));

			MsgConvertNumber(pMsg->addressList[0].addressVal, newPhoneNum);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery),
					"SELECT CONV_ID FROM (SELECT B.CONV_ID FROM %s A, %s B WHERE A.ADDRESS_VAL LIKE '%%%%%s' AND A.CONV_ID=B.CONV_ID) GROUP BY CONV_ID HAVING COUNT(CONV_ID)=1;",
					MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, newPhoneNum);
		} else {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery),
					"SELECT CONV_ID FROM (SELECT B.CONV_ID FROM %s A, %s B WHERE A.ADDRESS_VAL = '%s' AND A.CONV_ID=B.CONV_ID) GROUP BY CONV_ID HAVING COUNT(CONV_ID)=1;",
					MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList[0].addressVal);
		}

		int rowCnt = 0;
		msg_thread_id_t convId = 0;
		err = pDbHandle->getTable(sqlQuery, &rowCnt);

		/* No record or other error */
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			pDbHandle->freeTable();
			return false;
		}

		if(rowCnt > 0) {
			convId = pDbHandle->getColumnToInt(1);

			MSG_DEBUG("CONV_ID : [%d]", convId);

			if (convId > 0) {
				*pConvId = convId;
				pDbHandle->freeTable();
				return true;
			} else {
				pDbHandle->freeTable();
				return false;
			}
		}

	} else { /* multiple address */
		if (strlen(pMsg->addressList[0].addressVal) > MAX_PRECONFIG_NUM) {
			char newPhoneNum[MAX_PRECONFIG_NUM+1];

			memset(newPhoneNum, 0x00, sizeof(newPhoneNum));

			MsgConvertNumber(pMsg->addressList[0].addressVal, newPhoneNum);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery),
					"SELECT CONV_ID FROM (SELECT B.CONV_ID FROM %s A, %s B WHERE A.ADDRESS_VAL LIKE '%%%%%s' AND A.CONV_ID=B.CONV_ID) GROUP BY CONV_ID HAVING COUNT(CONV_ID)=%d;",
					MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, newPhoneNum, pMsg->nAddressCnt);
		} else {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery),
					"SELECT CONV_ID FROM (SELECT B.CONV_ID FROM %s A, %s B WHERE A.ADDRESS_VAL = '%s' AND A.CONV_ID=B.CONV_ID) GROUP BY CONV_ID HAVING COUNT(CONV_ID)=%d;",
					MSGFW_ADDRESS_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList[0].addressVal, pMsg->nAddressCnt);
		}

		int rowCnt = 0;
		int convId = 0;

		MSG_DEBUG("Query [%s]", sqlQuery);

		err = pDbHandle->getTable(sqlQuery, &rowCnt);

		/* No record or other error */
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			pDbHandle->freeTable();
			return false;
		}

		for (int i = 1; i <= rowCnt; i++) {
			convId = pDbHandle->getColumnToInt(i);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery),
					"SELECT COUNT(*) FROM %s WHERE CONV_ID=%d AND (",
					MSGFW_ADDRESS_TABLE_NAME, convId);

			for (int j = 0; j<(pMsg->nAddressCnt); j++ ) {

				if (j!=0)
					strncat(sqlQuery, "OR ", MAX_QUERY_LEN-strlen(sqlQuery));

				if (strlen(pMsg->addressList[j].addressVal) > MAX_PRECONFIG_NUM) {

					strncat(sqlQuery, "ADDRESS_VAL LIKE '%%%%", MAX_QUERY_LEN-strlen(sqlQuery));

					char newPhoneNum[MAX_PRECONFIG_NUM+1];
					memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
					MsgConvertNumber(pMsg->addressList[j].addressVal, newPhoneNum);

					strncat(sqlQuery, newPhoneNum, MAX_QUERY_LEN-strlen(sqlQuery));

					strncat(sqlQuery, "' ", MAX_QUERY_LEN-strlen(sqlQuery));
				} else {
					strncat(sqlQuery, "ADDRESS_VAL = '", MAX_QUERY_LEN-strlen(sqlQuery));

					strncat(sqlQuery, pMsg->addressList[j].addressVal, MAX_QUERY_LEN-strlen(sqlQuery));

					strncat(sqlQuery, "' ", MAX_QUERY_LEN-strlen(sqlQuery));
				}
			}
			strncat(sqlQuery, ");", MAX_QUERY_LEN-strlen(sqlQuery));
			MSG_DEBUG("Query [%s]", sqlQuery);
			if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				MSG_DEBUG("Query Failed [%s]", sqlQuery);
				pDbHandle->freeTable();
				pDbHandle->finalizeQuery();
				return false;
			}

			if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
				if (pMsg->nAddressCnt == pDbHandle->columnInt(0)) {
					*pConvId = convId;
					pDbHandle->finalizeQuery();
					pDbHandle->freeTable();
					return true;
				}
			}
			pDbHandle->finalizeQuery();
		}
		pDbHandle->freeTable();
	}

	return false;
}


int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T msgType)
{
	int msgCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgType == MSG_SMS_TYPE) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
				WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d, %d, %d, %d) AND FOLDER_ID = %d AND READ_STATUS = 0;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_SMS_TYPE, MSG_NORMAL_SMS, MSG_STATUS_REPORT_SMS, MSG_CONCAT_SIM_SMS, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_MWI_VOICE_SMS, MSG_SYNCML_CP, MSG_INBOX_ID);
	} else if (msgType == MSG_MMS_TYPE) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
				WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d) AND FOLDER_ID = %d AND READ_STATUS = 0;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_MMS_TYPE, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS, MSG_NOTIFICATIONIND_MMS, MSG_INBOX_ID);
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


msg_error_t MsgStoAddContactInfo(MsgDbHandler *pDbHandle, MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber)
{
	char newPhoneNum[MAX_PRECONFIG_NUM+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	if (strlen(pNumber) > MAX_PRECONFIG_NUM) {
		memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
		MsgConvertNumber(pNumber, newPhoneNum);

		MSG_DEBUG("Phone Number to Compare : [%s]", newPhoneNum);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
				CONTACT_ID = %d, FIRST_NAME = ?, LAST_NAME = ?, IMAGE_PATH = '%s' \
				WHERE ADDRESS_VAL LIKE '%%%%%s';",
				MSGFW_ADDRESS_TABLE_NAME, pContactInfo->contactId, pContactInfo->imagePath, newPhoneNum);
	} else {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
				CONTACT_ID = %d, FIRST_NAME = ?, LAST_NAME = ?, IMAGE_PATH = '%s' \
				WHERE ADDRESS_VAL = '%s';",
				MSGFW_ADDRESS_TABLE_NAME, pContactInfo->contactId, pContactInfo->imagePath, pNumber);
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	pDbHandle->bindText(pContactInfo->firstName, 1);

	pDbHandle->bindText(pContactInfo->lastName, 2);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		pDbHandle->finalizeQuery();
		MSG_DEBUG("Update contact Info. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int contactId)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE CONTACT_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, contactId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS) {
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return err;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
			CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
			WHERE CONTACT_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, contactId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to execute query");
		return MSG_ERR_DB_EXEC;
	}

	MsgDbHandler tmpDbHandle;
	for (int i=1; i<=rowCnt; i++)
		MsgStoSetConversationDisplayName(&tmpDbHandle, (msg_thread_id_t)pDbHandle->getColumnToInt(i));

	pDbHandle->freeTable();

	return err;
}


msg_error_t MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int contactId, const char *pNumber)
{
	char newPhoneNum[MAX_PRECONFIG_NUM+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (strlen(pNumber) > MAX_PRECONFIG_NUM) {
		MsgConvertNumber(pNumber, newPhoneNum);

		MSG_DEBUG("Phone Number to Compare : [%s]", newPhoneNum);

		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
				CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
				WHERE CONTACT_ID = %d AND ADDRESS_VAL NOT LIKE '%%%s';",
				MSGFW_ADDRESS_TABLE_NAME, contactId, newPhoneNum);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
				CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
				WHERE CONTACT_ID = %d AND ADDRESS_VAL <> '%s';",
				MSGFW_ADDRESS_TABLE_NAME, contactId, pNumber);
	}

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to execute query");
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
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

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

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

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

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

	if (pDbHandle->getRowId(MSGFW_CONVERSATION_TABLE_NAME, pConvId) != MSG_SUCCESS) {
		return MSG_ERR_DB_EXEC;
	}

	// Add Conversation
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

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		pDbHandle->freeTable();
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return err;
	}

	int order = MsgGetContactNameOrder();
	msg_struct_s *pAddrInfo = NULL;
	MSG_ADDRESS_INFO_S *address = NULL;

	for (int i = 1; i <= rowCnt; i++)
	{
		memset(displayName, 0x00, sizeof(displayName));
		MsgDbHandler tmpDbHandle;
		msg_struct_list_s addressList = {0,};
		MsgStoGetAddressByConvId(&tmpDbHandle, (msg_thread_id_t)pDbHandle->getColumnToInt(i), order, &addressList);

		for (int j = 0; j < addressList.nCount; j++)
		{
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
			MSG_DEBUG("Query Failed [%s]", sqlQuery);
			return MSG_ERR_DB_PREPARE;
		}

		pDbHandle->bindText(displayName, 1);

		if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			pDbHandle->finalizeQuery();
			MSG_DEBUG("Update Conversation disply name. Fail [%s]", sqlQuery);
			return MSG_ERR_DB_STEP;
		}

		pDbHandle->finalizeQuery();

		// free address list
		for(int j = 0; j < MAX_TO_ADDRESS_CNT; j++){
			msg_struct_s *pStruct = (msg_struct_s *)addressList.msg_struct_info[j];
			delete [] (MSG_ADDRESS_INFO_S *)pStruct->data;
			delete [] (msg_struct_s *)pStruct;
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

	msg_struct_list_s addressList = {0,};

	int order = MsgGetContactNameOrder();
	msg_struct_s *pAddrInfo = NULL;
	MSG_ADDRESS_INFO_S *address = NULL;

	memset(displayName, 0x00, sizeof(displayName));

	MsgStoGetAddressByConvId(pDbHandle, convId, order, &addressList);

	for (int j = 0; j < addressList.nCount; j++)
	{
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
		MSG_DEBUG("Update Conversation disply name. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	for(int j = 0; j < MAX_TO_ADDRESS_CNT; j++){
		msg_struct_s *pStruct = (msg_struct_s *)addressList.msg_struct_info[j];
		delete [] (MSG_ADDRESS_INFO_S *)pStruct->data;
		delete [] (msg_struct_s *)pStruct;
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

	return err;
}

char *MsgStoReplaceString(const char *origStr, const char *oldStr, const char *newStr)
{
	if (origStr == NULL)
		return NULL;

	char *replaceStr = NULL;
	char *pTemp = NULL;
	int i = 0;
	int matchedCnt = 0;
	int oldStrLen = 0;
	int newStrLen = 0;

	if (g_strcmp0(oldStr, newStr) != 0) {
		oldStrLen = strlen(oldStr);
		newStrLen = strlen(newStr);

		for (i = 0; origStr[i] != '\0';) {
			if (memcmp(&origStr[i], oldStr, oldStrLen) == 0) {
				matchedCnt++;
				i += oldStrLen;
			} else {
				i++;
			}
		}
	} else {
		return g_strdup(origStr);
	}

	replaceStr = (char *)calloc(1, i + sizeof(char) * (matchedCnt * (newStrLen - oldStrLen) + 1));
	if (replaceStr == NULL)
		return NULL;

	pTemp = replaceStr;

	while (*origStr) {
		if (memcmp(origStr, oldStr, oldStrLen) == 0) {
			memcpy(pTemp, newStr, newStrLen);
			pTemp += newStrLen;
			origStr += oldStrLen;
		} else {
			*pTemp++ = *origStr++;
		}
	}

	return replaceStr;
}
