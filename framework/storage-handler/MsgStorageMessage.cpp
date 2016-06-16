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
#include <locale>
#include <glib.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgContact.h"
#include "MsgUtilFile.h"
#include "MsgMutex.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"
#include "MsgNotificationWrapper.h"
#include "MsgDevicedWrapper.h"

using namespace std;


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

MsgMutex delNotiMx;
MsgCndVar delNoticv;
bool delNotiRunning = false;

MsgMutex delLogMx;
MsgCndVar delLogcv;
bool delLogRunning = false;


/*==================================================================================================
                                     FUNCTION FOR THREAD
==================================================================================================*/
static gboolean resetNotification(void *pVoid)
{
	MSG_BEGIN();

	MsgRefreshAllNotification(true, false, MSG_ACTIVE_NOTI_TYPE_NONE);

	MSG_END();

	return FALSE;
}


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

msg_error_t MsgStoAddMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	unsigned int rowId = 0;
	msg_thread_id_t convId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	MsgDbHandler *dbHandle = getDbHandle();

	dbHandle->beginTrans();

	if (pMsg->nAddressCnt > 0) {
		err = MsgStoAddAddress(dbHandle, pMsg, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}

		pMsg->threadId = convId;
	}

	if (pMsg->msgId > 0) {
		rowId = pMsg->msgId;
	} else {
		/* get rowId */
		err = dbHandle->getRowId(MSGFW_MESSAGE_TABLE_NAME, &rowId);
		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}
		pMsg->msgId = (msg_message_id_t)rowId;
	}

	int fileSize = 0;

	char *pFileData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

	/* Get File Data */
	if (pMsg->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
			dbHandle->endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}

		MSG_DEBUG("file size [%d]", fileSize);
	}

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SUBS_ID, pMsg->sim_idx);

	char *imsi = NULL;
	if (MsgSettingGetString(keyName, &imsi) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	/* Add Message */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %u, %d, %d, %d, %d, %d, %d, %d, %d, ?, '', '', ?, 0, %d, '%s');",
			MSGFW_MESSAGE_TABLE_NAME, rowId, convId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType,
			(unsigned int)pMsg->displayTime, pMsg->dataSize, pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction,
			0, pMsg->bBackup, pMsg->sim_idx, imsi);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	g_free(imsi);
	imsi = NULL;

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->bindText(pMsg->subject, 1);

	if (pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		dbHandle->bindText("", 2);
	} else {
		if (pMsg->bTextSms == false)
			dbHandle->bindText(pFileData, 2);
		else
			dbHandle->bindText(pMsg->msgText, 2);
	}

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();

	if (pMsg->msgType.subType != MSG_SENDREQ_MMS) {
		err = MsgStoUpdateConversation(dbHandle, convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}
	}

#if 0
	/* In the case of MMS Message, load the MMS plugin to save MMS PDU */
	if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		if (pMsg->msgType.subType != MSG_DELIVERYIND_MMS && pMsg->msgType.subType != MSG_READORGIND_MMS) {
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);
			if (plg == NULL) {
				dbHandle->endTrans(false);
				return MSG_ERR_NULL_POINTER;
			}

			if (pFileData == NULL) {
				if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
					dbHandle->endTrans(false);
					return MSG_ERR_STORAGE_ERROR;
				}

				MSG_DEBUG("file size [%d]", fileSize);
			}

			err = plg->addMessage(pMsg, pSendOptInfo, pFileData);

			if (err != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_STORAGE_ERROR;
			}

			if (pMsg->msgType.subType == MSG_SENDREQ_MMS
					|| pMsg->msgType.subType == MSG_RETRIEVE_MMS
					|| pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS
					|| pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
				MSG_DEBUG("pMsg->msgText: %s, pMsg->thumbPath: %s ", pMsg->msgText, pMsg->thumbPath);

				err = MsgStoUpdateMMSMessage(pMsg);

				if (err != MSG_SUCCESS) {
					dbHandle->endTrans(false);
					return MSG_ERR_STORAGE_ERROR;
				}
			}
		}
	} else if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL) {
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);
		if (plg == NULL) {
			dbHandle->endTrans(false);
			return MSG_ERR_NULL_POINTER;
		}
		err = plg->addMessage(pMsg, pSendOptInfo, NULL);
		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}
#else
	if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		char *pMmsSerializedData = NULL;
		unique_ptr<char*, void(*)(char**)> buf_mms(&pMmsSerializedData, unique_ptr_deleter);

		if (pMsg->msgType.subType != MSG_DELIVERYIND_MMS && pMsg->msgType.subType != MSG_READORGIND_MMS) {
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);

			if (plg == NULL) {
				dbHandle->endTrans(false);
				return MSG_ERR_NULL_POINTER;
			}

			if (MsgOpenAndReadFile(pMsg->msgData, &pMmsSerializedData, &fileSize) == false) {
				dbHandle->endTrans(false);
				return MSG_ERR_STORAGE_ERROR;
			}

			MSG_DEBUG("file size [%d]", fileSize);

			err = plg->addMessage(pMsg, pSendOptInfo, pMmsSerializedData);

			if (err != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_STORAGE_ERROR;
			}
/*
 *			if (pMsg->msgText[0] != '\0') {
 *				g_file_get_contents((gchar*)pMsg->msgText, (gchar**)&pFileData, (gsize*)&fileSize, NULL);
 *				MSG_DEBUG("file size [%d]", fileSize);
 *			}
 */

			if (pMsg->msgType.subType == MSG_SENDREQ_MMS
				|| pMsg->msgType.subType == MSG_RETRIEVE_MMS
				|| pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS
				|| pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
				MSG_DEBUG("pMsg->msgText: %s, pMsg->thumbPath: %s ", pMsg->msgText, pMsg->thumbPath);
				err = MsgStoUpdateMMSMessage(pMsg);
			}
		}
	} else {
		if (pMsg->bTextSms == false) {
			if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
				dbHandle->endTrans(false);
				return MSG_ERR_STORAGE_ERROR;
			}
			MSG_DEBUG("file size [%d]", fileSize);
		}
	}
