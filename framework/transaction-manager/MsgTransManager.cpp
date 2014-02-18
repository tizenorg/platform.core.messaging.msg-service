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

#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <security-server.h>

#include "MsgDebug.h"
#include "MsgMemory.h"
#include "MsgException.h"
#include "MsgCppTypes.h"
#include "MsgContact.h"
#include "MsgIpcSocket.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFunction.h"
#include "MsgCmdHandler.h"
#include "MsgSettingHandler.h"
#include "MsgStorageHandler.h"
#include "MsgPluginManager.h"
#include "MsgTransManager.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgContactChangedCallback()
{
	msg_id_list_s msgIdList;
	memset(&msgIdList, 0x00, sizeof(msg_id_list_s));

	MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_CONTACT, &msgIdList);
}

/*==================================================================================================
                                     IMPLEMENTATION OF MsgTransactionManager - Member Functions
==================================================================================================*/
MsgTransactionManager* MsgTransactionManager::pInstance = NULL;
MsgIpcServerSocket MsgTransactionManager::servSock;


MsgTransactionManager::MsgTransactionManager() : running(false), mx(), cv()
{
	sentMsgMap.clear();

	statusCBFdMap.clear();
	newMsgCBList.clear();
	newMMSConfMsgCBList.clear();
	newSyncMLMsgCBList.clear();
	newLBSMsgCBList.clear();
	javaMMSList.clear();
	operationSyncMLMsgCBList.clear();
	storageChangeFdMap.clear();

	handlerMap.clear();

//	Fill in mMsgHandlers, as given in the below.
	handlerMap[MSG_CMD_ADD_MSG]				= &MsgAddMessageHandler;
	handlerMap[MSG_CMD_ADD_SYNCML_MSG]		= &MsgAddSyncMLMessageHandler;
	handlerMap[MSG_CMD_UPDATE_MSG] 			= &MsgUpdateMessageHandler;
	handlerMap[MSG_CMD_UPDATE_READ] 			= &MsgUpdateReadStatusHandler;
	handlerMap[MSG_CMD_UPDATE_PROTECTED]		= &MsgUpdateProtectedStatusHandler;
	handlerMap[MSG_CMD_DELETE_MSG]			= &MsgDeleteMessageHandler;
	handlerMap[MSG_CMD_DELALL_MSGINFOLDER] 	= &MsgDeleteAllMessageInFolderHandler;
	handlerMap[MSG_CMD_MOVE_MSGTOFOLDER]	= &MsgMoveMessageToFolderHandler;
	handlerMap[MSG_CMD_MOVE_MSGTOSTORAGE]	= &MsgMoveMessageToStorageHandler;
	handlerMap[MSG_CMD_COUNT_MSG] 			= &MsgCountMessageHandler;
	handlerMap[MSG_CMD_GET_MSG]			 	= &MsgGetMessageHandler;
	handlerMap[MSG_CMD_GET_FOLDERVIEWLIST] 	= &MsgGetFolderViewListHandler;

	handlerMap[MSG_CMD_ADD_FOLDER] 			= &MsgAddFolderHandler;
	handlerMap[MSG_CMD_UPDATE_FOLDER]		= &MsgUpdateFolderHandler;
	handlerMap[MSG_CMD_DELETE_FOLDER] 		= &MsgDeleteFolderHandler;
	handlerMap[MSG_CMD_GET_FOLDERLIST] 		= &MsgGetFolderListHandler;

	handlerMap[MSG_CMD_ADD_FILTER] 			= &MsgAddFilterHandler;
	handlerMap[MSG_CMD_UPDATE_FILTER]			= &MsgUpdateFilterHandler;
	handlerMap[MSG_CMD_DELETE_FILTER] 		= &MsgDeleteFilterHandler;
	handlerMap[MSG_CMD_GET_FILTERLIST] 		= &MsgGetFilterListHandler;
	handlerMap[MSG_CMD_SET_FILTER_OPERATION] 	= &MsgSetFilterOperationHandler;
	handlerMap[MSG_CMD_GET_FILTER_OPERATION] 	= &MsgGetFilterOperationHandler;
	handlerMap[MSG_CMD_SET_FILTER_ACTIVATION] = &MsgSetFilterActivationHandler;

	handlerMap[MSG_CMD_GET_MSG_TYPE]			= &MsgGetMsgTypeHandler;

	handlerMap[MSG_CMD_SUBMIT_REQ]			= &MsgSubmitReqHandler;
	handlerMap[MSG_CMD_CANCEL_REQ]			= &MsgCancelReqHandler;

	handlerMap[MSG_CMD_REG_SENT_STATUS_CB]	= &MsgRegSentStatusCallbackHandler;
	handlerMap[MSG_CMD_REG_STORAGE_CHANGE_CB] = &MsgRegStorageChangeCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_MSG_CB]	= &MsgRegIncomingMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB]	= &MsgRegIncomingMMSConfMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_SYNCML_MSG_CB]	= &MsgRegIncomingSyncMLMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_PUSH_MSG_CB]	= &MsgRegIncomingPushMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_CB_MSG_CB]	= &MsgRegIncomingCBMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_LBS_MSG_CB]	= &MsgRegIncomingLBSMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_SYNCML_MSG_OPERATION_CB]	= &MsgRegSyncMLMsgOperationCallbackHandler;

	handlerMap[MSG_CMD_PLG_SENT_STATUS_CNF]	= &MsgSentStatusHandler;
	handlerMap[MSG_CMD_PLG_STORAGE_CHANGE_IND]	= &MsgStorageChangeHandler;
	handlerMap[MSG_CMD_PLG_INCOMING_MSG_IND]	= &MsgIncomingMsgHandler;
	handlerMap[MSG_CMD_PLG_INCOMING_MMS_CONF]	= &MsgIncomingMMSConfMsgHandler;
	handlerMap[MSG_CMD_PLG_INCOMING_PUSH_IND]	= &MsgIncomingPushMsgHandler;
	handlerMap[MSG_CMD_PLG_INCOMING_CB_IND]	= &MsgIncomingCBMsgHandler;

	handlerMap[MSG_CMD_PLG_INCOMING_SYNCML_IND] = &MsgIncomingSyncMLMsgHandler;
	handlerMap[MSG_CMD_PLG_INCOMING_LBS_IND] = &MsgIncomingLBSMsgHandler;
	handlerMap[MSG_CMD_PLG_INIT_SIM_BY_SAT]	= &MsgInitSimBySatHandler;

	handlerMap[MSG_CMD_GET_THREADVIEWLIST] 	= &MsgGetThreadViewListHandler;
	handlerMap[MSG_CMD_GET_CONVERSATIONVIEWLIST] 	= &MsgGetConversationViewListHandler;
	handlerMap[MSG_CMD_DELETE_THREADMESSAGELIST] 	= &MsgDeleteThreadMessageListHandler;

	handlerMap[MSG_CMD_GET_CONTACT_COUNT]	= &MsgCountMsgByContactHandler;
	handlerMap[MSG_CMD_GET_QUICKPANEL_DATA] 	= &MsgGetQuickPanelDataHandler;
	handlerMap[MSG_CMD_COUNT_BY_MSGTYPE] 	= &MsgCountMsgByTypeHandler;
	handlerMap[MSG_CMD_RESET_DB] 	= &MsgResetDatabaseHandler;
	handlerMap[MSG_CMD_GET_MEMSIZE] 	= &MsgGetMemSizeHandler;

	handlerMap[MSG_CMD_BACKUP_MESSAGE] 	= &MsgBackupMessageHandler;
	handlerMap[MSG_CMD_RESTORE_MESSAGE] = &MsgRestoreMessageHandler;

	handlerMap[MSG_CMD_UPDATE_THREAD_READ] = &MsgUpdateThreadReadStatusHandler;

	handlerMap[MSG_CMD_SYNCML_OPERATION] = &MsgSyncMLMsgOperationHandler;
	handlerMap[MSG_CMD_GET_REPORT_STATUS]			= &MsgGetReportStatusHandler;

	handlerMap[MSG_CMD_GET_THREAD_ID_BY_ADDRESS] = &MsgGetThreadIdByAddressHandler;
	handlerMap[MSG_CMD_GET_THREAD_INFO] = &MsgGetThreadInfoHandler;

	handlerMap[MSG_CMD_GET_SMSC_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_CB_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_SMS_SEND_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_MMS_SEND_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_MMS_RECV_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_PUSH_MSG_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_VOICE_MSG_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_GENERAL_MSG_OPT] = &MsgGetConfigHandler;
	handlerMap[MSG_CMD_GET_MSG_SIZE_OPT] = &MsgGetConfigHandler;

	handlerMap[MSG_CMD_SET_SMSC_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_CB_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_SMS_SEND_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_MMS_SEND_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_MMS_RECV_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_PUSH_MSG_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_VOICE_MSG_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_GENERAL_MSG_OPT] = &MsgSetConfigHandler;
	handlerMap[MSG_CMD_SET_MSG_SIZE_OPT] = &MsgSetConfigHandler;

	handlerMap[MSG_CMD_ADD_PUSH_EVENT] = &MsgAddPushEventHandler;
	handlerMap[MSG_CMD_DELETE_PUSH_EVENT] = &MsgDeletePushEventHandler;
	handlerMap[MSG_CMD_UPDATE_PUSH_EVENT] = &MsgUpdatePushEventHandler;
	handlerMap[MSG_CMD_DELETE_MESSAGE_BY_LIST] = &MsgDeleteMessageByListHandler;

	handlerMap[MSG_CMD_CONTACT_SYNC] = &MsgContactSyncEventHandler;
}


