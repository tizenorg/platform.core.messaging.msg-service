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
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>

#include <bundle_internal.h>
#include <eventsystem.h>

#include "MsgDebug.h"
#include "MsgMemory.h"
#include "MsgException.h"
#include "MsgCppTypes.h"
#include "MsgContact.h"
#include "MsgIpcSocket.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFunction.h"
#include "MsgUtilFile.h"
#include "MsgLbs.h"
#include "MsgCmdHandler.h"
#include "MsgSettingHandler.h"
#include "MsgStorageHandler.h"
#include "MsgPluginManager.h"
#include "MsgTransManager.h"

#define MSG_CHECK_PRIVILEGE
#define MSG_EVENT_MSG_ID_LEN	(32)

void MsgMakeErrorEvent(MSG_CMD_TYPE_T cmdType, msg_error_t errType, int *pEventSize, char **ppEvent)
{
	if (*ppEvent) delete [] *ppEvent;

	*pEventSize = sizeof(MSG_EVENT_S);
	*ppEvent = new char[sizeof(MSG_EVENT_S)];

	MSG_EVENT_S* pMsgEvent = (MSG_EVENT_S*)*ppEvent;

	pMsgEvent->eventType = cmdType;
	pMsgEvent->result = errType;
}

/*==================================================================================================
                                     IMPLEMENTATION OF MsgTransactionManager - Member Functions
==================================================================================================*/
MsgTransactionManager* MsgTransactionManager::pInstance = NULL;
MsgIpcServerSocket MsgTransactionManager::servSock;


MsgTransactionManager::MsgTransactionManager() : running(false), mx(), mxQ(), cv(), eventQueue()
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

	handlerMap[MSG_CMD_SUBMIT_REQ]			= &MsgSubmitReqHandler;

	handlerMap[MSG_CMD_REG_SENT_STATUS_CB]	= &MsgRegSentStatusCallbackHandler;
	handlerMap[MSG_CMD_REG_STORAGE_CHANGE_CB] = &MsgRegStorageChangeCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_MSG_CB]	= &MsgRegIncomingMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB]	= &MsgRegIncomingMMSConfMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_SYNCML_MSG_CB]	= &MsgRegIncomingSyncMLMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_PUSH_MSG_CB]	= &MsgRegIncomingPushMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_CB_MSG_CB]	= &MsgRegIncomingCBMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_INCOMING_LBS_MSG_CB]	= &MsgRegIncomingLBSMsgCallbackHandler;
	handlerMap[MSG_CMD_REG_SYNCML_MSG_OPERATION_CB]	= &MsgRegSyncMLMsgOperationCallbackHandler;
	handlerMap[MSG_CMD_REG_REPORT_MSG_INCOMING_CB] = &MsgRegIncomingReportMsgCallbackHandler;

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
	handlerMap[MSG_CMD_DELETE_THREADMESSAGELIST] 	= &MsgDeleteThreadMessageListHandler;
	handlerMap[MSG_CMD_SET_TEMP_ADDRESS_TABLE]	= &MsgSetTempAddressTableHandler;

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
	handlerMap[MSG_CMD_ADD_SIM_MSG] = &MsgAddSimMessageHandler;
	handlerMap[MSG_CMD_PLG_RESEND_MESSAGE] = &MsgResendMessageHandler;
#ifdef FEATURE_SMS_CDMA
	handlerMap[MSG_CMD_PLG_CHECK_UNIQUENESS] = &MsgCheckUniquenessHandler;
#endif
	handlerMap[MSG_CMD_UPDATE_IMSI] = &MsgUpdateIMSIHandler;
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

static void* worker_event_queue(void* arg)
{
	MsgTransactionManager::instance()->workerEventQueue();
	return NULL;
}

