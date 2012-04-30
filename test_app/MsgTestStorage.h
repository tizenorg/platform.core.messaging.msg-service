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


/**
 *	@file 		MsgTestStorage.h
 *	@brief 		Defines storage test function of messaging framework
 *	@version		1.0
 */

#ifndef MSG_TEST_STORAGE_H
#define MSG_TEST_STORAGE_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Storage Test Function
 *	@section		Program
 *	- Program : Messaging Storage Test Function Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_STORAGE_TEST_FUNCTION	Messaging Storage Test Function
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**	@fn		void MsgTestAddMessage(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgAddMessage.
 *	@param[in]	hMsgHandle is Message handle. \n
 */
void MsgTestAddMessage(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestGetMessage(MSG_HANDLE_T hMsgHandle, int MsgId)
 *	@brief	Tests MsgGetMessage.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	MsgId is the ID of the message to be returned. \n
 */
void MsgTestGetMessage(MSG_HANDLE_T hMsgHandle, int MsgId);


/**	@fn		void MsgTestGetMessageList(MSG_HANDLE_T hMsgHandle, int FolderId)
 *	@brief	Tests MsgGetMsgCommInfoList.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	FolderId is the ID of the folder to be returned. \n
 */
void MsgTestGetMessageList(MSG_HANDLE_T hMsgHandle, int FolderId);


/**	@fn		void MsgTestUpdateMessage(MSG_HANDLE_T hMsgHandle, msg_message_t *pMsg)
 *	@brief	Tests MsgUpdateMessage.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 */
void MsgTestUpdateMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg);

/**	@fn		void MsgTestUpdateMMSMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
 *	@brief	Tests MsgUpdateMessage.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 */
void MsgTestUpdateMMSMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId);

/**	@fn		void MsgTestMoveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T MsgId)
 *	@brief	Tests MsgMoveMessageToFolder.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	MsgId is the ID of the message to be moved. \n
 */
void MsgTestMoveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T MsgId);


/**	@fn		void MsgTestMoveStorageMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T MsgId, MSG_STORAGE_ID_T storageId)
 *	@brief	Tests MsgMoveMessageToStorage.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	MsgId is the ID of the message to be moved. \n
 *	@param[in]	storageId is the destination storage ID. \n
 */
void MsgTestMoveStorageMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg);


/**	@fn		void MsgTestAddFolder(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgAddFolder.
 *	@remarks
	Pre-condition : The storage has to be properly setup and initialized. \n
	The code below is to create folder.
 *	@code
	void MsgTestAddFolder(MSG_HANDLE_T hMsgHandle)
	{
		if (hMsgHandle == NULL)
		{
			MSG_DEBUG("Handle is NULL");
			return;
		}

		MSG_ERROR_T err = MSG_SUCCESS;

		// Make Folder
		MSG_FOLDER_INFO_S folderInfo;

		folderInfo.folderId = g_folderList.nCount + 1;
		folderInfo.folderType = MSG_FOLDER_TYPE_USER_DEF;

		char strName[MAX_FOLDER_NAME_SIZE+1];
		memset(strName, 0x00, sizeof(strName));
		cin.getline(strName, MAX_FOLDER_NAME_SIZE);
		strncpy(folderInfo.folderName, strName, MAX_FOLDER_NAME_SIZE);

		MSG_DEBUG("folderId [%d]", folderInfo.folderId);
		MSG_DEBUG("folderType [%d]", folderInfo.folderType);
		MSG_DEBUG("folderName [%s]", folderInfo.folderName);

		print("Start Creating New Folder...");

		// Create Folder
		err = MsgAddFolder(hMsgHandle, &folderInfo);

		if (err == MSG_SUCCESS)
			print("Creating New Folder is OK!");
		else
			print("Creating New Folder is failed!");
	}
 *	@endcode
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestAddFolder(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestUpdateFolder(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgUpdateFolder.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestUpdateFolder(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestDeleteFolder(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgDeleteFolder.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestDeleteFolder(MSG_HANDLE_T hMsgHandle);

void MsgTestDeleteMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId);

/**	@fn		void MsgPrintMMSBody(msg_message_t pMsg)
 *	@brief	Prints mms body's (content) information.
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 */
void MsgPrintMMSBody(msg_message_t pMsg);