MsgTransactionManager::~MsgTransactionManager()
{
//	pthread_cond_init(&retCV, NULL); // = PTHREAD_COND_INITIALIZER;

}


MsgTransactionManager* MsgTransactionManager::instance()
{
	if (!pInstance)
		pInstance = new MsgTransactionManager();

	return pInstance;
}


void MsgTransactionManager::run()
{
	servSock.open(MSG_SOCKET_PATH);

	fd_set readfds = servSock.fdSet();
	int nfds = 0;

	MSG_DEBUG("Start Transaction Manager");

	while(1)
	{
		readfds = servSock.fdSet();
		nfds = servSock.maxFd();

		MSG_DEBUG("Wait For Select() : nfds %d", nfds);

		// set Status;
		setTMStatus();

		if(select(nfds, &readfds, NULL, NULL, NULL) == -1) {
			THROW(MsgException::SELECT_ERROR, "select error : %s", strerror(errno));
		}

		try
		{
			for (int i=0 ; i < nfds; i++)
			{
				if (FD_ISSET(i, &readfds))
				{
					if (i == servSock.fd()) // if it is socket connection request
						servSock.accept();
					else
						handleRequest(i);
				}
			}
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());
		}
		catch (exception& e)
		{
			MSG_FATAL("%s", e.what());
		}

		// Release Memory
		//MsgReleaseMemory();
	}
}


