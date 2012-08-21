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
#include "MsgSoundPlayer.h"
#include "MsgGconfWrapper.h"
#include "MsgSqliteWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"
#include "MsgNotificationWrapper.h"
#include "MsgMmsMessage.h"

using namespace std;


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;

Mutex delNotiMx;
CndVar delNoticv;
bool delNotiRunning = false;

Mutex delLogMx;
CndVar delLogcv;
bool delLogRunning = false;


/*==================================================================================================
                                     FUNCTION FOR THREAD
==================================================================================================*/
static gboolean startToDeleteNoti(void *pVoid)
{
	MSG_BEGIN();

	msg_id_list_s *pMsgIdList = (msg_id_list_s *)pVoid;

	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	delNotiMx.lock();

	while (delNotiRunning) {
		delNoticv.wait(delNotiMx.pMutex());
	}

	delNotiRunning = true;

	for (int i = 0; i < pMsgIdList->nCount; i++) {
		MsgDeleteNotiByMsgId(pMsgIdList->msgIdList[i]);

		/** sleep for moment */
		if ((i%100 == 0) && (i != 0))
			usleep(70000);
	}

	delNotiRunning = false;

	delNoticv.signal();
	delNotiMx.unlock();

	// memory free
	if (pMsgIdList != NULL) {
		//free peer info list
		if (pMsgIdList->msgIdList != NULL)
			delete [] pMsgIdList->msgIdList;

		delete [] pMsgIdList;
	}

	MSG_END();

	return FALSE;
}


static gboolean startToDeletePhoneLog(void *pVoid)
{
	MSG_BEGIN();

	msg_id_list_s *pMsgIdList = (msg_id_list_s *)pVoid;

	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	delLogMx.lock();

	while (delLogRunning){
		delLogcv.wait(delLogMx.pMutex());
	}

	delLogRunning = true;

	int transBegin = MsgContactSVCBeginTrans();

	for (int i = 0; i < pMsgIdList->nCount; i++) {
		if (transBegin == 0)
			MsgDeletePhoneLog(pMsgIdList->msgIdList[i]);

		/** sleep for moment */
		if ((i%100 == 0) && (i!=0)) {
			if (transBegin == 0) {
				MsgContactSVCEndTrans(true);
				usleep(70000);
			}
			transBegin = MsgContactSVCBeginTrans();
		}
	}

	if (transBegin == 0)
		MsgContactSVCEndTrans(true);

	delLogRunning = false;

	delLogcv.signal();
	delLogMx.unlock();

	// memory free
	if (pMsgIdList != NULL) {
		//free peer info list
		if (pMsgIdList->msgIdList != NULL)
			delete [] pMsgIdList->msgIdList;

		delete [] pMsgIdList;
	}

	MSG_END();

	return FALSE;
}

