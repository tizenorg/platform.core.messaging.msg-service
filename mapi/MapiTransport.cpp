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

#include <errno.h>

#include "MsgHandle.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MapiTransport.h"
#include "MapiControl.h"
#include "MapiStorage.h"
#include "MapiMessage.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_submit_req(MSG_HANDLE_T handle, MSG_REQUEST_S *req)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL ||req == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->submitReq(req);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_TRANSPORT_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_sent_status_callback(MSG_HANDLE_T handle, msg_sent_status_cb cb, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regSentStatusCallback(cb, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_sms_message_callback(MSG_HANDLE_T handle, msg_sms_incoming_cb cb, unsigned short port, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regSmsMessageCallback(cb, port, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_mms_conf_message_callback(MSG_HANDLE_T handle, msg_mms_conf_msg_incoming_cb cb, const char *app_id, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	if (app_id && strlen(app_id) > MAX_MMS_JAVA_APPID_LEN)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regMmsConfMessageCallback(cb, app_id, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_syncml_message_callback(MSG_HANDLE_T handle,  msg_syncml_msg_incoming_cb cb, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regSyncMLMessageCallback(cb, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_lbs_message_callback(MSG_HANDLE_T handle, msg_lbs_msg_incoming_cb cb, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regLBSMessageCallback(cb, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_syncml_message_operation_callback(MSG_HANDLE_T handle,  msg_syncml_msg_operation_cb cb, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regSyncMLMessageOperationCallback(cb, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}


EXPORT_API int msg_syncml_message_operation(MSG_HANDLE_T handle,  MSG_MESSAGE_ID_T msgId)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || msgId < 1)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->operateSyncMLMessage(msgId);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_TRANSPORT_ERROR;
	}

	return err;
}


static MSG_HANDLE_T msgHandle = NULL;
static msg_simple_sent_status_cb sentStatusCallback = NULL;

static void sent_status_cb_func(MSG_HANDLE_T handle, MSG_SENT_STATUS_S *sent_status, void *user_param)
{
	MSG_DEBUG("Sent Status [%d]", sent_status->status);

	msg_simple_sent_status_cb pfunc = sentStatusCallback;

	pfunc(sent_status, user_param);

	MSG_DEBUG("After entering callback function.");

	// Close control handle instance
	//	msg_close_msg_handle(&msgHandle);

	//	MSG_DEBUG("After msg_close_msg_handle.");

}


static int msg_send_single_sms(const char *phone_num, const char *sms_text, msg_simple_sent_status_cb cb, void *user_param)
{
	if (phone_num == NULL || sms_text == NULL || cb == NULL)
	{
		MSG_DEBUG("Invalid parameter [%s] [%s] [%s]", phone_num, sms_text, cb);
		return -EINVAL;
	}

	if (strlen(phone_num) > MAX_PHONE_NUMBER_LEN)
	{
		MSG_DEBUG("Phone Number is too long [%s]", phone_num);
		return -EINVAL;
	}

	MSG_REQUEST_S msgReq = {0};
	MSG_ERROR_T retVal = MSG_SUCCESS;

	// Open control handle instance
	if ((retVal = msg_open_msg_handle(&msgHandle)) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgOpenMsgHandle is failed. Error Code = %d", retVal);
		return retVal;
	}

	msgReq.msg = msg_new_message();
	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) msgReq.msg;
	/* when sending SMS */
	reqmsg->msgType.mainType = MSG_SMS_TYPE;
	reqmsg->msgType.subType = MSG_NORMAL_SMS;
	reqmsg->msgId = 0;
	reqmsg->folderId = MSG_OUTBOX_ID;

	/* fill the destination number in msgReq */
	reqmsg->nAddressCnt = 1;
	reqmsg->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN; // telephone number
	reqmsg->addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO; // telephone number
	memset(reqmsg->addressList, 0x00, MAX_ADDRESS_VAL_LEN);
	strncpy(reqmsg->addressList[0].addressVal, phone_num, MAX_ADDRESS_VAL_LEN);
	MSG_DEBUG("AddressVal = [%s]", reqmsg->addressList[0].addressVal);

	reqmsg->msgPort.valid = false;

	/* fill the msg text in msgReq */
	reqmsg->dataSize = strlen(sms_text);
	reqmsg->pData = (void*)malloc(reqmsg->dataSize+1);
	strncpy((char *)reqmsg->pData, sms_text, reqmsg->dataSize);

	sentStatusCallback = cb;

	// register sent status callback
	retVal = msg_reg_sent_status_callback(msgHandle, sent_status_cb_func, user_param);

	if (retVal != MSG_SUCCESS)
	{
		MSG_DEBUG("msg_reg_sent_status_callback() is failed. Error Code = %d", retVal);
		msg_close_msg_handle(&msgHandle);
		return retVal;
	}

	// sending message request
	retVal = msg_submit_req(msgHandle, &msgReq);

	if (retVal != MSG_SUCCESS)
	{
		MSG_DEBUG("msg_submit_req() is failed. Error Code = %d", retVal);
		msg_close_msg_handle(&msgHandle);
		return retVal;
	}

	/* Releasing msg_message_t is mandotory. Unless, memory leaks */
	msg_release_message(&(msgReq.msg));

	return MSG_SUCCESS;
}