void MsgTransactionManager::run()
{
	servSock.open(MSG_SOCKET_PATH);

	fd_set readfds = servSock.fdSet();
	int nfds = 0;

	MSG_DEBUG("Start Transaction Manager");

	// Set Msg FW Ready Flag
	if(MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, true) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetBool FAIL : VCONFKEY_MSG_SERVER_READY");
	MSG_INFO("### VCONFKEY_MSG_SERVER_READY ###");

     /* running worker for plg task */
	pthread_t tv;
	if (pthread_create (&tv, NULL, &worker_event_queue, NULL) != 0) {
		THROW(MsgException::SERVER_READY_ERROR, "cannot create thread [%d]", errno);
	}

	while(1)
	{
		readfds = servSock.fdSet();
		nfds = servSock.maxFd();

		MSG_DEBUG("Wait For Select() : nfds %d", nfds);

		if(select(nfds, &readfds, NULL, NULL, NULL) == -1) {
			THROW(MsgException::SELECT_ERROR, "select error : %s", g_strerror(errno));
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

	MutexLocker lock(mx);

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
		MSG_DEBUG("No sent status cb found");
		return NULL;
	}

	return &(it->second);
}


void MsgTransactionManager::delProxyInfo(int reqId)
{
	MutexLocker lock(mx);

	sentmsg_map::iterator it = sentMsgMap.find(reqId);

	if (it == sentMsgMap.end())
	{
		THROW(MsgException::SENT_STATUS_ERROR, "channel info does not exist");
	}

	sentMsgMap.erase(it);
}

void MsgTransactionManager::workerEventQueue()
{
	MSG_CMD_S* pCmd = NULL;
	int (*pfHandler)(const MSG_CMD_S*, char**) = NULL;
	char* pEventData = NULL;

	int fd = -1;
	int eventSize = 0;

	while (1) {
		mxQ.lock();
		while (!eventQueue.front(&pCmd)) { /* if no item, wait */
			MSG_DEBUG ("waiting for task");
			cv.wait(mxQ.pMutex());
		}
		eventQueue.pop_front();	/* pop it from queue*/
		mxQ.unlock();

		if (!pCmd) {
			MSG_FATAL("pCmd NULL");
			continue;
		}

		memcpy (&fd, pCmd->cmdCookie, sizeof(int));
		if (fd < 0 ) {
			MSG_FATAL("fd [%d] < 0", fd);
			g_free (pCmd); pCmd = NULL;
			continue;
		}
		pfHandler = handlerMap[pCmd->cmdType];
		if (!pfHandler) {
			MSG_FATAL("No handler for %d", pCmd->cmdType);
			MsgMakeErrorEvent(pCmd->cmdType, MSG_ERR_INVALID_PARAMETER, &eventSize, &pEventData);
		} else {
			// run handler function
			eventSize = pfHandler(pCmd, &pEventData);

			if (eventSize == 0 || pEventData == NULL) {
				MSG_FATAL("event size[%d] = 0 or event data = NULL", eventSize);
				MsgMakeErrorEvent(pCmd->cmdType, MSG_ERR_INVALID_PARAMETER, &eventSize, &pEventData);
			}
		}

		MSG_DEBUG("Replying to fd [%d], size [%d]", fd, eventSize);
		servSock.write(fd, pEventData, eventSize);
		g_free (pCmd); pCmd = NULL;
		g_free (pEventData); pEventData = NULL;
	}
}

void MsgTransactionManager::handleRequest(int fd)
{
	MSG_BEGIN();

	MSG_DEBUG("Event from fd %d", fd);

	char* buf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&buf, unique_ptr_deleter);
	int len = 0;
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
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	int eventSize = 0;

	// decoding cmd from APP
	MSG_CMD_S* pCmd = (MSG_CMD_S*) buf;
	MSG_DEBUG("Command Type [%d : %s]", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType));

	if (pCmd->cmdType > MSG_CMD_NUM)
		THROW(MsgException::OUT_OF_RANGE, "request CMD is not defined");

	// check privilege
