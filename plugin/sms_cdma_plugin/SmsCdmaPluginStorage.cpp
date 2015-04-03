/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgContact.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgUtilFunction.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"

#include "SmsCdmaPluginMain.h"
#include "SmsCdmaPluginStorage.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginStorage - Member Functions
==================================================================================================*/
SmsPluginStorage* SmsPluginStorage::pInstance = NULL;


SmsPluginStorage::SmsPluginStorage()
{

}


SmsPluginStorage::~SmsPluginStorage()
{

}


SmsPluginStorage* SmsPluginStorage::instance()
{
	if (!pInstance) {
		MSG_DEBUG("pInstance is NULL. Now creating instance.");
		pInstance = new SmsPluginStorage();
	}

	return pInstance;
}


msg_error_t SmsPluginStorage::insertMsgRef(MSG_MESSAGE_INFO_S *pMsg, unsigned char msgRef, int index)
{
	MSG_BEGIN();

	time_t curTime = time(NULL);

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];
	char *normalNum = NULL;

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	normalNum = msg_normalize_number(pMsg->addressList[index].addressVal);

	MSG_SEC_DEBUG("Insert MsgID=[%d], Address=[%s], MsgRef=[%d], Time=[%d]", \
			pMsg->msgId, normalNum, (int)msgRef, (int)curTime);

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %s, %d, 0, -1, %d);",
			MSGFW_SMS_REPORT_TABLE_NAME, pMsg->msgId, normalNum, (int)msgRef, (int)curTime);

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("MsgExecQuery() : [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;

}


