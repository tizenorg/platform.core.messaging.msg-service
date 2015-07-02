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
#include <dlfcn.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgException.h"
#include "MsgIpcSocket.h"
#include "MsgUtilFunction.h"
#include "MsgCmdTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgPluginManager.h"
#include "MsgMmsTypes.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgSentStatusListener(MSG_SENT_STATUS_S *pSentStatus)
{
	MSG_BEGIN();

	MSG_DEBUG("SENT STATUS %d, %d", pSentStatus->reqId, pSentStatus->status);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}



	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_SENT_STATUS_S); // cmd type, MSG_SENT_STATUS

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_SENT_STATUS_CNF;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pSentStatus, sizeof(MSG_SENT_STATUS_S));

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	MSG_DEBUG("Waiting result for SENT STATUS");

	char *temp = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&temp, unique_ptr_deleter);
	unsigned int len;
	client.read(&temp, &len);

	// close connection to msgfw daemon
	client.close();

	MSG_END();
}


void MsgStorageChangeListener(msg_storage_change_type_t storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	MSG_DEBUG("StorageChangeType : [%d], msg ID : [%d]", storageChangeType, pMsgInfo->msgId);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
	client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_MESSAGE_INFO_S) + sizeof(msg_storage_change_type_t);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_STORAGE_CHANGE_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN+sizeof(MSG_MESSAGE_INFO_S)), &storageChangeType, sizeof(msg_storage_change_type_t));

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	MSG_DEBUG("Waiting result for STORAGE CHANGE");

	char *temp = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&temp, unique_ptr_deleter);
	unsigned int len;
	client.read(&temp, &len);

	// close connection to msgfw daemon
	client.close();

	MSG_END();
}


msg_error_t MsgIncomingMessageListener(MSG_MESSAGE_INFO_S *pMsg)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// Check Invalid Message Structure
	if (pMsg == NULL)
	{
		MSG_DEBUG("pMsg is NULL !!");

		return MSG_ERR_NULL_MESSAGE;
	}

	// Allocate Memory to Command Data
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(pMsg, &encodedData);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + dataSize; // cmd type, MSG_MESSAGE_INFO_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_MSG_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), encodedData, dataSize);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;

	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_MSG_IND)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

#ifdef FEATURE_SMS_CDMA
	memcpy(&(pMsg->msgId), pEvent->data, sizeof(msg_message_id_t));
#endif

	MSG_END();

	return (pEvent->result);
}


msg_error_t MsgIncomingSyncMLMessageListener(MSG_SYNCML_MESSAGE_DATA_S *pSyncMLData)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_SYNCML_MESSAGE_DATA_S); // cmd type, MSG_SYNCML_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_SYNCML_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pSyncMLData, sizeof(MSG_SYNCML_MESSAGE_DATA_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_SYNCML_MSG_IND)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

msg_error_t MsgIncomingPushMessageListener(MSG_PUSH_MESSAGE_DATA_S *pPushData)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_PUSH_MESSAGE_DATA_S); // cmd type, MSG_SYNCML_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_PUSH_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pPushData, sizeof(MSG_PUSH_MESSAGE_DATA_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_PUSH_MSG_IND)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


msg_error_t MsgIncomingCBMessageListener(MSG_CB_MSG_S *pCbMsg, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// Check Invalid Message Structure
	if (pCbMsg == NULL)
	{
		MSG_DEBUG("pMsg is NULL !!");

		return MSG_ERR_NULL_MESSAGE;
	}
	int cmdSize = 0;

	// Allocate Memory to Command Data
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(pMsgInfo, &encodedData);

	// composing command
	if(pCbMsg->type == MSG_ETWS_SMS)
		cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CB_MSG_S); // cmd type, MSG_CB_MSG_S
	else
		cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_CB_MSG_S) + dataSize; // cmd type, MSG_CB_MSG_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_CB_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pCbMsg, sizeof(MSG_CB_MSG_S));

	if(pCbMsg->type != MSG_ETWS_SMS)
		memcpy((void*)((char*)pCmd + sizeof(MSG_CMD_TYPE_T)+ MAX_COOKIE_LEN + sizeof(MSG_CB_MSG_S)), encodedData, dataSize);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;

	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_CB_MSG_IND)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


