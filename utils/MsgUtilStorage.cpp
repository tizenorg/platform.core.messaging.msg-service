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

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgUtilStorage.h"

#include <sys/stat.h>
#include <sys/vfs.h>


static int msgCntLimit[MSG_COUNT_LIMIT_MAILBOX_TYPE_MAX][MSG_COUNT_LIMIT_MSG_TYPE_MAX] = {{1500, 500, 0, 50, 50}, {50, 50, 0, 0, 0}, {1000, 250, 0, 0, 0}, {50, 50, 0, 0, 0}, {0, 0, 200, 0, 0}};


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
unsigned int MsgStoAddMessageTable(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, unsigned int AddrId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int msgId = 0;

	err = pDbHandle->getRowId(MSGFW_MESSAGE_TABLE_NAME, &msgId);

	if (err != MSG_SUCCESS)
	{
		return 0;
	}

	int fileSize = 0;

	char* pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	// Get File Data
	if (pMsgInfo->bTextSms == false)
	{
		if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
			return 0;

		MSG_DEBUG("file size [%d]", fileSize);
	}

	// Add Message
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %ld, %d, ?, ?, ?, ?, %d, 0, %d, 0, 0);",
				MSGFW_MESSAGE_TABLE_NAME, msgId, AddrId, pMsgInfo->folderId, msgId, pMsgInfo->storageId,
				pMsgInfo->msgType.mainType, pMsgInfo->msgType.subType, pMsgInfo->displayTime, pMsgInfo->dataSize,
				pMsgInfo->networkStatus, pMsgInfo->bRead, pMsgInfo->bProtected, pMsgInfo->priority,
				pMsgInfo->direction, pMsgInfo->scheduledTime, pMsgInfo->bBackup, MSG_DELIVERY_REPORT_NONE, MSG_READ_REPORT_NONE);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
	{
		return 0;
	}

	pDbHandle->bindText(pMsgInfo->subject, 1);

	pDbHandle->bindText(pMsgInfo->msgData, 2);

	pDbHandle->bindText(pMsgInfo->thumbPath, 3);

	if (pMsgInfo->bTextSms == false)
		pDbHandle->bindText(pFileData, 4);
	else
		pDbHandle->bindText(pMsgInfo->msgText, 4);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		return 0;
	}

	pDbHandle->finalizeQuery();

	return msgId;
}


