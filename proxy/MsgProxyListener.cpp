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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgUtilFunction.h"
#include "MsgUtilFile.h"
#include "MsgProxyListener.h"
#include "MsgGconfWrapper.h"

void MsgServerRestartCb(keynode_t *key, void* data)
{
	bool bReady = false;
	MSG_DEBUG("Message Service Running State Changed");
	// server is currently booting and service is not available until the end of booting
	MsgSettingGetBool(VCONFKEY_MSG_SERVER_READY, &bReady);
	MSG_INFO("Message Service Running State Changed bReady:(%d)", bReady);

	//bReady false indicates that server has restarted. Hence the proxylistener needs to be reset
	if (bReady == false) {
		MSG_DEBUG("Message Service Is Restarted. Resetting ProxyListener");
		MsgProxyListener::instance()->resetProxyListener();
	}
}

gboolean readSocket(GIOChannel *source, GIOCondition condition, gpointer data)
{
	MSG_BEGIN();
#if 0
	if ((G_IO_ERR & condition) || (G_IO_HUP & condition) || (G_IO_NVAL & condition))
	{
		MSG_DEBUG("IO condition Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}
#endif
	if (G_IO_ERR & condition)
	{
		MSG_ERR("IO Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	if (G_IO_HUP & condition)
	{
		MSG_ERR("socket fd Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	if (G_IO_NVAL & condition)
	{
		MSG_ERR("Invaild socket Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	char* buf = NULL;
	AutoPtr<char> eventBuf(&buf);
	unsigned int len;

	int n = MsgProxyListener::instance()->readFromSocket(&buf, &len);

	if (n > 0)
	{
		MSG_DEBUG(">>Receiving %d bytes", n);
		MsgProxyListener::instance()->handleEvent((MSG_EVENT_S*)buf);
	}
	else if (n == 0)
	{
		MSG_WARN("Server closed connection");
		MsgProxyListener::instance()->stop();
		return FALSE;
	}
	else // dataSize < 0
	{
		MSG_DEBUG("Data is not for Listener");
	}

	MSG_END();

	return TRUE;
}


/*==================================================================================================
                                     IMPLEMENTATION OF MsgListenerThread - Member Functions
==================================================================================================*/
MsgProxyListener* MsgProxyListener::pInstance = NULL;


MsgProxyListener::MsgProxyListener() : running(0)
{
	sentStatusCBList.clear();
	newMessageCBList.clear();
	newMMSConfMessageCBList.clear();
	newSyncMLMessageCBList.clear();
	newLBSMessageCBList.clear();
	openHandleSet.clear();
	MsgSettingRegVconfCBCommon(VCONFKEY_MSG_SERVER_READY, MsgServerRestartCb);
}


MsgProxyListener::~MsgProxyListener()
{
	sentStatusCBList.clear();
	newMessageCBList.clear();
	newMMSConfMessageCBList.clear();
	newSyncMLMessageCBList.clear();
	newLBSMessageCBList.clear();
	openHandleSet.clear();
	MsgSettingRemoveVconfCBCommon(VCONFKEY_MSG_SERVER_READY, MsgServerRestartCb);
}


MsgProxyListener* MsgProxyListener::instance()
{
	static Mutex mm;
	MutexLocker lock(mm);

	if (!pInstance) {
		pInstance = new MsgProxyListener();
	}

	return pInstance;
}


void MsgProxyListener::start(MsgHandle* pMsgHandle)
{
	MutexLocker lock(mx);

	handle_set::iterator it = openHandleSet.find(pMsgHandle);
	if (it == openHandleSet.end())
		openHandleSet.insert(pMsgHandle);
	if (running == 0)
	{
		cliSock.connect(MSG_SOCKET_PATH);
		cv.signal(); // wake up the waiting thread

		int fd = cliSock.fd();

		MSG_DEBUG("Socket Fd : %d", fd);

		channel = g_io_channel_unix_new(fd); // initializes ref_count = 1

		eventSourceId = g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL), &readSocket, NULL); // increments ref_count =2

		MSG_DEBUG("Call g_io_add_watch() : %d", eventSourceId);
	}

	running++;
	MSG_DEBUG("add Listener and [%d] are running.", running);
}


void MsgProxyListener::stop()
{
	MSG_BEGIN();

	if (running > 1)
	{
		running--;
		MSG_DEBUG("There are still running Listener. [%d] left.", running);
	}
	else if (running == 1)
	{
		MutexLocker lock(mx);

		running--;

		g_io_channel_unref(channel); // decrements ref_count = 1

		g_source_remove(eventSourceId);

		cliSock.close();

		MSG_DEBUG("client Listener is terminated.");
	}

	MSG_END();
}


bool MsgProxyListener::regSentStatusEventCB(MsgHandle* pMsgHandle, msg_sent_status_cb pfSentStatus, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_SENT_STATUS_CB_ITEM_S>::iterator it = sentStatusCBList.begin();

	for (; it != sentStatusCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfSentStatusCB == pfSentStatus) {
			MSG_DEBUG("msg_sent_status_cb() callback : [%p] is already registered!!!", pfSentStatus);
			return false;
		}
	}

	MSG_SENT_STATUS_CB_ITEM_S sentStatusCB = {pMsgHandle, pfSentStatus, pUserParam};

	sentStatusCBList.push_back(sentStatusCB);

	return true;
}


bool MsgProxyListener::regMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_sms_incoming_cb pfNewMessage, int port, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_INCOMING_CB_ITEM_S>::iterator it = newMessageCBList.begin();

	for (; it != newMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->port == port && it->pfIncomingCB == pfNewMessage) {
			MSG_DEBUG("msg_sms_incoming_cb() callback : Port Number [%d] is already registered!!!", port);
			return false;
		}
	}

	MSG_INCOMING_CB_ITEM_S incomingCB = {pMsgHandle, pfNewMessage, port, pUserParam};

	newMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regMMSConfMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_mms_conf_msg_incoming_cb pfNewMMSConfMessage, const char *pAppId, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_MMS_CONF_INCOMING_CB_ITEM_S>::iterator it = newMMSConfMessageCBList.begin();

	for (; it != newMMSConfMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfMMSConfIncomingCB == pfNewMMSConfMessage) {

			if(pAppId == NULL) {
				MSG_DEBUG("msg_mms_conf_msg_incoming_cb() callback is already registered!!!");
				return false;
			} else if(!strncmp(it->appId, pAppId, MAX_MMS_JAVA_APPID_LEN)) {
				MSG_DEBUG("msg_mms_conf_msg_incoming_cb() callback : AppId [%s] is already registered!!!", pAppId);
				return false;
			}
		}
	}

	MSG_MMS_CONF_INCOMING_CB_ITEM_S incomingConfCB = {pMsgHandle, pfNewMMSConfMessage, {0}, pUserParam};

	if (pAppId != NULL)
		strncpy(incomingConfCB.appId, pAppId, MAX_MMS_JAVA_APPID_LEN);

	newMMSConfMessageCBList.push_back(incomingConfCB);

	return true;
}


bool MsgProxyListener::regPushMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_push_msg_incoming_cb pfNewPushMessage, const char *pAppId, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_PUSH_INCOMING_CB_ITEM_S>::iterator it = newPushMessageCBList.begin();

	for (; it != newPushMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfPushIncomingCB == pfNewPushMessage) {

			if(pAppId == NULL) {
				MSG_DEBUG("msg_push_msg_incoming_cb() callback is already registered!!!");
				return false;
			} else if(!strncmp(it->appId, pAppId, MAX_WAPPUSH_ID_LEN)) {
				MSG_DEBUG("msg_push_msg_incoming_cb() callback : AppId [%s] is already registered!!!", pAppId);
				return false;
			}
		}
	}

	MSG_PUSH_INCOMING_CB_ITEM_S incomingPushCB = {pMsgHandle, pfNewPushMessage, {0}, pUserParam};

	if (pAppId != NULL)
		strncpy(incomingPushCB.appId, pAppId, MAX_WAPPUSH_ID_LEN);

	newPushMessageCBList.push_back(incomingPushCB);

	return true;
}

