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

static unsigned int refId = 0;

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

	MSG_MSGID_LIST_S *pMsgIdList = (MSG_MSGID_LIST_S *)pVoid;

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
MSG_ERROR_T MsgStoAddMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, int addrIdx)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int rowId = 0;
	unsigned int addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	if (pMsg->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, &(pMsg->addressList[addrIdx]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}

		pMsg->addressList[addrIdx].threadId = (MSG_THREAD_ID_T)addrId;
	}

	err = dbHandle.getRowId(MSGFW_MESSAGE_TABLE_NAME, &rowId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	pMsg->msgId = (MSG_MESSAGE_ID_T)rowId;

	if (addrIdx == 0)
		refId = pMsg->msgId;

	pMsg->referenceId = refId;

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

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %ld, %d, ?, ?, ?, ?, %d, 0, %d, 0, 0);",
				MSGFW_MESSAGE_TABLE_NAME, rowId, addrId, pMsg->folderId, pMsg->referenceId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType,
				pMsg->displayTime, pMsg->dataSize, pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction,
				pMsg->scheduledTime, pMsg->bBackup, MSG_DELIVERY_REPORT_NONE, MSG_READ_REPORT_NONE);

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
		err = MsgStoUpdateAddress(&dbHandle, addrId);

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


MSG_ERROR_T MsgStoUpdateMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, int addrIdx)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int addrId = 0;

	dbHandle.beginTrans();

	if (pMsg->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, &(pMsg->addressList[addrIdx]), &addrId);

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

				pSendOptInfo->option.mmsSendOptInfo.priority = (MSG_PRIORITY_TYPE_T)MsgSettingGetInt(MMS_SEND_PRIORITY);
			}
		}
	}

	// Update Message
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET ADDRESS_ID = %d, FOLDER_ID = %d, STORAGE_ID = %d, MAIN_TYPE = %d, SUB_TYPE = %d, \
								DISPLAY_TIME = %lu, DATA_SIZE = %d, NETWORK_STATUS = %d, READ_STATUS = %d, \
								PROTECTED = %d, PRIORITY = %d, MSG_DIRECTION = %d, SCHEDULED_TIME = %lu, BACKUP = %d, SUBJECT = ?, MSG_DATA = ?, THUMB_PATH = ?, MSG_TEXT = ? \
								WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, addrId, pMsg->folderId, pMsg->storageId, pMsg->msgType.mainType, pMsg->msgType.subType, pMsg->displayTime, pMsg->dataSize,
			pMsg->networkStatus, pMsg->bRead, pMsg->bProtected, pMsg->priority, pMsg->direction, pMsg->scheduledTime, pMsg->bBackup, pMsg->msgId);

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
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET DELREP_REQ = %d, KEEP_COPY = %d, REPLY_PATH = %d \
										WHERE MSG_ID = %d;",
					MSGFW_SMS_SENDOPT_TABLE_NAME, pSendOptInfo->bDeliverReq, pSendOptInfo->bKeepCopy, pSendOptInfo->option.smsSendOptInfo.bReplyPath, pMsg->msgId);

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

	err = MsgStoUpdateAddress(&dbHandle, addrId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	err = MsgStoClearAddressTable(&dbHandle);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoUpdateReadStatus(MSG_MESSAGE_ID_T MsgId, bool bRead)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	MSG_STORAGE_ID_T storageId;

	if (MsgStoSetReadStatus(&dbHandle, MsgId, bRead) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoUpdateAddress() Error");
		return MSG_ERR_STORAGE_ERROR;
	}

	// Get STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT STORAGE_ID \
				        FROM %s \
				     WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MsgId);

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
						MSGFW_SIM_MSG_TABLE_NAME, MsgId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		MSG_SIM_ID_T simId;

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