MSG_ERROR_T MsgStoSetReadStatus(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId, bool bRead)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_STATUS = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, (int)bRead, MsgId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	// Get MAIN_TYPE, SUB_TYPE, STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MAIN_TYPE, A.SUB_TYPE, B.CONTACT_ID, B.ADDRESS_ID \
				        FROM %s A, %s B \
				     WHERE A.MSG_ID = %d AND A.ADDRESS_ID = B.ADDRESS_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MsgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	MSG_MESSAGE_TYPE_S msgType;
	MSG_CONTACT_ID_T contactId;
	unsigned int addrId;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW)
	{
		msgType.mainType = pDbHandle->columnInt(0);
		msgType.subType = pDbHandle->columnInt(1);
		contactId = pDbHandle->columnInt(2);
		addrId = pDbHandle->columnInt(3);
	}
	else
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	MSG_DEBUG("Main Type:[%d] SubType:[%d] ContactId:[%d] AddrId:[%d]", msgType.mainType, msgType.subType, contactId, addrId);

	if (MsgStoUpdateAddress(pDbHandle, addrId) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoUpdateAddress() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	int smsCnt = 0, mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(pDbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(pDbHandle, MSG_MMS_TYPE);

	MsgSettingSetIndicator(smsCnt, mmsCnt);

	MsgDeleteNotiByMsgId(MsgId);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetOldestMessage(MsgDbHandler *pDbHandle, const MSG_MESSAGE_INFO_S *pMsgInfo, MSG_MESSAGE_ID_T *pMsgId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID \
		 						       FROM %s \
						 		    WHERE SUB_TYPE = %d AND FOLDER_ID = %d AND STORAGE_ID = %d AND PROTECTED = 0 \
						 		     ORDER BY DISPLAY_TIME ASC",
				MSGFW_MESSAGE_TABLE_NAME, pMsgInfo->msgType.subType, pMsgInfo->folderId, MSG_STORAGE_PHONE);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW)
	{
		*pMsgId = pDbHandle->columnInt(0);
	}
	else
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoCheckMsgCntFull(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S* pMsgType, MSG_FOLDER_ID_T FolderId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	struct statfs buf = {0};

	if (statfs(MSG_DATA_ROOT_PATH, &buf) == -1)
	{
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

	return err;

	MSG_END();

	return err;
}


MSG_ERROR_T MsgStoCountMsgByLimitCategory(MsgDbHandler *pDbHandle, const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount, MSG_FOLDER_ID_T folderId )
{
	if (pMsgType == NULL)
	{
		MSG_DEBUG("pMsgType is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	*pMsgCount = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_WAP_SI_SMS ||pMsgType->subType == MSG_WAP_SL_SMS)) // PUSH
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_INBOX_ID);
	}
	else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_CB_SMS)) // CB
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_CB_SMS, MSG_CBMSGBOX_ID);
	}
	else if ((pMsgType->mainType == MSG_SMS_TYPE) && (pMsgType->subType == MSG_SYNCML_CP)) // Provision
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SYNCML_CP, MSG_INBOX_ID);
	}
	else if ((pMsgType->mainType == MSG_SMS_TYPE)) // SMS
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE NOT IN (%d, %d, %d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_CB_SMS, MSG_SYNCML_CP, MSG_INBOX_ID); // etc SMS
	}
	else if ((pMsgType->mainType == MSG_MMS_TYPE) && (pMsgType->subType == MSG_SENDREQ_MMS || pMsgType->subType == MSG_SENDCONF_MMS || pMsgType->subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgType->subType == MSG_RETRIEVE_MANUALCONF_MMS || pMsgType->subType == MSG_NOTIFICATIONIND_MMS)) // MMS
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d, %d) AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SENDREQ_MMS, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS, MSG_NOTIFICATIONIND_MMS, folderId);
	}
	else
	{
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW)
	{
		*pMsgCount = pDbHandle->columnInt(0);
	}
	else
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


int MsgStoCheckMsgCntLimit(const MSG_MESSAGE_TYPE_S* pMsgType, MSG_FOLDER_ID_T FolderId)
{
	int msgboxType = -1;
	int msgType= -1;

	switch (FolderId)
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
			MSG_DEBUG("Unknown mailbox Type [%d]", FolderId);
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


MSG_ERROR_T MsgStoAddAddress(MsgDbHandler *pDbHandle, const MSG_ADDRESS_INFO_S *pAddrInfo, unsigned int *pAddrId)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Check if new address or not
	if (MsgExistAddress(pDbHandle, pAddrInfo->addressVal, pAddrId) == true)
	{
		MSG_DEBUG("The address already exists. ID : [%d], Value : [%s]", *pAddrId, pAddrInfo->addressVal);
		return err;
	}

	// Get Contact Info
	MSG_CONTACT_INFO_S contactInfo = {0};

	err = MsgGetContactInfo(pAddrInfo, &contactInfo);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgGetContactInfo() fail [%d]", err);
		return err;
	}

	if (pDbHandle->getRowId(MSGFW_ADDRESS_TABLE_NAME, pAddrId) != MSG_SUCCESS)
	{
		return err;
	}

	// Add Address
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, '%s', %d, ?, ?, ?, '%s', 0, 0, 0, 0, 0, 0, 0, 0, '');",
				MSGFW_ADDRESS_TABLE_NAME, *pAddrId, pAddrInfo->addressType, pAddrInfo->recipientType, pAddrInfo->addressVal,
				contactInfo.contactId, contactInfo.imagePath);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	pDbHandle->bindText(contactInfo.displayName, 1);
	pDbHandle->bindText(contactInfo.firstName, 2);
	pDbHandle->bindText(contactInfo.lastName, 3);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		MSG_DEBUG("Add Address Info. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return err;
}


