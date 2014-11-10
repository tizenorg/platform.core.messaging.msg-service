/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <time.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgProxyListener.h"
#include "MsgHandle.h"


#define MAX_ADDRESS_LEN 			21 // including '+'

/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Transport Member Functions
==================================================================================================*/
msg_error_t MsgHandle::submitReq(MSG_REQUEST_S* pReq)
{
	MSG_BEGIN();

	if (pReq == NULL)
		THROW(MsgException::INVALID_PARAM, "pReq is NULL");

#ifdef CHECK_SENT_STATUS_CALLBACK
	if (MsgProxyListener::instance()->getSentStatusCbCnt() <= 0)
		THROW(MsgException::SENT_STATUS_ERROR,"Register sent status callback");
#endif

	MSG_REQUEST_INFO_S reqInfo = {0};
	char trId[MMS_TR_ID_LEN+1] = {0};

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;

	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	if (reqmsg->subType != MSG_SENDREQ_JAVA_MMS) {
		// In case MMS read report, get address value later.
		if(reqmsg->subType != MSG_READREPLY_MMS) {
			if ((reqmsg->addr_list->nCount == 0) || (reqmsg->addr_list->nCount > MAX_TO_ADDRESS_CNT)) {
				MSG_DEBUG("Recipient address count error [%d]", reqmsg->addr_list->nCount );
				return MSG_ERR_INVALID_MESSAGE;
			}
		}

		/* Begin: Setting default values for submit request */
	//	pReq->msg.msgId = 0; 	// Set Request ID: internal use
	//	pReq->msg.folderId = MSG_OUTBOX_ID; 	// Set Folder ID
		if (reqmsg->subType == MSG_RETRIEVE_MMS) {
			reqmsg->networkStatus = MSG_NETWORK_RETRIEVING;
		} else {
			reqmsg->networkStatus = MSG_NETWORK_SENDING;
		}

		reqmsg->bRead = false;
		reqmsg->bProtected = false;
		reqmsg->priority = MSG_MESSAGE_PRIORITY_NORMAL;
		reqmsg->direction = MSG_DIRECTION_TYPE_MO;
		reqmsg->storageId = MSG_STORAGE_PHONE;

		time_t curTime = time(NULL);

		if (curTime < 0)
			THROW(MsgException::INVALID_RESULT, "time error : %s", strerror(errno));

		reqmsg->displayTime = curTime;
		/* End : Setting default values for submit request */
	} else {
		//in case of JAVA MMS msg, parse mms transaction id from pMmsData
		reqmsg->networkStatus = MSG_NETWORK_SENDING;
		strncpy(trId, (char*)reqmsg->pMmsData+3,MMS_TR_ID_LEN);
		MSG_DEBUG("JavaMMS transaction Id:%s ",trId);
	}

	// Convert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(reqmsg, &(reqInfo.msgInfo));

	/* Check address validation */
	if (reqInfo.msgInfo.msgType.mainType == MSG_SMS_TYPE) {
		for(int i=0; i<reqmsg->addr_list->nCount; i++) {
				if (reqInfo.msgInfo.addressList[i].addressVal[0] == '+' && strlen(reqInfo.msgInfo.addressList[i].addressVal)>MAX_ADDRESS_LEN) {
					return MSG_ERR_INVALID_PARAMETER;
				} else if (strlen(reqInfo.msgInfo.addressList[i].addressVal)>(MAX_ADDRESS_LEN-1)) {
					return MSG_ERR_INVALID_PARAMETER;
				}
		}
	}

	MSG_MESSAGE_TYPE_S msgType = {0,};

	msgType.mainType = reqmsg->mainType;
	msgType.subType = reqmsg->subType;
	msgType.classType = reqmsg->classType;

	convertSendOptStruct((const MSG_SENDINGOPT_S *)pReq->sendOpt, &(reqInfo.sendOptInfo), msgType);

	reqInfo.reqId = 0;

	/* Register proxy info used for receiving sent status */
	MSG_PROXY_INFO_S chInfo = {0};

	chInfo.listenerFd = MsgProxyListener::instance()->getRemoteFd();

#if defined(__x86_64__) || defined(__aarch64__)
	chInfo.handleAddr = (uint64_t) this;
#else
	chInfo.handleAddr = (unsigned int) this;
#endif

	/* Allocate Memory to Command Data */
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_REQUEST_INFO_S) + sizeof(MSG_PROXY_INFO_S);

	// In case of JAVA MMS msg, add trId
	if (reqmsg->subType == MSG_SENDREQ_JAVA_MMS)
		cmdSize += sizeof(trId);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SUBMIT_REQ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &reqInfo, sizeof(MSG_REQUEST_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_REQUEST_INFO_S)), &chInfo, sizeof(MSG_PROXY_INFO_S));

	// In case of JAVA MMS msg, add trId
	if (reqmsg->subType == MSG_SENDREQ_JAVA_MMS)
		memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_REQUEST_INFO_S)+sizeof(MSG_PROXY_INFO_S)), &trId, sizeof(trId));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*) pEventData;

	int* pReqId = (int*) pEvent->data;
	pReq->reqId = *pReqId;
	MSG_DEBUG("SENT_REQ_ID: %d", pReq->reqId);

	if (pEvent->eventType != MSG_EVENT_SUBMIT_REQ)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	MSG_END();

	return pEvent->result;
}


