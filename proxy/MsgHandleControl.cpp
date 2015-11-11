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

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgContact.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"
#include "MsgProxyListener.h"
#include "MsgHandle.h"
#include "MsgSqliteWrapper.h"

/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Control Member Functions
==================================================================================================*/
MsgHandle::MsgHandle() : mCounter(0), mClientSock()
{
	memset(mConnectionId, 0x00, sizeof(mConnectionId));
	memset(mCookie, 0x00, sizeof(mCookie));
}


MsgHandle::~MsgHandle()
{

}


void MsgHandle::openHandle()
{
	bool bReady = false;

	MsgProxyListener::instance()->insertOpenHandleSet(this);

	/* server is currently booting and service is not available until the end of booting */
	MsgSettingGetBool(VCONFKEY_MSG_SERVER_READY, &bReady);

	if (bReady == false) {
		THROW(MsgException::SERVER_READY_ERROR, "Msg Server is not ready !!!!!");
	} else {
		MSG_INFO("Msg Server is ready !!!!!");
	}

	/* Open Socket IPC */
	connectSocket();
}


void MsgHandle::closeHandle(MsgHandle* pHandle)
{
	MSG_BEGIN();

	/* Remove CB List of closing Handle */
	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->clearListOfClosedHandle(pHandle);
	/* eventListener->stop(); */

	/* Close Socket IPC */
	disconnectSocket();

#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	/* Close Contact Service */
	if (MsgCloseContactSvc() != MSG_SUCCESS) {
		MSG_WARN("Fail to close contact service.");
	}
#endif // MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	removeDbHandle(); /* unregister db handler */

	MSG_END();
}


void MsgHandle::connectSocket()
{
	mClientSock.connect(MSG_SOCKET_PATH);
}


void MsgHandle::disconnectSocket()
{
	mClientSock.close();
}


void MsgHandle::write(const char *pCmdData, int cmdSize, char **ppEvent)
{
	if (pCmdData == NULL || ppEvent == NULL) {
		THROW(MsgException::INVALID_PARAM, "Param is NULL");
	}

	int ret = 0;

	/* Send Command to MSG FW */
	MutexLocker lock(mx);
	ret = mClientSock.write(pCmdData, cmdSize);
	if (ret < 0)
		THROW(MsgException::IPC_ERROR, "IPC write error");

	char *tmpEvent = NULL;

	while (1) {
		/* Receive Result from MSG FW */
		read(&tmpEvent);

		if (tmpEvent == NULL) {
			MSG_DEBUG("Event Data is NULL!!");
			break;
		}

		if (!CheckEventData(tmpEvent)) {
			delete [] tmpEvent;
			tmpEvent = NULL;
		} else {
			*ppEvent = tmpEvent;
			break;
		}
	}
}


void MsgHandle::read(char **ppEvent)
{
	unsigned int dataSize = 0;

	int ret = mClientSock.read(ppEvent, &dataSize);

	if (ret == CLOSE_CONNECTION_BY_SIGNAL) {
		THROW(MsgException::IPC_ERROR, "Server closed connection");
	}
}