void MsgTransactionManager::write(int fd, const char* buf, int len)
{
	servSock.write(fd, buf, len);
}


void MsgTransactionManager::insertSentMsg(int reqId, MSG_PROXY_INFO_S* pPrxInfo)
{
	if (pPrxInfo == NULL)
		THROW(MsgException::SENT_STATUS_ERROR, "Input Parameter is NULL");

	MSG_DEBUG("msg for submit: reqId %d listenerFd %d handleAddr %x", reqId, pPrxInfo->listenerFd, pPrxInfo->handleAddr);

	fd_map::iterator it = statusCBFdMap.find(pPrxInfo->listenerFd);

	if (it == statusCBFdMap.end()) { // if the status CB is not registered
		MSG_DEBUG("No sent_status registered for fd %d", pPrxInfo->listenerFd);
	} else {
		sentMsgMap.insert(make_pair(reqId, *pPrxInfo));
	}
}


MSG_PROXY_INFO_S* MsgTransactionManager::getProxyInfo(int reqId)
{
	sentmsg_map::iterator it = sentMsgMap.find(reqId);

	if (it == sentMsgMap.end())
	{
		//THROW(MsgException::SENT_STATUS_ERROR, "No submit request for %d", reqId);
		MSG_DEBUG("No sent status cb found (exception: mobile tracker)");
		return NULL;
	}

	return &(it->second);
}