msg_error_t SmsPluginStorage::updateMsgDeliverStatus(MSG_MESSAGE_INFO_S *pMsgInfo, unsigned char msgRef)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	msg_message_id_t msgId = 0;
	int rowCnt = 0;
	char *normalNum = NULL;

	normalNum = msg_normalize_number(pMsgInfo->addressList[0].addressVal);

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE ADDRESS_VAL = %s AND MSG_REF > 0 ORDER BY TIME ASC;",
			MSGFW_SMS_REPORT_TABLE_NAME, normalNum);
	MSG_DEBUG("[SQL Query] %s", sqlQuery);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW)
		msgId = dbHandle->columnInt(0);

	dbHandle->finalizeQuery();

	pMsgInfo->msgId = msgId;

	/** Update Status - MSG_MESSAGE_TABLE */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT * FROM %s WHERE MSG_ID = %d AND MSG_REF > 0;",
			MSGFW_SMS_REPORT_TABLE_NAME, msgId);

	if (dbHandle->getTable(sqlQuery, &rowCnt) != MSG_SUCCESS)
		return MSG_ERR_DB_GETTABLE;

	MSG_DEBUG("Selected row count = [%d]", rowCnt);

	if (rowCnt == 1 && pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_SUCCESS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d WHERE MSG_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, (int)pMsgInfo->networkStatus, msgId);
		MSG_DEBUG("[SQL Query] %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed : [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	}

	/** Update Status - MSG_REPORT_TABLE */
	if (pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_SUCCESS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MSG_REF = -1, STATUS = %d, TIME = %d WHERE MSG_ID = %d and ADDRESS_VAL = '%s';",
				MSGFW_SMS_REPORT_TABLE_NAME, 1, (int)pMsgInfo->displayTime, msgId, normalNum);
		MSG_DEBUG("[SQL Query] %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed : [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	} else if(pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_EXPIRED) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MSG_REF = -1, STATUS = %d, TIME = %d WHERE MSG_ID = %d and ADDRESS_VAL = '%s';",
				MSGFW_SMS_REPORT_TABLE_NAME, 0, (int)pMsgInfo->displayTime, msgId, normalNum);
		MSG_DEBUG("[SQL Query] %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed : [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	} else if(pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_PENDING) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MSG_REF = -1, STATUS = %d, TIME = %d WHERE MSG_ID = %d and ADDRESS_VAL = '%s';",
				MSGFW_SMS_REPORT_TABLE_NAME, 3, (int)pMsgInfo->displayTime, msgId, normalNum);
		MSG_DEBUG("[SQL Query] %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed : [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	} else if(pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_FAIL) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET MSG_REF = -1, STATUS = %d, TIME = %d WHERE MSG_ID = %d and ADDRESS_VAL = '%s';",
				MSGFW_SMS_REPORT_TABLE_NAME, 8, (int)pMsgInfo->displayTime, msgId, normalNum);
		MSG_DEBUG("[SQL Query] %s", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			MSG_DEBUG("Query Failed : [%s]", sqlQuery);
			return MSG_ERR_DB_EXEC;
		}
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPluginStorage::updateSentMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_network_status_t status)
{
	MSG_BEGIN();

	if (!pMsgInfo || (pMsgInfo && pMsgInfo->msgId <= 0)) {
		MSG_DEBUG("Invalid message id");
		return MSG_ERR_INVALID_MESSAGE_ID;
	}

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MSG_DEBUG("Update Msg ID : [%d], Network Status : [%d] ", pMsgInfo->msgId, status);

	/** Move Msg to SENTBOX */
	if (status == MSG_NETWORK_SEND_SUCCESS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d, FOLDER_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, status, MSG_SENTBOX_ID, pMsgInfo->msgId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d, READ_STATUS = 0 WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, status, pMsgInfo->msgId);
	}

	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("MsgExecQuery() : [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	if (status == MSG_NETWORK_SEND_SUCCESS) {
		MSG_DEBUG("MsgAddPhoneLog() : folderId [%d]", pMsgInfo->folderId);
		MsgAddPhoneLog(pMsgInfo);
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPluginStorage::checkMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/**  Check whether storage is full or not */
	err = checkStorageStatus(pMsgInfo);

	if (err != MSG_SUCCESS) {
		if (pMsgInfo->msgType.classType == MSG_CLASS_0) {
			pMsgInfo->folderId = 0;
			err = MSG_SUCCESS;
		}
//		else if(pMsgInfo->msgType.classType == MSG_CLASS_2 &&
//				(pMsgInfo->msgType.subType == MSG_NORMAL_SMS || pMsgInfo->msgType.subType == MSG_REJECT_SMS)) {
//			err = addClass2Message(pMsgInfo);
//		}
		else if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS){
			err = MSG_SUCCESS;
		}
		return err;
	}

	/**  Amend message information for type **/
	if (pMsgInfo->msgType.subType == MSG_NORMAL_SMS || pMsgInfo->msgType.subType == MSG_REJECT_SMS) {
		MSG_DEBUG("Normal SMS");

		if (pMsgInfo->msgType.classType == MSG_CLASS_2) {
//			err = addClass2Message(pMsgInfo);
		} else if (pMsgInfo->msgType.classType == MSG_CLASS_0) {
			/** Class 0 Msg should be saved in hidden folder */
			pMsgInfo->folderId = 0;
		}

	} else if ((pMsgInfo->msgType.subType >= MSG_REPLACE_TYPE1_SMS) && (pMsgInfo->msgType.subType <= MSG_REPLACE_TYPE7_SMS)) {
		MSG_DEBUG("Replace SM Type [%d]", pMsgInfo->msgType.subType-3);

		if (pMsgInfo->msgType.classType == MSG_CLASS_2) {
//			err = addClass2Message(pMsgInfo);
		} else if (pMsgInfo->msgType.classType == MSG_CLASS_0) {
			/** Class 0 Msg should be saved in hidden folder */
			pMsgInfo->folderId = 0;
			pMsgInfo->msgType.subType = MSG_NORMAL_SMS;
		}

	} else if ((pMsgInfo->msgType.subType >= MSG_MWI_VOICE_SMS) && (pMsgInfo->msgType.subType <= MSG_MWI_OTHER_SMS)) {
		if (pMsgInfo->bStore == true) {
			MSG_DEBUG("MWI Message");

//			if (pMsgInfo->msgType.classType == MSG_CLASS_2) {
//				err = addClass2Message(pMsgInfo);
//			}
		}
	} else {
		MSG_DEBUG("No matching type [%d]", pMsgInfo->msgType.subType);
	}

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Success to check message !!");
	} else {
		MSG_DEBUG("fail to check message !! : [%d]", err);
	}

	return err;
}


