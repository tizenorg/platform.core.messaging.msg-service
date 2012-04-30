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

#include <time.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgProxyListener.h"
#include "MsgHandle.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Transport Member Functions
==================================================================================================*/
MSG_ERROR_T MsgHandle::submitReq(MSG_REQUEST_S* pReq)
{
	MSG_BEGIN();

	if (pReq == NULL)
		THROW(MsgException::INVALID_PARAM, "pReq is NULL");

	MSG_REQUEST_INFO_S reqInfo = {0};
	char trId[MMS_TR_ID_LEN+1] = {0};

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) pReq->msg;

	if (reqmsg->msgType.subType != MSG_SENDREQ_JAVA_MMS) {
		// In case MMS read report, get address value later.
		if(reqmsg->msgType.subType != MSG_READREPLY_MMS) {
			if ((reqmsg->nAddressCnt == 0) || (reqmsg->nAddressCnt > MAX_TO_ADDRESS_CNT)) {
				MSG_DEBUG("Recipient address count error [%d]", reqmsg->nAddressCnt );
				return MSG_ERR_INVALID_MESSAGE;
			}
		}

		/* Begin: Setting default values for submit request */
	//	pReq->msg.msgId = 0; 	// Set Request ID: internal use
	//	pReq->msg.folderId = MSG_OUTBOX_ID; 	// Set Folder ID
		if (reqmsg->msgType.subType == MSG_RETRIEVE_MMS) {
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

	convertSendOptStruct(&(pReq->sendOpt), &(reqInfo.sendOptInfo), reqmsg->msgType);

	reqInfo.reqId = 0;

	/* Register proxy info used for receiving sent status */
	MSG_PROXY_INFO_S chInfo = {0};

	chInfo.listenerFd = MsgProxyListener::instance()->getRemoteFd();

	chInfo.handleAddr = (unsigned int) this;

	/* Allocate Memory to Command Data */
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_REQUEST_INFO_S) + sizeof(MSG_PROXY_INFO_S);

	// In case of JAVA MMS msg, add trId
	if (reqmsg->msgType.subType == MSG_SENDREQ_JAVA_MMS)
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
	if (reqmsg->msgType.subType == MSG_SENDREQ_JAVA_MMS)
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


MSG_ERROR_T MsgHandle::cancelReq(MSG_REQUEST_ID_T reqId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_REQUEST_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_CANCEL_REQ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &reqId, sizeof(MSG_REQUEST_ID_T));

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


MSG_ERROR_T MsgHandle::regSentStatusCallback(msg_sent_status_cb onStatusChanged, void *pUserParam)
{
	if (!onStatusChanged)
		THROW(MsgException::INVALID_PARAM, "onStatusChanged is null");

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	int listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

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


MSG_ERROR_T MsgHandle::regSmsMessageCallback(msg_sms_incoming_cb onMsgIncoming, unsigned short port, void *pUserParam)
{
	if( (!onMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	cmdParam.listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon
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


MSG_ERROR_T MsgHandle::regMmsConfMessageCallback(msg_mms_conf_msg_incoming_cb onMMSConfMsgIncoming, const char *pAppId, void *pUserParam)
{
	if( (!onMMSConfMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onMMSConfMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	cmdParam.listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon
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


MSG_ERROR_T MsgHandle::regSyncMLMessageCallback(msg_syncml_msg_incoming_cb onSyncMLMsgIncoming, void *pUserParam)
{
	if( (!onSyncMLMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onSyncMLMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	cmdParam.listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon
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


MSG_ERROR_T MsgHandle::regLBSMessageCallback(msg_lbs_msg_incoming_cb onLBSMsgIncoming, void *pUserParam)
{
	if( (!onLBSMsgIncoming) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onLBSMsgIncoming);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	cmdParam.listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon
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


MSG_ERROR_T MsgHandle::regSyncMLMessageOperationCallback(msg_syncml_msg_operation_cb onSyncMLMsgOperation, void *pUserParam)
{
	if( (!onSyncMLMsgOperation) )
		THROW(MsgException::INVALID_PARAM, "Param %p", onSyncMLMsgOperation);

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

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

	cmdParam.listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon
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


MSG_ERROR_T MsgHandle::operateSyncMLMessage(MSG_MESSAGE_ID_T msgId)
{
	if( msgId < 1)
		THROW(MsgException::INVALID_PARAM, "Param msgId %d", msgId);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SYNCML_OPERATION;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msgId, sizeof(MSG_MESSAGE_ID_T));

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