bool MsgProxyListener::regCBMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_cb_incoming_cb pfNewCBMessage, bool bSave, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_CB_INCOMING_CB_ITEM_S>::iterator it = newCBMessageCBList.begin();

	for (; it != newCBMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfCBIncomingCB == pfNewCBMessage) {
			MSG_DEBUG("msg_CB_incoming_cb() callback : [%p] is already registered!!!", pfNewCBMessage);
			 it->bsave = bSave;
			 it->userParam = pUserParam;
			return false;
		}
	}

	MSG_CB_INCOMING_CB_ITEM_S incomingCB = {pMsgHandle, pfNewCBMessage, bSave, pUserParam};

	newCBMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regReportMsgIncomingCB(MsgHandle* pMsgHandle, msg_report_msg_incoming_cb pfReportMessage, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_REPORT_INCOMING_CB_ITEM_S>::iterator it = reportMessageCBList.begin();

	for (; it != reportMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfReportMsgIncomingCB == pfReportMessage) {
			MSG_DEBUG("msg_report_msg_incoming_cb() callback : [%p] is already registered!!!", pfReportMessage);
			 it->userParam = pUserParam;
			return false;
		}
	}

	MSG_REPORT_INCOMING_CB_ITEM_S incomingCB = {pMsgHandle, pfReportMessage, pUserParam};

	reportMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regSyncMLMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_incoming_cb pfNewSyncMLMessage, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_SYNCML_INCOMING_CB_ITEM_S>::iterator it = newSyncMLMessageCBList.begin();

	for (; it != newSyncMLMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfSyncMLIncomingCB == pfNewSyncMLMessage) {
			MSG_DEBUG("msg_syncml_msg_incoming_cb() callback : [%p] is already registered!!!", pfNewSyncMLMessage);
			return false;
		}
	}

	MSG_SYNCML_INCOMING_CB_ITEM_S incomingCB = {pMsgHandle, pfNewSyncMLMessage, pUserParam};

	newSyncMLMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regLBSMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_lbs_msg_incoming_cb pfNewLBSMsgIncoming, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_LBS_INCOMING_CB_ITEM_S>::iterator it = newLBSMessageCBList.begin();

	for (; it != newLBSMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfLBSMsgIncoming == pfNewLBSMsgIncoming) {
			MSG_DEBUG("msg_lbs_msg_incoming_cb() callback : [%p] is already registered!!!", pfNewLBSMsgIncoming);
			return false;
		}
	}

	MSG_LBS_INCOMING_CB_ITEM_S incomingCB = {pMsgHandle, pfNewLBSMsgIncoming, pUserParam};

	newLBSMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regSyncMLMessageOperationEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_operation_cb pfSyncMLMessageOperation, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_SYNCML_OPERATION_CB_ITEM_S>::iterator it = operationSyncMLMessageCBList.begin();

	for (; it != operationSyncMLMessageCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfSyncMLOperationCB == pfSyncMLMessageOperation) {
			MSG_DEBUG("msg_syncml_msg_incoming_cb() callback : [%p] is already registered!!!", pfSyncMLMessageOperation);
			return false;
		}
	}

	MSG_SYNCML_OPERATION_CB_ITEM_S incomingCB = {pMsgHandle, pfSyncMLMessageOperation, pUserParam};

	operationSyncMLMessageCBList.push_back(incomingCB);

	return true;
}