MSG_ERROR_T MsgStoUpdateThreadReadStatus(MSG_THREAD_ID_T ThreadId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	int rowCnt = 0;
	int index = 1;
	MSG_MSGID_LIST_S *pUnreadMsgIdList = NULL;

	pUnreadMsgIdList = (MSG_MSGID_LIST_S *)new char[sizeof(MSG_MSGID_LIST_S)];
	memset(pUnreadMsgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	char sqlQuery[MAX_QUERY_LEN+1];

	// Get MSG_ID List
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID \
						FROM %s A \
						WHERE ADDRESS_ID = %d AND READ_STATUS = 0;",
					MSGFW_MESSAGE_TABLE_NAME, ThreadId);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return err;
	}

	pUnreadMsgIdList->nCount = rowCnt;

	MSG_DEBUG("unreadMsgIdList.nCount [%d]", pUnreadMsgIdList->nCount);

	pUnreadMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pUnreadMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();


	// Get sim MSG_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID \
				        FROM %s A \
				     WHERE ADDRESS_ID = %d AND READ_STATUS = 0 AND STORAGE_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, ThreadId, MSG_STORAGE_SIM);

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
					WHERE ADDRESS_ID = %d AND READ_STATUS = 0;",
					MSGFW_MESSAGE_TABLE_NAME, 1, ThreadId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;


	if (MsgStoUpdateAddress(&dbHandle, ThreadId) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoUpdateAddress() Error");
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


MSG_ERROR_T MsgStoUpdateProtectedStatus(MSG_MESSAGE_ID_T MsgId, bool bProtected)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET PROTECTED = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, (int)bProtected, MsgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDeleteMessage(MSG_MESSAGE_ID_T MsgId, bool bCheckIndication)
{
	MSG_BEGIN();

	MSG_DEBUG("Msg Id : %d", MsgId);

	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	// Get SUB_TYPE, STORAGE_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MAIN_TYPE, A.SUB_TYPE, A.FOLDER_ID, A.STORAGE_ID, A.READ_STATUS, B.CONTACT_ID, B.ADDRESS_ID \
				        FROM %s A, %s B \
				     WHERE A.MSG_ID = %d AND A.ADDRESS_ID = B.ADDRESS_ID;",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	MSG_MESSAGE_TYPE_S msgType;
	MSG_FOLDER_ID_T folderId;
	MSG_STORAGE_ID_T storageId;
	MSG_CONTACT_ID_T contactId;

	bool bRead;
	unsigned int addrId;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = dbHandle.columnInt(0);
		msgType.subType = dbHandle.columnInt(1);
		folderId = dbHandle.columnInt(2);
		storageId = dbHandle.columnInt(3);
		bRead = dbHandle.columnInt(4);
		contactId = dbHandle.columnInt(5);
		addrId = dbHandle.columnInt(6);

		MSG_DEBUG("Main Type:[%d] SubType:[%d] FolderId:[%d] StorageId:[%d] ContactId:[%d] AddressId:[%d]", msgType.mainType, msgType.subType, folderId, storageId, contactId, addrId);
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
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_ID FROM %s WHERE MSG_ID = %d;", MSGFW_SIM_MSG_TABLE_NAME, MsgId);

		MSG_DEBUG("sqlQuery is [%s]", sqlQuery);

		MSG_SIM_ID_T simMsgId;

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

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SCHEDULED_MSG_TABLE_NAME, MsgId);

	// Delete Message from scheduled msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (msgType.mainType == MSG_SMS_TYPE) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SMS_SENDOPT_TABLE_NAME, MsgId);

		// Delete SMS Send Option
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}

		if (msgType.subType == MSG_CB_SMS || msgType.subType == MSG_JAVACB_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_CB_MSG_TABLE_NAME, MsgId);

			// Delete Push Message from push table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType >= MSG_WAP_SI_SMS && msgType.subType <= MSG_WAP_CO_SMS) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_PUSH_MSG_TABLE_NAME, MsgId);

			// Delete Push Message from push table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		} else if (msgType.subType == MSG_SYNCML_CP) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SYNCML_MSG_TABLE_NAME, MsgId);

			// Delete SyncML Message from syncML table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else if (msgType.mainType == MSG_MMS_TYPE) {
		//get multiple recipient message count
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE REFERENCE_ID IN \
									(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
						MSGFW_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MsgId);

		int rowCnt = 0;

		err = dbHandle.getTable(sqlQuery, &rowCnt);

		if (err != MSG_SUCCESS) {
			dbHandle.freeTable();
			return err;
		}

		dbHandle.freeTable();
		MSG_DEBUG("rowCnt = %d", rowCnt);

		if (rowCnt == 1) {
			char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
			char thumbnailpath[MSG_FILEPATH_LEN_MAX] = {0,};
			char dirPath[MSG_FILEPATH_LEN_MAX]= {0,};

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE REFERENCE_ID IN \
									(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
								MMS_PLUGIN_MESSAGE_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MsgId);

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

			const char *tableList[] = {MMS_PLUGIN_MESSAGE_TABLE_NAME, MMS_PLUGIN_ATTRIBUTE_TABLE_NAME};

			int listCnt = sizeof(tableList)/sizeof(char *);

			for (int i = 0; i < listCnt; i++) {
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE REFERENCE_ID IN \
									(SELECT REFERENCE_ID FROM %s WHERE MSG_ID = %d);",
										tableList[i], MSGFW_MESSAGE_TABLE_NAME, MsgId);

				// Delete Data from MMS table
				if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.endTrans(false);
					return MSG_ERR_DB_EXEC;
				}
			}
		} else {
			MSG_DEBUG("There is more multi-recipient mms. MMS DB table is not destroyed");
		}
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, MsgId);

	// Delete Message from msg table
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Clear Address table
	if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	// Update Address Info.
	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
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

		MsgDeleteNotiByMsgId(MsgId);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDeleteAllMessageInFolder(MSG_FOLDER_ID_T FolderId, bool bOnlyDB, MSG_MSGID_LIST_S *pMsgIdList)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	queue<MSG_THREAD_ID_T> threadList;

	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
						MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SCHEDULED_MSG_TABLE_NAME,
						MSGFW_SMS_SENDOPT_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME,
						MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char *);
	int rowCnt = 0;

	// Get Address ID in Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DISTINCT(B.ADDRESS_ID) \
					FROM %s A, %s B \
				     WHERE A.ADDRESS_ID = B.ADDRESS_ID \
				          AND A.FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, FolderId);

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
		threadList.push((MSG_THREAD_ID_T)dbHandle.getColumnToInt(i));
	}

	dbHandle.freeTable();

	/*** Get msg id list **/
	MSG_MSGID_LIST_S *pToDeleteMsgIdList = NULL;
	pToDeleteMsgIdList = (MSG_MSGID_LIST_S *)new char[sizeof(MSG_MSGID_LIST_S)];
	memset(pToDeleteMsgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, FolderId);

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

	pToDeleteMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pToDeleteMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();
	/*** **/

	/*** Delete Sim Message In Folder **/
	if (FolderId >= MSG_INBOX_ID) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s \
					     WHERE FOLDER_ID = %d AND STORAGE_ID = %d",
				MSGFW_MESSAGE_TABLE_NAME, FolderId, MSG_STORAGE_SIM);

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
		}

		dbHandle.freeTable();
	}

	dbHandle.beginTrans();

	for (int i = 0; i < listCnt; i++) {
		if (!strcmp(tableList[i], MMS_PLUGIN_MESSAGE_TABLE_NAME)) {
			//MMS_PLUGIN_MESSAGE_TABLE and MMS_PLUGIN_ATTRIBUTE_TABLE will be updated together
			int rowCnt = 0;
			int msgCnt = 0;
			int index = 2;

			//get mms msg id list
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, REFERENCE_ID FROM %s \
								WHERE FOLDER_ID = %d AND MAIN_TYPE = %d",
								MSGFW_MESSAGE_TABLE_NAME, FolderId, MSG_MMS_TYPE);

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
				int msgId = 0;
				int refId = 0;

				msgId = dbHandle.getColumnToInt(index++);
				refId = dbHandle.getColumnToInt(index++);

				//get multiple recipient message count
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE REFERENCE_ID = %d;",
								MSGFW_MESSAGE_TABLE_NAME, refId);

				if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.freeTable();
					dbHandle.endTrans(false);
					err = MSG_ERR_DB_PREPARE;

					goto FREE_MEMORY;
				}

				msgCnt = 0;

				if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
					msgCnt = dbHandle.columnInt(0);
				} else {
					dbHandle.finalizeQuery();
					dbHandle.freeTable();
					dbHandle.endTrans(false);
					err = MSG_ERR_DB_STEP;

					goto FREE_MEMORY;
				}

				dbHandle.finalizeQuery();

				//delete mms raw file and db(mms_table and mms_attr_table) data
				if (msgCnt == 1) {
					char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
					char dirPath[MSG_FILEPATH_LEN_MAX] = {0,};
					char thumbnailPath[MSG_FILEPATH_LEN_MAX] = {0,};

					//get raw file_path
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_MESSAGE_TABLE_NAME, refId);

					MSG_DEBUG("sqlQuery [%s]", sqlQuery);
					if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_PREPARE;

						goto FREE_MEMORY;
					}

					if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
						strncpy(filePath, (char *)dbHandle.columnText(0), MSG_FILEPATH_LEN_MAX);
					} else {
						dbHandle.finalizeQuery();
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_STEP;

						goto FREE_MEMORY;
					}

					dbHandle.finalizeQuery();

					MSG_DEBUG("filePath [%s]", filePath);

					//delete raw file
					snprintf(dirPath, sizeof(dirPath), "%s.dir", filePath);

					if (remove(filePath) == -1)
						MSG_DEBUG("Fail to delete file [%s]", filePath);
					else
						MSG_DEBUG("Success to delete file [%s]", filePath);

					MsgRmRf(dirPath);
					rmdir(dirPath);
					// delete thumbanil

					char *fileName = NULL;
					fileName = strrchr(filePath, '/');

					snprintf(thumbnailPath, sizeof(thumbnailPath), MSG_THUMBNAIL_PATH"%s.jpg", fileName+1);

					if (remove(thumbnailPath) == -1)
						MSG_DEBUG("Fail to delete thumbnail [%s]", thumbnailPath);
					else
						MSG_DEBUG("Success to delete thumbnail [%s]", thumbnailPath);

					//delete MMS_PLUGIN_MESSAGE_TABLE_NAME table data
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_MESSAGE_TABLE_NAME, refId);

					if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
						MSG_DEBUG("sqlQuery [%s]", sqlQuery);
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_EXEC;

						goto FREE_MEMORY;
					}

					//delete  MMS_PLUGIN_ATTRIBUTE_TABLE_NAME table data
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, refId);

					if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
						MSG_DEBUG("sqlQuery [%s]", sqlQuery);
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_EXEC;

						goto FREE_MEMORY;
					}
				} else {
					MSG_DEBUG("There is remained multi-recipient mms for this msg = %d", msgId);
				}
			}

			dbHandle.freeTable();
		} else {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));

			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
								(SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d);",
								tableList[i], MSGFW_MESSAGE_TABLE_NAME, FolderId);

			// Delete Message in specific folder from table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				MSG_DEBUG("sqlQuery [%s]", sqlQuery);
				dbHandle.endTrans(false);
				err = MSG_ERR_DB_EXEC;

				goto FREE_MEMORY;
			}
		}
	}

	// Clear Address table
	if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		err = MSG_ERR_DB_EXEC;

		goto FREE_MEMORY;
	}

	// Update Address
	while (!threadList.empty()) {
		err = MsgStoUpdateAddress(&dbHandle, threadList.front());

		threadList.pop();

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);

			goto FREE_MEMORY;
		}
	}

	dbHandle.endTrans(true);

	if (FolderId == MSG_INBOX_ID) {
		int smsCnt = 0;
		int mmsCnt = 0;

		smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
		mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

		MsgSettingSetIndicator(smsCnt, mmsCnt);
	}