msg_error_t MsgIncomingLBSMessageListener(MSG_LBS_MESSAGE_DATA_S *pLBSData)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_LBS_MESSAGE_DATA_S); // cmd type, MSG_LBS_MESSAGE_DATA_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_LBS_IND;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pLBSData, sizeof(MSG_LBS_MESSAGE_DATA_S));

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INCOMING_LBS_MSG_IND)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


msg_error_t MsgInitSimBySatListener()
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INIT_SIM_BY_SAT;

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_INIT_SIM_BY_SAT)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INVALID_RESULT, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

/* MMS_Incoming_listener */
msg_error_t MsgMmsConfIncomingListener(MSG_MESSAGE_INFO_S *pMsg, msg_request_id_t *pReqId)
{
	MSG_BEGIN();
	MSG_SEC_DEBUG("pMsg = %s, pReqId = %d ", pMsg->msgData, *pReqId);

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// Allocate Memory to Command Data
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(pMsg, &encodedData);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(msg_request_id_t) + dataSize; // cmd type, MSG_MESSAGE_INFO_S, msg_request_id_t
	MSG_DEBUG("cmdSize : %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S *pCmd = (MSG_CMD_S *)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_INCOMING_MMS_CONF; // cmd type

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN); // cmd cookie

	// cmd data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), pReqId, sizeof(msg_request_id_t));
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN + sizeof(msg_request_id_t)), encodedData, dataSize);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char *retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize = 0;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	MSG_EVENT_S *pEvent = (MSG_EVENT_S *)retBuf;

	if(pEvent->eventType != MSG_EVENT_PLG_INCOMING_MMS_CONF)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}


msg_error_t MsgSimMessageListener(MSG_MESSAGE_INFO_S *pMsg, int *simIdList, msg_message_id_t *retMsgId, int size)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// Check Invalid Message Structure
	if (pMsg == NULL)
	{
		MSG_DEBUG("pMsg is NULL !!");

		return MSG_ERR_NULL_MESSAGE;
	}

	// Allocate Memory to Command Data
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);
	int dataSize = MsgEncodeMsgInfo(pMsg, &encodedData);

	char* encodedData2 = NULL;
	unique_ptr<char*, void(*)(char**)> buf2(&encodedData2, unique_ptr_deleter);
	encodedData2 = (char*)new char[dataSize + sizeof(int) + (sizeof(int)*size) + 1];

	char *offset = NULL;
	memcpy(encodedData2, encodedData, dataSize);
	offset = encodedData2+dataSize;

	memcpy(offset, &size, sizeof(int));
	offset += sizeof(int);
	MSG_DEBUG("size [%d]", size);

	memcpy(offset, simIdList, (sizeof(int)*size));

	MSG_DEBUG("simIdList[0] [%d]", simIdList[0]);

	dataSize += ((sizeof(int)*size) + 1);

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + dataSize; // cmd type, MSG_MESSAGE_INFO_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_ADD_SIM_MSG;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), encodedData2, dataSize);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;

	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_ADD_SIM_MSG)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	//CID 48645: pEvent->data is an array hence null check is not required on it.
	if (retMsgId) {
		memcpy(retMsgId, pEvent->data, sizeof(msg_message_id_t));
		MSG_DEBUG("Saved SIM message ID = [%d]", *retMsgId);
	}

	MSG_END();

	return (pEvent->result);
}

msg_error_t MsgResendMessageListener(void)
{
	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_RESEND_MESSAGE;

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_RESEND_MESSAGE)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INVALID_RESULT, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