static int msg_verify_number(const char *raw, char *trimmed)
{
	if( !(raw && trimmed) )
	{
		MSG_DEBUG("Phone Number is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_DEBUG("Phone Number is NULL");
	for( int i=0, j=0 ; raw[i] ; i++ )
	{
		if ( (raw[i] >= '0' && raw[i] <= '9') || (raw[i] == ',') || raw[i] == ' ' || raw[i] == '+' )
			trimmed[j++] = raw[i];
		else if( raw[i] == '-')
			continue;
		else
		{
			MSG_DEBUG("Unacceptable character in telephone number: [%c]", raw[i]);
			return MSG_ERR_INVALID_PARAMETER;
		}
	}
	MSG_DEBUG("Trimming [%s]->[%s]", raw, trimmed);
	return MSG_SUCCESS;
}


EXPORT_API int msg_sms_send(const char *phone_num_list, const char *sms_text, msg_simple_sent_status_cb cb, void *user_param)
{
	if (phone_num_list == NULL || sms_text == NULL || cb == NULL)
	{
		MSG_DEBUG("Invalid parameter [%s] [%s] [%s]", phone_num_list, sms_text, cb);
		return -EINVAL;
	}

	char trimmed_num[strlen(phone_num_list)+1];
	bzero(trimmed_num, strlen(phone_num_list)+1);

	MSG_ERROR_T retVal = msg_verify_number(phone_num_list, trimmed_num);

	if ( retVal != MSG_SUCCESS )
		return retVal;

	for( char* cur_num = strtok(trimmed_num,", "); cur_num ; cur_num = strtok(NULL,", "))
	{
		if (strlen(cur_num) > MAX_PHONE_NUMBER_LEN)
		{
			MSG_DEBUG("Phone number is too long [%s], and sending is skipped", cur_num);
			continue;
		}

		MSG_DEBUG("phone number: [%s]", cur_num);
		MSG_DEBUG("text: [%s]", sms_text);
		retVal = msg_send_single_sms(cur_num, sms_text, cb, user_param);

		if (retVal != MSG_SUCCESS)
			return retVal;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_sms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || !req) {
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	req->reqId = 1; // arbitrary number

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req->msg;

	if (reqmsg->dataSize <= 0) {
		MSG_FATAL("msg size is invalid : [%d]", reqmsg->dataSize);
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (reqmsg->msgType.mainType != MSG_SMS_TYPE) {
		MSG_DEBUG("mainType is not SMS [%d]", reqmsg->msgType.mainType);
		reqmsg->msgType.mainType = MSG_SMS_TYPE;
	}

	if (reqmsg->msgType.subType > MSG_CONCAT_SIM_SMS) {
		MSG_DEBUG("subType is not SMS [%d]", reqmsg->msgType.subType);
		reqmsg->msgType.subType = MSG_NORMAL_SMS;
	}

	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed! [%d]", err);

	return err;
}


EXPORT_API int msg_mms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || !req)
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	req->reqId = 1; // arbitrary number

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req->msg;

	if (reqmsg->dataSize <= 0)
	{
		MSG_FATAL("MMS data size is invalid");
		return MSG_ERR_INVALID_PARAMETER;
	}

	reqmsg->msgType.mainType = MSG_MMS_TYPE;
	reqmsg->msgType.subType = MSG_SENDREQ_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed! [%d]", err);

	return err;
}


EXPORT_API int msg_mms_send_read_report(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msgId, MSG_READ_REPORT_STATUS_T mms_read_status)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_REQUEST_S req = {0};

	if (handle == NULL)
	{
		return MSG_ERR_INVALID_PARAMETER;
	}

	req.reqId = 1; // arbitrary number

	req.msg = msg_new_message();

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req.msg;

	reqmsg->msgPort.valid = false;
	reqmsg->msgId = msgId;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->msgType.mainType = MSG_MMS_TYPE;
	reqmsg->msgType.subType = MSG_READREPLY_MMS;

	reqmsg->dataSize = sizeof(MSG_READ_REPORT_STATUS_T);
	reqmsg->pMmsData = (char *)calloc(reqmsg->dataSize, 1);

	memcpy((char *)reqmsg->pMmsData, &mms_read_status, sizeof(MSG_READ_REPORT_STATUS_T));

	MSG_DEBUG("mms_read_status [%d]", mms_read_status);

	err = msg_submit_req(handle, &req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	free(reqmsg->pMmsData);

	return err;
}


EXPORT_API int msg_mms_forward_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || !req )
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req->msg;

	req->reqId = 1;

	reqmsg->msgType.mainType = MSG_MMS_TYPE;
	reqmsg->msgType.subType = MSG_FORWARD_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	return err;
}


EXPORT_API int msg_mms_retrieve_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	if ( handle == NULL|| !req)
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req->msg;

	reqmsg->msgType.mainType = MSG_MMS_TYPE;
	reqmsg->msgType.subType = MSG_RETRIEVE_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_RETRIEVING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	return err;
}


/* reject_msg_support */
EXPORT_API int msg_mms_reject_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	if (handle == NULL || !req )
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *reqmsg = (MSG_MESSAGE_S*) req->msg;

	reqmsg->msgType.mainType = MSG_MMS_TYPE;
	reqmsg->msgType.subType = MSG_NOTIFYRESPIND_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	return err;
}
/* reject_msg_support */