/*** Set pMsgIdList **/
	if (pMsgIdList != NULL && pToDeleteMsgIdList->nCount > 0) {
		pMsgIdList->nCount = pToDeleteMsgIdList->nCount;

		pMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T)*pToDeleteMsgIdList->nCount];
		memcpy(pMsgIdList->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(MSG_MESSAGE_ID_T)*pToDeleteMsgIdList->nCount);
	}
/*** **/

/*** Create thread  for noti delete. **/
	if (!bOnlyDB) {
		if (pToDeleteMsgIdList->nCount > 0) {

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


MSG_ERROR_T MsgStoMoveMessageToFolder(MSG_MESSAGE_ID_T MsgId, MSG_FOLDER_ID_T DestFolderId)
{
	MSG_MESSAGE_TYPE_S msgType;

	MsgStoGetMsgType(MsgId, &msgType);

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (msgType.mainType == MSG_SMS_TYPE) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, DestFolderId, MsgId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET FOLDER_ID = %d WHERE REFERENCE_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, DestFolderId, MsgId);
	}

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_EXEC;

	unsigned int addrId = 0;
	/* get address id */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID FROM %s WHERE MSG_ID = %d;",
							MSGFW_MESSAGE_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		addrId = dbHandle.columnInt(0);
	}
	MSG_DEBUG("addrId : %d",  addrId);

	dbHandle.finalizeQuery();

	/* update address table */
	MsgStoUpdateAddress(&dbHandle, addrId);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoMoveMessageToStorage(MSG_MESSAGE_ID_T MsgId, MSG_STORAGE_ID_T DestStorageId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	MSG_DEBUG("MsgId : %d, DestStorageId : %d", MsgId, DestStorageId);

	switch (DestStorageId) {
	case MSG_STORAGE_SIM : // Move message to sim card
		{
			MSG_MESSAGE_INFO_S msgInfo;
			SMS_SIM_ID_LIST_S simIdList;

			memset(&simIdList, 0x00, sizeof(SMS_SIM_ID_LIST_S));

			if ((err = MsgStoGetMessage(MsgId, &msgInfo, NULL)) != MSG_SUCCESS)
				return err;

			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

			if ((err = plg->saveSimMessage(&msgInfo, &simIdList)) != MSG_SUCCESS)
				return err;

			dbHandle.beginTrans();

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, DestStorageId, MsgId);

			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}

			for (unsigned int i = 0; i < simIdList.count; i++) {
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d);",
								MSGFW_SIM_MSG_TABLE_NAME, MsgId, simIdList.simId[i]);

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
					MSGFW_MESSAGE_TABLE_NAME, MsgId);

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
				MSG_SIM_ID_T simMsgId;

				// get sim message id
				memset(sqlQuery, 0x00, sizeof(sqlQuery));

				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT SIM_ID FROM %s WHERE MSG_ID = %d;",
					MSGFW_SIM_MSG_TABLE_NAME, MsgId);

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
							MSGFW_SIM_MSG_TABLE_NAME, MsgId);
				MSG_DEBUG("sqlQuery is %s.", sqlQuery);

				if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.endTrans(false);
					return MSG_ERR_DB_EXEC;
				}

				//Move storage id
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET STORAGE_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, DestStorageId, MsgId);

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


MSG_ERROR_T MsgStoCountMessage(MSG_FOLDER_ID_T FolderId, MSG_COUNT_INFO_S *pCountInfo)
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
								    UNION ALL SELECT COUNT(MSG_ID) FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d ;",
							MSGFW_MESSAGE_TABLE_NAME, FolderId,
							MSGFW_MESSAGE_TABLE_NAME, FolderId,
							MSGFW_MESSAGE_TABLE_NAME, FolderId, MSG_SMS_TYPE,
							MSGFW_MESSAGE_TABLE_NAME, FolderId, MSG_MMS_TYPE);

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


MSG_ERROR_T MsgStoCountMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount)
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