void MsgTransactionManager::delProxyInfo(int reqId)
{
	sentmsg_map::iterator it = sentMsgMap.find(reqId);

	if (it == sentMsgMap.end())
	{
		THROW(MsgException::SENT_STATUS_ERROR, "channel info does not exist");
	}

	sentMsgMap.erase(it);
}


void MsgTransactionManager::handleRequest(int fd)
{
	MSG_BEGIN();

	MSG_DEBUG("Event from fd %d", fd);

	char* buf = NULL;
	AutoPtr<char> wrap(&buf);
	int len;
	int ret = servSock.read(fd, &buf, &len);

	if( ret == CLOSE_CONNECTION_BY_SIGNAL || ret == CLOSE_CONNECTION_BY_USER || ret < 0)
	{
		MSG_DEBUG("Read value [%d]", ret);
		cleanup(fd);
		return;
	}

	if (len <= 0)
		THROW(MsgException::INVALID_RESULT, "read buffer size <= 0");

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = 0;

	// decoding cmd from APP
	MSG_CMD_S* pCmd = (MSG_CMD_S*) buf;
	MSG_DEBUG("Command Type [%d : %s]", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType));

	if (pCmd->cmdType > MSG_CMD_NUM)
		THROW(MsgException::OUT_OF_RANGE, "request CMD is not defined");

	// check privilege
	if (checkPrivilege(fd, pCmd->cmdType) == false) {
		MSG_DEBUG("No Privilege rule. Not allowed.");
#ifdef MSG_CHECK_PRIVILEGE
		eventSize = sizeof(MSG_EVENT_S);

		pEventData = new char[eventSize];

		MSG_EVENT_S* pMsgEvent = (MSG_EVENT_S*)pEventData;

		pMsgEvent->eventType = pCmd->cmdType;
		pMsgEvent->result = MSG_ERR_SECURITY_ERROR;

		MSG_DEBUG("Replying to fd [%d], size [%d]", fd, eventSize);
		servSock.write(fd, pEventData, eventSize);

		return;
#endif
	}

	// determine the handler based on pCmd->cmdType
	int (*pfHandler)(const MSG_CMD_S*, char**) = NULL;

	pfHandler = handlerMap[pCmd->cmdType];

	if (!pfHandler)
		THROW(MsgException::INVALID_PARAM, "No handler for %d", pCmd->cmdType);

	// run handler function
	eventSize = pfHandler(pCmd, &pEventData);

	if (eventSize == 0 || pEventData == NULL)
		THROW(MsgException::INVALID_RESULT, "event size = 0 or event data = NULL");

	MSG_DEBUG("Replying to fd [%d], size [%d]", fd, eventSize);

	servSock.write(fd, pEventData, eventSize);

	MSG_END();
}


