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

#include <stdio.h>
#include "MsgUtilFile.h"
#include "MmsPluginDebug.h"
#include "MmsPluginTypes.h"
#include "MmsPluginTransport.h"
#include "MmsPluginUserAgent.h"
#include "MsgGconfWrapper.h"

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
	reqItem.simId = pReqInfo->msgInfo.sim_idx;

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));

	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_MSISDN, pReqInfo->msgInfo.sim_idx);
	char *msisdn = NULL;
	if (MsgSettingGetString(keyName, &msisdn) != MSG_SUCCESS) {
		MSG_INFO("MsgSettingGetString() is failed");
	}

	MSG_DEBUG("pReqInfo->msgInfo.msgType.subType [%d]", pReqInfo->msgInfo.msgType.subType);

	switch (pReqInfo->msgInfo.msgType.subType) {
	case MSG_SENDREQ_MMS:
	case MSG_SENDREQ_JAVA_MMS:
		MSG_DEBUG("######### SEND REQUEST : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_SEND_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		MSG_MMS_VLD_INFO("%d, MMS Send Start %s->%s, Success", pReqInfo->msgInfo.msgId, (msisdn == NULL)?"ME":msisdn, pReqInfo->msgInfo.addressList[0].addressVal);
		break;

	case MSG_GET_MMS:
		MSG_DEBUG("######### AUTO RETRIEVE : GET TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_RETRIEVE_AUTO;
		reqItem.eHttpCmdType = eHTTP_CMD_GET_TRANSACTION;
		reqItem.getDataLen = pReqInfo->msgInfo.dataSize;
		reqItem.pGetData = (char *)malloc(reqItem.getDataLen);
		if (reqItem.pGetData)
			memcpy(reqItem.pGetData, pReqInfo->msgInfo.msgData, reqItem.getDataLen);
		memcpy(reqItem.url, pReqInfo->msgInfo.msgURL, MMS_LOCATION_LEN);
		MSG_MMS_VLD_INFO("%d, MMS Receive Auto Start %s->%s, Success", pReqInfo->msgInfo.msgId, pReqInfo->msgInfo.addressList[0].addressVal, (msisdn == NULL)?"ME":msisdn);
		break;

	case MSG_NOTIFYRESPIND_MMS: {/* reject */
		MSG_DEBUG("######### MANUAL RETRIEVE : SEND NOTIFY RESPONSE IND");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_NOTIFYRESP_IND;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		memcpy(reqItem.url, pReqInfo->msgInfo.msgURL, MMS_LOCATION_LEN);
		int ret = remove(pReqInfo->msgInfo.msgData);
		if (ret != 0) {
			MSG_DEBUG("remove fail\n");
		}
		break;
	}
	case MSG_RETRIEVE_MMS:
		MSG_DEBUG("######### MANUAL RETRIEVE : GET TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_RETRIEVE_MANUAL;
		reqItem.eHttpCmdType = eHTTP_CMD_GET_TRANSACTION;
		reqItem.getDataLen = pReqInfo->msgInfo.dataSize;
		reqItem.pGetData = (char *)malloc(reqItem.getDataLen);
		if (reqItem.pGetData)
			memcpy(reqItem.pGetData, pReqInfo->msgInfo.msgData, reqItem.getDataLen);
		memcpy(reqItem.url, pReqInfo->msgInfo.msgData, MMS_LOCATION_LEN);
		MSG_MMS_VLD_INFO("%d, MMS Receive Manual Start %s->%s, Success", pReqInfo->msgInfo.msgId, pReqInfo->msgInfo.addressList[0].addressVal, (msisdn == NULL)?"ME":msisdn);
		break;

	case MSG_READREPLY_MMS: {
		MSG_DEBUG("######### SEND READ REPORT : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_READREPORT_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		/* remove x-Read-Rec.ind file */
		int ret = remove(pReqInfo->msgInfo.msgData);
		if (ret != 0) {
			MSG_DEBUG("remove fail\n");
		}
		break;
	}
	case MSG_READRECIND_MMS: {
		MSG_DEBUG("######### SEND READREC IND : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_READREC_IND;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		/* remove x-Read-Rec.ind file */
		int ret = remove(pReqInfo->msgInfo.msgData);
		if (ret != 0) {
			MSG_DEBUG("remove fail\n");
		}
		break;
	}
	case MSG_FORWARD_MMS:
		MSG_DEBUG("######### SEND FORWARD MSG : POST TRANSACTION");
		reqItem.msgId = pReqInfo->msgInfo.msgId;
		reqItem.eMmsPduType = eMMS_SEND_REQ;
		reqItem.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;
		reqItem.pPostData = MsgOpenAndReadMmsFile(pReqInfo->msgInfo.msgData, 0, -1, &reqItem.postDataLen);
		MSG_MMS_VLD_INFO("%d, MMS Send Start %s->%s, Success", pReqInfo->msgInfo.msgId, (msisdn == NULL)?"ME":msisdn, pReqInfo->msgInfo.addressList[0].addressVal);
		break;
	}

	MmsPluginUaManager::instance()->addMmsReqEntity(reqItem);
	MmsPluginUaManager::instance()->start();

	if (msisdn) {
		free(msisdn);
		msisdn = NULL;
	}
}

void MmsPluginTransport::cancelRequest(msg_request_id_t reqId)
{
}