MSG_ERROR_T MsgStoGetMessage(MSG_MESSAGE_ID_T MsgId, MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int addrId = 0;

	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];

	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.ADDRESS_ID, A.FOLDER_ID, A.REFERENCE_ID, A.STORAGE_ID, A.MAIN_TYPE, A.SUB_TYPE, A.DISPLAY_TIME, A.DATA_SIZE, \
						  A.NETWORK_STATUS, A.READ_STATUS, A.PROTECTED, A.BACKUP, A.PRIORITY, A.MSG_DIRECTION, A.SCHEDULED_TIME, A.SUBJECT, A.MSG_TEXT, \
					          B.ADDRESS_TYPE, B.RECIPIENT_TYPE, B.CONTACT_ID, B.ADDRESS_VAL, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME, A.THUMB_PATH \
					FROM %s A, %s B WHERE A.MSG_ID = %d AND A.ADDRESS_ID = B.ADDRESS_ID;",
					MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pMsg->msgId = dbHandle.columnInt(0);
		addrId = dbHandle.columnInt(1);
		pMsg->folderId = dbHandle.columnInt(2);
		pMsg->referenceId = dbHandle.columnInt(3);
		pMsg->storageId = dbHandle.columnInt(4);
		pMsg->msgType.mainType = dbHandle.columnInt(5);
		pMsg->msgType.subType = dbHandle.columnInt(6);
		pMsg->displayTime = (time_t)dbHandle.columnInt(7);
		pMsg->dataSize = dbHandle.columnInt(8);
		pMsg->networkStatus = dbHandle.columnInt(9);
		pMsg->bRead = dbHandle.columnInt(10);
		pMsg->bProtected = dbHandle.columnInt(11);
		pMsg->bBackup = dbHandle.columnInt(12);
		pMsg->priority = dbHandle.columnInt(13);
		pMsg->direction= dbHandle.columnInt(14);
		pMsg->scheduledTime = (time_t)dbHandle.columnInt(15);

		strncpy(pMsg->subject, (char *)dbHandle.columnText(16), MAX_SUBJECT_LEN);

		/* Temp_File_Handling */
		if (pMsg->msgType.mainType == MSG_SMS_TYPE || pMsg->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
			if (pMsg->dataSize > MAX_MSG_TEXT_LEN) {
				char msgData[pMsg->dataSize+1];
				memset(msgData, 0x00, sizeof(msgData));

				strncpy(msgData, (char *)dbHandle.columnText(17), pMsg->dataSize);

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
				strncpy(pMsg->msgText, (char *)dbHandle.columnText(17), pMsg->dataSize);

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
			if (dbHandle.columnText(17) != NULL)
				strncpy(pMsg->msgText, (char *)dbHandle.columnText(17), MAX_MSG_TEXT_LEN);
		}

		if (addrId > 0)
			pMsg->nAddressCnt = 1;
		else
			pMsg->nAddressCnt = 0;

		pMsg->addressList[0].threadId = addrId;
		pMsg->addressList[0].addressType = dbHandle.columnInt(18);
		pMsg->addressList[0].recipientType = dbHandle.columnInt(19);
		pMsg->addressList[0].contactId = dbHandle.columnInt(20);

		if (dbHandle.columnText(21) != NULL)
			strncpy(pMsg->addressList[0].addressVal, (char *)dbHandle.columnText(21), MAX_ADDRESS_VAL_LEN);

		if (dbHandle.columnText(22) != NULL && ((char *)dbHandle.columnText(22))[0]!='\0') {
			MSG_DEBUG("displayName  : [%s]", dbHandle.columnText(22));
			strncpy(pMsg->addressList[0].displayName, (char *)dbHandle.columnText(22), MAX_DISPLAY_NAME_LEN);
		} else {
			if (dbHandle.columnText(23) != NULL)
				strncpy(firstName, (char *)dbHandle.columnText(23), MAX_DISPLAY_NAME_LEN);

			if (dbHandle.columnText(24) != NULL)
				strncpy(lastName, (char *)dbHandle.columnText(24), MAX_DISPLAY_NAME_LEN);

			int order = MsgGetContactNameOrder();

			if (order == 0) {
				if (strlen(firstName) > 0) {
					strncpy(pMsg->addressList[0].displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (strlen(lastName) > 0) {
					strncat(pMsg->addressList[0].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
					strncat(pMsg->addressList[0].displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
				}
			} else if (order == 1) {
				if (strlen(lastName) > 0) {
					strncpy(pMsg->addressList[0].displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(pMsg->addressList[0].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
				}

				if (strlen(firstName) > 0) {
					strncat(pMsg->addressList[0].displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
				}
			}
		}

		// thumbnail path
		if (dbHandle.columnText(25)!=NULL && ((char *)dbHandle.columnText(25))[0] != '\0') {
			strncpy(pMsg->thumbPath, (char *)dbHandle.columnText(25), MSG_FILEPATH_LEN_MAX);
			MSG_DEBUG("pMsg->thumbPath : [%s]", pMsg->thumbPath);
		}
	} else {
		dbHandle.finalizeQuery();
		MSG_DEBUG("%s", sqlQuery);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	// Get MMS body if it is MMS.
	if ((pMsg->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS && (pMsg->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsg->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS)) ||
		pMsg->msgType.subType == MSG_SENDREQ_MMS ||
		pMsg->msgType.subType == MSG_SENDCONF_MMS) {
		MSG_ERROR_T err = MSG_SUCCESS;
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
	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pSendOptInfo != NULL) {
		MsgStoGetSmsSendOpt(pMsg->msgId, pSendOptInfo);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetFolderViewList(MSG_FOLDER_ID_T FolderId, const MSG_SORT_RULE_S *pSortRule, MSG_LIST_S *pMsgFolderViewList)
{
	if (pMsgFolderViewList == NULL) {
		MSG_DEBUG("pMsgFolderViewList is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	int rowCnt = 0;
	int index = 27; // numbers of index

	unsigned int addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlSort[64];
	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	// Get Name Order
	int order = MsgGetContactNameOrder();

	// Get Message In Folder
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (FolderId == MSG_ALLBOX_ID) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.ADDRESS_ID, A.FOLDER_ID, A.REFERENCE_ID, A.STORAGE_ID, A.MAIN_TYPE, A.SUB_TYPE, \
							A.DISPLAY_TIME, A.DATA_SIZE, A.NETWORK_STATUS, A.READ_STATUS, A.PROTECTED, A.BACKUP, A.PRIORITY, \
							A.MSG_DIRECTION, A.SCHEDULED_TIME, A.SUBJECT, A.MSG_TEXT, B.ADDRESS_TYPE, B.RECIPIENT_TYPE, \
							B.CONTACT_ID, B.ADDRESS_VAL, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME, A.ATTACHMENT_COUNT, A.THUMB_PATH \
								FROM %s A, %s B \
							     WHERE A.ADDRESS_ID = B.ADDRESS_ID \
								  AND A.FOLDER_ID < %d ",
						MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, MSG_CBMSGBOX_ID);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.ADDRESS_ID, A.FOLDER_ID, A.REFERENCE_ID, A.STORAGE_ID, A.MAIN_TYPE, A.SUB_TYPE, \
							A.DISPLAY_TIME, A.DATA_SIZE, A.NETWORK_STATUS, A.READ_STATUS, A.PROTECTED, A.BACKUP, A.PRIORITY, \
							A.MSG_DIRECTION, A.SCHEDULED_TIME, A.SUBJECT, A.MSG_TEXT, B.ADDRESS_TYPE, B.RECIPIENT_TYPE, \
							B.CONTACT_ID, B.ADDRESS_VAL, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME, A.ATTACHMENT_COUNT, A.THUMB_PATH \
								FROM %s A, %s B \
							     WHERE A.ADDRESS_ID = B.ADDRESS_ID \
								  AND A.FOLDER_ID = %d ",
						MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, FolderId);
	}

	memset(sqlSort, 0x00, sizeof(sqlSort));
	MsgMakeSortRule(pSortRule, sqlSort);
	strncat(sqlQuery, sqlSort, strlen(sqlSort));

	MSG_ERROR_T  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		pMsgFolderViewList->nCount = 0;
		pMsgFolderViewList->msgInfo = NULL;

		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("sqlQuery is - %s", sqlQuery);
		dbHandle.freeTable();
		return err;
	}

	pMsgFolderViewList->nCount = rowCnt;

	MSG_DEBUG("pMsgCommInfoList->nCount [%d]", pMsgFolderViewList->nCount);

	pMsgFolderViewList->msgInfo = (msg_message_t *)new char[sizeof(MSG_MESSAGE_S *) * rowCnt];

	MSG_MESSAGE_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pMsgFolderViewList->msgInfo[i] = (msg_message_t)new char[sizeof(MSG_MESSAGE_S)];

		pTmp = (MSG_MESSAGE_S *)pMsgFolderViewList->msgInfo[i];

		memset(pTmp, 0x00, sizeof(pTmp));

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		addrId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->referenceId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->msgType.mainType = dbHandle.getColumnToInt(index++);
		pTmp->msgType.subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction= dbHandle.getColumnToInt(index++);
		pTmp->scheduledTime = (time_t)dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->msgType.mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->msgType.subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			pTmp->pData = (void *)new char[pTmp->dataSize + 2];
			memset(pTmp->pData, 0x00, sizeof(pTmp->pData));

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		if (addrId > 0)
			pTmp->nAddressCnt = 1;
		else
			pTmp->nAddressCnt = 0;

		pTmp->addressList[0].threadId = addrId;
		pTmp->addressList[0].addressType = dbHandle.getColumnToInt(index++);
		pTmp->addressList[0].recipientType = dbHandle.getColumnToInt(index++);
		pTmp->addressList[0].contactId = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pTmp->addressList[0].addressVal);

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

		strncpy(pTmp->addressList[0].displayName, displayName, MAX_DISPLAY_NAME_LEN);

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoAddSyncMLMessage(MSG_MESSAGE_INFO_S *pMsgInfo, int ExtId, int PinCode)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int rowId = 0;
	unsigned int addrId = 0;

	dbHandle.beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, &(pMsgInfo->addressList[0]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	// Add Message Table
	rowId = MsgStoAddMessageTable(&dbHandle, pMsgInfo, addrId);

	if (rowId <= 0) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d);",
					MSGFW_SYNCML_MSG_TABLE_NAME, rowId, ExtId, PinCode);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	pMsgInfo->msgId = (MSG_MESSAGE_ID_T)rowId;
	pMsgInfo->referenceId = (MSG_REFERENCE_ID_T)rowId;

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


MSG_ERROR_T MsgStoGetMsgType(MSG_MESSAGE_ID_T msgId, MSG_MESSAGE_TYPE_S *pMsgType)
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


MSG_ERROR_T MsgStoGetQuickPanelData(MSG_QUICKPANEL_TYPE_T Type, MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_ERROR_T	err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (Type == MSG_QUICKPANEL_SMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_SMS_TYPE, MSG_NORMAL_SMS);
	} else if (Type == MSG_QUICKPANEL_MMS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE IN (%d, %d) AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_MMS_TYPE, MSG_RETRIEVE_AUTOCONF_MMS, MSG_RETRIEVE_MANUALCONF_MMS);
	} else if (Type == MSG_QUICKPANEL_DELIVER_REP) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE SUB_TYPE IN (%d, %d) AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_STATUS_REPORT_SMS, MSG_DELIVERYIND_MMS);
	} else if (Type == MSG_QUICKPANEL_READ_REP) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_MMS_TYPE, MSG_READORGIND_MMS);
	} else if (Type == MSG_QUICKPANEL_VOICEMAIL) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MWI_VOICE_SMS);
	} else if (Type == MSG_QUICKPANEL_MMS_NOTI) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE FOLDER_ID = %d AND MAIN_TYPE = %d AND SUB_TYPE = %d AND READ_STATUS = 0 ORDER BY DISPLAY_TIME DESC;",
			MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_MMS_TYPE, MSG_NOTIFICATIONIND_MMS);
	}

	// Get Message ID
	MSG_MESSAGE_ID_T msgId;

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


