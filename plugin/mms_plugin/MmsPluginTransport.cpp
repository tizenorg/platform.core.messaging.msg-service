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

#include <stdio.h>
#include "MmsPluginEventHandler.h"
#include "MmsPluginTransport.h"
#include "MsgUtilFile.h"
#include "MmsPluginCodec.h"
#include "MmsPluginMessage.h"
#include "MmsPluginUserAgent.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MmsPluginTransport - Member Functions
==================================================================================================*/
MmsPluginTransport *MmsPluginTransport::pInstance = NULL;


MmsPluginTransport::MmsPluginTransport()
{

}


MmsPluginTransport::~MmsPluginTransport()
{

}


MmsPluginTransport *MmsPluginTransport::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginTransport();

	return pInstance;
}


void MmsPluginTransport::submitRequest(const MSG_REQUEST_INFO_S *pReqInfo)
{
	mmsTranQEntity reqItem = {0};

	reqItem.isCompleted = false;
	reqItem.reqID = pReqInfo->reqId;

	MSG_DEBUG("pReqInfo->msgInfo.msgType.subType [%d]", pReqInfo->msgInfo.msgType.subType);

	switch (pReqInfo->msgInfo.msgType.subType) {
	case MSG_SENDREQ_MMS:
	case MSG_SENDREQ_JAVA_MMS:
		MSG_DEBUG("######### SEND REQUEST : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_SEND_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		break;

	case MSG_GET_MMS:
		MSG_DEBUG("######### AUTO RETRIEVE : GET TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_RETRIEVE_AUTO;
		reqItem.eHttpCmdType = eHTTP_CMD_GET_TRANSACTION;
		reqItem.getDataLen = pReqInfo->msgInfo.dataSize;
		reqItem.pGetData = (char *)malloc(reqItem.getDataLen);
		memcpy(reqItem.pGetData, pReqInfo->msgInfo.msgData, reqItem.getDataLen);
		break;

	case MSG_NOTIFYRESPIND_MMS:
		MSG_DEBUG("######### MANUAL RETRIEVE : SEND NOTIFY RESPONSE IND");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_NOTIFYRESP_IND;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		remove(pReqInfo->msgInfo.msgData);
		break;

	case MSG_RETRIEVE_MMS:
		MSG_DEBUG("######### MANUAL RETRIEVE : GET TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_RETRIEVE_MANUAL;
		reqItem.eHttpCmdType = eHTTP_CMD_GET_TRANSACTION;
		reqItem.getDataLen = pReqInfo->msgInfo.dataSize;
		reqItem.pGetData = (char *)malloc(reqItem.getDataLen);
		memcpy(reqItem.pGetData, pReqInfo->msgInfo.msgData, reqItem.getDataLen);
		break;

	case MSG_READREPLY_MMS:
		MSG_DEBUG("######### SEND READ REPORT : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_READREPORT_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		// remove x-Read-Rec.ind file
		remove(pReqInfo->msgInfo.msgData);
		break;

	case MSG_READRECIND_MMS:
		MSG_DEBUG("######### SEND READREC IND : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_READREC_IND;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		// remove x-Read-Rec.ind file
		remove(pReqInfo->msgInfo.msgData);
		break;

	case MSG_FORWARD_MMS:
		MSG_DEBUG("######### SEND FORWARD MSG : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_SEND_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		break;
	}

	MmsPluginUaManager::instance()->addMmsReqEntity(reqItem);
	MmsPluginUaManager::instance()->start();
}


void MmsPluginTransport::cancelRequest(msg_request_id_t reqId)
{


}