//	if (checkPrivilege(pCmd->cmdType, pCmd->cmdCookie) == false) {
	if (checkPrivilege(fd, pCmd->cmdType) == false) {
		MSG_DEBUG("No Privilege rule. Not allowed.");
#ifdef MSG_CHECK_PRIVILEGE
		MsgMakeErrorEvent(pCmd->cmdType, MSG_ERR_PERMISSION_DENIED, &eventSize, &pEventData);

		MSG_DEBUG("Replying to fd [%d], size [%d]", fd, eventSize);
		servSock.write(fd, pEventData, eventSize);

		return;
#endif
	}

	// determine the handler based on pCmd->cmdType
	int (*pfHandler)(const MSG_CMD_S*, char**) = NULL;

	switch (pCmd->cmdType) {
	case MSG_CMD_PLG_SENT_STATUS_CNF:
	case MSG_CMD_PLG_STORAGE_CHANGE_IND:
	case MSG_CMD_PLG_INCOMING_MSG_IND:
	case MSG_CMD_PLG_INCOMING_MMS_CONF:
	case MSG_CMD_PLG_INCOMING_SYNCML_IND:
	case MSG_CMD_PLG_INCOMING_LBS_IND:
	case MSG_CMD_PLG_INIT_SIM_BY_SAT:
	case MSG_CMD_PLG_INCOMING_PUSH_IND:
	case MSG_CMD_PLG_INCOMING_CB_IND: {

		MSG_CMD_S* pCmdDup = (MSG_CMD_S*) calloc (1, len); /* pCmdDup should be freed afterward */
		if (pCmdDup != NULL) {
			memcpy (pCmdDup, pCmd, len);
			memcpy (pCmdDup->cmdCookie, &fd, sizeof(int)); /* Now, cmdCookie keeps fd for return */

			mxQ.lock(); /* aquire lock before adding cmd */
			eventQueue.push_back(pCmdDup);
			cv.signal(); /* wake up worker */
			mxQ.unlock();
		}
		break;
	}
	default:
		pfHandler = handlerMap[pCmd->cmdType];
		if (!pfHandler) {
			MSG_FATAL("No handler for %d", pCmd->cmdType);
			MsgMakeErrorEvent(pCmd->cmdType, MSG_ERR_INVALID_PARAMETER, &eventSize, &pEventData);
		} else {
			// run handler function
			memcpy (pCmd->cmdCookie, &fd, sizeof(int)); /* Now, cmdCookie keeps fd for return */
			eventSize = pfHandler(pCmd, &pEventData);

			if (eventSize == 0 || pEventData == NULL) {
				MSG_FATAL("event size[%d] = 0 or event data = NULL", eventSize);
				MsgMakeErrorEvent(pCmd->cmdType, MSG_ERR_INVALID_PARAMETER, &eventSize, &pEventData);
			}
		}

		MSG_DEBUG("Replying to fd [%d], size [%d]", fd, eventSize);

		servSock.write(fd, pEventData, eventSize);
	}

	MSG_END();
}