// terminating the socket connection between ipc server and ipc client
void MsgTransactionManager::cleanup(int fd)
{
	MSG_BEGIN();

	servSock.close(fd);

	MSG_DEBUG("fd %d disonnected", fd);

	// remove sent msg info for fd
	sentmsg_map::iterator sentmsg_it = sentMsgMap.begin();

	for (; sentmsg_it != sentMsgMap.end(); sentmsg_it++)
	{
		if (sentmsg_it->second.listenerFd == fd)
		{
			sentmsg_it->second.listenerFd = 0;
			sentmsg_it->second.handleAddr = 0;
		}
	}

	// remove sent status callback for fd
	statusCBFdMap.erase(fd);

	// remove all newMsgCBs for fd
	newmsg_list::iterator newmsg_it = newMsgCBList.begin();

	while (newmsg_it != newMsgCBList.end())
	{
		if (newmsg_it->listenerFd == fd)
		{
			newmsg_it = newMsgCBList.erase(newmsg_it);
		}
		else
		{
			++newmsg_it;
		}
	}

	// remove all newMMSConfMsgCBs for fd
	mmsconf_list::iterator mmsconf_it = newMMSConfMsgCBList.begin();

	while (mmsconf_it != newMMSConfMsgCBList.end())
	{
		if (mmsconf_it->listenerFd == fd)
		{
			mmsconf_it = newMMSConfMsgCBList.erase(mmsconf_it);
		}
		else
		{
			++mmsconf_it;
		}
	}

	// remove all newSyncMLMsgCBs for fd
	syncmlmsg_list::iterator syncmlmsg_it = newSyncMLMsgCBList.begin();

	while (syncmlmsg_it != newSyncMLMsgCBList.end())
	{
		if (syncmlmsg_it->listenerFd == fd)
		{
			syncmlmsg_it = newSyncMLMsgCBList.erase(syncmlmsg_it);
		}
		else
		{
			++syncmlmsg_it;
		}
	}

	// remove all newLBSMsgCBs for fd
	lbsmsg_list::iterator lbsmsg_it = newLBSMsgCBList.begin();

	while (lbsmsg_it != newLBSMsgCBList.end())
	{
		if (lbsmsg_it->listenerFd == fd)
		{
			lbsmsg_it = newLBSMsgCBList.erase(lbsmsg_it);
		}
		else
		{
			++lbsmsg_it;
		}
	}

	// remove all newPushMsgCBs for fd
	pushmsg_list::iterator pushmsg_it = newPushMsgCBList.begin();

	while (pushmsg_it != newPushMsgCBList.end())
	{
		if (pushmsg_it->listenerFd == fd)
		{
			pushmsg_it = newPushMsgCBList.erase(pushmsg_it);
		}
		else
		{
			++pushmsg_it;
		}
	}

	// remove all newCBMsgCBs for fd
	cbmsg_list::iterator cbmsg_it = newCBMsgCBList.begin();
	bool bSave = false;

	while (cbmsg_it != newCBMsgCBList.end())
	{
		if (cbmsg_it->listenerFd == fd)
		{
			cbmsg_it = newCBMsgCBList.erase(cbmsg_it);
		}
		else
		{
			if(cbmsg_it->bsave == true)
				bSave = true;
			++cbmsg_it;
		}
	}

	// remove all operationSyncMLMsgCBs for fd
	syncmlop_list::iterator syncmlop_it = operationSyncMLMsgCBList.begin();

	while (syncmlop_it != operationSyncMLMsgCBList.end())
	{
		if (syncmlop_it->listenerFd == fd)
		{
			syncmlop_it = operationSyncMLMsgCBList.erase(syncmlop_it);
		}
		else
		{
			++syncmlop_it;
		}
	}

	// remove storage change callback for fd
	storageChangeFdMap.erase(fd);

	MSG_END();
}


