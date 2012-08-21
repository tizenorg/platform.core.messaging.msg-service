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

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgUtilFunction.h"
#include "MsgProxyListener.h"
#include "MsgHandle.h"

#include "MsgStorageHandler.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Storage Member Functions
==================================================================================================*/
int MsgHandle::addMessage(MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt)
{
	MSG_MESSAGE_INFO_S msgInfo = {0};
	MSG_SENDINGOPT_INFO_S sendOptInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(pMsg, &msgInfo);

	// Covert MSG_SENDINGOPT_S to MSG_SENDINGOPT_INFO_S
	MSG_MESSAGE_TYPE_S msgType = {0,};

	msgType.mainType = pMsg->mainType;
	msgType.subType = pMsg->subType;
	msgType.classType = pMsg->classType;

	convertSendOptStruct(pSendOpt, &sendOptInfo, msgType);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_SENDINGOPT_INFO_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_ADD_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msgInfo, sizeof(MSG_MESSAGE_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), &sendOptInfo, sizeof(MSG_SENDINGOPT_INFO_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_ADD_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result != MSG_SUCCESS) return pEvent->result;

	msg_message_id_t msgId = 0;

	// Decode Return Data
	MsgDecodeMsgId(pEvent->data, &msgId);

	return (int)msgId;
}