#ifdef FEATURE_SMS_CDMA
bool MsgCheckUniquenessListener(MSG_UNIQUE_INDEX_S *p_msg, msg_message_id_t msgId, bool bInsert)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// Check Invalid Message Structure
	if (p_msg == NULL)
	{
		MSG_DEBUG("p_msg is NULL !!");

		return MSG_ERR_NULL_MESSAGE;
	}

	// Allocate Memory to Command Data
	char* encodedData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&encodedData, unique_ptr_deleter);

	int dataSize = sizeof(bool) + sizeof(msg_message_id_t) + sizeof(MSG_UNIQUE_INDEX_S);

	encodedData = (char*)new char[dataSize];

	MSG_DEBUG("Encoded Teleservice Msg Id = [%d]", p_msg->tele_msgId);
	MSG_DEBUG("Encoded Address = [%s]", p_msg->address);
	MSG_DEBUG("Encoded Sub Address = [%s]", p_msg->sub_address);

	void* p = (void*)encodedData;

	memcpy(p, &(bInsert), sizeof(bool));
	p = (void*)((char*)p + sizeof(bool));

	memcpy(p, &(msgId), sizeof(msg_message_id_t));
	p = (void*)((char*)p + sizeof(msg_message_id_t));

	memcpy(p, p_msg, sizeof(MSG_UNIQUE_INDEX_S));
	p = (void*)((char*)p + sizeof(MSG_UNIQUE_INDEX_S));

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + dataSize; // cmd type, MSG_MESSAGE_INFO_S

	MSG_DEBUG("cmdSize: %d", cmdSize);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*) cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_PLG_CHECK_UNIQUENESS;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);

	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), encodedData, dataSize);

	// Send Command to Messaging FW
	client.write(cmdBuf, cmdSize);

	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;

	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	// the result is used for making delivery report
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_PLG_CHECK_UNIQUENESS)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INCOMING_MSG_ERROR, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	if (pEvent->result == MSG_SUCCESS)
		return true;
	else
		return false;
}
#endif

msg_error_t MsgSimImsiListener(int sim_idx)
{
	MSG_BEGIN();

	// establish connection to msgfw daemon
	MsgIpcClientSocket client;
	try
	{
		client.connect(MSG_SOCKET_PATH);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_UNKNOWN;
	}

	// composing command
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(int);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);

	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_UPDATE_IMSI;

	memset(pCmd->cmdCookie, 0x00, MAX_COOKIE_LEN);
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), (const void *)&sim_idx, sizeof(int));

	// Send Command to Transaction Manager
	client.write(cmdBuf, cmdSize);

	// Receive result from Transaction Manager
	char* retBuf = NULL;
	unique_ptr<char*, void(*)(char**)> wrap(&retBuf, unique_ptr_deleter);
	unsigned int retSize;
	client.read(&retBuf, &retSize);

	// close connection to msgfw daemon
	client.close();

	// Decoding the result from FW and Returning it to plugin
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)retBuf;

	if (pEvent->eventType != MSG_EVENT_UPDATE_IMSI)
		MSG_FATAL("Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));
		//THROW(MsgException::INVALID_RESULT, "Wrong result(evt type %d : %s) received", pEvent->eventType, MsgDbgEvtStr(pEvent->eventType));

	MSG_END();

	return (pEvent->result);
}

