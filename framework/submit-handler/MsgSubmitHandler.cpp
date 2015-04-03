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

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"
#include "MsgSubmitHandler.h"



/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgSubmitReq(MSG_REQUEST_INFO_S *pReqInfo, bool bScheduled)
{
	msg_error_t err = MSG_SUCCESS;
	static int reqId = 1;

	pReqInfo->reqId = reqId;
	reqId++;

	MSG_DEBUG("==== Msg ID = [%d] ====", pReqInfo->msgInfo.msgId);
	MSG_DEBUG("==== Folder ID = [%d] ====", pReqInfo->msgInfo.folderId);
	MSG_INFO("==== Main Type = [%d] ====", pReqInfo->msgInfo.msgType.mainType);
	MSG_INFO("==== Sub Type = [%d] ====", pReqInfo->msgInfo.msgType.subType);
	MSG_DEBUG("==== Class Type = [%d] ====", pReqInfo->msgInfo.msgType.classType);
	MSG_DEBUG("==== Message Data = [%s] ====", pReqInfo->msgInfo.msgData);
	MSG_SEC_DEBUG("==== Message Text = [%s] ====", pReqInfo->msgInfo.msgText);
	MSG_INFO("==== SIM Index = [%d] ====", pReqInfo->msgInfo.sim_idx);

	MSG_DEBUG("==== bSetting = [%d] ====", pReqInfo->sendOptInfo.bSetting);

	if (pReqInfo->msgInfo.msgType.mainType == MSG_SMS_TYPE)
	{
		MSG_DEBUG("==== deliver = [%d] ====", pReqInfo->sendOptInfo.bDeliverReq);
		MSG_DEBUG("==== keepcopy = [%d] ====", pReqInfo->sendOptInfo.bKeepCopy);
		MSG_DEBUG("==== bReplyPath = [%d] ====", pReqInfo->sendOptInfo.option.smsSendOptInfo.bReplyPath);

		err = MsgSubmitReqSMS(pReqInfo);
	}
	else if (pReqInfo->msgInfo.msgType.mainType == MSG_MMS_TYPE)
	{
		MSG_DEBUG("==== deliver = [%d] ====", pReqInfo->sendOptInfo.bDeliverReq);
		MSG_DEBUG("==== keepcopy = [%d] ====", pReqInfo->sendOptInfo.bKeepCopy);
		MSG_DEBUG("==== bReadReq = [%d] ====", pReqInfo->sendOptInfo.option.mmsSendOptInfo.bReadReq);
		MSG_DEBUG("==== priority = [%d] ====", pReqInfo->sendOptInfo.option.mmsSendOptInfo.priority);
		MSG_DEBUG("==== expiryTime = [%d] ====", pReqInfo->sendOptInfo.option.mmsSendOptInfo.expiryTime.time);

		err = MsgSubmitReqMMS(pReqInfo);
	}

	return err;
}


msg_error_t MsgSubmitReqSMS(MSG_REQUEST_INFO_S *pReqInfo)
{
	msg_error_t err = MSG_SUCCESS;

	// submit request based on msgType;
	MSG_MAIN_TYPE_T mainType = pReqInfo->msgInfo.msgType.mainType;
	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

	if (plg == NULL)
		THROW(MsgException::PLUGIN_ERROR, "No plugin for %d type", mainType);

	// If MSG ID > 0 -> MSG in DRAFT
	// Move Folder to OUTBOX
	if (pReqInfo->msgInfo.msgPort.valid == false) {
		pReqInfo->msgInfo.folderId = MSG_OUTBOX_ID;

	if (pReqInfo->msgInfo.msgId > 0 && (pReqInfo->msgInfo.folderId == MSG_DRAFT_ID || pReqInfo->msgInfo.folderId == MSG_OUTBOX_ID))
		err = MsgStoUpdateMessage(&(pReqInfo->msgInfo), &(pReqInfo->sendOptInfo));
	}

	err = plg->submitReq(pReqInfo);

	return err;
}

