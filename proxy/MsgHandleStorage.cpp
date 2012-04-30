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
int MsgHandle::addMessage(const MSG_MESSAGE_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt)
{
	MSG_MESSAGE_INFO_S msgInfo = {0};
	MSG_SENDINGOPT_INFO_S sendOptInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(pMsg, &msgInfo);

	// Covert MSG_SENDINGOPT_S to MSG_SENDINGOPT_INFO_S
	convertSendOptStruct(pSendOpt, &sendOptInfo, pMsg->msgType);

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

	MSG_MESSAGE_ID_T msgId = 0;

	// Decode Return Data
	MsgDecodeMsgId(pEvent->data, &msgId);

	return (int)msgId;
}


MSG_ERROR_T MsgHandle::addSyncMLMessage(const MSG_SYNCML_MESSAGE_S *pSyncMLMsg)
{
	MSG_MESSAGE_INFO_S msgInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct((MSG_MESSAGE_S*)pSyncMLMsg->msg, &msgInfo);

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


MSG_ERROR_T MsgHandle::updateMessage(const MSG_MESSAGE_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt)
{
	MSG_MESSAGE_INFO_S msgInfo;
	MSG_SENDINGOPT_INFO_S sendOptInfo;

	// Covert MSG_MESSAGE_S to MSG_MESSAGE_INFO_S
	convertMsgStruct(pMsg, &msgInfo);

	if(pSendOpt != NULL)
		convertSendOptStruct(pSendOpt, &sendOptInfo, pMsg->msgType);

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


MSG_ERROR_T MsgHandle::updateReadStatus(MSG_MESSAGE_ID_T MsgId, bool bRead)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T) + sizeof(bool);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_READ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), &bRead, sizeof(bool));

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


MSG_ERROR_T MsgHandle::updateProtectedStatus(MSG_MESSAGE_ID_T MsgId, bool bProtected)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T) + sizeof(bool);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_PROTECTED;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), &bProtected, sizeof(bool));

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


MSG_ERROR_T MsgHandle::deleteMessage(MSG_MESSAGE_ID_T MsgId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELETE_MSG;

	// Copy Cookie
	memcpy((void*)pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));

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


MSG_ERROR_T MsgHandle::deleteAllMessagesInFolder(MSG_FOLDER_ID_T FolderId, bool bOnlyDB)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_FOLDER_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELALL_MSGINFOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(MSG_FOLDER_ID_T));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_FOLDER_ID_T)), &bOnlyDB, sizeof(bool));

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


MSG_ERROR_T MsgHandle::moveMessageToFolder(MSG_MESSAGE_ID_T MsgId, MSG_FOLDER_ID_T DestFolderId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T) + sizeof(MSG_FOLDER_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_MOVE_MSGTOFOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+sizeof(MSG_MESSAGE_ID_T)+MAX_COOKIE_LEN), &DestFolderId, sizeof(MSG_FOLDER_ID_T));

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


MSG_ERROR_T MsgHandle::moveMessageToStorage(MSG_MESSAGE_ID_T MsgId, MSG_STORAGE_ID_T DestStorageId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T) + sizeof(MSG_STORAGE_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_MOVE_MSGTOSTORAGE;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), &DestStorageId, sizeof(MSG_STORAGE_ID_T));

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


MSG_ERROR_T MsgHandle::countMessage(MSG_FOLDER_ID_T FolderId, MSG_COUNT_INFO_S *pCountInfo)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_FOLDER_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_COUNT_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(MSG_FOLDER_ID_T));

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


MSG_ERROR_T MsgHandle::countMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount)
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
	memcpy(pMsgCount, (void*)((char*)pEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(MSG_ERROR_T)), sizeof(int));

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgHandle::countMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList)
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
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pAddrInfo, sizeof(MSG_THREAD_LIST_INDEX_S));

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