bool MsgTransactionManager::checkPrivilege(int fd, MSG_CMD_TYPE_T CmdType)
{
	bool bAllowed = true;
	switch(CmdType)
	{
	case MSG_CMD_GET_MSG:
	case MSG_CMD_COUNT_MSG:
	case MSG_CMD_COUNT_BY_MSGTYPE:
	{
		int ret = security_server_check_privilege_by_sockfd(fd, "msg-service::read", "rw");
		if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
			MSG_DEBUG("No msg-service::read rw rule.");
			bAllowed = false;
		}
	}
	break;
	case MSG_CMD_OPEN_HANDLE:
	case MSG_CMD_SUBMIT_REQ:
	case MSG_CMD_SET_CB_OPT:
	case MSG_CMD_ADD_PUSH_EVENT:
	case MSG_CMD_DELETE_PUSH_EVENT:
	{
		int ret = security_server_check_privilege_by_sockfd(fd, "msg-service::write", "rw");
		if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
			MSG_DEBUG("No msg-service::write rw rule.");
			bAllowed = false;
		}
	}
	break;
	case MSG_CMD_REG_INCOMING_MSG_CB:
	{
		int ret = security_server_check_privilege_by_sockfd(fd, "msg-service::smstrigger", "rw");
		if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
			MSG_DEBUG("No msg-service::smstrigger rw rule.");
			bAllowed = false;
		}
	}
	break;
	case MSG_CMD_REG_INCOMING_CB_MSG_CB:
	{
		int ret = security_server_check_privilege_by_sockfd(fd, "msg-service::cellbroadcast", "rw");
		if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
			MSG_DEBUG("No msg-service::cellbroadcast rw rule.");
			bAllowed = false;
		}
	}
	break;
	case MSG_CMD_REG_INCOMING_PUSH_MSG_CB:
	{
		int ret = security_server_check_privilege_by_sockfd(fd, "msg-service::wappush", "rw");
		if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
			MSG_DEBUG("No msg-service::wappush rw rule.");
			bAllowed = false;
		}
	}
	break;
	}

	return bAllowed;
}


void MsgTransactionManager::setSentStatusCB(int listenerFd)
{
	if (listenerFd <= 0)
		THROW(MsgException::INVALID_PARAM,"InParam Error: listenerFd %d",listenerFd);

	statusCBFdMap[listenerFd] = true;
}


void MsgTransactionManager::setIncomingMsgCB(MSG_CMD_REG_INCOMING_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	newmsg_list::iterator it = newMsgCBList.begin();

	for (; it != newMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType) && (it->port == pCbInfo->port))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d, port %d", it->listenerFd, it->msgType, it->port);
			return;
		}
	}

	newMsgCBList.push_back(*pCbInfo);
}


void MsgTransactionManager::setMMSConfMsgCB(MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	mmsconf_list::iterator it = newMMSConfMsgCBList.begin();

	for (; it != newMMSConfMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType) && (!strncmp(it->appId, pCbInfo->appId, MAX_MMS_JAVA_APPID_LEN)))
		{
			MSG_DEBUG("Duplicated MMSConfMessageCB info fd:%d, mType:%d, appId:%s", it->listenerFd, it->msgType, it->appId);
			return;
		}
	}

	newMMSConfMsgCBList.push_back(*pCbInfo);
}


