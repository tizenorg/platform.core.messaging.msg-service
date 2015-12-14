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


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgStoGetText(msg_message_id_t msgId, char *pSubject, char *pMsgText)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SUBJECT, MSG_TEXT FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if(dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if(dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		char *subject = (char*)dbHandle->columnText(0);
		char *text = (char*)dbHandle->columnText(1);

		if(subject)
			strncpy(pSubject, subject, MAX_SUBJECT_LEN);
		if(text)
			strncpy(pMsgText, text, MAX_MSG_TEXT_LEN);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}



msg_error_t MsgStoUpdateMMSMessage(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	MSG_SEC_DEBUG("pMsg->msgText [%s]", pMsg->msgText);
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	unsigned int fileSize = 0;
	char *pFileData = NULL;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	MSG_INFO("[%d] [%d]", pMsg->msgType.subType, pMsg->networkStatus);

	if(pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		if( pMsg->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS ) {
			if (pMsg->displayTime > 0) {
				snprintf(sqlQuery, sizeof(sqlQuery),
						"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, FOLDER_ID = %d, DISPLAY_TIME = %lu, SUBJECT = ?, NETWORK_STATUS = %d, MSG_TEXT = ?, THUMB_PATH = '%s', DATA_SIZE = %d WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->folderId, pMsg->displayTime, pMsg->networkStatus, pMsg->thumbPath,  pMsg->dataSize, pMsg->msgId);
			} else {
				snprintf(sqlQuery, sizeof(sqlQuery),
						"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, FOLDER_ID = %d, SUBJECT = ?, NETWORK_STATUS = %d, MSG_TEXT = ?, THUMB_PATH = '%s', DATA_SIZE = %d WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->folderId, pMsg->networkStatus, pMsg->thumbPath,  pMsg->dataSize, pMsg->msgId);
			}

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				return MSG_ERR_DB_PREPARE;
			}

			dbHandle->bindText(pMsg->subject, 1);

			if (pMsg->msgText[0] != '\0' && g_file_get_contents((gchar*)pMsg->msgText, (gchar**)&pFileData, (gsize*)&fileSize, NULL) == true) {
				dbHandle->bindText(pFileData, 2);
			} else {
				dbHandle->bindText("", 2);
			}

		} else if (pMsg->networkStatus == MSG_NETWORK_RETRIEVE_FAIL) {
			snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, SUBJECT = ?, NETWORK_STATUS = %d, MSG_TEXT = ?, THUMB_PATH = '%s' WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->networkStatus, pMsg->thumbPath,  pMsg->msgId);

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				return MSG_ERR_DB_PREPARE;
			}

//			if (MsgOpenAndReadFile(pMsg->msgText, &pFileData, &fileSize) == false) {
			if (g_file_get_contents((gchar*)pMsg->msgText, (gchar**)&pFileData, (gsize*)&fileSize, NULL) == false) {
				return MSG_ERR_STORAGE_ERROR;
			}

			dbHandle->bindText(pMsg->subject, 1);

			if (pMsg->msgText[0] != '\0' && g_file_get_contents((gchar*)pMsg->msgText, (gchar**)&pFileData, (gsize*)&fileSize, NULL) == true) {
				dbHandle->bindText(pFileData, 2);
			} else {
				dbHandle->bindText("", 2);
			}
		}
	} else if (pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery),
				"UPDATE %s SET MSG_TEXT = ?, THUMB_PATH = '%s', DATA_SIZE = %d WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, pMsg->thumbPath, pMsg->dataSize, pMsg->msgId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			return MSG_ERR_DB_PREPARE;
		}

		if (pMsg->msgText[0] != '\0' && g_file_get_contents((gchar*)pMsg->msgText, (gchar**)&pFileData, (gsize*)&fileSize, NULL) == true) {
			dbHandle->bindText(pFileData, 1);
		} else {
			dbHandle->bindText("", 1);
		}
	} else {
		if (pMsg->msgType.subType == MSG_SENDCONF_MMS && pMsg->networkStatus == MSG_NETWORK_SEND_FAIL) {
			snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, FOLDER_ID = %d, NETWORK_STATUS = %d, DISPLAY_TIME = %lu, READ_STATUS = 0 WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->folderId, pMsg->networkStatus, pMsg->displayTime, pMsg->msgId);

		} else {
			snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET MAIN_TYPE = %d, SUB_TYPE = %d, FOLDER_ID = %d, NETWORK_STATUS = %d, DISPLAY_TIME = %lu WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->folderId, pMsg->networkStatus, pMsg->displayTime, pMsg->msgId);
		}
		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			return MSG_ERR_DB_PREPARE;
		}
	}

	MSG_DEBUG("[%s]", sqlQuery);

	err = dbHandle->stepQuery();

	if (pFileData != NULL) {
		g_free(pFileData);
	}

	if (err != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		MSG_DEBUG("Update MMS Message. Fail.");
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	/* PLM P141008-05143  :  Notification.Ind address is 1, but MMS retreived Conf address is correct.
	 * So adding code for comparing exist address and new address and replace with new address(MMSconf) address */

	if(pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		if (pMsg->addressList) {
			if (pMsg->nAddressCnt == 1) {
				char tmpAddressVal[MAX_ADDRESS_VAL_LEN+1] = {0, };
				msg_address_type_t tmpAddressType;
				msg_recipient_type_t tmpRecipientType;
				int tmpConvId;

				/* compare stored address and currnt address */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));

				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
						"A.CONV_ID, "
						"A.ADDRESS_TYPE, "
						"A.RECIPIENT_TYPE, "
						"A.ADDRESS_VAL "
						"FROM %s A, %s B "
						"WHERE A.CONV_ID = B.CONV_ID "
						"AND B.MSG_ID = %d;",
						MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
						pMsg->msgId);

				MSG_DEBUG("[%s]", sqlQuery);

				if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
					return MSG_ERR_DB_PREPARE;
				}

				err = dbHandle->stepQuery();

				if (err == MSG_ERR_DB_ROW) {
					tmpConvId = dbHandle->columnInt(0);
					tmpAddressType = dbHandle->columnInt(1);
					tmpRecipientType = dbHandle->columnInt(2);
					strncpy(tmpAddressVal, (char*)dbHandle->columnText(3), MAX_ADDRESS_VAL_LEN);

					dbHandle->finalizeQuery();

					/* compare stored addressList and current addressList */
					if (tmpAddressType != pMsg->addressList->addressType ||
						tmpRecipientType != pMsg->addressList->recipientType ||
						(strncmp(tmpAddressVal, pMsg->addressList->addressVal, MAX_ADDRESS_VAL_LEN) != 0)) {
						MSG_WARN("AddressList of NotiInd and MMSConf are different!!, Replace AddressList to MMSConf data");
						MSG_WARN("AddType [NotiInd : %d], [MMSConf : %d]", tmpAddressType, pMsg->addressList->addressType);
						MSG_WARN("RcptType [NotiInd : %d], [MMSConf : %d]", tmpRecipientType, pMsg->addressList->recipientType);
						MSG_SEC_INFO("AddressVal [NotiInd : %s], [MMSConf : %s]", tmpAddressVal, pMsg->addressList->addressVal);

						/* If MMSConf AddressList is already exist, Replace exist ConvId with matching msgId */
						memset(sqlQuery, 0x00, sizeof(sqlQuery));

						snprintf(sqlQuery, sizeof(sqlQuery),
							"SELECT CONV_ID FROM %s WHERE ADDRESS_VAL = '%s'",
							MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList->addressVal);

						MSG_DEBUG("[%s]", sqlQuery);

						if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
							return MSG_ERR_DB_PREPARE;
						}

						err = dbHandle->stepQuery();

						if (err == MSG_ERR_DB_ROW) {
							tmpConvId = dbHandle->columnInt(0);

							dbHandle->finalizeQuery();

							memset(sqlQuery, 0x00, sizeof(sqlQuery));

							snprintf(sqlQuery, sizeof(sqlQuery),
								"UPDATE %s SET CONV_ID = %d WHERE MSG_ID = %d;",
								MSGFW_MESSAGE_TABLE_NAME, tmpConvId, pMsg->msgId);

							MSG_DEBUG("[%s]", sqlQuery);

							if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
								return MSG_ERR_DB_PREPARE;
							}

							err = dbHandle->stepQuery();

							if (err != MSG_ERR_DB_DONE) {
								dbHandle->finalizeQuery();
								MSG_ERR("Replacing CONV_ID with exist one. Fail.");
								return MSG_ERR_DB_STEP;
							}

							dbHandle->finalizeQuery();
						} else {
							dbHandle->finalizeQuery();

							memset(sqlQuery, 0x00, sizeof(sqlQuery));

							snprintf(sqlQuery, sizeof(sqlQuery),
								"UPDATE %s SET ADDRESS_TYPE = %d, RECIPIENT_TYPE = %d, ADDRESS_VAL = '%s' WHERE CONV_ID = %d;",
								MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList->addressType, pMsg->addressList->recipientType, pMsg->addressList->addressVal, tmpConvId);

							MSG_DEBUG("[%s]", sqlQuery);

							if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
								return MSG_ERR_DB_PREPARE;
							}

							err = dbHandle->stepQuery();

							if (err != MSG_ERR_DB_DONE) {
								dbHandle->finalizeQuery();
								MSG_ERR("Replacing Address with MMSConf Address. Fail.");
								return MSG_ERR_DB_STEP;
							}

							dbHandle->finalizeQuery();
						}
					}
				} else {
					dbHandle->finalizeQuery();
					return MSG_ERR_DB_STEP;
				}
			} else if (pMsg->nAddressCnt > 1) {
				msg_thread_id_t conv_id = 0;
				msg_thread_id_t prev_conv_id = MsgGetThreadId(dbHandle, pMsg->msgId);

				err = MsgStoAddAddress(dbHandle, pMsg, &conv_id);
				if (err != MSG_SUCCESS) {
					MSG_ERR("MsgStoAddAddress is failed");
					return err;
				}
				pMsg->threadId = conv_id;

				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery),
					"UPDATE %s SET CONV_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, conv_id, pMsg->msgId);

				err = dbHandle->execQuery(sqlQuery);
				if (err != MSG_SUCCESS) {
					MSG_ERR("execQuery is failed");
					return err;
				}

				MSG_DEBUG("prev_conv_id[%d] conv_id[%d]", prev_conv_id, conv_id);
				if (prev_conv_id != 0 && prev_conv_id != conv_id) {
					if (MsgStoUpdateConversation(dbHandle, prev_conv_id) != MSG_SUCCESS) {
						MSG_DEBUG("MsgStoUpdateConversation() Error");
						return MSG_ERR_STORAGE_ERROR;
					}

					if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
						MSG_DEBUG("MsgStoClearConversationTable() Error");
					}
				}
			}
		}
	}

	msg_thread_id_t convId = MsgGetThreadId(dbHandle, pMsg->msgId);

	MSG_DEBUG("Conversation id:[%d]", convId);

	if (convId > 0) {
		if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoUpdateConversation() Error");
			dbHandle->freeTable();

			return MSG_ERR_STORAGE_ERROR;
		}

		if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoClearConversationTable() Error");
		}
	} else {
		return MSG_ERR_DB_STEP;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetContentLocation(MSG_MESSAGE_INFO_S* pMsgInfo)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONTENTS_LOCATION FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, pMsgInfo->msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (dbHandle->columnText(0) != NULL) {
			strncpy(pMsgInfo->msgData, (char*)dbHandle->columnText(0), MAX_MSG_DATA_LEN);
			pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
		} else {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_NORECORD;
		}
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoSetReadReportSendStatus(msg_message_id_t msgId, int readReportSendStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();
	bool bReadReportSent = false;

	if((MmsRecvReadReportSendStatus)readReportSendStatus == MMS_RECEIVE_READ_REPORT_SENT)
		bReadReportSent = true;
	else
		bReadReportSent = false;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_REPORT_SEND_STATUS = %d, READ_REPORT_SENT = %d WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, (MmsRecvReadReportSendStatus)readReportSendStatus, (int)bReadReportSent, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetOrgAddressList(MSG_MESSAGE_INFO_S *pMsg)
{
	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0, index = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"A.ADDRESS_TYPE, "
			"A.RECIPIENT_TYPE, "
			"A.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.CONV_ID = B.CONV_ID "
			"AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return err;
	}

	for (int i = 0; i < rowCnt; i++) {
		pMsg->addressList[i].addressType = dbHandle->getColumnToInt(index++);
		pMsg->addressList[i].recipientType = dbHandle->getColumnToInt(index++);
		dbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pMsg->addressList[i].addressVal);
	}

	pMsg->nAddressCnt = rowCnt;

	dbHandle->freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetSubject(msg_message_id_t msgId, char *pSubject)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SUBJECT FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		strncpy(pSubject, (char*)dbHandle->columnText(0), MAX_SUBJECT_LEN);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetRecipientList(msg_message_id_t msgId, MSG_RECIPIENTS_LIST_S *pRecipientList)
{
	if (pRecipientList == NULL) {
		MSG_DEBUG("pRecipientList is NULL");
		return MSG_ERR_NULL_POINTER;
	}
	MsgDbHandler *dbHandle = getDbHandle();
	int rowCnt = 0, index = 0;
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"B.ADDRESS_TYPE, "
			"B.RECIPIENT_TYPE, "
			"B.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.MSG_ID = %d "
			"AND A.CONV_ID = B.CONV_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME,
			msgId);

	if (dbHandle->getTable(sqlQuery, &rowCnt, &index) != MSG_SUCCESS) {
		dbHandle->freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	pRecipientList->recipientCnt = rowCnt;

	MSG_DEBUG("pRecipientList->recipientCnt [%d]", pRecipientList->recipientCnt);

	pRecipientList->recipientAddr = (MSG_ADDRESS_INFO_S*)new char[sizeof(MSG_ADDRESS_INFO_S)*rowCnt];

	MSG_ADDRESS_INFO_S* pTmp = pRecipientList->recipientAddr;

	for (int i = 0; i < rowCnt; i++) {
		pTmp->addressType = dbHandle->getColumnToInt(index++);
		pTmp->recipientType = dbHandle->getColumnToInt(index++);

		memset(pTmp->addressVal, 0x00, sizeof(pTmp->addressVal));
		dbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pTmp->addressVal);

		strncpy(pTmp->displayName, pTmp->addressVal, MAX_DISPLAY_NAME_LEN);

		pTmp++;
	}

	dbHandle->freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetReadStatus(msg_message_id_t msgId, bool *pReadStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT READ_STATUS FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		*pReadStatus = (bool)dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddrInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	// Add Address
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"A.ADDRESS_TYPE, "
			"A.RECIPIENT_TYPE, "
			"A.CONTACT_ID, "
			"A.ADDRESS_VAL "
			"FROM %s A, %s B "
			"WHERE A.CONV_ID = B.CONV_ID "
			"AND B.MSG_ID = %d;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
			msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pAddrInfo->addressType = dbHandle->columnInt(0);
		pAddrInfo->recipientType = dbHandle->columnInt(1);
		pAddrInfo->contactId = dbHandle->columnInt(2);

		strncpy(pAddrInfo->addressVal, (char*)dbHandle->columnText(3), MAX_ADDRESS_VAL_LEN);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetReadReportSendStatus(msg_message_id_t msgId, int *pReadReportSendStatus)
{
	MmsRecvReadReportSendStatus readReportSendStatus = MMS_RECEIVE_READ_REPORT_NO_SEND;
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT "
			"READ_REPORT_SEND_STATUS "
			"FROM %s WHERE MSG_ID = %d;",
			MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		readReportSendStatus = (MmsRecvReadReportSendStatus)dbHandle->columnInt(0);

		*pReadReportSendStatus = readReportSendStatus;

	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}
