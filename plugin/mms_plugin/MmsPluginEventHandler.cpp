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