#endif
	dbHandle->endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	msg_thread_id_t convId = 0;
	MsgDbHandler *dbHandle = getDbHandle();
	dbHandle->beginTrans();

	MSG_MAIN_TYPE_T	prevType;
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	if (dbHandle->stepQuery() != MSG_ERR_DB_ROW) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	prevType = dbHandle->columnInt(0);
	dbHandle->finalizeQuery();

	/* check msg type with previous type */
	if (prevType != pMsg->msgType.mainType) {
		MSG_DEBUG("different msg type to update [prev : %d], [current : %d]", prevType, pMsg->msgType.mainType);

		err = MsgStoDeleteMessage(pMsg->msgId, false);
		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}

		err = MsgStoAddMessage(pMsg, pSendOptInfo);
		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}

		dbHandle->endTrans(false);
		return err;
	}

	if (pMsg->nAddressCnt > 0) {
		pMsg->threadId = 0;
		err = MsgStoAddAddress(dbHandle, pMsg, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}
	}

	int fileSize = 0;

	char *pFileData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

	/* Get File Data */
	if (pMsg->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
			dbHandle->endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}

	if (pSendOptInfo != NULL) {
		/* Get Global setting value if bSetting == false */
		if (pSendOptInfo->bSetting == false) {
			if (MsgSettingGetBool(MSG_KEEP_COPY, &pSendOptInfo->bKeepCopy) != MSG_SUCCESS)
				MSG_INFO("MsgSettingGetBool() is failed");

			if (pMsg->msgType.mainType == MSG_SMS_TYPE) {
				if (MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &pSendOptInfo->bDeliverReq) != MSG_SUCCESS)
					MSG_INFO("MsgSettingGetBool() is failed");

				if (MsgSettingGetBool(SMS_SEND_REPLY_PATH, &pSendOptInfo->option.smsSendOptInfo.bReplyPath) != MSG_SUCCESS)
					MSG_INFO("MsgSettingGetBool() is failed");
			} else if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
				if (MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &pSendOptInfo->bDeliverReq) != MSG_SUCCESS)
					MSG_INFO("MsgSettingGetBool() is failed");

				if (MsgSettingGetBool(MMS_SEND_READ_REPLY, &pSendOptInfo->option.mmsSendOptInfo.bReadReq) != MSG_SUCCESS)
					MSG_INFO("MsgSettingGetBool() is failed");

				int tmpVal = 0;
				if (MsgSettingGetInt(MMS_SEND_EXPIRY_TIME, &tmpVal) != MSG_SUCCESS) {
					MSG_INFO("MsgSettingGetInt() is failed");
				}
				pSendOptInfo->option.mmsSendOptInfo.expiryTime.time = (unsigned int)tmpVal;

				if (MsgSettingGetInt(MMS_SEND_DELIVERY_TIME, &tmpVal) != MSG_SUCCESS) {
					MSG_INFO("MsgSettingGetInt() is failed");
				}
				MSG_MMS_DELIVERY_TIME_T deliveryTime = (MSG_MMS_DELIVERY_TIME_T)tmpVal;

				if (deliveryTime == MSG_DELIVERY_TIME_CUSTOM) {
					pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime = true;
					if (MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY, &tmpVal) != MSG_SUCCESS) {
						MSG_INFO("MsgSettingGetInt() is failed");
					}
					pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = (unsigned int)tmpVal;
				} else {
					pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime = false;
					pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = (unsigned int)deliveryTime;
				}

				if (MsgSettingGetInt(MMS_SEND_PRIORITY, &tmpVal) != MSG_SUCCESS) {
					MSG_INFO("MsgSettingGetInt() is failed");
				}
				pSendOptInfo->option.mmsSendOptInfo.priority = (msg_priority_type_t)tmpVal;
			}
		}
	}

	/* Update Message */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET CONV_ID = %d, FOLDER_ID = %d, STORAGE_ID = %d, MAIN_TYPE = %d, SUB_TYPE = %d, \
			DISPLAY_TIME = %lu, DATA_SIZE = %d, NETWORK_STATUS = %d, READ_STATUS = %d, PROTECTED = %d, PRIORITY = %d, MSG_DIRECTION = %d, \
			BACKUP = %d, SUBJECT = ?, MSG_TEXT = ? \
			WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, convId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->displayTime, pMsg->dataSize,
			pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction, pMsg->bBackup, pMsg->msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->bindText(pMsg->subject, 1);

	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->bTextSms == false)
		dbHandle->bindText(pFileData, 2);
	else
		dbHandle->bindText(pMsg->msgText, 2);

	MSG_DEBUG("%s", sqlQuery);

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();

	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL) {
		if (pSendOptInfo->bSetting == true) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET "
					"DELREP_REQ = %d, "
					"KEEP_COPY = %d, "
					"REPLY_PATH = %d, "
					"ENCODE_TYPE = %d "
					"WHERE MSG_ID = %d;",
					MSGFW_SMS_SENDOPT_TABLE_NAME,
					pSendOptInfo->bDeliverReq,
					pSendOptInfo->bKeepCopy,
					pSendOptInfo->option.smsSendOptInfo.bReplyPath,
					pMsg->encodeType,
					pMsg->msgId);

			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);
		if (plg == NULL) {
			dbHandle->endTrans(false);
			return MSG_ERR_NULL_POINTER;
		}

		err = plg->updateMessage(pMsg, pSendOptInfo, pFileData);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}

		if (pMsg->msgType.subType == MSG_SENDREQ_MMS) {
			MSG_SEC_DEBUG("pMsg->msgText: %s, pMsg->thumbPath: %s ", pMsg->msgText, pMsg->thumbPath);

			err = MsgStoUpdateMMSMessage(pMsg);

			if (err != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_STORAGE_ERROR;
			}
		}
	}

	err = MsgStoUpdateConversation(dbHandle, convId);

	if (err != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	err = MsgStoClearConversationTable(dbHandle);

	if (err != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateReadStatus(msg_message_id_t msgId, bool bRead)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_storage_id_t storageId;

	MsgDbHandler *dbHandle = getDbHandle();
	if (MsgStoSetReadStatus(dbHandle, msgId, bRead) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoSetReadStatus() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	MsgRefreshAllNotification(true, false, MSG_ACTIVE_NOTI_TYPE_NONE);

	if (bRead == true) {
#ifndef FEATURE_SMS_CDMA
		/* Get STORAGE_ID */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT STORAGE_ID FROM %s WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, msgId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			storageId = dbHandle->columnInt(0);
		} else {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_STEP;
		}
		dbHandle->finalizeQuery();

		MSG_DEBUG("StorageId:[%d]", storageId);

		/* Update Read Status for SIM Msg */
		if (storageId == MSG_STORAGE_SIM) {
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

			if (plg == NULL) {
				MSG_DEBUG("SMS Plug-in is NULL");
				return MSG_ERR_NULL_POINTER;
			}

			/* Get SIM Msg ID */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_SLOT_ID, SIM_ID FROM %s WHERE MSG_ID = %d;",
					MSGFW_SIM_MSG_TABLE_NAME, msgId);

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
				return MSG_ERR_DB_PREPARE;

			msg_sim_id_t simId;
			msg_sim_slot_id_t sim_idx;

			while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
				sim_idx = dbHandle->columnInt(0);
				simId = dbHandle->columnInt(1);

				if (plg->setReadStatus(sim_idx, simId) != MSG_SUCCESS) {
					MSG_DEBUG("Fail to Set Read Status for SIM SMS");
					continue;
				}
			}
			dbHandle->finalizeQuery();
		}
#endif
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateThreadReadStatus(msg_thread_id_t threadId, msg_id_list_s *pMsgIdList)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	/*** Get msg id list **/
	int rowCnt = 0, index = 0;
	pMsgIdList->nCount = 0;
	MsgDbHandler *dbHandle = getDbHandle();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s "
			"WHERE CONV_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d AND FOLDER_ID < %d;",
			MSGFW_MESSAGE_TABLE_NAME,
			threadId, MSG_STORAGE_PHONE, MSG_SPAMBOX_ID);

	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	pMsgIdList->nCount = rowCnt;
	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	pMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pMsgIdList->msgIdList[i] = dbHandle->getColumnToInt(index++);

	dbHandle->freeTable();
	/*** **/

	/* set read status */
	dbHandle->beginTrans();
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_STATUS = %d "
			"WHERE CONV_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d AND FOLDER_ID < %d;",
			MSGFW_MESSAGE_TABLE_NAME, 1,
			threadId, MSG_STORAGE_PHONE, MSG_SPAMBOX_ID);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}


	if (MsgStoUpdateConversation(dbHandle, threadId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoUpdateConversation() Error");
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);
	if (g_idle_add(resetNotification, NULL) == 0) {
		MSG_DEBUG("resetNotification() Error");
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateProtectedStatus(msg_message_id_t msgId, bool bProtected)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET PROTECTED = %d WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, (int)bProtected, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteMessage(msg_message_id_t msgId, bool bCheckIndication)
{
	MSG_BEGIN();

	MSG_DEBUG("Msg Id : %d", msgId);

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();

	/* delete report notification */
	char tempAddr[MAX_ADDRESS_VAL_LEN+1];
	memset(tempAddr, 0x00, sizeof(tempAddr));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.ADDRESS_VAL FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND A.MSG_ID = %d;"
			, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() != MSG_ERR_DB_ROW) {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_DONE;
	}

	snprintf(tempAddr, sizeof(tempAddr), "%s", dbHandle->columnText(0));

	MSG_SEC_DEBUG("Updated address = %s", tempAddr);

	MsgDeleteReportNotification(tempAddr);

	dbHandle->finalizeQuery();

	/* Get SUB_TYPE, STORAGE_ID */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE, FOLDER_ID, STORAGE_ID, READ_STATUS, CONV_ID, SIM_INDEX \
			FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	MSG_MESSAGE_TYPE_S msgType;
	msg_folder_id_t folderId;
	msg_storage_id_t storageId;
	msg_thread_id_t convId;
	msg_sim_slot_id_t simIndex;
	bool bRead;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = dbHandle->columnInt(0);
		msgType.subType = dbHandle->columnInt(1);
		folderId = dbHandle->columnInt(2);
		storageId = dbHandle->columnInt(3);
		bRead = dbHandle->columnInt(4);
		convId = dbHandle->columnInt(5);
		simIndex = dbHandle->columnInt(6);

		MSG_DEBUG("Main Type:[%d] SubType:[%d] FolderId:[%d] StorageId:[%d] ReadStatus:[%d] ConversationId:[%d], simIndex=[%d]", msgType.mainType, msgType.subType, folderId, storageId, bRead, convId, simIndex);
	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);

		dbHandle->finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	if (plg == NULL) {
		MSG_DEBUG("SMS Plug-in is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	dbHandle->beginTrans();

#ifndef FEATURE_SMS_CDMA
	/* Check sim message */
	if (storageId == MSG_STORAGE_SIM) {
		msg_sim_id_t simMsgId;
		msg_sim_slot_id_t sim_idx;

		while (true) {
			/* get sim message id */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_SLOT_ID, SIM_ID FROM %s WHERE MSG_ID = %d;",
					MSGFW_SIM_MSG_TABLE_NAME, msgId);

			MSG_DEBUG("sqlQuery is [%s]", sqlQuery);

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_PREPARE;
			}

			if (dbHandle->stepQuery() != MSG_ERR_DB_ROW) {
				break;
			}

			sim_idx = dbHandle->columnInt(0);
			simMsgId = dbHandle->columnInt(1);

			MSG_DEBUG("SIM Msg Id : [%d]", simMsgId);

			err = plg->deleteSimMessage(sim_idx, simMsgId);

			if (err != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				return err;
			}

			dbHandle->finalizeQuery();

			/*Sim message delete in db table */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE SIM_ID = %d AND SIM_SLOT_ID =%d;", MSGFW_SIM_MSG_TABLE_NAME, simMsgId, sim_idx);

			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->finalizeQuery();
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}

		dbHandle->finalizeQuery();
	}
#endif
	/* each type has to be handled in plug in ? */
	if (msgType.mainType == MSG_SMS_TYPE) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_SMS_SENDOPT_TABLE_NAME, msgId);

		/* Delete SMS Send Option */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_SMS_REPORT_TABLE_NAME, msgId);

		/* Delete Data from SMS_REPORT table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (msgType.subType == MSG_CB_SMS || msgType.subType == MSG_JAVACB_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_CB_MSG_TABLE_NAME, msgId);

			/* Delete Push Message from push table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType >= MSG_WAP_SI_SMS && msgType.subType <= MSG_WAP_CO_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_PUSH_MSG_TABLE_NAME, msgId);

			/* Delete Push Message from push table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType == MSG_SYNCML_CP) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_SYNCML_MSG_TABLE_NAME, msgId);

			/* Delete SyncML Message from syncML table */
			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else if (msgType.mainType == MSG_MMS_TYPE) {
		char filePath[MSG_FILEPATH_LEN_MAX + 1] = {0, };
		char dirPath[MSG_FILEPATH_LEN_MAX + 1]= {0, };

		/*remove multipart */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			memset(filePath, 0x00, sizeof(filePath));
			strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
		}
		dbHandle->finalizeQuery();

		/* Delete Data from Multipart table */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_MMS_MULTIPART_TABLE_NAME, msgId);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);

			snprintf(dirPath, MSG_FILEPATH_LEN_MAX, "%s.dir", filePath);

			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);

			MsgRmRf(dirPath);

			rmdir(dirPath);

		} else {
			MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			return MSG_ERR_DB_STEP;
		}

		dbHandle->finalizeQuery();

		/* remove thumbnail file */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT VALUE FROM %s "
				"WHERE MSG_ID = %d AND (TYPE=%d OR TYPE=%d);",
				MSGFW_MMS_PREVIEW_TABLE_NAME, msgId, MSG_MMS_ITEM_TYPE_IMG, MSG_MMS_ITEM_TYPE_VIDEO);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			memset(filePath, 0x00, sizeof(filePath));
			strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
		}

		dbHandle->finalizeQuery();

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_MMS_PREVIEW_TABLE_NAME, msgId);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

		/* Delete Data from MMS table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_REPORT_TABLE_NAME, msgId);

		/* Delete Data from MMS STATUS table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgId);

	/* Delete Message from msg table */
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

