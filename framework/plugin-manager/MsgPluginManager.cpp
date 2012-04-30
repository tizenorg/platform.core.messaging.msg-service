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

#include <stdio.h>
#include <dlfcn.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgIpcSocket.h"
#include "MsgCmdTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgPluginManager.h"
#include "MsgMmsTypes.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgSentStatusListener(MSG_SENT_STATUS_S *pSentStatus)
{
	MSG_BEGIN();

	MSG_DEBUG("SENT STATUS %d, %d", pSentStatus->reqId, pSentStatus->status);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_SENT_STATUS_S); // cmd type, MSG_SENT_STATUS

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_SENT_STATUS_CNF;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pSentStatus, sizeof(MSG_SENT_STATUS_S));

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	MSG_DEBUG("Waiting result for SENT STATUS");

	char *temp = NULL;
	AutoPtr<char> wrap(&temp);
	int len;
	client.read(&temp, &len);

	// close connection to msgfw daemon
	client.close();

	MSG_END();
}


void MsgStorageChangeListener(MSG_STORAGE_CHANGE_TYPE_T storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MSG_DEBUG("StorageChangeType : [%d], msg ID : [%d]", storageChangeType, pMsgInfo->msgId);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_STORAGE_CHANGE_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_STORAGE_CHANGE_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), &storageChangeType, sizeof(MSG_STORAGE_CHANGE_TYPE_T));

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	MSG_DEBUG("Waiting result for STORAGE CHANGE");

	char *temp = NULL;
	AutoPtr<char> wrap(&temp);
	int len;
	client.read(&temp, &len);

	// close connection to msgfw daemon
	client.close();

	MSG_END();
}


MSG_ERROR_T MsgIncomingMessageListener(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// Check Invalid Message Structure
	if (pMsg == NULL)
	{
		MSG_DEBUG("pMsg is NULL !!");

		return MSG_ERR_NULL_MESSAGE;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S); // cmd type, MSG_MESSAGE_INFO_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_MSG_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pMsg, sizeof(MSG_MESSAGE_INFO_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	char* retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	int retSize;

	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_MSG_IND)
		THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


MSG_ERROR_T MsgIncomingSyncMLMessageListener(MSG_SYNCML_MESSAGE_DATA_S *pSyncMLData)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_SYNCML_MESSAGE_DATA_S); // cmd type, MSG_SYNCML_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_SYNCML_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pSyncMLData, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND)
		THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


MSG_ERROR_T MsgIncomingLBSMessageListener(MSG_LBS_MESSAGE_DATA_S *pLBSData)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_LBS_MESSAGE_DATA_S); // cmd type, MSG_LBS_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_LBS_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pLBSData, sizeof(MSG_LBS_MESSAGE_DATA_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_LBS_MSG_IND)
		THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


