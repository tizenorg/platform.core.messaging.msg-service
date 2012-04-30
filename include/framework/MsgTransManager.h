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
	void setSyncMLMsgCB(MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCbinfo);
	void setLBSMsgCB(MSG_CMD_REG_INCOMING_LBS_MSG_CB_S *pCbinfo);
	void setJavaMMSList(MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S *pTrId);
	void setSyncMLMsgOperationCB(MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S *pCbinfo);
	void setStorageChangeCB(int listenerFd);

	javamms_list& getJavaMMSList();

	void broadcastIncomingMsgCB(const MSG_ERROR_T err, const MSG_MESSAGE_INFO_S *msgInfo);
	void broadcastMMSConfCB(const MSG_ERROR_T err, const MSG_MESSAGE_INFO_S *msgInfo, const MMS_RECV_DATA_S *mmsRecvData);
	void broadcastSyncMLMsgCB(const MSG_ERROR_T err, const MSG_SYNCML_MESSAGE_DATA_S *syncMLData);
	void broadcastLBSMsgCB(const MSG_ERROR_T err, const MSG_LBS_MESSAGE_DATA_S *lbsData);
	void broadcastSyncMLMsgOperationCB(const MSG_ERROR_T err, const int msgId, const int extId);
	void broadcastStorageChangeCB(const MSG_ERROR_T err, const MSG_STORAGE_CHANGE_TYPE_T storageChangeType, const MSG_MSGID_LIST_S *pMsgIdList);

	void setTMStatus();
	void getTMStatus();

private:
	MsgTransactionManager();
	~MsgTransactionManager();

	void handleRequest(int fd);
	void cleanup(int fd);
	bool checkPrivilege(MSG_CMD_TYPE_T CmdType, const char *pCookie);

	static MsgTransactionManager* pInstance;

	static MsgIpcServerSocket servSock;
	bool running;

	handler_map handlerMap;

	sentmsg_map sentMsgMap; 		// req_id, listener_fd, msghandle_addr
	fd_map statusCBFdMap; 		// src_fd, true if registered

	newmsg_list newMsgCBList;	// src_fd, msgType, port if registered
	mmsconf_list newMMSConfMsgCBList;	// src_fd, msgType, port if registered
	syncmlmsg_list newSyncMLMsgCBList; 	// src_fd, msgType, port if registered
	lbsmsg_list newLBSMsgCBList; 	// src_fd, msgType, port if registered
	javamms_list javaMMSList; // trId list to distinguish sent Java MMS msg when sendconf received
	syncmlop_list operationSyncMLMsgCBList; 	// src_fd, msgType, port if registered

	fd_map storageChangeFdMap; 	// src_fd, true if registered

	Mutex mx;
	CndVar cv;
};

#endif //MSG_TRANSACTION_MANAGER_H