MSG_ERROR_T MsgStoUpdateAddress(MsgDbHandler *pDbHandle, unsigned int AddrId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	int unreadCnt = 0, smsCnt = 0, mmsCnt = 0;

	char msgText[MAX_THREAD_DATA_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	// Get Unread Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
				     		   		WHERE ADDRESS_ID = %d \
				     		        	     AND FOLDER_ID = %d \
				     		        	     AND READ_STATUS = 0;",
				MSGFW_MESSAGE_TABLE_NAME, AddrId, MSG_INBOX_ID);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW)
	{
		unreadCnt = pDbHandle->columnInt(0);
	}
	else if (err != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get SMS Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
				     		   		WHERE ADDRESS_ID = %d \
				     		        	     AND MAIN_TYPE = %d \
				     		        	     AND FOLDER_ID > 0 AND FOLDER_ID < %d;",
				MSGFW_MESSAGE_TABLE_NAME, AddrId, MSG_SMS_TYPE, MSG_CBMSGBOX_ID);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW)
	{
		smsCnt = pDbHandle->columnInt(0);
	}
	else if (err != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get MMS Count
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
				     		   		WHERE ADDRESS_ID = %d \
				     		        	     AND MAIN_TYPE = %d \
				     		        	     AND SUB_TYPE NOT IN (%d, %d, %d) \
				     		        	     AND FOLDER_ID > 0 AND FOLDER_ID < %d;",
				MSGFW_MESSAGE_TABLE_NAME, AddrId, MSG_MMS_TYPE, MSG_DELIVERYIND_MMS,
				MSG_READRECIND_MMS, MSG_READORGIND_MMS, MSG_CBMSGBOX_ID);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW)
	{
		mmsCnt = pDbHandle->columnInt(0);
	}
	else if (err != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Get Latest Msg Data
	MSG_MAIN_TYPE_T mainType = MSG_UNKNOWN_TYPE;
	MSG_SUB_TYPE_T subType = MSG_NORMAL_SMS;
	MSG_DIRECTION_TYPE_T direction = MSG_DIRECTION_TYPE_MO;
	time_t msgTime = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE, MSG_DIRECTION, DISPLAY_TIME, SUBJECT, MSG_TEXT FROM %s \
				      WHERE ADDRESS_ID = %d \
				      AND FOLDER_ID > 0 AND FOLDER_ID < %d \
				      ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, AddrId, MSG_CBMSGBOX_ID);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	err = pDbHandle->stepQuery();

	if (err == MSG_ERR_DB_ROW)
	{
		mainType = pDbHandle->columnInt(0);
		subType = pDbHandle->columnInt(1);
		direction = pDbHandle->columnInt(2);

		msgTime = (time_t)pDbHandle->columnInt(3);

		memset(msgText, 0x00, sizeof(msgText));

		if (mainType == MSG_SMS_TYPE)
		{
			if (pDbHandle->columnText(5) != NULL)
				strncpy(msgText, (char*)pDbHandle->columnText(5), MAX_THREAD_DATA_LEN);
		}
		else if (mainType == MSG_MMS_TYPE)
		{
			if (pDbHandle->columnText(4) != NULL)
			{
				strncpy(msgText, (char*)pDbHandle->columnText(4), MAX_THREAD_DATA_LEN);
			}

			if ((strlen(msgText) <= 0) && (pDbHandle->columnText(5) != NULL) && (subType != MSG_NOTIFICATIONIND_MMS))
			{
				memset(msgText, 0x00, sizeof(msgText));
				strncpy(msgText, (char*)pDbHandle->columnText(5), MAX_THREAD_DATA_LEN);
			}
		}
	}
	else if (err != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	// Update Address Table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET UNREAD_CNT = %d, SMS_CNT = %d, MMS_CNT = %d, MAIN_TYPE = %d, SUB_TYPE = %d, MSG_DIRECTION = %d, MSG_TIME = %ld, MSG_TEXT = ? \
				      WHERE ADDRESS_ID = %d;",
				MSGFW_ADDRESS_TABLE_NAME, unreadCnt, smsCnt, mmsCnt, mainType, subType, direction, msgTime, AddrId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	pDbHandle->bindText(msgText, 1);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		MSG_DEBUG("Update Address Info. Fail [%d] [%s]", err, sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoClearAddressTable(MsgDbHandler *pDbHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s \
				     WHERE ADDRESS_ID NOT IN (SELECT ADDRESS_ID FROM %s) \
				          AND ADDRESS_ID <> 0;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME);

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


bool MsgExistAddress(MsgDbHandler *pDbHandle, const char *pAddress, unsigned int *pAddrId)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	*pAddrId = 0;

	if (strlen(pAddress) > MAX_PRECONFIG_NUM)
	{
		char newPhoneNum[MAX_PRECONFIG_NUM+1];

		memset(newPhoneNum, 0x00, sizeof(newPhoneNum));

		MsgConvertNumber(pAddress, newPhoneNum);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID FROM %s WHERE ADDRESS_VAL LIKE '%%%%%s';",
						MSGFW_ADDRESS_TABLE_NAME, newPhoneNum);
	}
	else
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID FROM %s WHERE ADDRESS_VAL = '%s';",
						MSGFW_ADDRESS_TABLE_NAME, pAddress);
	}

	int rowCnt = 0, addrId = 0;

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	// No record or other error
	if (err != MSG_SUCCESS)
	{
		pDbHandle->freeTable();
		return false;
	}

	addrId = pDbHandle->getColumnToInt(1);

	MSG_DEBUG("AddressId : [%d]", addrId);

	if (addrId > 0)
	{
		*pAddrId = addrId;

		pDbHandle->freeTable();
		return true;
	}
	else
	{
		pDbHandle->freeTable();
		return false;
	}

	pDbHandle->freeTable();

	return false;
}