MSG_ERROR_T MsgInitSimBySatListener()
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INIT_SIM_BY_SAT;

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INIT_SIM_BY_SAT)
		THROW(MsgException::INVALID_RESULT, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

/* MMS_Incoming_listener */
MSG_ERROR_T MsgMmsConfIncomingListener(MSG_MESSAGE_INFO_S *pMsg, MSG_REQUEST_ID_T *pReqId)
{
	MSG_BEGIN();
	MSG_DEBUG("pMsg = %s, pReqId = %d ", pMsg->msgData, *pReqId);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	client.connect(MSG_SOCKET_PATH);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_REQUEST_ID_T); // cmd type, MSG_MESSAGE_INFO_S, MSG_REQUEST_ID_T
	MSG_DEBUG("cmdSize : %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S *pCmd = (MSG_CMD_S *)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_MMS_CONF; // cmd type

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN); // cmd cookie

	// cmd data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pMsg, sizeof(MSG_MESSAGE_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), pReqId, sizeof(MSG_REQUEST_ID_T));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char *retBuf = NULL;
	AutoPtr<char> wrap(&retBuf);
	int retSize = 0;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	//Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S *pEvent = (MSG_EVENT_S *)retBuf;

	if(pEvent->eventType != MSG_EVENT_PLG_INCOMING_MMS_CONF && pEvent->eventType != MSG_EVENT_PLG_SENT_STATUS_CNF)
		THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

/*==================================================================================================
                                     IMPLEMENTATION OF MsgPlugin - Member Functions
==================================================================================================*/
MsgPlugin::MsgPlugin(MSG_MAIN_TYPE_T mainType, const char *libPath): mSupportedMsg(mainType)
{
	MSG_DEBUG("msg type : [%d] library path : [%s]", mainType, libPath);

	bzero(&mPlgHandler, sizeof(mPlgHandler));

	if (libPath == NULL)
		THROW(MsgException::INVALID_PARAM, "libPath NULL");

        void* libHandle = NULL;

	libHandle = dlopen(libPath, RTLD_NOW);

	if (!libHandle)
		THROW(MsgException::PLUGIN_ERROR, "ERROR dlopen library : [%s] [%s]", libPath, dlerror());

	// Clear Error
	dlerror();

	// assign the c function pointers
	MSG_ERROR_T(*pFunc)(MSG_PLUGIN_HANDLER_S*) = NULL;

	pFunc = (MSG_ERROR_T(*)(MSG_PLUGIN_HANDLER_S*))dlsym(libHandle, "MsgPlgCreateHandle");

	char *error = dlerror();

	if (error != NULL)
		THROW(MsgException::PLUGIN_ERROR, "ERROR dlsym library : [%s]", dlerror());

	if ((*pFunc)(&mPlgHandler) != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to create plugin handle");

	// Initialize Plug-in
	if (initialize() != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to initialize plugin");

	MSG_PLUGIN_LISTENER_S fwListener = {0};
	fwListener.pfSentStatusCb 			= &MsgSentStatusListener;
	fwListener.pfStorageChangeCb 		= &MsgStorageChangeListener;
	fwListener.pfMsgIncomingCb 		= &MsgIncomingMessageListener;
	fwListener.pfInitSimBySatCb			= &MsgInitSimBySatListener;
	fwListener.pfSyncMLMsgIncomingCb 	= &MsgIncomingSyncMLMessageListener;
	fwListener.pfLBSMsgIncomingCb 		= &MsgIncomingLBSMessageListener;
	fwListener.pfMmsConfIncomingCb = &MsgMmsConfIncomingListener;

	if (registerListener(&fwListener) != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to register listener");

//	dlclose(libHandle);
}


MsgPlugin::~MsgPlugin()
{


}


MSG_ERROR_T MsgPlugin::initialize()
{
	if ( mPlgHandler.pfInitialize != NULL)
		return mPlgHandler.pfInitialize();
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


void MsgPlugin::finalize()
{
	if (mPlgHandler.pfFinalize != NULL)
		mPlgHandler.pfFinalize();
}


MSG_ERROR_T MsgPlugin::submitReq(MSG_REQUEST_INFO_S *pReqInfo, bool bReqCb)
{
	if (mPlgHandler.pfSubmitRequest != NULL)
		return mPlgHandler.pfSubmitRequest(pReqInfo, bReqCb);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::registerListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	if (mPlgHandler.pfRegisterListener != NULL)
		return mPlgHandler.pfRegisterListener(pListener);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::checkSimStatus(MSG_SIM_STATUS_T *pStatus)
{
	if (mPlgHandler.pfRegisterListener != NULL)
		return mPlgHandler.pfCheckSimStatus(pStatus);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::checkDeviceStatus()
{
	if (mPlgHandler.pfRegisterListener != NULL)
		return mPlgHandler.pfCheckDeviceStatus();
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::initSimMessage()
{
	if (mPlgHandler.pfInitSimMessage != NULL)
		return mPlgHandler.pfInitSimMessage();
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::saveSimMessage(MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	if (mPlgHandler.pfSaveSimMessage != NULL)
		return mPlgHandler.pfSaveSimMessage(pMsgInfo, pSimIdList);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::deleteSimMessage(MSG_SIM_ID_T SimMsgId)
{
	if (mPlgHandler.pfDeleteSimMessage != NULL)
		return mPlgHandler.pfDeleteSimMessage(SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::setReadStatus(MSG_SIM_ID_T SimMsgId)
{
	if (mPlgHandler.pfSetReadStatus != NULL)
		return mPlgHandler.pfSetReadStatus(SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::setMemoryStatus(MSG_ERROR_T Error)
{
	if (mPlgHandler.pfSetMemoryStatus != NULL)
		return mPlgHandler.pfSetMemoryStatus(Error);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::initConfigData(MSG_SIM_STATUS_T SimStatus)
{
	if (mPlgHandler.pfInitConfigData != NULL)
		return mPlgHandler.pfInitConfigData(SimStatus);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::setConfigData(const MSG_SETTING_S *pSetting)
{
	if (mPlgHandler.pfSetConfigData != NULL)
		return mPlgHandler.pfSetConfigData(pSetting);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


MSG_ERROR_T MsgPlugin::getConfigData(MSG_SETTING_S *pSetting)
{
	if (mPlgHandler.pfGetConfigData != NULL)
		return mPlgHandler.pfGetConfigData(pSetting);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}

MSG_ERROR_T MsgPlugin::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	if (mPlgHandler.pfAddMessage != NULL) {
		return mPlgHandler.pfAddMessage(pMsgInfo, pSendOptInfo, pFileData);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}

MSG_ERROR_T MsgPlugin::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	if (mPlgHandler.pfUpdateMessage != NULL) {
		return mPlgHandler.pfUpdateMessage(pMsgInfo, pSendOptInfo, pFileData);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


MSG_ERROR_T MsgPlugin::processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject)
{
	if (mPlgHandler.pfProcessReceivedInd != NULL) {
		return mPlgHandler.pfProcessReceivedInd(pMsgInfo, pRequest, bReject);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


MSG_ERROR_T MsgPlugin::getMmsMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo,  MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg)
{
	if (mPlgHandler.pfGetMmsMessage != NULL) {
		return mPlgHandler.pfGetMmsMessage(pMsg, pSendOptInfo, pMmsMsg, pDestMsg);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


MSG_ERROR_T MsgPlugin::updateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	if (mPlgHandler.pfUpdateRejectStatus != NULL) {
		return mPlgHandler.pfUpdateRejectStatus(pMsgInfo);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


MSG_ERROR_T MsgPlugin::composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	if (mPlgHandler.pfComposeReadReport != NULL) {
		return mPlgHandler.pfComposeReadReport(pMsgInfo);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


MSG_ERROR_T MsgPlugin::restoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char* pRecvBody, int rcvdBodyLen, char* filePath)
{
	if (mPlgHandler.pfRestoreMsg != NULL)
		return mPlgHandler.pfRestoreMsg(pMsgInfo,pRecvBody, rcvdBodyLen, filePath);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


/*==================================================================================================
                                     IMPLEMENTATION OF MsgPluginManager - Member Functions
==================================================================================================*/
MsgPluginManager* MsgPluginManager::pInstance = NULL;


MsgPluginManager* MsgPluginManager::instance()
{
	if (pInstance == NULL)
		pInstance = new MsgPluginManager();

	return pInstance;
}


MsgPluginManager::MsgPluginManager()
{

}


void MsgPluginManager::initialize()
{
	char path[64];

	memset(path, 0x00, sizeof(path));
	snprintf(path, sizeof(path), "%s%s", MSG_PLUGIN_CFG_PATH, MSG_PLUGIN_CFG_NAME);

	loadPlugins(path);
}


void MsgPluginManager::finalize()
{
	MsgPluginMap::iterator it;

	for (it = plgMap.begin(); it != plgMap.end(); it++)
	{
		MsgPlugin temp = it->second;
		temp.finalize();
	}

	plgMap.clear();
}


void MsgPluginManager::loadPlugins(const char* path)
{
	/* read plugins from configuration file */
	FILE* fp = MsgOpenFile(path, "rt");

	MsgPlgConfig plgConf = MsgPlgConfig(fp);

	for (int i=0; i < plgConf.titleCount(); i++)
	{
		MsgPlgToken tok;

		plgConf.token(i, 0, tok);
		const char* content = tok.getVal();

		MSG_MAIN_TYPE_T mainType = strstr(content,"sms")? MSG_SMS_TYPE:
							(strstr(content,"mms")? MSG_MMS_TYPE: MSG_UNKNOWN_TYPE);

		plgConf.token(i, 1, tok);
		const char* libPath = tok.getVal();

		MsgPlugin* pDupPlgCheck = getPlugin(mainType);

		if (pDupPlgCheck)
			THROW(MsgException::PLUGIN_ERROR, "Plugin for type %d is duplicated", mainType);

		MsgPlugin newPlg(mainType, libPath);

		plgMap.insert(make_pair(mainType, newPlg));
	}

	MsgCloseFile(fp);
}


MsgPlugin* MsgPluginManager::getPlugin(MSG_MAIN_TYPE_T mainType)
{
	/* Implementing the content */
	MsgPluginMap::iterator it = plgMap.find(mainType);

	if (it == plgMap.end())
		return NULL;

	return &(it->second);
}

