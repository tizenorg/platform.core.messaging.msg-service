/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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

#ifndef __MSG_PROXY_LISTENER_H__
#define __MSG_PROXY_LISTENER_H__


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgIpcSocket.h"
#include "MsgTypes.h"
#include "MsgMutex.h"
#include "MsgHandle.h"

#include <map>
#include <list>
#include <glib.h>


/*==================================================================================================
                                         VARIABLES
==================================================================================================*/
typedef struct
{
	MsgHandle* hAddr;
	msg_sent_status_cb pfSentStatusCB;
	void* userParam;
} MSG_SENT_STATUS_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_sms_incoming_cb pfIncomingCB;
	int port;
	void* userParam;
} MSG_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_mms_conf_msg_incoming_cb pfMMSConfIncomingCB;
	char appId[MAX_MMS_JAVA_APPID_LEN+1];
	void* userParam;
} MSG_MMS_CONF_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_push_msg_incoming_cb pfPushIncomingCB;
	char appId[MAX_WAPPUSH_ID_LEN+1];
	void* userParam;
} MSG_PUSH_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_cb_incoming_cb pfCBIncomingCB;
	bool bsave;
	void* userParam;
} MSG_CB_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_syncml_msg_incoming_cb pfSyncMLIncomingCB;
	void* userParam;
} MSG_SYNCML_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_lbs_msg_incoming_cb pfLBSMsgIncoming;
	void* userParam;
} MSG_LBS_INCOMING_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_syncml_msg_operation_cb pfSyncMLOperationCB;
	void* userParam;
} MSG_SYNCML_OPERATION_CB_ITEM_S;

typedef struct
{
	MsgHandle* hAddr;
	msg_storage_change_cb pfStorageChangeCB;
	void* userParam;
} MSG_STORAGE_CHANGE_CB_ITEM_S;


typedef std::list<MSG_SENT_STATUS_CB_ITEM_S> MsgSentStatusCBList;
typedef std::list<MSG_INCOMING_CB_ITEM_S> MsgNewMessageCBList;
typedef std::list<MSG_MMS_CONF_INCOMING_CB_ITEM_S> MsgNewMMSConfMessageCBList;
typedef std::list<MSG_PUSH_INCOMING_CB_ITEM_S> MsgNewPushMessageCBList;
typedef std::list<MSG_CB_INCOMING_CB_ITEM_S> MsgNewCBMessageCBList;
typedef std::list<MSG_SYNCML_INCOMING_CB_ITEM_S> MsgNewSyncMLMessageCBList;
typedef std::list<MSG_LBS_INCOMING_CB_ITEM_S> MsgNewLBSMessageCBList;
typedef std::list<MSG_SYNCML_OPERATION_CB_ITEM_S> MsgOperationSyncMLMessageCBList;
typedef std::list<MSG_STORAGE_CHANGE_CB_ITEM_S> MsgStorageChangeCBList;


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgProxyListener
{
public:
	static MsgProxyListener* instance();

	void start();
	void stop();

	bool regSentStatusEventCB(MsgHandle* pMsgHandle, msg_sent_status_cb pfSentStatus, void *pUserParam);
	bool regMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_sms_incoming_cb pfNewMessage, int port, void *pUserParam);
	bool regMMSConfMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_mms_conf_msg_incoming_cb pfNewMMSConfMessage, const char *pAppId, void *pUserParam);
	bool regPushMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_push_msg_incoming_cb pfNewPushMessage, const char *pAppId, void *pUserParam);
	bool regCBMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_cb_incoming_cb pfNewCBMessage, bool bSave, void *pUserParam);
	bool regSyncMLMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_incoming_cb pfNewSyncMLMessage, void *pUserParam);
	bool regLBSMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_lbs_msg_incoming_cb pfNewLBSMsgIncoming, void *pUserParam);
	bool regSyncMLMessageOperationEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_operation_cb pfSyncMLMessageOperation, void *pUserParam);
	bool regStorageChangeEventCB(MsgHandle* pMsgHandle, msg_storage_change_cb pfStorageChangeOperation, void *pUserParam);

	void clearListOfClosedHandle(MsgHandle* pMsgHandle);

	void handleEvent(const MSG_EVENT_S* ptr);

	int getRemoteFd();
	int readFromSocket(char** buf, unsigned int* len);
#ifdef CHECK_SENT_STATUS_CALLBACK
	int getSentStatusCbCnt();
#endif

private:
	MsgProxyListener();
	~MsgProxyListener();

	static MsgProxyListener* pInstance;

	unsigned int running;

	MsgIpcClientSocket cliSock;

	Mutex mx;
	CndVar cv;

	MsgSentStatusCBList sentStatusCBList;
	MsgNewMessageCBList newMessageCBList;
	MsgNewMMSConfMessageCBList newMMSConfMessageCBList;
	MsgNewPushMessageCBList newPushMessageCBList;
	MsgNewCBMessageCBList newCBMessageCBList;
	MsgNewSyncMLMessageCBList newSyncMLMessageCBList;
	MsgNewLBSMessageCBList newLBSMessageCBList;
	MsgOperationSyncMLMessageCBList operationSyncMLMessageCBList;
	MsgStorageChangeCBList storageChangeCBList;

	GIOChannel *channel;
	guint eventSourceId;
};

#endif // __MSG_PROXY_LISTENER_H__