msg_error_t MsgHandle::addSyncMLMessage(const MSG_SYNCML_MESSAGE_S *pSyncMLMsg)
{
	MSG_MESSAGE_INFO_S msgInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	msg_struct_s *msg = (msg_struct_s *)pSyncMLMsg->msg;
	convertMsgStruct((MSG_MESSAGE_HIDDEN_S *)msg->data, &msgInfo);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int) + sizeof(int) + sizeof(MSG_MESSAGE_INFO_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_ADD_SYNCML_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &pSyncMLMsg->extId, sizeof(int));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)), &pSyncMLMsg->pinCode, sizeof(int));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)+sizeof(int)), &msgInfo, sizeof(MSG_MESSAGE_INFO_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_ADD_SYNCML_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::updateMessage(const MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt)
{
	MSG_MESSAGE_INFO_S msgInfo;
	MSG_SENDINGOPT_INFO_S sendOptInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(pMsg, &msgInfo);

	if(pSendOpt != NULL) {
		MSG_MESSAGE_TYPE_S msgType = {0,};

		msgType.mainType = pMsg->mainType;
		msgType.subType = pMsg->subType;
		msgType.classType = pMsg->classType;

		convertSendOptStruct(pSendOpt, &sendOptInfo, msgType);
	}

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S) + sizeof(MSG_SENDINGOPT_INFO_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msgInfo, sizeof(MSG_MESSAGE_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), &sendOptInfo, sizeof(MSG_SENDINGOPT_INFO_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_UPDATE_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::updateReadStatus(msg_message_id_t MsgId, bool bRead)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t) + sizeof(bool);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_READ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_message_id_t)), &bRead, sizeof(bool));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_UPDATE_READ)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::updateProtectedStatus(msg_message_id_t MsgId, bool bProtected)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t) + sizeof(bool);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_PROTECTED;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_message_id_t)), &bProtected, sizeof(bool));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_UPDATE_PROTECTED)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::deleteMessage(msg_message_id_t MsgId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELETE_MSG;

	// Copy Cookie
	memcpy((void*)pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_DELETE_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::deleteAllMessagesInFolder(msg_folder_id_t FolderId, bool bOnlyDB)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_folder_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELALL_MSGINFOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(msg_folder_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_folder_id_t)), &bOnlyDB, sizeof(bool));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_DELALL_MSGINFOLDER)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::moveMessageToFolder(msg_message_id_t MsgId, msg_folder_id_t DestFolderId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t) + sizeof(msg_folder_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_MOVE_MSGTOFOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+sizeof(msg_message_id_t)+MAX_COOKIE_LEN), &DestFolderId, sizeof(msg_folder_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_MOVE_MSGTOFOLDER)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::moveMessageToStorage(msg_message_id_t MsgId, msg_storage_id_t DestStorageId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t) + sizeof(msg_storage_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_MOVE_MSGTOSTORAGE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(msg_message_id_t)), &DestStorageId, sizeof(msg_storage_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_MOVE_MSGTOSTORAGE)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::countMessage(msg_folder_id_t FolderId, MSG_COUNT_INFO_S *pCountInfo)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_folder_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_COUNT_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(msg_folder_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_COUNT_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeCountInfo(pEvent->data, pCountInfo);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::countMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_TYPE_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_COUNT_BY_MSGTYPE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pMsgType, sizeof(MSG_MESSAGE_TYPE_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_COUNT_BY_MSGTYPE)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	memcpy(pMsgCount, (void*)((char*)pEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(int));

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::countMsgByContact(const MSG_THREAD_LIST_INDEX_INFO_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) +  sizeof(MSG_THREAD_LIST_INDEX_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_CONTACT_COUNT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pAddrInfo, sizeof(msg_contact_id_t));
	msg_struct_s *pAddr = (msg_struct_s *)pAddrInfo->msgAddrInfo;

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN + sizeof(msg_contact_id_t)), pAddr->data, sizeof(MSG_ADDRESS_INFO_S));
	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_CONTACT_COUNT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeContactCount(pEvent->data, pMsgThreadCountList);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getMessage(msg_message_id_t MsgId, MSG_MESSAGE_HIDDEN_S *pMsg, MSG_SENDINGOPT_S *pSendOpt)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(msg_message_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_MSG)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS)
		return pEvent->result;

	// Decode Return Data
	MSG_MESSAGE_INFO_S msgInfo;
	MSG_SENDINGOPT_INFO_S sendOptInfo;
	MsgDecodeMsgInfo(pEvent->data, &msgInfo, &sendOptInfo);

	// Covert MSG_MESSAGE_INFO_S to MSG_MESSAGE_HIDDEN_S
	convertMsgStruct(&msgInfo, pMsg);

	if(pSendOpt != NULL) {
		MSG_MESSAGE_TYPE_S msgType = {0,};

		msgType.mainType = pMsg->mainType;
		msgType.subType = pMsg->subType;
		msgType.classType = pMsg->classType;

		convertSendOptStruct(&sendOptInfo, pSendOpt, msgType);
	}

	// Delete Temp File
	if (msgInfo.bTextSms == false)
	{
		// Delete Temp File
		MsgDeleteFile(msgInfo.msgData); //ipc
	}

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getFolderViewList(msg_folder_id_t FolderId, const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pMsgFolderViewList)
{
	msg_error_t err = MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetFolderViewList(FolderId, (MSG_SORT_RULE_S *)pSortRule, pMsgFolderViewList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoGetFolderViewList() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::addFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_FOLDER_INFO_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_ADD_FOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pFolderInfo, sizeof(MSG_FOLDER_INFO_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_ADD_FOLDER)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::updateFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_FOLDER_INFO_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_FOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pFolderInfo, sizeof(MSG_FOLDER_INFO_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_UPDATE_FOLDER)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;;
}


msg_error_t MsgHandle::deleteFolder(msg_folder_id_t FolderId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_folder_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELETE_FOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(msg_folder_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_DELETE_FOLDER)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::getFolderList(msg_struct_list_s *pFolderList)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_FOLDERLIST;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_FOLDERLIST)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeFolderList(pEvent->data, pFolderList);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetThreadViewList(pSortRule, pThreadViewList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoGetThreadViewList() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::getConversationViewList(msg_thread_id_t ThreadId, msg_struct_list_s *pConvViewList)
{
	MSG_BEGIN();

	msg_error_t err =  MSG_SUCCESS;

	MsgStoConnectDB();
	err = MsgStoGetConversationViewList(ThreadId, pConvViewList);
	MsgStoDisconnectDB();

	if(err != MSG_SUCCESS)
		return err;


// Update Read Status for the Thead ID
#if 1
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_thread_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_THREAD_READ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &ThreadId, sizeof(msg_thread_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_UPDATE_THREAD_READ)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}
#endif

	MSG_END();

	return err;
}


msg_error_t MsgHandle::deleteThreadMessageList(msg_thread_id_t ThreadId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_THREAD_LIST_INDEX_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELETE_THREADMESSAGELIST;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &ThreadId, sizeof(msg_thread_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_DELETE_THREADMESSAGELIST)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getQuickPanelData(msg_quickpanel_type_t Type, MSG_MESSAGE_HIDDEN_S *pMsg)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_quickpanel_type_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_QUICKPANEL_DATA;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &Type, sizeof(msg_quickpanel_type_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_QUICKPANEL_DATA)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MSG_MESSAGE_INFO_S msgInfo;

	memcpy(&msgInfo, (void*)((char*)pEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(msg_error_t)), sizeof(MSG_MESSAGE_INFO_S));

	// Covert MSG_MESSAGE_INFO_S to MSG_MESSAGE_S
	convertMsgStruct(&msgInfo, pMsg);

	// Delete Temp File
	if (msgInfo.bTextSms == false)
	{
		// Delete Temp File
		MsgDeleteFile(msgInfo.msgData); //ipc
	}

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::resetDatabase()
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_RESET_DB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_RESET_DB)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::getMemSize(unsigned int* memsize)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MEMSIZE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_MEMSIZE)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeMemSize(pEvent->data, memsize);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::backupMessage()
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_BACKUP_MESSAGE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_BACKUP_MESSAGE)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::restoreMessage()
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_RESTORE_MESSAGE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_RESTORE_MESSAGE)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::searchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoSearchMessage(pSearchString, pThreadViewList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoSearchMessage() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::searchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, msg_struct_list_s *pMsgList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoSearchMessage(pSearchCon, offset, limit, pMsgList);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoSearchMessage() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::getRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetRejectMsgList(pNumber, pRejectMsgList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoGetRejectMsgList() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::regStorageChangeCallback(msg_storage_change_cb onStorageChange, void *pUserParam)
{
	if (!onStorageChange)
		THROW(MsgException::INVALID_PARAM, "onStorageChange is null");

	MsgProxyListener* eventListener = MsgProxyListener::instance();

	eventListener->start();

	if (eventListener->regStorageChangeEventCB(this, onStorageChange, pUserParam) == false) // callback was already registered, just return SUCCESS
		return MSG_SUCCESS;

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int); // cmd type, listenerFd
	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_REG_STORAGE_CHANGE_CB;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	int listenerFd = eventListener->getRemoteFd(); // fd that is reserved to the "listener thread" by msgfw daemon

	MSG_DEBUG("remote fd %d", listenerFd);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &listenerFd, sizeof(listenerFd));

	MSG_DEBUG("reg status [%d : %s], %d", pCmd->cmdType, MsgDbgCmdStr(pCmd->cmdType), listenerFd);

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_REG_STORAGE_CHANGE_CB)
	{
		THROW(MsgException::INVALID_PARAM, "Event Data Error");
	}

	return pEvent->result;
}