MSG_ERROR_T MsgHandle::getMessage(MSG_MESSAGE_ID_T MsgId, MSG_MESSAGE_S *pMsg, MSG_SENDINGOPT_S *pSendOpt)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MSG;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &MsgId, sizeof(MSG_MESSAGE_ID_T));

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

	// Covert MSG_MESSAGE_INFO_S to MSG_MESSAGE_S
	convertMsgStruct(&msgInfo, pMsg);

	if(pSendOpt != NULL)
		convertSendOptStruct(&sendOptInfo, pSendOpt, pMsg->msgType);

	// Delete Temp File
	if (msgInfo.bTextSms == false)
	{
		// Delete Temp File
		MsgDeleteFile(msgInfo.msgData); //ipc
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgHandle::getFolderViewList(MSG_FOLDER_ID_T FolderId, const MSG_SORT_RULE_S *pSortRule, MSG_LIST_S *pMsgFolderViewList)
{
	MSG_ERROR_T err = MSG_SUCCESS;

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


MSG_ERROR_T MsgHandle::addFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
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


MSG_ERROR_T MsgHandle::updateFolder(const MSG_FOLDER_INFO_S *pFolderInfo)
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


MSG_ERROR_T MsgHandle::deleteFolder(MSG_FOLDER_ID_T FolderId)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_FOLDER_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_DELETE_FOLDER;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &FolderId, sizeof(MSG_FOLDER_ID_T));

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


MSG_ERROR_T MsgHandle::getFolderList(MSG_FOLDER_LIST_S *pFolderList)
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


MSG_ERROR_T MsgHandle::getThreadViewList(const MSG_SORT_RULE_S *pSortRule, MSG_THREAD_VIEW_LIST_S *pThreadViewList)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


MSG_ERROR_T MsgHandle::getConversationViewList(MSG_THREAD_ID_T ThreadId, MSG_LIST_S *pConvViewList)
{
	MSG_BEGIN();

	MSG_ERROR_T err =  MSG_SUCCESS;

	MsgStoConnectDB();
	err = MsgStoGetConversationViewList(ThreadId, pConvViewList);
	MsgStoDisconnectDB();

	if(err != MSG_SUCCESS)
		return err;


// Update Read Status for the Thead ID
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_THREAD_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_THREAD_READ;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &ThreadId, sizeof(MSG_THREAD_ID_T));

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

	MSG_END();

	return err;
}


MSG_ERROR_T MsgHandle::deleteThreadMessageList(MSG_THREAD_ID_T ThreadId)
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
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &ThreadId, sizeof(MSG_THREAD_ID_T));

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


MSG_ERROR_T MsgHandle::getQuickPanelData(MSG_QUICKPANEL_TYPE_T Type, MSG_MESSAGE_S *pMsg)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_QUICKPANEL_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_QUICKPANEL_DATA;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &Type, sizeof(MSG_QUICKPANEL_TYPE_T));

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

	memcpy(&msgInfo, (void*)((char*)pEvent+sizeof(MSG_EVENT_TYPE_T)+sizeof(MSG_ERROR_T)), sizeof(MSG_MESSAGE_INFO_S));

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


MSG_ERROR_T MsgHandle::resetDatabase()
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


MSG_ERROR_T MsgHandle::getMemSize(unsigned int* memsize)
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


MSG_ERROR_T MsgHandle::searchMessage(const char *pSearchString, MSG_THREAD_VIEW_LIST_S *pThreadViewList)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


MSG_ERROR_T MsgHandle::searchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, MSG_LIST_S *pMsgList)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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



MSG_ERROR_T MsgHandle::getMsgIdList(MSG_REFERENCE_ID_T RefId, MSG_MSGID_LIST_S *pMsgIdList)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	err = MsgStoConnectDB();

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoConnectDB() Error!!");
		return err;
	}

	err = MsgStoGetMsgIdList(RefId, pMsgIdList);

	if (err != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgStoSearchMessage() Error!!");
		return err;
	}

	MsgStoDisconnectDB();

	return err;
}


MSG_ERROR_T MsgHandle::getRejectMsgList(const char *pNumber, MSG_REJECT_MSG_LIST_S *pRejectMsgList)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


MSG_ERROR_T MsgHandle::regStorageChangeCallback(msg_storage_change_cb onStorageChange, void *pUserParam)
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


MSG_ERROR_T MsgHandle::getReportStatus(MSG_MESSAGE_ID_T msg_id, MSG_REPORT_STATUS_INFO_S *pReport_status)
{
	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_ID_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_REPORT_STATUS;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &msg_id, sizeof(MSG_MESSAGE_ID_T));

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