bool MsgProxyListener::regStorageChangeEventCB(MsgHandle* pMsgHandle, msg_storage_change_cb pfStorageChangeOperation, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_STORAGE_CHANGE_CB_ITEM_S>::iterator it = storageChangeCBList.begin();

	for (; it != storageChangeCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle && it->pfStorageChangeCB == pfStorageChangeOperation) {
			MSG_DEBUG("msg_storage_change_cb() callback : [%p] is already registered!!!", pfStorageChangeOperation);
			return false;
		}
	}

	MSG_STORAGE_CHANGE_CB_ITEM_S changeCB = {pMsgHandle, pfStorageChangeOperation, pUserParam};

	storageChangeCBList.push_back(changeCB);

	return true;
}


void MsgProxyListener::clearListOfClosedHandle(MsgHandle* pMsgHandle)
{
	MSG_BEGIN();

	MutexLocker lock(mx);

	// sent status CB list
	std::list<MSG_SENT_STATUS_CB_ITEM_S>::iterator it = sentStatusCBList.begin();

	for (; it != sentStatusCBList.end(); )
	{
		if (it->hAddr == pMsgHandle)
		{
			sentStatusCBList.erase(it++);

			//Stop client Listener
			stop();
		}
		else
			++it;
	}

	// new message CB list
	std::list<MSG_INCOMING_CB_ITEM_S>::iterator it2 = newMessageCBList.begin();

	for (; it2 != newMessageCBList.end(); )
	{
		if (it2->hAddr == pMsgHandle)
		{
			newMessageCBList.erase(it2++);

			//Stop client Listener
			stop();
		}
		else
			++it2;
	}

	// MMS conf Message CB list
	std::list<MSG_MMS_CONF_INCOMING_CB_ITEM_S>::iterator it3 = newMMSConfMessageCBList.begin();

	for (; it3 != newMMSConfMessageCBList.end(); )
	{
		if (it3->hAddr == pMsgHandle)
		{
			newMMSConfMessageCBList.erase(it3++);

			//Stop client Listener
			stop();
		}
		else
			++it3;
	}

	// SyncML Message CB list
	std::list<MSG_SYNCML_INCOMING_CB_ITEM_S>::iterator it4 = newSyncMLMessageCBList.begin();

	for (; it4 != newSyncMLMessageCBList.end(); )
	{
		if (it4->hAddr == pMsgHandle)
		{
			newSyncMLMessageCBList.erase(it4++);

			//Stop client Listener
			stop();
		}
		else
			++it4;
	}

	// LBS Message CB list
	std::list<MSG_LBS_INCOMING_CB_ITEM_S>::iterator it5 = newLBSMessageCBList.begin();

	for (; it5 != newLBSMessageCBList.end(); )
	{
		if (it5->hAddr == pMsgHandle)
		{
			newLBSMessageCBList.erase(it5++);

			//Stop client Listener
			stop();
		}
		else
			++it5;
	}

	// Push Message CB list
	std::list<MSG_PUSH_INCOMING_CB_ITEM_S>::iterator it6 = newPushMessageCBList.begin();

	for (; it6 != newPushMessageCBList.end(); )
	{
		if (it6->hAddr == pMsgHandle)
		{
			newPushMessageCBList.erase(it6++);

			//Stop client Listener
			stop();
		}
		else
			++it6;

	}

	// CB Message CB list
	std::list<MSG_CB_INCOMING_CB_ITEM_S>::iterator it7 = newCBMessageCBList.begin();

	for (; it7 != newCBMessageCBList.end(); )
	{

		if (it7->hAddr == pMsgHandle)
		{

			newCBMessageCBList.erase(it7++);

			//Stop client Listener
			stop();
		}
		else
		{
			++it7;
		}
	}

	// Storage change Message CB list
	std::list<MSG_STORAGE_CHANGE_CB_ITEM_S>::iterator it8 = storageChangeCBList.begin();

	for (; it8 != storageChangeCBList.end(); )
	{
		if (it8->hAddr == pMsgHandle)
		{
			storageChangeCBList.erase(it8++);

			//Stop client Listener
			stop();
		}
		else
			++it8;
	}


	// Report message incoming CB list
	std::list<MSG_REPORT_INCOMING_CB_ITEM_S>::iterator it9 = reportMessageCBList.begin();
	for (; it9 != reportMessageCBList.end(); )
	{
		if (it9->hAddr == pMsgHandle)
		{
			reportMessageCBList.erase(it9++);

			//Stop client Listener
			stop();
		}
		else
			++it9;
	}


	MSG_END();
}