msg_error_t MsgSubmitReqMMS(MSG_REQUEST_INFO_S *pReqInfo)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	MSG_RECIPIENTS_LIST_S pRecipientList;

	MsgPlugin *sms_plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	if (sms_plg == NULL){
		THROW(MsgException::PLUGIN_ERROR, "No plugin for %d type", MSG_SMS_TYPE);
	}

	int defaultNetworkSimId = 0;

	err = sms_plg->getDefaultNetworkSimId(&defaultNetworkSimId);
	if (err != MSG_SUCCESS) {
		MSG_ERR("getDefaultNetworkSimId is failed=[%d]", err);
		return err;
	} else if (pReqInfo->msgInfo.sim_idx != defaultNetworkSimId) {
		MSG_ERR("It is not default network SIM ID, request SIM=[%d], activated SIM=[%d]", pReqInfo->msgInfo.sim_idx, defaultNetworkSimId);
		return MSG_ERR_INVALID_PARAMETER;

	}

	MSG_MAIN_TYPE_T msgMainType = pReqInfo->msgInfo.msgType.mainType;
	MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(msgMainType);

	if(!plg)
	{
		MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);
		MSG_DEBUG("No Plugin for %d type", msgMainType);

		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}

	//	If MSG ID > 0 -> MSG in DRAFT
	// Move Folder to OUTBOX
	/* reject_msg_support */
	MSG_DEBUG("Not scheduled MMS, pReqInfo->msgInfo.msgType.subType [%d]", pReqInfo->msgInfo.msgType.subType);

	if(pReqInfo->msgInfo.msgType.subType == MSG_SENDREQ_JAVA_MMS)
	{
		char fileName[MAX_COMMON_INFO_SIZE+1] = {0};

		// copy whole of MMS PDU filepath to msgData
		strncpy(fileName, pReqInfo->msgInfo.msgData, MAX_COMMON_INFO_SIZE);
		memset(pReqInfo->msgInfo.msgData, 0x00, MAX_MSG_DATA_LEN+1);
		snprintf(pReqInfo->msgInfo.msgData, MAX_MSG_DATA_LEN+1, "%s%s", MSG_IPC_DATA_PATH, fileName);

		MSG_SEC_DEBUG("JAVA MMS PDU filepath:%s", pReqInfo->msgInfo.msgData);

		// submit request
		err = plg->submitReq(pReqInfo);

		if(err != MSG_SUCCESS)
		{
			MSG_DEBUG("Update Network Status : [%d]", err);
			MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo),MSG_NETWORK_SEND_FAIL);
		}

		return err;
	}
	else if((pReqInfo->msgInfo.msgType.subType == MSG_SENDREQ_MMS) || (pReqInfo->msgInfo.msgType.subType == MSG_FORWARD_MMS))
	{
		if(pReqInfo->msgInfo.msgId > 0 && (pReqInfo->msgInfo.folderId == MSG_DRAFT_ID || pReqInfo->msgInfo.folderId == MSG_OUTBOX_ID)) {
			MSG_DEBUG("Not New Message.");
			pReqInfo->msgInfo.folderId = MSG_OUTBOX_ID;
			err = MsgStoUpdateMessage(&(pReqInfo->msgInfo), &(pReqInfo->sendOptInfo));
		} else {
			MSG_DEBUG("New Message.");
			pReqInfo->msgInfo.msgId = 0;
			pReqInfo->msgInfo.folderId = MSG_OUTBOX_ID;
			err = MsgStoAddMessage(&(pReqInfo->msgInfo), &(pReqInfo->sendOptInfo));
		}
	}
	else if(pReqInfo->msgInfo.msgType.subType == MSG_READREPLY_MMS)
	{
		msg_read_report_status_t readStatus;

		err = MsgStoCheckReadReportStatus(pReqInfo->msgInfo.msgId);
		if(err != MSG_SUCCESS) {

			MSG_INFO("msg id [%d], read report could NOT send [%d]", pReqInfo->msgInfo.msgId, err);
			return err;
		}

		memcpy(&readStatus, pReqInfo->msgInfo.msgData, sizeof(msg_read_report_status_t));
		if (readStatus == MSG_READ_REPORT_REJECT_BY_USER) {

			MSG_INFO("msg id [%d], read report reject by user", pReqInfo->msgInfo.msgId);

			err = MsgStoSetReadReportSendStatus(pReqInfo->msgInfo.msgId, MMS_RECEIVE_READ_REPORT_NO_SENT);

			return err;
		}

		err = MsgStoGetRecipientList(pReqInfo->msgInfo.msgId, &pRecipientList);
		if(err != MSG_SUCCESS)
			return MSG_ERR_PLUGIN_STORAGE;

		pReqInfo->msgInfo.nAddressCnt = pRecipientList.recipientCnt;
		pReqInfo->msgInfo.addressList = pRecipientList.recipientAddr;

//			for(int i = 0; i < pRecipientList.recipientCnt; i++)
//			{
//				pReqInfo->msgInfo.addressList[i].addressType = pRecipientList.recipientAddr[i].addressType;
//				pReqInfo->msgInfo.addressList[i].recipientType = MSG_RECIPIENTS_TYPE_TO;
//				pReqInfo->msgInfo.addressList[i].contactId = pRecipientList.recipientAddr[i].contactId;
//				strncpy(pReqInfo->msgInfo.addressList[i].addressVal, pRecipientList.recipientAddr[i].addressVal, MAX_ADDRESS_VAL_LEN);
//			}

		char subject[MAX_SUBJECT_LEN+1];

		err = MsgStoGetSubject(pReqInfo->msgInfo.msgId, subject);
		if(err != MSG_SUCCESS)
			MSG_DEBUG("Getting subject returned not a MSG_SUCCESS");

		strncpy(pReqInfo->msgInfo.subject, subject, MAX_SUBJECT_LEN);

		err = plg->composeReadReport(&(pReqInfo->msgInfo));
	}
	else if(pReqInfo->msgInfo.msgType.subType == MSG_RETRIEVE_MMS)
	{
		MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo), pReqInfo->msgInfo.networkStatus);
	}

	/* reject_msg_support */

	if(err != MSG_SUCCESS)
	{
		MSG_DEBUG("Fail to Add/Update Message : MsgStoMoveMessageToFolder()/MsgStoAddMessage()");
		return err;
	}

	switch(pReqInfo->msgInfo.msgType.subType)
	{
		case MSG_SENDREQ_MMS:
		case MSG_FORWARD_MMS:
			MsgDeleteFile(pReqInfo->msgInfo.msgData);
			memset(pReqInfo->msgInfo.msgData, 0x00, MAX_MSG_DATA_LEN+1);
			snprintf(pReqInfo->msgInfo.msgData, MAX_MSG_DATA_LEN+1, "%s%d.mms", MSG_DATA_PATH, pReqInfo->msgInfo.msgId);
			break;

		case MSG_READREPLY_MMS:
		case MSG_READRECIND_MMS:
			break;
		default:
			break;
	}

	// update content location from db
	if(pReqInfo->msgInfo.msgType.subType == MSG_RETRIEVE_MMS)
		err = MsgStoGetContentLocation(&(pReqInfo->msgInfo));
	/* reject_msg_support */
	else if(pReqInfo->msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS)
		err = plg->updateRejectStatus(&(pReqInfo->msgInfo));
	/* reject_msg_support */

	// Check SIM is present or not
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_CHANGED, pReqInfo->msgInfo.sim_idx);
	MSG_SIM_STATUS_T simStatus = (MSG_SIM_STATUS_T)MsgSettingGetInt(keyName);

	if(simStatus == MSG_SIM_STATUS_NOT_FOUND)
	{
		MSG_DEBUG("SIM is not present...");
		MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);

		return MSG_ERR_NO_SIM;
	}

	if(err == MSG_SUCCESS)
		err = plg->submitReq(pReqInfo);

	if(err == MSG_SUCCESS && ( pReqInfo->msgInfo.msgType.subType == MSG_READREPLY_MMS || pReqInfo->msgInfo.msgType.subType == MSG_READRECIND_MMS ))
		MsgStoSetReadReportSendStatus(pReqInfo->msgInfo.msgId, MMS_RECEIVE_READ_REPORT_SENT);

	if (err != MSG_SUCCESS)
	{
		if(pReqInfo->msgInfo.msgType.subType == MSG_RETRIEVE_MMS )
			MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo), MSG_NETWORK_RETRIEVE_FAIL);
		else
			MsgStoUpdateNetworkStatus(dbHandle, &(pReqInfo->msgInfo), MSG_NETWORK_SEND_FAIL);
	}


	return err;
}

