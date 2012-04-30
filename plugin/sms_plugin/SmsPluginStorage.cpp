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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgContact.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "SmsPluginMain.h"
#include "SmsPluginSimMsg.h"
#include "SmsPluginStorage.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginStorage - Member Functions
==================================================================================================*/
SmsPluginStorage* SmsPluginStorage::pInstance = NULL;


SmsPluginStorage::SmsPluginStorage()
{
}


SmsPluginStorage::~SmsPluginStorage()
{
	if (dbHandle.disconnect() != MSG_SUCCESS) {
		MSG_DEBUG("DB Disconnect Fail");
	}
}


SmsPluginStorage* SmsPluginStorage::instance()
{
	if (!pInstance) {
		MSG_DEBUG("pInstance is NULL. Now creating instance.");
		pInstance = new SmsPluginStorage();
	}

	return pInstance;
}


MSG_ERROR_T SmsPluginStorage::updateSentMsg(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_NETWORK_STATUS_T Status)
{
	MSG_BEGIN();

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	MSG_DEBUG("Update Msg ID : [%d], Network Status : [%d] ", pMsgInfo->msgId, Status);

	/** Move Msg to SENTBOX */
	if (Status == MSG_NETWORK_SEND_SUCCESS) {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d, FOLDER_ID = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, Status, MSG_SENTBOX_ID, pMsgInfo->msgId);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "UPDATE %s SET NETWORK_STATUS = %d WHERE MSG_ID = %d;",
					MSGFW_MESSAGE_TABLE_NAME, Status, pMsgInfo->msgId);
	}

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		MSG_DEBUG("MsgExecQuery() : [%s]", sqlQuery);
		return MSG_ERR_DB_EXEC;
	}

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginStorage::addSimMessage(MSG_MESSAGE_INFO_S *pSimMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int msgId = 0;
	unsigned int addrId = 0;
	unsigned int simId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	err = MsgStoAddAddress(&dbHandle, &(pSimMsgInfo->addressList[0]), &addrId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	err = dbHandle.getRowId(MSGFW_MESSAGE_TABLE_NAME, &msgId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	simId = pSimMsgInfo->msgId;
	pSimMsgInfo->msgId = (MSG_MESSAGE_ID_T)msgId;

	SMS_CONCAT_SIM_MSG_S concatSimMsg = {0};

	/** Get Data from Concat SIM Msg */
	if (pSimMsgInfo->msgType.subType == MSG_CONCAT_SIM_SMS && pSimMsgInfo->bTextSms == false) {

		int fileSize = 0;

		char* pFileData = NULL;
		AutoPtr<char> buf(&pFileData);

		if (MsgOpenAndReadFile(pSimMsgInfo->msgData, &pFileData, &fileSize) == false) {
			dbHandle.endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}


		memcpy(&concatSimMsg, (SMS_CONCAT_SIM_MSG_S*)pFileData, fileSize);

		/** Delete temporary file */
		MsgDeleteFile(pSimMsgInfo->msgData); /** ipc */

		MSG_DEBUG("SIM ID [%d], MSG DATA [%s]", concatSimMsg.simIdCnt, concatSimMsg.msgData);
	}

	/**  Add Message */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %d, %d, %d, %d, %d, %ld, %d, %d, %d, %d, %d, %d, %ld, %d, ?, '', '', ?, %d, 0, %d, 0, 0);",
				MSGFW_MESSAGE_TABLE_NAME, msgId, addrId, pSimMsgInfo->folderId, 0, pSimMsgInfo->storageId,
				pSimMsgInfo->msgType.mainType, pSimMsgInfo->msgType.subType, pSimMsgInfo->displayTime, pSimMsgInfo->dataSize,
				pSimMsgInfo->networkStatus, pSimMsgInfo->bRead, pSimMsgInfo->bProtected, pSimMsgInfo->priority,
				pSimMsgInfo->direction, pSimMsgInfo->scheduledTime, pSimMsgInfo->bBackup, MSG_DELIVERY_REPORT_NONE, MSG_READ_REPORT_NONE);

	MSG_DEBUG("QUERY : %s", sqlQuery);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_PREPARE;
	}

	dbHandle.bindText(pSimMsgInfo->subject, 1);

	if (pSimMsgInfo->msgType.subType == MSG_CONCAT_SIM_SMS && pSimMsgInfo->bTextSms == false)
		dbHandle.bindText(concatSimMsg.msgData, 2);
	else
		dbHandle.bindText(pSimMsgInfo->msgText, 2);

	if (dbHandle.stepQuery() != MSG_ERR_DB_DONE) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	/** Insert to Sim table */
	if (pSimMsgInfo->msgType.subType == MSG_CONCAT_SIM_SMS && pSimMsgInfo->bTextSms == false) {

		MSG_DEBUG("sim count : %d", concatSimMsg.simIdCnt);

		for (unsigned int i = 0; i < concatSimMsg.simIdCnt; i++) {
			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d);",
						MSGFW_SIM_MSG_TABLE_NAME, msgId, concatSimMsg.simIdList[i]);

			MSG_DEBUG("QUERY : %s", sqlQuery);

			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.endTrans(false);
				return MSG_ERR_DB_EXEC;
			}
		}
	} else {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d);",
					MSGFW_SIM_MSG_TABLE_NAME, msgId, simId);

		MSG_DEBUG("QUERY : %s", sqlQuery);

		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	/**  Update Address Info. */
	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	return err;
}