// terminating the socket connection between ipc server and ipc client
void MsgTransactionManager::cleanup(int fd)
{
	MSG_BEGIN();

	MutexLocker lock(mx);

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

	MSG_DEBUG("After erase fd [%d], statusCBFdMap has below.", fd);
	fd_map::iterator it = statusCBFdMap.begin();
	for (; it!=statusCBFdMap.end(); ++it)
		MSG_DEBUG("[%d]", it->first);

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
	//bool bSave = false;

	while (cbmsg_it != newCBMsgCBList.end())
	{
		if (cbmsg_it->listenerFd == fd)
		{
			cbmsg_it = newCBMsgCBList.erase(cbmsg_it);
		}
		else
		{
			//if (cbmsg_it->bsave == true)
			//	bSave = true;
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

	MSG_DEBUG("After erase fd [%d], storageChangeFdMap has below.", fd);
	it = storageChangeFdMap.begin();
	for (; it!=storageChangeFdMap.end(); ++it)
		MSG_DEBUG("[%d]", it->first);

	// remove report msg incoming callback for fd
	reportMsgCBFdMap.erase(fd);

	MSG_DEBUG("After erase fd [%d], reportMsgCBFdMap has below.", fd);
	it = reportMsgCBFdMap.begin();
	for (; it!=reportMsgCBFdMap.end(); ++it)
		MSG_DEBUG("[%d]", it->first);

	MSG_END();
}


bool MsgTransactionManager::checkPrivilege(int fd, MSG_CMD_TYPE_T CmdType)
{
	bool bAllowed = true;

	int ret;
	char *peer_client = NULL;
	char *peer_user = NULL;
	char *peer_session = NULL;
	pid_t peer_pid;

	if (p_cynara == NULL) {
		if (this->initCynara() == false) {
			MSG_ERR("Cynara initialize failed. It will try again when API is called.");
			bAllowed = false;
			goto _END_OF_FUNC;
		}
	}

	ret = cynara_creds_socket_get_client(fd, client_method, &peer_client);
	if (ret != CYNARA_API_SUCCESS) {
		MSG_ERR("cynara_creds_socket_get_client() is failed [%d]", ret);
		bAllowed = false;
		goto _END_OF_FUNC;
	}

	ret = cynara_creds_socket_get_user(fd, user_method, &peer_user);
	if (ret != CYNARA_API_SUCCESS) {
		MSG_ERR("cynara_creds_socket_get_user() is failed [%d]", ret);
		bAllowed = false;
		goto _END_OF_FUNC;
	}

	ret =  cynara_creds_socket_get_pid(fd, &peer_pid);
	if (ret != CYNARA_API_SUCCESS) {
		MSG_ERR("cynara_creds_socket_get_pid() is failed [%d]", ret);
		bAllowed = false;
		goto _END_OF_FUNC;
	}

	peer_session = cynara_session_from_pid(peer_pid);
	if (peer_session == NULL) {
		MSG_ERR("cynara_session_from_pid() is failed");
		bAllowed = false;
		goto _END_OF_FUNC;
	}

	switch(CmdType)
	{
	case MSG_CMD_GET_MSG:
	case MSG_CMD_COUNT_MSG:
	case MSG_CMD_COUNT_BY_MSGTYPE:
	case MSG_CMD_REG_INCOMING_MSG_CB:
	case MSG_CMD_REG_INCOMING_CB_MSG_CB:
	case MSG_CMD_REG_INCOMING_PUSH_MSG_CB:
	case MSG_CMD_REG_SENT_STATUS_CB:
	case MSG_CMD_REG_INCOMING_MMS_CONF_MSG_CB:
	case MSG_CMD_REG_INCOMING_SYNCML_MSG_CB:
	case MSG_CMD_REG_INCOMING_LBS_MSG_CB:
	case MSG_CMD_REG_SYNCML_MSG_OPERATION_CB:
	case MSG_CMD_REG_REPORT_MSG_INCOMING_CB:
	case MSG_CMD_GET_CONTACT_COUNT:
	case MSG_CMD_GET_FOLDERLIST:
	case MSG_CMD_GET_QUICKPANEL_DATA:
	case MSG_CMD_GET_MEMSIZE:
	case MSG_CMD_BACKUP_MESSAGE:
	case MSG_CMD_REG_STORAGE_CHANGE_CB:
	case MSG_CMD_GET_REPORT_STATUS:
	case MSG_CMD_GET_THREAD_ID_BY_ADDRESS:
	case MSG_CMD_GET_THREAD_INFO:
	case MSG_CMD_SYNCML_OPERATION:
	case MSG_CMD_GET_FILTERLIST:
	case MSG_CMD_GET_FILTER_OPERATION:
	case MSG_CMD_GET_SMSC_OPT:
	case MSG_CMD_GET_CB_OPT:
	case MSG_CMD_GET_SMS_SEND_OPT:
	case MSG_CMD_GET_MMS_SEND_OPT:
	case MSG_CMD_GET_MMS_RECV_OPT:
	case MSG_CMD_GET_PUSH_MSG_OPT:
	case MSG_CMD_GET_VOICE_MSG_OPT:
	case MSG_CMD_GET_GENERAL_MSG_OPT:
	case MSG_CMD_GET_MSG_SIZE_OPT:
	{
		ret = cynara_check(p_cynara, peer_client, peer_session, peer_user,
				"http://tizen.org/privilege/message.read");
		if (ret != CYNARA_API_ACCESS_ALLOWED) {
			MSG_INFO("privilege [read] not allowd : [%d]", ret);
			bAllowed = false;
		}
	}
	break;
	case MSG_CMD_SUBMIT_REQ:
	case MSG_CMD_SET_CB_OPT:
	case MSG_CMD_ADD_PUSH_EVENT:
	case MSG_CMD_DELETE_PUSH_EVENT:
	case MSG_CMD_UPDATE_PUSH_EVENT:
	case MSG_CMD_ADD_MSG:
	case MSG_CMD_ADD_SYNCML_MSG:
	case MSG_CMD_UPDATE_MSG:
	case MSG_CMD_UPDATE_READ:
	case MSG_CMD_UPDATE_PROTECTED:
	case MSG_CMD_DELETE_MSG:
	case MSG_CMD_DELALL_MSGINFOLDER:
	case MSG_CMD_MOVE_MSGTOFOLDER:
	case MSG_CMD_MOVE_MSGTOSTORAGE:
	case MSG_CMD_DELETE_THREADMESSAGELIST:
	case MSG_CMD_ADD_FOLDER:
	case MSG_CMD_UPDATE_FOLDER:
	case MSG_CMD_DELETE_FOLDER:
	case MSG_CMD_RESET_DB:
	case MSG_CMD_RESTORE_MESSAGE:
	case MSG_CMD_DELETE_MESSAGE_BY_LIST:
	case MSG_CMD_UPDATE_THREAD_READ:
	case MSG_CMD_ADD_FILTER:
	case MSG_CMD_UPDATE_FILTER:
	case MSG_CMD_DELETE_FILTER:
	case MSG_CMD_SET_FILTER_OPERATION:
	case MSG_CMD_SET_FILTER_ACTIVATION:
	case MSG_CMD_SET_SMSC_OPT:
	case MSG_CMD_SET_SMS_SEND_OPT:
	case MSG_CMD_SET_MMS_SEND_OPT:
	case MSG_CMD_SET_MMS_RECV_OPT:
	case MSG_CMD_SET_PUSH_MSG_OPT:
	case MSG_CMD_SET_VOICE_MSG_OPT:
	case MSG_CMD_SET_GENERAL_MSG_OPT:
	case MSG_CMD_SET_MSG_SIZE_OPT:
	{
		ret = cynara_check(p_cynara, peer_client, peer_session, peer_user,
				"http://tizen.org/privilege/message.write");
		if (ret != CYNARA_API_ACCESS_ALLOWED) {
			MSG_INFO("privilege [write] not allowd : [%d]", ret);
			bAllowed = false;
		}
	}
	break;
	}

_END_OF_FUNC:
	MSG_FREE(peer_client);
	MSG_FREE(peer_user);
	MSG_FREE(peer_session);

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

	MutexLocker lock(mx);

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

	MutexLocker lock(mx);

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

	MutexLocker lock(mx);

	pushmsg_list::iterator it = newPushMsgCBList.begin();

	for (; it != newPushMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType) && !strncmp(it->appId, pCbInfo->appId, MAX_WAPPUSH_ID_LEN))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}

	newPushMsgCBList.push_back(*pCbInfo);
}

