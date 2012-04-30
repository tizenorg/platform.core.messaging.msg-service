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
 *	@file 		MsgTestTransport.h
 *	@brief 		Defines transport test function of messaging framework
 *	@version		1.0
 */

#ifndef MSG_TEST_TRANSPORT_H
#define MSG_TEST_TRANSPORT_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Transport Test Function
 *	@section		Program
 *	- Program : Messaging Transport Test Function Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_TRANSPORT_TEST_FUNCTION	Messaging Transport Test Function
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**	@fn		MSG_ERROR_T MsgTestSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t *pMsg)
 *	@brief	Tests MsgSubmitReq.
 *	@remarks
	This function requires two pieces of elements ready.
	One is message structure which is composed on application, and the other is message handle.
	An connected message handle is required for both sending a message, and receiving a message. \n
	To send a SMS, precondition : there should be SMS plugin already plugged into framework.
	The SMS plugin should provide SMS extension structure, subtypes, and classtypes. \n
	To get the result, application MUST implement the callback function, MsgOnStatusChangedCallback(), and register it to message handle. \n
	To receive messages, an application MUST also implement the callback function, MsgOnMessageIncomingCallback(), and register it to message handle.
	In this example, only SMS is interested to receive and process, while other types of messages would be ignored by the application.
	Besides that, an application MAY set a filter list, in this way, only incoming messages which satisfy the filter list will be passed to the registered callback function. \n
	Supposing the subtype are defined as below, and the callback functions, MsgOnStatusChangedCallback() and MsgOnMessageIncomingCallback(), are defined too.
 *	@code
	enum _MSG_SUB_TYPE_E
	{
		MSG_NORMAL_SMS = 0,	// Text SMS message
		MSG_WAPPUSH_SMS,		// WAP Push message
		MSG_CB_SMS,			// Cell Broadcasting message
	};

	enum _MSG_CLASS_TYPE_E
	{
		MSG_CLASS_0 = 0,		// Immediately presented on the recipient device display
		MSG_CLASS_1,			// Stored in the mobile equipment or SIM (depending on memory availability)
		MSG_CLASS_2,			// Stored in SIM
		MSG_CLASS_3,			// Transferred to the terminal equipment (such as PDA or PC) which is connected to the mobile equipment
		MSG_CLASS_NONE,
	};

	void sentStatusCB(MSG_HANDLE_T hMsgHandle, MSG_SENT_STATUS_S* pMsgStatus)
	{
		print("sentStatusCB() called");
		printf("reqId : %d status : %d", pMsgStatus->reqId, pMsgStatus->status);
	}

	void incomingMessageCB (MSG_HANDLE_T hMsgHandle, msg_message_t* pNewMsg)
	{
		cout << "New Message Received" << endl;
		MSG_DEBUG("msgId : %d", pNewMsg->msgId);
		MSG_DEBUG("folderId : %d", pNewMsg->folderId);
		MSG_DEBUG("msgType.mainType = %d", pNewMsg->msgType.mainType);
		MSG_DEBUG("msgType.subType = %d", pNewMsg->msgType.subType);
		MSG_DEBUG("nAddressCnt = %d", pNewMsg->nAddressCnt);
		MSG_DEBUG("addressList[0].addressType = %d", pNewMsg->addressList[0].addressType);
		MSG_DEBUG("addressList[0].addressVal = %s", pNewMsg->addressList[0].addressVal);
		MSG_DEBUG("displayTime = %s", pNewMsg->displayTime);
		MSG_DEBUG("networkStatus = %d", pNewMsg->networkStatus);
		MSG_DEBUG("bRead = %d", pNewMsg->bRead);
		MSG_DEBUG("bProtected = %d", pNewMsg->bProtected);
		MSG_DEBUG("bHasAttach = %d", pNewMsg->bHasAttach);
		MSG_DEBUG("bHasDrm = %d", pNewMsg->bHasDrm);
		MSG_DEBUG("priority = %d", pNewMsg->priority);
		MSG_DEBUG("dataSize = %d", pNewMsg->dataSize);
		MSG_DEBUG("msgData = %s", (char*)pNewMsg->pData);
	}

	MSG_ERROR_T MsgTestSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t *pMsg)
	{
		if (hMsgHandle == NULL)
		{
			MSG_DEBUG("Handle is NULL");
			return MSG_ERR_NULL_MSGHANDLE;
		}

		MSG_ERROR_T err = MSG_SUCCESS;

		MSG_REQUEST_S req;

		// Make Request Message
		if (pMsg == NULL)
		{
			MSG_DEBUG("Message is NULL");
			return MSG_ERR_NULL_MESSAGE;
		}
		else
		{
			req.reqId = 1; // arbitrary number
			req.msg.msgId = pMsg->msgId;
			req.msg.folderId = MSG_OUTBOX_ID; // outbox fixed
			req.msg.msgType = pMsg->msgType;
			req.msg.accountId = pMsg->accountId;
			req.msg.storageId = pMsg->storageId;

			req.msg.nAddressCnt = pMsg->nAddressCnt;

			for (int i = 0; i < pMsg->nAddressCnt; i++)
			{
				req.msg.addressList[i].addressType = pMsg->addressList[i].addressType;
				strncpy(req.msg.addressList[i].addressVal, pMsg->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN);
			}

			strncpy(req.msg.displayTime, pMsg->displayTime, MAX_DISPLAY_TIME_LEN);

			req.msg.networkStatus = MSG_NETWORK_SENDING;
			req.msg.bRead = pMsg->bRead;
			req.msg.bProtected = pMsg->bProtected;
			req.msg.bHasAttach = pMsg->bHasAttach;
			req.msg.bHasDrm = pMsg->bHasDrm;
			req.msg.priority = pMsg->priority;
			req.msg.direction = pMsg->direction;
			req.msg.dataSize = pMsg->dataSize;

			req.msg.pData = (void*)new char[req.msg.dataSize];
			strncpy((char*)req.msg.pData, (char*)pMsg->pData, pMsg->dataSize);
		}

		MSG_DEBUG("==== [MsgTestSubmitReq] Message ID = [%d] ====", req.msg.msgId);
		MSG_DEBUG("==== [MsgTestSubmitReq] Folder ID = [%d] ====", req.msg.folderId);
		MSG_DEBUG("==== [MsgTestSubmitReq] Main Type = [%d] ====", req.msg.msgType.mainType);
		MSG_DEBUG("==== [MsgTestSubmitReq] Sub Type = [%d] ====", req.msg.msgType.subType);
		MSG_DEBUG("==== [MsgTestSubmitReq] Message Data = [%s] ====", (char*)req.msg.pData);

		print("Start Sending Message...");

		// Submit Request
		err = MsgSubmitReq(hMsgHandle, &req);

		if (err == MSG_SUCCESS)
			print("Sending Message is OK!");
		else
			print("Sending Message is failed!");

		return err;
	}
 *	@endcode
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMsg is a pointer to an msg_message_t structure. \n
 *	@return	MSG_ERROR_T
 *	@retval	MSG_SUCCESS				Success in operation. \n
 *	@retval	MSG_ERR_NULL_MSGHANDLE		Message handle is NULL. \n
 *	@retval	MSG_ERR_MEMORY_ERROR		Memory is error. \n
 *	@retval	MSG_ERR_NULL_MESSAGE		Message is NULL. \n
 *	@retval	MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid. \n
 *	@retval	MSG_ERR_NULL_POINTER		pMsg is NULL. \n
 *	@retval	MSG_ERR_PLUGIN				Generic error code for plugin. \n
 */
MSG_ERROR_T MsgTestSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt);

MSG_ERROR_T MsgRetreiveMMSMessage(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg);

MSG_ERROR_T MsgTestScheduledSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt);


/**
 *	@}
 */

#endif //MSG_TEST_TRANSPORT_H