MSG_ERROR_T SmsPluginStorage::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	/**  Check whether storage is full or not */
	err = checkStorageStatus(pMsgInfo);

	if (err != MSG_SUCCESS) {
		return err;
	}

	if (pMsgInfo->msgType.subType == MSG_NORMAL_SMS || pMsgInfo->msgType.subType == MSG_REJECT_SMS) {

		MSG_DEBUG("Add Normal SMS");

		if (pMsgInfo->msgType.classType == MSG_CLASS_2) {
			err = SmsPluginSimMsg::instance()->saveClass2Message(pMsgInfo);

			if (err == MSG_SUCCESS) {
				MSG_DEBUG("Success to saveSimMessage.");
			} else {
				MSG_DEBUG("Fail to saveSimMessage : [%d]", err);
			}
		} else {
			/** Class 0 Msg should be saved in hidden folder */
			if (pMsgInfo->msgType.classType == MSG_CLASS_0) {
				pMsgInfo->folderId = 0;
			}

			/**  Add into DB */
			err = addSmsMessage(pMsgInfo);
		}

	} else if ((pMsgInfo->msgType.subType == MSG_CB_SMS) || (pMsgInfo->msgType.subType == MSG_JAVACB_SMS)) {
		MSG_DEBUG("Add CB Message");
		err = addCbMessage(pMsgInfo);
	} else if ((pMsgInfo->msgType.subType >= MSG_REPLACE_TYPE1_SMS) && (pMsgInfo->msgType.subType <= MSG_REPLACE_TYPE7_SMS)) {
		MSG_DEBUG("Add Replace SM Type [%d]", pMsgInfo->msgType.subType-3);
		err = addReplaceTypeMsg(pMsgInfo);
	} else if ((pMsgInfo->msgType.subType >= MSG_MWI_VOICE_SMS) && (pMsgInfo->msgType.subType <= MSG_MWI_OTHER_SMS)) {
		MSG_DEBUG("Add MWI Message");
		err = addSmsMessage(pMsgInfo);
	} else if ((pMsgInfo->msgType.subType == MSG_WAP_SI_SMS) || (pMsgInfo->msgType.subType == MSG_WAP_CO_SMS)) {
		MSG_DEBUG("Add WAP Push Message");
		switch (pMsgInfo->msgType.subType)
		{
			case MSG_WAP_SI_SMS:
			{
				// save push message information
				err = addWAPMessage(pMsgInfo);
			}
			break;

			case MSG_WAP_CO_SMS:
			{
				err = handleCOWAPMessage(pMsgInfo);
			}
			break;
		}
	} else if (pMsgInfo->msgType.subType == MSG_STATUS_REPORT_SMS) {
		MSG_DEBUG("Add Status Report");
		err = addSmsMessage(pMsgInfo);
	}

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Success to add message !!");
	} else {
		MSG_DEBUG("fail to add message !! : [%d]", err);
	}

	return err;
}


