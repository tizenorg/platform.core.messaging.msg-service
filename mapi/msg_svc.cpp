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

#include <string.h>

#include "MsgDebug.h"
#include "MsgTypes.h"
#include "MsgHandle.h"
#include "MsgTextConvert.h"
#include "MsgException.h"
#include "MsgMemory.h"

#include "msg_private.h"
#include "msg.h"

void __msg_release_list_item(gpointer data)
{
	if (data == NULL)
		return;

	msg_release_struct((msg_struct_t*)(&data));
}


EXPORT_API msg_struct_t msg_create_struct(int field)
{
	CHECK_MSG_SUPPORTED_RETURN_NULL(MSG_TELEPHONY_SMS_FEATURE);
	msg_struct_s *msg_struct = new msg_struct_s;
	memset(msg_struct, 0x00, sizeof(msg_struct_s));

	msg_struct->type = field;
	msg_struct->data = NULL;

	switch (field) {
	case MSG_STRUCT_MESSAGE_INFO:
		msg_message_create_struct(msg_struct);
		break;
	case MSG_STRUCT_CONV_INFO: {
		msg_struct->data = (void *)new MSG_CONVERSATION_VIEW_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_CONVERSATION_VIEW_S));
		break;
	}
	case MSG_STRUCT_FILTER: {
		msg_struct->data = (void *)new MSG_FILTER_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_FILTER_S));
		break;
	}
	case MSG_STRUCT_THREAD_INFO: {
		msg_struct->data = (void *)new MSG_THREAD_VIEW_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_THREAD_VIEW_S));
		break;
	}
	case MSG_STRUCT_SENDOPT: {
		MSG_SENDINGOPT_S *pOpt = (MSG_SENDINGOPT_S *)new MSG_SENDINGOPT_S;
		memset(pOpt, 0x00, sizeof(MSG_SENDINGOPT_S));
		msg_struct->data = pOpt;

		msg_struct_s *pSms = new msg_struct_s;
		msg_struct_s *pMms = new msg_struct_s;

		pOpt->mmsSendOpt = (msg_struct_t)pMms;
		pOpt->smsSendOpt = (msg_struct_t)pSms;

		pMms->type = MSG_STRUCT_MMS_SENDOPT;
		pMms->data = new MMS_SENDINGOPT_S;
		memset(pMms->data, 0x00, sizeof(MMS_SENDINGOPT_S));

		pSms->type = MSG_STRUCT_SMS_SENDOPT;
		pSms->data = new SMS_SENDINGOPT_S;
		memset(pSms->data, 0x00, sizeof(SMS_SENDINGOPT_S));
		break;
	}
	case MSG_STRUCT_SYNCML_INFO: {
		MSG_SYNCML_MESSAGE_S *pSyncML = (MSG_SYNCML_MESSAGE_S *)new MSG_SYNCML_MESSAGE_S;
		memset(pSyncML, 0x00, sizeof(MSG_SYNCML_MESSAGE_S));
		msg_struct->data = pSyncML;

		msg_struct_s *pStruct = new msg_struct_s;
		pSyncML->msg = (msg_struct_t)pStruct;
		pStruct->type = MSG_STRUCT_MESSAGE_INFO;
		msg_message_create_struct(pStruct);

		break;
	}
	case MSG_STRUCT_COUNT_INFO: {
		msg_struct->data = new MSG_COUNT_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_COUNT_INFO_S));
		break;
	}
	case MSG_STRUCT_THREAD_COUNT_INFO: {
		msg_struct->data = new MSG_THREAD_COUNT_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_THREAD_COUNT_INFO_S));
		break;
	}
	case MSG_STRUCT_THREAD_LIST_INDEX: {
		MSG_THREAD_LIST_INDEX_INFO_S * pThread = (MSG_THREAD_LIST_INDEX_INFO_S *)new MSG_THREAD_LIST_INDEX_INFO_S;
		memset(pThread, 0x00, sizeof(MSG_THREAD_LIST_INDEX_INFO_S));

		msg_struct->data = (void *)pThread;

		msg_struct_s *pStruct = new msg_struct_s;
		pThread->msgAddrInfo = (msg_struct_t)pStruct;

		pStruct->type = MSG_STRUCT_ADDRESS_INFO;
		pStruct->data = new MSG_ADDRESS_INFO_S;
		memset(pStruct->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

		break;
	}
	case MSG_STRUCT_SORT_RULE: {
		msg_struct->data = new MSG_SORT_RULE_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_SORT_RULE_S));
		break;
	}
	case MSG_STRUCT_FOLDER_INFO: {
		msg_struct->data = new MSG_FOLDER_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_FOLDER_INFO_S));
		break;
	}
	case MSG_STRUCT_MSG_LIST_CONDITION: {
		msg_struct->data = new MSG_LIST_CONDITION_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_LIST_CONDITION_S));

		msg_struct_s *pStruct = new msg_struct_s;
		((MSG_LIST_CONDITION_S *)msg_struct->data)->sortRule = (msg_struct_t)pStruct;

		pStruct->type = MSG_STRUCT_SORT_RULE;
		pStruct->data = new MSG_SORT_RULE_S;
		memset(pStruct->data, 0x00, sizeof(MSG_SORT_RULE_S));

		break;
	}
	case MSG_STRUCT_REPORT_STATUS_INFO: {
		msg_struct->data = new MSG_REPORT_STATUS_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_REPORT_STATUS_INFO_S));
		break;
	}
	case MSG_STRUCT_SETTING_SMSC_OPT: {
		msg_struct->data = new MSG_SMSC_LIST_HIDDEN_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_SMSC_LIST_HIDDEN_S));

		MSG_SMSC_LIST_HIDDEN_S *pTmp = (MSG_SMSC_LIST_HIDDEN_S *)msg_struct->data;
		pTmp->simIndex = MSG_SIM_SLOT_ID_1; /* default sim index */

		msg_struct_list_s *smsc_list = (msg_struct_list_s *)new msg_struct_list_s;
		memset(smsc_list, 0x00, sizeof(msg_struct_list_s));

		pTmp->smsc_list = smsc_list;

		smsc_list->msg_struct_info = (msg_struct_t *)calloc(SMSC_LIST_MAX, sizeof(msg_struct_t));

		if (smsc_list->msg_struct_info != NULL) {
			msg_struct_s *pStructTmp = NULL;

			for (int i = 0; i < SMSC_LIST_MAX; i++) {
				pStructTmp = (msg_struct_s *)new msg_struct_s;
				pStructTmp->type = MSG_STRUCT_SETTING_SMSC_INFO;
				pStructTmp->data = new MSG_SMSC_DATA_S;
				memset(pStructTmp->data, 0x00, sizeof(MSG_SMSC_DATA_S));
				smsc_list->msg_struct_info[i] = (msg_struct_t)pStructTmp;
			}
		}
		break;
	}
	case MSG_STRUCT_SETTING_SMSC_INFO: {
		msg_struct->data = new MSG_SMSC_DATA_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_SMSC_DATA_S));
		break;
	}
	case MSG_STRUCT_SETTING_CB_OPT: {
		msg_struct->data = new MSG_CBMSG_OPT_HIDDEN_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_CBMSG_OPT_HIDDEN_S));

		MSG_CBMSG_OPT_HIDDEN_S *pTmp = (MSG_CBMSG_OPT_HIDDEN_S *)msg_struct->data;
		pTmp->simIndex = MSG_SIM_SLOT_ID_1; /* default sim index */

		pTmp->channelData = (msg_struct_list_s *)new msg_struct_list_s;
		memset(pTmp->channelData, 0x00, sizeof(msg_struct_list_s));

		pTmp->channelData->msg_struct_info = (msg_struct_t *)calloc(CB_CHANNEL_MAX, sizeof(msg_struct_t));

		if (pTmp->channelData->msg_struct_info != NULL) {
			msg_struct_s *pStructTmp = NULL;

			for (int i = 0; i < CB_CHANNEL_MAX; i++) {
				pStructTmp = (msg_struct_s *)new msg_struct_s;
				pStructTmp->type = MSG_STRUCT_SETTING_CB_CHANNEL_INFO;
				pStructTmp->data = new MSG_CB_CHANNEL_INFO_S;

				memset(pStructTmp->data, 0x00, sizeof(MSG_CB_CHANNEL_INFO_S));
				pTmp->channelData->msg_struct_info[i] = (msg_struct_t)pStructTmp;
			}
		}
		break;
	}
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO: {
		msg_struct->data = new MSG_CB_CHANNEL_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_CB_CHANNEL_INFO_S));
		break;
	}
	case MSG_STRUCT_SETTING_SMS_SEND_OPT: {
		msg_struct->data = new MSG_SMS_SENDOPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_SMS_SENDOPT_S));
		break;
	}
	case MSG_STRUCT_SETTING_MMS_SEND_OPT: {
		msg_struct->data = new MSG_MMS_SENDOPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_MMS_SENDOPT_S));
		break;
	}
	case MSG_STRUCT_SETTING_MMS_RECV_OPT: {
		msg_struct->data = new MSG_MMS_RECVOPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_MMS_RECVOPT_S));
		break;
	}
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT: {
		msg_struct->data = new MSG_PUSHMSG_OPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_PUSHMSG_OPT_S));
		break;
	}
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT: {
		msg_struct->data = new MSG_VOICEMAIL_OPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_VOICEMAIL_OPT_S));
		MSG_VOICEMAIL_OPT_S *pTmp = (MSG_VOICEMAIL_OPT_S *)msg_struct->data;
		pTmp->simIndex = MSG_SIM_SLOT_ID_1; /* default sim index */
		break;
	}
	case MSG_STRUCT_SETTING_GENERAL_OPT: {
		msg_struct->data = new MSG_GENERAL_OPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_GENERAL_OPT_S));
		break;
	}
	case MSG_STRUCT_SETTING_MSGSIZE_OPT: {
		msg_struct->data = new MSG_MSGSIZE_OPT_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_MSGSIZE_OPT_S));
		break;
	}
	case MSG_STRUCT_SMS_SENDOPT: {
		msg_struct->data = new SMS_SENDINGOPT_S;
		memset(msg_struct->data, 0x00, sizeof(SMS_SENDINGOPT_S));
		break;
	}
	case MSG_STRUCT_MMS_SENDOPT: {
		msg_struct->data = new MMS_SENDINGOPT_S;
		memset(msg_struct->data, 0x00, sizeof(MMS_SENDINGOPT_S));
		break;
	}
	case MSG_STRUCT_REQUEST_INFO: {
		MSG_REQUEST_S *pRequest = (MSG_REQUEST_S *)new MSG_REQUEST_S;
		memset(pRequest, 0x00, sizeof(MSG_REQUEST_S));
		msg_struct->data = pRequest;

		msg_struct_s *pMsg = (msg_struct_s *)new msg_struct_s;
		msg_struct_s *pOpt = (msg_struct_s *)new msg_struct_s;

		pRequest->msg = (msg_struct_t)pMsg;
		pRequest->sendOpt = (msg_struct_t)pOpt;

		pMsg->type = MSG_STRUCT_MESSAGE_INFO;
		msg_message_create_struct(pMsg);

		pOpt->type = MSG_STRUCT_SENDOPT;
		pOpt->data = new MSG_SENDINGOPT_S;
		memset(pOpt->data, 0x00, sizeof(MSG_SENDINGOPT_S));
		msg_struct_s *pSms = new msg_struct_s;
		msg_struct_s *pMms = new msg_struct_s;

		MSG_SENDINGOPT_S *sendOpt = (MSG_SENDINGOPT_S *)pOpt->data;

		sendOpt->mmsSendOpt = (msg_struct_t)pMms;
		sendOpt->smsSendOpt = (msg_struct_t)pSms;

		pMms->type = MSG_STRUCT_MMS_SENDOPT;
		pMms->data = new MMS_SENDINGOPT_S;
		memset(pMms->data, 0x00, sizeof(MMS_SENDINGOPT_S));

		pSms->type = MSG_STRUCT_SMS_SENDOPT;
		pSms->data = new SMS_SENDINGOPT_S;
		memset(pSms->data, 0x00, sizeof(SMS_SENDINGOPT_S));
		break;
	}
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
	case MSG_STRUCT_MULTIPART_INFO:
		msg_struct->data = msg_mms_create_struct_data(field);
		break;
	case MSG_STRUCT_PUSH_CONFIG_INFO:
		msg_struct->data = new MSG_PUSH_EVENT_INFO_S;
		memset(msg_struct->data, 0x00, sizeof(MSG_PUSH_EVENT_INFO_S));
		break;
	default:
		delete msg_struct;
		return NULL;
		break;
	}

	return (msg_struct_t) msg_struct;
}


