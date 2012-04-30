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
	bool regSyncMLMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_incoming_cb pfNewSyncMLMessage, void *pUserParam);
	bool regLBSMessageIncomingEventCB(MsgHandle* pMsgHandle, msg_lbs_msg_incoming_cb pfNewLBSMsgIncoming, void *pUserParam);
	bool regSyncMLMessageOperationEventCB(MsgHandle* pMsgHandle, msg_syncml_msg_operation_cb pfSyncMLMessageOperation, void *pUserParam);
	bool regStorageChangeEventCB(MsgHandle* pMsgHandle, msg_storage_change_cb pfStorageChangeOperation, void *pUserParam);

	void clearListOfClosedHandle(MsgHandle* pMsgHandle);

	void handleEvent(const MSG_EVENT_S* ptr);

	int getRemoteFd();
	int readFromSocket(char** buf, int* len);

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
	MsgNewSyncMLMessageCBList newSyncMLMessageCBList;
	MsgNewLBSMessageCBList newLBSMessageCBList;
	MsgOperationSyncMLMessageCBList operationSyncMLMessageCBList;
	MsgStorageChangeCBList storageChangeCBList;

	GIOChannel *channel;
	guint eventSourceId;
};

#endif // __MSG_PROXY_LISTENER_H__