msg_error_t SmsPluginStorage::addSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;

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

		pMsgInfo->threadId = convId;
	}

	/**  Add Message Table */
	rowId = MsgStoAddMessageTable(dbHandle, pMsgInfo);

	if (rowId <= 0) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	/** Update conversation table */
	err = MsgStoUpdateConversation(dbHandle, convId);

	if (err != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return err;
	}

	err = dbHandle->endTrans(true);
	if (err != MSG_SUCCESS) {
		return err;
	}

	pMsgInfo->msgId = (msg_message_id_t)rowId;

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t SmsPluginStorage::deleteSmsMessage(msg_message_id_t msgId)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	char sqlQuery[MAX_QUERY_LEN+1];

	 /**  Get SUB_TYPE, STORAGE_ID */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE, FOLDER_ID, CONV_ID, SIM_INDEX \
			FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, msgId);

	if (dbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	MSG_MESSAGE_TYPE_S msgType;
	msg_folder_id_t folderId;

	msg_thread_id_t convId;
	int simIndex;

	if (dbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = dbHandle->columnInt(0);
		msgType.subType = dbHandle->columnInt(1);
		folderId = dbHandle->columnInt(2);
		convId = dbHandle->columnInt(3);
		simIndex = dbHandle->columnInt(4);

		MSG_DEBUG("Main Type:[%d] SubType:[%d] FolderId:[%d] ConversationId:[%d] simIndex:[%d]", msgType.mainType, msgType.subType, folderId, convId, simIndex);
	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
		dbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle->finalizeQuery();

	dbHandle->beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SMS_SENDOPT_TABLE_NAME, msgId);

	/**  Delete SMS Send Option */
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (msgType.subType == MSG_CB_SMS || msgType.subType == MSG_JAVACB_SMS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_CB_MSG_TABLE_NAME, msgId);

		/** Delete Push Message from push table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	} else if (msgType.subType >= MSG_WAP_SI_SMS && msgType.subType <= MSG_WAP_CO_SMS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_PUSH_MSG_TABLE_NAME, msgId);

		/**  Delete Push Message from push table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	} else if (msgType.subType == MSG_SYNCML_CP) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SYNCML_MSG_TABLE_NAME, msgId);

		/**  Delete SyncML Message from syncML table */
		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle->endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	/** Delete Message from msg table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, msgId);
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/** Delete Message from msg_report table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SMS_REPORT_TABLE_NAME, msgId);
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/** Delete Message from msg_sim table */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SIM_MSG_TABLE_NAME, msgId);
	if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Clear Conversation table */
	if (MsgStoClearConversationTable(dbHandle) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Update conversation table.*/
	if (MsgStoUpdateConversation(dbHandle, convId) != MSG_SUCCESS) {
		dbHandle->endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle->endTrans(true);

	if (folderId == MSG_INBOX_ID) {
		msgType.classType = MSG_CLASS_NONE;

		/**  Set memory status in SIM */
		if (MsgStoCheckMsgCntFull(dbHandle, &msgType, folderId) == MSG_SUCCESS) {
			MSG_DEBUG("Set Memory Status");
			SmsPlgSetMemoryStatus(MSG_SUCCESS);
		}
	}

//	MsgRefreshAllNotification(true, false, false);

	return MSG_SUCCESS;
}


msg_error_t SmsPluginStorage::addSmsSendOption(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo)
{
	MSG_BEGIN();

	MsgDbHandler *dbHandle = getDbHandle();
	msg_error_t err = MSG_SUCCESS;

	if (pSendOptInfo->bSetting == false) {
		MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &pSendOptInfo->bDeliverReq);
		MsgSettingGetBool(SMS_SEND_REPLY_PATH, &pSendOptInfo->option.smsSendOptInfo.bReplyPath);

//		if (pSendOptInfo->bDeliverReq || pSendOptInfo->option.smsSendOptInfo.bReplyPath) {
//			pSendOptInfo->bSetting = true;
			MsgSettingGetBool(MSG_KEEP_COPY, &pSendOptInfo->bKeepCopy);
//		}
	}

//	if (pSendOptInfo->bSetting == true) {
		char sqlQuery[MAX_QUERY_LEN+1];

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d);",
				MSGFW_SMS_SENDOPT_TABLE_NAME, pMsg->msgId, pSendOptInfo->bDeliverReq,
				pSendOptInfo->bKeepCopy, pSendOptInfo->option.smsSendOptInfo.bReplyPath, pMsg->encodeType);

		MSG_DEBUG("Query = [%s]", sqlQuery);

		if (dbHandle->execQuery(sqlQuery) != MSG_SUCCESS) {
			err = MSG_ERR_DB_EXEC;
		}
//	}

	MSG_END();

	return err;
}