static gboolean updateUnreadMsgCount(void *pVoid)
{
	MSG_BEGIN();

	int smsCnt = 0;
	int mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

	MsgSettingSetIndicator(smsCnt, mmsCnt);

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

	dbHandle.beginTrans();

	if (pMsg->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, pMsg, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}

		pMsg->threadId = convId;
	}


	///////// temporary code for draft message in conversation view.
	if(convId > 0 && pMsg->folderId == MSG_DRAFT_ID) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery),
				"DELETE FROM %s WHERE CONV_ID = %d AND FOLDER_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, convId, MSG_DRAFT_ID);

		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		err = dbHandle.execQuery(sqlQuery);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("fail to delete draft messages.");
		}
	}
	////////

	err = dbHandle.getRowId(MSGFW_MESSAGE_TABLE_NAME, &rowId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	pMsg->msgId = (msg_message_id_t)rowId;

	int fileSize = 0;

	char *pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	// Get File Data
	if (pMsg->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
			dbHandle.endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
		MSG_DEBUG("file size [%d]", fileSize);
	}

	// Add Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %ld, %d, ?, ?, ?, ?, %d, 0, %d, 0, 0);",
			MSGFW_MESSAGE_TABLE_NAME, rowId, convId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType,
			pMsg->displayTime, pMsg->dataSize, pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction,
			0, pMsg->bBackup, MSG_DELIVERY_REPORT_NONE, MSG_READ_REPORT_NONE);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.bindText(pMsg->subject, 1);

	dbHandle.bindText(pMsg->msgData, 2);

	dbHandle.bindText(pMsg->thumbPath, 3);

	if (pMsg->bTextSms == false)
		dbHandle.bindText(pFileData, 4);
	else
		dbHandle.bindText(pMsg->msgText, 4);

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.finalizeQuery();
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

	if (pMsg->msgType.subType != MSG_SENDREQ_MMS) {
		err = MsgStoUpdateConversation(&dbHandle, convId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	dbHandle.endTrans(true);

	/* In the case of MMS Message, load the MMS plugin to save MMS PDU */
	if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		MMS_MESSAGE_DATA_S mmsMsg;
		memset(&mmsMsg, 0x00, sizeof(MMS_MESSAGE_DATA_S));

		if (pMsg->dataSize == 0) {
			MSG_DEBUG("pMsg->dataSize == 0, So Making emtpy MMS body.");
			char * tempMmsBody = _MsgMmsSerializeMessageData(&mmsMsg, &(pMsg->dataSize));
			memcpy(&pMsg->msgText, tempMmsBody, pMsg->dataSize);
			free(tempMmsBody);
		}

		if (pMsg->msgType.subType != MSG_DELIVERYIND_MMS && pMsg->msgType.subType != MSG_READORGIND_MMS) {
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);

			//pMsg->msgId = pMsg->refernceId;

			err = plg->addMessage(pMsg, pSendOptInfo, pFileData);

			if (err != MSG_SUCCESS)
				return MSG_ERR_STORAGE_ERROR;

			if (pMsg->msgType.subType == MSG_SENDREQ_MMS) {
				MSG_DEBUG("pMsg->msgText: %s, pMsg->thumbPath: %s ", pMsg->msgText, pMsg->thumbPath);

				err = MsgStoUpdateMMSMessage(pMsg);

				if (err != MSG_SUCCESS)
					return MSG_ERR_STORAGE_ERROR;

			}
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	msg_thread_id_t convId = 0;

	dbHandle.beginTrans();

	if (pMsg->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, pMsg, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	int fileSize = 0;

	char *pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	// Get File Data
	if (pMsg->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
			dbHandle.endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}

	if (pSendOptInfo != NULL) {
		// Get Global setting value if bSetting == false
		if (pSendOptInfo->bSetting == false) {
			MsgSettingGetBool(MSG_KEEP_COPY, &pSendOptInfo->bKeepCopy);

			if (pMsg->msgType.mainType == MSG_SMS_TYPE) {
				MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &pSendOptInfo->bDeliverReq);
				MsgSettingGetBool(SMS_SEND_REPLY_PATH, &pSendOptInfo->option.smsSendOptInfo.bReplyPath);
			} else if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
				MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &pSendOptInfo->bDeliverReq);
				MsgSettingGetBool(MMS_SEND_READ_REPLY, &pSendOptInfo->option.mmsSendOptInfo.bReadReq);
				pSendOptInfo->option.mmsSendOptInfo.expiryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);

				MSG_MMS_DELIVERY_TIME_T deliveryTime = (MSG_MMS_DELIVERY_TIME_T)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);

				if (deliveryTime == MSG_DELIVERY_TIME_CUSTOM) {
					pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime = true;
					pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY);
				} else {
					pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime = false;
					pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time = (unsigned int)deliveryTime;
				}

				pSendOptInfo->option.mmsSendOptInfo.priority = (msg_priority_type_t)MsgSettingGetInt(MMS_SEND_PRIORITY);
			}
		}
	}

	// Update Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET CONV_ID = %d, FOLDER_ID = %d, STORAGE_ID = %d, MAIN_TYPE = %d, SUB_TYPE = %d, \
			DISPLAY_TIME = %lu, DATA_SIZE = %d, NETWORK_STATUS = %d, READ_STATUS = %d, PROTECTED = %d, PRIORITY = %d, MSG_DIRECTION = %d, \
			BACKUP = %d, SUBJECT = ?, MSG_DATA = ?, THUMB_PATH = ?, MSG_TEXT = ? \
			WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, convId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->displayTime, pMsg->dataSize,
			pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction, pMsg->bBackup, pMsg->msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.bindText(pMsg->subject, 1);

	dbHandle.bindText(pMsg->msgData, 2);

	dbHandle.bindText(pMsg->thumbPath, 3);

	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->bTextSms == false)
		dbHandle.bindText(pFileData, 4);
	else
		dbHandle.bindText(pMsg->msgText, 4);

	MSG_DEBUG("%s", sqlQuery);

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.finalizeQuery();
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL) {
		if (pSendOptInfo->bSetting == true) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET \
					DELREP_REQ = %d, KEEP_COPY = %d, REPLY_PATH = %d \
					WHERE MSG_ID = %d;",
					MSGFW_SMS_SENDOPT_TABLE_NAME, pSendOptInfo->bDeliverReq,
					pSendOptInfo->bKeepCopy, pSendOptInfo->option.smsSendOptInfo.bReplyPath, pMsg->msgId);

			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_MMS_TYPE);

		err = plg->updateMessage(pMsg, pSendOptInfo, pFileData);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}

	err = MsgStoUpdateConversation(&dbHandle, convId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	err = MsgStoClearConversationTable(&dbHandle);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateReadStatus(msg_message_id_t msgId, bool bRead)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_storage_id_t storageId;

	if (MsgStoSetReadStatus(&dbHandle, msgId, bRead) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoSetReadStatus() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	// Get STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT STORAGE_ID FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		storageId = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	MSG_DEBUG("StorageId:[%d]", storageId);

	// Update Read Status for SIM Msg
	if (storageId == MSG_STORAGE_SIM) {
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

		if (plg == NULL) {
			MSG_DEBUG("SMS Plug-in is NULL");
			return MSG_ERR_NULL_POINTER;
		}

		// Get SIM Msg ID
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_ID FROM %s WHERE MSG_ID = %d;",
				MSGFW_SIM_MSG_TABLE_NAME, msgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		msg_sim_id_t simId;

		while (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
			simId = dbHandle.columnInt(0);

			if (plg->setReadStatus(simId) != MSG_SUCCESS) {
				MSG_DEBUG("Fail to Set Read Status for SIM SMS");
				continue;
			}
		}

		dbHandle.finalizeQuery();
	}

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateThreadReadStatus(msg_thread_id_t threadId)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	int rowCnt = 0;
	int index = 1;
	msg_id_list_s *pUnreadMsgIdList = NULL;

	pUnreadMsgIdList = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
	memset(pUnreadMsgIdList, 0x00, sizeof(msg_id_list_s));

	char sqlQuery[MAX_QUERY_LEN+1];

	// Get MSG_ID List
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s A WHERE CONV_ID = %d AND READ_STATUS = 0;",
			MSGFW_MESSAGE_TABLE_NAME, threadId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return err;
	}

	pUnreadMsgIdList->nCount = rowCnt;

	MSG_DEBUG("unreadMsgIdList.nCount [%d]", pUnreadMsgIdList->nCount);

	pUnreadMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pUnreadMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();


	// Get sim MSG_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s A \
			WHERE CONV_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_STORAGE_SIM);

	rowCnt = 0;
	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return err;
	}

	for (int i = 1; i <= rowCnt; i++) {
		MsgStoUpdateReadStatus(dbHandle.getColumnToInt(i), true);
	}

	dbHandle.freeTable();

	// set read status
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET READ_STATUS = %d \
			WHERE CONV_ID = %d AND READ_STATUS = 0;",
			MSGFW_MESSAGE_TABLE_NAME, 1, threadId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;


	if (MsgStoUpdateConversation(&dbHandle, threadId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoUpdateConversation() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	if (g_idle_add(updateUnreadMsgCount, NULL) == 0) {
		MSG_DEBUG("updateUnreadMsgCount() Error");
	}

	if (pUnreadMsgIdList->nCount > 0) {
		if (g_idle_add(startToDeleteNoti, (void *)pUnreadMsgIdList) == 0) {
			MSG_DEBUG("startToDeleteNoti not invoked: %s", strerror(errno));
			// memory free
			if (pUnreadMsgIdList != NULL) {
				//free peer info list
				if (pUnreadMsgIdList->msgIdList != NULL)
					delete [] pUnreadMsgIdList->msgIdList;

				delete [] pUnreadMsgIdList;
			}
			err = MSG_ERR_UNKNOWN;
		}
	}
	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoUpdateProtectedStatus(msg_message_id_t msgId, bool bProtected)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET PROTECTED = %d WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, (int)bProtected, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteMessage(msg_message_id_t msgId, bool bCheckIndication)
{
	MSG_BEGIN();

	MSG_DEBUG("Msg Id : %d", msgId);

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Get SUB_TYPE, STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MAIN_TYPE, A.SUB_TYPE, A.FOLDER_ID, A.STORAGE_ID, A.READ_STATUS, B.CONV_ID \
			FROM %s A, %s B WHERE A.MSG_ID = %d AND A.CONV_ID = B.CONV_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Query Failed [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	MSG_MESSAGE_TYPE_S msgType;
	msg_folder_id_t folderId;
	msg_storage_id_t storageId;
	msg_thread_id_t convId;
	bool bRead;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = dbHandle.columnInt(0);
		msgType.subType = dbHandle.columnInt(1);
		folderId = dbHandle.columnInt(2);
		storageId = dbHandle.columnInt(3);
		bRead = dbHandle.columnInt(4);
		convId = dbHandle.columnInt(5);

		MSG_DEBUG("Main Type:[%d] SubType:[%d] FolderId:[%d] StorageId:[%d] ConversationId:[%d]", msgType.mainType, msgType.subType, folderId, storageId, convId);
	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);

		dbHandle.finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	if (plg == NULL) {
		MSG_DEBUG("SMS Plug-in is NULL");

		return MSG_ERR_NULL_POINTER;
	}

	dbHandle.beginTrans();

	// Check sim message
	if (storageId == MSG_STORAGE_SIM) {
		// get sim message id
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_ID FROM %s WHERE MSG_ID = %d;",
				MSGFW_SIM_MSG_TABLE_NAME, msgId);

		MSG_DEBUG("sqlQuery is [%s]", sqlQuery);

		msg_sim_id_t simMsgId;

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		while (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
			simMsgId = dbHandle.columnInt(0);

			MSG_DEBUG("SIM Msg Id : [%d]", simMsgId);

			err = plg->deleteSimMessage(simMsgId);

			if (err != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				return err;
			}

			//Sim message delete in db table
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE SIM_ID = %d;", MSGFW_SIM_MSG_TABLE_NAME, simMsgId);

			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}

		dbHandle.finalizeQuery();
	}

	/* each type has to be handled in plug in ? */
	if (msgType.mainType == MSG_SMS_TYPE) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MSGFW_SMS_SENDOPT_TABLE_NAME, msgId);

		// Delete SMS Send Option
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (msgType.subType == MSG_CB_SMS || msgType.subType == MSG_JAVACB_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_CB_MSG_TABLE_NAME, msgId);

			// Delete Push Message from push table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType >= MSG_WAP_SI_SMS && msgType.subType <= MSG_WAP_CO_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_PUSH_MSG_TABLE_NAME, msgId);

			// Delete Push Message from push table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType == MSG_SYNCML_CP) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
					MSGFW_SYNCML_MSG_TABLE_NAME, msgId);

			// Delete SyncML Message from syncML table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else if (msgType.mainType == MSG_MMS_TYPE) {

		char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
		char thumbnailpath[MSG_FILEPATH_LEN_MAX] = {0,};
		char dirPath[MSG_FILEPATH_LEN_MAX]= {0,};

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_PREPARE;
		}

		if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
			strncpy(filePath, (char *)dbHandle.columnText(0), MSG_FILEPATH_LEN_MAX);

			snprintf(dirPath, MSG_FILEPATH_LEN_MAX, "%s.dir", filePath);

			if (remove(filePath) == -1)
				MSG_DEBUG("Fail to delete file [%s]", filePath);
			else
				MSG_DEBUG("Success to delete file [%s]", filePath);

			MsgRmRf(dirPath);

			rmdir(dirPath);

			// remove thumbnail file
			char *fileName = NULL;
			fileName = strrchr(filePath, '/');

			snprintf(thumbnailpath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"%s.jpg", fileName+1);
			if(remove(thumbnailpath) == -1)
				MSG_DEBUG("Fail to delete thumbnail [%s]", thumbnailpath);
			else
				MSG_DEBUG("Success to delete thumbnail [%s]", thumbnailpath);

		} else {
			MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
			dbHandle.finalizeQuery();
			dbHandle.endTrans(false);
			return MSG_ERR_DB_STEP;
		}

		dbHandle.finalizeQuery();

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
				MMS_PLUGIN_MESSAGE_TABLE_NAME, msgId);

		// Delete Data from MMS table
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgId);

	// Delete Message from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Update conversation table.
	if (MsgStoUpdateConversation(&dbHandle, convId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	if (msgType.mainType == MSG_SMS_TYPE && folderId == MSG_INBOX_ID) {
		msgType.classType = MSG_CLASS_NONE;

		// Set memory status in SIM
		if (MsgStoCheckMsgCntFull(&dbHandle, &msgType, folderId) == MSG_SUCCESS) {
			MSG_DEBUG("Set Memory Status");

			plg->setMemoryStatus(MSG_SUCCESS);
		}
	}

	if (bCheckIndication == true) {
		MSG_DEBUG("bCheckIndication is true.");

		int smsCnt = 0;
		int mmsCnt = 0;

		smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
		mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

		MsgSettingSetIndicator(smsCnt, mmsCnt);

		MsgDeleteNotiByMsgId(msgId);
	}

	//Delete phone log
	MsgDeletePhoneLog(msgId);

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteAllMessageInFolder(msg_folder_id_t folderId, bool bOnlyDB, msg_id_list_s *pMsgIdList)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<msg_thread_id_t> threadList;

	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME, 
						MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char *);
	int rowCnt = 0;

	// Get conversation ID from Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(CONV_ID) FROM %s WHERE FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, folderId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("sql query is %s.", sqlQuery);

		dbHandle.freeTable();
		return err;
	}

	if (rowCnt <= 0) {
		dbHandle.freeTable();

		return MSG_SUCCESS;
	}

	for (int i = 1; i <= rowCnt; i++) {
		MSG_DEBUG("thread ID : %d", dbHandle.getColumnToInt(i));
		threadList.push((msg_thread_id_t)dbHandle.getColumnToInt(i));
	}

	dbHandle.freeTable();

	/*** Get msg id list **/
	msg_id_list_s *pToDeleteMsgIdList = NULL;
	pToDeleteMsgIdList = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
	memset(pToDeleteMsgIdList, 0x00, sizeof(msg_id_list_s));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, folderId);

	rowCnt = 0;
	int index = 1;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		dbHandle.freeTable();

		goto FREE_MEMORY;
	}

	if (rowCnt <= 0) {
		dbHandle.freeTable();
		err = MSG_SUCCESS;

		goto FREE_MEMORY;
	}

	pToDeleteMsgIdList->nCount = rowCnt;

	MSG_DEBUG("pToDeleteMsgIdList->nCount [%d]", pToDeleteMsgIdList->nCount);

	pToDeleteMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pToDeleteMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();
	/*** **/

	/*** Delete Sim Message In Folder **/
	if (folderId >= MSG_INBOX_ID) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND STORAGE_ID = %d",
				MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_STORAGE_SIM);

		MSG_DEBUG("sql query is %s.", sqlQuery);

		rowCnt = 0;

		err = dbHandle.getTable(sqlQuery, &rowCnt);

		if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
			dbHandle.freeTable();

			goto FREE_MEMORY;
		}

		for (int i = 1; i <= rowCnt; i++) {
			err = MsgStoDeleteMessage(dbHandle.getColumnToInt(i), false);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoDeleteMessage() Error!!!");

				dbHandle.freeTable();

				goto FREE_MEMORY;
			}

			//Delete phone log
			MsgDeletePhoneLog(dbHandle.getColumnToInt(i));
		}

		dbHandle.freeTable();
	}

	dbHandle.beginTrans();

	for (int i = 0; i < listCnt; i++) {
		if (!strcmp(tableList[i], MMS_PLUGIN_MESSAGE_TABLE_NAME)) {

			int rowCnt = 0;

			char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
			char dirPath[MSG_FILEPATH_LEN_MAX] = {0,};
			char thumbnailPath[MSG_FILEPATH_LEN_MAX] = {0,};

			//get mms msg id list
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.FILE_PATH FROM %s A, %s B \
					WHERE A.FOLDER_ID = %d AND A.MAIN_TYPE = %d AND A.MSG_ID = B.MSG_ID",
					MSGFW_MESSAGE_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME, folderId, MSG_MMS_TYPE);

			MSG_DEBUG("sqlQuery [%s]", sqlQuery);

			err = dbHandle.getTable(sqlQuery, &rowCnt);
			MSG_DEBUG("rowCnt %d", rowCnt);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("sqlQuery [%s]", sqlQuery);

				dbHandle.freeTable();
				dbHandle.endTrans(false);

				goto FREE_MEMORY;
			}

			for (int i = 1; i <= rowCnt; i++) {

				memset(filePath, 0x00, sizeof(filePath));
				dbHandle.getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);

				MSG_DEBUG("filePath [%s]", filePath);

				//delete raw file
				snprintf(dirPath, sizeof(dirPath), "%s.dir", filePath);

				if (remove(filePath) == -1)
					MSG_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_DEBUG("Success to delete file [%s]", filePath);

				MsgRmRf(dirPath);

				rmdir(dirPath);
				// delete thumbnail

				char *fileName = NULL;
				fileName = strrchr(filePath, '/');

				snprintf(thumbnailPath, sizeof(thumbnailPath), MSG_THUMBNAIL_PATH"%s.jpg", fileName+1);

				if (remove(thumbnailPath) == -1)
					MSG_DEBUG("Fail to delete thumbnail [%s]", thumbnailPath);
				else
					MSG_DEBUG("Success to delete thumbnail [%s]", thumbnailPath);

			}

			dbHandle.freeTable();
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
				(SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d);",
				tableList[i], MSGFW_MESSAGE_TABLE_NAME, folderId);

		// Delete Message in specific folder from table
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbHandle.endTrans(false);
			err = MSG_ERR_DB_EXEC;

			goto FREE_MEMORY;
		}
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		err = MSG_ERR_DB_EXEC;

		goto FREE_MEMORY;
	}

	// Update Address
	while (!threadList.empty()) {
		err = MsgStoUpdateConversation(&dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);

			goto FREE_MEMORY;
		}
	}

	dbHandle.endTrans(true);

	if (folderId == MSG_INBOX_ID) {
		int smsCnt = 0;
		int mmsCnt = 0;

		smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
		mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

		MsgSettingSetIndicator(smsCnt, mmsCnt);
	}

