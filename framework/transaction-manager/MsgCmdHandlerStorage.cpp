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

#include <stdlib.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgCppTypes.h"
#include "MsgUtilFunction.h"
#include "MsgStorageHandler.h"
#include "MsgPluginManager.h"
#include "MsgTransManager.h"
#include "MsgCmdHandler.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgAddMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Message Info
	MSG_MESSAGE_INFO_S* pMsgInfo = (MSG_MESSAGE_INFO_S*)pCmd->cmdData;

	// Get Sending Option
	MSG_SENDINGOPT_INFO_S* pSendOptInfo = (MSG_SENDINGOPT_INFO_S*)(pCmd->cmdData + sizeof(MSG_MESSAGE_INFO_S));

	// Add Message with address
	for (int i = 0; i < pMsgInfo->nAddressCnt; i++)
	{
		err = MsgStoAddMessage(pMsgInfo, pSendOptInfo, i);

		if (err == MSG_SUCCESS)
		{
			MSG_DEBUG("Command Handle Success : MsgStoAddMessage()");

			// Get First Msg Id
			if (i == 0)
			{
				// Encoding Message ID
				dataSize = MsgEncodeMsgId(&(pMsgInfo->msgId), &encodedData);
			}
		}
		else
		{
			MSG_DEBUG("Command Handle Fail : MsgStoAddMessage()");
		}
	}

	// Add Message without address
	if (pMsgInfo->nAddressCnt == 0)
	{
		err = MsgStoAddMessage(pMsgInfo, pSendOptInfo);

		if (err == MSG_SUCCESS)
		{
			MSG_DEBUG("Command Handle Success : MsgStoAddMessage()");

			// Encoding Message ID
			dataSize = MsgEncodeMsgId(&(pMsgInfo->msgId), &encodedData);
		}
		else
		{
			MSG_DEBUG("Command Handle Fail : MsgStoAddMessage()");
		}
	}

	// Delete Temp File for Message Data
	if (pMsgInfo->bTextSms == false)
		MsgDeleteFile(pMsgInfo->msgData); //ipc

	//storage change CB
	MSG_MSGID_LIST_S msgIdList;
	MSG_MESSAGE_ID_T msgIds[1];
	memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	msgIdList.nCount = 1;
	msgIds[0] = pMsgInfo->msgId;
	msgIdList.msgIdList = msgIds;

	MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_INSERT, &msgIdList);

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_ADD_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgAddSyncMLMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	int extId = 0, pinCode = 0;

	MSG_MESSAGE_INFO_S msgInfo = {};

	memcpy(&extId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(int));
	memcpy(&pinCode, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)), sizeof(int));
	memcpy(&msgInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(int)+sizeof(int)), sizeof(MSG_MESSAGE_INFO_S));

	// Add Message
	err = MsgStoAddSyncMLMessage(&msgInfo, extId, pinCode);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoAddSyncMLMessage()");

		// broadcast to listener threads, here
		MsgTransactionManager::instance()->broadcastIncomingMsgCB(err, &msgInfo);
		//storage change CB
		MSG_MSGID_LIST_S msgIdList;
		MSG_MESSAGE_ID_T msgIds[1];
		memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

		msgIdList.nCount = 1;
		msgIds[0] = msgInfo.msgId;
		msgIdList.msgIdList = msgIds;

		MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_INSERT, &msgIdList);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoAddSyncMLMessage()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_ADD_SYNCML_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgUpdateMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Message Info
	MSG_MESSAGE_INFO_S* pMsgInfo = (MSG_MESSAGE_INFO_S*)pCmd->cmdData;

	// Get Sending Option
	MSG_SENDINGOPT_INFO_S* pSendOptInfo = (MSG_SENDINGOPT_INFO_S*)(pCmd->cmdData + sizeof(MSG_MESSAGE_INFO_S));

	// Update Message
	err = MsgStoUpdateMessage(pMsgInfo, pSendOptInfo);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Command Handle Success : MsgStoUpdateMessage()");
	else
		MSG_DEBUG("Command Handle Fail : MsgStoUpdateMessage()");

	// Delete Temp File for Message Data
	if (pMsgInfo->bTextSms == false)
		MsgDeleteFile(pMsgInfo->msgData); //ipc

	//storage change CB
	MSG_MSGID_LIST_S msgIdList;
	MSG_MESSAGE_ID_T msgIds[1];
	memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	msgIdList.nCount = 1;
	msgIds[0] = pMsgInfo->msgId;
	msgIdList.msgIdList = msgIds;

	MsgTransactionManager::instance()->broadcastStorageChangeCB(err, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgUpdateReadStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_MESSAGE_ID_T msgId;
	bool readStatus;

	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_ID_T));
	memcpy(&readStatus, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), sizeof(bool));

	// Update Read Status
	err = MsgStoUpdateReadStatus(msgId, readStatus);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoUpdateReadStatus()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoUpdateReadStatus()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_READ, err, (void**)ppEvent);

	return eventSize;
}


int MsgUpdateThreadReadStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_THREAD_ID_T threadId;
	int unReadCnt = 0;

	memcpy(&threadId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_THREAD_ID_T));

	MsgStoGetThreadUnreadCnt(threadId, &unReadCnt);

	MSG_DEBUG("unReadCnt [%d]", unReadCnt);

	if (unReadCnt > 0) {

		err = MsgStoUpdateThreadReadStatus(threadId);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("Command Handle Success : MsgStoUpdateThreadReadStatus()");
		else
			MSG_DEBUG("Command Handle Fail : MsgStoUpdateThreadReadStatus()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_THREAD_READ, err, (void**)ppEvent);

	return eventSize;

}


int MsgUpdateProtectedStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_MESSAGE_ID_T msgId;
	bool protectedStatus;

	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_ID_T));
	memcpy(&protectedStatus, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), sizeof(bool));

	// Update Protected Status
	err = MsgStoUpdateProtectedStatus(msgId, protectedStatus);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoUpdateProtectedStatus()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoUpdateProtectedStatus()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_PROTECTED, err, (void**)ppEvent);

	return eventSize;
}


int MsgDeleteMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	int extId = 0;

	MSG_MESSAGE_ID_T* msgId = (MSG_MESSAGE_ID_T*)pCmd->cmdData;

	MsgStoGetSyncMLExtId(*msgId, &extId);

	// Delete Message
	err = MsgStoDeleteMessage(*msgId, true);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Command Handle Success : MsgStoDeleteMessage()");

		if(extId > 0) {
			// broadcast to listener threads, here
			MsgTransactionManager::instance()->broadcastSyncMLMsgOperationCB(err, -1, extId);
		}

		MSG_MSGID_LIST_S msgIdList;
		MSG_MESSAGE_ID_T msgIds[1];
		memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

		msgIdList.nCount = 1;
		msgIds[0] = *msgId;
		msgIdList.msgIdList = msgIds;

		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
	} else {
		MSG_DEBUG("Command Handle Fail : MsgStoDeleteMessage()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_DELETE_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgDeleteAllMessageInFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_FOLDER_ID_T* folderId = (MSG_FOLDER_ID_T*)pCmd->cmdData;

	bool bOnlyDB;
	memcpy(&bOnlyDB, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_FOLDER_ID_T)), sizeof(bool));


	MSG_MSGID_LIST_S msgIdList;
	memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	// Delete Message
	err = MsgStoDeleteAllMessageInFolder(*folderId, bOnlyDB, &msgIdList);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Command Handle Success : MsgStoDeleteAllMessageInFolder()");
		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
		if(msgIdList.msgIdList != NULL)
			delete [] (char*)msgIdList.msgIdList;
	} else {
		MSG_DEBUG("Command Handle Fail : MsgStoDeleteAllMessageInFolder()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_DELALL_MSGINFOLDER, err, (void**)ppEvent);

	return eventSize;

}


int MsgMoveMessageToFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_MESSAGE_ID_T msgId;
	MSG_FOLDER_ID_T folderId;

	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_ID_T));
	memcpy(&folderId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), sizeof(MSG_FOLDER_ID_T));

	// Move Message
	err = MsgStoMoveMessageToFolder(msgId, folderId);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Command Handle Success : MsgStoMoveMessageToFolder()");

		MSG_MSGID_LIST_S msgIdList;
		MSG_MESSAGE_ID_T msgIds[1];
		memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

		msgIdList.nCount = 1;
		msgIds[0] = msgId;
		msgIdList.msgIdList = msgIds;

		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_UPDATE, &msgIdList);
	} else {
		MSG_DEBUG("Command Handle Fail : MsgStoMoveMessageToFolder()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_MOVE_MSGTOFOLDER, err, (void**)ppEvent);

	return eventSize;
}


int MsgMoveMessageToStorageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	MSG_MESSAGE_ID_T msgId;
	MSG_STORAGE_ID_T storageId;

	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_ID_T));
	memcpy(&storageId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_ID_T)), sizeof(MSG_STORAGE_ID_T));

	// Move Message
	err = MsgStoMoveMessageToStorage(msgId, storageId);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoMoveMessageToStorage()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoMoveMessageToStorage()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_MOVE_MSGTOSTORAGE, err, (void**)ppEvent);

	return eventSize;
}


int MsgCountMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Folder ID
	MSG_FOLDER_ID_T* folderId = (MSG_FOLDER_ID_T*)pCmd->cmdData;

	// Get Message Count
	MSG_COUNT_INFO_S countInfo;

	err = MsgStoCountMessage(*folderId, &countInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoCountMessage()");

		// Encoding Messaging Count Data
		dataSize = MsgEncodeCountInfo(&countInfo, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoCountMessage()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_COUNT_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgCountMsgByTypeHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Folder ID
	MSG_MESSAGE_TYPE_S* pMsgType = (MSG_MESSAGE_TYPE_S*)pCmd->cmdData;

	int nMsgCnt = 0;

	// Get Message Count
	err = MsgStoCountMsgByType(pMsgType, &nMsgCnt);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoCountMsgByType()");

		// Encoding Messaging Count Data
		dataSize = MsgEncodeCountByMsgType(nMsgCnt, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoCountMsgByType()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_COUNT_BY_MSGTYPE, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetMessageHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	// Get Message ID
	MSG_MESSAGE_ID_T* msgId = (MSG_MESSAGE_ID_T*)pCmd->cmdData;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Message
	MSG_MESSAGE_INFO_S msgInfo;
	MSG_SENDINGOPT_INFO_S sendOptInfo;

	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
	memset(&sendOptInfo, 0x00, sizeof(MSG_SENDINGOPT_INFO_S));

	err = MsgStoGetMessage(*msgId, &msgInfo, &sendOptInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetMessage()");

		// Encoding Message Info  Data
		dataSize = MsgEncodeMsgInfo(&msgInfo, &sendOptInfo, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetMessage()");
	}

//	MsgSoundPlayStop();

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_MSG, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetFolderViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	// Get Folder ID
	MSG_FOLDER_ID_T folderId;
	MSG_SORT_RULE_S sortRule;

	memcpy(&folderId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_FOLDER_ID_T));
	memcpy(&sortRule, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_FOLDER_ID_T)), sizeof(MSG_SORT_RULE_S));

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Message Common Info
	MSG_LIST_S folderViewList;

	err = MsgStoGetFolderViewList(folderId, &sortRule, &folderViewList);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetFolderViewList()");

		// Encoding Folder View List Data
		dataSize = MsgEncodeFolderViewList(&folderViewList, &encodedData);

		MSG_DEBUG("dataSize [%d]", dataSize);

		if (folderViewList.msgInfo != NULL)
		{
			delete [] folderViewList.msgInfo;
			folderViewList.msgInfo = NULL;
		}
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetFolderViewList()");
		return err;
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_FOLDERVIEWLIST, MSG_SUCCESS, (void**)ppEvent);

	return eventSize;
}


int MsgAddFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Folder Info
	MSG_FOLDER_INFO_S* pFolderInfo = (MSG_FOLDER_INFO_S*)pCmd->cmdData;

	// Add Folder
	err = MsgStoAddFolder(pFolderInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoAddFolder()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoAddFolder()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_ADD_FOLDER, err, (void**)ppEvent);

	return eventSize;
}


int MsgUpdateFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Folder Info
	MSG_FOLDER_INFO_S* pFolderInfo = (MSG_FOLDER_INFO_S*)pCmd->cmdData;

	// Update Folder
	err = MsgStoUpdateFolder(pFolderInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoUpdateFolder()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoUpdateFolder()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_FOLDER, err, (void**)ppEvent);

	return eventSize;
}


int MsgDeleteFolderHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Folder Info
	MSG_FOLDER_ID_T* pFolderInfo = (MSG_FOLDER_ID_T*)pCmd->cmdData;

	// Delete Folder
	err = MsgStoDeleteFolder(*pFolderInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoDeleteFolder()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoDeleteFolder()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_DELETE_FOLDER, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetFolderListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Storage List
	MSG_FOLDER_LIST_S folderList;

	err = MsgStoGetFolderList(&folderList);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetFolderList()");

		// Encoding Folder List Data
		dataSize = MsgEncodeFolderList(&folderList, &encodedData);

		delete [] folderList.folderInfo;
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetFolderList()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_FOLDERLIST, err, (void**)ppEvent);

	return eventSize;
}


int MsgInitSimBySatHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Sim Init - Later
	//Run msg-init-app

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_PLG_INIT_SIM_BY_SAT, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetMsgTypeHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);


	int dataSize = 0, eventSize = 0;

	// Get Message ID
	MSG_MESSAGE_ID_T msgId;

	memcpy(&msgId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_MESSAGE_ID_T));

	// Get Msg Type
	MSG_MESSAGE_TYPE_S msgType;

	err = MsgStoGetMsgType(msgId, &msgType);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetMsgType()");

		// Encoding Storage List Data
		dataSize = MsgEncodeMsgType(&msgType, &encodedData);

	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetMsgType()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_MSG_TYPE, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetThreadViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SORT_RULE_S sortRule = {0};

	memcpy(&sortRule, pCmd->cmdData, sizeof(MSG_SORT_RULE_S));

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Thread View List
	MSG_THREAD_VIEW_LIST_S msgThreadViewList;

	err = MsgStoGetThreadViewList(&sortRule, &msgThreadViewList);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetThreadViewList()");

		// Encoding Folder View List Data
		dataSize = MsgEncodeThreadViewList(&msgThreadViewList, &encodedData);

		MSG_DEBUG("dataSize [%d]", dataSize);

		if (msgThreadViewList.msgThreadInfo != NULL)
		{
			delete [] msgThreadViewList.msgThreadInfo;
			msgThreadViewList.msgThreadInfo = NULL;
		}
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetThreadViewList()");
		return err;
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_THREADVIEWLIST, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetConversationViewListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_THREAD_ID_T threadId;

	memcpy(&threadId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_THREAD_ID_T));

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	MSG_LIST_S convViewList;

	err = MsgStoGetConversationViewList(threadId, &convViewList);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetConversationViewList()");

		// Encoding Folder View List Data
		dataSize = MsgEncodeConversationViewList(&convViewList, &encodedData);

		MSG_DEBUG("dataSize [%d]", dataSize);

		if (convViewList.msgInfo != NULL)
		{
			delete [] convViewList.msgInfo;
			convViewList.msgInfo = NULL;
		}
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetConversationViewList()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_CONVERSATIONVIEWLIST, err, (void**)ppEvent);

	return eventSize;
}


int MsgDeleteThreadMessageListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_THREAD_ID_T threadId;
	bool isSyncMLMsg = false;

	memcpy(&threadId, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_THREAD_ID_T));

	int eventSize = 0;

	isSyncMLMsg = MsgStoCheckSyncMLMsgInThread(threadId);

	MSG_MSGID_LIST_S msgIdList;
	memset(&msgIdList, 0x00, sizeof(MSG_MSGID_LIST_S));

	err = MsgStoDeleteThreadMessageList(threadId, &msgIdList);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Command Handle Success : MsgStoDeleteThreadMessageList()");

		if(isSyncMLMsg == true) {
			// broadcast to listener threads, here
			MsgTransactionManager::instance()->broadcastSyncMLMsgOperationCB(err, -1, -1);
		}

		MsgTransactionManager::instance()->broadcastStorageChangeCB(MSG_SUCCESS, MSG_STORAGE_CHANGE_DELETE, &msgIdList);
		if(msgIdList.msgIdList != NULL)
			delete [] (char*)msgIdList.msgIdList;
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoDeleteThreadMessageList()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_DELETE_THREADMESSAGELIST, err, (void**)ppEvent);

	return eventSize;
}


int MsgCountMsgByContactHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	// Get From address
	MSG_THREAD_LIST_INDEX_S addrInfo;

	memcpy(&addrInfo, (void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), sizeof(MSG_THREAD_LIST_INDEX_S));

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Message Common Info
	MSG_THREAD_COUNT_INFO_S threadCountInfo = {0};

	err = MsgStoCountMsgByContact(&addrInfo, &threadCountInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoCountMsgByContact()");

		// Encoding Folder View List Data
		dataSize = MsgEncodeMsgGetContactCount(&threadCountInfo, &encodedData);

		MSG_DEBUG("dataSize [%d]", dataSize);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoCountMsgByContact()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_CONTACT_COUNT, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetQuickPanelDataHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	// Get Message ID
	MSG_QUICKPANEL_TYPE_T* type = (MSG_QUICKPANEL_TYPE_T*)pCmd->cmdData;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Message
	MSG_MESSAGE_INFO_S msgInfo;

	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	err = MsgStoGetQuickPanelData(*type, &msgInfo);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetQuickPanelData()");

		// Encoding Message Info Data
		dataSize = MsgEncodeMsgInfo(&msgInfo, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetQuickPanelData()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_QUICKPANEL_DATA, err, (void**)ppEvent);

	return eventSize;
}


int MsgResetDatabaseHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	int eventSize = 0;

	// Reset DB
	err = MsgStoResetDatabase();

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Command Handle Success : MsgStoResetDatabase()");
	else
		MSG_DEBUG("Command Handle Fail : MsgStoResetDatabase()");

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_RESET_DB, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetMemSizeHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Memory size
	unsigned int memsize = 0;

	memsize = MsgDu(MSG_DATA_ROOT_PATH);

	dataSize = MsgEncodeMemSize(&memsize, &encodedData);

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_MEMSIZE, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetReportStatusHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	// Get Message ID
	MSG_MESSAGE_ID_T* msgId = (MSG_MESSAGE_ID_T*)pCmd->cmdData;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	MSG_REPORT_STATUS_INFO_S reportStatus;

	memset(&reportStatus, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S));

	err = MsgStoGetReportStatus(*msgId, &reportStatus);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgGetReportStatusHandler()");

		// Encoding Report Status Data
		dataSize = MsgEncodeReportStatus(&reportStatus, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgGetReportStatusHandler()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_REPORT_STATUS, err, (void**)ppEvent);

	return eventSize;
}
