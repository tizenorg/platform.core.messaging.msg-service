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

#include <aul.h>
#include <bundle.h>
#include <eventsystem.h>

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
#include "MsgUtilStorage.h"
#include "MsgAlarm.h"
#include "MsgCmdHandler.h"
#include "MsgDevicedWrapper.h"
#include "MsgMmsMessage.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgSubmitReqHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;
	bool bNewMsg = true;

	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	int ret[3];
	int eventSize = 0;

	MSG_REQUEST_INFO_S reqInfo = {0, };
	MSG_PROXY_INFO_S proxyInfo = {0, };

	reqInfo.msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&reqInfo.msgInfo.addressList, unique_ptr_deleter);

	/* Get Message Request */
	memcpy(&reqInfo.reqId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(msg_request_id_t));

	/* Storing Request ID, Proxy Info for Sent Status CNF */
	memcpy(&proxyInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_request_id_t)), sizeof(MSG_PROXY_INFO_S));

	MsgDecodeMsgInfo((char *)(pCmd->cmdData+sizeof(msg_request_id_t)+sizeof(MSG_PROXY_INFO_S)), &reqInfo.msgInfo, &reqInfo.sendOptInfo);


	if (reqInfo.msgInfo.msgType.mainType == MSG_MMS_TYPE
			&& reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_MMS) {
		int fd = 0;
		memcpy(&fd, pCmd->cmdCookie, sizeof(int));
		if (reqInfo.msgInfo.bTextSms == false) {
			err = MsgMmsCheckFilepathSmack(fd, reqInfo.msgInfo.msgData);
			if (err != MSG_SUCCESS) {
				return MsgMakeEvent(NULL, 0, MSG_EVENT_SUBMIT_REQ, err, (void**)ppEvent);
			}
		}
	}

	if (reqInfo.msgInfo.msgId > 0)
		bNewMsg = false;

	/* Submit Request */
	err = MsgSubmitReq(&reqInfo, false);

	if (err == MSG_SUCCESS) {
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
		/* Retrieve MMS shall not be kept in sentMsg */
		if ((reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_MMS) ||
			(reqInfo.msgInfo.msgType.subType == MSG_FORWARD_MMS) ||
			(reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_JAVA_MMS))
		MsgTransactionManager::instance()->insertSentMsg(reqId, &proxyInfo);
	}

	/* keep transaction Id list for distinguish java MMS sent msg when sendconf received */
	if (reqInfo.msgInfo.msgType.mainType == MSG_MMS_TYPE &&
		reqInfo.msgInfo.msgType.subType == MSG_SENDREQ_JAVA_MMS) {
		MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S trId = {0};
		memcpy(&trId.id, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_REQUEST_INFO_S)+sizeof(MSG_PROXY_INFO_S)), MMS_TR_ID_LEN);

		char* pFileName;
		pFileName = strstr(reqInfo.msgInfo.msgData, "MSG_");
		strncpy(trId.pduFileName, pFileName, MAX_COMMON_INFO_SIZE);

		MSG_SEC_DEBUG("java MMS msg trId:%s filepath:%s", trId.id, reqInfo.msgInfo.msgData);

		MsgTransactionManager* tm = MsgTransactionManager::instance();
		tm->setJavaMMSList(&trId);
	}

		ret[0] = reqId;
		ret[1] = reqInfo.msgInfo.msgId;
		ret[2] = reqInfo.msgInfo.threadId;

	/* Make Event Data */
	eventSize = MsgMakeEvent(ret, sizeof(ret), MSG_EVENT_SUBMIT_REQ, err, (void**)ppEvent);

	/* reject_msg_support */
	if (((reqInfo.msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS) &&
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
	} else {
		MSG_DEBUG("No need to broadcast storage change CB");
	}

	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	MsgStoAutoDeleteConversation(reqInfo.msgInfo.threadId, &msgIdList);
	if (msgIdList.msgIdList) {
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
		delete [] (char*)msgIdList.msgIdList;
	}

	return eventSize;
}


int MsgRegSentStatusCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	int listenerFd = 0;
	memcpy(&listenerFd, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(int));
	MSG_DEBUG("Registering sent status CB for %d", listenerFd);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setSentStatusCB(listenerFd);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_SENT_STATUS_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming SMS CB for fd %d mType %d port %d", pCmdData->listenerFd, pCmdData->msgType, pCmdData->port);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setIncomingMsgCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingMMSConfMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming MMS Conf CB for fd:%d mType:%d appId:%s", pCmdData->listenerFd, pCmdData->msgType, pCmdData->appId);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setMMSConfMsgCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_MMS_CONF_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}

int MsgRegIncomingPushMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming Push Msg CB for fd:%d mType:%d appId:%s", pCmdData->listenerFd, pCmdData->msgType, pCmdData->appId);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setPushMsgCB(pCmdData);
	/* MsgTransactionManager::instance()->sendPendigPushMsg(pCmdData); */


	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_PUSH_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}



int MsgRegIncomingCBMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_CB_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_CB_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming CB Msg CB for fd:%d mType:%d: bSave: %d", pCmdData->listenerFd, pCmdData->msgType, pCmdData->bsave);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setCBMsgCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_CB_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingSyncMLMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming Sync ML Msg CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setSyncMLMsgCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_SYNCML_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingLBSMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_INCOMING_LBS_MSG_CB_S *pCmdData = (MSG_CMD_REG_INCOMING_LBS_MSG_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering incoming LBS Msg CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setLBSMsgCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_INCOMING_LBS_MSG_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegSyncMLMsgOperationCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S *pCmdData = (MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S*) pCmd->cmdData;
	MSG_DEBUG("Registering SyncML Msg ooperation CB for fd %d mType %d", pCmdData->listenerFd, pCmdData->msgType);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setSyncMLMsgOperationCB(pCmdData);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_SYNCML_MSG_OPERATION_CB, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegStorageChangeCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	int listenerFd = 0;
	memcpy(&listenerFd, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(int));
	MSG_DEBUG("Registering storage change CB for %d", listenerFd);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setStorageChangeCB(listenerFd);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_STORAGE_CHANGE_CB, MsgException::SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgRegIncomingReportMsgCallbackHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	int listenerFd = 0;
	memcpy(&listenerFd, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(int));
	MSG_DEBUG("Registering report msg incoming CB for %d", listenerFd);

	/* storing dst fd in list */
	MsgTransactionManager::instance()->setReportMsgCB(listenerFd);

	/* Make Event Data */
	int eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_REG_REPORT_MSG_INCOMING_CB, MsgException::SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgSentStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Message Request */
	MSG_SENT_STATUS_S* pStatus = (MSG_SENT_STATUS_S*) pCmd->cmdData;

	MSG_DEBUG("REQID %d, STATUS %d", pStatus->reqId, pStatus->status);

	/* storing dst fd in list */
	MSG_PROXY_INFO_S* prxInfo = MsgTransactionManager::instance()->getProxyInfo(pStatus->reqId);

	/* when no sent status cb is found */
	if (!prxInfo) {
		return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
	}

	MSG_DEBUG("REQID %d, listenerFD %d, handleAddr %x, msgId %d", pStatus->reqId, prxInfo->listenerFd, prxInfo->handleAddr, prxInfo->sentMsgId);

	/* if APP send and quit(not exist at this time), don't send the data up. */
	if (prxInfo->handleAddr == 0) {
		/* just making data which will be passed to plugin. it indicates "handling evt success" */
		MsgTransactionManager::instance()->delProxyInfo(pStatus->reqId);

		return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);
	}

	unsigned long ret[3] = {0}; /* reqid, status, object */

	ret[0] = (unsigned long)pStatus->reqId;
	ret[1] = (unsigned long)pStatus->status;
	ret[2] = prxInfo->handleAddr;

	/* Make Event Data for APP */
	int eventSize = MsgMakeEvent(ret, sizeof(ret), MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);

	/* Send to listener thread, here */
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
	bool isClass2msg = false;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Incoming Message */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	MsgDecodeMsgInfo((char *)pCmd->cmdData, &msgInfo);

	/* broadcast to listener threads, here */
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[2];
	msg_message_id_t class2msgId = 0;
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

#ifdef MSG_NOTI_INTEGRATION
	if (msgInfo.msgType.classType == MSG_CLASS_2) {
		class2msgId = msgInfo.msgId;
		isClass2msg = true;
	}