#ifdef FEATURE_SMS_CDMA
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_UNIQUENESS_INFO_TABLE_NAME, msgId);

	/* Delete Message from uniqueness table */
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}
#endif

	if (convId > 0) {
		/* Clear Conversation table */
		if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		/* Update conversation table. */
		if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}
	dbHandle->endTrans(true);

	if (msgType.mainType == MSG_SMS_TYPE && folderId == MSG_INBOX_ID) {
		msgType.classType = MSG_CLASS_NONE;

		/* Set memory status in SIM */
		if (MsgStoCheckMsgCntFull(dbHandle, &msgType, folderId) == MSG_SUCCESS) {
			MSG_DEBUG("Set Memory Status");

			plg->setMemoryStatus(simIndex, MSG_SUCCESS);
		}
	}

	if (bCheckIndication == true) {
		MSG_DEBUG("bCheckIndication is true.");
		MsgRefreshAllNotification(true, false, MSG_ACTIVE_NOTI_TYPE_NONE);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteAllMessageInFolder(msg_folder_id_t folderId, bool bOnlyDB, msg_id_list_s *pMsgIdList)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<msg_thread_id_t> threadList;

#ifdef FEATURE_SMS_CDMA
	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME,
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MMS_PREVIEW_TABLE_NAME,
						MSGFW_REPORT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
						MSGFW_UNIQUENESS_INFO_TABLE_NAME};
#else
	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME,
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MMS_PREVIEW_TABLE_NAME,
						MSGFW_REPORT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME};
#endif

	int listCnt = sizeof(tableList)/sizeof(char *);
	int rowCnt = 0;
	MsgDbHandler *dbHandle = getDbHandle();
	signed char folder_id;

	/* Get conversation ID from Folder */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, folderId);

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++) {
		MSG_DEBUG("thread ID : %d", dbHandle->getColumnToInt(i));
		threadList.push((msg_thread_id_t)dbHandle->getColumnToInt(i));
	}

	dbHandle->freeTable();

	/*** Get msg id list **/
	msg_id_list_s *pToDeleteMsgIdList = NULL;
	pToDeleteMsgIdList = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
	if (pToDeleteMsgIdList == NULL) {
		MSG_DEBUG("pToDeleteMsgIdList is NULL.");
		return MSG_ERR_NULL_POINTER;
	}
	memset(pToDeleteMsgIdList, 0x00, sizeof(msg_id_list_s));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, folderId);

	rowCnt = 0;
	int index = 0;

	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		goto FREE_MEMORY;
	}

	if (rowCnt <= 0) {
		dbHandle->freeTable();
		err = MSG_SUCCESS;

		goto FREE_MEMORY;
	}

	pToDeleteMsgIdList->nCount = rowCnt;

	MSG_DEBUG("pToDeleteMsgIdList->nCount [%d]", pToDeleteMsgIdList->nCount);

	pToDeleteMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];
	memset(pToDeleteMsgIdList->msgIdList, 0x00, sizeof(msg_message_id_t) * rowCnt);

	for (int i = 0; i < rowCnt; i++)
		pToDeleteMsgIdList->msgIdList[i] = dbHandle->getColumnToInt(index++);

	dbHandle->freeTable();
	/*** **/

	/*** Delete Sim Message In Folder **/
	folder_id = (signed char)folderId;
	if (folder_id >= MSG_INBOX_ID) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND STORAGE_ID = %d",
				MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_STORAGE_SIM);

		rowCnt = 0;

		err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);
		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
			dbHandle->freeTable();
			goto FREE_MEMORY;
		}

		for (int i = 1; i <= rowCnt; i++) {
			err = MsgStoDeleteMessage(dbHandle->getColumnToInt(i), false);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoDeleteMessage() Error!!!");

				dbHandle->freeTable();

				goto FREE_MEMORY;
			}

			/*Delete phone log */
/*			MsgDeletePhoneLog(dbHandle->getColumnToInt(i)); */
		}

		dbHandle->freeTable();
	}

	dbHandle->beginTrans();

	for (int i = 0; i < listCnt; i++) {
		if (!strcmp(tableList[i], MMS_PLUGIN_MESSAGE_TABLE_NAME)) {
			int rowCnt = 0;

			char filePath[MSG_FILEPATH_LEN_MAX] = {0, };
			char dirPath[MSG_FILEPATH_LEN_MAX] = {0, };
			/* remove multipart */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s;",
					MSGFW_MMS_MULTIPART_TABLE_NAME);

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_PREPARE;
			}

			while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
				memset(filePath, 0x00, sizeof(filePath));
				strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
				if (remove(filePath) == -1)
					MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
			}
			dbHandle->finalizeQuery();

			/* Delete Data from Multipart table */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;",
					MSGFW_MMS_MULTIPART_TABLE_NAME);

			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				err = MSG_ERR_DB_EXEC;
				dbHandle->endTrans(false);
				goto FREE_MEMORY;
			}

			/*get mms msg id list */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.FILE_PATH FROM %s A, %s B \
					WHERE A.FOLDER_ID = %d AND A.MAIN_TYPE = %d AND A.MSG_ID = B.MSG_ID",
					MSGFW_MESSAGE_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME, folderId, MSG_MMS_TYPE);

			err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);
			MSG_DEBUG("rowCnt %d", rowCnt);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("Fail to getTable().");

				dbHandle->freeTable();
				dbHandle->endTrans(false);

				goto FREE_MEMORY;
			}

			for (int i = 1; i <= rowCnt; i++) {
				memset(filePath, 0x00, sizeof(filePath));
				dbHandle->getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);

				MSG_SEC_DEBUG("filePath [%s]", filePath);

				/*delete raw file */
				snprintf(dirPath, sizeof(dirPath), "%s.dir", filePath);

				if (remove(filePath) == -1)
					MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_SEC_DEBUG("Success to delete file [%s]", filePath);

				MsgRmRf(dirPath);

				rmdir(dirPath);
			}

			dbHandle->freeTable();
		}

		/* delete thumbnail */
		char filePath[MSG_FILEPATH_LEN_MAX] = {0, };
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT VALUE FROM %s "
				"WHERE (TYPE=%d OR TYPE=%d) "
				"AND (MSG_ID IN (SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d));",
				MSGFW_MMS_PREVIEW_TABLE_NAME, MSG_MMS_ITEM_TYPE_IMG, MSG_MMS_ITEM_TYPE_VIDEO, MSGFW_MESSAGE_TABLE_NAME, folderId);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		while (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
			memset(filePath, 0x00, sizeof(filePath));
			strncpy(filePath, (char *)dbHandle->columnText(0), MSG_FILEPATH_LEN_MAX);
			if (remove(filePath) == -1)
				MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
		}

		dbHandle->finalizeQuery();

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
				(SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d);",
				tableList[i], MSGFW_MESSAGE_TABLE_NAME, folderId);

		/* Delete Message in specific folder from table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbHandle->endTrans(false);
			err = MSG_ERR_DB_EXEC;

			goto FREE_MEMORY;
		}
	}

	/* Clear Conversation table */
	if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		err = MSG_ERR_DB_EXEC;

		goto FREE_MEMORY;
	}

	/* Update Address */
	while (!threadList.empty()) {
		err = MsgStoUpdateConversation(dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);

			goto FREE_MEMORY;
		}
	}

	dbHandle->endTrans(true);

	/* Set pMsgIdList */
	if (pMsgIdList != NULL && pToDeleteMsgIdList->nCount > 0) {
		pMsgIdList->nCount = pToDeleteMsgIdList->nCount;

		pMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * pToDeleteMsgIdList->nCount];
		memset(pMsgIdList->msgIdList, 0x00, sizeof(msg_message_id_t) * pToDeleteMsgIdList->nCount);
		memcpy(pMsgIdList->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(msg_message_id_t) * pToDeleteMsgIdList->nCount);
	}

	/* Create thread  for noti and phone log delete. */
	if (!bOnlyDB) {
		if (pToDeleteMsgIdList->nCount > 0) {
			/* memory free */
			if (pToDeleteMsgIdList != NULL) {
				/*free peer info list */
				if (pToDeleteMsgIdList->msgIdList != NULL)
					delete [] pToDeleteMsgIdList->msgIdList;

				delete [] pToDeleteMsgIdList;
			}

			MsgRefreshAllNotification(true, false, MSG_ACTIVE_NOTI_TYPE_NONE);
		}
	}

	return MSG_SUCCESS;

FREE_MEMORY:
	MSG_DEBUG("Error case Free Memory");

	while (!threadList.empty()) {
		threadList.front();
		threadList.pop();
	}

	/* memory free */
	if (pToDeleteMsgIdList != NULL) {
		/*free peer info list */
		if (pToDeleteMsgIdList->msgIdList != NULL)
			delete [] pToDeleteMsgIdList->msgIdList;

		delete [] pToDeleteMsgIdList;
	}

	return err;
}