void MsgHandle::convertMsgStruct(const MSG_MESSAGE_HIDDEN_S *pSrc, MSG_MESSAGE_INFO_S *pDest)
{
	MSG_BEGIN();

	pDest->msgId = pSrc->msgId;
	pDest->threadId = pSrc->threadId;
	pDest->folderId = pSrc->folderId;
	pDest->msgType.mainType = pSrc->mainType;
	pDest->msgType.subType = pSrc->subType;
	pDest->msgType.classType= pSrc->classType;
	pDest->storageId = pSrc->storageId;

	if (g_list_length(pSrc->addressList) > 0) {
		msg_struct_s *addr_info = NULL;
		MSG_ADDRESS_INFO_S *address = NULL;

		pDest->nAddressCnt = g_list_length(pSrc->addressList);
		pDest->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * pDest->nAddressCnt];
		memset(pDest->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S) * pDest->nAddressCnt);

		for (int i = 0; i < pDest->nAddressCnt; i++) {
			addr_info = (msg_struct_s *)g_list_nth_data(pSrc->addressList,(guint)i);
			address = (MSG_ADDRESS_INFO_S *)addr_info->data;

			pDest->addressList[i].addressType = address->addressType;
			pDest->addressList[i].recipientType = address->recipientType;
			pDest->addressList[i].contactId = address->contactId;
			strncpy(pDest->addressList[i].addressVal, address->addressVal, MAX_ADDRESS_VAL_LEN);
			strncpy(pDest->addressList[i].displayName, address->displayName, MAX_DISPLAY_NAME_LEN);
		}
	} else {

		msg_struct_list_s *addr_info_s = pSrc->addr_list;

		if (addr_info_s && (addr_info_s->nCount > 0)) {
			msg_struct_s *addr_info = NULL;
			MSG_ADDRESS_INFO_S *address = NULL;

			pDest->nAddressCnt = addr_info_s->nCount;

			pDest->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * addr_info_s->nCount];
			memset(pDest->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S) * addr_info_s->nCount);

			for (int i = 0; i < addr_info_s->nCount; i++) {
				addr_info = (msg_struct_s *)addr_info_s->msg_struct_info[i];

				address = (MSG_ADDRESS_INFO_S *)addr_info->data;

				pDest->addressList[i].addressType = address->addressType;
				pDest->addressList[i].recipientType = address->recipientType;
				pDest->addressList[i].contactId = address->contactId;
				strncpy(pDest->addressList[i].addressVal, address->addressVal, MAX_ADDRESS_VAL_LEN);
				strncpy(pDest->addressList[i].displayName, address->displayName, MAX_DISPLAY_NAME_LEN);
			}
		}
	}

	strncpy(pDest->replyAddress, pSrc->replyAddress, MAX_PHONE_NUMBER_LEN);
	strncpy(pDest->subject, pSrc->subject, MAX_SUBJECT_LEN);

	pDest->displayTime = pSrc->displayTime;
	pDest->networkStatus = pSrc->networkStatus;
	pDest->encodeType = pSrc->encodeType;
	pDest->bRead = pSrc->bRead;
	pDest->bProtected = pSrc->bProtected;
	pDest->bBackup = pSrc->bBackup;
	pDest->priority = pSrc->priority;
	pDest->direction = pSrc->direction;

	/* Set Port Info. */
	pDest->msgPort.valid = pSrc->bPortValid;

	if (pDest->msgPort.valid == true) {
		pDest->msgPort.dstPort = pSrc->dstPort;
		pDest->msgPort.srcPort = pSrc->srcPort;
	}

	MSG_DEBUG("nSize = %d",  pSrc->dataSize);
	MSG_DEBUG("mainType = %d",  pSrc->mainType);

	if (pSrc->mainType == MSG_SMS_TYPE) {
		if (pSrc->pData != NULL) {
			pDest->bTextSms = true;
			pDest->dataSize = pSrc->dataSize;

			memset(pDest->msgText, 0x00, sizeof(pDest->msgText));

			if (pSrc->dataSize > MAX_MSG_TEXT_LEN) {
				/* Save Message Data into File */
				char fileName[MSG_FILENAME_LEN_MAX+1];
				memset(fileName, 0x00, sizeof(fileName));

				if (MsgCreateFileName(fileName) == false)
					THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

				MSG_SEC_DEBUG("Save pSrc->pData into file : size[%d] name[%s]", pDest->dataSize, fileName);

				if (MsgWriteIpcFile(fileName, (char*)pSrc->pData, pSrc->dataSize) == false)
					THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

				memset(pDest->msgData, 0x00, sizeof(pDest->msgData));
				strncpy(pDest->msgData, fileName, MAX_MSG_DATA_LEN);

				pDest->bTextSms = false;

			} else {
				if (pDest->encodeType == MSG_ENCODE_8BIT)
					memcpy(pDest->msgText, pSrc->pData, pSrc->dataSize);
				else
					strncpy(pDest->msgText, (char*)pSrc->pData, pSrc->dataSize);
			}

			MSG_DEBUG("pData = %s", pSrc->pData);
		} else {
			MSG_DEBUG("pSrc->pData is NULL.");
			pDest->bTextSms = true;
			pDest->dataSize = 0;
		}

		MSG_SEC_DEBUG("msgText = %s", pDest->msgText);
	} else if (pSrc->mainType == MSG_MMS_TYPE) {

		pDest->bTextSms = false;
		pDest->dataSize = pSrc->dataSize;

		if (pSrc->subType == MSG_READREPLY_MMS) {
			memset(pDest->msgData, 0x00, sizeof(pDest->msgData));

			if (pSrc->mmsDataSize < MAX_MSG_DATA_LEN)
				memcpy(pDest->msgData, pSrc->pMmsData, pSrc->mmsDataSize);
			else
				memcpy(pDest->msgData, pSrc->pMmsData, MAX_MSG_DATA_LEN);

		} else {
			/* Save Message Data into File */
			char fileName[MSG_FILENAME_LEN_MAX+1];
			memset(fileName, 0x00, sizeof(fileName));

			if (MsgCreateFileName(fileName) == false)
				THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

			/* change file extension in case of java MMS msg */
			if (pSrc->subType == MSG_SENDREQ_JAVA_MMS) {
				char* pFileNameExt = NULL;
				pFileNameExt = strstr(fileName,"DATA");
				if (pFileNameExt)
					snprintf(pFileNameExt, strlen("JAVA")+1, "JAVA");
			}

			MSG_SEC_DEBUG("Save Message Data into file : size[%d] name[%s]", pSrc->mmsDataSize, fileName);
			if (MsgWriteIpcFile(fileName, (char*)pSrc->pMmsData, pSrc->mmsDataSize) == false)
				THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

			memset(pDest->msgData, 0x00, sizeof(pDest->msgData));
			strncpy(pDest->msgData, fileName, MAX_MSG_DATA_LEN);
			if (pSrc->pData) {
				strncpy(pDest->msgText, (char*)pSrc->pData, MAX_MSG_TEXT_LEN);
			}

			if (strlen(pSrc->thumbPath) > 0) {
				memset(pDest->thumbPath, 0x00, sizeof(pDest->thumbPath));
				memcpy(pDest->thumbPath, pSrc->thumbPath, sizeof(pDest->thumbPath));
			}
		}
	}

	pDest->sim_idx = pSrc->simIndex;

	MSG_END();
}