msg_error_t MsgCancelReq(msg_request_id_t reqId)
{
	msg_error_t err = MSG_SUCCESS;

	return err;
}


msg_error_t MsgUpdateSentMsg(msg_message_id_t MsgId, msg_network_status_t Status)
{
	msg_error_t err = MSG_SUCCESS;

	bool bKeepCopy = true;

#ifdef MSG_MMS_KEEPCOPY
	MSG_SENDINGOPT_INFO_S sendOpt = {};

	if (msgType.mainType == MSG_MMS_TYPE)
	{
		if (MsgStoGetMmsSendOpt(MsgId, &sendOpt) == MSG_SUCCESS)
		{
			bKeepCopy = sendOpt.bKeepCopy;
		}
		else
		{
			ret = MsgSettingGetBool(MSG_KEEP_COPY, &bKeepCopy);
		}
	}
#endif

	// Move Msg to SENTBOX
	if (Status == MSG_NETWORK_SEND_SUCCESS)
	{
		MSG_DEBUG(" In Status == MSG_NETWORK_SEND_SUCCESS and  bKeepCopy is [%d]", bKeepCopy);
		if (bKeepCopy == true)
			err = MsgStoMoveMessageToFolder(MsgId, MSG_SENTBOX_ID);
		else
			err = MsgStoDeleteMessage(MsgId, false);
	}

	return err;
}