#endif

	/* normal process */
	err = MsgHandleIncomingMsg(&msgInfo, &sendNoti);

	if (msgInfo.msgType.mainType == MSG_SMS_TYPE) {
		/* Send system event */
		bundle *b = NULL;
		b = bundle_create();
		if (b) {
			char msgId[MSG_EVENT_MSG_ID_LEN] = {0, };
			snprintf(msgId, sizeof(msgId), "%u", msgInfo.msgId);
			bundle_add_str(b, EVT_KEY_MSG_ID, msgId);

			if (msgInfo.msgType.subType >= MSG_WAP_SI_SMS && msgInfo.msgType.subType <= MSG_WAP_CO_SMS) {
				bundle_add_str(b, EVT_KEY_MSG_TYPE, EVT_VAL_PUSH);
			} else {
				bundle_add_str(b, EVT_KEY_MSG_TYPE, EVT_VAL_SMS);
				bundle_add_str(b, "cmd", "incoming_msg");
				int ret = aul_launch_app_for_uid("org.tizen.msg-manager", b, msg_get_login_user());
				if (ret <= 0) {
					MSG_DEBUG("aul_launch_app_for_uid() is failed : %d", ret);
				}
			}
			eventsystem_send_system_event(SYS_EVENT_INCOMMING_MSG, b);
			bundle_free(b);
		}
	}

	if (isClass2msg == true) {
		msgIdList.nCount = 2;
		msgIds[0] = class2msgId;
		msgIds[1] = msgInfo.msgId;
		msgIdList.msgIdList = msgIds;
		isClass2msg = false;
	} else {
		msgIdList.nCount = 1;
		msgIds[0] = msgInfo.msgId;
		msgIdList.msgIdList = msgIds;
	}

	if (sendNoti == true) {
		MsgTransactionManager::instance()->broadcastIncomingMsgCB(err, &msgInfo);
		if (msgInfo.msgType.subType > MSG_TYPE0_SMS && msgInfo.msgType.subType < MSG_WAP_SI_SMS) { /* if it is replacement message. */
			MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
		} else {
			MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
		}
	} else if (msgInfo.msgPort.valid || (msgInfo.msgType.subType >= MSG_MWI_VOICE_SMS && msgInfo.msgType.subType <= MSG_MWI_OTHER_SMS)) {
		MsgTransactionManager::instance()->broadcastIncomingMsgCB(err, &msgInfo);
	} else if (msgInfo.folderId == MSG_SPAMBOX_ID) {
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	} else if (msgInfo.msgType.subType == MSG_STATUS_REPORT_SMS || msgInfo.msgType.subType == MSG_DELIVERYIND_MMS) {
		MsgTransactionManager::instance()->broadcastReportMsgCB(err, MSG_REPORT_TYPE_DELIVERY, &msgInfo);
	} else if (msgInfo.msgType.subType == MSG_READORGIND_MMS) {
		MsgTransactionManager::instance()->broadcastReportMsgCB(err, MSG_REPORT_TYPE_READ, &msgInfo);
	}

	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));
	MsgStoAutoDeleteConversation(msgInfo.threadId, &msgIdList);
	if (msgIdList.msgIdList) {
		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
		delete [] (char*)msgIdList.msgIdList;
	}

#ifdef FEATURE_SMS_CDMA
	eventSize = MsgMakeEvent(&msgInfo.msgId, sizeof(msg_message_id_t), MSG_EVENT_PLG_INCOMING_MSG_IND, err, (void**)ppEvent);
#else
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MSG_IND, err, (void**)ppEvent);
#endif

	if (msgInfo.bTextSms == false) {
		MsgDeleteFile(msgInfo.msgData); /*ipc */
		memset(msgInfo.msgData, 0x00, sizeof(msgInfo.msgData));
	}

	MSG_END();

	return eventSize;
}

int MsgIncomingMMSConfMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	msg_error_t err = MSG_SUCCESS;
	int eventsize = 0;

	MSG_MESSAGE_INFO_S msgInfo;
	msg_request_id_t reqID;

	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	memcpy(&reqID, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(msg_request_id_t));
	MsgDecodeMsgInfo((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_request_id_t), &msgInfo);

	MSG_DEBUG(" pMsg = %s, pReqId = %d ", msgInfo.msgData, reqID);
	MSG_DEBUG(" msgtype subtype is [%d]", msgInfo.msgType.subType);

	/* For Storage change callback */
	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = msgInfo.msgId;
	msgIdList.msgIdList = msgIds;

	MsgDbHandler *dbHandle = getDbHandle();
	int tmpAddrCnt = 0;
	MSG_ADDRESS_INFO_S *tmpAddr = NULL;
	int order = MsgGetContactNameOrder();
	err = MsgStoGetAddressByMsgId(dbHandle, msgInfo.msgId, order, &tmpAddrCnt, &tmpAddr);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoGetAddressByMsgId() fail.");
	}

	if (msgInfo.msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || msgInfo.msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		if (msgInfo.networkStatus != MSG_NETWORK_RETRIEVE_SUCCESS) {
			if (msgInfo.addressList) {
				delete[] msgInfo.addressList;
				msgInfo.addressList = NULL;
			}

			msgInfo.addressList = tmpAddr;
			msgInfo.nAddressCnt = tmpAddrCnt;
		} else {
			if (tmpAddr) {
				delete [] tmpAddr;
				tmpAddr = NULL;
			}
		}

		msg_thread_id_t prev_conv_id = MsgGetThreadId(dbHandle, msgInfo.msgId);
		MSG_SUB_TYPE_T recv_sub_type = msgInfo.msgType.subType; /* Check retrieve mode to determine broadcast type */

		err = MsgHandleMmsConfIncomingMsg(&msgInfo, reqID);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgHandleMmsConfIncomingMsg failed.");
			return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MMS_CONF, err, (void**)ppEvent);
		}

		MMS_RECV_DATA_S* pMmsRecvData = (MMS_RECV_DATA_S*)msgInfo.msgData;

		if (pMmsRecvData->msgAppId.valid == true) {
			MSG_DEBUG("valid : %d, appId : %s", pMmsRecvData->msgAppId.valid, pMmsRecvData->msgAppId.appId);
		} else {
			msgInfo.bTextSms = true;
			msgInfo.dataSize = 0 ;
			memset(msgInfo.msgData, 0x00, sizeof(MMS_RECV_DATA_S));
		}

		/* broadcast to listener threads, here */
		MsgTransactionManager::instance()->broadcastMMSConfCB(msgInfo.networkStatus, &msgInfo, pMmsRecvData);

		/* determine broadcast type with retrieve mode */
		if (recv_sub_type == MSG_RETRIEVE_AUTOCONF_MMS) {
			MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
		} else {
			if (prev_conv_id == msgInfo.threadId
				|| msgInfo.networkStatus == MSG_NETWORK_RETRIEVE_FAIL || msgInfo.networkStatus == MSG_NETWORK_RETRIEVE_PENDING) {
				MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
			} else {
				MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
				MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
			}
		}

		/* make return event */
		eventsize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MMS_CONF, MSG_SUCCESS, (void**)ppEvent);

	} else if (msgInfo.msgType.subType == MSG_SENDREQ_MMS || msgInfo.msgType.subType == MSG_SENDCONF_MMS) {
		if (msgInfo.addressList) {
			delete[] msgInfo.addressList;
			msgInfo.addressList = NULL;
		}

		msgInfo.addressList = tmpAddr;
		msgInfo.nAddressCnt = tmpAddrCnt;

		MSG_PROXY_INFO_S* prxInfo = MsgTransactionManager::instance()->getProxyInfo(reqID);

		/* when no sent status cb is found */
		if (prxInfo) {
			/* No need to update javaMMS sent messages */
			javamms_list& listenerList = MsgTransactionManager::instance()->getJavaMMSList();
			javamms_list::iterator it = listenerList.begin();

			MSG_DEBUG("listenerList size:%d ", listenerList.size());

			if (msgInfo.networkStatus == MSG_NETWORK_SEND_FAIL && msgInfo.msgType.subType == MSG_SENDREQ_MMS) {
				for ( ; it != listenerList.end() ; it++) {
					if (strstr(it->pduFileName, "JAVA")) {
						MSG_SEC_DEBUG("JAVA MMS fileName:%s", it->pduFileName);
						MsgDeleteFile(it->pduFileName);		/* ipc */
						listenerList.erase(it);
						goto __BYPASS_UPDATE;
					}
				}
			} else {
				/*msgData has MMS_RECV_DATA_S */
				MMS_RECV_DATA_S* pMmsRecvData = (MMS_RECV_DATA_S*)msgInfo.msgData;

				for ( ; it != listenerList.end() ; it++) {
					if (!strcmp(it->id, pMmsRecvData->szTrID)) {
						MSG_SEC_DEBUG("find sent JAVA MMS message trId:%s from listener list trId:%s", pMmsRecvData->szTrID, it->id);
						MsgDeleteFile(it->pduFileName); /* ipc */
						listenerList.erase(it);
						goto __BYPASS_UPDATE;
					}
				}
			}
		} else {
			MSG_DEBUG("prxInfo is NULL");
		}

		err = MsgHandleMmsConfIncomingMsg(&msgInfo, reqID);

		if (err != MSG_SUCCESS) {
			return MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MMS_CONF, err, (void**)ppEvent);
		}

