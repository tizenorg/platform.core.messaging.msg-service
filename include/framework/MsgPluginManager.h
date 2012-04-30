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

	MSG_ERROR_T initialize();
	void finalize();

	MSG_ERROR_T submitReq(MSG_REQUEST_INFO_S *pReqInfo, bool bReqCb);
	MSG_ERROR_T registerListener(MSG_PLUGIN_LISTENER_S *pListener);
	MSG_ERROR_T checkSimStatus(MSG_SIM_STATUS_T *pStatus);
	MSG_ERROR_T checkDeviceStatus();

	MSG_ERROR_T initSimMessage();
	MSG_ERROR_T saveSimMessage(MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);
	MSG_ERROR_T deleteSimMessage(MSG_SIM_ID_T SimMsgId);
	MSG_ERROR_T setReadStatus(MSG_SIM_ID_T SimMsgId);
	MSG_ERROR_T setMemoryStatus(MSG_ERROR_T Error);

	MSG_ERROR_T initConfigData(MSG_SIM_STATUS_T SimStatus);
	MSG_ERROR_T setConfigData(const MSG_SETTING_S *pSetting);
	MSG_ERROR_T getConfigData(MSG_SETTING_S *pSetting);

	// MMS handlers
	MSG_ERROR_T addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	MSG_ERROR_T updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	MSG_ERROR_T processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject);
	MSG_ERROR_T getMmsMessage(MSG_MESSAGE_INFO_S *pMsg,  MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg);
	MSG_ERROR_T updateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

	MSG_ERROR_T restoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char* pRecvBody, int rcvdBodyLen, char* filePath);

	operator void*() const {
		return (mSupportedMsg==MSG_UNKNOWN_TYPE)? NULL:(void*) this;
	}

private:
	MSG_MAIN_TYPE_T 			mSupportedMsg;
	MSG_PLUGIN_HANDLER_S 	mPlgHandler;

        void*	 				mLibHandler;    // plugin library pointer
};


/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/
typedef std::map<MSG_MAIN_TYPE_T, MsgPlugin> MsgPluginMap;


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