msg_error_t SmsPluginStorage::checkStorageStatus(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;

	MsgDbHandler *dbHandle = getDbHandle();
	err = MsgStoCheckMsgCntFull(dbHandle, &(pMsgInfo->msgType), pMsgInfo->folderId);

	if (err != MSG_SUCCESS) {

		if (err == MSG_ERR_MESSAGE_COUNT_FULL) {
			bool bAutoErase = false;

			MsgSettingGetBool(MSG_AUTO_ERASE, &bAutoErase);

			MSG_DEBUG("bAutoErase: %d", bAutoErase);

			if (bAutoErase == true) {
				msg_message_id_t msgId;

				/** Find the oldest message's msgId */
				err = MsgStoGetOldestMessage(dbHandle, pMsgInfo, &msgId);

				if (err != MSG_SUCCESS)
					return err;

				/** Delete the corresponding message. */
				err = deleteSmsMessage(msgId);
			}
		}

		return err;
	}

	return err;
}


msg_error_t SmsPluginStorage::getRegisteredPushEvent(char* pPushHeader, int *count, char *application_id, int app_id_len, char *content_type, int content_type_len)
{
	msg_error_t err = MSG_SUCCESS;

	int rowCnt = 0, index = 3;

	MsgDbHandler *dbHandle = getDbHandle();

	char sqlQuery[MAX_QUERY_LEN+1] = {0, };

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT CONTENT_TYPE, APP_ID, APPCODE FROM %s", MSGFW_PUSH_CONFIG_TABLE_NAME);

	err = dbHandle->getTable(sqlQuery, &rowCnt);
	MSG_DEBUG("rowCnt: %d", rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle->freeTable();
		return MSG_SUCCESS;
	}
	else if ( err != MSG_SUCCESS) {
		dbHandle->freeTable();
		return err;
	}

	char contentType[MAX_WAPPUSH_CONTENT_TYPE_LEN + 1] = {0,};
	char appId[MAX_WAPPUSH_ID_LEN + 1] = {0,};
	int appcode = 0, default_appcode = 0;
	bool found = false;
	char *_content_type = NULL, *_app_id = NULL;
	*count = 0;


	for (int i = 0; i < rowCnt; i++) {
		memset(contentType, 0, MAX_WAPPUSH_CONTENT_TYPE_LEN);
		memset(appId, 0, MAX_WAPPUSH_ID_LEN);

		dbHandle->getColumnToString(index++, MAX_WAPPUSH_CONTENT_TYPE_LEN + 1, contentType);
		dbHandle->getColumnToString(index++, MAX_WAPPUSH_ID_LEN + 1, appId);
		appcode = dbHandle->getColumnToInt(index++);

		//MSG_DEBUG("content_type: %s, app_id: %s", content_type, app_id);
		_content_type = strcasestr(pPushHeader, contentType);
		if(_content_type) {
			_app_id = strcasestr(pPushHeader, appId);
			if(appcode)
				default_appcode = appcode;

			if(_app_id) {
				PUSH_APPLICATION_INFO_S pInfo = {0, };
				pInfo.appcode = appcode;
				MSG_SEC_DEBUG("appcode: %d, app_id: %s", pInfo.appcode, appId);
				snprintf(application_id, app_id_len, "%s", appId);
				snprintf(content_type, content_type_len, "%s", contentType);
				pushAppInfoList.push_back(pInfo);
				(*count)++;
				found = true;
			}
		}
	}

	if(!found && default_appcode != SMS_WAP_APPLICATION_LBS)
	{
		// perform default action.
		PUSH_APPLICATION_INFO_S pInfo = {0, };
		pInfo.appcode = default_appcode;
		memset(appId, 0, MAX_WAPPUSH_ID_LEN + 1);
		snprintf(application_id, app_id_len, "%s", appId);
		snprintf(content_type, content_type_len, "%s", contentType);
		pushAppInfoList.push_back(pInfo);
		*count = 1;
	}
	dbHandle->freeTable();

	return err;
}


msg_error_t SmsPluginStorage::getnthPushEvent(int index, int *appcode)
{
	msg_error_t err = MSG_SUCCESS;
	if((unsigned int)index > pushAppInfoList.size() - 1)
		return MSG_ERR_INVALID_PARAMETER;

	std::list<PUSH_APPLICATION_INFO_S>::iterator it = pushAppInfoList.begin();
	int count = 0;
	for (; it != pushAppInfoList.end(); it++)
	{
		if(index == count){
			*appcode = it->appcode;
			break;
		}
		count++;
	}

	return err;
}


msg_error_t SmsPluginStorage::releasePushEvent()
{
	msg_error_t err = MSG_SUCCESS;
	std::list<PUSH_APPLICATION_INFO_S>::iterator it = pushAppInfoList.begin();

	for (; it != pushAppInfoList.end(); it++)
		it = pushAppInfoList.erase(it);

	return err;
}