/*** Set pMsgIdList **/
	if (pMsgIdList != NULL && pToDeleteMsgIdList->nCount > 0) {
		pMsgIdList->nCount = pToDeleteMsgIdList->nCount;

		pMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount];
		memcpy(pMsgIdList->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount);
	}
/*** **/

/*** Create thread  for noti and phone log delete. **/
	if (!bOnlyDB) {
		if (pToDeleteMsgIdList->nCount > 0) {

			msg_id_list_s *pToDeleteMsgIdListCpy = NULL;
			pToDeleteMsgIdListCpy = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
			memset(pToDeleteMsgIdListCpy, 0x00, sizeof(msg_id_list_s));

			pToDeleteMsgIdListCpy->nCount = pToDeleteMsgIdList->nCount;

			pToDeleteMsgIdListCpy->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount];
			memcpy(pToDeleteMsgIdListCpy->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount);

			if (g_idle_add(startToDeleteNoti, (void *)pToDeleteMsgIdList) == 0) {
				MSG_DEBUG("startToDeleteNoti not invoked: %s", strerror(errno));
				// memory free
				if (pToDeleteMsgIdList != NULL) {
					//free peer info list
					if (pToDeleteMsgIdList->msgIdList != NULL)
						delete [] pToDeleteMsgIdList->msgIdList;

					delete [] pToDeleteMsgIdList;
				}
				err = MSG_ERR_UNKNOWN;
			}

			if (g_idle_add(startToDeletePhoneLog, (void *)pToDeleteMsgIdListCpy) == 0) {
				MSG_DEBUG("startToDeletePhoneLog not invoked: %s", strerror(errno));
				// memory free
				if (pToDeleteMsgIdListCpy != NULL) {
					//free peer info list
					if (pToDeleteMsgIdListCpy->msgIdList != NULL)
						delete [] pToDeleteMsgIdListCpy->msgIdList;

					delete [] pToDeleteMsgIdListCpy;
				}
				err = MSG_ERR_UNKNOWN;
			}
		}
	}
/*** **/

	return MSG_SUCCESS;

FREE_MEMORY:
	MSG_DEBUG("Error case Free Memory");

	while (!threadList.empty()) {
		threadList.front();
		threadList.pop();
	}

	// memory free
	if (pToDeleteMsgIdList != NULL) {
		//free peer info list
		if (pToDeleteMsgIdList->msgIdList != NULL)
			delete [] pToDeleteMsgIdList->msgIdList;

		delete [] pToDeleteMsgIdList;
	}

	return err;

}


msg_error_t MsgStoMoveMessageToFolder(msg_message_id_t msgId, msg_folder_id_t destFolderId)
{
	MSG_MESSAGE_TYPE_S msgType;
	msg_thread_id_t convId;

	MsgStoGetMsgType(msgId, &msgType);

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgType.mainType == MSG_SMS_TYPE)
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, destFolderId, msgId);
	else
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, destFolderId, msgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	/* get conversation id */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID FROM %s WHERE MSG_ID = %d;",
							MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW)
		convId = dbHandle.columnInt(0);

	MSG_DEBUG("convId : %d",  convId);

	dbHandle.finalizeQuery();

	/* update conversation table */
	MsgStoUpdateConversation(&dbHandle, convId);

	return MSG_SUCCESS;
}