MSG_ERROR_T MsgStoGetThreadViewList(const MSG_SORT_RULE_S *pSortRule, MSG_THREAD_VIEW_LIST_S *pThreadViewList)
{
	pThreadViewList->nCount = 0;
	pThreadViewList->msgThreadInfo = NULL;

	int rowCnt = 0;
	int index = 15; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlSort[64];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID, CONTACT_ID, ADDRESS_VAL, \
			DISPLAY_NAME, FIRST_NAME, LAST_NAME, IMAGE_PATH, \
			UNREAD_CNT, SMS_CNT, MMS_CNT, MAIN_TYPE, SUB_TYPE, \
			MSG_DIRECTION, MSG_TIME, MSG_TEXT \
			FROM %s \
			WHERE ADDRESS_ID <> 0 AND (SMS_CNT > 0 OR MMS_CNT > 0) ",
			MSGFW_ADDRESS_TABLE_NAME);

	memset(sqlSort, 0x00, sizeof(sqlSort));
	MsgMakeSortRule(pSortRule, sqlSort);
	strncat(sqlQuery, sqlSort, sizeof(sqlQuery) - strlen(sqlQuery) - 1);

	MSG_ERROR_T  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	char firstName[MAX_THREAD_NAME_LEN+1];
	char lastName[MAX_THREAD_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];

	int order = MsgGetContactNameOrder();

	pThreadViewList->nCount = rowCnt;

	MSG_DEBUG("pThreadViewList->nCount [%d]", pThreadViewList->nCount);

	pThreadViewList->msgThreadInfo = (msg_thread_view_t *)new char[sizeof(MSG_THREAD_VIEW_S *)*rowCnt];

	MSG_THREAD_VIEW_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pThreadViewList->msgThreadInfo[i] = (msg_thread_view_t)new char[sizeof(MSG_THREAD_VIEW_S)];

		pTmp = (MSG_THREAD_VIEW_S *)pThreadViewList->msgThreadInfo[i];

		pTmp->threadId = dbHandle.getColumnToInt(index++);
		pTmp->contactId = dbHandle.getColumnToInt(index++);

		memset(pTmp->threadAddr, 0x00, sizeof(pTmp->threadAddr));
		dbHandle.getColumnToString(index++, MAX_THREAD_ADDR_LEN, pTmp->threadAddr);

		memset(displayName, 0x00, sizeof(displayName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, displayName);

		memset(firstName, 0x00, sizeof(firstName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, firstName);

		memset(lastName, 0x00, sizeof(lastName));
		dbHandle.getColumnToString(index++, MAX_THREAD_NAME_LEN, lastName);

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

		memset(pTmp->threadName, 0x00, sizeof(pTmp->threadName));
		strncpy(pTmp->threadName, displayName, MAX_THREAD_NAME_LEN);

		memset(pTmp->threadImagePath, 0x00, sizeof(pTmp->threadImagePath));
		dbHandle.getColumnToString(index++, MAX_IMAGE_PATH_LEN, pTmp->threadImagePath);

		pTmp->unreadCnt = dbHandle.getColumnToInt(index++);
		pTmp->smsCnt = dbHandle.getColumnToInt(index++);
		pTmp->mmsCnt = dbHandle.getColumnToInt(index++);

		pTmp->threadType.mainType = dbHandle.getColumnToInt(index++);
		pTmp->threadType.subType = dbHandle.getColumnToInt(index++);

		pTmp->direction = dbHandle.getColumnToInt(index++);
		pTmp->threadTime = (time_t)dbHandle.getColumnToInt(index++);

		memset(pTmp->threadData, 0x00, sizeof(pTmp->threadData));
		dbHandle.getColumnToString(index++, MAX_THREAD_DATA_LEN, pTmp->threadData);
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetConversationViewList(MSG_THREAD_ID_T ThreadId, MSG_LIST_S *pConvViewList)
{
	MSG_BEGIN();

	pConvViewList->nCount = 0;
	pConvViewList->msgInfo = NULL;

	int rowCnt = 0;
	int index = 20; /** numbers of index */

	unsigned int addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, ADDRESS_ID, FOLDER_ID, REFERENCE_ID, STORAGE_ID, MAIN_TYPE, SUB_TYPE, \
							DISPLAY_TIME, DATA_SIZE, NETWORK_STATUS, READ_STATUS, PROTECTED, BACKUP, PRIORITY, \
							MSG_DIRECTION, SCHEDULED_TIME, SUBJECT, MSG_TEXT, ATTACHMENT_COUNT, THUMB_PATH \
							FROM %s \
							WHERE ADDRESS_ID = %d AND FOLDER_ID > %d AND FOLDER_ID < %d \
							ORDER BY DISPLAY_TIME, MSG_ID ASC;",
					MSGFW_MESSAGE_TABLE_NAME, ThreadId, MSG_ALLBOX_ID, MSG_CBMSGBOX_ID);

	MSG_ERROR_T err = dbHandle.getTable(sqlQuery, &rowCnt);

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

	pConvViewList->msgInfo = (msg_message_t *)new char[sizeof(MSG_MESSAGE_S *) * rowCnt];

	MSG_MESSAGE_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pConvViewList->msgInfo[i] = (msg_message_t)new char[sizeof(MSG_MESSAGE_S)];

		pTmp = (MSG_MESSAGE_S *)pConvViewList->msgInfo[i];

		memset(pTmp, 0x00, sizeof(pTmp));

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		addrId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->referenceId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->msgType.mainType = dbHandle.getColumnToInt(index++);
		pTmp->msgType.subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction = dbHandle.getColumnToInt(index++);
		pTmp->scheduledTime = (time_t)dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->msgType.mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->msgType.subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			pTmp->pData = (void *)new char[pTmp->dataSize+2];
			memset(pTmp->pData, 0x00, pTmp->dataSize+2);

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);

	}

	dbHandle.freeTable();

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoDeleteThreadMessageList(MSG_THREAD_ID_T ThreadId, MSG_MSGID_LIST_S *pMsgIdList)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];
	/*** Get msg id list **/
	MSG_MSGID_LIST_S *pToDeleteMsgIdList = NULL;

	int rowCnt = 0;
	int index = 1;
	// Set Indicator
	int smsCnt = 0;
	int mmsCnt = 0;

	const char *tableList[] = {MSGFW_PUSH_MSG_TABLE_NAME, MSGFW_CB_MSG_TABLE_NAME,
					MSGFW_SYNCML_MSG_TABLE_NAME, MSGFW_SCHEDULED_MSG_TABLE_NAME,
					MSGFW_SMS_SENDOPT_TABLE_NAME, MMS_PLUGIN_MESSAGE_TABLE_NAME,
					MSGFW_MESSAGE_TABLE_NAME};

	int listCnt = sizeof(tableList)/sizeof(char *);

	pToDeleteMsgIdList = (MSG_MSGID_LIST_S *)new char[sizeof(MSG_MSGID_LIST_S)];
	memset(pToDeleteMsgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE ADDRESS_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, ThreadId);

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

	pToDeleteMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T) * rowCnt];

	for (int i = 0; i < rowCnt; i++)
		pToDeleteMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);

	dbHandle.freeTable();
	/*** **/

	/*** Delete Sim Message **/
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID \
					FROM %s \
				     WHERE ADDRESS_ID = %d \
				     	  AND STORAGE_ID = %d",
			MSGFW_MESSAGE_TABLE_NAME, ThreadId, MSG_STORAGE_SIM);

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
			//MMS_PLUGIN_MESSAGE_TABLE_NAME and MMS_PLUGIN_ATTRIBUTE_TABLE_NAME will be updated together
			int rowCnt = 0;
			int msgCnt = 0;
			int index = 2;

			//get mms msg id list
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID, REFERENCE_ID FROM %s \
								WHERE ADDRESS_ID = %d AND MAIN_TYPE = %d",
								MSGFW_MESSAGE_TABLE_NAME, ThreadId, MSG_MMS_TYPE);

			err = dbHandle.getTable(sqlQuery, &rowCnt);
			MSG_DEBUG("rowCnt %d", rowCnt);

			if (err != MSG_SUCCESS && err != MSG_ERR_DB_NORECORD) {
				MSG_DEBUG("sqlQuery [%s]", sqlQuery);

				dbHandle.freeTable();
				dbHandle.endTrans(false);

				goto FREE_MEMORY;
			}

			for (int i = 1; i <= rowCnt; i++) {
				int msgId = 0;
				int refId = 0;

				msgId = dbHandle.getColumnToInt(index++);
				refId = dbHandle.getColumnToInt(index++);

				//get multiple recipient message count
				memset(sqlQuery, 0x00, sizeof(sqlQuery));
				snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s WHERE REFERENCE_ID = %d;",
								MSGFW_MESSAGE_TABLE_NAME, refId);

				if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
					dbHandle.freeTable();
					dbHandle.endTrans(false);
					err = MSG_ERR_DB_PREPARE;

					goto FREE_MEMORY;
				}

				msgCnt = 0;

				if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
					msgCnt = dbHandle.columnInt(0);
				} else {
					dbHandle.finalizeQuery();
					dbHandle.freeTable();
					dbHandle.endTrans(false);
					err = MSG_ERR_DB_STEP;

					goto FREE_MEMORY;
				}

				dbHandle.finalizeQuery();

				//delete mms raw file and db(mms_table and mms_attr_table) data
				if (msgCnt == 1) {
					char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
					char dirPath[MSG_FILEPATH_LEN_MAX] = {0,};
					char thumbnailPath[MSG_FILEPATH_LEN_MAX] = {0,};

					//get raw file_path
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILE_PATH FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_MESSAGE_TABLE_NAME, refId);

					if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_PREPARE;

						goto FREE_MEMORY;
					}

					if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
						strncpy(filePath, (char *)dbHandle.columnText(0), MSG_FILEPATH_LEN_MAX);
					} else {
						dbHandle.finalizeQuery();
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_STEP;

						goto FREE_MEMORY;
					}

					dbHandle.finalizeQuery();

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

					//delete MMS_PLUGIN_MESSAGE_TABLE_NAME table data
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_MESSAGE_TABLE_NAME, refId);

					if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
						MSG_DEBUG("sqlQuery [%s]", sqlQuery);
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_EXEC;

						goto FREE_MEMORY;
					}

					//delete  MMS_PLUGIN_ATTRIBUTE_TABLE_NAME table data
					memset(sqlQuery, 0x00, sizeof(sqlQuery));
					snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE REFERENCE_ID = %d;",
										MMS_PLUGIN_ATTRIBUTE_TABLE_NAME, refId);

					if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
						MSG_DEBUG("sqlQuery [%s]", sqlQuery);
						dbHandle.freeTable();
						dbHandle.endTrans(false);
						err = MSG_ERR_DB_EXEC;

						goto FREE_MEMORY;
					}
				} else {
					MSG_DEBUG("There is remained multi-recipient mms for this msg = %d", msgId);
				}
			}

			dbHandle.freeTable();
		} else {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID IN \
									(SELECT MSG_ID FROM %s WHERE ADDRESS_ID = %d);",
									tableList[i], MSGFW_MESSAGE_TABLE_NAME, ThreadId);

			// Delete Message in specific folder from table
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				MSG_DEBUG("sqlQuery [%s]", sqlQuery);
				dbHandle.endTrans(false);
				err = MSG_ERR_DB_EXEC;

				goto FREE_MEMORY;
			}
		}
	}

	// Delete Address table
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (ThreadId > 0) {
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s \
					     WHERE ADDRESS_ID = %d;",
					MSGFW_ADDRESS_TABLE_NAME, ThreadId);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("sqlQuery [%s]", sqlQuery);
			dbHandle.endTrans(false);
			err = MSG_ERR_DB_EXEC;

			goto FREE_MEMORY;
		}
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

		pMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T)*pToDeleteMsgIdList->nCount];
		memcpy(pMsgIdList->msgIdList, pToDeleteMsgIdList->msgIdList, sizeof(MSG_MESSAGE_ID_T)*pToDeleteMsgIdList->nCount);
	}