int MsgStoGetUnreadCnt(MsgDbHandler *pDbHandle, MSG_MAIN_TYPE_T MsgType)
{
	int msgCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (MsgType == MSG_SMS_TYPE)
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
									WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d, %d, %d, %d) AND \
									FOLDER_ID = %d AND READ_STATUS = 0;",
									MSGFW_MESSAGE_TABLE_NAME, MSG_SMS_TYPE, MSG_NORMAL_SMS, MSG_STATUS_REPORT_SMS, MSG_CONCAT_SIM_SMS, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS, MSG_MWI_VOICE_SMS, MSG_SYNCML_CP, MSG_INBOX_ID);
	}
	else if (MsgType == MSG_MMS_TYPE)
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s \
									WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d) AND \
									FOLDER_ID = %d AND READ_STATUS = 0;",
									MSGFW_MESSAGE_TABLE_NAME, MSG_MMS_TYPE, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS, MSG_NOTIFICATIONIND_MMS, MSG_INBOX_ID);
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return 0;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW)
	{
		msgCnt = pDbHandle->columnInt(0);
	}
	else
	{
		pDbHandle->finalizeQuery();
		return 0;
	}

	pDbHandle->finalizeQuery();

	return msgCnt;
}


MSG_ERROR_T MsgStoAddContactInfo(MsgDbHandler *pDbHandle, MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber)
{
	char newPhoneNum[MAX_PRECONFIG_NUM+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	if (strlen(pNumber) > MAX_PRECONFIG_NUM)
	{
		memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
		MsgConvertNumber(pNumber, newPhoneNum);

		MSG_DEBUG("Phone Number to Compare : [%s]", newPhoneNum);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE MSG_ADDRESS_TABLE SET CONTACT_ID = %d, DISPLAY_NAME = ?, FIRST_NAME = ?, LAST_NAME = ?, IMAGE_PATH = '%s' \
										WHERE ADDRESS_VAL LIKE '%%%%%s';",
									pContactInfo->contactId,
									pContactInfo->imagePath,
									newPhoneNum);
	}
	else
	{
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE MSG_ADDRESS_TABLE SET CONTACT_ID = %d, DISPLAY_NAME = ?, FIRST_NAME = ?, LAST_NAME = ?, IMAGE_PATH = '%s' \
										WHERE ADDRESS_VAL = '%s';",
									pContactInfo->contactId,
									pContactInfo->imagePath,
									pNumber);
	}

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	pDbHandle->bindText(pContactInfo->displayName, 1);

	pDbHandle->bindText(pContactInfo->firstName, 2);

	pDbHandle->bindText(pContactInfo->lastName, 3);

	if (pDbHandle->stepQuery() != MSG_ERR_DB_DONE)
	{
		pDbHandle->finalizeQuery();
		MSG_DEBUG("Update contact Info. Fail [%s]", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int ContactId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE MSG_ADDRESS_TABLE SET CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
				WHERE CONTACT_ID = %d;", ContactId);

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
	{
		MSG_DEBUG("Fail to execute query");
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoClearContactInfo(MsgDbHandler *pDbHandle, int ContactId, const char *pNumber)
{
	char newPhoneNum[MAX_PRECONFIG_NUM+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (strlen(pNumber) > MAX_PRECONFIG_NUM)
	{
		MsgConvertNumber(pNumber, newPhoneNum);

		MSG_DEBUG("Phone Number to Compare : [%s]", newPhoneNum);

		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE MSG_ADDRESS_TABLE SET CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
    				WHERE CONTACT_ID = %d AND ADDRESS_VAL NOT LIKE '%%%s';", ContactId, newPhoneNum);
	}
	else
	{
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE MSG_ADDRESS_TABLE SET CONTACT_ID = 0, DISPLAY_NAME = '', FIRST_NAME = '', LAST_NAME = '', IMAGE_PATH = '' \
    				WHERE CONTACT_ID = %d AND ADDRESS_VAL <> '%s';", ContactId, pNumber);
	}

	if (pDbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
	{
		MSG_DEBUG("Fail to execute query");
		return MSG_ERR_DB_EXEC;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetMmsRawFilePath(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T msgId, char *pFilePath)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE REFERENCE_ID IN \
				(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW)
	{
		if (pDbHandle->columnText(0) != NULL)
		{
			strncpy(pFilePath, (char*)pDbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
		}
	}
	else
	{
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


bool MsgStoCheckReadReportRequested(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0;
	bool bReadReportRequested = false;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ASK_READ_REPLY FROM %s WHERE REFERENCE_ID IN \
				(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
				MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MsgId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
	{
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return bReadReportRequested;
	}

	if (rowCnt != 1)
	{
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return bReadReportRequested;
	}

	bReadReportRequested = pDbHandle->getColumnToInt(1);

	pDbHandle->freeTable();

	return bReadReportRequested;
}


bool MsgStoCheckReadReportIsSent(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	int rowCnt = 0;
	bool bReadReportIsSent = true;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT READ_REPORT_SENT FROM %s WHERE REFERENCE_ID IN \
				(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
				MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MsgId);

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD)
	{
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]Failed to Get Table");
		return bReadReportIsSent;
	}

	if (rowCnt != 1)
	{
		pDbHandle->freeTable();
		MSG_DEBUG("[Error]MSG_ERR_DB_NORECORD");
		return bReadReportIsSent;
	}

	bReadReportIsSent = (bool)pDbHandle->getColumnToInt(1);

	pDbHandle->freeTable();

	return bReadReportIsSent;
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
	int replaceSize = 0;

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