msg_error_t MsgHandle::cancelReq(msg_request_id_t reqId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_request_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_CANCEL_REQ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &reqId, sizeof(msg_request_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_CANCEL_REQ)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regSentStatusCallback(msg_sent_status_cb onStatusChanged, void *pUserParam)
{
	if (!onStatusChanged)
		THROW(MsgException::INVALID_PARAM, "onStatusChanged is null");

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regSentStatusEventCB(this, onStatusChanged, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int); // cmd type, listenerFd
	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_SENT_STATUS_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	int listenerFd = clientFd;

	MSG_DEBUG("remote fd %d", listenerFd);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &listenerFd, sizeof(listenerFd));

	MSG_DEBUG("reg status [%d : %s], %d", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType), listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_SENT_STATUS_CB)
	{
		THROW(MsgException::INVALID_PARAM, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regSmsMessageCallback(msg_sms_incoming_cb onMsgIncoming, unsigned short port, void *pUserParam)
{
	if( (!onMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regMessageIncomingEventCB(this, onMsgIncoming, port, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_MSG_CB_S); //sizeof(int) + sizeof; // cmd type, listener fd

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_INCOMING_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_SMS_TYPE;
	cmdParam.port = port;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d, port %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd, cmdParam.port);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regMmsConfMessageCallback(msg_mms_conf_msg_incoming_cb onMMSConfMsgIncoming, const char *pAppId, void *pUserParam)
{
	if( (!onMMSConfMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onMMSConfMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regMMSConfMessageIncomingEventCB(this, onMMSConfMsgIncoming, pAppId, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_MMS_TYPE;

	if (pAppId)
		strncpy(cmdParam.appId, pAppId, MAX_MMS_JAVA_APPID_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd:%d, appId:%s", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd,  (pAppId)? cmdParam.appId:"NULL" );

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regSyncMLMessageCallback(msg_syncml_msg_incoming_cb onSyncMLMsgIncoming, void *pUserParam)
{
	if( (!onSyncMLMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onSyncMLMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regSyncMLMessageIncomingEventCB(this, onSyncMLMsgIncoming, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S); //sizeof(int) + sizeof; // cmd type, listener fd

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_SYNCML_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regLBSMessageCallback(msg_lbs_msg_incoming_cb onLBSMsgIncoming, void *pUserParam)
{
	if( (!onLBSMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onLBSMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regLBSMessageIncomingEventCB(this, onLBSMsgIncoming, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_LBS_MSG_CB_S); //sizeof(int) + sizeof; // cmd type, listener fd
	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_LBS_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_INCOMING_LBS_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_LBS_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regSyncMLMessageOperationCallback(msg_syncml_msg_operation_cb onSyncMLMsgOperation, void *pUserParam)
{
	if( (!onSyncMLMsgOperation) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onSyncMLMsgOperation);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regSyncMLMessageOperationEventCB(this, onSyncMLMsgOperation, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S); //sizeof(int) + sizeof; // cmd type, listener fd

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_SYNCML_MSG_OPERATION_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("register syncML msg operation callback [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::regPushMessageCallback(msg_push_msg_incoming_cb onPushMsgIncoming, const char *pAppId, void *pUserParam)
{
	if( (!onPushMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onPushMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regPushMessageIncomingEventCB(this, onPushMsgIncoming, pAppId, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_PUSH_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	if (pAppId)
		strncpy(cmdParam.appId, pAppId, MAX_WAPPUSH_ID_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd:%d, appId:%s", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd,  (pAppId)? cmdParam.appId:"NULL" );

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_PUSH_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::regCBMessageCallback(msg_cb_incoming_cb onCBIncoming, bool bSave, void *pUserParam)
{
	if( (!onCBIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onCBIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	int clientFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if (clientFd < 0)
		return MSG_ERR_TRANSPORT_ERROR;

	if (eventListener->regCBMessageIncomingEventCB(this, onCBIncoming, bSave, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CMD_REG_INCOMING_CB_MSG_CB_S); //sizeof(int) + sizeof; // cmd type, listener fd

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_INCOMING_CB_MSG_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_CMD_REG_CB_INCOMING_MSG_CB_S cmdParam = {0};

	cmdParam.listenerFd = clientFd; // fd that is reserved to the "listener thread" by msgfw daemon
	cmdParam.msgType = MSG_SMS_TYPE;
	cmdParam.bsave = bSave;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_CB_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::operateSyncMLMessage(msg_message_id_t msgId)
{
	if( msgId < 1)
		THROW(MsgException::INVALID_PARAM, "Param msgId %d", msgId);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SYNCML_OPERATION;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msgId, sizeof(msg_message_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SYNCML_OPERATION)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