/*** **/

	/*** Create thread  for noti delete. **/
	if (pToDeleteMsgIdList->nCount > 0) {
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


MSG_ERROR_T MsgStoCountMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pThreadCountInfo)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pAddrInfo->contactId > 0) {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
						SUM(CASE WHEN READ_STATUS=0 AND FOLDER_ID=%d THEN 1 ELSE 0 END), \
						SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END), \
						SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END) \
						FROM (SELECT * FROM %s A  JOIN %s B ON A.ADDRESS_ID = B.ADDRESS_ID WHERE B.CONTACT_ID = %d)",
					MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE,
					MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->contactId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*) AS TOTAL, \
						SUM(CASE WHEN READ_STATUS=0 AND FOLDER_ID=%d THEN 1 ELSE 0 END), \
						SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END), \
						SUM(CASE WHEN MAIN_TYPE=%d THEN 1 ELSE 0 END) \
						FROM (SELECT * FROM %s A  JOIN %s B ON A.ADDRESS_ID = B.ADDRESS_ID WHERE B.ADDRESS_VAL = '%s')",
					MSG_INBOX_ID, MSG_SMS_TYPE, MSG_MMS_TYPE,
					MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pAddrInfo->msgAddrInfo.addressVal);
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


MSG_ERROR_T MsgStoSearchMessage(const char *pSearchString, MSG_THREAD_VIEW_LIST_S *pThreadViewList)
{
	if (!pSearchString)
		return MSG_ERR_NULL_POINTER;

	// Clear Out Parameter
	pThreadViewList->nCount = 0;
	pThreadViewList->msgThreadInfo = NULL;

	tr1::unordered_set<MSG_THREAD_ID_T> IdList;
	queue<MSG_THREAD_VIEW_S> searchList;

	MSG_THREAD_VIEW_S threadView;

	char sqlQuery[MAX_QUERY_LEN+1];

	char firstName[MAX_DISPLAY_NAME_LEN+1];
	char lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];

	// Get Name Order
	int order = MsgGetContactNameOrder();

	// Replace string for '%' and '_' character
	char *ext1_str = NULL;
	char *ext2_str = NULL;

	ext1_str = MsgStoReplaceString(pSearchString, "_", "\\_");
	ext2_str = MsgStoReplaceString(ext1_str, "%", "\\%");

	// Search - Address, Name
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_ID, A.CONTACT_ID, A.ADDRESS_VAL, \
						A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, A.IMAGE_PATH, \
						A.UNREAD_CNT, A.SMS_CNT, A.MMS_CNT, \
						A.MSG_DIRECTION, B.DISPLAY_TIME, B.MSG_TEXT \
						FROM %s A, %s B \
			             	WHERE A.ADDRESS_ID = B.ADDRESS_ID AND A.ADDRESS_ID <> 0 \
			                  	AND B.FOLDER_ID > 0 AND B.FOLDER_ID < %d \
			                  	AND ( B.MSG_TEXT LIKE '%%%s%%' ESCAPE '\\' \
			                  	OR B.SUBJECT LIKE '%%%s%%' ESCAPE '\\' \
			                  	OR A.ADDRESS_VAL LIKE '%%%s%%' ESCAPE '\\' \
			                  	OR A.DISPLAY_NAME LIKE '%%%s%%' ESCAPE '\\' \
			                  	OR A.FIRST_NAME LIKE '%%%s%%' ESCAPE '\\' \
			                  	OR A.LAST_NAME LIKE '%%%s%%' ESCAPE '\\') \
			              	ORDER BY B.DISPLAY_TIME DESC;",
						MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_CBMSGBOX_ID,
						ext2_str, ext2_str, ext2_str, ext2_str, ext2_str, ext2_str);


	if (ext1_str) {
		free(ext1_str);
		ext1_str = NULL;
	}

	if (ext2_str) {
		free(ext2_str);
		ext2_str = NULL;
	}

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	while (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		memset(&threadView, 0x00, sizeof(threadView));

		threadView.threadId = dbHandle.columnInt(0);
		threadView.contactId = dbHandle.columnInt(1);

		memset(threadView.threadAddr, 0x00, sizeof(threadView.threadAddr));
		strncpy(threadView.threadAddr, (char *)dbHandle.columnText(2), MAX_THREAD_ADDR_LEN);

		memset(displayName, 0x00, sizeof(displayName));
		strncpy(displayName, (char *)dbHandle.columnText(3), MAX_DISPLAY_NAME_LEN);

		memset(firstName, 0x00, sizeof(firstName));
		strncpy(firstName, (char *)dbHandle.columnText(4), MAX_DISPLAY_NAME_LEN);

		memset(lastName, 0x00, sizeof(lastName));
		strncpy(lastName, (char *)dbHandle.columnText(5), MAX_DISPLAY_NAME_LEN);

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

		strncpy(threadView.threadName, displayName, MAX_THREAD_NAME_LEN);

		strncpy(threadView.threadImagePath, (char *)dbHandle.columnText(6), MAX_IMAGE_PATH_LEN);

		threadView.unreadCnt = dbHandle.columnInt(7);
		threadView.smsCnt = dbHandle.columnInt(8);
		threadView.mmsCnt = dbHandle.columnInt(9);

		threadView.direction = dbHandle.columnInt(10);
		threadView.threadTime = (time_t)dbHandle.columnInt(11);

		strncpy(threadView.threadData, (char *)dbHandle.columnText(12), MAX_THREAD_DATA_LEN);

		tr1::unordered_set<MSG_THREAD_ID_T>::iterator it;

		it = IdList.find(threadView.threadId);

 		if (it == IdList.end()) {
			IdList.insert(threadView.threadId);
			searchList.push(threadView);
		}

	}

	dbHandle.finalizeQuery();

	// Add data to Out Parameter
	pThreadViewList->nCount = searchList.size();
	pThreadViewList->msgThreadInfo = (msg_thread_view_t *)new char[sizeof(MSG_THREAD_VIEW_S *) * searchList.size()];

	MSG_THREAD_VIEW_S *pTmp = NULL;

	int index = 0;

	while (!searchList.empty()) {
		pThreadViewList->msgThreadInfo[index] = (msg_thread_view_t)new char[sizeof(MSG_THREAD_VIEW_S)];

		pTmp = (MSG_THREAD_VIEW_S *)pThreadViewList->msgThreadInfo[index];

		memcpy(pTmp, &(searchList.front()), sizeof(MSG_THREAD_VIEW_S));

		searchList.pop();

		index++;
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoSearchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, MSG_LIST_S *pMsgList)
{
	// Clear Out Parameter
	pMsgList->nCount = 0;
	pMsgList->msgInfo = NULL;

	int rowCnt = 0;
	int index = 27; // numbers of index

	unsigned int addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];
	char sqlQuerySubset[(MAX_QUERY_LEN/5)+1];

	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];

	char *ext1_str = NULL;
	char *ext2_str = NULL;

	// Get Name Order
	int order = MsgGetContactNameOrder();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.MSG_ID, A.ADDRESS_ID, A.FOLDER_ID, A.REFERENCE_ID, A.STORAGE_ID, A.MAIN_TYPE, A.SUB_TYPE, \
							A.DISPLAY_TIME, A.DATA_SIZE, A.NETWORK_STATUS, A.READ_STATUS, A.PROTECTED, A.BACKUP, A.PRIORITY, \
							A.MSG_DIRECTION, A.SCHEDULED_TIME, A.SUBJECT, A.MSG_TEXT, B.ADDRESS_TYPE, B.RECIPIENT_TYPE, \
							B.CONTACT_ID, B.ADDRESS_VAL, B.DISPLAY_NAME, B.FIRST_NAME, B.LAST_NAME, A.ATTACHMENT_COUNT, A.THUMB_PATH \
							FROM %s A, %s B \
							WHERE A.ADDRESS_ID = B.ADDRESS_ID AND B.ADDRESS_ID <> 0 ",
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
		snprintf(sqlQuerySubset, sizeof(sqlQuerySubset), "AND B.ADDRESS_VAL LIKE '%%%s%%' ESCAPE '\\'", ext2_str);
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

	MSG_ERROR_T err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();

		return MSG_SUCCESS;
	} else if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pMsgList->nCount = rowCnt;

	MSG_DEBUG("pMsgList->nCount [%d]", pMsgList->nCount);

	pMsgList->msgInfo = (msg_message_t *)new char[sizeof(MSG_MESSAGE_S *) * rowCnt];

	MSG_MESSAGE_S *pTmp = NULL;

	for (int i = 0; i < rowCnt; i++) {
		pMsgList->msgInfo[i] = (msg_message_t)new char[sizeof(MSG_MESSAGE_S)];

		pTmp = (MSG_MESSAGE_S *)pMsgList->msgInfo[i];

		memset(pTmp, 0x00, sizeof(pTmp));

		pTmp->pData = NULL;
		pTmp->pMmsData = NULL;

		pTmp->msgId = dbHandle.getColumnToInt(index++);
		addrId = dbHandle.getColumnToInt(index++);
		pTmp->folderId = dbHandle.getColumnToInt(index++);
		pTmp->referenceId = dbHandle.getColumnToInt(index++);
		pTmp->storageId = dbHandle.getColumnToInt(index++);
		pTmp->msgType.mainType = dbHandle.getColumnToInt(index++);
		pTmp->msgType.subType = dbHandle.getColumnToInt(index++);
		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);
		pTmp->dataSize = dbHandle.getColumnToInt(index++);
		pTmp->networkStatus = dbHandle.getColumnToInt(index++);
		pTmp->bRead = dbHandle.getColumnToInt(index++);
		pTmp->bProtected = dbHandle.getColumnToInt(index++);
		pTmp->bBackup = dbHandle.getColumnToInt(index++);
		pTmp->priority = dbHandle.getColumnToInt(index++);
		pTmp->direction= dbHandle.getColumnToInt(index++);
		pTmp->scheduledTime = (time_t)dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_SUBJECT_LEN, pTmp->subject);

		if (pTmp->msgType.mainType == MSG_MMS_TYPE &&
			(pTmp->networkStatus == MSG_NETWORK_RETRIEVING || pTmp->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pTmp->msgType.subType == MSG_NOTIFICATIONIND_MMS)) {
			pTmp->pData = NULL;
			index++;
		} else {
			MSG_DEBUG("pTmp->dataSize [%d]", pTmp->dataSize);
			pTmp->pData = (void *)new char[pTmp->dataSize + 2];
			memset(pTmp->pData, 0x00, sizeof(pTmp->pData));

			dbHandle.getColumnToString(index++, pTmp->dataSize+1, (char *)pTmp->pData);
		}

		if (addrId > 0)
			pTmp->nAddressCnt = 1;
		else
			pTmp->nAddressCnt = 0;

		pTmp->addressList[0].threadId = addrId;
		pTmp->addressList[0].addressType = dbHandle.getColumnToInt(index++);
		pTmp->addressList[0].recipientType = dbHandle.getColumnToInt(index++);
		pTmp->addressList[0].contactId = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MAX_ADDRESS_VAL_LEN, pTmp->addressList[0].addressVal);

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

		strncpy(pTmp->addressList[0].displayName, displayName, MAX_DISPLAY_NAME_LEN);

		pTmp->attachCount = dbHandle.getColumnToInt(index++);

		dbHandle.getColumnToString(index++, MSG_FILEPATH_LEN_MAX, pTmp->thumbPath);

	}
	dbHandle.freeTable();

	return MSG_SUCCESS;
}



