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
		// remove mms file
		remove(pReqInfo->msgInfo.msgData);
		break;

	case MSG_READRECIND_MMS:
		MSG_DEBUG("######### SEND READREC IND : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_READREC_IND;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
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


void MmsPluginTransport::cancelRequest(MSG_REQUEST_ID_T reqId)
{


}