msg_error_t MsgHandle::getReportStatus(msg_message_id_t msg_id, MSG_REPORT_STATUS_INFO_S *pReport_status)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_message_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_REPORT_STATUS;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msg_id, sizeof(msg_message_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_REPORT_STATUS)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeReportStatus(pEvent->data, pReport_status);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetAddressList(threadId, (msg_struct_list_s *)pAddrList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoGetThreadViewList() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


msg_error_t MsgHandle::getThreadIdByAddress(msg_struct_list_s *pAddrList, msg_thread_id_t *pThreadId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(pAddrList->nCount) + (sizeof(MSG_ADDRESS_INFO_S)*pAddrList->nCount);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_THREAD_ID_BY_ADDRESS;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &pAddrList->nCount, sizeof(pAddrList->nCount));
	int addSize = sizeof(MSG_ADDRESS_INFO_S);
	for(int i=0; i<pAddrList->nCount; i++) {
		memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+sizeof(pAddrList->nCount)+(addSize*i)+MAX_COOKIE_LEN), ((msg_struct_s *)(pAddrList->msg_struct_info[i]))->data, sizeof(MSG_ADDRESS_INFO_S));
	}

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);


	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_THREAD_ID_BY_ADDRESS)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if(pEvent->result != MSG_SUCCESS) return pEvent->result;

	// Decode Return Data
	MsgDecodeThreadId(pEvent->data, pThreadId);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getThread(msg_thread_id_t threadId, MSG_THREAD_VIEW_S* pThreadInfo)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_thread_id_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_THREAD_INFO;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &threadId, sizeof(msg_thread_id_t));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_THREAD_INFO)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	// Decode Return Data
	MsgDecodeThreadInfo(pEvent->data, pThreadInfo);

	return MSG_SUCCESS;
}


msg_error_t MsgHandle::getMessageList(msg_folder_id_t folderId, msg_thread_id_t threadId, msg_message_type_t msgType, msg_storage_id_t storageId, msg_struct_list_s *pMsgList)
{
	msg_error_t err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetMessageList(folderId, threadId, msgType, storageId, pMsgList);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgStoSearchMessage() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}
