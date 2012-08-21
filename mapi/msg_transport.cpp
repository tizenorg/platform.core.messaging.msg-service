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
#include "MsgUtilFunction.h"

#include "msg.h"
#include "msg_private.h"
#include "msg_transport.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_submit_req(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL ||req == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pStruct = (msg_struct_s *)req;

	try
	{
		err = pHandle->submitReq((MSG_REQUEST_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_TRANSPORT_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_sent_status_callback(msg_handle_t handle, msg_sent_status_cb cb, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_reg_sms_message_callback(msg_handle_t handle, msg_sms_incoming_cb cb, unsigned short port, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_reg_mms_conf_message_callback(msg_handle_t handle, msg_mms_conf_msg_incoming_cb cb, const char *app_id, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_reg_syncml_message_callback(msg_handle_t handle,  msg_syncml_msg_incoming_cb cb, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_reg_lbs_message_callback(msg_handle_t handle, msg_lbs_msg_incoming_cb cb, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_reg_syncml_message_operation_callback(msg_handle_t handle,  msg_syncml_msg_operation_cb cb, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

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


EXPORT_API int msg_syncml_message_operation(msg_handle_t handle,  msg_message_id_t msgId)
{
	msg_error_t err =  MSG_SUCCESS;

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


static msg_handle_t msgHandle = NULL;
static msg_simple_sent_status_cb sentStatusCallback = NULL;

static void sent_status_cb_func(msg_handle_t handle, msg_struct_t sent_status, void *user_param)
{
//	MSG_DEBUG("Sent Status [%d]", sent_status->status);

	msg_simple_sent_status_cb pfunc = sentStatusCallback;
// TODO : Fixme
	pfunc((msg_struct_t)sent_status, user_param);

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

	msg_struct_s req = {0,};
	MSG_REQUEST_S msgReq = {0};

	req.type = MSG_STRUCT_REQUEST_INFO;
	req.data = (void *)&msgReq;

	msg_error_t retVal = MSG_SUCCESS;

	// Open control handle instance
	if ((retVal = msg_open_msg_handle(&msgHandle)) != MSG_SUCCESS)
	{
		MSG_DEBUG("MsgOpenMsgHandle is failed. Error Code = %d", retVal);
		return retVal;
	}

	//msgReq.msg = msg_new_message();
	MSG_MESSAGE_HIDDEN_S msg_info = {0,};
	msg_struct_s msg = {0,};

	msg.type = MSG_STRUCT_MESSAGE_INFO;
	msg.data = &msg_info;

	/* when sending SMS */
	msg_info.mainType = MSG_SMS_TYPE;
	msg_info.subType = MSG_NORMAL_SMS;
	msg_info.msgId = 0;
	msg_info.folderId = MSG_OUTBOX_ID;

	/* fill the destination number in msgReq */
	msg_struct_list_s addr_list = {0,};

	addr_list.nCount = 1;

	msg_struct_s addr_info[addr_list.nCount];
	memset(addr_info, 0, sizeof(msg_struct_s) * addr_list.nCount);
	addr_list.msg_struct_info = (msg_struct_t *)&addr_info;

	MSG_ADDRESS_INFO_S address[addr_list.nCount];
	memset(address, 0, sizeof(MSG_ADDRESS_INFO_S) * addr_list.nCount);

	for (int i = 0; i < addr_list.nCount; i++) {
		addr_info[i].type = MSG_STRUCT_ADDRESS_INFO;
		addr_info[i].data = (void *)&address[i];

		address[i].addressType = MSG_ADDRESS_TYPE_PLMN;
		snprintf(address[i].addressVal, MAX_ADDRESS_VAL_LEN+1, "%s", phone_num);

		address[i].recipientType = MSG_RECIPIENTS_TYPE_TO;
	}

	msg_info.addr_list = &addr_list;

	msg_info.bPortValid = false;

	/* fill the msg text in msgReq */
	msg_info.dataSize = strlen(sms_text);
	msg_info.pData = (void*)malloc(msg_info.dataSize+1);
	strncpy((char *)msg_info.pData, sms_text, msg_info.dataSize);

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
	msgReq.msg = (msg_struct_t)&msg;

	retVal = msg_submit_req(msgHandle, (msg_struct_t)&req);

	if (retVal != MSG_SUCCESS)
	{
		MSG_DEBUG("msg_submit_req() is failed. Error Code = %d", retVal);
		msg_close_msg_handle(&msgHandle);
		return retVal;
	}

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

	msg_error_t retVal = msg_verify_number(phone_num_list, trimmed_num);

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


EXPORT_API int msg_sms_send_message(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || !req) {
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	msg_struct_s *req_s = (msg_struct_s *)req;
	MSG_REQUEST_S *pReq = (MSG_REQUEST_S *)req_s->data;

	pReq->reqId = 1;

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;

	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	if (reqmsg->dataSize <= 0) {
		MSG_FATAL("msg size is invalid : [%d]", reqmsg->dataSize);
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (reqmsg->mainType != MSG_SMS_TYPE) {
		MSG_DEBUG("mainType is not SMS [%d]", reqmsg->mainType);
		reqmsg->mainType = MSG_SMS_TYPE;
	}

	if (reqmsg->subType > MSG_CONCAT_SIM_SMS) {
		MSG_DEBUG("subType is not SMS [%d]", reqmsg->subType);
		reqmsg->subType = MSG_NORMAL_SMS;
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


EXPORT_API int msg_mms_send_message(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || !req)
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_struct_s *req_s = (msg_struct_s *)req;
	MSG_REQUEST_S *pReq = (MSG_REQUEST_S *)req_s->data;

	pReq->reqId = 1;

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;

	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	if (reqmsg->dataSize <= 0)
	{
		MSG_FATAL("MMS data size is invalid");
		return MSG_ERR_INVALID_PARAMETER;
	}

	reqmsg->mainType = MSG_MMS_TYPE;
	reqmsg->subType = MSG_SENDREQ_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed! [%d]", err);

	return err;
}


EXPORT_API int msg_mms_send_read_report(msg_handle_t handle, msg_message_id_t msgId, msg_read_report_status_t mms_read_status)
{
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
	{
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_struct_t req_t = msg_create_struct(MSG_STRUCT_REQUEST_INFO);

	msg_struct_s *req_s = (msg_struct_s *)req_t;
	MSG_REQUEST_S *req = (MSG_REQUEST_S *)req_s->data;
	msg_struct_s *msg_s = (msg_struct_s *)req->msg;
	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S *)msg_s->data;
	void *read_report_data = NULL;
	size_t read_report_datasize;

	read_report_datasize = sizeof(msg_read_report_status_t);
	read_report_data = (void *)calloc(read_report_datasize, 1);
	if(read_report_data == NULL) {
		return MSG_ERR_MEMORY_ERROR;
	}

	MSG_DEBUG("mms_read_status [%d]", mms_read_status);
	memcpy(read_report_data, &mms_read_status, read_report_datasize);

	req->reqId = 1;

	reqmsg->bPortValid = false;
	reqmsg->msgId = msgId;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->mainType = MSG_MMS_TYPE;
	reqmsg->subType = MSG_READREPLY_MMS;

	reqmsg->dataSize = read_report_datasize;
	reqmsg->pMmsData = read_report_data;

	err = msg_submit_req(handle, req_t);
	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	free(read_report_data);
	read_report_data = NULL;
	reqmsg->pMmsData = NULL;

	msg_release_struct(&req_t);

	return err;
}


EXPORT_API int msg_mms_forward_message(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || !req )
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_struct_s *req_s = (msg_struct_s *)req;
	MSG_REQUEST_S *pReq = (MSG_REQUEST_S *)req_s->data;

	pReq->reqId = 1;

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;

	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	reqmsg->mainType = MSG_MMS_TYPE;
	reqmsg->subType = MSG_FORWARD_MMS;
	reqmsg->folderId = MSG_OUTBOX_ID; // outbox fixed
	reqmsg->networkStatus = MSG_NETWORK_SENDING;

	err = msg_submit_req(handle, req);

	if (err == MSG_SUCCESS)
		MSG_DEBUG("Sending Message is OK!");
	else
		MSG_DEBUG("Sending Message is failed!");

	return err;
}


EXPORT_API int msg_mms_retrieve_message(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err = MSG_SUCCESS;

	if ( handle == NULL|| !req)
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_struct_s *req_s = (msg_struct_s *)req;
	MSG_REQUEST_S *pReq = (MSG_REQUEST_S *)req_s->data;

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;
	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	reqmsg->mainType = MSG_MMS_TYPE;
	reqmsg->subType = MSG_RETRIEVE_MMS;
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
EXPORT_API int msg_mms_reject_message(msg_handle_t handle, msg_struct_t req)
{
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || !req )
	{
		MSG_FATAL("handle or req is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_struct_s *req_s = (msg_struct_s *)req;
	MSG_REQUEST_S *pReq = (MSG_REQUEST_S *)req_s->data;

	msg_struct_s *msg_s = (msg_struct_s *)pReq->msg;
	MSG_MESSAGE_HIDDEN_S *reqmsg = (MSG_MESSAGE_HIDDEN_S*)msg_s->data;

	reqmsg->mainType = MSG_MMS_TYPE;
	reqmsg->subType = MSG_NOTIFYRESPIND_MMS;
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


int msg_request_get_int(void *request_info, int field)
{
	int result = -1;
	MSG_REQUEST_S *pRequest = (MSG_REQUEST_S *)request_info;
	switch(field)
	{
	case MSG_REQUEST_REQUESTID_INT:
		result = pRequest->reqId;
		break;

	default:
		break;
	}
	return result;
}


int msg_request_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err =  MSG_SUCCESS;

	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_REQUEST_S *pRequest = (MSG_REQUEST_S *)msg_struct->data;
	switch(field)
	{
	case MSG_REQUEST_MESSAGE_HND:
		*value = (void *)pRequest->msg;
		break;
	case MSG_REQUEST_SENDOPT_HND:
		*value = (void *)pRequest->sendOpt;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;

	}
	return err;
}

int msg_request_set_int(void *request_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!request_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REQUEST_S *pRequest = (MSG_REQUEST_S *)request_info;
	switch(field)
	{
	case MSG_REQUEST_REQUESTID_INT:
		pRequest->reqId = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
	break;
	}

	return err;
}

int msg_request_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_REQUEST_S *pRequest = (MSG_REQUEST_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;


	switch(field)
	{
	case MSG_REQUEST_MESSAGE_HND:
	{
		pTmp = (msg_struct_s *)pRequest->msg;
		MSG_MESSAGE_HIDDEN_S *pSrc = (MSG_MESSAGE_HIDDEN_S *)value->data;
		MSG_MESSAGE_HIDDEN_S *pDst = (MSG_MESSAGE_HIDDEN_S *)pTmp->data;
		msg_message_copy_message(pSrc, pDst);
		break;
	}
	case MSG_REQUEST_SENDOPT_HND:
	{
		pTmp = (msg_struct_s *)pRequest->sendOpt;
		MSG_SENDINGOPT_S *pSrc = (MSG_SENDINGOPT_S *)value->data;
		MSG_SENDINGOPT_S *pDst = (MSG_SENDINGOPT_S *)pTmp->data;
		pDst->bDeliverReq = pSrc->bDeliverReq;
		pDst->bKeepCopy = pSrc->bKeepCopy;
		pDst->bSetting = pSrc->bSetting;

		msg_struct_s *tmpDstMmsSendOpt = (msg_struct_s *)pDst->mmsSendOpt;
		msg_struct_s *tmpDstSmsSendOpt = (msg_struct_s *)pDst->smsSendOpt;

		msg_struct_s *tmpSrcMmsSendOpt = (msg_struct_s *)pDst->mmsSendOpt;
		msg_struct_s *tmpSrcSmsSendOpt = (msg_struct_s *)pDst->smsSendOpt;

		tmpDstMmsSendOpt->type = tmpSrcMmsSendOpt->type;
		memcpy(tmpDstMmsSendOpt->data, tmpSrcMmsSendOpt->data, sizeof(MMS_SENDINGOPT_S));

		tmpDstSmsSendOpt->type = tmpSrcSmsSendOpt->type;
		memcpy(tmpDstSmsSendOpt->data, tmpSrcSmsSendOpt->data, sizeof(SMS_SENDINGOPT_S));

		break;
	}
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}
	return err;
}


int msg_sent_status_get_int(MSG_SENT_STATUS_S *sent_status_info, int field)
{
	int result = -1;

	switch(field)
	{
	case MSG_SENT_STATUS_REQUESTID_INT:
		result = sent_status_info->reqId;
		break;
	case MSG_SENT_STATUS_NETWORK_STATUS_INT:
		result = sent_status_info->status;
		break;
	default:
		break;
	}
	return result;
}
