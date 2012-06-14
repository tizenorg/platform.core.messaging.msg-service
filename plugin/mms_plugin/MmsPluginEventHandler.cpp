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

#include<stdio.h>
#include<stdlib.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgMmsTypes.h"
#include "MsgSoundPlayer.h"
#include "MsgStorageHandler.h"
#include "MmsPluginTransport.h"
#include "MmsPluginEventHandler.h"
#include "MmsPluginCodec.h"
#include "MmsPluginInternal.h"
#include "MmsPluginSmil.h"
#include "MsgMmsMessage.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginEventHandler - Member Functions
==================================================================================================*/
MmsPluginEventHandler *MmsPluginEventHandler::pInstance = NULL;


MmsPluginEventHandler::MmsPluginEventHandler()
{
	memset(&listener, 0x00, sizeof(MSG_PLUGIN_LISTENER_S));
}


MmsPluginEventHandler::~MmsPluginEventHandler()
{

}


MmsPluginEventHandler *MmsPluginEventHandler::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginEventHandler();

	return pInstance;
}


void MmsPluginEventHandler::registerListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	listener = *pListener;
}


void MmsPluginEventHandler::handleMmsReceivedData(mmsTranQEntity *pRequest, char *pRetrievedFilePath)
{
	MSG_MESSAGE_INFO_S msgInfo = {0,};

	switch (pRequest->eMmsPduType) {
	// received data is send-conf
	case eMMS_SEND_CONF:
		MmsPluginInternal::instance()->processSendConf(&msgInfo, pRequest);

		// callback to MSG FW
		listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

		//MsgDeleteFile(pRetrievedFilePath + strlen(MSG_DATA_PATH)); // not ipc
		remove(pRetrievedFilePath); // not ipc
		break;

	// received data is retrieve-conf
	case eMMS_RETRIEVE_AUTO_CONF:
	case eMMS_RETRIEVE_MANUAL_CONF:
		MmsPluginInternal::instance()->processRetrieveConf(&msgInfo, pRequest, pRetrievedFilePath);

		// callback to MSG FW
		listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);
		break;

	case eMMS_FORWARD_CONF:
		MmsPluginInternal::instance()->processForwardConf(&msgInfo, pRequest);
		break;

	default:
		break;
	}
}


void MmsPluginEventHandler::handleMmsError(mmsTranQEntity *pRequest)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_MESSAGE_INFO_S msgInfo = {};
	MMS_RECV_DATA_S	recvData = {{0}, };

	MSG_DEBUG("pRequest->msgId [%d]", pRequest->msgId);

	switch (pRequest->eMmsPduType) {
	case eMMS_SEND_REQ:
	case eMMS_SEND_CONF:
		msgInfo.msgId = pRequest->msgId;
		//Set only changed members
		msgInfo.msgType.mainType = MSG_MMS_TYPE;

		if (pRequest->eMmsPduType == eMMS_SEND_REQ)
			msgInfo.msgType.subType = MSG_SENDREQ_MMS;
		else
			msgInfo.msgType.subType = MSG_SENDCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_SEND_FAIL;

		msgInfo.folderId = MSG_OUTBOX_ID;

		listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);
		break;

	case eMMS_RETRIEVE_AUTO:
	case eMMS_RETRIEVE_AUTO_CONF:
		msgInfo.msgId = pRequest->msgId;
		//Set only changed members
		msgInfo.msgType.mainType = MSG_MMS_TYPE;
		msgInfo.msgType.subType = MSG_RETRIEVE_AUTOCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		msgInfo.folderId = MSG_INBOX_ID;

		err = listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

		break;

	case eMMS_RETRIEVE_MANUAL:
	case eMMS_RETRIEVE_MANUAL_CONF:
		msgInfo.msgId = pRequest->msgId;
		//Set only changed members
		msgInfo.msgType.mainType = MSG_MMS_TYPE;
		msgInfo.msgType.subType = MSG_RETRIEVE_MANUALCONF_MMS;

		msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
		msgInfo.folderId = MSG_INBOX_ID;

		err = listener.pfMmsConfIncomingCb(&msgInfo, &pRequest->reqID);

		break;

	default:
		break;
	}

	MSG_END();
}

