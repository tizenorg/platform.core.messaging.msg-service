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

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgSoundPlayer.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"
#include "MsgUtilFunction.h"
#include "MsgSubmitHandler.h"
#include "MsgDeliverHandler.h"
#include "MsgStorageHandler.h"
#include "MsgTransManager.h"
#include "MsgPluginManager.h"
#include "MsgCmdHandler.h"
#include "MsgUtilStorage.h"

#include <alarm.h>

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgSubmitReqHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;
	bool bNewMsg = true;

	int eventSize = 0;

	MSG_REQUEST_INFO_S reqInfo = {0,};
	MSG_PROXY_INFO_S proxyInfo = {0,};

	// Get Message Request
	memcpy(&reqInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_REQUEST_INFO_S));

	// Storing Request ID, Proxy Info for Sent Status CNF
	memcpy(&proxyInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_REQUEST_INFO_S)), sizeof(MSG_PROXY_INFO_S));

	if (reqInfo.msgInfo.msgId > 0)
		bNewMsg = false;

	// Submit Request
	err = MsgSubmitReq(&reqInfo, false);

	if (err == MSG_SUCCESS){
		MSG_DEBUG("Command Handle Success : MsgSubmitReq()");
	} else {
		MSG_DEBUG("Command Handle Fail : MsgSubmitReq()");
	}

	int reqId = reqInfo.reqId;
	proxyInfo.sentMsgId = reqInfo.msgInfo.msgId;

	MSG_DEBUG("REQID: %d, MSGID: %d", reqId, proxyInfo.sentMsgId);

	if (reqInfo.msgInfo.msgType.mainType == MSG_SMS_TYPE) {
		MsgTransactionManager::instance()->insertSentMsg(reqId, &proxyInfo);
	} else if (reqInfo.msgInfo.msgType.mainType == MSG_MMS_TYPE) {
		// Retrieve MMS shall not be kept in sentMsg
		if ((reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_MMS) ||
			(reqInfo.msgInfo.msgType.subType == MSG_FORWARD_MMS) ||
			(reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_JAVA_MMS))
		MsgTransactionManager::instance()->insertSentMsg(reqId, &proxyInfo);
	}

	// keep transaction Id list for distinguish java MMS sent msg when sendconf received
	if (reqInfo.msgInfo.msgType.mainType == MSG_MMS_TYPE &&
		reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_JAVA_MMS) {
		MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S trId={0};
		memcpy(&trId.id, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_REQUEST_INFO_S)+sizeof(MSG_PROXY_INFO_S)), MMS_TR_ID_LEN);

		char* pFileName;
		pFileName = strstr(reqInfo.msgInfo.msgData, "MSG_");
		strncpy(trId.pduFileName, pFileName, MAX_COMMON_INFO_SIZE);

		MSG_DEBUG("java MMS msg trId:%s filepath:%s ",trId.id, reqInfo.msgInfo.msgData);

		MsgTransactionManager* tm = MsgTransactionManager::instance();
		tm->setJavaMMSList(&trId);
	}

	// Make Event Data
	eventSize = MsgMakeEvent(&reqId, sizeof(reqId), MSG_EVENT_SUBMIT_REQ, err, (void**)ppEvent);

	/* reject_msg_support */
	if(((reqInfo.msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS) &&
		 (reqInfo.msgInfo.msgType.mainType == MSG_MMS_TYPE)))
		err = MsgStoDeleteMessage(reqInfo.msgInfo.msgId, true);

	/** send storage CB */
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = reqInfo.msgInfo.msgId;
	msgIdList.msgIdList = msgIds;

	if ((err == MSG_SUCCESS || err != MSG_ERR_PLUGIN_STORAGE) && reqInfo.msgInfo.msgPort.valid == false) {
		if (bNewMsg) {
			MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
		} else {
			MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
		}
	} else if (err == MSG_ERR_SECURITY_ERROR) { // Case of MDM enabled, it returns MSG_ERR_SECURITY_ERROR.
		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
	} else {
		MSG_DEBUG("No need to broadcast storage change CB");
	}

	return eventSize;
}


int MsgCancelReqHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Request ID
	msg_request_id_t* reqId = (msg_request_id_t*)pCmd->cmdData;

	// Cancel Request
	err = MsgCancelReq(*reqId);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgSubCancelReq()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgSubCancelReq()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_CANCEL_REQ, err, (void**)ppEvent);

	return eventSize;
}


int MsgRegSentStatusCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	int listenerFd = *((int*) pCmd->cmdData);
	MSG_DEBUG("Registering sent status CB for %d", listenerFd);

	// storing dst fd in list
	MsgTransactionManager::instance()->setSentStatusCB(listenerFd);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_SENT_STATUS_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming SMS CB for fd %d mType %d port %d", pCmdData->listenerFd, pCmdData->msgType, pCmdData->port);

	// storing dst fd in list
	MsgTransactionManager::instance()->setIncomingMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingMMSConfMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming MMS Conf CB for fd:%d mType:%d appId:%s", pCmdData->listenerFd, pCmdData->msgType, pCmdData->appId);

	// storing dst fd in list
	MsgTransactionManager::instance()->setMMSConfMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}

int MsgRegIncomingPushMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming Push Msg CB for fd:%d mType:%d appId:%s", pCmdData->listenerFd, pCmdData->msgType, pCmdData->appId);

	// storing dst fd in list
	MsgTransactionManager::instance()->setPushMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_PUSH_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}

int MsgRegIncomingCBMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_CB_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_CB_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming Push Msg CB for fd:%d mType:%d", pCmdData->listenerFd, pCmdData->msgType);

	// storing dst fd in list
	MsgTransactionManager::instance()->setCBMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_CB_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingSyncMLMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming Sync ML Msg CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	// storing dst fd in list
	MsgTransactionManager::instance()->setSyncMLMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingLBSMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_INCOMING_LBS_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_LBS_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming LBS Msg CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	// storing dst fd in list
	MsgTransactionManager::instance()->setLBSMsgCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_LBS_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegSyncMLMsgOperationCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S *pCmdData = (MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering SyncML Msg ooperation CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	// storing dst fd in list
	MsgTransactionManager::instance()->setSyncMLMsgOperationCB(pCmdData);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegStorageChangeCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if( !pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	int listenerFd = *((int*) pCmd->cmdData);
	MSG_DEBUG("Registering storage change CB for %d", listenerFd);

	// storing dst fd in list
	MsgTransactionManager::instance()->setStorageChangeCB(listenerFd);

	// Make Event Data
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_STORAGE_CHANGE_CB, MsgException::SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgSentStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Message Request
	MSG_SENT_STATUS_S* pStatus = (MSG_SENT_STATUS_S*) pCmd->cmdData;

	MSG_DEBUG("REQID %d, STATUS %d", pStatus->reqId, pStatus->status);

	// storing dst fd in list
	MSG_PROXY_INFO_S* prxInfo = MsgTransactionManager::instance()->getProxyInfo(pStatus->reqId);

	// when no sent status cb is found (in case of mobile tracker)
	if (!prxInfo)
	{
		return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
	}

	MSG_DEBUG("REQID %d, listenerFD %d, handleAddr %x, msgId %d", pStatus->reqId, prxInfo->listenerFd, prxInfo->handleAddr, prxInfo->sentMsgId);

	// if APP send and quit(not exist at this time), don't send the data up.
	if (prxInfo->handleAddr == 0)
	{
		// just making data which will be passed to plugin. it indicates "handling evt success"
		MsgTransactionManager::instance()->delProxyInfo(pStatus->reqId);

		return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
	}

#if defined(__x86_64__) || defined(__aarch64__)
	uint64_t ret[3] = {0}; //3// reqid, status, object
#else
	unsigned int ret[3] = {0}; //3// reqid, status, object
#endif

	ret[0] = pStatus->reqId;
	ret[1] = pStatus->status;
	ret[2] = prxInfo->handleAddr;

	// Make Event Data for APP
	int eventSize = MsgMakeEvent(ret, sizeof(ret), MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);

	// Send to listener thread, here
	MsgTransactionManager::instance()->write(prxInfo->listenerFd, *ppEvent, eventSize);

	MsgTransactionManager::instance()->delProxyInfo(pStatus->reqId);

	return eventSize;
}


int MsgIncomingMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	int eventSize = 0;
	bool sendNoti = true;

	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Incoming Message
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	memcpy(&msgInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_INFO_S));

	// normal process
	err = MsgHandleIncomingMsg(&msgInfo, &sendNoti);

	// broadcast to listener threads, here
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = msgInfo.msgId;
	msgIdList.msgIdList = msgIds;

	if (sendNoti == true) {
		MsgTransactionManager::instance()->broadcastIncomingMsgCB(err, &msgInfo);
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	} else if(msgInfo.msgPort.valid)
	{
		MsgTransactionManager::instance()->broadcastIncomingMsgCB(err, &msgInfo);
	}
	else if (msgInfo.folderId == MSG_SPAMBOX_ID) {
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	}

	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MSG_IND, err, (void**)ppEvent);

	MSG_END();

	return eventSize;
}

int MsgIncomingMMSConfMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();
	msg_error_t err = MSG_SUCCESS;
	int eventsize = 0;

	MSG_MESSAGE_INFO_S msgInfo = {0};
	msg_request_id_t reqID;

	memcpy(&msgInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_INFO_S));
	memcpy(&reqID, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), sizeof(msg_request_id_t));

	MSG_DEBUG(" pMsg = %s, pReqId = %d ", msgInfo.msgData, reqID);
	MSG_DEBUG(" msgtype subtype is [%d]", msgInfo.msgType.subType);

	// For Storage change callback
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = msgInfo.msgId;
	msgIdList.msgIdList = msgIds;

	//err = MsgStoGetAddrInfo(msgInfo.msgId, &(msgInfo.addressList[0]));
	err = MsgStoGetOrgAddressList(&msgInfo);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("MsgStoGetOrgAddressList() success.");
//		msgInfo.nAddressCnt = 1;
	} else {
		MSG_DEBUG("MsgStoGetOrgAddressList() fail.");
	}

	if(msgInfo.msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || msgInfo.msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {

		err = MsgHandleMmsConfIncomingMsg(&msgInfo, reqID);

		if(err != MSG_SUCCESS)
			return err;

		MMS_RECV_DATA_S* pMmsRecvData = (MMS_RECV_DATA_S*)msgInfo.msgData;

		if (pMmsRecvData->msgAppId.valid == true) {
			MSG_DEBUG("valid : %d, appId : %s", pMmsRecvData->msgAppId.valid, pMmsRecvData->msgAppId.appId);
		} else {
			msgInfo.bTextSms = true;
			msgInfo.dataSize = 0 ;
			memset(msgInfo.msgData, 0x00, sizeof(MMS_RECV_DATA_S));
		}

		eventsize = MsgMakeEvent(&msgInfo, sizeof(MSG_MESSAGE_INFO_S), MSG_EVENT_PLG_INCOMING_MMS_CONF, msgInfo.networkStatus, (void**)ppEvent);

		// broadcast to listener threads, here
		MsgTransactionManager::instance()->broadcastMMSConfCB(msgInfo.networkStatus, &msgInfo, pMmsRecvData);
		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
	} else if (msgInfo.msgType.subType == MSG_SENDREQ_MMS || msgInfo.msgType.subType == MSG_SENDCONF_MMS) {
		MSG_PROXY_INFO_S* prxInfo = MsgTransactionManager::instance()->getProxyInfo(reqID);

		// when no sent status cb is found (in case of mobile tracker)
		if (!prxInfo) {
			MSG_DEBUG("prxInfo is NULL");
			eventsize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
		} else {
			// No need to update javaMMS sent messages
			javamms_list& listenerList = MsgTransactionManager::instance()->getJavaMMSList();
			javamms_list::iterator it = listenerList.begin();

			MSG_DEBUG("listenerList size:%d ",listenerList.size());

			if (msgInfo.networkStatus == MSG_NETWORK_SEND_FAIL && msgInfo.msgType.subType == MSG_SENDREQ_MMS) {
				for ( ; it != listenerList.end() ; it++) {
					if (strstr(it->pduFileName, "JAVA")) {
						MSG_DEBUG("JAVA MMS fileName:%s", it->pduFileName);
						MsgDeleteFile(it->pduFileName);		// ipc
						listenerList.erase(it);
						goto __BYPASS_UPDATE;
					}
				}
			} else {
				//msgData has MMS_RECV_DATA_S
				MMS_RECV_DATA_S* pMmsRecvData = (MMS_RECV_DATA_S*)msgInfo.msgData;

				for ( ; it != listenerList.end() ; it++) {
					if(!strcmp(it->id, pMmsRecvData->szTrID)) {
						MSG_DEBUG("find sent JAVA MMS message trId:%s from listener list trId:%s",pMmsRecvData->szTrID, it->id);
						MsgDeleteFile(it->pduFileName); // ipc
						listenerList.erase(it);
						goto __BYPASS_UPDATE;
					}
				}
			}
		}

		err = MsgHandleMmsConfIncomingMsg(&msgInfo, reqID);

		if(err != MSG_SUCCESS)
			return err;

__BYPASS_UPDATE:
		if (msgInfo.networkStatus == MSG_NETWORK_SEND_FAIL) {
			MSG_DEBUG("message-dialog: send fail");
			MsgInsertTicker("Sending multimedia message failed.", SENDING_MULTIMEDIA_MESSAGE_FAILED);
		} else {
			MSG_DEBUG("message-dialog: send success");
			MsgInsertTicker("Multimedia message sent.", MULTIMEDIA_MESSAGE_SENT);

			MSG_DEBUG("Enter MsgAddPhoneLog() : msgInfo.addressList[0].addressVal [%s]", msgInfo.addressList[0].addressVal);
			MsgAddPhoneLog(&msgInfo);
		}

		if (prxInfo) {
			if (prxInfo->handleAddr == 0) {
				// just making data which will be passed to plugin. it indicates "handling evt success"
				MsgTransactionManager::instance()->delProxyInfo(reqID);

				return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
			}

#if defined(__x86_64__) || defined(__aarch64__)
			uint64_t ret[3] = {0}; //3// reqid, status, object
#else
			unsigned int ret[3] = {0}; //3// reqid, status, object
#endif

			ret[0] = reqID;
			ret[1] = msgInfo.networkStatus;
			ret[2] = prxInfo->handleAddr;

			// Make Event Data for APP
			eventsize = MsgMakeEvent(ret, sizeof(ret), MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);

			// Send to listener thread, here
			MsgTransactionManager::instance()->write(prxInfo->listenerFd, *ppEvent, eventsize);

			MsgTransactionManager::instance()->delProxyInfo(reqID);
		}

		msgInfo.bTextSms = true;
		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
	}

	MSG_END();
	return eventsize;
}

int MsgIncomingPushMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	int eventSize = 0;

	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	MSG_PUSH_MESSAGE_DATA_S pushData;
	memset(&pushData, 0x00, sizeof(MSG_PUSH_MESSAGE_DATA_S));

	// Get Incoming Message
	memcpy(&pushData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_PUSH_MESSAGE_DATA_S));



	// broadcast to listener threads, here
	MsgTransactionManager::instance()->broadcastPushMsgCB(MSG_SUCCESS, &pushData);

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	MSG_END();
	return eventSize;
}

int MsgIncomingCBMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	int eventSize = 0;

	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Incoming Message
	MSG_CB_MSG_S cbMsg;
	memset(&cbMsg, 0x00, sizeof(MSG_CB_MSG_S));

	memcpy(&cbMsg, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_CB_MSG_S));

	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	MsgTransactionManager::instance()->broadcastCBMsgCB(err, &cbMsg);

	bool bSave = false;
	MsgSettingGetBool(CB_SAVE, &bSave);

	if(bSave && cbMsg.type!= MSG_ETWS_SMS) {
		msgIdList.nCount = 1;
		msgIds[0] = (msg_message_id_t)cbMsg.messageId;
		msgIdList.msgIdList = msgIds;
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	}
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_CB_MSG_IND, err, (void**)ppEvent);

	MSG_END();

	return eventSize;
}

int MsgIncomingSyncMLMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	MSG_SYNCML_MESSAGE_DATA_S syncMLData;
	memset(&syncMLData, 0x00, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	// Get Incoming Message
	memcpy(&syncMLData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	int eventSize = 0;

	// broadcast to listener threads, here
	MsgTransactionManager::instance()->broadcastSyncMLMsgCB(MSG_SUCCESS, &syncMLData);

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgIncomingLBSMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	int eventSize = 0;

	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	MSG_LBS_MESSAGE_DATA_S lbsData;
	memset(&lbsData, 0x00, sizeof(MSG_LBS_MESSAGE_DATA_S));

	// Get Incoming Message
	memcpy(&lbsData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_LBS_MESSAGE_DATA_S));

	// broadcast to listener threads, here
	MsgTransactionManager::instance()->broadcastLBSMsgCB(MSG_SUCCESS, &lbsData);

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_LBS_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgSyncMLMsgOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int eventSize = 0;

	msg_message_id_t msgId = 0;
	int extId = 0;

	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	// Get Data
	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(msg_message_id_t));
	memcpy(&extId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_message_id_t)), sizeof(int));

	err = MsgStoGetSyncMLExtId(msgId, &extId);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetSyncMLExtId()");

		// broadcast to listener threads, here
		MsgTransactionManager::instance()->broadcastSyncMLMsgOperationCB(err, msgId, extId);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetSyncMLExtId()");
	}

	// Make Event Data to Client
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_SYNCML_OPERATION, err, (void**)ppEvent);

	return eventSize;
}


int MsgStorageChangeHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	// input check
	if (!pCmd || !ppEvent)
		THROW(MsgException::INVALID_PARAM, "pCmd or ppEvent is null");

	msg_storage_change_type_t storageChangeType;

	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	memcpy(&msgInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_INFO_S));
	memcpy(&storageChangeType, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), sizeof(msg_storage_change_type_t));

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int eventSize = 0;

	MSG_DEBUG("storageChangeType : [%d], msg Id : [%d]", storageChangeType, msgInfo.msgId);

	// broadcast to listener threads, here
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = msgInfo.msgId;
	msgIdList.msgIdList = msgIds;

	MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, storageChangeType, &msgIdList);

	// Make Event Data to Client
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_STORAGE_CHANGE_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}