__BYPASS_UPDATE:
		if (msgInfo.networkStatus == MSG_NETWORK_SEND_FAIL) {
			MSG_DEBUG("message-dialog: send fail");
			MsgInsertTicker("Sending multimedia message failed.", SENDING_MULTIMEDIA_MESSAGE_FAILED, true, msgInfo.msgId);
/*			MsgSoundPlayer::instance()->MsgSoundPlayStart(NULL, MSG_NORMAL_SOUND_PLAY); */
		} else {
			MSG_DEBUG("message-dialog: send success");

#if 0 /* disabled as per UX request to not show success notification : 2015. 09. 18 */
			bool bTTS = false;

			if (MsgSettingGetBool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &bTTS) != MSG_SUCCESS) {
				MSG_DEBUG("MsgSettingGetBool is failed.");
			}

			if (bTTS) {
				MsgInsertTicker("Multimedia message sent.", MULTIMEDIA_MESSAGE_SENT, false, 0);
			}

#endif
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			MSG_SEC_DEBUG("Enter MsgAddPhoneLog() : msgInfo.addressList[0].addressVal [%s]", msgInfo.addressList[0].addressVal);
			MsgAddPhoneLog(&msgInfo);
			/* Send system event */
			bundle *b = NULL;
			b = bundle_create();
			if (b) {
				bundle_add_str(b, EVT_KEY_MSG_TYPE, EVT_VAL_MMS);
				char msgId[MSG_EVENT_MSG_ID_LEN] = {0, };
				snprintf(msgId, sizeof(msgId), "%u", msgInfo.msgId);
				bundle_add_str(b, EVT_KEY_MSG_ID, msgId);
				eventsystem_send_system_event(SYS_EVENT_INCOMMING_MSG, b);
				bundle_add_str(b, "cmd", "incoming_msg");
				int ret = aul_launch_app_for_uid("org.tizen.msg-manager", b, msg_get_login_user());
				if (ret <= 0) {
					MSG_DEBUG("aul_launch_app_for_uid() is failed : %d", ret);
				}
				bundle_free(b);
			}
#endif /*MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
		}

		if (prxInfo) {
			if (prxInfo->handleAddr == 0) {
				/* just making data which will be passed to plugin. it indicates "handling evt success" */
				MsgTransactionManager::instance()->delProxyInfo(reqID);
			} else {
				unsigned long ret[3] = {0}; /* reqid, status, object */

				ret[0] = (unsigned long)reqID;
				ret[1] = (unsigned long)msgInfo.networkStatus;
				ret[2] = prxInfo->handleAddr;

				/* Make Event Data for APP */
				eventsize = MsgMakeEvent(ret, sizeof(ret), MSG_EVENT_PLG_SENT_STATUS_CNF, MSG_SUCCESS, (void**)ppEvent);

				/* Send to listener thread, here */
				MsgTransactionManager::instance()->write(prxInfo->listenerFd, *ppEvent, eventsize);

				MsgTransactionManager::instance()->delProxyInfo(reqID);
			}
		}

		eventsize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_MMS_CONF, MSG_SUCCESS, (void**)ppEvent);
		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
	} else {
		/*To avoid prevent memory leak.. this case will not occur. eventsize will be return as 0. */
		if (tmpAddr) {
			delete[] tmpAddr;
			tmpAddr = NULL;
		}
	}

	MSG_END();
	return eventsize;
}

int MsgIncomingPushMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	int eventSize = 0;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	MSG_PUSH_MESSAGE_DATA_S pushData;
	memset(&pushData, 0x00, sizeof(MSG_PUSH_MESSAGE_DATA_S));

	/* Get Incoming Message */
	memcpy(&pushData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_PUSH_MESSAGE_DATA_S));



	/* broadcast to listener threads, here */
	MsgTransactionManager::instance()->broadcastPushMsgCB(MSG_SUCCESS, &pushData);

	/* Make Event Data */
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	MSG_END();
	return eventSize;
}

int MsgIncomingCBMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	int eventSize = 0;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Incoming Message */
	MSG_CB_MSG_S cbMsg;
	memset(&cbMsg, 0x00, sizeof(MSG_CB_MSG_S));

	memcpy(&cbMsg, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_CB_MSG_S));

	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	MsgDecodeMsgInfo((char *)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_CB_MSG_S), &msgInfo);

	MSG_DEBUG("CB MSG ADDRESS COUNT=%d", msgInfo.nAddressCnt);

	if (MsgStoAddCBMsg(&msgInfo) != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoAddCBMsg is fail");
	}

	if (msgInfo.msgType.classType == MSG_CLASS_0) {
		MsgLaunchClass0(msgInfo.msgId);
		bool isFavorites = false;
		if (!checkBlockingMode(msgInfo.addressList[0].addressVal, &isFavorites)) {
			MsgPlayTTSMode(msgInfo.msgType.subType, msgInfo.msgId, isFavorites);
			MsgSoundPlayer::instance()->MsgSoundPlayStart(&(msgInfo.addressList[0]), MSG_SOUND_PLAY_USER);
		}
	} else {
		MsgInsertNotification(&msgInfo);
	}

	if (MsgCheckNotificationSettingEnable())
		MsgChangePmState();

	msg_id_list_s msgIdList;
	msg_message_id_t msgIds[1];
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	msgIdList.nCount = 1;
	msgIds[0] = (msg_message_id_t)msgInfo.msgId;
	msgIdList.msgIdList = msgIds;
	MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	MsgTransactionManager::instance()->broadcastCBMsgCB(err, &cbMsg, msgInfo.msgId);

	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_CB_MSG_IND, err, (void**)ppEvent);

	MSG_END();

	return eventSize;
}

int MsgIncomingSyncMLMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	MSG_SYNCML_MESSAGE_DATA_S syncMLData;
	memset(&syncMLData, 0x00, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	/* Get Incoming Message */
	memcpy(&syncMLData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	int eventSize = 0;

	/* broadcast to listener threads, here */
	MsgTransactionManager::instance()->broadcastSyncMLMsgCB(MSG_SUCCESS, &syncMLData);

	/* Make Event Data */
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgIncomingLBSMsgHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	int eventSize = 0;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	MSG_LBS_MESSAGE_DATA_S lbsData;
	memset(&lbsData, 0x00, sizeof(MSG_LBS_MESSAGE_DATA_S));

	/* Get Incoming Message */
	memcpy(&lbsData, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_LBS_MESSAGE_DATA_S));

	/* broadcast to listener threads, here */
	MsgTransactionManager::instance()->broadcastLBSMsgCB(MSG_SUCCESS, &lbsData);

	/* Make Event Data */
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INCOMING_LBS_MSG_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgSyncMLMsgOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	int eventSize = 0;

	msg_message_id_t msgId = 0;
	int extId = 0;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Data */
	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(msg_message_id_t));
	memcpy(&extId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_message_id_t)), sizeof(int));

	err = MsgStoGetSyncMLExtId(msgId, &extId);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Command Handle Success : MsgStoGetSyncMLExtId()");

		/* broadcast to listener threads, here */
		MsgTransactionManager::instance()->broadcastSyncMLMsgOperationCB(err, msgId, extId);
	} else {
		MSG_DEBUG("Command Handle Fail : MsgStoGetSyncMLExtId()");
	}

	/* Make Event Data to Client */
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_SYNCML_OPERATION, err, (void**)ppEvent);

	return eventSize;
}


int MsgStorageChangeHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	msg_id_list_s msgIdList;
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));
	msg_storage_change_type_t storageChangeType;

	memcpy(&msgIdList.nCount, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(int));

	msgIdList.msgIdList = new msg_message_id_t[msgIdList.nCount];
	for (int i = 0; i < msgIdList.nCount; i++) {
		memcpy(&msgIdList.msgIdList[i], (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)+i*sizeof(msg_message_id_t)), sizeof(msg_message_id_t));
	}

	memcpy(&storageChangeType, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)+(sizeof(msg_message_id_t)*msgIdList.nCount)), sizeof(msg_storage_change_type_t));

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	int eventSize = 0;

	MSG_DEBUG("msgIdList.nCount [%d]", msgIdList.nCount);
	for (int i = 0; i < msgIdList.nCount; i++) {
		MSG_DEBUG("storageChangeType : [%d], msg Id : [%d]", storageChangeType, msgIdList.msgIdList[i]);
	}

	/* broadcast to listener threads, here */
	MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, storageChangeType, &msgIdList);

	if (msgIdList.msgIdList) {
		delete [] msgIdList.msgIdList;
	}

	/* Make Event Data to Client */
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_STORAGE_CHANGE_IND, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}

int MsgResendMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;
	int eventSize = 0;

	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get the msgIdList of sending failed message. */
	int *failed_msg_list = NULL;
	int count = 0;
	unique_ptr<int*, void(*)(int**)> failed_list(&failed_msg_list, unique_ptr_deleter);


	err = MsgStoGetFailedMessage(&failed_msg_list, &count);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoGetFailedMessage() Error!! [%d]", err);
	}

	for (int i = 0; i < count ; ++i) {
		MSG_REQUEST_INFO_S reqInfo = {0};
		reqInfo.msgInfo.addressList = NULL;
		unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&reqInfo.msgInfo.addressList, unique_ptr_deleter);
		reqInfo.msgInfo.msgId = failed_msg_list[i];
		err = MsgStoGetMessage(reqInfo.msgInfo.msgId, &(reqInfo.msgInfo), &(reqInfo.sendOptInfo));

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoGetMessage() Error!! [%d]", err);
		}

		if (reqInfo.msgInfo.msgType.subType ==  MSG_GET_MMS || \
				reqInfo.msgInfo.msgType.subType  == MSG_NOTIFICATIONIND_MMS || \
				reqInfo.msgInfo.msgType.subType  == MSG_RETRIEVE_MMS) {
			MSG_WARN("retrieve pending id[%d]", reqInfo.msgInfo.msgId);
			/* For retrieving failed msg (MMS)*/
			reqInfo.msgInfo.msgType.subType = MSG_RETRIEVE_MMS;
			reqInfo.msgInfo.folderId = MSG_OUTBOX_ID; /* outbox fixed */
			reqInfo.msgInfo.networkStatus = MSG_NETWORK_RETRIEVING;
		} else {
			/* For sending failed msg */
			reqInfo.msgInfo.networkStatus = MSG_NETWORK_SENDING;
		}

		msg_id_list_s msgIdList;
		msg_message_id_t msgIds[1];
		memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

		msgIdList.nCount = 1;
		msgIds[0] = reqInfo.msgInfo.msgId;
		msgIdList.msgIdList = msgIds;

		err = MsgSubmitReq(&reqInfo, false);

		if (err == MSG_SUCCESS) {
			MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
			if (err == MSG_SUCCESS)
				MSG_DEBUG("MsgSubmitReq() Success");
			else
				MSG_DEBUG("MsgSubmitReq() Fail, [%d]", err);
		} else {
			MSG_DEBUG("MsgSubmitReq() Fail, [%d]", err);
		}
	}

	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_RESEND_MESSAGE, err, (void**)ppEvent);

	MSG_END();
	return eventSize;
}

#ifdef FEATURE_SMS_CDMA
int MsgCheckUniquenessHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	int eventSize = 0;

	/* input check */
	if (!pCmd || !ppEvent) {
		MSG_DEBUG("pCmd or ppEvent is null");
		return 0;
	}

	/* Get Incoming Message */
	bool bInsert;
	memcpy(&bInsert, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(bool));

	msg_message_id_t msgId;
	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(bool)), sizeof(msg_message_id_t));

	MSG_UNIQUE_INDEX_S p_msg;
	memcpy(&p_msg, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(bool)+sizeof(msg_message_id_t)), sizeof(MSG_UNIQUE_INDEX_S));

	MSG_DEBUG("Decoded Teleservice Msg Id = [%d]", p_msg.tele_msgId);
	MSG_DEBUG("Decoded Address = [%s]", p_msg.address);
	MSG_DEBUG("Decoded Sub Address = [%s]", p_msg.sub_address);

	err = MsgCheckUniqueness(bInsert, msgId, &p_msg);

	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_CHECK_UNIQUENESS, err, (void**)ppEvent);

	MSG_END();

	return eventSize;
}
#endif