msg_error_t MsgStoDeleteMessageByList(msg_id_list_s *pMsgIdList)
{
	std::string q;

	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	if (pMsgIdList->nCount < 1) {
		MSG_DEBUG("pMsgIdList->nCount < 1");
		return err;
	}

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<msg_thread_id_t> threadList1, threadList2;

#ifdef FEATURE_SMS_CDMA
	const char *tableList[] = {MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MMS_PREVIEW_TABLE_NAME,
						MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME,
						MSGFW_REPORT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
						MSGFW_SMS_REPORT_TABLE_NAME, MSGFW_MMS_MULTIPART_TABLE_NAME,
						MSGFW_UNIQUENESS_INFO_TABLE_NAME};
#else
	const char *tableList[] = {MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MMS_PREVIEW_TABLE_NAME,
						MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME,
						MSGFW_REPORT_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
						MSGFW_SMS_REPORT_TABLE_NAME, MSGFW_MMS_MULTIPART_TABLE_NAME};
#endif

	int listCnt = sizeof(tableList)/sizeof(char *);
	int rowCnt = 0;
	MsgDbHandler *dbHandle = getDbHandle();

	dbHandle->beginTrans();
	/* reset msgid temp table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", MSGFW_TMP_MSGID_TABLE_NAME);
	MSG_DEBUG("[%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (?)", MSGFW_TMP_MSGID_TABLE_NAME);
	MSG_DEBUG("[%s]", sqlQuery);
	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	for (int i = 0; i < pMsgIdList->nCount; i++) {
		dbHandle->resetQuery();
		dbHandle->bindInt(pMsgIdList->msgIdList[i], 1);
		if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	dbHandle->finalizeQuery();
	dbHandle->endTrans(true);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE MSG_ID IN %s;", MSGFW_MESSAGE_TABLE_NAME, MSGFW_TMP_MSGID_TABLE_NAME);

	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	MSG_DEBUG("rowCnt [%d]", rowCnt);

	if (rowCnt < 1) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++) {
		MSG_DEBUG("thread ID : %d", dbHandle->getColumnToInt(i));
		threadList1.push((msg_thread_id_t)dbHandle->getColumnToInt(i));
		threadList2.push((msg_thread_id_t)dbHandle->getColumnToInt(i));
	}
	dbHandle->freeTable();

	/* delete report notification */
	char tempAddr[MAX_ADDRESS_VAL_LEN+1];
	while (!threadList1.empty()) {
		memset(tempAddr, 0x00, sizeof(tempAddr));
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.ADDRESS_VAL FROM %s A, %s B WHERE A.CONV_ID = B.CONV_ID AND A.CONV_ID = %d;"
				, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, threadList1.front());

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbHandle->stepQuery() != MSG_ERR_DB_ROW) {
			dbHandle->finalizeQuery();
			return MSG_ERR_DB_DONE;
		}

		snprintf(tempAddr, sizeof(tempAddr), "%s", dbHandle->columnText(0));

		MSG_SEC_DEBUG("Updated address = %s", tempAddr);

		MsgDeleteReportNotification(tempAddr);

		dbHandle->finalizeQuery();

		threadList1.pop();
	}


	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE STORAGE_ID = %d AND MSG_ID IN %s;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM, MSGFW_TMP_MSGID_TABLE_NAME);

	rowCnt = 0;
	err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		rowCnt = 0;
	}

	for (int i = 1; i <= rowCnt; i++) {
		err = MsgStoDeleteMessage(dbHandle->getColumnToInt(i), false);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoDeleteMessage() Error!!!");
		}
	}
	dbHandle->freeTable();

	dbHandle->beginTrans();
	for (int i = 0; i < listCnt; i++) {
		if ( !i ) {
			char filePath[MSG_FILEPATH_LEN_MAX] = {0, };
			char dirPath[MSG_FILEPATH_LEN_MAX] = {0, };

			rowCnt = 0;

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID IN %s;", MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_TMP_MSGID_TABLE_NAME);

			err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("Fail to getTable().");
				rowCnt = 0;
			}

			MSG_DEBUG("rowCnt %d", rowCnt);

			for (int i = 1; i <= rowCnt; i++) {
				dbHandle->getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);

				MSG_SEC_DEBUG("filePath [%s]", filePath);

				/* Delete raw file. */
				snprintf(dirPath, sizeof(dirPath), "%s.dir", filePath);

				if (remove(filePath) == -1)
					MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_SEC_DEBUG("Success to delete file [%s]", filePath);

				MsgRmRf(dirPath);

				rmdir(dirPath);
			}
			dbHandle->freeTable();

		} else if (i == 1) {
			char filePath[MSG_FILEPATH_LEN_MAX] = {0, };

			rowCnt = 0;

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT VALUE FROM %s WHERE (TYPE = %d OR TYPE = %d) AND MSG_ID IN %s;",
					MSGFW_MMS_PREVIEW_TABLE_NAME, MSG_MMS_ITEM_TYPE_IMG, MSG_MMS_ITEM_TYPE_VIDEO, MSGFW_TMP_MSGID_TABLE_NAME);

			err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("Fail to getTable().");
				rowCnt = 0;
			}

			MSG_DEBUG("rowCnt %d", rowCnt);

			for (int i = 1; i <= rowCnt; i++) {
				memset(filePath, 0x00, sizeof(filePath));
				dbHandle->getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);
				if (remove(filePath) == -1)
					MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
			}
			dbHandle->freeTable();
		} else if (!strcmp(tableList[i], MSGFW_MMS_MULTIPART_TABLE_NAME)) {
				/* MMS file path to delete. */
				char filePath[MSG_FILEPATH_LEN_MAX] = {0, };

				rowCnt = 0;

				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID IN %s;", MSGFW_MMS_MULTIPART_TABLE_NAME, MSGFW_TMP_MSGID_TABLE_NAME);

				err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

				if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
					MSG_DEBUG("Fail to getTable().");
					rowCnt = 0;
				}

				MSG_DEBUG("rowCnt %d", rowCnt);

				for (int i = 1; i <= rowCnt; i++) {
					memset(filePath, 0x00, sizeof(filePath));
					dbHandle->getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);
					if (filePath[0] != '\0') {
						if (remove(filePath) == -1)
							MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
						else
							MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
					}
				}
				dbHandle->freeTable();

				/* MMS thumbnail path to delete */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT THUMB_FILE_PATH FROM %s WHERE MSG_ID IN %s;", MSGFW_MMS_MULTIPART_TABLE_NAME, MSGFW_TMP_MSGID_TABLE_NAME);

				err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

				if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
					MSG_DEBUG("Fail to getTable().");
					rowCnt = 0;
				}

				MSG_DEBUG("rowCnt %d", rowCnt);

				for (int i = 1; i <= rowCnt; i++) {
					memset(filePath, 0x00, sizeof(filePath));
					dbHandle->getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);
					if (filePath[0] != '\0') {
						if (remove(filePath) == -1)
							MSG_SEC_DEBUG("Fail to delete file [%s]", filePath);
						else
							MSG_SEC_DEBUG("Success to delete file [%s]", filePath);
					}
				}

				dbHandle->freeTable();
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN %s;", tableList[i], MSGFW_TMP_MSGID_TABLE_NAME);

		/* Delete Message in specific folder from table */
		err = dbHandle->execQuery(sqlQuery);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Fail to execQuery().");
		}
	}

	/* Clear Conversation table */
	err = MsgStoClearConversationTable(dbHandle);
	if (err != MSG_SUCCESS) MSG_DEBUG("Fail to MsgStoClearConversationTable().");

	/* Update Address */
	while (!threadList2.empty()) {
		err = MsgStoUpdateConversation(dbHandle, threadList2.front());
		if (err != MSG_SUCCESS) MSG_DEBUG("Fail to MsgStoUpdateConversation().");
		threadList2.pop();
	}
	err = dbHandle->endTrans(true);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to endTrans(true).");
		dbHandle->endTrans(false);
		return err;
	}

	if (g_idle_add(resetNotification, NULL) == 0) {
		MSG_DEBUG("resetNotification() Error");
	}

	MSG_END();
	return MSG_SUCCESS;
}


msg_error_t MsgStoMoveMessageToFolder(msg_message_id_t msgId, msg_folder_id_t destFolderId)
{
	msg_error_t err = MSG_SUCCESS;
	MSG_MESSAGE_TYPE_S msgType;
	msg_thread_id_t convId = 0;

	MsgStoGetMsgType(msgId, &msgType);

	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgType.mainType == MSG_SMS_TYPE)
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, destFolderId, msgId);
	else
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, destFolderId, msgId);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	/* get conversation id */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID FROM %s WHERE MSG_ID = %d;",
							MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW)
		convId = dbHandle->columnInt(0);

	MSG_DEBUG("convId : %d",  convId);

	dbHandle->finalizeQuery();

	/* update conversation table */
	err = MsgStoUpdateConversation(dbHandle, convId);

	/* update notification */
	signed char dest_folder_id = (signed char)destFolderId;
	if (dest_folder_id != MSG_SPAMBOX_ID)
		MsgRefreshAllNotification(true, false, MSG_ACTIVE_NOTI_TYPE_NONE);

	return err;
}