msg_error_t MsgStoMoveMessageToStorage(msg_message_id_t msgId, msg_storage_id_t destStorageId)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	MSG_DEBUG("msgId : %d, destStorageId : %d", msgId, destStorageId);

	switch (destStorageId) {
	case MSG_STORAGE_SIM : // Move message to sim card
		{
			MSG_MESSAGE_INFO_S msgInfo;
			SMS_SIM_ID_LIST_S simIdList;

			memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
			memset(&simIdList, 0x00, sizeof(SMS_SIM_ID_LIST_S));

			if ((err = MsgStoGetMessage(msgId, &msgInfo, NULL)) != MSG_SUCCESS)
				return err;

			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

			if ((err = plg->saveSimMessage(&msgInfo, &simIdList)) != MSG_SUCCESS)
				return err;

			dbHandle.beginTrans();

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, destStorageId, msgId);

			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}

			for (unsigned int i = 0; i < simIdList.count; i++) {
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d);",
						MSGFW_SIM_MSG_TABLE_NAME, msgId, simIdList.simId[i]);

				if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.endTrans(false);
					return MSG_ERR_DB_EXEC;
				}
			}

			dbHandle.endTrans(true);
		}
	break;

	default: //Moving message to memory (when destination storage id is MSG_STORAGE_PHONE)
		{
			bool bSimMsg = false;
			int rowCnt = 0;

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT STORAGE_ID FROM %s WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, msgId);

			err = dbHandle.getTable(sqlQuery, &rowCnt);

			if (err != MSG_SUCCESS) {
				dbHandle.freeTable();
				return err;
			}

			if (dbHandle.getColumnToInt(1) == MSG_STORAGE_SIM) {
				MSG_DEBUG("It is SIM Message");
				bSimMsg = true;
			}

			dbHandle.freeTable();

			if (bSimMsg == true) {
				msg_sim_id_t simMsgId;

				// get sim message id
				memset(sqlQuery, 0x00, sizeof(sqlQuery));

				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_ID FROM %s WHERE MSG_ID = %d;",
						MSGFW_SIM_MSG_TABLE_NAME, msgId);

				MSG_DEBUG("sqlQuery is %s.", sqlQuery);

				err = dbHandle.getTable(sqlQuery, &rowCnt);

				if (err != MSG_SUCCESS) {
					dbHandle.freeTable();
					return err;
				}

				//Delete messages in sim card
				MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

				for (int i = 0; i < rowCnt; i++) {
					simMsgId = dbHandle.getColumnToInt(i+1);

					MSG_DEBUG("simMsgId is %d.", simMsgId);

					if ((err = plg->deleteSimMessage(simMsgId)) != MSG_SUCCESS) {
						dbHandle.freeTable();
						return err;
					}
				}

				dbHandle.freeTable();

				dbHandle.beginTrans();

				//Delete Messages in SIM Msg table
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
						MSGFW_SIM_MSG_TABLE_NAME, msgId);

				MSG_DEBUG("sqlQuery is %s.", sqlQuery);

				if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.endTrans(false);
					return MSG_ERR_DB_EXEC;
				}

				//Move storage id
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, destStorageId, msgId);

				if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.endTrans(false);
					return MSG_ERR_DB_EXEC;
				}

				dbHandle.endTrans(true);
			}
		}
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

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND READ_STATUS = 1 \
			UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND READ_STATUS = 0 \
			UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d \
			UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d;",
			MSGFW_MESSAGE_TABLE_NAME, folderId,
			MSGFW_MESSAGE_TABLE_NAME, folderId,
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_SMS_TYPE,
			MSGFW_MESSAGE_TABLE_NAME, folderId, MSG_MMS_TYPE);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nReadCnt = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nUnreadCnt = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nSms = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pCountInfo->nMms = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

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

	// SMS
	if ((pMsgType->mainType == MSG_SMS_TYPE) &&
			(pMsgType->subType == MSG_NORMAL_SMS || pMsgType->subType == MSG_STATUS_REPORT_SMS || pMsgType->subType == MSG_CONCAT_SIM_SMS)) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_NORMAL_SMS, MSG_STATUS_REPORT_SMS, MSG_CONCAT_SIM_SMS);
	} else if ((pMsgType->mainType == MSG_SMS_TYPE) &&
			(pMsgType->subType == MSG_WAP_SI_SMS || pMsgType->subType == MSG_WAP_SL_SMS)) {
		// PUSH
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_WAP_SI_SMS, MSG_WAP_SL_SMS);
	} else if ((pMsgType->mainType == MSG_MMS_TYPE) &&
			(pMsgType->subType == MSG_SENDREQ_MMS || pMsgType->subType == MSG_SENDCONF_MMS || pMsgType->subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgType->subType == MSG_RETRIEVE_MANUALCONF_MMS)) {
		// MMS
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d, %d, %d);",
				MSGFW_MESSAGE_TABLE_NAME, pMsgType->mainType, MSG_SENDREQ_MMS, MSG_SENDCONF_MMS, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);
	} else {
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		*pMsgCount = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMessage(msg_message_id_t msgId, MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	int order = MsgGetContactNameOrder();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, \
			SUB_TYPE, DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, \
			BACKUP, PRIORITY, MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, THUMB_PATH \
			FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pMsg->msgId = dbHandle.columnInt(0);
		pMsg->threadId = dbHandle.columnInt(1);
		pMsg->folderId = dbHandle.columnInt(2);
		pMsg->storageId = dbHandle.columnInt(3);
		pMsg->msgType.mainType = dbHandle.columnInt(4);
		pMsg->msgType.subType = dbHandle.columnInt(5);
		pMsg->displayTime = (time_t)dbHandle.columnInt(6);
		pMsg->dataSize = dbHandle.columnInt(7);
		pMsg->networkStatus = dbHandle.columnInt(8);
		pMsg->bRead = dbHandle.columnInt(9);
		pMsg->bProtected = dbHandle.columnInt(10);
		pMsg->bBackup = dbHandle.columnInt(11);
		pMsg->priority = dbHandle.columnInt(12);
		pMsg->direction= dbHandle.columnInt(13);

		strncpy(pMsg->subject, (char *)dbHandle.columnText(15), MAX_SUBJECT_LEN);

		/* Temp_File_Handling */
		if (pMsg->msgType.mainType == MSG_SMS_TYPE || pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
			if (pMsg->dataSize > MAX_MSG_TEXT_LEN) {
				char msgData[pMsg->dataSize+1];
				memset(msgData, 0x00, sizeof(msgData));

				strncpy(msgData, (char *)dbHandle.columnText(16), pMsg->dataSize);

				// Save Message Data into File
				char fileName[MAX_COMMON_INFO_SIZE+1];
				memset(fileName, 0x00, sizeof(fileName));

				if (MsgCreateFileName(fileName) == false) {
					dbHandle.finalizeQuery();
					return MSG_ERR_STORAGE_ERROR;
				}

				MSG_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsg->dataSize, fileName);

				if (MsgWriteIpcFile(fileName, msgData, pMsg->dataSize) == false) {
					dbHandle.finalizeQuery();
					return MSG_ERR_STORAGE_ERROR;
				}

				strncpy(pMsg->msgData, fileName, MAX_MSG_DATA_LEN);

				pMsg->bTextSms = false;
			} else {
				memset(pMsg->msgText, 0x00, sizeof(pMsg->msgText));
				strncpy(pMsg->msgText, (char *)dbHandle.columnText(16), pMsg->dataSize);

				// For WAP PUSH SI Message
				if (pMsg->msgType.subType == MSG_WAP_SI_SMS) {
					strncat(pMsg->msgText, "\n", MAX_MSG_TEXT_LEN-strlen(pMsg->msgText));
					strncat(pMsg->msgText, pMsg->subject, MAX_MSG_TEXT_LEN-strlen(pMsg->msgText));
					MSG_DEBUG("pMsg->msgText : [%s]", pMsg->msgText);
					pMsg->dataSize = sizeof(pMsg->msgText);
				}

				pMsg->bTextSms = true;
			}
		} else {
			if (dbHandle.columnText(16) != NULL)
				strncpy(pMsg->msgText, (char *)dbHandle.columnText(16), MAX_MSG_TEXT_LEN);
		}

		// thumbnail path
		if (dbHandle.columnText(17)!=NULL && ((char *)dbHandle.columnText(17))[0] != '\0') {
			strncpy(pMsg->thumbPath, (char *)dbHandle.columnText(17), MSG_FILEPATH_LEN_MAX);
			MSG_DEBUG("pMsg->thumbPath : [%s]", pMsg->thumbPath);
		}
	} else {
		dbHandle.finalizeQuery();
		MSG_DEBUG("%s", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();


	// get address information.
	MsgStoGetAddressByMsgId(&dbHandle, pMsg->msgId, order, &pMsg->nAddressCnt, pMsg->addressList);

	// Get MMS body if it is MMS.
	if ((pMsg->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS &&
			(pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS)) ||
			pMsg->msgType.subType == MSG_SENDREQ_MMS || pMsg->msgType.subType == MSG_SENDCONF_MMS) {
		msg_error_t err = MSG_SUCCESS;
		MMS_MESSAGE_DATA_S	mmsMsg;
		char *pDestMsg = NULL;

		// call mms plugin to get mms specific message data
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsg->msgType.mainType);
		memset(&mmsMsg, 0, sizeof(MMS_MESSAGE_DATA_S));
		err =  plg->getMmsMessage(pMsg, pSendOptInfo, &mmsMsg, &pDestMsg);

		if (err != MSG_SUCCESS) {
			if (pDestMsg) {
				free(pDestMsg);
				pDestMsg = NULL;
			}
			return MSG_ERR_STORAGE_ERROR;
		}

		memset(pMsg->msgData, 0, MAX_MSG_DATA_LEN+1);

		// Encode MMS specific data to MMS_MESSAGE_DATA_S
		if (pMsg->dataSize > MAX_MSG_DATA_LEN) {
			// Save Message Data into File
			char tempFileName[MAX_COMMON_INFO_SIZE+1];
			memset(tempFileName, 0x00, sizeof(tempFileName));

			if (MsgCreateFileName(tempFileName) == false) {
				if(pDestMsg) {
					free (pDestMsg);
					pDestMsg = NULL;
				}
				return MSG_ERR_STORAGE_ERROR;
			}
			MSG_DEBUG("Save Message Data into file : size[%d] name[%s]\n", pMsg->dataSize, tempFileName);

			if (MsgWriteIpcFile(tempFileName, pDestMsg, pMsg->dataSize) == false) {
				if(pDestMsg) {
					free (pDestMsg);
					pDestMsg = NULL;
				}
				return MSG_ERR_STORAGE_ERROR;
			}
			strncpy(pMsg->msgData, tempFileName, MAX_MSG_DATA_LEN);
			//pMsg->dataSize = strlen(fileName);
			pMsg->bTextSms = false;
		} else {
			strncpy(pMsg->msgData, pDestMsg, pMsg->dataSize);
			pMsg->bTextSms = true;
		}

		if (pDestMsg) {
			free(pDestMsg);
			pDestMsg = NULL;
		}
	}

	// Get SMS Sending Options
	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL)
		MsgStoGetSmsSendOpt(pMsg->msgId, pSendOptInfo);

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetFolderViewList(msg_folder_id_t folderId, const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pMsgFolderViewList)
{
	if (pMsgFolderViewList == NULL) {
		MSG_DEBUG("pMsgFolderViewList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0;
	int index = 19; // numbers of index


	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlSort[64];

	// Get Name Order
	int order = MsgGetContactNameOrder();

	// Get Message In Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (folderId == MSG_ALLBOX_ID) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
				DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, BACKUP, PRIORITY, \
				MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, THUMB_PATH \
				FROM %s WHERE FOLDER_ID < %d ",
				MSGFW_MESSAGE_TABLE_NAME, MSG_SPAMBOX_ID);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
				DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, BACKUP, PRIORITY, \
				MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, THUMB_PATH \
				FROM %s WHERE FOLDER_ID = %d ",
				MSGFW_MESSAGE_TABLE_NAME, folderId);
	}

	memset(sqlSort, 0x00, sizeof(sqlSort));
	MsgMakeSortRule(pSortRule, sqlSort);
	strncat(sqlQuery, sqlSort, strlen(sqlSort));

	msg_error_t  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		pMsgFolderViewList->nCount = 0;
		pMsgFolderViewList->msg_struct_info = NULL;

		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("sqlQuery is - %s", sqlQuery);
		dbHandle.freeTable();
		return err;
	}

	pMsgFolderViewList->nCount = rowCnt;

	MSG_DEBUG("pMsgCommInfoList->nCount [%d]", pMsgFolderViewList->nCount);

	pMsgFolderViewList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * rowCnt];

	msg_struct_s *msg = NULL;
	MSG_MESSAGE_HIDDEN_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pMsgFolderViewList->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];;

		msg = (msg_struct_s *)pMsgFolderViewList->msg_struct_info[i];

		msg->type = MSG_STRUCT_MESSAGE_INFO;
		msg->data = new char[sizeof(MSG_MESSAGE_HIDDEN_S)];

		pTmp = (MSG_MESSAGE_HIDDEN_S *)msg->data;

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		pTmp->threadId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->mainType = dbHandle.getColumnToInt(index++);
		pTmp->subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction= dbHandle.getColumnToInt(index++);
		index++; // This field is reserved.

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			pTmp->pData = (void *)new char[pTmp->dataSize + 2];
			memset(pTmp->pData, 0x00, sizeof(pTmp->pData));

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		// get address information from db.
		msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

		MsgStoGetAddressByMsgId(&dbHandle, pTmp->msgId, order, addr_list);

		pTmp->addr_list = addr_list;

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoAddSyncMLMessage(MSG_MESSAGE_INFO_S *pMsgInfo, int extId, int pinCode)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int rowId = 0;
	msg_thread_id_t convId = 0;

	dbHandle.beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, pMsgInfo, &convId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	// Add Message Table
	pMsgInfo->threadId = convId;
	rowId = MsgStoAddMessageTable(&dbHandle, pMsgInfo);

	if (rowId <= 0) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
			MSGFW_SYNCML_MSG_TABLE_NAME, rowId, extId, pinCode);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (MsgStoUpdateConversation(&dbHandle, convId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	pMsgInfo->msgId = (msg_message_id_t)rowId;

	MsgSoundPlayStart();

	int smsCnt = 0;
	int mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

	MsgSettingHandleNewMsg(smsCnt, mmsCnt);

	MsgInsertNoti(&dbHandle, pMsgInfo);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMsgType(msg_message_id_t msgId, MSG_MESSAGE_TYPE_S *pMsgType)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pMsgType->mainType = dbHandle.columnInt(0);
		pMsgType->subType = dbHandle.columnInt(1);
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetQuickPanelData(msg_quickpanel_type_t QPtype, MSG_MESSAGE_INFO_S *pMsg)
{
	msg_error_t	err = MSG_SUCCESS;

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

	// Get Message ID
	msg_message_id_t msgId;

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgId = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	// Get Message Info
	err = MsgStoGetMessage(msgId, pMsg, NULL);

	return err;
}


msg_error_t MsgStoGetThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList)
{
	pThreadViewList->nCount = 0;
	pThreadViewList->msg_struct_info = NULL;

	int rowCnt = 0;
	int index = 10; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID, UNREAD_CNT, SMS_CNT, MMS_CNT, \
			MAIN_TYPE, SUB_TYPE, MSG_DIRECTION, DISPLAY_TIME, DISPLAY_NAME, MSG_TEXT \
			FROM %s WHERE SMS_CNT > 0 OR MMS_CNT > 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_CONVERSATION_TABLE_NAME);

	msg_error_t  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);
		dbHandle.freeTable();
		return err;
	}

	if (rowCnt < 1) {
		MSG_DEBUG("rowCnt is %d", rowCnt);
		dbHandle.freeTable();
		return err;
	}

	pThreadViewList->nCount = rowCnt;

	MSG_DEBUG("pThreadViewList->nCount [%d]", pThreadViewList->nCount);

	pThreadViewList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t)*rowCnt];

	MSG_THREAD_VIEW_S *pTmp = NULL;
	msg_struct_s *thread_t = NULL;

	for (int i = 0; i < rowCnt; i++) {
		thread_t = (msg_struct_s *)new msg_struct_s;
		pThreadViewList->msg_struct_info[i] = (msg_struct_t)thread_t;

		thread_t->type = MSG_STRUCT_THREAD_INFO;
		thread_t->data = new MSG_THREAD_VIEW_S;

		pTmp = (MSG_THREAD_VIEW_S *)thread_t->data;
		memset(pTmp, 0x00, sizeof(MSG_THREAD_VIEW_S));

		pTmp->threadId = dbHandle.getColumnToInt(index++);

		pTmp->unreadCnt = dbHandle.getColumnToInt(index++);
		pTmp->smsCnt = dbHandle.getColumnToInt(index++);
		pTmp->mmsCnt = dbHandle.getColumnToInt(index++);

		pTmp->mainType = dbHandle.getColumnToInt(index++);
		pTmp->subType = dbHandle.getColumnToInt(index++);

		pTmp->direction = dbHandle.getColumnToInt(index++);
		pTmp->threadTime = (time_t)dbHandle.getColumnToInt(index++);

		memset(pTmp->threadName, 0x00, sizeof(pTmp->threadName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, pTmp->threadName);

		memset(pTmp->threadData, 0x00, sizeof(pTmp->threadData));
		dbHandle.getColumnToString(index++, MAX_THREAD_DATA_LEN, pTmp->threadData);
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetConversationViewList(msg_thread_id_t threadId, msg_struct_list_s *pConvViewList)
{
	MSG_BEGIN();

	pConvViewList->nCount = 0;
	pConvViewList->msg_struct_info = NULL;

	int rowCnt = 0;
	int index = 19; /** numbers of index */
	int order = 0;
	char sqlQuery[MAX_QUERY_LEN+1];

	// get address information.
	order = MsgGetContactNameOrder();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
			DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, BACKUP, PRIORITY, \
			MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, THUMB_PATH \
			FROM %s WHERE CONV_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d ORDER BY DISPLAY_TIME, MSG_ID ASC;",
			MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_ALLBOX_ID, MSG_SPAMBOX_ID);

	msg_error_t err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);
		dbHandle.freeTable();
		return err;
	}

	pConvViewList->nCount = rowCnt;

	MSG_DEBUG("pConvViewList->nCount [%d]", pConvViewList->nCount);

	pConvViewList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * rowCnt];

	msg_struct_s *msg = NULL;
	MSG_MESSAGE_HIDDEN_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pConvViewList->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];;

		msg = (msg_struct_s *)pConvViewList->msg_struct_info[i];

		msg->type = MSG_STRUCT_MESSAGE_INFO;
		msg->data = new char[sizeof(MSG_MESSAGE_HIDDEN_S)];

		pTmp = (MSG_MESSAGE_HIDDEN_S *)msg->data;

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		pTmp->threadId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->mainType = dbHandle.getColumnToInt(index++);
		pTmp->subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction = dbHandle.getColumnToInt(index++);
		index++; // This field is reserved.

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			pTmp->pData = (void *)new char[pTmp->dataSize+2];
			memset(pTmp->pData, 0x00, pTmp->dataSize+2);

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);

		// set address list handle.
		msg_struct_list_s *addrlist = (msg_struct_list_s *)new msg_struct_list_s;
		memset(addrlist, 0x00, sizeof(msg_struct_list_s));
		MsgDbHandler dbHandleForInner;
		MsgStoGetAddressByConvId(&dbHandleForInner, threadId, order, addrlist);

		pTmp->addr_list = addrlist;

	}

	dbHandle.freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoDeleteThreadMessageList(msg_thread_id_t threadId, msg_id_list_s *pMsgIdList)
{
	msg_error_t err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	/*** Get msg id list **/
	msg_id_list_s *pToDeleteMsgIdList = NULL;

	int rowCnt = 0;
	int index = 1;
	// Set Indicator
	int smsCnt = 0;
	int mmsCnt = 0;

	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
			MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SMS_SENDOPT_TABLE_NAME, 
			MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char *);

	pToDeleteMsgIdList = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
	memset(pToDeleteMsgIdList, 0x00, sizeof(msg_id_list_s));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE CONV_ID = %d", MSGFW_MESSAGE_TABLE_NAME, threadId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		dbHandle.freeTable();

		goto FREE_MEMORY;
	}

	if (rowCnt <= 0) {
		dbHandle.freeTable();
		err = MSG_SUCCESS;

		goto FREE_MEMORY;
	}

	pToDeleteMsgIdList->nCount = rowCnt;

	MSG_DEBUG("pToDeleteMsgIdList->nCount [%d]", pToDeleteMsgIdList->nCount);

	pToDeleteMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pToDeleteMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();
	/*** **/

	/*** Delete Sim Message **/
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE CONV_ID = %d AND STORAGE_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, threadId, MSG_STORAGE_SIM);

	rowCnt = 0;

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		MSG_DEBUG("sqlQuery [%s]", sqlQuery);

		dbHandle.freeTable();

		goto FREE_MEMORY;
	}

	for (int i = 1; i <= rowCnt; i++) {
		err = MsgStoDeleteMessage(dbHandle.getColumnToInt(i), false);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoDeleteMessage() Error!!!");

			dbHandle.freeTable();

			goto FREE_MEMORY;
		}
	}

	dbHandle.freeTable();
	/*** **/

	dbHandle.beginTrans();

	for (int i = 0; i < listCnt; i++) {
		if (!strcmp(tableList[i], MMS_PLUGIN_MESSAGE_TABLE_NAME)) {

			int rowCnt = 0;

			//get mms msg id list
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s A, %s B\
					WHERE A.CONV_ID = %d AND A.MAIN_TYPE = %d AND A.MSG_ID = B.MSG_ID;",
					MSGFW_MESSAGE_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME, threadId, MSG_MMS_TYPE);

			err = dbHandle.getTable(sqlQuery, &rowCnt);
			MSG_DEBUG("rowCnt %d", rowCnt);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("sqlQuery [%s]", sqlQuery);

				dbHandle.freeTable();
				dbHandle.endTrans(false);

				goto FREE_MEMORY;
			}

			for (int i = 1; i <= rowCnt; i++) {

				char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
				char dirPath[MSG_FILEPATH_LEN_MAX] = {0,};
				char thumbnailPath[MSG_FILEPATH_LEN_MAX] = {0,};

				dbHandle.getColumnToString(i, MSG_FILEPATH_LEN_MAX, filePath);

				MSG_DEBUG("filePath [%s]", filePath);

				//delete raw file
				snprintf(dirPath, sizeof(dirPath), "%s.dir", filePath);

				if (remove(filePath) == -1)
					MSG_DEBUG("Fail to delete file [%s]", filePath);
				else
					MSG_DEBUG("Success to delete file [%s]", filePath);

				MsgRmRf(dirPath);

				// remove directory also
				rmdir(dirPath);

				// delete thumbnail
				char *fileName = NULL;
				fileName = strrchr(filePath, '/');

				snprintf(thumbnailPath, sizeof(thumbnailPath), MSG_THUMBNAIL_PATH"%s.jpg", fileName+1);

				if (remove(thumbnailPath) == -1)
					MSG_DEBUG("Fail to delete thumbnail [%s]", thumbnailPath);
				else
					MSG_DEBUG("Success to delete thumbnail [%s]", thumbnailPath);
			}

			dbHandle.freeTable();
		}

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
				(SELECT MSG_ID FROM %s WHERE CONV_ID = %d);",
				tableList[i], MSGFW_MESSAGE_TABLE_NAME, threadId);

		// Delete Message in specific folder from table
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbHandle.endTrans(false);
			err = MSG_ERR_DB_EXEC;

			goto FREE_MEMORY;
		}
	}

	// Clear Conversation table
	if (MsgStoClearConversationTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		err = MSG_ERR_DB_EXEC;

		goto FREE_MEMORY;
	}

	dbHandle.endTrans(true);

	MSG_MESSAGE_TYPE_S msgType;

	msgType.mainType = MSG_SMS_TYPE;
	msgType.subType = MSG_NORMAL_SMS;
	msgType.classType = MSG_CLASS_NONE;

	// Set memory status in SIM
	if (MsgStoCheckMsgCntFull(&dbHandle, &msgType, MSG_INBOX_ID) == MSG_SUCCESS) {
		MSG_DEBUG("Set Memory Status");

		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

		if (plg == NULL) {
			MSG_DEBUG("SMS Plug-in is NULL");
			err = MSG_ERR_NULL_POINTER;

			goto FREE_MEMORY;
		}

		plg->setMemoryStatus(MSG_SUCCESS);
	}

	// Set Indicator
	smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

	MsgSettingSetIndicator(smsCnt, mmsCnt);

