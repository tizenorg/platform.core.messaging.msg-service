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

#include <time.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgUtilFunction.h"
#include "MsgProxyListener.h"
#include "MsgHandle.h"


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

	reqInfo.msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&reqInfo.msgInfo.addressList, unique_ptr_deleter);

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;

	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	if (reqmsg->simIndex <= 0) {
		MSG_DEBUG("Wrong SIM Index [%d]", reqmsg->simIndex);
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (reqmsg->subType != MSG_SENDREQ_JAVA_MMS) {
		// In case MMS read report, get address value later.
		if(reqmsg->subType != MSG_READREPLY_MMS) {
			if (reqmsg->addr_list && (reqmsg->addr_list->nCount > 0) && (reqmsg->addr_list->nCount <= MAX_TO_ADDRESS_CNT)) {
				MSG_DEBUG("Recipient address count [%d]", reqmsg->addr_list->nCount );
			} else if (g_list_length(reqmsg->addressList) > 0) {
				MSG_DEBUG("Recipient address count [%d]", g_list_length(reqmsg->addressList) );
			} else {
				MSG_DEBUG("Address count is invalid.");
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
		//reqmsg->bProtected = false;
		reqmsg->priority = MSG_MESSAGE_PRIORITY_NORMAL;
		reqmsg->direction = MSG_DIRECTION_TYPE_MO;
		reqmsg->storageId = MSG_STORAGE_PHONE;

		time_t curTime = time(NULL);

		if (curTime < 0)
			THROW(MsgException::INVALID_RESULT, "time error : %s", g_strerror(errno));

		reqmsg->displayTime = curTime;
		/* End : Setting default values for submit request */
	} else {
		//in case of JAVA MMS msg, parse mms transaction id from pMmsData
		reqmsg->networkStatus = MSG_NETWORK_SENDING;
		strncpy(trId, (char*)reqmsg->pMmsData+3,MMS_TR_ID_LEN);
		MSG_SEC_DEBUG("JavaMMS transaction Id:%s ",trId);
	}

	// Convert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(reqmsg, &(reqInfo.msgInfo));

	MSG_MESSAGE_TYPE_S msgType = {0,};

	msgType.mainType = reqmsg->mainType;
	msgType.subType = reqmsg->subType;
	msgType.classType = reqmsg->classType;

	msg_struct_s *send_opt_s = (msg_struct_s *)pReq->sendOpt;
	MSG_SENDINGOPT_S *send_opt = (MSG_SENDINGOPT_S *)send_opt_s->data;

	convertSendOptStruct((const MSG_SENDINGOPT_S *)send_opt, &(reqInfo.sendOptInfo), msgType);

	reqInfo.reqId = 0;

	/* Register proxy info used for receiving sent status */
	MSG_PROXY_INFO_S chInfo = {0};

	chInfo.listenerFd = MsgProxyListener::instance()->getRemoteFd();

	chInfo.handleAddr = (unsigned long) this;

	/* Allocate Memory to Command Data */
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(&reqInfo.msgInfo, &reqInfo.sendOptInfo, &encodedData);

	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_request_id_t) + dataSize + sizeof(MSG_PROXY_INFO_S);

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
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &reqInfo.reqId, sizeof(msg_request_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_request_id_t)), &chInfo, sizeof(MSG_PROXY_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_request_id_t)+sizeof(MSG_PROXY_INFO_S)), encodedData, dataSize);

	// In case of JAVA MMS msg, add trId
	if (reqmsg->subType == MSG_SENDREQ_JAVA_MMS)
		memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_request_id_t)+sizeof(MSG_PROXY_INFO_S)+dataSize), &trId, sizeof(trId));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*) pEventData;

	int* pReqId = (int*) pEvent->data;
	pReq->reqId = *pReqId;
	MSG_DEBUG("SENT_REQ_ID: %d", pReq->reqId);

	if (pEvent->eventType != MSG_EVENT_SUBMIT_REQ)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error:%d", pEvent->eventType);
	}

	MSG_END();

	return pEvent->result;
}


msg_error_t MsgHandle::regSentStatusCallback(msg_sent_status_cb onStatusChanged, void *pUserParam)
{
	if (!onStatusChanged)
		THROW(MsgException::INVALID_PARAM, "onStatusChanged is null");

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regSentStatusEventCB(this, onStatusChanged, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int); // cmd type, listenerFd
	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_SENT_STATUS_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_DEBUG("remote fd %d", remoteFd);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &remoteFd, sizeof(remoteFd));

	MSG_DEBUG("reg status [%d : %s], %d", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType), remoteFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regMessageIncomingEventCB(this, onMsgIncoming, port, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;
	cmdParam.port = port;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d, port %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd, cmdParam.port);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regMMSConfMessageIncomingEventCB(this, onMMSConfMsgIncoming, pAppId, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_MMS_TYPE;

	if (pAppId)
		strncpy(cmdParam.appId, pAppId, MAX_MMS_JAVA_APPID_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd:%d, appId:%s", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd,  (pAppId)? cmdParam.appId:"NULL" );

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regSyncMLMessageIncomingEventCB(this, onSyncMLMsgIncoming, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regLBSMessageIncomingEventCB(this, onLBSMsgIncoming, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regSyncMLMessageOperationEventCB(this, onSyncMLMsgOperation, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("register syncML msg operation callback [%s], fd %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regPushMessageIncomingEventCB(this, onPushMsgIncoming, pAppId, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;

	if (pAppId)
		strncpy(cmdParam.appId, pAppId, MAX_WAPPUSH_ID_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd:%d, appId:%s", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd,  (pAppId)? cmdParam.appId:"NULL" );

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


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

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regCBMessageIncomingEventCB(this, onCBIncoming, bSave, pUserParam) == false)
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

	cmdParam.listenerFd = remoteFd;
	cmdParam.msgType = MSG_SMS_TYPE;
	cmdParam.bsave = bSave;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &cmdParam, sizeof(cmdParam));

	MSG_DEBUG("reg new msg [%s], fd: %d, bSave: %d", MsgDbgCmdStr(pCmd->cmdType), cmdParam.listenerFd, cmdParam.bsave);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_INCOMING_CB_MSG_CB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::regReportMessageCallback(msg_report_msg_incoming_cb onReportMsgCB, void *pUserParam)
{
	if (!onReportMsgCB)
		THROW(MsgException::INVALID_PARAM, "onReportMsgCB is null");

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start(this);

	int remoteFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	if(remoteFd == -1 )
		return MSG_ERR_INVALID_MSGHANDLE;

	if (eventListener->regReportMsgIncomingCB(this, onReportMsgCB, pUserParam) == false)
		return MSG_ERR_INVALID_PARAMETER;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int); // cmd type, listenerFd
	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_REPORT_MSG_INCOMING_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_DEBUG("remote fd %d", remoteFd);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &remoteFd, sizeof(remoteFd));

	MSG_DEBUG("reg status [%d : %s], %d", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType), remoteFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_REPORT_MSG_INCOMING_CB)
	{
		THROW(MsgException::INVALID_PARAM, "Event Data Error");
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
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SYNCML_OPERATION)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}