void MsgProxyListener::handleEvent(const MSG_EVENT_S* pMsgEvent)
{
	MSG_BEGIN();

	if (!pMsgEvent)
		THROW(MsgException::INVALID_PARAM, "pMsgEvent is NULL");

	if (pMsgEvent->eventType == MSG_EVENT_PLG_SENT_STATUS_CNF)
	{
		unsigned int chInfo[3] = {0}; //3// reqid, status, object

		memcpy(&chInfo, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(chInfo));

		msg_struct_s status = {0,};
		MSG_SENT_STATUS_S statusData = {(msg_request_id_t)chInfo[0], (msg_network_status_t)chInfo[1]};

		status.type = MSG_STRUCT_SENT_STATUS_INFO;
		status.data = (void *)&statusData;

		mx.lock();

		MsgSentStatusCBList::iterator it = sentStatusCBList.begin();

		for( ; it != sentStatusCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_sent_status_cb pfunc = it->pfSentStatusCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, (msg_struct_t)&status, param);
		}

		mx.unlock();
	}
	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_MSG_IND )
	{
		MSG_MESSAGE_INFO_S msgInfo;
		memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

		msgInfo.addressList = NULL;
		AutoPtr<MSG_ADDRESS_INFO_S> addressListBuf(&msgInfo.addressList);

		MsgDecodeMsgInfo((char *)pMsgEvent->data, &msgInfo);

		int portKey = (msgInfo.msgPort.valid)? msgInfo.msgPort.dstPort: 0;

		mx.lock();

		MsgNewMessageCBList::iterator it = newMessageCBList.begin();
		MsgNewMessageCBList matchList;

		for( ; it != newMessageCBList.end() ; it++)
		{
			if( portKey == it->port)
			{
				matchList.push_back(*it);
			}
		}

		mx.unlock();

		it = matchList.begin();

		for( ; it != matchList.end(); it++ )
		{
			MsgHandle* pHandle = it->hAddr;

			MSG_MESSAGE_HIDDEN_S msgHidden = {0,};

			msgHidden.pData = NULL;
			msgHidden.pMmsData = NULL;

			/* Allocate memory for address list of message */
			msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

			addr_list->nCount = 0;
			addr_list->msg_struct_info = (msg_struct_t *)calloc(MAX_TO_ADDRESS_CNT, sizeof(MSG_ADDRESS_INFO_S *));

			msg_struct_s *pTmp = NULL;

			for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
				addr_list->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
				pTmp = (msg_struct_s *)addr_list->msg_struct_info[i];
				pTmp->type = MSG_STRUCT_ADDRESS_INFO;
				pTmp->data = new MSG_ADDRESS_INFO_S;
				memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				addr_list->msg_struct_info[i] = (msg_struct_t)pTmp;
			}

			msgHidden.addr_list = addr_list;

			try {
				pHandle->convertMsgStruct(&msgInfo, &msgHidden);
			}
			catch (MsgException& e) {
				MSG_FATAL("%s", e.what());
			}

			msg_struct_s msg = {0,};
			msg.type = MSG_STRUCT_MESSAGE_INFO;
			msg.data = &msgHidden;

			msg_sms_incoming_cb pfunc = it->pfIncomingCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, (msg_struct_t) &msg, param);

			delete [] (char*)msgHidden.pData;
			if (msgHidden.pMmsData != NULL)
				delete [] (char*)msgHidden.pMmsData;

			// address Memory Free
			if (msgHidden.addr_list!= NULL)
			{
				for(int i=0; i<MAX_TO_ADDRESS_CNT; i++) {
					msg_struct_s * addrInfo = (msg_struct_s *)msgHidden.addr_list->msg_struct_info[i];
					delete (MSG_ADDRESS_INFO_S *)addrInfo->data;
					addrInfo->data = NULL;
					delete (msg_struct_s *)msgHidden.addr_list->msg_struct_info[i];
					msgHidden.addr_list->msg_struct_info[i] = NULL;
				}

				g_free(msgHidden.addr_list->msg_struct_info);

				delete msgHidden.addr_list;
				msgHidden.addr_list = NULL;
			}

		}

	}
	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_MMS_CONF )
	{
		MSG_MESSAGE_INFO_S msgInfo;
		memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

		msgInfo.addressList = NULL;
		AutoPtr<MSG_ADDRESS_INFO_S> addressListBuf(&msgInfo.addressList);

		MsgDecodeMsgInfo((char *)pMsgEvent->data, &msgInfo);


	 	MMS_RECV_DATA_S* pMmsRecvData = ( MMS_RECV_DATA_S*)msgInfo.msgData;

		char* appIdKey = (pMmsRecvData->msgAppId.valid)? pMmsRecvData->msgAppId.appId: NULL;

		mx.lock();

		MsgNewMMSConfMessageCBList::iterator it = newMMSConfMessageCBList.begin();
		MsgNewMMSConfMessageCBList matchList;

		for( ; it != newMMSConfMessageCBList.end() ; it++)
		{
			if(appIdKey)
			{
				if(!strcmp(appIdKey, it->appId))
					matchList.push_back(*it);
			}
			else//(appIdKey == NULL && it->appId[0] == 0)
			{
				if(it->appId[0] == 0)
					matchList.push_back(*it);
			}
		}

		mx.unlock();

		// Contents of msgData removed and replaced to retrievedFilePath for convertMsgStruct
		// it is moved from UpdateMessage in MmsPluginStorage.cpp
		char tempFileName[MSG_FILENAME_LEN_MAX+1] = {0};  // check MSG_FILENAME_LEN_MAX

		strncpy(tempFileName, pMmsRecvData->retrievedFilePath, MSG_FILENAME_LEN_MAX);

		memset(msgInfo.msgData, 0, MAX_MSG_DATA_LEN+1);
		memcpy(msgInfo.msgData, tempFileName + strlen(MSG_DATA_PATH), MAX_MSG_DATA_LEN);

		it = matchList.begin();

		for( ; it != matchList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			MSG_MESSAGE_HIDDEN_S msgHidden = {0,};

			msgHidden.pData = NULL;
			msgHidden.pMmsData = NULL;

			/* Allocate memory for address list of message */
			msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

			addr_list->nCount = 0;
			addr_list->msg_struct_info = (msg_struct_t *)calloc(MAX_TO_ADDRESS_CNT, sizeof(MSG_ADDRESS_INFO_S *));

			msg_struct_s *pTmp = NULL;

			for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
				addr_list->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
				pTmp = (msg_struct_s *)addr_list->msg_struct_info[i];
				pTmp->type = MSG_STRUCT_ADDRESS_INFO;
				pTmp->data = new MSG_ADDRESS_INFO_S;
				memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				addr_list->msg_struct_info[i] = (msg_struct_t)pTmp;
			}

			msgHidden.addr_list = addr_list;

			try {
				pHandle->convertMsgStruct(&msgInfo, &msgHidden);
			}
			catch (MsgException& e) {
				MSG_FATAL("%s", e.what());
			}

			msg_struct_s msg = {0,};
			msg.type = MSG_STRUCT_MESSAGE_INFO;
			msg.data = &msgHidden;

			msg_mms_conf_msg_incoming_cb pfunc = it->pfMMSConfIncomingCB;

			void* param = it->userParam;
			pfunc((msg_handle_t)pHandle, (msg_struct_t) &msg, param);

			delete [] (char*)msgHidden.pData;
			if (msgHidden.pMmsData != NULL)
				delete [] (char*)msgHidden.pMmsData;

			// address Memory Free
			if (msgHidden.addr_list!= NULL)
			{
				for(int i=0; i<MAX_TO_ADDRESS_CNT; i++) {
					msg_struct_s * addrInfo = (msg_struct_s *)msgHidden.addr_list->msg_struct_info[i];
					delete (MSG_ADDRESS_INFO_S *)addrInfo->data;
					addrInfo->data = NULL;
					delete (msg_struct_s *)msgHidden.addr_list->msg_struct_info[i];
					msgHidden.addr_list->msg_struct_info[i] = NULL;
				}

				g_free(msgHidden.addr_list->msg_struct_info);

				delete msgHidden.addr_list;
				msgHidden.addr_list = NULL;
			}

			// Here the retrieved message will be deleted from native storage.
			// as of now, msg which have appId is considered as JAVA MMS message and it will be sent and handled in JAVA app.
			if(appIdKey)
			{
				MSG_DEBUG("delete received JAVA MMS message:%s from native storage",tempFileName);
				pHandle->deleteMessage(msgInfo.msgId);
			}
		}
	}
	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND )
	{
		MSG_SYNCML_MESSAGE_DATA_S* pSyncMLData = (MSG_SYNCML_MESSAGE_DATA_S *)pMsgEvent->data;

		MSG_DEBUG("msgType [%d]", pSyncMLData->syncmlType);

		mx.lock();

		MsgNewSyncMLMessageCBList::iterator it = newSyncMLMessageCBList.begin();

		for( ; it != newSyncMLMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_syncml_msg_incoming_cb pfunc = it->pfSyncMLIncomingCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, pSyncMLData->syncmlType, pSyncMLData->pushBody, pSyncMLData->pushBodyLen, pSyncMLData->wspHeader, pSyncMLData->wspHeaderLen, pSyncMLData->simIndex, param);
		}

		mx.unlock();
	}
	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_LBS_MSG_IND )
	{
		MSG_LBS_MESSAGE_DATA_S* pLBSData = (MSG_LBS_MESSAGE_DATA_S *)pMsgEvent->data;

		mx.lock();

		MsgNewLBSMessageCBList::iterator it = newLBSMessageCBList.begin();

		for( ; it != newLBSMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_lbs_msg_incoming_cb pfunc = it->pfLBSMsgIncoming;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, pLBSData->pushHeader, pLBSData->pushBody, pLBSData->pushBodyLen, param);
		}

		mx.unlock();
	}
	else if ( pMsgEvent->eventType == MSG_EVENT_SYNCML_OPERATION )
	{
		int msgId;
		int extId;

		memcpy(&msgId, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(int));
		memcpy(&extId, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(int)), sizeof(int));

		MSG_DEBUG("msgId [%d]", msgId);
		MSG_DEBUG("extId [%d]", extId);

		mx.lock();

		MsgOperationSyncMLMessageCBList::iterator it = operationSyncMLMessageCBList.begin();

		for( ; it != operationSyncMLMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_syncml_msg_operation_cb pfunc = it->pfSyncMLOperationCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, msgId, extId, param);
		}

		mx.unlock();
	}
	else if (pMsgEvent->eventType == MSG_EVENT_PLG_STORAGE_CHANGE_IND)
	{
		msg_storage_change_type_t storageChangeType;
		msg_id_list_s msgIdList;
		memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

		// Decode event data
		memcpy(&storageChangeType, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(msg_storage_change_type_t));
		memcpy(&msgIdList.nCount, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(msg_storage_change_type_t)), sizeof(int));

		if(msgIdList.nCount > 0)
			msgIdList.msgIdList = (msg_message_id_t*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(msg_storage_change_type_t)+sizeof(int));
		else
			msgIdList.msgIdList = NULL;

		MSG_DEBUG("storageChangeType [%d], msgIdList.nCount [%d]", storageChangeType, msgIdList.nCount);

		mx.lock();

		MsgStorageChangeCBList::iterator it = storageChangeCBList.begin();

		for( ; it != storageChangeCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_storage_change_cb pfunc = it->pfStorageChangeCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, storageChangeType, (msg_id_list_s*)&msgIdList, param);
		}

		mx.unlock();
	}

	else if (pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_CB_MSG_IND)
	{
		MSG_CB_MSG_S *pCbMsg = (MSG_CB_MSG_S *)pMsgEvent->data;

		mx.lock();

		MsgNewCBMessageCBList::iterator it = newCBMessageCBList.begin();

		for( ; it != newCBMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;
			msg_struct_s msg = {0,};

			msg.type = MSG_STRUCT_CB_MSG;
			msg.data = pCbMsg;

			msg_cb_incoming_cb pfunc = it->pfCBIncomingCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, (msg_struct_t) &msg, param);
		}

		mx.unlock();
	}

	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND )
	{
		MSG_PUSH_MESSAGE_DATA_S* pPushData = (MSG_PUSH_MESSAGE_DATA_S *)pMsgEvent->data;

		mx.lock();

		MsgNewPushMessageCBList::iterator it = newPushMessageCBList.begin();

		for( ; it != newPushMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_push_msg_incoming_cb pfunc = it->pfPushIncomingCB;

			void* param = it->userParam;

			if(!strncmp(it->appId, pPushData->pushAppId, MAX_WAPPUSH_ID_LEN))
				pfunc((msg_handle_t)pHandle, pPushData->pushHeader, pPushData->pushBody, pPushData->pushBodyLen, param);
		}

		mx.unlock();
	}

	else if (pMsgEvent->eventType == MSG_EVENT_PLG_REPORT_MSG_INCOMING_IND)
	{
		msg_report_type_t reportType;
		msg_message_id_t msgId;
		int addr_len;
		char *addr_val;

		// Decode event data
		memcpy(&reportType, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(msg_report_type_t));
		memcpy(&msgId, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(msg_report_type_t)), sizeof(msg_message_id_t));
		memcpy(&addr_len, (void*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(msg_report_type_t)+sizeof(msg_message_id_t)), sizeof(int));
		addr_val = (char*)((char*)pMsgEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)+sizeof(msg_report_type_t)+sizeof(msg_message_id_t)+sizeof(int));
		addr_val[addr_len] = '\0';

		MSG_SEC_DEBUG("reportType [%d], msgId [%d], Address Length [%d], Address Value [%s]", reportType, msgId, addr_len, addr_val);

		mx.lock();

		MsgReportMessageCBList::iterator it = reportMessageCBList.begin();

		for( ; it != reportMessageCBList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			msg_report_msg_incoming_cb pfunc = it->pfReportMsgIncomingCB;

			void* param = it->userParam;

			pfunc((msg_handle_t)pHandle, reportType, msgId, addr_len, addr_val, param);
		}

		mx.unlock();
	}

	MSG_END();
}