MSG_ERROR_T SmsPluginStorage::addSmsMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int rowId = 0;
	unsigned int addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {

		err = MsgStoAddAddress(&dbHandle, &(pMsgInfo->addressList[0]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}

		pMsgInfo->addressList[0].threadId = (MSG_THREAD_ID_T)addrId;
	}

	/**  Add Message Table */
	rowId = MsgStoAddMessageTable(&dbHandle, pMsgInfo, addrId);

	if (rowId <= 0) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	/** Update Address table */
	err = MsgStoUpdateAddress(&dbHandle, addrId);

	if (err != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	dbHandle.endTrans(true);

	pMsgInfo->msgId = (MSG_MESSAGE_ID_T)rowId;
	pMsgInfo->referenceId = (MSG_REFERENCE_ID_T)rowId;

	MSG_END();

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginStorage::updateSmsMessage(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int addrId = 0;

	dbHandle.beginTrans();

	if (pMsg->nAddressCnt > 0) {

		err = MsgStoAddAddress(&dbHandle, &(pMsg->addressList[0]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	int fileSize = 0;

	char* pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	/**  Get File Data */
	if (pMsg->bTextSms == false) {
		if (MsgOpenAndReadFile(pMsg->msgData, &pFileData, &fileSize) == false) {
			dbHandle.endTrans(false);
			return MSG_ERR_STORAGE_ERROR;
		}
	}

	/**  Update Message */
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
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	dbHandle.finalizeQuery();

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


MSG_ERROR_T SmsPluginStorage::deleteSmsMessage(MSG_MESSAGE_ID_T MsgId)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	 /**  Get SUB_TYPE, STORAGE_ID */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MAIN_TYPE, SUB_TYPE, FOLDER_ID, ADDRESS_ID \
				        FROM %s WHERE MSG_ID = %d;",
			MSGFW_MESSAGE_TABLE_NAME, MsgId);

	if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	MSG_MESSAGE_TYPE_S msgType;
	MSG_FOLDER_ID_T folderId;

	unsigned int addrId;

	if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
		msgType.mainType = dbHandle.columnInt(0);
		msgType.subType = dbHandle.columnInt(1);
		folderId = dbHandle.columnInt(2);
		addrId = dbHandle.columnInt(3);

		MSG_DEBUG("Main Type:[%d] SubType:[%d] FolderId:[%d] AddressId:[%d]", msgType.mainType, msgType.subType, folderId, addrId);
	} else {
		MSG_DEBUG("MsgStepQuery() Error [%s]", sqlQuery);
		dbHandle.finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	dbHandle.finalizeQuery();

	dbHandle.beginTrans();

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SCHEDULED_MSG_TABLE_NAME, MsgId);

	/** Delete Message from scheduled msg table */
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SMS_SENDOPT_TABLE_NAME, MsgId);

	/**  Delete SMS Send Option */
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	if (msgType.subType == MSG_CB_SMS || msgType.subType == MSG_JAVACB_SMS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_CB_MSG_TABLE_NAME, MsgId);

		/** Delete Push Message from push table */
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	} else if (msgType.subType >= MSG_WAP_SI_SMS && msgType.subType <= MSG_WAP_CO_SMS) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_PUSH_MSG_TABLE_NAME, MsgId);

		/**  Delete Push Message from push table */
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	} else if (msgType.subType == MSG_SYNCML_CP) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_SYNCML_MSG_TABLE_NAME, MsgId);

		/**  Delete SyncML Message from syncML table */
		if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return MSG_ERR_DB_EXEC;
		}
	}

	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;", MSGFW_MESSAGE_TABLE_NAME, MsgId);

	/** Delete Message from msg table */
	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Clear Address table */
	if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Update Address Info.*/
	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	if (folderId == MSG_INBOX_ID) {
		msgType.classType = MSG_CLASS_NONE;

		/**  Set memory status in SIM */
		if (MsgStoCheckMsgCntFull(&dbHandle, &msgType, folderId) == MSG_SUCCESS) {
			MSG_DEBUG("Set Memory Status");
			SmsPlgSetMemoryStatus(MSG_SUCCESS);
		}
	}

	int smsCnt = 0, mmsCnt = 0;

	smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
	mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

	MsgSettingHandleNewMsg(smsCnt, mmsCnt);
	MsgDeleteNotiByMsgId(MsgId);

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginStorage::addCbMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned int rowId = 0, addrId = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	dbHandle.beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {
		err = MsgStoAddAddress(&dbHandle, &(pMsgInfo->addressList[0]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}

		pMsgInfo->addressList[0].threadId = (MSG_THREAD_ID_T)addrId;
	}

	/**  Add Message Table */
	rowId = MsgStoAddMessageTable(&dbHandle, pMsgInfo, addrId);

	if (rowId <= 0) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	/**  Get CB Msg ID */
	unsigned short cbMsgId = (unsigned short)pMsgInfo->msgId;

	/** Add CB Msg in MSG_CBMSG_TABLE */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	sprintf(sqlQuery, "INSERT INTO %s VALUES (%d, %d);",
				MSGFW_CB_MSG_TABLE_NAME, rowId, cbMsgId);

	if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_EXEC;
	}

	/**  Update Address Info. */
	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	/** Assign Msg ID */
	pMsgInfo->msgId = (MSG_MESSAGE_ID_T)rowId;
	pMsgInfo->referenceId = (MSG_REFERENCE_ID_T)rowId;

	return err;
}