void MsgTransactionManager::setPushMsgCB(MSG_CMD_REG_INCOMING_PUSH_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	pushmsg_list::iterator it = newPushMsgCBList.begin();

	for (; it != newPushMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	newPushMsgCBList.push_back(*pCbInfo);
}

void MsgTransactionManager::setCBMsgCB(MSG_CMD_REG_INCOMING_CB_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	cbmsg_list::iterator it = newCBMsgCBList.begin();

	for (; it != newCBMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	if(pCbInfo->bsave)
		if(MsgSettingSetBool(CB_SAVE, pCbInfo->bsave) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetBool FAIL: CB_SAVE");


	newCBMsgCBList.push_back(*pCbInfo);
}

void MsgTransactionManager::setSyncMLMsgCB(MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	syncmlmsg_list::iterator it = newSyncMLMsgCBList.begin();

	for (; it != newSyncMLMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	newSyncMLMsgCBList.push_back(*pCbInfo);
}


void MsgTransactionManager::setLBSMsgCB(MSG_CMD_REG_INCOMING_LBS_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	lbsmsg_list::iterator it = newLBSMsgCBList.begin();

	for (; it != newLBSMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	newLBSMsgCBList.push_back(*pCbInfo);
}


void MsgTransactionManager::setJavaMMSList(MSG_CMD_REG_INCOMING_JAVAMMS_TRID_S *pTrId)
{
	if (!pTrId)
	{
		MSG_FATAL("trId NULL");
		return;
	}

	javamms_list::iterator it;

	for (it = javaMMSList.begin(); it != javaMMSList.end(); it++)
	{
		if (!strcmp(it->id, pTrId->id))
		{
			MSG_DEBUG("Duplicated javaMMS transaction Id:%s", it->id);
			return;
		}
	}

	javaMMSList.push_back(*pTrId);
}


void MsgTransactionManager::setSyncMLMsgOperationCB(MSG_CMD_REG_SYNCML_MSG_OPERATION_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	syncmlop_list::iterator it = operationSyncMLMsgCBList.begin();

	for (; it != operationSyncMLMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	operationSyncMLMsgCBList.push_back(*pCbInfo);
}


void MsgTransactionManager::setStorageChangeCB(int listenerFd)
{
	if (listenerFd <= 0)
		THROW(MsgException::INVALID_PARAM,"InParam Error: listenerFd %d", listenerFd);

	storageChangeFdMap[listenerFd] = true;
}


javamms_list& MsgTransactionManager::getJavaMMSList()
{
	return javaMMSList;
}


void MsgTransactionManager::broadcastIncomingMsgCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(msgInfo, sizeof(MSG_MESSAGE_INFO_S), MSG_EVENT_PLG_INCOMING_MSG_IND, err, (void**)(&pEventData));

	MSG_DEBUG("valid %d dstport %d", msgInfo->msgPort.valid, msgInfo->msgPort.dstPort);

	newmsg_list::iterator it = newMsgCBList.begin();

	for (; it != newMsgCBList.end(); it++)
	{
		MSG_DEBUG("fd %d dstport %d",it->listenerFd, it->port);

		if ((msgInfo->msgPort.valid == false) && (it->port == 0))
		{
			MSG_DEBUG("Send incoming normal msg to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		}
		else if ((msgInfo->msgPort.valid == true) && (it->port == msgInfo->msgPort.dstPort))
		{
			MSG_DEBUG("Send incoming port msg to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		}
	}

	MSG_END();
}


void MsgTransactionManager::broadcastMMSConfCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo, const MMS_RECV_DATA_S *mmsRecvData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(msgInfo, sizeof(MSG_MESSAGE_INFO_S), MSG_EVENT_PLG_INCOMING_MMS_CONF, err, (void**)(&pEventData));

	mmsconf_list::iterator it = newMMSConfMsgCBList.begin();

	for (; it != newMMSConfMsgCBList.end(); it++)
	{
		MSG_DEBUG("fd:%d appId:%s",it->listenerFd, it->appId);

		if (mmsRecvData->msgAppId.valid == true)
		{
			if (!strcmp(it->appId, mmsRecvData->msgAppId.appId))
			{
				MSG_DEBUG("Send incoming java msg to listener %d", it->listenerFd);
				write(it->listenerFd, pEventData, eventSize);
			}
		}
		else
		{
			if (strlen(it->appId) <= 0)
			{
				MSG_DEBUG("Send incoming normal msg to listener %d", it->listenerFd);
				write(it->listenerFd, pEventData, eventSize);
			}
		}
	}


	MSG_END();
}

void MsgTransactionManager::broadcastPushMsgCB(const msg_error_t err, const MSG_PUSH_MESSAGE_DATA_S *pushData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(pushData, sizeof(MSG_PUSH_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND, err, (void**)(&pEventData));

	pushmsg_list::iterator it = newPushMsgCBList.begin();

	for (; it != newPushMsgCBList.end(); it++)
	{
		if (!strcmp(it->appId, pushData->pushAppId))
		{
			MSG_DEBUG("Send incoming Push information to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		}
	}

	MSG_END();
}

void MsgTransactionManager::broadcastCBMsgCB(const msg_error_t err, const MSG_CB_MSG_S *cbMsg)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(cbMsg, sizeof(MSG_CB_MSG_S), MSG_EVENT_PLG_INCOMING_CB_MSG_IND, err, (void**)(&pEventData));

	cbmsg_list::iterator it = newCBMsgCBList.begin();

	for (; it != newCBMsgCBList.end(); it++)
	{
		MSG_DEBUG("Send incoming CB information to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}

	MSG_END();
}

void MsgTransactionManager::broadcastSyncMLMsgCB(const msg_error_t err, const MSG_SYNCML_MESSAGE_DATA_S *syncMLData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(syncMLData, sizeof(MSG_SYNCML_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND, err, (void**)(&pEventData));

	syncmlmsg_list::iterator it = newSyncMLMsgCBList.begin();

	for (; it != newSyncMLMsgCBList.end(); it++)
	{
		MSG_DEBUG("Send incoming SyncML information to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}

	MSG_END();
}


void MsgTransactionManager::broadcastLBSMsgCB(const msg_error_t err, const MSG_LBS_MESSAGE_DATA_S *lbsData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	int eventSize = MsgMakeEvent(lbsData, sizeof(MSG_LBS_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_LBS_MSG_IND, err, (void**)(&pEventData));

	lbsmsg_list::iterator it = newLBSMsgCBList.begin();

	for (; it != newLBSMsgCBList.end(); it++)
	{
		MSG_DEBUG("Send incoming LBS msg to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}

	MSG_END();
}


void MsgTransactionManager::broadcastSyncMLMsgOperationCB(const msg_error_t err, const int msgId, const int extId)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	// Encoding Storage Change Data
	int dataSize = MsgEncodeSyncMLOperationData(msgId, extId, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_SYNCML_OPERATION, err, (void**)(&pEventData));

	syncmlop_list::iterator it = operationSyncMLMsgCBList.begin();

	for( ; it != operationSyncMLMsgCBList.end() ; it++ )
	{
		MSG_DEBUG("Send SyncML operation to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}

	MSG_END();
}


void MsgTransactionManager::broadcastStorageChangeCB(const msg_error_t err, const msg_storage_change_type_t storageChangeType, const msg_id_list_s *pMsgIdList)
{
	MSG_BEGIN();

	if(pMsgIdList == NULL) {
		MSG_DEBUG("pMsgIdList is NULL.");
		return;
	}

	MSG_DEBUG("storageChangeType [%d]", storageChangeType);

	int dataSize = 0;

	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	// Encoding Storage Change Data
	dataSize = MsgEncodeStorageChangeData(storageChangeType, pMsgIdList, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_PLG_STORAGE_CHANGE_IND, err, (void**)(&pEventData));

	fd_map::iterator it = storageChangeFdMap.begin();

	for (; it != storageChangeFdMap.end(); it++)
	{
		MSG_DEBUG("Send Storage Change Callback to listener %d", it->first);
		write(it->first, pEventData, eventSize);
	}

	MSG_END();
}


void MsgTransactionManager::setTMStatus()
{
	MSG_BEGIN();
	mx.lock();
	cv.signal();
	mx.unlock();
	MSG_END();
}


void MsgTransactionManager::getTMStatus()
{
	MSG_BEGIN();
	mx.lock();

	int ret = 0;

	ret = cv.timedwait(mx.pMutex(), 3);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("MsgTransactionManager::getTMStatus TIME-OUT");
	}
	MSG_END();
}