void MsgTransactionManager::setCBMsgCB(MSG_CMD_REG_INCOMING_CB_MSG_CB_S *pCbInfo)
{
	MSG_BEGIN();
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	MutexLocker lock(mx);

	cbmsg_list::iterator it = newCBMsgCBList.begin();

	for (; it != newCBMsgCBList.end(); it++)
	{
		if ((it->listenerFd == pCbInfo->listenerFd) && (it->msgType == pCbInfo->msgType))
		{
			MSG_DEBUG("Duplicated messageCB info fd %d, mType %d", it->listenerFd, it->msgType);
			return;
		}
	}
	MSG_DEBUG("bSave : [%d]", pCbInfo->bsave);

	if(pCbInfo->bsave)
		if(MsgSettingSetBool(CB_SAVE, pCbInfo->bsave) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetBool FAIL: CB_SAVE");


	newCBMsgCBList.push_back(*pCbInfo);

	MSG_END();
}


void MsgTransactionManager::setSyncMLMsgCB(MSG_CMD_REG_INCOMING_SYNCML_MSG_CB_S *pCbInfo)
{
	if (!pCbInfo)
	{
		MSG_FATAL("cbinfo NULL");
		return;
	}

	MutexLocker lock(mx);

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

	MutexLocker lock(mx);

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
			MSG_SEC_DEBUG("Duplicated javaMMS transaction Id:%s", it->id);
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

	MutexLocker lock(mx);

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

	MutexLocker lock(mx);

	storageChangeFdMap[listenerFd] = true;
}


void MsgTransactionManager::setReportMsgCB(int listenerFd)
{
	if (listenerFd <= 0)
		THROW(MsgException::INVALID_PARAM,"InParam Error: listenerFd %d", listenerFd);

	MutexLocker lock(mx);

	reportMsgCBFdMap[listenerFd] = true;
}


javamms_list& MsgTransactionManager::getJavaMMSList()
{
	return javaMMSList;
}


void MsgTransactionManager::broadcastIncomingMsgCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo)
{
	MSG_BEGIN();

	if ((msgInfo->msgPort.valid == true) && (msgInfo->msgPort.dstPort == MSG_LBS_PORT)) {
		MSG_DEBUG("Message for LBS.");

		if (msgInfo->bTextSms == false) {
			MSG_DEBUG("msgInfo->bTextSms == false");

			int fileSize = 0;

			char* pFileData = NULL;
			unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

			if (MsgOpenAndReadFile(msgInfo->msgData, &pFileData, &fileSize) == true)
				MsgLbsSms(pFileData, fileSize);
			else
				MSG_DEBUG("MsgOpenAndReadFile failed.");
		} else {
			MsgLbsSms(msgInfo->msgText, (int)msgInfo->dataSize);
		}
		return;
	}

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(msgInfo, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_PLG_INCOMING_MSG_IND, err, (void**)(&pEventData));

	MSG_DEBUG("valid %d dstport %d", msgInfo->msgPort.valid, msgInfo->msgPort.dstPort);

	MutexLocker lock(mx);

	newmsg_list::iterator it = newMsgCBList.begin();

	for (; it != newMsgCBList.end(); it++)
	{
		MSG_DEBUG("fd %d dstport %d",it->listenerFd, it->port);

		if ((msgInfo->msgPort.valid == false) && (it->port == 0)) {
			MSG_DEBUG("Send incoming normal msg to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		} else if ((msgInfo->msgPort.valid == true) && (it->port == msgInfo->msgPort.dstPort)) {
			MSG_DEBUG("Send incoming port msg to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		}
	}

	/* Send system event */
	bundle *b = NULL;
	b = bundle_create();
	if (b) {
		if (msgInfo->msgType.subType >= MSG_WAP_SI_SMS && msgInfo->msgType.subType <= MSG_WAP_CO_SMS) {
			bundle_add(b, EVT_KEY_MSG_TYPE, EVT_VAL_PUSH);
		} else {
			bundle_add(b, EVT_KEY_MSG_TYPE, EVT_VAL_SMS);
		}
		char msgId[MSG_EVENT_MSG_ID_LEN] = {0,};
		snprintf(msgId, sizeof(msgId), "%u", msgInfo->msgId);
		bundle_add(b, EVT_KEY_MSG_ID, msgId);
		eventsystem_send_system_event(SYS_EVENT_INCOMMING_MSG, b);
		bundle_free(b);
	}

	MSG_END();
}


void MsgTransactionManager::broadcastMMSConfCB(const msg_error_t err, const MSG_MESSAGE_INFO_S *msgInfo, const MMS_RECV_DATA_S *mmsRecvData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(msgInfo, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_PLG_INCOMING_MMS_CONF, err, (void**)(&pEventData));

	MutexLocker lock(mx);

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
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	int eventSize = MsgMakeEvent(pushData, sizeof(MSG_PUSH_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

	pushmsg_list::iterator it = newPushMsgCBList.begin();

	for (; it != newPushMsgCBList.end(); it++)
	{
		MSG_DEBUG("registered_appid : %s, incoming_appid: %s", it->appId, pushData->pushAppId);
		if (!strcmp(it->appId, pushData->pushAppId))
		{
			MSG_DEBUG("Send incoming Push information to listener %d", it->listenerFd);
			write(it->listenerFd, pEventData, eventSize);
		}
	}

	MSG_END();
}

void MsgTransactionManager::broadcastCBMsgCB(const msg_error_t err, const MSG_CB_MSG_S *cbMsg, msg_message_id_t cbMsgId)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	int eventSize = MsgMakeEvent(cbMsg, sizeof(MSG_CB_MSG_S), MSG_EVENT_PLG_INCOMING_CB_MSG_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

	cbmsg_list::iterator it = newCBMsgCBList.begin();

	for (; it != newCBMsgCBList.end(); it++)
	{
		MSG_DEBUG("Send incoming CB information to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}

	/* Send system event */
	bundle *b = NULL;
	b = bundle_create();
	if (b) {
		bundle_add(b, EVT_KEY_MSG_TYPE, EVT_VAL_CB);
		char msgId[MSG_EVENT_MSG_ID_LEN] = {0,};
		snprintf(msgId, sizeof(msgId), "%u", cbMsgId);
		bundle_add(b, EVT_KEY_MSG_ID, msgId);
		eventsystem_send_system_event(SYS_EVENT_INCOMMING_MSG, b);
		bundle_free(b);
	}

	MSG_END();
}

void MsgTransactionManager::broadcastSyncMLMsgCB(const msg_error_t err, const MSG_SYNCML_MESSAGE_DATA_S *syncMLData)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	int eventSize = MsgMakeEvent(syncMLData, sizeof(MSG_SYNCML_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

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

#if 0
	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	int eventSize = MsgMakeEvent(lbsData, sizeof(MSG_LBS_MESSAGE_DATA_S), MSG_EVENT_PLG_INCOMING_LBS_MSG_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

	lbsmsg_list::iterator it = newLBSMsgCBList.begin();

	for (; it != newLBSMsgCBList.end(); it++)
	{
		MSG_DEBUG("Send incoming LBS msg to listener %d", it->listenerFd);
		write(it->listenerFd, pEventData, eventSize);
	}
#else
	MsgLbsWapPush(lbsData->pushHeader, lbsData->pushBody, lbsData->pushBodyLen);
#endif
	MSG_END();
}


void MsgTransactionManager::broadcastSyncMLMsgOperationCB(const msg_error_t err, const int msgId, const int extId)
{
	MSG_BEGIN();

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	// Encoding Storage Change Data
	int dataSize = MsgEncodeSyncMLOperationData(msgId, extId, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_SYNCML_OPERATION, err, (void**)(&pEventData));

	MutexLocker lock(mx);

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
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	// Encoding Storage Change Data
	dataSize = MsgEncodeStorageChangeData(storageChangeType, pMsgIdList, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_PLG_STORAGE_CHANGE_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

	fd_map::iterator it = storageChangeFdMap.begin();

	for (; it != storageChangeFdMap.end(); it++)
	{
		MSG_DEBUG("Send Storage Change Callback to listener %d", it->first);
		write(it->first, pEventData, eventSize);
	}

	MSG_END();
}


void MsgTransactionManager::broadcastReportMsgCB(const msg_error_t err, const msg_report_type_t reportMsgType, const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	if(pMsgInfo == NULL) {
		MSG_DEBUG("pMsgInfo is NULL.");
		return;
	}

	MSG_DEBUG("reportMsgType [%d]", reportMsgType);

	int dataSize = 0;

	char* pEventData = NULL;
	unique_ptr<char*, void(*)(char**)> eventBuf(&pEventData, unique_ptr_deleter);

	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	// Encoding Storage Change Data
	dataSize = MsgEncodeReportMsgData(reportMsgType, pMsgInfo, &encodedData);

	int eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_PLG_REPORT_MSG_INCOMING_IND, err, (void**)(&pEventData));

	MutexLocker lock(mx);

	fd_map::iterator it = reportMsgCBFdMap.begin();

	for (; it != reportMsgCBFdMap.end(); it++)
	{
		MSG_DEBUG("Send Report Message Incoming Callback to listener %d", it->first);
		write(it->first, pEventData, eventSize);
	}

	MSG_END();
}


bool MsgTransactionManager::initCynara()
{
	int ret;

	ret = cynara_initialize(&p_cynara, NULL);

	if (ret == CYNARA_API_SUCCESS) {
		MSG_INFO("cynara_initialize() is successful");
	} else {
		MSG_INFO("cynara_initialize() is failed [%d]", ret);
		return false;
	}

	ret = cynara_creds_get_default_client_method(&client_method);
	if (ret != CYNARA_API_SUCCESS) {
		MSG_ERR("cynara_creds_get_default_client_method() is failed [%d]", ret);
		return false;
	}

	ret = cynara_creds_get_default_user_method(&user_method);
	if (ret != CYNARA_API_SUCCESS) {
		MSG_ERR("cynara_creds_get_default_user_method() is failed [%d]", ret);
		return false;
	}

	return true;
}


void MsgTransactionManager::finishCynara()
{
	int ret;

	ret = cynara_finish(p_cynara);

	if (ret == CYNARA_API_SUCCESS) {
		MSG_INFO("cynara_finish() is successful");
	} else {
		MSG_INFO("cynara_finish() is failed [%d]",ret);
	}

	p_cynara = NULL;
}