MSG_ERROR_T MsgStoGetMsgIdList(MSG_REFERENCE_ID_T RefId, MSG_MSGID_LIST_S *pMsgIdList)
{
	// Clear Out Parameter
	pMsgIdList->nCount = 0;
	pMsgIdList->msgIdList = NULL;

	int rowCnt = 0;
	int index = 1; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];

	// Search - Address, Name
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery),
		"SELECT MSG_ID \
					FROM %s \
			             WHERE REFERENCE_ID = %d \
			              ORDER BY MSG_ID ASC;",
			              MSGFW_MESSAGE_TABLE_NAME, RefId);

	MSG_ERROR_T err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pMsgIdList->nCount = rowCnt;

	MSG_DEBUG("pMsgIdList->nCount [%d]", pMsgIdList->nCount);

	pMsgIdList->msgIdList = (MSG_MESSAGE_ID_T *)new char[sizeof(MSG_MESSAGE_ID_T *) * rowCnt];

	for (int i = 0; i < rowCnt; i++) {
		pMsgIdList->msgIdList[i] = (MSG_MESSAGE_ID_T)new char[sizeof(MSG_MESSAGE_ID_T)];

		pMsgIdList->msgIdList[i] = dbHandle.getColumnToInt(index++);
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


MSG_ERROR_T MsgStoGetRejectMsgList(const char *pNumber, MSG_REJECT_MSG_LIST_S *pRejectMsgList)
{
	// Clear Out Parameter
	pRejectMsgList->nCount = 0;
	pRejectMsgList->rejectMsgInfo = NULL;

	int rowCnt = 0;
	int index = 3; // numbers of index

	char sqlQuery[MAX_QUERY_LEN+1];

	// Search Reject Msg
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	if (pNumber != NULL) {
		char phoneNumber[MAX_PRECONFIG_NUM+1];
		memset(phoneNumber, 0x00, sizeof(phoneNumber));

		if (strlen(pNumber) > MAX_PRECONFIG_NUM) {
			MsgConvertNumber(pNumber, phoneNumber);
		} else {
			strncpy(phoneNumber, pNumber, MAX_PRECONFIG_NUM);
		}

		snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT B.MSG_ID, B.MSG_TEXT, B.DISPLAY_TIME \
						FROM %s A, %s B \
				             WHERE A.ADDRESS_ID = B.ADDRESS_ID \
				                  AND B.MAIN_TYPE = %d \
				                  AND B.SUB_TYPE = %d \
				                  AND A.ADDRESS_VAL LIKE '%%%s' \
				              ORDER BY B.DISPLAY_TIME DESC;",
				              MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				              MSG_SMS_TYPE, MSG_REJECT_SMS, phoneNumber);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT B.MSG_ID, B.MSG_TEXT, B.DISPLAY_TIME \
						FROM %s A, %s B \
				             WHERE A.ADDRESS_ID = B.ADDRESS_ID \
				                  AND B.MAIN_TYPE = %d \
				                  AND B.SUB_TYPE = %d \
				              ORDER BY B.DISPLAY_TIME DESC;",
				              MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME,
				              MSG_SMS_TYPE, MSG_REJECT_SMS);
	}

	MSG_ERROR_T  err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("%s", sqlQuery);

		dbHandle.freeTable();

		return err;
	}

	pRejectMsgList->nCount = rowCnt;

	MSG_DEBUG("pRejectMsgList->nCount [%d]", pRejectMsgList->nCount);

	pRejectMsgList->rejectMsgInfo = (MSG_REJECT_MSG_INFO_S *)new char[sizeof(MSG_REJECT_MSG_INFO_S)*rowCnt];

	MSG_REJECT_MSG_INFO_S *pTmp = pRejectMsgList->rejectMsgInfo;

	for (int i = 0; i < rowCnt; i++) {
		pTmp->msgId = dbHandle.getColumnToInt(index++);

		memset(pTmp->msgText, 0x00, sizeof(pTmp->msgText));
		dbHandle.getColumnToString(index++, MAX_MSG_TEXT_LEN, pTmp->msgText);

		pTmp->displayTime = (time_t)dbHandle.getColumnToInt(index++);

		pTmp++;
	}

	dbHandle.freeTable();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetSyncMLExtId(MSG_MESSAGE_ID_T msgId, int *extId)
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