msg_error_t MsgStoMoveMessageToStorage(msg_message_id_t msgId, msg_storage_id_t destStorageId)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();

	MSG_DEBUG("msgId : %d, destStorageId : %d", msgId, destStorageId);

	switch (destStorageId) {
	case MSG_STORAGE_SIM : /* Move message to sim card */
	case MSG_STORAGE_SIM2 : {
			MSG_MESSAGE_INFO_S msgInfo;
			SMS_SIM_ID_LIST_S simIdList;

			memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
			memset(&simIdList, 0x00, sizeof(SMS_SIM_ID_LIST_S));

			if ((err = MsgStoGetMessage(msgId, &msgInfo, NULL)) != MSG_SUCCESS)
				return err;

			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);
			if (plg == NULL) {
				MSG_DEBUG("SMS Plug-in is NULL");
				return MSG_ERR_NULL_POINTER;
			}

			if (destStorageId == MSG_STORAGE_SIM)
				msgInfo.sim_idx = 1;
			else if (destStorageId == MSG_STORAGE_SIM2)
				msgInfo.sim_idx = 2;

			if ((err = plg->saveSimMessage(&msgInfo, &simIdList)) != MSG_SUCCESS)
				return err;

			dbHandle->beginTrans();

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d, SIM_INDEX = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, MSG_STORAGE_SIM, msgInfo.sim_idx, msgId);
			MSG_DEBUG("SQL query=[%s]", sqlQuery);

			if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle->endTrans(false);
				return MSG_ERR_DB_EXEC;
			}

			for (unsigned int i = 0; i < simIdList.count; i++) {
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
						MSGFW_SIM_MSG_TABLE_NAME, msgInfo.sim_idx, simIdList.simId[i], msgId);
				MSG_DEBUG("SQL query=[%s]", sqlQuery);

				if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle->endTrans(false);
					return MSG_ERR_DB_EXEC;
				}
			}

			dbHandle->endTrans(true);
		}
	break;

	default: { /* Moving message to memory (when destination storage id is MSG_STORAGE_PHONE) */
#ifndef FEATURE_SMS_CDMA
			bool bSimMsg = false;
			int rowCnt = 0;

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT STORAGE_ID FROM %s WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, msgId);

			err = dbHandle->getTable(sqlQuery, &rowCnt, NULL);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("Fail to getTable().");
				dbHandle->freeTable();
				return err;
			}

			if (dbHandle->getColumnToInt(1) == MSG_STORAGE_SIM) {
				MSG_DEBUG("It is SIM Message");
				bSimMsg = true;
			}

			dbHandle->freeTable();

			if (bSimMsg == true) {
				msg_sim_id_t simMsgId;
				msg_sim_slot_id_t sim_idx;

				/* get sim message id */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));

				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_SLOT_ID, SIM_ID FROM %s WHERE MSG_ID = %d;",
						MSGFW_SIM_MSG_TABLE_NAME, msgId);

				int index = 0;

				err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

				if (err != MSG_SUCCESS) {
					MSG_DEBUG("Fail to getTable().");
					dbHandle->freeTable();
					return err;
				}

				/* Delete messages in sim card */
				MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);
				if (plg == NULL) {
					MSG_DEBUG("SMS Plug-in is NULL");
					dbHandle->freeTable();
					return MSG_ERR_NULL_POINTER;
				}

				for (int i = 0; i < rowCnt; i++) {
					sim_idx =  dbHandle->getColumnToInt(index++);
					simMsgId = dbHandle->getColumnToInt(index++);

					MSG_DEBUG("simMsgId is %d.", simMsgId);

					if ((err = plg->deleteSimMessage(sim_idx, simMsgId)) != MSG_SUCCESS) {
						dbHandle->freeTable();
						return err;
					}
				}

				dbHandle->freeTable();

				dbHandle->beginTrans();

				/* Delete Messages in SIM Msg table */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
						MSGFW_SIM_MSG_TABLE_NAME, msgId);
				MSG_DEBUG("SQL query=[%s]", sqlQuery);

				if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle->endTrans(false);
					return MSG_ERR_DB_EXEC;
				}

				/* Move storage id */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, destStorageId, msgId);
				MSG_DEBUG("SQL query=[%s]", sqlQuery);

				if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle->endTrans(false);
					return MSG_ERR_DB_EXEC;
				}

				dbHandle->endTrans(true);
			}
#endif
		}
		break;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoCountMessage(msg_folder_id_t folderId, MSG_COUNT_INFO_S *pCountInfo)
{
	if (pCountInfo == NULL) {
		MSG_DEBUG("pCountInfo is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	char sqlQuery[MAX_QUERY_LEN+1];
	MsgDbHandler *dbHandle = getDbHandle();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND READ_STATUS = 1 AND STORAGE_ID = %d "
			"UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d "
			"UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND STORAGE_ID = %d "
			"UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_STORAGE_PHONE,
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_STORAGE_PHONE,
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_SMS_TYPE, MSG_STORAGE_PHONE,
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_MMS_TYPE, MSG_STORAGE_PHONE);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nReadCnt = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nUnreadCnt = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nSms = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nMms = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoCountMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount)
{
	if (pMsgType == NULL) {
		MSG_DEBUG("pMsgType is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	*pMsgCount = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MsgDbHandler *dbHandle = getDbHandle();
	/* SMS */
	if ((pMsgType->mainType == MSG_SMS_TYPE) &&
			(pMsgType->subType == MSG_NORMAL_SMS || pMsgType->subType == MSG_STATUS_REPORT_SMS || pMsgType->subType == MSG_CONCAT_SIM_SMS)) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_NORMAL_SMS, MSG_STATUS_REPORT_SMS, MSG_CONCAT_SIM_SMS);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) &&
			(pMsgType->subType == MSG_WAP_SI_SMS || pMsgType->subType == MSG_WAP_SL_SMS)) {
		/* PUSH */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS);
	} else if ((pMsgType->mainType == MSG_MMS_TYPE) &&
			(pMsgType->subType == MSG_SENDREQ_MMS || pMsgType->subType == MSG_SENDCONF_MMS || pMsgType->subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgType->subType == MSG_RETRIEVE_MANUALCONF_MMS)) {
		/* MMS */
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SENDREQ_MMS, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);
	} else {
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		*pMsgCount = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMessage(msg_message_id_t msgId, MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, \
			SUB_TYPE, DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, \
			BACKUP, PRIORITY, MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, THUMB_PATH, SIM_INDEX \
			FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	MsgDbHandler *dbHandle = getDbHandle();

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pMsg->msgId = dbHandle->columnInt(0);
		pMsg->threadId = dbHandle->columnInt(1);
		pMsg->folderId = dbHandle->columnInt(2);
		pMsg->storageId = dbHandle->columnInt(3);
		pMsg->msgType.mainType = dbHandle->columnInt(4);
		pMsg->msgType.subType = dbHandle->columnInt(5);
		pMsg->displayTime = (time_t)dbHandle->columnInt(6);
		pMsg->dataSize = dbHandle->columnInt(7);
		pMsg->networkStatus = dbHandle->columnInt(8);
		pMsg->bRead = dbHandle->columnInt(9);
		pMsg->bProtected = dbHandle->columnInt(10);
		pMsg->bBackup = dbHandle->columnInt(11);
		pMsg->priority = dbHandle->columnInt(12);
		pMsg->direction = dbHandle->columnInt(13);

		strncpy(pMsg->subject, (char *)dbHandle->columnText(15), MAX_SUBJECT_LEN);

		/* Temp_File_Handling */
		if (pMsg->msgType.mainType == MSG_SMS_TYPE || pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
			if (pMsg->dataSize > MAX_MSG_TEXT_LEN) {
				char msgData[pMsg->dataSize+1];
				memset(msgData, 0x00, sizeof(msgData));

				strncpy(msgData, (char *)dbHandle->columnText(16), pMsg->dataSize);

				/* Save Message Data into File */
				char fileName[MSG_FILENAME_LEN_MAX+1];
				memset(fileName, 0x00, sizeof(fileName));

				if (MsgCreateFileName(fileName) == false) {
					dbHandle->finalizeQuery();
					return MSG_ERR_STORAGE_ERROR;
				}

				MSG_SEC_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsg->dataSize, fileName);

				if (MsgWriteIpcFile(fileName, msgData, pMsg->dataSize) == false) {
					dbHandle->finalizeQuery();
					return MSG_ERR_STORAGE_ERROR;
				}

				strncpy(pMsg->msgData, fileName, MAX_MSG_DATA_LEN);

				pMsg->bTextSms = false;
			} else {
				memset(pMsg->msgText, 0x00, sizeof(pMsg->msgText));
				strncpy(pMsg->msgText, (char *)dbHandle->columnText(16), pMsg->dataSize);

				/* For WAP PUSH SI Message */
				if (pMsg->msgType.subType == MSG_WAP_SI_SMS) {
					strncat(pMsg->msgText, "\n", MAX_MSG_TEXT_LEN-strlen(pMsg->msgText));
					strncat(pMsg->msgText, pMsg->subject, MAX_MSG_TEXT_LEN-strlen(pMsg->msgText));
					MSG_SEC_DEBUG("pMsg->msgText : [%s]", pMsg->msgText);
					pMsg->dataSize = sizeof(pMsg->msgText);
				}

				pMsg->bTextSms = true;
			}
		} else {
			if (dbHandle->columnText(16) != NULL)
				strncpy(pMsg->msgText, (char *)dbHandle->columnText(16), MAX_MSG_TEXT_LEN);
		}

		/* thumbnail path */
		if (dbHandle->columnText(17)!= NULL && ((char *)dbHandle->columnText(17))[0] != '\0') {
			strncpy(pMsg->thumbPath, (char *)dbHandle->columnText(17), MSG_FILEPATH_LEN_MAX);
			MSG_DEBUG("pMsg->thumbPath : [%s]", pMsg->thumbPath);
		}
		pMsg->sim_idx = dbHandle->columnInt(18);
	} else {
		dbHandle->finalizeQuery();
		MSG_DEBUG("%s", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();


	/* get address information. */
	MsgStoGetAddressByMsgId(dbHandle, pMsg->msgId, &pMsg->nAddressCnt, &pMsg->addressList);

	/* Get MMS body if it is MMS. */
	if ((pMsg->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS &&
			(pMsg->msgType.subType == MSG_RETRIEVE_MMS || pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS)) ||
			pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_SENDCONF_MMS || pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		msg_error_t err = MSG_SUCCESS;
		char *pDestMsg = NULL;
		int temp_size = pMsg->dataSize; /* save raw file size; */

		/* call mms plugin to get mms specific message data */
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsg->msgType.mainType);
		if (plg == NULL) {
			MSG_DEBUG("SMS Plug-in is NULL");
			return MSG_ERR_NULL_POINTER;
		}

		err =  plg->getMmsMessage(pMsg, pSendOptInfo, &pDestMsg);
		if (err != MSG_SUCCESS) {
			if (pDestMsg) {
				delete [] pDestMsg;
				pDestMsg = NULL;
			}
			return MSG_ERR_STORAGE_ERROR;
		}

		memset(pMsg->msgData, 0, MAX_MSG_DATA_LEN+1);

		/* Encode MMS specific data to MMS_MESSAGE_DATA_S */
		if (pMsg->dataSize > MAX_MSG_DATA_LEN) {
			/* Save Message Data into File */
			char tempFileName[MSG_FILENAME_LEN_MAX+1];
			memset(tempFileName, 0x00, sizeof(tempFileName));

			if (MsgCreateFileName(tempFileName) == false) {
				if(pDestMsg) {
					delete [] pDestMsg;
					pDestMsg = NULL;
				}
				return MSG_ERR_STORAGE_ERROR;
			}
			MSG_SEC_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsg->dataSize, tempFileName);

			if (MsgWriteIpcFile(tempFileName, pDestMsg, pMsg->dataSize) == false) {
				if(pDestMsg) {
					delete [] pDestMsg;
					pDestMsg = NULL;
				}
				return MSG_ERR_STORAGE_ERROR;
			}
			strncpy(pMsg->msgData, tempFileName, MAX_MSG_DATA_LEN);
			pMsg->bTextSms = false;
		} else {
			strncpy(pMsg->msgData, pDestMsg, pMsg->dataSize);
			pMsg->bTextSms = true;
		}

		pMsg->dataSize = temp_size; /*raw file size; */

		if (pDestMsg) {
			delete [] pDestMsg;
			pDestMsg = NULL;
		}
	}

	/* Get SMS Sending Options */
	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL)
		MsgStoGetSmsSendOpt(pMsg, pSendOptInfo);

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetFailedMessage(int **failed_msg_list, int *count)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];
	int rowCnt = 0, index = 0;
	MsgDbHandler *dbHandle = getDbHandle();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	/* Folder ID == 2 : sending failed message, Folder ID == 1 : retrieve failed message */
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE (NETWORK_STATUS = %d OR NETWORK_STATUS == %d) AND (FOLDER_ID = %d OR FOLDER_ID = %d);",
				MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SEND_PENDING, MSG_NETWORK_RETRIEVE_PENDING, MSG_OUTBOX_ID, MSG_INBOX_ID);
	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if(err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}
	*count = rowCnt;
	int *list = new int[rowCnt];

	for(int i = 0; i < rowCnt; ++i) {
		list[i] = dbHandle->getColumnToInt(index++);
	}
	*failed_msg_list = list;
	dbHandle->freeTable();

	MSG_END();
	return err;
}

