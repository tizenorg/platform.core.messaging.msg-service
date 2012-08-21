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

#include <glib.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "MmsPluginTypes.h"
#include "MmsPluginMain.h"
#include "MmsPluginTransport.h"
#include "MsgDebug.h"
#include "MsgException.h"

#include "MmsPluginMain.h"
#include "MmsPluginMessage.h"
#include "MmsPluginStorage.h"
#include "MmsPluginInternal.h"
#include "MmsPluginEventHandler.h"
#include "MsgGconfWrapper.h"
#include "MmsPluginCodec.h"
#include "MsgUtilFile.h"
#include "MmsPluginUserAgent.h"


/*==================================================================================================
							FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgPlgCreateHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	if (pPluginHandle == NULL) {
		MSG_DEBUG("MMS plugin: create handler error ");
		return MSG_ERR_NULL_POINTER;
	} else {
		pPluginHandle->pfInitialize = MmsInitialize;
		pPluginHandle->pfFinalize = MmsFinalize;
		pPluginHandle->pfRegisterListener = MmsRegisterListener;
		pPluginHandle->pfSubmitRequest = MmsSubmitRequest;
		pPluginHandle->pfAddMessage = MmsAddMessage;
		pPluginHandle->pfProcessReceivedInd = MmsProcessReceivedInd;
		pPluginHandle->pfUpdateMessage = MmsUpdateMessage;
		pPluginHandle->pfGetMmsMessage = MmsGetMmsMessage;
		pPluginHandle->pfUpdateRejectStatus = MmsUpdateRejectStatus;
		pPluginHandle->pfComposeReadReport = MmsComposeReadReport;
		pPluginHandle->pfRestoreMsg = MmsRestoreMsg;

		MSG_DEBUG("MMS plugin: create handler OK");
		MSG_DEBUG ("MMS plugin %p", pPluginHandle);
	}

	return MSG_SUCCESS;
}


msg_error_t MmsPlgDestroyHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle)
{
	MSG_BEGIN();

	if (pPluginHandle != NULL) {
		free(pPluginHandle);
		pPluginHandle = NULL;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsInitialize()
{
	MSG_BEGIN();

	// remove temp files
	MsgMmsInitDir();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsFinalize()
{
	MSG_BEGIN();

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsRegisterListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	MSG_BEGIN();

	MmsPluginEventHandler::instance()->registerListener(pListener);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsSubmitRequest(MSG_REQUEST_INFO_S *pReqInfo)
{
	MSG_BEGIN();

	try {
		MmsPluginTransport::instance()->submitRequest(pReqInfo);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsAddMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	MSG_BEGIN();

	try {
		MmsPluginStorage::instance()->addMessage(pMsgInfo, pSendOptInfo, pFileData);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsProcessReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject)
{
	MSG_BEGIN();

	MSG_DEBUG("MMS Plugin ProcessReceivedInd");

	try {
		MmsPluginInternal::instance()->processReceivedInd(pMsgInfo, pRequest, bReject);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_TRANSPORT;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsUpdateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo,  char *pFileData)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	try {
		if (pMsgInfo->networkStatus == MSG_NETWORK_NOT_SEND) {
			err = MmsPluginStorage::instance()->updateMessage(pMsgInfo, pSendOptInfo, pFileData);
		} else {
			//[Update Message ID & File path only in case of retrieve. Else update Message ID]
			if (pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgInfo->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
				err = MmsPluginStorage::instance()->updateConfMessage(pMsgInfo);
			} else {
				err = MmsPluginStorage::instance()->updateMsgServerID(pMsgInfo, pSendOptInfo);
			}
		}
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	MSG_END();

	return err;
}


msg_error_t MmsGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg)
{
	MSG_BEGIN();

	msg_error_t	err = MSG_SUCCESS;

	try {
		err = MmsPluginStorage::instance()->plgGetMmsMessage(pMsg, pSendOptInfo, pMmsMsg, pDestMsg);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	MSG_END();

	return err;
}


msg_error_t MmsUpdateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	try {
		char szTrID[MMS_TR_ID_LEN + 1] = {0x00};
		bool bReportAllowed;

		err = MmsPluginStorage::instance()->getTrID(pMsgInfo,szTrID,sizeof(szTrID));
		if (err != MSG_SUCCESS)
			MSG_DEBUG("MmsPlgUpdRejectStatus : Get MMS Transacation id Failed");

		memset(pMsgInfo->msgData, 0, MAX_MSG_DATA_LEN + 1);
		MsgSettingGetBool(MMS_SEND_REPORT_ALLOWED, &bReportAllowed);
		if (MmsPluginInternal::instance()->encodeNotifyRespInd(szTrID, MSG_DELIVERY_REPORT_REJECTED, bReportAllowed, pMsgInfo->msgData)) {
			MSG_DEBUG("MmsPlgUpdRejectStatus : Encode Notify Response Success");
			pMsgInfo->dataSize = strlen(pMsgInfo->msgData);
			pMsgInfo->bTextSms = true;
		} else
			MSG_DEBUG("MmsPlgSetRejectStatus : Encode Notify Response Failed");
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	if (err != MSG_SUCCESS)
		MSG_DEBUG("MmsPlgSetRejectStatus : Update MMS Message Failed");

	MSG_END();

	return err;
}


msg_error_t MmsComposeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	try {
 		MmsPluginStorage::instance()->composeReadReport(pMsgInfo);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_PLUGIN_STORAGE;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MmsRestoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char *pRcvBody, int rcvdBodyLen, char *filePath)
{
	MSG_BEGIN();

	if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		FILE *pFile;
		_MmsInitHeader();
		_MmsRegisterDecodeBuffer(gszMmsLoadBuf1,  gszMmsLoadBuf2, MSG_MMS_DECODE_BUFFER_MAX);

		if ((pFile = MsgOpenFile(pMsgInfo->msgData, "rb+")) == NULL) {
			MSG_DEBUG("File Open Error: %s", pMsgInfo->msgData);
		} else {
			//Decode Header
			if (!MmsBinaryDecodeMsgHeader(pFile, rcvdBodyLen))
				MSG_DEBUG("Decoding Header Failed \r\n");

			MsgCloseFile(pFile);
		}
	} else {
		MSG_DEBUG(":::%d :%s ",rcvdBodyLen, pRcvBody);

		if (filePath) {
			snprintf(filePath, MAX_FULL_PATH_SIZE, MSG_DATA_PATH"BODY_%lu.DATA", random() % 1000000000 + 1);
		} else {
			return MSG_ERR_NULL_POINTER;
		}

		// create temp file
		if (!MsgOpenCreateAndOverwriteFile(filePath, (char*)pRcvBody,rcvdBodyLen))
			return MSG_ERR_PLUGIN_STORAGE;
	}

	MSG_END();

	return MSG_SUCCESS;
}