/*** Set pMsgIdList **/
	if (pMsgIdList != NULL && pToDeleteMsgIdList->nCount > 0) {
		pMsgIdList->nCount = pToDeleteMsgIdList->nCount;

		pMsgIdList->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount];
		memcpy(pMsgIdList->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount);
	}
/*** **/

	/*** Create thread  for noti and phone log delete. **/
	if (pToDeleteMsgIdList->nCount > 0) {
		msg_id_list_s *pToDeleteMsgIdListCpy = NULL;
		pToDeleteMsgIdListCpy = (msg_id_list_s *)new char[sizeof(msg_id_list_s)];
		memset(pToDeleteMsgIdListCpy, 0x00, sizeof(msg_id_list_s));

		pToDeleteMsgIdListCpy->nCount = pToDeleteMsgIdList->nCount;

		pToDeleteMsgIdListCpy->msgIdList = (msg_message_id_t *)new char[sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount];
		memcpy(pToDeleteMsgIdListCpy->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(msg_message_id_t)*pToDeleteMsgIdList->nCount);

		if (g_idle_add(startToDeleteNoti, (void *)pToDeleteMsgIdList) == 0) {
			MSG_DEBUG("startToDeleteNoti not invoked: %s", strerror(errno));
			// memory free
			if (pToDeleteMsgIdList != NULL) {
				//free peer info list
				if (pToDeleteMsgIdList->msgIdList != NULL)
					delete [] pToDeleteMsgIdList->msgIdList;

				delete [] pToDeleteMsgIdList;
			}
			err = MSG_ERR_UNKNOWN;
		}

		if (g_idle_add(startToDeletePhoneLog, (void *)pToDeleteMsgIdListCpy) == 0) {
			MSG_DEBUG("startToDeletePhoneLog not invoked: %s", strerror(errno));
			// memory free
			if (pToDeleteMsgIdListCpy != NULL) {
				//free peer info list
				if (pToDeleteMsgIdListCpy->msgIdList != NULL)
					delete [] pToDeleteMsgIdListCpy->msgIdList;

				delete [] pToDeleteMsgIdListCpy;
			}
			err = MSG_ERR_UNKNOWN;
		}
	}
	/*** **/

	return MSG_SUCCESS;