msg_error_t MsgStoAddSyncMLMessage(MSG_MESSAGE_INFO_S *pMsgInfo, int extId, int pinCode)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int rowId = 0;
	msg_thread_id_t convId = 0;
	MsgDbHandler *dbHandle = getDbHandle();

	dbHandle->beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(dbHandle, pMsgInfo, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return err;
		}
	}

	/* Add Message Table */
	pMsgInfo->threadId = convId;
	rowId = MsgStoAddMessageTable(dbHandle, pMsgInfo);

	if (rowId <= 0) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
			MSGFW_SYNCML_MSG_TABLE_NAME, rowId, extId, pinCode);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);

	pMsgInfo->msgId = (msg_message_id_t)rowId;

	MsgInsertNotification(pMsgInfo);
	MsgChangePmState();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMsgType(msg_message_id_t msgId, MSG_MESSAGE_TYPE_S *pMsgType)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pMsgType->mainType = dbHandle->columnInt(0);
		pMsgType->subType = dbHandle->columnInt(1);
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetQuickPanelData(msg_quickpanel_type_t QPtype, MSG_MESSAGE_INFO_S *pMsg)
{
	msg_error_t	err = MSG_SUCCESS;

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (QPtype == MSG_QUICKPANEL_SMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_SMS_TYPE, MSG_NORMAL_SMS);
	} else if (QPtype == MSG_QUICKPANEL_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d) AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_MMS_TYPE, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);
	} else if (QPtype == MSG_QUICKPANEL_DELIVER_REP) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE SUB_TYPE IN (%d, %d) AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_STATUS_REPORT_SMS, MSG_DELIVERYIND_MMS);
	} else if (QPtype == MSG_QUICKPANEL_READ_REP) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_MMS_TYPE, MSG_READORGIND_MMS);
	} else if (QPtype == MSG_QUICKPANEL_VOICEMAIL) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MWI_VOICE_SMS);
	} else if (QPtype == MSG_QUICKPANEL_MMS_NOTI) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
				MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_MMS_TYPE, MSG_NOTIFICATIONIND_MMS);
	}

	/* Get Message ID */
	msg_message_id_t msgId;

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgId = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	/* Get Message Info */
	err = MsgStoGetMessage(msgId, pMsg, NULL);

	return err;
}


msg_error_t MsgStoDeleteThreadMessageList(msg_thread_id_t threadId, bool bIncludeProtect, msg_id_list_s *pMsgIdList)
{
	msg_error_t err = MSG_SUCCESS;

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	/*** Get msg id list **/
	int rowCnt = 0, index = 0;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

#ifdef MSG_NOTI_INTEGRATION
	if (bIncludeProtect) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE \
				CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE \
				CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0;",
				MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID, MSG_STORAGE_PHONE);
	}
#else
	if (bIncludeProtect) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE \
				CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE \
				CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d AND STORAGE_ID = %d AND PROTECTED = 0;",
				MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);
	}
#endif

	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_ERR("Fail to getTable().");
		dbHandle->freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle->freeTable();

		err = MsgStoClearConversationTable(dbHandle);
		if (err != MSG_SUCCESS) {
			MSG_ERR("Fail to MsgStoClearConversationTable().");
			return err;
		}

		err = MsgStoUpdateConversation(dbHandle, threadId);
		if (err != MSG_SUCCESS) {
			MSG_ERR("Fail to MsgStoUpdateConversation().");
			return err;
		}

		return MSG_SUCCESS;
	}

	pMsgIdList->nCount = rowCnt;

	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	pMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pMsgIdList->msgIdList[i] = dbHandle->getColumnToInt(index++);

	dbHandle->freeTable();
	/*** **/

	err = MsgStoDeleteMessageByList(pMsgIdList);
	if (err != MSG_SUCCESS) {
		MSG_ERR("Fail to MsgStoDeleteMessageByList().");
		return err;
	}

	return err;
}


