/*
* Copyright 2012  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.tizenopensource.org/license
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
#include "MsgUtilFile.h"
#include "MsgProxyListener.h"
#include "MsgGconfWrapper.h"


gboolean readSocket(GIOChannel *source, GIOCondition condition, gpointer data)
{
	MSG_BEGIN();

	if (G_IO_ERR & condition)
	{
		MSG_DEBUG("IO Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	if (G_IO_HUP & condition)
	{
		MSG_DEBUG("socket fd Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	if (G_IO_NVAL & condition)
	{
		MSG_DEBUG("Invaild socket Error!!! [%d]", condition);

		MsgProxyListener::instance()->stop();
		return FALSE;
	}

	char* buf = NULL;
	int len;

	int n = MsgProxyListener::instance()->readFromSocket(&buf, &len);

	if (n > 0)
	{
		MSG_DEBUG(">>Receiving %d bytes", n);
		MsgProxyListener::instance()->handleEvent((MSG_EVENT_S*)buf);
	}
	else if (n == 0)
	{
		MSG_DEBUG("Server closed connection");
		MsgProxyListener::instance()->stop();
		return FALSE;
	}
	else // dataSize < 0
	{
		MSG_DEBUG("Data is not for Listener");
	}

	if (buf)
	{
		delete [] buf;
		buf = NULL;
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
}


MsgProxyListener::~MsgProxyListener()
{
	sentStatusCBList.clear();
	newMessageCBList.clear();
	newMMSConfMessageCBList.clear();
	newSyncMLMessageCBList.clear();
	newLBSMessageCBList.clear();
}


MsgProxyListener* MsgProxyListener::instance()
{
	static Mutex mm;
	MutexLocker lock(mm);

	if (!pInstance)
		pInstance = new MsgProxyListener();

	return pInstance;
}


void MsgProxyListener::start()
{
	if (running == 0)
	{
		mx.lock();
		cliSock.connect(MSG_SOCKET_PATH);
		cv.signal(); // wake up the waiting thread
		mx.unlock();

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
		if (it->hAddr == pMsgHandle && it->pfSentStatusCB == pfSentStatus)
		{
			MSG_DEBUG("msg_sent_status_cb() callback : [%p] is already registered!!!", pfSentStatus);

			it->userParam = pUserParam;

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
		if (it->port == port && it->pfIncomingCB == pfNewMessage)
		{
			MSG_DEBUG("msg_sms_incoming_cb() callback : Port Number [%d] is already registered!!!", port);

			it->userParam = pUserParam;

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
		if (it->pfMMSConfIncomingCB == pfNewMMSConfMessage)
		{

			if(pAppId == NULL)
			{
				MSG_DEBUG("msg_mms_conf_msg_incoming_cb() callback is already registered!!!");

				return false;
			}
			else if(!strncmp(it->appId, pAppId, MAX_MMS_JAVA_APPID_LEN))
			{
				MSG_DEBUG("msg_mms_conf_msg_incoming_cb() callback : AppId [%s] is already registered!!!", pAppId);

				it->userParam = pUserParam;

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
		if (it->pfPushIncomingCB == pfNewPushMessage)
		{

			if(pAppId == NULL)
			{
				MSG_DEBUG("msg_push_msg_incoming_cb() callback is already registered!!!");

				return false;
			}
			else if(!strncmp(it->appId, pAppId, MAX_WAPPUSH_ID_LEN))
			{
				MSG_DEBUG("msg_push_msg_incoming_cb() callback : AppId [%s] is already registered!!!", pAppId);

				it->userParam = pUserParam;

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
		if (it->pfCBIncomingCB == pfNewCBMessage)
		{
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

bool MsgProxyListener::regSyncMLMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_incoming_cb pfNewSyncMLMessage, void *pUserParam)
{
	MutexLocker lock(mx);

	std::list<MSG_SYNCML_INCOMING_CB_ITEM_S>::iterator it = newSyncMLMessageCBList.begin();

	for (; it != newSyncMLMessageCBList.end(); it++)
	{
		if (it->pfSyncMLIncomingCB == pfNewSyncMLMessage)
		{
			MSG_DEBUG("msg_syncml_msg_incoming_cb() callback : [%p] is already registered!!!", pfNewSyncMLMessage);

			it->userParam = pUserParam;

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
		if (it->pfLBSMsgIncoming == pfNewLBSMsgIncoming)
		{
			MSG_DEBUG("msg_lbs_msg_incoming_cb() callback : [%p] is already registered!!!", pfNewLBSMsgIncoming);

			it->userParam = pUserParam;

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
		if (it->pfSyncMLOperationCB == pfSyncMLMessageOperation)
		{
			MSG_DEBUG("msg_syncml_msg_incoming_cb() callback : [%p] is already registered!!!", pfSyncMLMessageOperation);

			it->userParam = pUserParam;

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
		if (it->pfStorageChangeCB == pfStorageChangeOperation)
		{
			MSG_DEBUG("msg_storage_change_cb() callback : [%p] is already registered!!!", pfStorageChangeOperation);

			it->userParam = pUserParam;

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

	// sent status CB list
	std::list<MSG_SENT_STATUS_CB_ITEM_S>::iterator it = sentStatusCBList.begin();

	for (; it != sentStatusCBList.end(); it++)
	{
		if (it->hAddr == pMsgHandle)
		{
			sentStatusCBList.erase(it);
			it = sentStatusCBList.begin();

			//Stop client Listener
			stop();
		}
	}

	// new message CB list
	std::list<MSG_INCOMING_CB_ITEM_S>::iterator it2 = newMessageCBList.begin();

	for (; it2 != newMessageCBList.end(); it2++)
	{
		if (it2->hAddr == pMsgHandle)
		{
			newMessageCBList.erase(it2);
			it2 = newMessageCBList.begin();

			//Stop client Listener
			stop();
		}
	}

	// MMS conf Message CB list
	std::list<MSG_MMS_CONF_INCOMING_CB_ITEM_S>::iterator it3 = newMMSConfMessageCBList.begin();

	for (; it3 != newMMSConfMessageCBList.end(); it3++)
	{
		if (it3->hAddr == pMsgHandle)
		{
			newMMSConfMessageCBList.erase(it3);
			it3 = newMMSConfMessageCBList.begin();

			//Stop client Listener
			stop();
		}
	}

	// SyncML Message CB list
	std::list<MSG_SYNCML_INCOMING_CB_ITEM_S>::iterator it4 = newSyncMLMessageCBList.begin();

	for (; it4 != newSyncMLMessageCBList.end(); it4++)
	{
		if (it4->hAddr == pMsgHandle)
		{
			newSyncMLMessageCBList.erase(it4);
			it4 = newSyncMLMessageCBList.begin();

			//Stop client Listener
			stop();
		}
	}

	// LBS Message CB list
	std::list<MSG_LBS_INCOMING_CB_ITEM_S>::iterator it5 = newLBSMessageCBList.begin();

	for (; it5 != newLBSMessageCBList.end(); it5++)
	{
		if (it5->hAddr == pMsgHandle)
		{
			newLBSMessageCBList.erase(it5);
			it5 = newLBSMessageCBList.begin();

			//Stop client Listener
			stop();
		}
	}

	// Push Message CB list
	std::list<MSG_PUSH_INCOMING_CB_ITEM_S>::iterator it6 = newPushMessageCBList.begin();

	for (; it6 != newPushMessageCBList.end(); it6++)
	{
		if (it6->hAddr == pMsgHandle)
		{
			newPushMessageCBList.erase(it6);
			it6 = newPushMessageCBList.begin();

			//Stop client Listener
			stop();
		}

	}

	// CB Message CB list
	std::list<MSG_CB_INCOMING_CB_ITEM_S>::iterator it7 = newCBMessageCBList.begin();

	bool bSave = false;
	for (; it7 != newCBMessageCBList.end(); it7++)
	{

		if (it7->hAddr == pMsgHandle)
		{

			newCBMessageCBList.erase(it7);
			it7 = newCBMessageCBList.begin();

			//Stop client Listener
			stop();
		}
		else
		{
			if(it7->bsave == true)
				bSave = true;
		}
	}

	if(!bSave)
		MsgSettingSetBool(CB_SAVE, bSave);

	// Storage change Message CB list
	std::list<MSG_STORAGE_CHANGE_CB_ITEM_S>::iterator it8 = storageChangeCBList.begin();

	for (; it8 != storageChangeCBList.end(); it8++)
	{
		if (it8->hAddr == pMsgHandle)
		{
			storageChangeCBList.erase(it8);
			it8 = storageChangeCBList.begin();

			//Stop client Listener
			stop();
		}
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
		MSG_MESSAGE_INFO_S *pMsgInfo = (MSG_MESSAGE_INFO_S *)pMsgEvent->data;
		int portKey = (pMsgInfo->msgPort.valid)? pMsgInfo->msgPort.dstPort: 0;

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

			MSG_MESSAGE_INFO_S *pMsgInfo = (MSG_MESSAGE_INFO_S *)pMsgEvent->data;
			MSG_MESSAGE_HIDDEN_S msgHidden = {0,};

			msgHidden.pData = NULL;
			msgHidden.pMmsData = NULL;

			/* Allocate memory for address list of message */
			msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

			addr_list->nCount = 0;
			addr_list->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_ADDRESS_INFO_S *)*MAX_TO_ADDRESS_CNT];

			msg_struct_s *pTmp = NULL;

			for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
				addr_list->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
				pTmp = (msg_struct_s *)addr_list->msg_struct_info[i];
				pTmp->type = MSG_STRUCT_ADDRESS_INFO;
				pTmp->data = new MSG_ADDRESS_INFO_S;
				memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				addr_list->msg_struct_info[i] = (msg_struct_t)pTmp;
			}

			msgHidden.addr_list = addr_list;

			pHandle->convertMsgStruct(pMsgInfo, &msgHidden);

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

				delete [] msgHidden.addr_list->msg_struct_info;

				delete msgHidden.addr_list;
				msgHidden.addr_list = NULL;
			}

		}

	}
	else if ( pMsgEvent->eventType == MSG_EVENT_PLG_INCOMING_MMS_CONF )
	{
		MSG_MESSAGE_INFO_S *pMsgInfo = (MSG_MESSAGE_INFO_S *)pMsgEvent->data;
	 	MMS_RECV_DATA_S* pMmsRecvData = ( MMS_RECV_DATA_S*)pMsgInfo->msgData;

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

		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN+1);
		memcpy(pMsgInfo->msgData, tempFileName + strlen(MSG_DATA_PATH), strlen(tempFileName));

		it = matchList.begin();

		for( ; it != matchList.end() ; it++)
		{
			MsgHandle* pHandle = it->hAddr;

			MSG_MESSAGE_INFO_S *pMsgInfo = (MSG_MESSAGE_INFO_S *)pMsgEvent->data;
			MSG_MESSAGE_HIDDEN_S msgHidden = {0,};

			msgHidden.pData = NULL;
			msgHidden.pMmsData = NULL;

			/* Allocate memory for address list of message */
			msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

			addr_list->nCount = 0;
			addr_list->msg_struct_info = (msg_struct_t *)new char[sizeof(MSG_ADDRESS_INFO_S *)*MAX_TO_ADDRESS_CNT];

			msg_struct_s *pTmp = NULL;

			for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
				addr_list->msg_struct_info[i] = (msg_struct_t)new char[sizeof(msg_struct_s)];
				pTmp = (msg_struct_s *)addr_list->msg_struct_info[i];
				pTmp->type = MSG_STRUCT_ADDRESS_INFO;
				pTmp->data = new MSG_ADDRESS_INFO_S;
				memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

				addr_list->msg_struct_info[i] = (msg_struct_t)pTmp;
			}

			msgHidden.addr_list = addr_list;

			pHandle->convertMsgStruct(pMsgInfo, &msgHidden);

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

				delete [] msgHidden.addr_list->msg_struct_info;

				delete msgHidden.addr_list;
				msgHidden.addr_list = NULL;
			}

			// Here the retrieved message will be deleted from native storage.
			// as of now, msg which have appId is considered as JAVA MMS message and it will be sent and handled in JAVA app.
			if(appIdKey)
			{
				MSG_DEBUG("delete received JAVA MMS message:%s from native storage",tempFileName);
				pHandle->deleteMessage(pMsgInfo->msgId);
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

			pfunc((msg_handle_t)pHandle, pSyncMLData->syncmlType, pSyncMLData->pushBody, pSyncMLData->pushBodyLen, pSyncMLData->wspHeader, pSyncMLData->wspHeaderLen, param);
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

			pfunc((msg_handle_t)pHandle, pPushData->pushHeader, pPushData->pushBody, pPushData->pushBodyLen, param);
		}

		mx.unlock();
	}

	MSG_END();
}


int  MsgProxyListener::getRemoteFd()
{
	MutexLocker lock(mx);

	int tmpFd = cliSock.getRemoteFd();

	MSG_DEBUG("listener fd [%d]", tmpFd);

	if( tmpFd == -1 )
	{
		cv.wait(mx.pMutex());
	}

	return cliSock.getRemoteFd();
}


int MsgProxyListener::readFromSocket(char** buf, int* len)
{
	return cliSock.read(buf, len);
}