MSG_ERROR_T MsgStoGetReportStatus(MSG_MESSAGE_ID_T msgId, MSG_REPORT_STATUS_INFO_S *pReportStatus)
{
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT DELIVERY_REPORT_STATUS, DELIVERY_REPORT_TIME, READ_REPORT_STATUS, READ_REPORT_TIME \
									FROM %s WHERE MSG_ID = %d;",
							MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		pReportStatus->deliveryStatus = (MSG_DELIVERY_REPORT_STATUS_T)dbHandle.columnInt(0);
		pReportStatus->deliveryStatusTime = (time_t)dbHandle.columnInt(1);
		pReportStatus->readStatus = (MSG_READ_REPORT_STATUS_T)dbHandle.columnInt(2);
		pReportStatus->readStatusTime = (time_t)dbHandle.columnInt(3);
	}

	dbHandle.finalizeQuery();

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgStoGetThreadUnreadCnt(MSG_THREAD_ID_T ThreadId, int *cnt)
{
	MSG_BEGIN();

	int msgCnt = 0;
	*cnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	// Get MSG_ID
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(MSG_ID) FROM %s A \
				     WHERE ADDRESS_ID = %d AND READ_STATUS = 0;",
				MSGFW_MESSAGE_TABLE_NAME, ThreadId);

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