msg_error_t MsgStoSetTempAddressTable(MSG_ADDRESS_INFO_S *pAddrInfo, int addr_cnt)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle->beginTrans();
	/* reset address temp table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s;", MSGFW_ADDRESS_TEMP_TABLE_NAME);
	MSG_DEBUG("[%s]", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle->finalizeQuery();
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle->finalizeQuery();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (?);", MSGFW_ADDRESS_TEMP_TABLE_NAME);
	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	char newPhoneNum[MAX_ADDRESS_VAL_LEN+1];
	char tmpNum[MAX_ADDRESS_VAL_LEN+1];
	for (int i = 0; i < addr_cnt; i++) {
		memset(newPhoneNum, 0x00, sizeof(newPhoneNum));
		memset(tmpNum, 0x00, sizeof(tmpNum));
		MsgConvertNumber(pAddrInfo[i].addressVal, tmpNum, sizeof(tmpNum));
		snprintf(newPhoneNum, sizeof(newPhoneNum), "%%%%%s", tmpNum);
		dbHandle->resetQuery();
		dbHandle->bindText(newPhoneNum, 1);
		if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	dbHandle->finalizeQuery();
	dbHandle->endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoCountMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pThreadCountInfo)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pAddrInfo->contactId > 0) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
				SUM(CASE WHEN READ_STATUS = 0 AND FOLDER_ID = %d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE = %d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE = %d THEN 1 ELSE 0 END) \
				FROM (SELECT * FROM %s A  JOIN %s B ON A.CONV_ID = B.CONV_ID WHERE B.CONTACT_ID = %d)",
				MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->contactId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
				SUM(CASE WHEN READ_STATUS = 0 AND FOLDER_ID = %d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE = %d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE = %d THEN 1 ELSE 0 END) \
				FROM (SELECT * FROM %s A JOIN %s B ON A.CONV_ID = B.CONV_ID WHERE B.ADDRESS_VAL = '%s')",
				MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->msgAddrInfo.addressVal);
	}

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		pThreadCountInfo->totalCount = dbHandle->columnInt(0);
		pThreadCountInfo->unReadCount = dbHandle->columnInt(1);
		pThreadCountInfo->smsMsgCount = dbHandle->columnInt(2);
		pThreadCountInfo->mmsMsgCount = dbHandle->columnInt(3);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetSyncMLExtId(msg_message_id_t msgId, int *extId)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXT_ID FROM %s WHERE MSG_ID = %d;",
			MSGFW_SYNCML_MSG_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		*extId = dbHandle->columnInt(0);
	} else {
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetSmsReportStatus(msg_message_id_t msgId, int *count, MSG_REPORT_STATUS_INFO_S **pReportStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];
	msg_error_t err = MSG_SUCCESS;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT ADDRESS_VAL, STATUS_TYPE, STATUS, TIME "
			"FROM %s "
			"WHERE MSG_ID = %d "
			"order by TIME ASC;"
			, MSGFW_SMS_REPORT_TABLE_NAME, msgId);

	int rowCnt = 0, index = 0;
	err = dbHandle->getTable(sqlQuery, &rowCnt, &index);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		dbHandle->freeTable();
		if (err == MSG_ERR_DB_NORECORD)
			return MSG_SUCCESS;
		return err;
	}

	*count =  rowCnt;
	MSG_REPORT_STATUS_INFO_S *report_status = (MSG_REPORT_STATUS_INFO_S*)new char[sizeof(MSG_REPORT_STATUS_INFO_S)*rowCnt];
	memset(report_status, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S)*rowCnt);

	for (int i = 0; i < rowCnt; i++) {
		dbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, report_status[i].addressVal);
		report_status[i].type = dbHandle->getColumnToInt(index++);
		report_status[i].status = dbHandle->getColumnToInt(index++);
		report_status[i].statusTime = (time_t)dbHandle->getColumnToInt(index++);

		MSG_SEC_DEBUG("(%d/%d) address = %s, report_type = %d, report_status = %d, report_time = %d", i+1, rowCnt, report_status[i].addressVal, report_status[i].type, report_status[i].status, report_status[i].statusTime);
	}

	*pReportStatus = report_status;

	dbHandle->freeTable();

	return MSG_SUCCESS;
}

msg_error_t MsgStoGetMmsReportStatus(msg_message_id_t msgId, int *count, MSG_REPORT_STATUS_INFO_S **pReportStatus)
{
	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_direction_type_t	direction = MSG_DIRECTION_TYPE_MO;

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_DIRECTION FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		direction = dbHandle->columnInt(0);
	}

	dbHandle->finalizeQuery();

	if (direction == MSG_DIRECTION_TYPE_MO) {/*get received report list of MO message */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		/*MSG_ID INTEGER , ADDRESS_VAL TEXT , STATUS_TYPE INTEGER , STATUS INTEGER DEFAULT 0 , TIME DATETIME) */
		/*select * from MSG_REPORT_TABLE where MSG_ID=38 order by ADDRESS_VAL DESC, STATUS_TYPE ASC; */
		snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT ADDRESS_VAL, STATUS_TYPE, STATUS, TIME "
				"FROM %s "
				"WHERE MSG_ID = %d "
				"order by ADDRESS_VAL DESC, STATUS_TYPE ASC;"
				, MSGFW_REPORT_TABLE_NAME, msgId);

		int rowCnt = 0, index = 0;
		msg_error_t  err = dbHandle->getTable(sqlQuery, &rowCnt, &index);
		if (err != MSG_SUCCESS) {
			MSG_DEBUG("Fail to getTable().");
			dbHandle->freeTable();
			if (err == MSG_ERR_DB_NORECORD)
				return MSG_SUCCESS;
			return err;
		}

		*count = rowCnt;
		MSG_REPORT_STATUS_INFO_S *report_status = (MSG_REPORT_STATUS_INFO_S*)new char[sizeof(MSG_REPORT_STATUS_INFO_S)*rowCnt];
		memset(report_status, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S)*rowCnt);

		for (int i = 0; i < rowCnt; i++) {
			dbHandle->getColumnToString(index++, MAX_ADDRESS_VAL_LEN, report_status[i].addressVal);
			report_status[i].type = dbHandle->getColumnToInt(index++);
			report_status[i].status = dbHandle->getColumnToInt(index++);
			report_status[i].statusTime = (time_t)dbHandle->getColumnToInt(index++);

			MSG_DEBUG("(%d/%d) addr = %s, report_type = %d, report_status = %d, report_time = %d", i+1, rowCnt, report_status[i].addressVal, report_status[i].type, report_status[i].status, report_status[i].statusTime);
		}

		*pReportStatus = report_status;

		dbHandle->freeTable();

	} else if (direction == MSG_DIRECTION_TYPE_MT) { /*read report sent status of MT message */
		int readReportSentStatus;

		if (MsgStoCheckReadReportRequested(dbHandle, msgId) == false) {
			*count = 0;
			*pReportStatus = NULL;
			return MSG_ERR_READREPORT_NOT_REQUESTED;
		}

		MsgStoGetReadReportSendStatus(msgId, &readReportSentStatus);

		*count =  1;

		MSG_REPORT_STATUS_INFO_S *report_status = (MSG_REPORT_STATUS_INFO_S*)new char[sizeof(MSG_REPORT_STATUS_INFO_S)];
		memset(report_status, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S));

		report_status->addressVal[0] = '\0';
		report_status->type = MSG_REPORT_TYPE_READ_REPORT_SENT;
		report_status->status = readReportSentStatus;
		report_status->statusTime = 0;

		MSG_DEBUG("report_type = %d, report_status = %d", report_status->type, report_status->status);

		*pReportStatus = report_status;
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetThreadIdByAddress(const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pThreadId)
{
	MsgDbHandler *dbHandle = getDbHandle();
	if(pMsg->nAddressCnt > 0) {
		if (MsgExistAddress(dbHandle, pMsg, pThreadId) == true) {
			MSG_DEBUG("Conversation ID : [%d]", *pThreadId);

			/* check thread count */
			MSG_THREAD_VIEW_S threadInfo;
			memset(&threadInfo, 0x00, sizeof(MSG_THREAD_VIEW_S));
			MsgStoGetThreadInfo(*pThreadId, &threadInfo);
			MSG_DEBUG("threadInfo.smsCnt [%d], threadInfo.mmsCnt [%d]", threadInfo.smsCnt, threadInfo.mmsCnt);
			if ((threadInfo.smsCnt + threadInfo.mmsCnt) > 0) {
				return MSG_SUCCESS;
			}
		}
	}

	*pThreadId = 0;
	return MSG_SUCCESS;
}