MSG_ERROR_T SmsPluginStorage::addReplaceTypeMsg(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char sqlQuery[MAX_QUERY_LEN+1];

	unsigned int addrId = 0, retCnt = 0;

	/** Check if new address or not */
	if (MsgExistAddress(&dbHandle, pMsgInfo->addressList[0].addressVal, &addrId) == true) {
		MSG_DEBUG("Address Info. exists [%d] [%s]", addrId, pMsgInfo->addressList[0].addressVal);

		/**  Find Replace Type Msg : Same Replace Type, Same Origin Address */
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT COUNT(*), A.MSG_ID FROM %s A, %s B \
					     WHERE A.ADDRESS_ID = B.ADDRESS_ID AND A.SUB_TYPE = %d AND B.ADDRESS_ID = %d;",
				MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pMsgInfo->msgType.subType, addrId);

		if (dbHandle.prepareQuery(sqlQuery) != MSG_SUCCESS)
			return MSG_ERR_DB_PREPARE;

		if (dbHandle.stepQuery() == MSG_ERR_DB_ROW) {
			retCnt = dbHandle.columnInt(0);
			pMsgInfo->msgId = dbHandle.columnInt(1);
		} else {
			dbHandle.finalizeQuery();
			return MSG_ERR_DB_STEP;
		}

		dbHandle.finalizeQuery();
	}

	/** Update New Replace Type Msg */
	if (retCnt == 1) {
		MSG_DEBUG("Update Replace Type Msg");
		err = updateSmsMessage(pMsgInfo);
	} else if (retCnt == 0) { /** Insert New Replace Type Msg */
		MSG_DEBUG("Insert Replace Type Msg");
		err = addSmsMessage(pMsgInfo);
	}

	return err;
}


