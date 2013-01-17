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

#ifndef MSG_PLUGIN_MANAGER_H
#define MSG_PLUGIN_MANAGER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <map>

#include "MsgTypes.h"
#include "MsgPluginInterface.h"
#include "MsgPluginConfig.h"


/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MSG_PLUGIN_CFG_PATH 	"/usr/share/msg-service/"
#define MSG_PLUGIN_CFG_NAME 	"plugin.cfg"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgPlugin
{
public:
	MsgPlugin(MSG_MAIN_TYPE_T plgType = MSG_UNKNOWN_TYPE, const char* libPath = NULL);
	~MsgPlugin();

	msg_error_t initialize();
	void finalize();

	msg_error_t submitReq(MSG_REQUEST_INFO_S *pReqInfo);
	msg_error_t registerListener(MSG_PLUGIN_LISTENER_S *pListener);
	msg_error_t checkSimStatus(MSG_SIM_STATUS_T *pStatus);
	msg_error_t checkDeviceStatus();

	msg_error_t initSimMessage();
	msg_error_t saveSimMessage(MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);
	msg_error_t deleteSimMessage(msg_sim_id_t SimMsgId);
	msg_error_t setReadStatus(msg_sim_id_t SimMsgId);
	msg_error_t setMemoryStatus(msg_error_t Error);

	msg_error_t initConfigData(MSG_SIM_STATUS_T SimStatus);
	msg_error_t setConfigData(const MSG_SETTING_S *pSetting);
	msg_error_t getConfigData(MSG_SETTING_S *pSetting);

	// MMS handlers
	msg_error_t addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	msg_error_t updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	msg_error_t processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject);
	msg_error_t getMmsMessage(MSG_MESSAGE_INFO_S *pMsg,  MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg);
	msg_error_t updateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

	msg_error_t restoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char* pRecvBody, int rcvdBodyLen, char* filePath);

	operator void*() const {
		return (mSupportedMsg==MSG_UNKNOWN_TYPE)? NULL:(void*) this;
	}

private:
	MSG_MAIN_TYPE_T 			mSupportedMsg;
	MSG_PLUGIN_HANDLER_S 	mPlgHandler;

	void	*mLibHandler;    // plugin library pointer
};


/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/
typedef std::map<MSG_MAIN_TYPE_T, MsgPlugin*> MsgPluginMap;


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgPluginManager
{
public:
	static MsgPluginManager* instance();

	void initialize();
	void finalize();
	MsgPlugin* getPlugin(MSG_MAIN_TYPE_T mainType);
	void loadPlugins(const char* path);

private:
	MsgPluginManager();
	~MsgPluginManager();

	static MsgPluginManager* pInstance;
	MsgPluginMap plgMap;
};

#endif // MSG_PLUGIN_MANAGER_H