FREE_MEMORY:
	MSG_DEBUG("Error case Free Memory");
	// memory free
	if (pToDeleteMsgIdList != NULL) {
		//free peer info list
		if (pToDeleteMsgIdList->msgIdList != NULL) {
			delete [] pToDeleteMsgIdList->msgIdList;
			pToDeleteMsgIdList->msgIdList = NULL;
		}

		delete [] pToDeleteMsgIdList;
		pToDeleteMsgIdList = NULL;
	}

	return err;
}


msg_error_t MsgStoCountMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pThreadCountInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pAddrInfo->contactId > 0) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
				SUM(CASE WHEN READ_STATUS=0 AND FOLDER_ID=%d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END) \
				FROM (SELECT * FROM %s A  JOIN %s B ON A.ADDRESS_ID = B.ADDRESS_ID WHERE B.CONTACT_ID = %d)",
				MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->contactId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
				SUM(CASE WHEN READ_STATUS=0 AND FOLDER_ID=%d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END), \
				SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END) \
				FROM (SELECT * FROM %s A  JOIN %s B ON A.ADDRESS_ID = B.ADDRESS_ID WHERE B.ADDRESS_VAL = '%s')",
				MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->msgAddrInfo.addressVal);
	}

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pThreadCountInfo->totalCount = dbHandle.columnInt(0);
		pThreadCountInfo->unReadCount = dbHandle.columnInt(1);
		pThreadCountInfo->smsMsgCount = dbHandle.columnInt(2);
		pThreadCountInfo->mmsMsgCount = dbHandle.columnInt(3);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoSearchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList)
{
	if (!pSearchString)
		return MSG_ERR_NULL_POINTER;

	// Clear Out Parameter
	pThreadViewList->nCount = 0;
	pThreadViewList->msg_struct_info = NULL;

	tr1::unordered_set<msg_thread_id_t> IdList;
	queue<MSG_THREAD_VIEW_S> searchList;

	MSG_THREAD_VIEW_S threadView;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Replace string for '%' and '_' character
	char *ext1_str = NULL;
	char *ext2_str = NULL;

	ext1_str = MsgStoReplaceString(pSearchString, "_", "\\_");
	ext2_str = MsgStoReplaceString(ext1_str, "%", "\\%");

	// Search - Address, Name
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT C.CONV_ID, C.UNREAD_CNT, C.SMS_CNT, C.MMS_CNT, C.DISPLAY_NAME, \
			B.MAIN_TYPE, B.SUB_TYPE, B.MSG_DIRECTION, B.DISPLAY_TIME, B.MSG_TEXT \
			FROM %s A, %s B, %s C \
			WHERE A.CONV_ID = B.CONV_ID AND B.FOLDER_ID > 0 AND B.FOLDER_ID < %d \
			AND ( B.MSG_TEXT LIKE '%%%s%%' ESCAPE '\\' \
			OR B.SUBJECT LIKE '%%%s%%' ESCAPE '\\' \
			OR A.ADDRESS_VAL LIKE '%%%s%%' ESCAPE '\\' \
			OR A.DISPLAY_NAME LIKE '%%%s%%' ESCAPE '\\' \
			OR A.FIRST_NAME LIKE '%%%s%%' ESCAPE '\\' \
			OR A.LAST_NAME LIKE '%%%s%%' ESCAPE '\\' ) \
			AND A.CONV_ID = C.CONV_ID \
			ORDER BY B.DISPLAY_TIME DESC;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSGFW_CONVERSATION_TABLE_NAME,
			MSG_SPAMBOX_ID, ext2_str, ext2_str, ext2_str, ext2_str, ext2_str, ext2_str);


	if (ext1_str) {
		free(ext1_str);
		ext1_str = NULL;
	}

	if (ext2_str) {
		free(ext2_str);
		ext2_str = NULL;
	}

	MSG_DEBUG("[%s]", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("Prepare query fail. [%s]", sqlQuery);
		return MSG_ERR_DB_PREPARE;
	}

	while (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		memset(&threadView, 0x00, sizeof(threadView));

		threadView.threadId = dbHandle.columnInt(0);
		threadView.unreadCnt = dbHandle.columnInt(1);
		threadView.smsCnt = dbHandle.columnInt(2);
		threadView.mmsCnt = dbHandle.columnInt(3);

		strncpy(threadView.threadName, (char *)dbHandle.columnText(4), MAX_THREAD_NAME_LEN);

		threadView.mainType = dbHandle.columnInt(5);
		threadView.subType = dbHandle.columnInt(6);

		threadView.direction = dbHandle.columnInt(7);
		threadView.threadTime = (time_t)dbHandle.columnInt(8);

		strncpy(threadView.threadData, (char *)dbHandle.columnText(9), MAX_THREAD_DATA_LEN);

		tr1::unordered_set<msg_thread_id_t>::iterator it;

		it = IdList.find(threadView.threadId);

 		if (it == IdList.end()) {
			IdList.insert(threadView.threadId);
			searchList.push(threadView);
		}

	}

	dbHandle.finalizeQuery();

	// Add data to Out Parameter
	pThreadViewList->nCount = searchList.size();
	pThreadViewList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * searchList.size()];

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