void MsgCopyReqInfo(MSG_REQUEST_INFO_S *pSrc, int addrIdx, MSG_REQUEST_INFO_S *pDest)
{
	MSG_BEGIN();

	memset(pDest, 0x00, sizeof(MSG_REQUEST_INFO_S));

	// Copy Request ID
	pDest->reqId = pSrc->reqId;

	// Copy Msg Info
	pDest->msgInfo.msgId = pSrc->msgInfo.msgId;

	pDest->msgInfo.threadId = pSrc->msgInfo.threadId;

	pDest->msgInfo.folderId = pSrc->msgInfo.folderId;

	pDest->msgInfo.msgType.mainType = pSrc->msgInfo.msgType.mainType;
	pDest->msgInfo.msgType.subType = pSrc->msgInfo.msgType.subType;
	pDest->msgInfo.msgType.classType = pSrc->msgInfo.msgType.classType;

	pDest->msgInfo.storageId = pSrc->msgInfo.storageId;

	pDest->msgInfo.nAddressCnt = 1;

	pDest->msgInfo.addressList[0].addressType = pSrc->msgInfo.addressList[addrIdx].addressType;
	pDest->msgInfo.addressList[0].recipientType = pSrc->msgInfo.addressList[addrIdx].recipientType;
	pDest->msgInfo.addressList[0].contactId = pSrc->msgInfo.addressList[addrIdx].contactId;
	strncpy(pDest->msgInfo.addressList[0].addressVal, pSrc->msgInfo.addressList[addrIdx].addressVal, MAX_ADDRESS_VAL_LEN);
	strncpy(pDest->msgInfo.addressList[0].displayName, pSrc->msgInfo.addressList[addrIdx].displayName, MAX_DISPLAY_NAME_LEN);

	strncpy(pDest->msgInfo.replyAddress, pSrc->msgInfo.replyAddress, MAX_PHONE_NUMBER_LEN);
	strncpy(pDest->msgInfo.subject, pSrc->msgInfo.subject, MAX_SUBJECT_LEN);

	pDest->msgInfo.displayTime = pSrc->msgInfo.displayTime;
	pDest->msgInfo.networkStatus = pSrc->msgInfo.networkStatus;
	pDest->msgInfo.encodeType = pSrc->msgInfo.encodeType;
	pDest->msgInfo.bRead = pSrc->msgInfo.bRead;
	pDest->msgInfo.bProtected = pSrc->msgInfo.bProtected;
	pDest->msgInfo.priority = pSrc->msgInfo.priority;
	pDest->msgInfo.direction = pSrc->msgInfo.direction;

	pDest->msgInfo.msgPort.valid = pSrc->msgInfo.msgPort.valid;

	if (pDest->msgInfo.msgPort.valid == true)
	{
		pDest->msgInfo.msgPort.dstPort = pSrc->msgInfo.msgPort.dstPort;
		pDest->msgInfo.msgPort.srcPort = pSrc->msgInfo.msgPort.srcPort;
	}

	pDest->msgInfo.bTextSms = pSrc->msgInfo.bTextSms;
	pDest->msgInfo.dataSize = pSrc->msgInfo.dataSize;

	strncpy(pDest->msgInfo.msgData, pSrc->msgInfo.msgData, MAX_MSG_DATA_LEN);

	if (pDest->msgInfo.bTextSms == true)
	{
		memcpy(pDest->msgInfo.msgText, pSrc->msgInfo.msgText, pDest->msgInfo.dataSize);
		pDest->msgInfo.msgText[pDest->msgInfo.dataSize] = '\0';
	}

	// Set Sending Info
	pDest->sendOptInfo.bSetting = pSrc->sendOptInfo.bSetting;

	if (pDest->sendOptInfo.bSetting == true)
	{
		pDest->sendOptInfo.bDeliverReq = pSrc->sendOptInfo.bDeliverReq;
		pDest->sendOptInfo.bKeepCopy = pSrc->sendOptInfo.bKeepCopy;

		if (pDest->msgInfo.msgType.mainType == MSG_SMS_TYPE)
		{
			pDest->sendOptInfo.option.smsSendOptInfo.bReplyPath = pSrc->sendOptInfo.option.smsSendOptInfo.bReplyPath;
		}
		else if (pDest->msgInfo.msgType.mainType == MSG_MMS_TYPE)
		{
			pDest->sendOptInfo.option.mmsSendOptInfo.priority = pSrc->sendOptInfo.option.mmsSendOptInfo.priority;
			pDest->sendOptInfo.option.mmsSendOptInfo.bReadReq = pSrc->sendOptInfo.option.mmsSendOptInfo.bReadReq;

			pDest->sendOptInfo.option.mmsSendOptInfo.expiryTime.type = pSrc->sendOptInfo.option.mmsSendOptInfo.expiryTime.type;
			pDest->sendOptInfo.option.mmsSendOptInfo.expiryTime.time = pSrc->sendOptInfo.option.mmsSendOptInfo.expiryTime.time;

			pDest->sendOptInfo.option.mmsSendOptInfo.bUseDeliveryCustomTime = pSrc->sendOptInfo.option.mmsSendOptInfo.bUseDeliveryCustomTime;
			pDest->sendOptInfo.option.mmsSendOptInfo.deliveryTime.type = pSrc->sendOptInfo.option.mmsSendOptInfo.deliveryTime.type;
			pDest->sendOptInfo.option.mmsSendOptInfo.deliveryTime.time = pSrc->sendOptInfo.option.mmsSendOptInfo.deliveryTime.time;
		}
	}

	MSG_END();
}