/**	@fn		void MsgPrintMessage(MSG_HANDLE_T hMsgHandle, msg_message_t *pMsg)
 *	@brief	Prints message information.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 */
void MsgPrintMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt);


/**	@fn		void MsgRunMsgMenu(MSG_HANDLE_T hMsgHandle, char Menu, msg_message_t *pMsg)
 *	@brief	Runs the selected function in the message menu.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	Menu indicates which menu is selected. \n
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 */
void MsgRunMsgMenu(MSG_HANDLE_T hMsgHandle, char Menu, msg_message_t pMsg, MSG_SENDINGOPT_S *pSendOpt);


/**	@fn		void MsgRunMsgListMenu(MSG_HANDLE_T hMsgHandle, char *pMenu, int FolderId, MSG_MAIN_TYPE_T mainType, MSG_NETWORK_STATUS_T NetworkStatus)
 *	@brief	Runs the selected function in the message list menu.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMenu is a pointer that indicates which menu is selected. \n
 *	@param[in]	FolderId is the ID of the folder to be run. \n
 */
void MsgRunMsgListMenu(MSG_HANDLE_T hMsgHandle, int MsgId, int FolderId, MSG_MESSAGE_TYPE_T MsgType, MSG_NETWORK_STATUS_T NetworkStatus);


/**	@fn		void MsgGetCurrentTime(time_t *pTime)
 *	@brief	Gets the current time.
 *	@param[out]	pTime is a pointer that indicates the current time.
 */
void MsgGetCurrentTime(time_t *pTime);


/**	@fn		char* MsgConvertMsgType(MSG_MAIN_TYPE_T MainType)
 *	@brief	Converts the message type.
 *	@param[in]	MainType indicates the message type to be returned.
 *	@retval	SMS \n
 *	@retval	MMS \n
 *	@retval	EMAIL \n
 */
const char* MsgConvertMsgType(MSG_MESSAGE_TYPE_T MsgType);

const char* MsgConvertStorageId(MSG_STORAGE_ID_T StorageId);

const char* MsgConvertNetworkStatus(MSG_NETWORK_STATUS_T status);

/**	@fn		char* MsgConvertReadStatus(bool ReadStatus)
 *	@brief	Converts the read status of a message.
 *	@param[in]	ReadStatus indicates whether a message is read or not.
 *	@retval	READ \n
 *	@retval	UNREAD \n
 */
const char* MsgConvertReadStatus(bool ReadStatus);


/**	@fn		char* MsgConvertProtectedStatus(bool ProtectedStatus)
 *	@brief	Converts the protected status of a message.
 *	@param[in]	ProtectedStatus indicates whether a message is protected or not.
 *	@retval	PROTECTED \n
 *	@retval	UNPROTECTED \n
 */
const char* MsgConvertProtectedStatus(bool ProtectedStatus);


/**	@fn		char* MsgConvertTime(const time_t *pTime)
 *	@brief	Converts the time_t value to string.
 *	@param[in]	time_t value.
 *	@retval	String type time present. \n
 */
void MsgConvertTime(time_t *pTime, char *pDisplayTme);

//MSG_ERROR_T convertMsgStruct(const msg_message_t *pSource, MSG_MESSAGE_INFO_S *pDest);

MSG_ERROR_T MsgTestSendReadReport(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, int mmsReadStatus, int version);

void MsgTestForwardMMSMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId);

void MsgTestRetrieveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId);

void MsgTestMsgGen(MSG_HANDLE_T hMsgHandle);

//thread view
void MsgThreadViewMain(MSG_HANDLE_T hMsgHandle);

void MsgRunThreadViewMenu(MSG_HANDLE_T  hMsgHandle, MSG_THREAD_LIST_INDEX_S *pAddrList);

/**
 *	@}
 */


#endif //MSG_TEST_STORAGE_H