MSG_ERROR_T SmsPluginStorage::addWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_PUSH_MESSAGE_S pushMsg = {};

	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	int fileSize = 0;

	char* pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
		return MSG_ERR_STORAGE_ERROR;

	MSG_DEBUG("fileSize : %d", fileSize);

	memcpy(&pushMsg, pFileData, fileSize);

	/** Delete temporary file */
	MsgDeleteFile(pMsgInfo->msgData);

	/** check pPushMsg data */

	MSG_DEBUG("check pushMsg data");
	MSG_DEBUG("pushMsg.action : [%d]", pushMsg.action);
	MSG_DEBUG("pushMsg.received : [%d]", pushMsg.received);
	MSG_DEBUG("pushMsg.created : [%d]", pushMsg.created);
	MSG_DEBUG("pushMsg.expires : [%d]", pushMsg.expires);
	MSG_DEBUG("pushMsg.id : [%s]", pushMsg.id);
	MSG_DEBUG("pushMsg.href : [%s]", pushMsg.href);
	MSG_DEBUG("pushMsg.contents : [%s]", pushMsg.contents);

	bool bProceed = true;

	/**  check validation of contents */
	if (checkPushMsgValidation(&pushMsg, &bProceed) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to check Push Message validation.");
	}

	/**  if validation check value is false */
	/** return and drop message. */
	if (bProceed == false)
		return MSG_ERR_INVALID_MESSAGE;

	/**  update subject */
	int len = strlen(pushMsg.contents);

	if (len > MAX_SUBJECT_LEN) {
		memcpy(pMsgInfo->subject, pushMsg.contents, MAX_SUBJECT_LEN);
		pMsgInfo->subject[MAX_SUBJECT_LEN] = '\0';
	} else {
		strncpy(pMsgInfo->subject, pushMsg.contents, MAX_SUBJECT_LEN);
	}

	/**  Update Msg Text - remove */
	strncpy(pMsgInfo->msgText, pushMsg.href, MAX_MSG_TEXT_LEN);
	pMsgInfo->dataSize = strlen(pMsgInfo->msgText);

	pMsgInfo->bTextSms = true;
	pMsgInfo->folderId = MSG_INBOX_ID;
	pMsgInfo->storageId = MSG_STORAGE_PHONE;

	unsigned int addrId = 0;

	dbHandle.beginTrans();

	if (pMsgInfo->nAddressCnt > 0) {

		err = MsgStoAddAddress(&dbHandle, &(pMsgInfo->addressList[0]), &addrId);

		if (err != MSG_SUCCESS) {
			dbHandle.endTrans(false);
			return err;
		}
	}

	/**  get last row count for Message id */
	unsigned int rowId = 0;

	/** Add Message Table */
	rowId = MsgStoAddMessageTable(&dbHandle, pMsgInfo, addrId);

	if (rowId <= 0) {
		dbHandle.endTrans(false);
		return MSG_ERR_DB_ROW;
	}

	/**  add msg_push_table */
	snprintf (sqlQuery, sizeof(sqlQuery), "INSERT INTO %s VALUES (%d, %d, %lu, %lu, ?, ?, ?)",
			            MSGFW_PUSH_MSG_TABLE_NAME, pMsgInfo->msgId, pushMsg.action, pushMsg.created, pushMsg.expires);

	if ((err = dbHandle.prepareQuery(sqlQuery)) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return err;
	}

	dbHandle.bindText(pushMsg.id, 1);

	dbHandle.bindText(pushMsg.href, 2);

	dbHandle.bindText(pushMsg.contents, 3);

	if ((err = dbHandle.stepQuery()) != MSG_ERR_DB_DONE) {
		dbHandle.endTrans(false);
		return err;
	}

	/** Update Address Info. */
	if (MsgStoUpdateAddress(&dbHandle, addrId) != MSG_SUCCESS) {
		dbHandle.endTrans(false);
		return MSG_ERR_STORAGE_ERROR;
	}

	dbHandle.endTrans(true);

	pMsgInfo->msgId = (MSG_MESSAGE_ID_T)rowId;
	pMsgInfo->referenceId = (MSG_REFERENCE_ID_T)rowId;

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginStorage::handleCOWAPMessage(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char href[MAX_PUSH_CACHEOP_MAX_URL_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	int fileSize = 0;

	char* pFileData = NULL;
	AutoPtr<char> buf(&pFileData);

	if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
		return MSG_ERR_STORAGE_ERROR;

	MSG_PUSH_CACHEOP_S *pPushMsg;

	pPushMsg = (MSG_PUSH_CACHEOP_S*)pFileData;

	for (int i = 0; i < pPushMsg->invalObjectCnt; i++) {

		int msgid = -1;

		memset(href, 0x00, sizeof(href));
		strncpy(href, &(pPushMsg->invalObjectUrl[i][0]), MAX_PUSH_CACHEOP_MAX_URL_LEN);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf (sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE HREF LIKE '%%%s%%';",
					MSGFW_PUSH_MSG_TABLE_NAME, href);

		dbHandle.beginTrans();

		err = dbHandle.prepareQuery(sqlQuery);

		if ((dbHandle.stepQuery() == MSG_ERR_DB_ROW) && err == MSG_SUCCESS) {

			msgid = dbHandle.getColumnToInt(1);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID= %d;",
       					MSGFW_PUSH_MSG_TABLE_NAME, msgid);

			/** Delete Message from Push table */
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, msgid);

			/** Delete Message from msg table */
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			/** Update all Address */
			if (updateAllAddress() != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			/** Clear Address table */
			if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}
		}

		dbHandle.finalizeQuery();

		dbHandle.endTrans(true);
	}

	for (int i = 0; i < pPushMsg->invalServiceCnt; i++) {

		int msgid = -1;

		memset(href, 0x00, sizeof(href));
		strncpy(href, &(pPushMsg->invalObjectUrl[i][0]), MAX_PUSH_CACHEOP_MAX_URL_LEN);

		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT MSG_ID FROM %s WHERE HREF LIKE '%%%s%%'",
					MSGFW_PUSH_MSG_TABLE_NAME, href);

		dbHandle.beginTrans();

		err = dbHandle.prepareQuery(sqlQuery);

		if ((dbHandle.stepQuery() == MSG_ERR_DB_ROW) && err == MSG_SUCCESS) {

			msgid = dbHandle.getColumnToInt(1);

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			sprintf(sqlQuery, "DELETE FROM %s WHERE MSG_ID='%d'",
						MSGFW_PUSH_MSG_TABLE_NAME, msgid);

			/** Delete Message from Push table */
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			memset(sqlQuery, 0x00, sizeof(sqlQuery));
			snprintf(sqlQuery, sizeof(sqlQuery), "DELETE FROM %s WHERE MSG_ID = %d;",
						MSGFW_MESSAGE_TABLE_NAME, msgid);

			/** Delete Message from msg table */
			if (dbHandle.execQuery(sqlQuery) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			/**  Update all Address */
			if (updateAllAddress() != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}

			/** Clear Address table */
			if (MsgStoClearAddressTable(&dbHandle) != MSG_SUCCESS) {
				dbHandle.finalizeQuery();
				dbHandle.endTrans(false);
				continue;
			}
		}

		dbHandle.finalizeQuery();

		dbHandle.endTrans(true);
	}

	/** delete temporary file */
	MsgDeleteFile(pMsgInfo->msgData);

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginStorage::checkPushMsgValidation(MSG_PUSH_MESSAGE_S *pPushMsg, bool *pbProceed)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	unsigned long oldExpireTime = 0;
	int rowCnt = 0;

	char sqlQuery[MAX_QUERY_LEN+1];

	/**  is push message is expired?? */
	if (pPushMsg->received > pPushMsg->expires) {
		MSG_DEBUG("Push Message is expired.");
		pbProceed = false;
		return err;
	}


	if (pPushMsg->action == MSG_PUSH_SL_ACTION_EXECUTE_LOW) {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXPIRES FROM %s WHERE ID = '%s' AND ACTION = %d",
					MSGFW_PUSH_MSG_TABLE_NAME, pPushMsg->id, pPushMsg->action);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT EXPIRES FROM %s WHERE ID = '%s'",
					MSGFW_PUSH_MSG_TABLE_NAME, pPushMsg->id);
	}

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (rowCnt < 1) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	}

	oldExpireTime = dbHandle.getColumnToInt(1);

	dbHandle.freeTable();

	if (pPushMsg->created < oldExpireTime) {
		MSG_DEBUG("Push Message is expired.");
		pbProceed = false;
		return err;
	}

	return err;
}