/*==================================================================================================
                                     IMPLEMENTATION OF MsgPlugin - Member Functions
==================================================================================================*/
MsgPlugin::MsgPlugin(MSG_MAIN_TYPE_T mainType, const char *libPath): mSupportedMsg(mainType)
{
	MSG_DEBUG("msg type : [%d] library path : [%s]", mainType, libPath);

	bzero(&mPlgHandler, sizeof(mPlgHandler));

	if (libPath == NULL)
		THROW(MsgException::INVALID_PARAM, "libPath NULL");

	mLibHandler = NULL;

	mLibHandler = dlopen(libPath, RTLD_NOW);

	if (!mLibHandler)
		THROW(MsgException::PLUGIN_ERROR, "ERROR dlopen library : [%s]", libPath);

	// assign the c function pointers
	msg_error_t(*pFunc)(MSG_PLUGIN_HANDLER_S*) = NULL;

	pFunc = (msg_error_t(*)(MSG_PLUGIN_HANDLER_S*))dlsym(mLibHandler, "MsgPlgCreateHandle");

	if (!pFunc)
		THROW(MsgException::PLUGIN_ERROR, "ERROR dlsym library");

	if ((*pFunc)(&mPlgHandler) != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to create plugin handle");

	MSG_PLUGIN_LISTENER_S fwListener = {0};
	fwListener.pfSentStatusCb 					= &MsgSentStatusListener;
	fwListener.pfStorageChangeCb 			= &MsgStorageChangeListener;
	fwListener.pfMsgIncomingCb 				= &MsgIncomingMessageListener;
	fwListener.pfInitSimBySatCb				= &MsgInitSimBySatListener;
	fwListener.pfSyncMLMsgIncomingCb 	= &MsgIncomingSyncMLMessageListener;
	fwListener.pfLBSMsgIncomingCb 		= &MsgIncomingLBSMessageListener;
	fwListener.pfMmsConfIncomingCb 		= &MsgMmsConfIncomingListener;
	fwListener.pfPushMsgIncomingCb 		= &MsgIncomingPushMessageListener;
	fwListener.pfCBMsgIncomingCb 			= &MsgIncomingCBMessageListener;
	fwListener.pfSimMsgIncomingCb 		= &MsgSimMessageListener;
	fwListener.pfResendMessageCb 		= &MsgResendMessageListener;
#ifdef FEATURE_SMS_CDMA
	fwListener.pfCheckUniquenessCb		= &MsgCheckUniquenessListener;
#endif
	fwListener.pfSimInitImsiCb 		= &MsgSimImsiListener;

	if (registerListener(&fwListener) != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to register listener");

	// Initialize Plug-in
	if (initialize() != MSG_SUCCESS)
		THROW(MsgException::PLUGIN_ERROR, "ERROR to initialize plugin");
}


MsgPlugin::~MsgPlugin()
{
	this->finalize();
	// close mLibHandler.
	if (mLibHandler != NULL)
		dlclose(mLibHandler);
}


msg_error_t MsgPlugin::initialize()
{
	if ( mPlgHandler.pfInitialize != NULL)
		return mPlgHandler.pfInitialize();
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


void MsgPlugin::finalize()
{
	if (mPlgHandler.pfFinalize != NULL)
		mPlgHandler.pfFinalize();
}


msg_error_t MsgPlugin::submitReq(MSG_REQUEST_INFO_S *pReqInfo)
{
	if (mPlgHandler.pfSubmitRequest != NULL)
		return mPlgHandler.pfSubmitRequest(pReqInfo);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::registerListener(MSG_PLUGIN_LISTENER_S *pListener)
{
	if (mPlgHandler.pfRegisterListener != NULL)
		return mPlgHandler.pfRegisterListener(pListener);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::saveSimMessage(MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	if (mPlgHandler.pfSaveSimMessage != NULL)
		return mPlgHandler.pfSaveSimMessage(pMsgInfo, pSimIdList);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}

#ifndef FEATURE_SMS_CDMA
msg_error_t MsgPlugin::deleteSimMessage(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	if (mPlgHandler.pfDeleteSimMessage != NULL)
		return mPlgHandler.pfDeleteSimMessage(sim_idx, SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::setReadStatus(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	if (mPlgHandler.pfSetReadStatus != NULL)
		return mPlgHandler.pfSetReadStatus(sim_idx, SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::setMemoryStatus(msg_sim_slot_id_t sim_idx, msg_error_t Error)
{
	if (mPlgHandler.pfSetMemoryStatus != NULL)
		return mPlgHandler.pfSetMemoryStatus(sim_idx, Error);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}
#else
msg_error_t MsgPlugin::deleteSimMessage(msg_sim_id_t SimMsgId)
{
	if (mPlgHandler.pfDeleteSimMessage != NULL)
		return mPlgHandler.pfDeleteSimMessage(SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::setReadStatus(msg_sim_id_t SimMsgId)
{
	if (mPlgHandler.pfSetReadStatus != NULL)
		return mPlgHandler.pfSetReadStatus(SimMsgId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::setMemoryStatus(msg_error_t Error)
{
	if (mPlgHandler.pfSetMemoryStatus != NULL)
		return mPlgHandler.pfSetMemoryStatus(Error);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}
#endif
msg_error_t MsgPlugin::setConfigData(const MSG_SETTING_S *pSetting)
{
	if (mPlgHandler.pfSetConfigData != NULL)
		return mPlgHandler.pfSetConfigData(pSetting);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


msg_error_t MsgPlugin::getConfigData(MSG_SETTING_S *pSetting)
{
	if (mPlgHandler.pfGetConfigData != NULL)
		return mPlgHandler.pfGetConfigData(pSetting);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}

msg_error_t MsgPlugin::addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	if (mPlgHandler.pfAddMessage != NULL) {
		return mPlgHandler.pfAddMessage(pMsgInfo, pSendOptInfo, pFileData);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}

msg_error_t MsgPlugin::updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData)
{
	if (mPlgHandler.pfUpdateMessage != NULL) {
		return mPlgHandler.pfUpdateMessage(pMsgInfo, pSendOptInfo, pFileData);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


msg_error_t MsgPlugin::processReceivedInd(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S *pRequest, bool *bReject)
{
	if (mPlgHandler.pfProcessReceivedInd != NULL) {
		return mPlgHandler.pfProcessReceivedInd(pMsgInfo, pRequest, bReject);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


msg_error_t MsgPlugin::getMmsMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char **pDestMsg)
{
	if (mPlgHandler.pfGetMmsMessage != NULL) {
		return mPlgHandler.pfGetMmsMessage(pMsg, pSendOptInfo, pDestMsg);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


msg_error_t MsgPlugin::updateRejectStatus(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	if (mPlgHandler.pfUpdateRejectStatus != NULL) {
		return mPlgHandler.pfUpdateRejectStatus(pMsgInfo);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


msg_error_t MsgPlugin::composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	if (mPlgHandler.pfComposeReadReport != NULL) {
		return mPlgHandler.pfComposeReadReport(pMsgInfo);
	} else {
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
	}
}


msg_error_t MsgPlugin::restoreMsg(MSG_MESSAGE_INFO_S *pMsgInfo, char* pRecvBody, int rcvdBodyLen, char* filePath)
{
	if (mPlgHandler.pfRestoreMsg != NULL)
		return mPlgHandler.pfRestoreMsg(pMsgInfo,pRecvBody, rcvdBodyLen, filePath);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}

msg_error_t MsgPlugin::getDefaultNetworkSimId(int *SimId)
{
	if (mPlgHandler.pfGetDefaultNetworkSimId != NULL)
		return mPlgHandler.pfGetDefaultNetworkSimId(SimId);
	else
		return MSG_ERR_INVALID_PLUGIN_HANDLE;
}


/*==================================================================================================
                                     IMPLEMENTATION OF MsgPluginManager - Member Functions
==================================================================================================*/
MsgPluginManager* MsgPluginManager::pInstance = NULL;


MsgPluginManager* MsgPluginManager::instance()
{
	if (pInstance == NULL)
		pInstance = new MsgPluginManager();

	return pInstance;
}


MsgPluginManager::MsgPluginManager()
{

}


void MsgPluginManager::initialize()
{
	int plg_len = sizeof(__msg_plg_items)/sizeof(MSG_PLG_TABLE_T);
	for (int i=0; i < plg_len; i++) {
		MsgPlugin* pDupPlgCheck = checkPlugin(__msg_plg_items[i].type);

		if (pDupPlgCheck) {
			MSG_DEBUG("Plugin for type %d is duplicated", __msg_plg_items[i].type);
			continue;
		}

		MsgPlugin *newPlg = NULL;

		try
		{
			newPlg = new MsgPlugin(__msg_plg_items[i].type, __msg_plg_items[i].path);
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());
			continue;
		}

		if (newPlg)
			plgMap.insert(make_pair(__msg_plg_items[i].type, newPlg));

	}
}


void MsgPluginManager::finalize()
{
	MsgPluginMap::iterator it;

	for (it = plgMap.begin(); it != plgMap.end(); it++)
	{
		MsgPlugin *temp = it->second;
		delete temp;
	}

	plgMap.clear();
}


MsgPlugin* MsgPluginManager::checkPlugin(MSG_MAIN_TYPE_T mainType)
{
	/* Implementing the content */
	MsgPluginMap::iterator it = plgMap.find(mainType);

	if (it == plgMap.end())
		return NULL;

	return it->second;
}


MsgPlugin* MsgPluginManager::getPlugin(MSG_MAIN_TYPE_T mainType)
{
	MsgPlugin *plugin = NULL;

	if (plgMap.size() == 0) {
		MSG_INFO("Msg plugin is initializing again");
		initialize();
	}

	plugin = checkPlugin(mainType);

	return plugin;
}