msg_error_t MsgStoSearchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, msg_struct_list_s *pMsgList)
{
	// Clear Out Parameter
	pMsgList->nCount = 0;
	pMsgList->msg_struct_info = NULL;

	int rowCnt = 0;
	int index = 26; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlQuerySubset[(MAX_QUERY_LEN/5)+1];

	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];

	char *ext1_str = NULL;
	char *ext2_str = NULL;

	// Get Name Order
	int order = MsgGetContactNameOrder();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.CONV_ID, A.FOLDER_ID, A.STORAGE_ID, A.MAIN_TYPE, A.SUB_TYPE, \
			A.DISPLAY_TIME, A.DATA_SIZE, A.NETWORK_STATUS, A.READ_STATUS, A.PROTECTED, A.BACKUP, A.PRIORITY, \
			A.MSG_DIRECTION, A.SCHEDULED_TIME, A.SUBJECT, A.MSG_TEXT, B.ADDRESS_TYPE, B.RECIPIENT_TYPE, \
			B.CONTACT_ID, B.ADDRESS_VAL, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME, A.ATTACHMENT_COUNT, A.THUMB_PATH \
			FROM %s A, %s B \
			WHERE A.CONV_ID = B.CONV_ID AND B.ADDRESS_ID <> 0 ",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME);

	//// folder
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (pSearchCon->folderId == MSG_ALLBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.FOLDER_ID > 0 AND A.FOLDER_ID < %d ", MSG_CBMSGBOX_ID);
	else if (pSearchCon->folderId == MSG_IOSBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.FOLDER_ID > 0 AND A.FOLDER_ID < %d ", MSG_DRAFT_ID);
	else
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND A.FOLDER_ID = %d ", pSearchCon->folderId);

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	//// msg type
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	switch (pSearchCon->msgType) {
		case MSG_TYPE_SMS:
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

	/// string
	if (pSearchCon->pSearchVal != NULL) {

		// Replace string for '%' and '_' character
		ext1_str = MsgStoReplaceString(pSearchCon->pSearchVal, "_", "\\_");
		ext2_str = MsgStoReplaceString(ext1_str, "%", "\\%");

		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND ( A.MSG_TEXT LIKE '%%%s%%' ESCAPE '\\' \
				OR A.SUBJECT LIKE '%%%s%%' ESCAPE '\\' \
				OR B.ADDRESS_VAL LIKE '%%%s%%' ESCAPE '\\' \
				OR B.DISPLAY_NAME LIKE '%%%s%%' ESCAPE '\\' \
				OR B.FIRST_NAME LIKE '%%%s%%' ESCAPE '\\' \
				OR B.LAST_NAME LIKE '%%%s%%' ESCAPE '\\') ",
				ext2_str, ext2_str, ext2_str, ext2_str, ext2_str, ext2_str);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

		if (ext1_str) {
			free(ext1_str);
			ext1_str = NULL;
		}

		if (ext2_str) {
			free(ext2_str);
			ext2_str = NULL;
		}
	}

	/// address
	if (pSearchCon->pAddressVal != NULL) {

		// Replace string for '%' and '_' character
		ext1_str = MsgStoReplaceString(pSearchCon->pAddressVal, "_", "\\_");
		ext2_str = MsgStoReplaceString(ext1_str, "%", "\\%");

		memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND B.ADDRESS_VAL LIKE '%%%s%%' ESCAPE '\\' ", ext2_str);
		strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

		if (ext1_str) {
			free(ext1_str);
			ext1_str = NULL;
		}

		if (ext2_str) {
			free(ext2_str);
			ext2_str = NULL;
		}
	}

	/// limit, offset
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (offset >= 0 && limit > 0)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.DISPLAY_TIME DESC LIMIT %d OFFSET %d;", limit, offset);
	else
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY A.DISPLAY_TIME DESC;");

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

	msg_error_t err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Get table fail. [%s]", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pMsgList->nCount = rowCnt;

	MSG_DEBUG("pMsgList->nCount [%d]", pMsgList->nCount);

	pMsgList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * rowCnt];

	MSG_MESSAGE_HIDDEN_S *pTmp = NULL;
	msg_struct_s *msg = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pMsgList->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;

		msg = (msg_struct_s *)pMsgList->msg_struct_info[i];

		msg->type = MSG_STRUCT_MESSAGE_INFO;
		msg->data = (int *)new char[sizeof(MSG_MESSAGE_HIDDEN_S)];

		pTmp = (MSG_MESSAGE_HIDDEN_S *)msg->data;

		memset(pTmp, 0x00, sizeof(MSG_MESSAGE_HIDDEN_S));

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		pTmp->threadId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->mainType = dbHandle.getColumnToInt(index++);
		pTmp->subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction= dbHandle.getColumnToInt(index++);
		index++; // This field is reserved.

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			MSG_DEBUG("pTmp->dataSize [%d]", pTmp->dataSize);
			pTmp->pData = (void *)new char[pTmp->dataSize + 2];
			memset(pTmp->pData, 0x00, sizeof(pTmp->pData));

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;
		msg_struct_s *addr_info = NULL;
		MSG_ADDRESS_INFO_S *address = NULL;

		addr_list->nCount = 1;
		addr_list->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t *)*MAX_TO_ADDRESS_CNT];

		msg_struct_s *pTmpAddr = NULL;

		for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
			addr_list->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
			pTmpAddr = (msg_struct_s *)addr_list->msg_struct_info[i];
			pTmpAddr->type = MSG_STRUCT_ADDRESS_INFO;
			pTmpAddr->data = new MSG_ADDRESS_INFO_S;
			memset(pTmpAddr->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

			addr_list->msg_struct_info[i] = (msg_struct_t)pTmpAddr;
		}

		addr_info = (msg_struct_s *)addr_list->msg_struct_info[0];
		address = (MSG_ADDRESS_INFO_S *)addr_info->data;
		address->addressType = dbHandle.getColumnToInt(index++);
		address->recipientType = dbHandle.getColumnToInt(index++);
		address->contactId = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_ADDRESS_VAL_LEN, address->addressVal);

		memset(displayName, 0x00, sizeof(displayName));
		dbHandle.getColumnToString(index++, MAX_DISPLAY_NAME_LEN, displayName);

		memset(firstName, 0x00, sizeof(firstName));
		dbHandle.getColumnToString(index++, MAX_DISPLAY_NAME_LEN, firstName);

		memset(lastName, 0x00, sizeof(lastName));
		dbHandle.getColumnToString(index++, MAX_DISPLAY_NAME_LEN, lastName);

		if (strlen(displayName) <= 0) {
			if (order == 0) {
				if (firstName[0] != '\0') {
					strncpy(displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (lastName[0] != '\0') {
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
					strncat(displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			} else if (order == 1) {
				if (lastName[0] != '\0') {
					strncpy(displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}

				if (firstName[0] != '\0') {
					strncat(displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			}
		}

		strncpy(address->displayName, displayName, MAX_DISPLAY_NAME_LEN);

		pTmp->addr_list = addr_list;

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);

	}
	dbHandle.freeTable();

	return MSG_SUCCESS;
}


void MsgConvertNumber(const char *pSrcNum, char *pDestNum)
{
	int overLen = 0;
	int i = 0;

	overLen = strlen(pSrcNum) - MAX_PRECONFIG_NUM;

	for (i = 0; i < MAX_PRECONFIG_NUM; i++)
		pDestNum[i] = pSrcNum[i+overLen];

	pDestNum[i] = '\0';
}


msg_error_t MsgStoGetRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList)
{
	// Clear Out Parameter
	pRejectMsgList->nCount = 0;
	pRejectMsgList->msg_struct_info = NULL;

	int rowCnt = 0;
	int index = 3; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];

	// Search Reject Msg
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pNumber != NULL) {
		char phoneNumber[MAX_PRECONFIG_NUM+1];
		memset(phoneNumber, 0x00, sizeof(phoneNumber));

		if (strlen(pNumber) > MAX_PRECONFIG_NUM)
			MsgConvertNumber(pNumber, phoneNumber);
		else
			strncpy(phoneNumber, pNumber, MAX_PRECONFIG_NUM);

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.MSG_ID, B.MSG_TEXT, B.DISPLAY_TIME \
				FROM %s A, %s B \
				WHERE A.CONV_ID = B.CONV_ID AND B.MAIN_TYPE = %d \
				AND B.SUB_TYPE = %d AND A.ADDRESS_VAL LIKE '%%%s' \
				ORDER BY B.DISPLAY_TIME DESC;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_SMS_TYPE, MSG_REJECT_SMS, phoneNumber);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT B.MSG_ID, B.MSG_TEXT, B.DISPLAY_TIME \
				FROM %s A, %s B \
				WHERE A.CONV_ID = B.CONV_ID AND B.MAIN_TYPE = %d AND B.SUB_TYPE = %d \
				ORDER BY B.DISPLAY_TIME DESC;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				MSG_SMS_TYPE, MSG_REJECT_SMS);
	}

	msg_error_t  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pRejectMsgList->nCount = rowCnt;

	MSG_DEBUG("pRejectMsgList->nCount [%d]", pRejectMsgList->nCount);

	pRejectMsgList->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_REJECT_MSG_INFO_S *)*rowCnt];

	msg_struct_s* pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pRejectMsgList->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];

		pTmp = (msg_struct_s *)pRejectMsgList->msg_struct_info[i];
		pTmp->type = MSG_STRUCT_REJECT_MSG_INFO;
		pTmp->data = new char[sizeof(MSG_FOLDER_INFO_S)];
		MSG_REJECT_MSG_INFO_S * pMsg = (MSG_REJECT_MSG_INFO_S *)pTmp->data;
		memset(pMsg, 0x00, sizeof(MSG_REJECT_MSG_INFO_S));

		pMsg->msgId = dbHandle.getColumnToInt(index++);
		memset(pMsg->msgText, 0x00, sizeof(pMsg->msgText));
		dbHandle.getColumnToString(index++, MAX_MSG_TEXT_LEN, pMsg->msgText);

		pMsg->displayTime = (time_t)dbHandle.getColumnToInt(index++);
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetSyncMLExtId(msg_message_id_t msgId, int *extId)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXT_ID FROM %s WHERE MSG_ID = %d;",
			MSGFW_SYNCML_MSG_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		*extId = dbHandle.columnInt(0);
	} else {
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetReportStatus(msg_message_id_t msgId, MSG_REPORT_STATUS_INFO_S *pReportStatus)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DELIVERY_REPORT_STATUS, \
			DELIVERY_REPORT_TIME, READ_REPORT_STATUS, READ_REPORT_TIME \
			FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pReportStatus->deliveryStatus = (msg_delivery_report_status_t)dbHandle.columnInt(0);
		pReportStatus->deliveryStatusTime = (time_t)dbHandle.columnInt(1);
		pReportStatus->readStatus = (msg_read_report_status_t)dbHandle.columnInt(2);
		pReportStatus->readStatusTime = (time_t)dbHandle.columnInt(3);
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetThreadIdByAddress(const MSG_MESSAGE_INFO_S *pMsg, msg_thread_id_t *pThreadId)
{
	if(pMsg->nAddressCnt > 0) {
		if (MsgExistAddress(&dbHandle, pMsg, pThreadId) == true) {
			MSG_DEBUG("Conversation ID : [%d]", *pThreadId);
		} else {
			*pThreadId = 0;
			return MSG_ERR_STORAGE_ERROR;
		}
	} else {
		*pThreadId = 0;
	}
	return MSG_SUCCESS;
}


msg_error_t MsgStoGetThreadUnreadCnt(msg_thread_id_t threadId, int *cnt)
{
	MSG_BEGIN();

	int msgCnt = 0;
	*cnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	// Get MSG_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s A \
			WHERE CONV_ID = %d AND READ_STATUS = 0;", MSGFW_MESSAGE_TABLE_NAME, threadId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgCnt = dbHandle.columnInt(0);
	}

	dbHandle.finalizeQuery();

	*cnt = msgCnt;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList)
{
	msg_error_t err = MSG_SUCCESS;

	int order = MsgGetContactNameOrder();

	err = MsgStoGetAddressByConvId(&dbHandle, threadId, order, pAddrList);

	return err;
}