MSG_ERROR_T SmsPluginStorage::checkStorageStatus(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	err = MsgStoCheckMsgCntFull(&dbHandle, &(pMsgInfo->msgType), pMsgInfo->folderId);

	if (err != MSG_SUCCESS) {

		if (err == MSG_ERR_MESSAGE_COUNT_FULL) {
			bool bAutoErase = false;

			MsgSettingGetBool(MSG_AUTO_ERASE, &bAutoErase);

			MSG_DEBUG("bAutoErase: %d", bAutoErase);

			if (bAutoErase == true) {
				MSG_MESSAGE_ID_T msgId;

				/** Find the oldest message's msgId */
				err = MsgStoGetOldestMessage(&dbHandle, pMsgInfo, &msgId);

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


MSG_ERROR_T SmsPluginStorage::updateAllAddress()
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int rowCnt = 0, index = 1;
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT ADDRESS_ID FROM %s",
				MSGFW_ADDRESS_TABLE_NAME);

	err = dbHandle.getTable(sqlQuery, &rowCnt);

	if (err == MSG_ERR_DB_NORECORD) {
		dbHandle.freeTable();
		return MSG_SUCCESS;
	} else if ( err != MSG_SUCCESS) {
		dbHandle.freeTable();
		return err;
	}


	for (int i = 0; i < rowCnt; i++) {

		err = MsgStoUpdateAddress(&dbHandle, index++);

		if (err != MSG_SUCCESS)
			break;
	}

	dbHandle.freeTable();

	return err;
}
