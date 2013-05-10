/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MSG_TRANSACTION_MANAGER_H
#define MSG_TRANSACTION_MANAGER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <list>
#include <map>

#include "MsgMutex.h"
#include "MsgIpcSocket.h"
#include "MsgCmdTypes.h"
#include "MsgInternalTypes.h"
#include "MsgTransportTypes.h"



/*==================================================================================================
                                         DEFINITION
==================================================================================================*/
typedef std::map<MSG_CMD_TYPE_T, int (*)(const MSG_CMD_S*, char**)> handler_map;
typedef std::map<int, MSG_PROXY_INFO_S> sentmsg_map;
typedef std::map<int, bool> fd_map;
typedef std::list<MSG_CMD_REG_INCOMING_MSG_CB_S> newmsg_list;
typedef std::list<MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S>	mmsconf_list;
typedef std::list<MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S> pushmsg_list;
typedef std::list<MSG_CMD_REG_INCOMING_CB_MSG_CB_S> cbmsg_list;
typedef std::list<MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S> syncmlmsg_list;
typedef std::list<MSG_CMD_REG_INCOMING_LBS_MSG_CB_S> lbsmsg_list;
typedef std::list<MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S> javamms_list;
typedef std::list<MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S> syncmlop_list;


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
void MsgContactChangedCallback();

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgTransactionManager
{
public:
	static MsgTransactionManager* instance();

	void run();
	void write(int fd, const char* buf, int len);

	// methods for sent status event
	void insertSentMsg(int reqId, MSG_PROXY_INFO_S* pChInfo);
	MSG_PROXY_INFO_S* getProxyInfo(int reqId);
	void delProxyInfo(int reqId);

	void setSentStatusCB(int listenerFd);
	void setIncomingMsgCB(MSG_CMD_REG_INCOMING_MSG_CB_S *pCbInfo);
	void setMMSConfMsgCB(MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S *pCbinfo);
	void setPushMsgCB(MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S *pCbinfo);
	void setCBMsgCB(MSG_CMD_REG_INCOMING_CB_MSG_CB_S *pCbInfo);
	void setSyncMLMsgCB(MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCbinfo);
	void setLBSMsgCB(MSG_CMD_REG_INCOMING_LBS_MSG_CB_S *pCbinfo);
	void setJavaMMSList(MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S *pTrId);
	void setSyncMLMsgOperationCB(MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S *pCbinfo);
	void setStorageChangeCB(int listenerFd);

	javamms_list& getJavaMMSList();

	void broadcastIncomingMsgCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo);
	void broadcastMMSConfCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo, const MMS_RECV_DATA_S *mmsRecvData);
	void broadcastPushMsgCB(const msg_error_t err, const MSG_PUSH_MESSAGE_DATA_S *pushData);
	void broadcastCBMsgCB(const msg_error_t err, const MSG_CB_MSG_S *cbMsg);
	void broadcastSyncMLMsgCB(const msg_error_t err, const MSG_SYNCML_MESSAGE_DATA_S *syncMLData);
	void broadcastLBSMsgCB(const msg_error_t err, const MSG_LBS_MESSAGE_DATA_S *lbsData);
	void broadcastSyncMLMsgOperationCB(const msg_error_t err, const int msgId, const int extId);
	void broadcastStorageChangeCB(const msg_error_t err, const msg_storage_change_type_t storageChangeType, const msg_id_list_s *pMsgIdList);

	void setTMStatus();
	void getTMStatus();

private:
	MsgTransactionManager();
	~MsgTransactionManager();

	void handleRequest(int fd);
	void cleanup(int fd);
	bool checkPrivilege(int fd, MSG_CMD_TYPE_T CmdType);

	static MsgTransactionManager* pInstance;

	static MsgIpcServerSocket servSock;
	bool running;

	handler_map handlerMap;

	sentmsg_map sentMsgMap; 		// req_id, listener_fd, msghandle_addr
	fd_map statusCBFdMap; 		// src_fd, true if registered

	newmsg_list newMsgCBList;	// src_fd, msgType, port if registered
	mmsconf_list newMMSConfMsgCBList;	// src_fd, msgType, port if registered
	pushmsg_list newPushMsgCBList;	// src_fd, msgType, port if registered
	cbmsg_list newCBMsgCBList;	// src_fd, msgType, port if registered
	syncmlmsg_list newSyncMLMsgCBList; 	// src_fd, msgType, port if registered
	lbsmsg_list newLBSMsgCBList; 	// src_fd, msgType, port if registered
	javamms_list javaMMSList; // trId list to distinguish sent Java MMS msg when sendconf received
	syncmlop_list operationSyncMLMsgCBList; 	// src_fd, msgType, port if registered

	fd_map storageChangeFdMap; 	// src_fd, true if registered

	Mutex mx;
	CndVar cv;
};

#endif //MSG_TRANSACTION_MANAGER_H