void MsgHandle::convertMsgStruct(const MSG_MESSAGE_INFO_S *pSrc, MSG_MESSAGE_HIDDEN_S *pDest)
{
	MSG_BEGIN();

	pDest->msgId = pSrc->msgId;
	pDest->threadId = pSrc->threadId;
	pDest->folderId = pSrc->folderId;
	pDest->mainType = pSrc->msgType.mainType;
	pDest->subType = pSrc->msgType.subType;
	pDest->storageId = pSrc->storageId;

	strncpy(pDest->replyAddress, pSrc->replyAddress, MAX_PHONE_NUMBER_LEN);
	strncpy(pDest->subject, pSrc->subject, MAX_SUBJECT_LEN);

	pDest->displayTime = pSrc->displayTime;
	pDest->networkStatus = pSrc->networkStatus;
	pDest->encodeType = pSrc->encodeType;
	pDest->bRead = pSrc->bRead;
	pDest->bProtected = pSrc->bProtected;
	pDest->bBackup = pSrc->bBackup;
	pDest->priority = pSrc->priority;
	pDest->direction = pSrc->direction;
	pDest->simIndex = pSrc->sim_idx;

	/* Set Port Info. */
	pDest->bPortValid = pSrc->msgPort.valid;

	if (pDest->bPortValid == true) {
		pDest->dstPort = pSrc->msgPort.dstPort;
		pDest->srcPort = pSrc->msgPort.srcPort;
	}

	if (pSrc->thumbPath[0] != '\0')
		strncpy(pDest->thumbPath, pSrc->thumbPath, MSG_FILEPATH_LEN_MAX);

	int maxCnt = (pSrc->nAddressCnt > 10)? MAX_TO_ADDRESS_CNT: pSrc->nAddressCnt;

	pDest->addr_list->nCount = maxCnt;

	msg_struct_s *addr_info_s = NULL;
	MSG_ADDRESS_INFO_S *addr_info = NULL;

	for (int i = 0; i < maxCnt; i++) {
		addr_info_s = (msg_struct_s *)pDest->addr_list->msg_struct_info[i];
		addr_info = (MSG_ADDRESS_INFO_S *)addr_info_s->data;

		addr_info->addressType = pSrc->addressList[i].addressType;
		addr_info->recipientType = pSrc->addressList[i].recipientType;
		addr_info->contactId = pSrc->addressList[i].contactId;
		strncpy(addr_info->addressVal, pSrc->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN);
		strncpy(addr_info->displayName, pSrc->addressList[i].displayName, MAX_DISPLAY_NAME_LEN);
		addr_info->displayName[MAX_DISPLAY_NAME_LEN] = '\0';
	}

	for (int i = 0; i < pSrc->nAddressCnt; i++) {
		addr_info_s = new msg_struct_s;
		memset(addr_info_s, 0x00, sizeof(msg_struct_s));

		addr_info_s->type = MSG_STRUCT_ADDRESS_INFO;
		addr_info_s->data = new MSG_ADDRESS_INFO_S;
		memset(addr_info_s->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

		addr_info = (MSG_ADDRESS_INFO_S *)addr_info_s->data;

		addr_info->addressType = pSrc->addressList[i].addressType;
		addr_info->recipientType = pSrc->addressList[i].recipientType;
		addr_info->contactId = pSrc->addressList[i].contactId;
		strncpy(addr_info->addressVal, pSrc->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN);
		strncpy(addr_info->displayName, pSrc->addressList[i].displayName, MAX_DISPLAY_NAME_LEN);
		addr_info->displayName[MAX_DISPLAY_NAME_LEN] = '\0';

		pDest->addressList = g_list_append(pDest->addressList, addr_info_s);
	}


	if (pSrc->bTextSms == false) {
		int fileSize = 0;

		char* pFileData = NULL;
		unique_ptr<char*, void(*)(char**)> buf(&pFileData, unique_ptr_deleter);

		pDest->dataSize = pSrc->dataSize;

		/* Get Message Data from File */
		if (MsgOpenAndReadFile(pSrc->msgData, &pFileData, &fileSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgOpenAndReadFile error");

		if (pSrc->msgType.mainType == MSG_SMS_TYPE) {
			if (pDest->encodeType == MSG_ENCODE_8BIT) {
				pDest->pData = (void*)new char[fileSize];
				memset(pDest->pData, 0x00, fileSize);
				memcpy(pDest->pData, pFileData, fileSize);
			} else {
				pDest->pData = (void*)new char[fileSize+1];
				memset(pDest->pData, 0x00, fileSize+1);
				strncpy((char*)pDest->pData, pFileData, fileSize);
			}
		} else {
			if (pSrc->msgText[0] != '\0') {
				pDest->pData = (void*)new char[strlen(pSrc->msgText)+1];
				memset(pDest->pData, 0x00, strlen(pSrc->msgText)+1);
				strncpy((char*)pDest->pData, pSrc->msgText, strlen(pSrc->msgText));
			}
			pDest->mmsDataSize = fileSize;
			pDest->pMmsData = (void*)new char[fileSize];
			memset(pDest->pMmsData, 0x00, fileSize);
			memcpy(pDest->pMmsData, pFileData, fileSize);
			MSG_SEC_DEBUG("Get Message Data from file : size[%d] name[%s]", pDest->mmsDataSize, pSrc->msgData);
		}

	} else {
		pDest->dataSize = pSrc->dataSize;

		if (pSrc->msgType.mainType == MSG_SMS_TYPE) {
			if (pDest->encodeType == MSG_ENCODE_8BIT || pSrc->msgType.subType == MSG_ETWS_SMS) {
				pDest->pData = (void*)new char[pDest->dataSize];
				memset(pDest->pData, 0x00, pDest->dataSize);
				memcpy(pDest->pData, pSrc->msgText, pDest->dataSize);
			} else {
				pDest->pData = (void*)new char[pDest->dataSize+1];
				memset(pDest->pData, 0x00, pDest->dataSize+1);
				strncpy((char*)pDest->pData, pSrc->msgText, pDest->dataSize);
			}
		} else {
			if (pSrc->msgText[0] != '\0') {
				pDest->pData = (void*)new char[strlen(pSrc->msgText)+1];
				memset(pDest->pData, 0x00, strlen(pSrc->msgText)+1);
				strncpy((char*)pDest->pData, pSrc->msgText, strlen(pSrc->msgText));
			}

			pDest->mmsDataSize = pDest->dataSize;
			pDest->pMmsData = (void*)new char[pDest->dataSize];
			memset(pDest->pMmsData, 0x00, pDest->dataSize);
			memcpy(pDest->pMmsData, pSrc->msgData, pDest->dataSize);
		}
	}

	MSG_END();
}


void MsgHandle::convertSendOptStruct(const MSG_SENDINGOPT_S* pSrc, MSG_SENDINGOPT_INFO_S* pDest, MSG_MESSAGE_TYPE_S msgType)
{
	MSG_BEGIN();

	pDest->bSetting = pSrc->bSetting;

	if (pSrc->bSetting == false) {
		MSG_DEBUG("No Sending Option");
		return;
	}

	pDest->bDeliverReq = pSrc->bDeliverReq;
	pDest->bKeepCopy = pSrc->bKeepCopy;

	MSG_DEBUG("pDest->bSetting = %d", pDest->bSetting);
	MSG_DEBUG("pDest->bDeliverReq = %d", pDest->bDeliverReq);
	MSG_DEBUG("pDest->bKeepCopy = %d", pDest->bKeepCopy);

	if (msgType.mainType == MSG_SMS_TYPE) {
		msg_struct_s *pStruct = (msg_struct_s *)pSrc->smsSendOpt;
		if (pStruct) {
			SMS_SENDINGOPT_S *pSms = (SMS_SENDINGOPT_S *)pStruct->data;
			if (pSms) {
				pDest->option.smsSendOptInfo.bReplyPath = pSms->bReplyPath;
			}
		}
	} else if (msgType.mainType == MSG_MMS_TYPE) {
		msg_struct_s *pStruct = (msg_struct_s *)pSrc->mmsSendOpt;
		if (pStruct) {
			MMS_SENDINGOPT_S *pMms = (MMS_SENDINGOPT_S *)pStruct->data;
			if (pMms) {
				pDest->option.mmsSendOptInfo.priority = pMms->priority;
				pDest->option.mmsSendOptInfo.bReadReq = pMms->bReadReq;

				MSG_DEBUG("pDest->option.mmsSendOpt.priority = %d", pMms->priority);
				MSG_DEBUG("pDest->option.mmsSendOpt.bReadReq = %d", pMms->bReadReq);

				if (pMms->expiryTime == 0) {
					pDest->option.mmsSendOptInfo.expiryTime.type = MMS_TIMETYPE_NONE;
					pDest->option.mmsSendOptInfo.expiryTime.time = pMms->expiryTime;
				} else {
					pDest->option.mmsSendOptInfo.expiryTime.type = MMS_TIMETYPE_RELATIVE;
					pDest->option.mmsSendOptInfo.expiryTime.time = pMms->expiryTime;
				}

				if (pMms->bUseDeliveryCustomTime == true) {
					pDest->option.mmsSendOptInfo.bUseDeliveryCustomTime = true;
				} else {
					pDest->option.mmsSendOptInfo.bUseDeliveryCustomTime = false;
				}
				pDest->option.mmsSendOptInfo.deliveryTime.type = MMS_TIMETYPE_RELATIVE;
				pDest->option.mmsSendOptInfo.deliveryTime.time = pMms->deliveryTime;

				MSG_DEBUG("pDest->option.mmsSendOpt.expiryTime = %d", pDest->option.mmsSendOptInfo.expiryTime.time);
			}
		}
	}

	MSG_END();
}


void MsgHandle::convertSendOptStruct(const MSG_SENDINGOPT_INFO_S* pSrc, MSG_SENDINGOPT_S* pDest, MSG_MESSAGE_TYPE_S msgType)
{
	MSG_BEGIN();

	pDest->bDeliverReq = pSrc->bDeliverReq;
	pDest->bKeepCopy = pSrc->bKeepCopy;

	MSG_DEBUG("pDest->bDeliverReq = %d", pDest->bDeliverReq);
	MSG_DEBUG("pDest->bKeepCopy = %d", pDest->bKeepCopy);

	if (msgType.mainType == MSG_SMS_TYPE) {
		msg_struct_s *pStruct = (msg_struct_s *)pDest->smsSendOpt;
		SMS_SENDINGOPT_S *pSms = (SMS_SENDINGOPT_S *)pStruct->data;
		pSms->bReplyPath = pSrc->option.smsSendOptInfo.bReplyPath;
	} else if (msgType.mainType == MSG_MMS_TYPE) {
		msg_struct_s *pStruct = (msg_struct_s *)pDest->mmsSendOpt;
		MMS_SENDINGOPT_S *pMms = (MMS_SENDINGOPT_S *)pStruct->data;
		pMms->priority = pSrc->option.mmsSendOptInfo.priority;
		pMms->bReadReq = pSrc->option.mmsSendOptInfo.bReadReq;
		pMms->expiryTime = pSrc->option.mmsSendOptInfo.expiryTime.time;
		pMms->deliveryTime = pSrc->option.mmsSendOptInfo.deliveryTime.time;

		MSG_DEBUG("pDest->option.mmsSendOpt.priority = %d", pMms->priority);
		MSG_DEBUG("pDest->option.mmsSendOpt.bReadReq = %d", pMms->bReadReq);
		MSG_DEBUG("pDest->option.mmsSendOpt.expiryTime = %d", pMms->expiryTime);
	}

	MSG_END();
}


int MsgHandle::getSettingCmdSize(MSG_OPTION_TYPE_T optionType)
{
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	switch (optionType) {
	case MSG_GENERAL_OPT:
		cmdSize += sizeof(MSG_GENERAL_OPT_S);
	break;
	case MSG_SMS_SENDOPT:
		cmdSize += sizeof(MSG_SMS_SENDOPT_S);
	break;
	case MSG_SMSC_LIST:
		cmdSize += sizeof(MSG_SMSC_LIST_S);
	break;
	case MSG_MMS_SENDOPT:
		cmdSize += sizeof(MSG_MMS_SENDOPT_S);
	break;
	case MSG_MMS_RECVOPT:
		cmdSize += sizeof(MSG_MMS_RECVOPT_S);
	break;
	case MSG_MMS_STYLEOPT:
		cmdSize += sizeof(MSG_MMS_STYLEOPT_S);
	break;
	case MSG_PUSHMSG_OPT:
		cmdSize += sizeof(MSG_PUSHMSG_OPT_S);
	break;
	case MSG_CBMSG_OPT:
		cmdSize += sizeof(MSG_CBMSG_OPT_S);
	break;
	case MSG_VOICEMAIL_OPT:
		cmdSize += sizeof(MSG_VOICEMAIL_OPT_S);
	break;
	case MSG_MSGSIZE_OPT:
		cmdSize += sizeof(MSG_MSGSIZE_OPT_S);
	break;
	}

	return cmdSize;
}


bool MsgHandle::CheckEventData(char *pEventData)
{
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	switch (pEvent->eventType) {
	case MSG_EVENT_PLG_SENT_STATUS_CNF:
	case MSG_EVENT_PLG_INCOMING_MSG_IND:
	case MSG_EVENT_PLG_INCOMING_MMS_CONF:
	case MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND:
	case MSG_EVENT_PLG_INCOMING_LBS_MSG_IND:
	case MSG_EVENT_PLG_STORAGE_CHANGE_IND:
	case MSG_EVENT_PLG_INCOMING_CB_MSG_IND:
	case MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND:
	case MSG_EVENT_PLG_REPORT_MSG_INCOMING_IND:
		return false;
		break;
	default :
		return true;
		break;
	}

	return true;
}