msg_error_t MsgStoGetThreadInfo(msg_thread_id_t threadId, MSG_THREAD_VIEW_S *pThreadInfo)
{
	MSG_BEGIN();

	int rowCnt;
	int index = 10; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONV_ID, UNREAD_CNT, SMS_CNT, MMS_CNT, \
			MAIN_TYPE, SUB_TYPE, MSG_DIRECTION, DISPLAY_TIME, DISPLAY_NAME, MSG_TEXT \
			FROM %s WHERE CONV_ID = %d;",
			MSGFW_CONVERSATION_TABLE_NAME, threadId);

	msg_error_t  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);
		dbHandle.freeTable();
		return err;
	}

	if (rowCnt < 1) {
		MSG_DEBUG("rowCnt is %d", rowCnt);
		dbHandle.freeTable();
		return err;
	} else {
		pThreadInfo->threadId = dbHandle.getColumnToInt(index++);

		pThreadInfo->unreadCnt = dbHandle.getColumnToInt(index++);
		pThreadInfo->smsCnt = dbHandle.getColumnToInt(index++);
		pThreadInfo->mmsCnt = dbHandle.getColumnToInt(index++);

		pThreadInfo->mainType = dbHandle.getColumnToInt(index++);
		pThreadInfo->subType = dbHandle.getColumnToInt(index++);

		pThreadInfo->direction = dbHandle.getColumnToInt(index++);
		pThreadInfo->threadTime = (time_t)dbHandle.getColumnToInt(index++);

		memset(pThreadInfo->threadName, 0x00, sizeof(pThreadInfo->threadName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, pThreadInfo->threadName);

		memset(pThreadInfo->threadData, 0x00, sizeof(pThreadInfo->threadData));
		dbHandle.getColumnToString(index++, MAX_THREAD_DATA_LEN, pThreadInfo->threadData);
	}

	dbHandle.freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgStoGetMessageList(msg_folder_id_t folderId, msg_thread_id_t threadId, msg_message_type_t msgType, msg_storage_id_t storageId, msg_struct_list_s *pMsgList)
{
	// Clear Out Parameter
	pMsgList->nCount = 0;
	pMsgList->msg_struct_info = NULL;

	int rowCnt = 0;
	int index = 19; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlQuerySubset[(MAX_QUERY_LEN/5)+1];

	// Get Name Order
	int order = MsgGetContactNameOrder();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, CONV_ID, FOLDER_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
			DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, BACKUP, PRIORITY, \
			MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, THUMB_PATH \
			FROM %s WHERE ", MSGFW_MESSAGE_TABLE_NAME);


	//// folder
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (folderId == MSG_ALLBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "FOLDER_ID > 0 AND FOLDER_ID < %d ", MSG_CBMSGBOX_ID);
	else if (folderId == MSG_IOSBOX_ID)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "FOLDER_ID > 0 AND FOLDER_ID < %d ", MSG_DRAFT_ID);
	else
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "FOLDER_ID = %d ", folderId);

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	//// thread
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (threadId > 0)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND CONV_ID = %d ", threadId);

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	//// msg type
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	switch (msgType) {
		case MSG_TYPE_SMS:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND MAIN_TYPE = %d AND SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_NORMAL_SMS);
			break;

		case MSG_TYPE_MMS:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND MAIN_TYPE = %d ", MSG_MMS_TYPE);
			break;

		case MSG_TYPE_MMS_JAVA:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND MAIN_TYPE = %d AND SUB_TYPE = %d ", MSG_MMS_TYPE, MSG_SENDREQ_JAVA_MMS);
			break;

		case MSG_TYPE_SMS_SYNCML:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND MAIN_TYPE = %d AND SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_SYNCML_CP);
			break;

		case MSG_TYPE_SMS_REJECT:
			snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND MAIN_TYPE = %d AND SUB_TYPE = %d ", MSG_SMS_TYPE, MSG_REJECT_SMS);
			break;

		default:
			MSG_DEBUG("msg type is not set.");
	}

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

	//// storage
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));

	if (storageId > MSG_STORAGE_UNKNOWN)
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND STORAGE_ID = %d ", storageId);

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));


	/// order
	memset(sqlQuerySubset, 0x00, sizeof(sqlQuerySubset));
	snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "ORDER BY DISPLAY_TIME DESC;");

	strncat(sqlQuery, sqlQuerySubset, MAX_QUERY_LEN-strlen(sqlQuery));

	msg_error_t err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("Get table fail. [%s]", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pMsgList->nCount = rowCnt;

	MSG_DEBUG("pMsgList->nCount [%d]", pMsgList->nCount);

	pMsgList->msg_struct_info = (msg_struct_t *)new char[sizeof(msg_struct_t) * rowCnt];

	MSG_MESSAGE_HIDDEN_S *pTmp = NULL;
	msg_struct_s *msg = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pMsgList->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;

		msg = (msg_struct_s *)pMsgList->msg_struct_info[i];

		msg->type = MSG_STRUCT_MESSAGE_INFO;
		msg->data = (int *)new char[sizeof(MSG_MESSAGE_HIDDEN_S)];

		pTmp = (MSG_MESSAGE_HIDDEN_S *)msg->data;

		memset(pTmp, 0x00, sizeof(MSG_MESSAGE_HIDDEN_S));

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		pTmp->threadId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->mainType = dbHandle.getColumnToInt(index++);
		pTmp->subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction = dbHandle.getColumnToInt(index++);
		index++; // This field is reserved.

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			pTmp->pData = (void *)new char[pTmp->dataSize+2];
			memset(pTmp->pData, 0x00, pTmp->dataSize+2);

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);

		// add address information.
		order = MsgGetContactNameOrder();

		msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;
		MsgStoGetAddressByMsgId(&dbHandle, pTmp->msgId, order, addr_list);

		pTmp->addr_list = addr_list;
	}
	dbHandle.freeTable();

	return MSG_SUCCESS;
}