static int _release_msg_struct(msg_struct_t *msg_struct_handle)
{
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || *msg_struct_handle == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)*msg_struct_handle;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO: {
		msg_message_release(&msg_struct);

		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_FILTER: {
		delete (MSG_FILTER_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_CONV_INFO: {
		MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S*)(msg_struct->data);

		if (pConv->multipart_list) {
			g_list_free_full((GList *)pConv->multipart_list, __msg_release_list_item);
			pConv->multipart_list = NULL;
		}

		if (pConv->pText) {
			delete [] pConv->pText;
			pConv->pText = NULL;
		}

		delete pConv;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_THREAD_INFO: {
		delete (MSG_THREAD_VIEW_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SENDOPT: {
		MSG_SENDINGOPT_S *pOpt = (MSG_SENDINGOPT_S*)(msg_struct->data);
		delete (MMS_SENDINGOPT_S *)(((msg_struct_s *)pOpt->mmsSendOpt)->data);
		((msg_struct_s *)pOpt->mmsSendOpt)->data = NULL;
		delete (msg_struct_s *)pOpt->mmsSendOpt;
		pOpt->mmsSendOpt = NULL;

		delete (SMS_SENDINGOPT_S *)(((msg_struct_s *)pOpt->smsSendOpt)->data);
		((msg_struct_s *)pOpt->smsSendOpt)->data = NULL;
		delete (msg_struct_s *)pOpt->smsSendOpt;
		pOpt->smsSendOpt = NULL;

		delete pOpt;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_SYNCML_INFO: {
		MSG_SYNCML_MESSAGE_S *pSyncML = (MSG_SYNCML_MESSAGE_S*)(msg_struct->data);
		msg_struct_s *msg = (msg_struct_s *)pSyncML->msg;
		msg_message_release(&msg);

		delete pSyncML;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_COUNT_INFO: {
		delete (MSG_COUNT_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_THREAD_COUNT_INFO: {
		delete (MSG_THREAD_COUNT_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_THREAD_LIST_INDEX: {
		MSG_THREAD_LIST_INDEX_INFO_S *pThread = (MSG_THREAD_LIST_INDEX_INFO_S*)(msg_struct->data);
		delete (MSG_ADDRESS_INFO_S *)(((msg_struct_s *)pThread->msgAddrInfo)->data);
		((msg_struct_s *)pThread->msgAddrInfo)->data = NULL;
		delete (msg_struct_s *)pThread->msgAddrInfo;
		pThread->msgAddrInfo = NULL;
		delete pThread;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_SORT_RULE: {
		delete (MSG_SORT_RULE_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_FOLDER_INFO: {
		delete (MSG_FOLDER_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_MSG_LIST_CONDITION: {
		MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S*)(msg_struct->data);
		MSG_SORT_RULE_S *pSortRule = (MSG_SORT_RULE_S *)(((msg_struct_s *)pCond->sortRule)->data);

		delete (MSG_SORT_RULE_S *)pSortRule;
		((msg_struct_s *)pCond->sortRule)->data = NULL;
		delete (msg_struct_s *)pCond->sortRule;
		pCond->sortRule = NULL;

		if (pCond->pAddressVal) {
			delete [] pCond->pAddressVal;
			pCond->pAddressVal = NULL;
		}

		if (pCond->pTextVal) {
			delete [] pCond->pTextVal;
			pCond->pTextVal = NULL;
		}

		delete (MSG_LIST_CONDITION_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_REPORT_STATUS_INFO: {
		delete (MSG_REPORT_STATUS_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_SMS_SENDOPT: {
		delete (SMS_SENDINGOPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_MMS_SENDOPT: {
		delete (MMS_SENDINGOPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;

		break;
	}
	case MSG_STRUCT_REQUEST_INFO: {
		MSG_REQUEST_S *pRequest = (MSG_REQUEST_S*)(msg_struct->data);

		MSG_SENDINGOPT_S *pSendingOpt = (MSG_SENDINGOPT_S *)(((msg_struct_s *)pRequest->sendOpt)->data);

		msg_struct_s *pMmsOpt = (msg_struct_s *)pSendingOpt->mmsSendOpt;
		msg_struct_s *pSmsOpt = (msg_struct_s *)pSendingOpt->smsSendOpt;

		delete (MMS_SENDINGOPT_S *)(pMmsOpt->data);
		pMmsOpt->data = NULL;
		delete (msg_struct_s *)pSendingOpt->mmsSendOpt;
		pSendingOpt->mmsSendOpt = NULL;
		delete (MMS_SENDINGOPT_S *)(pSmsOpt->data);
		pSmsOpt->data = NULL;
		delete (msg_struct_s *)pSendingOpt->smsSendOpt;
		pSendingOpt->smsSendOpt = NULL;

		delete (MSG_SENDINGOPT_S *)pSendingOpt;
		((msg_struct_s *)pRequest->sendOpt)->data = NULL;
		delete (msg_struct_s *)pRequest->sendOpt;
		pRequest->sendOpt = NULL;

		msg_struct_s *msg = (msg_struct_s *)pRequest->msg;
		msg_message_release(&msg);

		delete pRequest;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
	case MSG_STRUCT_MULTIPART_INFO: {
		msg_mms_release_struct(&msg_struct);
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_SMSC_OPT: {
		msg_struct_list_s *smsc_list = NULL;
		msg_struct_s *smsc_info = NULL;
		MSG_SMSC_LIST_HIDDEN_S *pTmp = NULL;

		pTmp = (MSG_SMSC_LIST_HIDDEN_S *)msg_struct->data;
		smsc_list = (msg_struct_list_s *)pTmp->smsc_list;

		for (int i = 0; i < SMSC_LIST_MAX; i++) {
			smsc_info = (msg_struct_s *)smsc_list->msg_struct_info[i];
			delete (MSG_SMSC_DATA_S*)(smsc_info->data);
			delete smsc_info;
		}

		g_free(smsc_list->msg_struct_info);

		delete smsc_list;

		delete pTmp;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO: {
		delete (MSG_CB_CHANNEL_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_CB_OPT: {
		msg_struct_list_s *cb_list = NULL;
		msg_struct_s *cb_info = NULL;
		MSG_CBMSG_OPT_HIDDEN_S *pTmp = NULL;

		pTmp = (MSG_CBMSG_OPT_HIDDEN_S *)msg_struct->data;
		cb_list = (msg_struct_list_s *)pTmp->channelData;

		for (int i = 0; i < CB_CHANNEL_MAX; i++) {
			cb_info = (msg_struct_s *)cb_list->msg_struct_info[i];
			delete (MSG_CB_CHANNEL_INFO_S*)(cb_info->data);
			delete cb_info;
		}

		g_free(cb_list->msg_struct_info);

		delete cb_list;

		delete pTmp;
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_SMS_SEND_OPT: {
		delete (MSG_SMS_SENDOPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_MMS_SEND_OPT: {
		delete (MSG_MMS_SENDOPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_MMS_RECV_OPT: {
		delete (MSG_MMS_RECVOPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT: {
		delete (MSG_PUSHMSG_OPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT: {
		delete (MSG_VOICEMAIL_OPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_GENERAL_OPT: {
		delete (MSG_GENERAL_OPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_SETTING_MSGSIZE_OPT: {
		delete (MSG_MSGSIZE_OPT_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_ADDRESS_INFO: {
		delete (MSG_ADDRESS_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_PUSH_CONFIG_INFO: {
		delete (MSG_PUSH_EVENT_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	case MSG_STRUCT_MEDIA_INFO: {
		delete (MSG_MEDIA_INFO_S*)(msg_struct->data);
		msg_struct->data = NULL;

		delete msg_struct;
		*msg_struct_handle = NULL;
		break;
	}
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_release_struct(msg_struct_t *msg_struct_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	int ret = _release_msg_struct(msg_struct_handle);

	return ret;
}

EXPORT_API int msg_release_list_struct(msg_struct_list_s *msg_struct_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_list == NULL)
		return MSG_ERR_NULL_POINTER;

	if (msg_struct_list->msg_struct_info == NULL) {
		if (msg_struct_list->nCount > 0)
			msg_struct_list->nCount = 0;
		return MSG_ERR_NULL_POINTER;
	}

	if (msg_struct_list->nCount > 0) {
		int listCnt = msg_struct_list->nCount;

		for (int i = 0; i < listCnt; i++) {
			_release_msg_struct(&(msg_struct_list->msg_struct_info[i]));
		}
	}

	/* free peer info list */
	g_free(msg_struct_list->msg_struct_info);
	msg_struct_list->msg_struct_info = NULL;

	msg_struct_list->nCount = 0;

	return err;
}

EXPORT_API int msg_get_int_value(msg_struct_t msg_struct_handle, int field, int *value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;
	MSG_TYPE_CHECK(msg_struct->type, field);

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_get_int_value(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_FILTER:
		err = msg_get_filter_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SYNCML_INFO:
		err = msg_syncml_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_COUNT_INFO:
		err = msg_count_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_COUNT_INFO:
		err = msg_thread_count_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_LIST_INDEX:
		err = msg_thread_index_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SORT_RULE:
		err = msg_sortrule_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_FOLDER_INFO:
		err = msg_folder_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_INFO:
		err = msg_thread_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_CONV_INFO:
		err = msg_conv_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REPORT_STATUS_INFO:
		err = msg_report_status_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_ADDRESS_INFO:
		err = msg_address_info_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS_SENDOPT:
		err = msg_mms_sendopt_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REJECT_MSG_INFO:
		err = msg_reject_message_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REQUEST_INFO:
		err = msg_request_get_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SENT_STATUS_INFO:
		err = msg_sent_status_get_int((MSG_SENT_STATUS_S *)msg_struct->data, field, value);
		break;
	case MSG_STRUCT_CB_MSG:
		err = msg_cb_message_get_int_value(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
	case MSG_STRUCT_MULTIPART_INFO:
		err = msg_mms_get_int_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_SETTING_SMSC_OPT:
	case MSG_STRUCT_SETTING_SMSC_INFO:
	case MSG_STRUCT_SETTING_CB_OPT:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
	case MSG_STRUCT_SETTING_GENERAL_OPT:
	case MSG_STRUCT_SETTING_MSGSIZE_OPT:
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_setting_get_int_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_MEDIA_INFO:
		err = msg_media_item_get_int(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_str_value(msg_struct_t msg_struct_handle, int field, char *value, int size)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	if (size < 0)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_get_str_value(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_FILTER:
		err = msg_get_filter_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_FOLDER_INFO:
		err = msg_folder_info_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_THREAD_INFO:
		err = msg_thread_info_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_CONV_INFO:
		err = msg_conv_info_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_get_str(msg_struct->data, field, value, size);
		break;

	case MSG_STRUCT_ADDRESS_INFO:
		err = msg_address_info_get_str(msg_struct->data, field, value, size);
		break;

	case MSG_STRUCT_REJECT_MSG_INFO:
		err = msg_reject_message_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MEDIA_INFO:
		err = msg_media_item_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
	case MSG_STRUCT_MULTIPART_INFO:
		err = msg_mms_get_str_value(msg_struct, field, value, size);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
	case MSG_STRUCT_SETTING_SMSC_INFO:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_setting_get_str_value(msg_struct, field, value, size);
		break;
	case MSG_STRUCT_PUSH_CONFIG_INFO:
		err = msg_push_config_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_REPORT_STATUS_INFO:
		err = msg_report_status_get_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_CB_MSG:
		err = msg_cb_message_get_str_value(msg_struct->data, field, value, size);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_bool_value(msg_struct_t msg_struct_handle, int field, bool *value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_FILTER:
		err = msg_get_filter_info_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_get_bool_value(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_INFO:
		err = msg_thread_info_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_CONV_INFO:
		err = msg_conv_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SENDOPT:
		err = msg_sendopt_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SORT_RULE:
		err = msg_sortrule_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS_SENDOPT:
		err = msg_mms_sendopt_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SMS_SENDOPT:
		err = msg_sms_sendopt_get_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
		err = msg_mms_get_bool_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_setting_get_bool_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_PUSH_CONFIG_INFO:
		err = msg_push_config_get_bool(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t *value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_get_struct_hnd(msg_struct->data, field, (void **)value);
		break;
	case MSG_STRUCT_SENDOPT:
		err = msg_sendopt_get_struct_handle(msg_struct, field, (void **)value);
		break;
	case MSG_STRUCT_SYNCML_INFO:
		err = msg_syncml_get_struct_handle(msg_struct, field, (void **)value);
		break;
	case MSG_STRUCT_THREAD_LIST_INDEX:
		err = msg_thread_index_get_struct_handle(msg_struct, field, (void **)value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_get_struct_handle(msg_struct, field, (void **)value);
		break;
	case MSG_STRUCT_MMS_MEDIA:
		err = msg_mms_get_struct_handle(msg_struct, field, (msg_struct_s**)value);
		break;
	case MSG_STRUCT_REQUEST_INFO:
		err = msg_request_get_struct_handle(msg_struct, field, (void **)value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_get_list_handle(msg_struct_t msg_struct_handle, int field, void **value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_get_list_hnd(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_CONV_INFO:
		err = msg_conversation_get_list_hnd(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
		err = msg_mms_get_list_handle(msg_struct, field, (msg_list_handle_t *)value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
	case MSG_STRUCT_SETTING_SMSC_OPT:
		err = msg_setting_get_list_handle(msg_struct, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_set_int_value(msg_struct_t msg_struct_handle, int field, int value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_set_int_value(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_FILTER:
		err = msg_set_filter_info_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SYNCML_INFO:
		err = msg_syncml_info_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_COUNT_INFO:
		err = msg_count_info_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_COUNT_INFO:
		err = msg_thread_count_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_THREAD_LIST_INDEX:
		err = msg_thread_index_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SORT_RULE:
		err = msg_sortrule_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_FOLDER_INFO:
		err = msg_folder_info_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REPORT_STATUS_INFO:
		err = msg_report_status_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_ADDRESS_INFO:
		err = msg_address_info_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS_SENDOPT:
		err = msg_mms_sendopt_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REJECT_MSG_INFO:
		err = msg_reject_message_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_REQUEST_INFO:
		err = msg_request_set_int(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
		err = msg_mms_set_int_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_SETTING_SMSC_OPT:
	case MSG_STRUCT_SETTING_SMSC_INFO:
	case MSG_STRUCT_SETTING_CB_OPT:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
	case MSG_STRUCT_SETTING_GENERAL_OPT:
	case MSG_STRUCT_SETTING_MSGSIZE_OPT:
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_setting_set_int_value(msg_struct, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_set_str_value(msg_struct_t msg_struct_handle, int field, const char *value, int size)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	if (size < 0)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_set_str_value(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_FILTER:
		err = msg_set_filter_info_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_FOLDER_INFO:
		err = msg_folder_info_set_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_set_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_ADDRESS_INFO:
		err = msg_address_info_set_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_REJECT_MSG_INFO:
		err = msg_reject_message_set_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
	case MSG_STRUCT_MULTIPART_INFO:
		err = msg_mms_set_str_value(msg_struct, field, value, size);
		break;
	case MSG_STRUCT_SETTING_GENERAL_OPT:
	case MSG_STRUCT_SETTING_SMSC_INFO:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_VOICE_MSG_OPT:
		err = msg_setting_set_str_value(msg_struct, field, value, size);
		break;
	case MSG_STRUCT_PUSH_CONFIG_INFO:
		err = msg_push_config_set_str(msg_struct->data, field, value, size);
		break;
	case MSG_STRUCT_MEDIA_INFO:
		err = msg_media_info_set_str(msg_struct->data, field, value, size);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_set_bool_value(msg_struct_t msg_struct_handle, int field, bool value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_FILTER:
		err = msg_set_filter_info_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_set_bool_value(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SENDOPT:
		err = msg_sendopt_set_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SORT_RULE:
		err = msg_sortrule_set_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_set_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS_SENDOPT:
		err = msg_mms_sendopt_set_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_SMS_SENDOPT:
		err = msg_sms_sendopt_set_bool(msg_struct->data, field, value);
		break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE:
	case MSG_STRUCT_MMS_MEDIA:
	case MSG_STRUCT_MMS_ATTACH:
	case MSG_STRUCT_MMS_REGION:
	case MSG_STRUCT_MMS_TRANSITION:
	case MSG_STRUCT_MMS_META:
	case MSG_STRUCT_MMS_SMIL_TEXT:
	case MSG_STRUCT_MMS_SMIL_AVI:
		err = msg_mms_set_bool_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_SETTING_CB_OPT:
	case MSG_STRUCT_SETTING_CB_CHANNEL_INFO:
	case MSG_STRUCT_SETTING_SMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_SEND_OPT:
	case MSG_STRUCT_SETTING_MMS_RECV_OPT:
	case MSG_STRUCT_SETTING_PUSH_MSG_OPT:
	case MSG_STRUCT_SETTING_GENERAL_OPT:
		err = msg_setting_set_bool_value(msg_struct, field, value);
		break;
	case MSG_STRUCT_PUSH_CONFIG_INFO:
		err = msg_push_config_set_bool(msg_struct->data, field, value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_set_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO:
		err = msg_message_set_struct_hnd(msg_struct->data, field, (void *)value);
		break;
	case MSG_STRUCT_SENDOPT:
		err = msg_sendopt_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	case MSG_STRUCT_SYNCML_INFO:
		err = msg_syncml_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	case MSG_STRUCT_THREAD_LIST_INDEX:
		err = msg_thread_index_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	case MSG_STRUCT_MSG_LIST_CONDITION:
		err = msg_list_condition_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	case MSG_STRUCT_MMS_MEDIA:
		err = msg_mms_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	case MSG_STRUCT_REQUEST_INFO:
		err = msg_request_set_struct_handle(msg_struct, field, (msg_struct_s *)value);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_set_list_handle(msg_struct_t msg_struct_handle, int field, void *value)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_struct_handle == NULL || value == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	if (msg_struct->data == NULL)
		return MSG_ERR_NULL_POINTER;

#if 0 /* No operations */
	switch (msg_struct->type) {
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}
#endif

	return err;
}

EXPORT_API int msg_list_add_item(msg_struct_t msg_struct_handle, int field, msg_struct_t *item)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	if (msg_struct_handle == NULL || item == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err = MSG_SUCCESS;
	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO: {
		err = msg_message_list_append(msg_struct_handle, field, item);
	}
	break;
	case MSG_STRUCT_MMS:
	case MSG_STRUCT_MMS_PAGE: {
		err = msg_mms_list_append(msg_struct_handle, field, item);
	}
	break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API msg_struct_t msg_list_nth_data(msg_list_handle_t list_handle, int index)
{
	CHECK_MSG_SUPPORTED_RETURN_NULL(MSG_TELEPHONY_SMS_FEATURE);
	if (list_handle == NULL)
		return NULL;

	return (msg_struct_t)g_list_nth_data((GList *)list_handle, (guint)index);
}

EXPORT_API int msg_list_length(msg_list_handle_t list_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	if (list_handle == NULL) {
		return 0;
	}

	return (int)g_list_length((GList *)list_handle);
}

EXPORT_API int msg_list_clear(msg_struct_t msg_struct_handle, int field)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	if (msg_struct_handle == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_error_t err = MSG_SUCCESS;
	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;

	switch (msg_struct->type) {
	case MSG_STRUCT_MESSAGE_INFO: {
		err = msg_message_list_clear(msg_struct_handle, field);
	}
	break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

EXPORT_API int msg_list_free(msg_list_handle_t list_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	if (list_handle == NULL)
		return MSG_ERR_NULL_POINTER;

	g_list_free_full((GList *)list_handle, __msg_release_list_item);
	list_handle = NULL;

	return MSG_SUCCESS;
}


EXPORT_API int msg_util_calculate_text_length(const char* msg_text, msg_encode_type_t msg_encode_type_to, unsigned int *text_size, unsigned int *segment_size, msg_encode_type_t *msg_encode_type_in)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (msg_text == NULL || text_size == NULL || segment_size == NULL || msg_encode_type_in == NULL)
		return MSG_ERR_INVALID_PARAMETER;

#ifdef FEATURE_SMS_CDMA
	if (msg_encode_type_to > MSG_ENCODE_ASCII7BIT) {
#else
	if (msg_encode_type_to > MSG_ENCODE_GSM7BIT_ABNORMAL) {
#endif
		MSG_FATAL("unsupported msg_encode_type [%d]", msg_encode_type_to);
		return MSG_ERR_INVALID_PARAMETER;
	}

	msg_encode_type_t encodeType = MSG_ENCODE_AUTO;
	MSG_LANGUAGE_ID_T langId = MSG_LANG_ID_RESERVED;

	int decodeLen = 0;
	int bufSize = 0;
	int textSize = 0;

	bool bAbnormal = false;

	textSize = strlen(msg_text);

	bufSize = textSize * 4;

	unsigned char decodeData[bufSize+1];
	memset(decodeData, 0x00, sizeof(decodeData));

	MsgTextConvert *textCvt = MsgTextConvert::instance();

	*text_size = 0;
	*segment_size = 0;

	switch (msg_encode_type_to) {
	case MSG_ENCODE_GSM7BIT:
		decodeLen = textCvt->convertUTF8ToGSM7bit(decodeData, bufSize, (const unsigned char*)msg_text, textSize, &langId, &bAbnormal);
		break;
	case MSG_ENCODE_UCS2:
		decodeLen = textCvt->convertUTF8ToUCS2(decodeData, bufSize, (const unsigned char*)msg_text, textSize);
		break;
	case MSG_ENCODE_AUTO:
		decodeLen = textCvt->convertUTF8ToAuto(decodeData, bufSize, (const unsigned char*)msg_text, textSize, &encodeType);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	/* calculate segment size. */
	int headerLen = 0;
	int concat = 5;
	int lang = 3;

	int headerSize = 0;
	int segSize = 0;

	if (langId != MSG_LANG_ID_RESERVED) {
		MSG_DEBUG("National Language Exists");
		headerSize += lang;
		headerLen = 1;
	}

	if (msg_encode_type_to == MSG_ENCODE_GSM7BIT || encodeType == MSG_ENCODE_GSM7BIT) {
		MSG_DEBUG("MSG_ENCODE_GSM7BIT");

		if ((decodeLen + headerSize) > 160) {
			headerLen = 1;
			segSize = ((140 - (headerLen + concat + headerSize)) * 8)/7;
		}
		else {
			segSize = ((140 - headerLen - headerSize) * 8) / 7;
		}

		if (bAbnormal)
			*msg_encode_type_in = MSG_ENCODE_GSM7BIT_ABNORMAL;
		else
			*msg_encode_type_in = MSG_ENCODE_GSM7BIT;

	} else if (msg_encode_type_to == MSG_ENCODE_UCS2 || encodeType == MSG_ENCODE_UCS2) {
		MSG_DEBUG("MSG_ENCODE_UCS2");

		if (decodeLen > 140) {
			headerLen = 1;
			segSize = 140 - (headerLen + concat);
		} else {
			segSize = 140;
		}

		*msg_encode_type_in = MSG_ENCODE_UCS2;
#ifdef FEATURE_SMS_CDMA
	} else if (encodeType == MSG_ENCODE_ASCII7BIT) {
		MSG_DEBUG("MSG_ENCODE_ASCII7BIT");

		if(decodeLen > 160)
			segSize = ((140*8) - ((headerLen + concat)*8)) / 7;
		else
			segSize = 160;

		*msg_encode_type_in = MSG_ENCODE_GSM7BIT;
#endif

	} else {
		MSG_DEBUG("Unsupported encode type.");
		err = MSG_ERR_INVALID_PARAMETER;
		return err;
	}

	MSG_DEBUG("decodeLen [%d] segSize [%d]", decodeLen, segSize);

	*text_size = decodeLen;
	*segment_size = segSize;

	return err;
}