int  MsgProxyListener::getRemoteFd()
{
	MSG_BEGIN();
	//MutexLocker lock(mx);

	int tmpFd = -1;
	int ret = mx.timedlock();

	if (ret != 0) {
		MSG_DEBUG("mx.timedlock fail [%d]", ret);
		return tmpFd;
	}

	tmpFd = cliSock.getRemoteFd();

	MSG_DEBUG("listener fd [%d]", tmpFd);

	if( tmpFd == -1 ) {
		ret = cv.timedwait(mx.pMutex(),1);
	}

	if (ret == ETIMEDOUT) {
		MSG_DEBUG("get listener fd TIME-OUT");
		return tmpFd;
	}

	tmpFd = cliSock.getRemoteFd();
	mx.unlock();

	MSG_END();

	return tmpFd;
}


int MsgProxyListener::readFromSocket(char** buf, unsigned int* len)
{
	return cliSock.read(buf, len);
}

void MsgProxyListener::resetProxyListener()
{
	MSG_BEGIN();
	MutexLocker lock(mx);
	handle_set::iterator it = openHandleSet.begin();
	for (; it != openHandleSet.end(); it++) {
		MsgHandle *handle = (MsgHandle *)*it;
		clearListOfClosedHandle(handle);
	}
	openHandleSet.clear();
	MSG_END();
}

#ifdef CHECK_SENT_STATUS_CALLBACK
int MsgProxyListener::getSentStatusCbCnt()
{
	int cbCnt = 0;

	cbCnt = sentStatusCBList.size();

	MSG_DEBUG("registered sent status callback count : [%d]", cbCnt);

	return cbCnt;
}
#endif