msg_error_t MsgStoGetThreadUnreadCnt(msg_thread_id_t threadId, int *cnt)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	int msgCnt = 0;
	*cnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	/* Get MSG_ID */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s A \
			WHERE CONV_ID = %d AND READ_STATUS = 0;", MSGFW_MESSAGE_TABLE_NAME, threadId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgCnt = dbHandle->columnInt(0);
	}

	dbHandle->finalizeQuery();

	*cnt = msgCnt;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetThreadInfo(msg_thread_id_t threadId, MSG_THREAD_VIEW_S *pThreadInfo)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	int rowCnt = 0, index = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.CONV_ID, A.UNREAD_CNT, A.SMS_CNT, A.MMS_CNT, "
			"A.MAIN_TYPE, A.SUB_TYPE, A.MSG_DIRECTION, A.DISPLAY_TIME, A.DISPLAY_NAME, A.MSG_TEXT, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.PROTECTED = 1) AS PROTECTED, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.FOLDER_ID = %d) AS DRAFT, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.NETWORK_STATUS = %d) AS FAILED, "
			"(SELECT COUNT(*) FROM %s B WHERE B.CONV_ID = A.CONV_ID AND B.NETWORK_STATUS = %d) AS SENDING "
			"FROM %s A WHERE A.CONV_ID = %d AND A.SMS_CNT + A.MMS_CNT > 0;",
			MSGFW_MESSAGE_TABLE_NAME,
			MSGFW_MESSAGE_TABLE_NAME, MSG_DRAFT_ID,
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SEND_FAIL,
			MSGFW_MESSAGE_TABLE_NAME, MSG_NETWORK_SENDING,
			MSGFW_CONVERSATION_TABLE_NAME, threadId);

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
	} else {
		pThreadInfo->threadId = dbHandle->getColumnToInt(index++);

		pThreadInfo->unreadCnt = dbHandle->getColumnToInt(index++);
		pThreadInfo->smsCnt = dbHandle->getColumnToInt(index++);
		pThreadInfo->mmsCnt = dbHandle->getColumnToInt(index++);

		pThreadInfo->mainType = dbHandle->getColumnToInt(index++);
		pThreadInfo->subType = dbHandle->getColumnToInt(index++);

		pThreadInfo->direction = dbHandle->getColumnToInt(index++);
		pThreadInfo->threadTime = (time_t)dbHandle->getColumnToInt(index++);

		memset(pThreadInfo->threadName, 0x00, sizeof(pThreadInfo->threadName));
		dbHandle->getColumnToString(index++, MAX_THREAD_NAME_LEN, pThreadInfo->threadName);

		memset(pThreadInfo->threadData, 0x00, sizeof(pThreadInfo->threadData));
		dbHandle->getColumnToString(index++, MAX_THREAD_DATA_LEN, pThreadInfo->threadData);

		int protectedCnt = dbHandle->getColumnToInt(index++);
		if (protectedCnt > 0)
			pThreadInfo->bProtected = true;

		int draftCnt = dbHandle->getColumnToInt(index++);
		if (draftCnt > 0)
			pThreadInfo->bDraft = true;

		int failedCnt = dbHandle->getColumnToInt(index++);
		if (failedCnt > 0)
			pThreadInfo->bSendFailed = true;

		int sendingCnt = dbHandle->getColumnToInt(index++);
		if (sendingCnt > 0)
			pThreadInfo->bSending = true;
	}

	dbHandle->freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoRestoreMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;
	char sqlQuery[MAX_QUERY_LEN+1];

	if(MsgExistConversation(dbHandle, pMsg->threadId)) {
		/* add message to address table  which having same thread_id and datetime; */
		for (int i = 0; i < pMsg->nAddressCnt; i++) {
			if(MsgExistAddress(dbHandle, pMsg, pMsg->threadId, i) == false) {
				unsigned int addrId;
				MSG_CONTACT_INFO_S contactInfo;
				memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

				/* Get Contact Info */
/*				if (MsgGetContactInfo(&(pMsg->addressList[i]), &contactInfo) != MSG_SUCCESS) { */
/*					MSG_DEBUG("MsgGetContactInfo() fail."); */
/*				} */
				err = dbHandle->getRowId(MSGFW_ADDRESS_TABLE_NAME, &addrId);
				if (err != MSG_SUCCESS) {
					MSG_DEBUG("pDbHandle->getRowId fail. [%d]", err);
					return err;
				}

				/* Add Address */
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, '%s', %d, %d, ?, ?, ?, ?, ?, '%s', 0);",
							MSGFW_ADDRESS_TABLE_NAME, addrId, pMsg->threadId, pMsg->addressList[i].addressType, pMsg->addressList[i].recipientType, pMsg->addressList[i].addressVal,
							contactInfo.contactId, contactInfo.addrbookId, contactInfo.imagePath);

				MSG_SEC_DEBUG("Add Address Info. [%s]", sqlQuery);

				if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
					return MSG_ERR_DB_PREPARE;

				dbHandle->bindText(contactInfo.firstName, 1);
				dbHandle->bindText(contactInfo.lastName, 2);
				dbHandle->bindText(contactInfo.middleName, 3);
				dbHandle->bindText(contactInfo.prefix, 4);
				dbHandle->bindText(contactInfo.suffix, 5);

				if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
					dbHandle->finalizeQuery();
					return MSG_ERR_DB_STEP;
				}

				dbHandle->finalizeQuery();

				/* set conversation display name by conv id */
				MsgStoSetConversationDisplayName(dbHandle, pMsg->threadId);
			}
		}


		if(!MsgExistMessage(dbHandle, pMsg)) {
			unsigned int rowId = 0;

			if (pMsg->threadId > 0 && pMsg->folderId == MSG_DRAFT_ID) {
				memset(sqlQuery, 0x00, sizeof(sqlQuery));

				snprintf(sqlQuery, sizeof(sqlQuery),
						"DELETE FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, pMsg->threadId, MSG_DRAFT_ID);

				MSG_DEBUG("sqlQuery [%s]", sqlQuery);

				err = dbHandle->execQuery(sqlQuery);

				if (err != MSG_SUCCESS) {
					MSG_DEBUG("fail to delete draft messages.");
				}
			}

			err = dbHandle->getRowId(MSGFW_MESSAGE_TABLE_NAME, &rowId);

			pMsg->msgId = (msg_message_id_t)rowId;

			int fileSize = 0;

			char *pFileData = NULL;
			unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

			/* Get File Data */
			if (pMsg->bTextSms == false) {
				if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
					return MSG_ERR_STORAGE_ERROR;
				}
				MSG_DEBUG("file size [%d]", fileSize);
			}

			char keyName[MAX_VCONFKEY_NAME_LEN];
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SUBS_ID, pMsg->sim_idx);

			char *imsi = NULL;
			if (MsgSettingGetString(keyName, &imsi) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}

			/* Add Message */
			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, 0, %d, ?, '', '', ?, 0, %d, '%s');",
					MSGFW_MESSAGE_TABLE_NAME, rowId, pMsg->threadId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType,
					pMsg->displayTime, pMsg->dataSize, pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction,
					pMsg->bBackup, pMsg->sim_idx, imsi);


			MSG_DEBUG("QUERY : %s", sqlQuery);

			if (imsi) {
				free(imsi);
				imsi = NULL;
			}

			if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
				return MSG_ERR_DB_EXEC;
			}

			dbHandle->bindText(pMsg->subject, 1);

			if (pMsg->bTextSms == false)
				dbHandle->bindText(pFileData, 2);
			else
				dbHandle->bindText(pMsg->msgText, 2);

			if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
				dbHandle->finalizeQuery();
				return MSG_ERR_DB_EXEC;
			}

			dbHandle->finalizeQuery();

			if (pMsg->msgType.subType != MSG_SENDREQ_MMS) {
				err = MsgStoUpdateConversation(dbHandle, pMsg->threadId);

				if (err != MSG_SUCCESS) {
					return err;
				}
			}

			/* In the case of MMS Message, load the MMS plugin to save MMS PDU */
			if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
				if (pMsg->msgType.subType != MSG_DELIVERYIND_MMS && pMsg->msgType.subType != MSG_READORGIND_MMS) {
					MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);
					if (plg == NULL)
						return MSG_ERR_NULL_POINTER;

					err = plg->addMessage(pMsg, pSendOptInfo, pFileData);

					if (err != MSG_SUCCESS)
						return MSG_ERR_STORAGE_ERROR;

					if (pMsg->msgType.subType == MSG_SENDREQ_MMS) {
						MSG_SEC_DEBUG("pMsg->msgText: %s, pMsg->thumbPath: %s ", pMsg->msgText, pMsg->thumbPath);

						err = MsgStoUpdateMMSMessage(pMsg);

						if (err != MSG_SUCCESS)
							return MSG_ERR_STORAGE_ERROR;
					}
				}
			}
		}
	} else {
		/* add message to conversation, message, address table; */
		pMsg->msgId = 0;
		err = MsgStoAddMessage(pMsg, pSendOptInfo);
	}

	MSG_END();

	return err;
}


msg_message_id_t MsgStoAddSimMessage(MSG_MESSAGE_INFO_S *pMsg, int *simIdList, int listSize)
{
	MSG_BEGIN();
	MsgDbHandler *dbHandle = getDbHandle();
	if (pMsg == NULL || pMsg->msgType.mainType == MSG_MMS_TYPE) {
		MSG_DEBUG("pMsg == NULL || pMsg->msgType.mainType == MSG_MMS_TYPE");
		return 0;
	}

	if ((pMsg->msgType.subType >= MSG_REPLACE_TYPE1_SMS) && (pMsg->msgType.subType <= MSG_REPLACE_TYPE7_SMS)) {
		if (pMsg->msgId > 0) {
			pMsg->bRead = false;
			if (MsgStoUpdateMessage(pMsg, NULL) != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoUpdateMessage is failed!!!");
			}
		} else {
			if (MsgStoAddMessage(pMsg, NULL) != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoAddMessage is failed!!!");
			}
		}
	} else {
		pMsg->msgId = 0;
		if (MsgStoAddMessage(pMsg, NULL) != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddMessage is failed!!!");
		}
	}

	if(simIdList) {
		dbHandle->beginTrans();

		MSG_DEBUG("simIdList exist.");
		int simId = 0;
		for(int i = 0; i < listSize; ++i) {
			if(simIdList[i]) {
				simId = simIdList[i] - 1;
				char sqlQuery[MAX_QUERY_LEN+1];
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
						MSGFW_SIM_MSG_TABLE_NAME, pMsg->sim_idx, simId, pMsg->msgId);

				MSG_DEBUG("QUERY : %s", sqlQuery);

				if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle->endTrans(false);
					return 0;
				}
			} else {
				break;
			}
		}

		dbHandle->endTrans(true);
	}

	MSG_END();

	return pMsg->msgId;
}

#ifdef FEATURE_SMS_CDMA
msg_error_t MsgCheckUniqueness(bool bInsert, msg_message_id_t msgId, MSG_UNIQUE_INDEX_S *p_msg)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	int nRowCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	if (!bInsert) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		dbHandle->beginTrans();

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE TELE_MSG_ID = %d AND ADDRESS = '%s' AND SUB_ADDRESS = '%s' AND TIME_STAMP = '%s' AND TELESVC_ID = %d;",
					MSGFW_UNIQUENESS_INFO_TABLE_NAME, p_msg->tele_msgId, p_msg->address, p_msg->sub_address, p_msg->time_stamp, p_msg->telesvc_id);

		dbHandle->getTable(sqlQuery, &nRowCnt, NULL);
		MSG_DEBUG("nRowCnt = [%d]", nRowCnt);

		dbHandle->freeTable();
		dbHandle->endTrans(true);

		if (nRowCnt == 0) {
			MSG_DEBUG("<<<<This incoming message is a new message>>>>");
			return MSG_SUCCESS;
		}
		else {
			MSG_DEBUG("<<<<This incoming message is a repeated message>>>>");
			return MSG_ERR_UNKNOWN;
		}
	}
	else {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		dbHandle->beginTrans();

		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, '%s', '%s', '%s', %d);",
					MSGFW_UNIQUENESS_INFO_TABLE_NAME, msgId, p_msg->tele_msgId, p_msg->address, p_msg->sub_address, p_msg->time_stamp, p_msg->telesvc_id);

		dbHandle->execQuery(sqlQuery);
		dbHandle->endTrans(true);
	}

	MSG_END();

	return MSG_SUCCESS;
}
#endif

msg_error_t MsgStoUpdateIMSI(int sim_idx)
{
	MSG_BEGIN();
	MSG_DEBUG("sim_idx = %d", sim_idx);

/*	msg_error_t err = MSG_SUCCESS; */
	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN+1];
	char keyName[MAX_VCONFKEY_NAME_LEN];

	dbHandle->beginTrans();

	if (sim_idx == 0) {
		MSG_DEBUG("sim index is 0");
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET SIM_INDEX = 0;",
				MSGFW_MESSAGE_TABLE_NAME);

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	} else {
		MSG_DEBUG("sim index is %d", sim_idx);

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SUBS_ID, sim_idx);

		char *imsi = NULL;
		if (MsgSettingGetString(keyName, &imsi) != MSG_SUCCESS) {
			MSG_INFO("MsgSettingGetString() is failed");
		}

		MSG_SEC_DEBUG("imsi value exist -> %s", imsi);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET SIM_INDEX = %d \
				WHERE SIM_IMSI LIKE '%s';",
				MSGFW_MESSAGE_TABLE_NAME, sim_idx, imsi);

		if (imsi) {
			free(imsi);
			imsi = NULL;
		}

		if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (dbHandle->stepQuery() != MSG_ERR_DB_DONE) {
			dbHandle->finalizeQuery();
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	dbHandle->finalizeQuery();
	dbHandle->endTrans(true);

	MSG_END();

	return MSG_SUCCESS;
}
