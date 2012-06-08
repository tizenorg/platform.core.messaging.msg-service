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


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgSettingTypes.h"
#include "MapiTransport.h"
#include "MapiSetting.h"
#include "MsgTestTransport.h"
#include "main.h"
#include "MapiMessage.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

MSG_ERROR_T MsgTestSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt)
{
	if ( !hMsgHandle || !pMsg )
	{
		MSG_DEBUG("Handle or pMsg is NULL");
		return MSG_ERR_NULL_MSGHANDLE;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_REQUEST_S req = {0};

	req.msg = pMsg;

	if( pSendOpt )
		memcpy(&req.sendOpt, pSendOpt, sizeof(req.sendOpt));

	MSG_DEBUG("==== [MsgTestSubmitReq] MSG ID = [%d] ====", msg_get_message_id(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Folder ID = [%d] ====", msg_get_folder_id(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Message Type = [%d] ====", msg_get_message_type(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] # number = [%d] ====", msg_get_address_count(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] to = [%s] ====", msg_get_ith_address(req.msg, 0));
//	MSG_DEBUG("==== [MsgTestSubmitReq] Message Data = [%s] ====", msg_sms_get_message_body(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Message Data size = [%d] ====", msg_get_message_body_size(req.msg));

	MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : bSetting = [%d] ====", req.sendOpt.bSetting);
	MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : bDelivery = [%d] ====", req.sendOpt.bDeliverReq);
	MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : bKeepCopy = [%d] ====", req.sendOpt.bKeepCopy);

	if(msg_is_sms(pMsg))
		MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : bReplyPath = [%d] ====", req.sendOpt.option.smsSendOpt.bReplyPath);
	else if(msg_is_mms(pMsg))
	{
		MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : bReadReq = [%d] ====", req.sendOpt.option.mmsSendOpt.bReadReq);
		MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : priority = [%d] ====", req.sendOpt.option.mmsSendOpt.priority);
		MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : expiryTime = [%lu] ====", req.sendOpt.option.mmsSendOpt.expiryTime);
		MSG_DEBUG("==== [MsgTestSubmitReq] sendOpt : deliveryTime = [%lu] ====", req.sendOpt.option.mmsSendOpt.deliveryTime);
	}

	print("Start Sending Message...");

	if (msg_is_mms(req.msg))
	{
		if (msg_get_message_type(req.msg) == MSG_TYPE_MMS)
			err = msg_mms_send_message(hMsgHandle, &req);
		else
			err = msg_mms_retrieve_message(hMsgHandle, &req);
	}
	else
		err = msg_sms_send_message(hMsgHandle, &req);

	if (err == MSG_SUCCESS)
		printf("Sending Message is successful!!!");
	else
		printf("Sending Message is failed!!! %d", err);

	return err;
}

MSG_ERROR_T MsgTestScheduledSubmitReq(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg, MSG_SENDINGOPT_S* pSendOpt)
{
	if ( !hMsgHandle || !pMsg )
	{
		MSG_DEBUG("Handle is NULL");
		return MSG_ERR_NULL_MSGHANDLE;
	}

	MSG_REQUEST_S req = {0};

	req.msg = pMsg;
	if( pSendOpt )
		req.sendOpt = *pSendOpt; //	memcpy(&req.sendOpt, pSendOpt, sizeof(req.sendOpt));

		time_t scheduledTime;

		time(&scheduledTime);
		scheduledTime = scheduledTime + 180;
	msg_set_scheduled_time(req.msg, scheduledTime);

	MSG_DEBUG("==== [MsgTestSubmitReq] MSG ID = [%d] ====", msg_get_message_id(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Folder ID = [%d] ====", msg_get_folder_id(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Msg Type = [%d] ====", msg_get_message_type(req.msg));
//	MSG_DEBUG("==== [MsgTestSubmitReq] Message Data = [%s] ====", msg_sms_get_message_body(req.msg));
	MSG_DEBUG("==== [MsgTestSubmitReq] Message Data size = [%d] ====", msg_get_message_body_size(req.msg));

	print("Start Sending Message...");

	MSG_ERROR_T err = MSG_SUCCESS;

	if (msg_is_mms(req.msg))
		{
		if (msg_get_message_type(req.msg) == MSG_TYPE_MMS)
			err = msg_mms_send_message(hMsgHandle, &req);
		else
			err = msg_mms_retrieve_message(hMsgHandle, &req);
	}
	else
		err = msg_sms_send_message(hMsgHandle, &req);

	if (err == MSG_SUCCESS)
		print("Sending Message is OK!");
	else
		print("Sending Message is failed!");

	return err;
}

