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

#include <errno.h>

#include "MsgHandle.h"
#include "MsgDebug.h"
#include "MsgException.h"

#include "msg.h"
#include "msg_private.h"
#include "msg_storage.h"


static int msg_get_msg_type(int mainType, int subType);
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_add_message(msg_handle_t handle, msg_struct_t opq_msg, const msg_struct_t send_opt)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	MSG_TYPE_CHECK(pMsgStruct->type, MSG_STRUCT_MESSAGE_INFO);

	msg_struct_s *pStruct = (msg_struct_s *)send_opt;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_SENDOPT);

	try {
		err = pHandle->addMessage((MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data, (MSG_SENDINGOPT_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_syncml_message(msg_handle_t handle, const msg_struct_t syncml_msg)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || syncml_msg == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)syncml_msg;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_SYNCML_INFO);

	try {
		err = pHandle->addSyncMLMessage((MSG_SYNCML_MESSAGE_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_message(msg_handle_t handle, const msg_struct_t opq_msg, const msg_struct_t send_opt)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	MSG_TYPE_CHECK(pMsgStruct->type, MSG_STRUCT_MESSAGE_INFO);

	msg_struct_s* pStruct = (msg_struct_s *)send_opt;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_SENDOPT);

	MSG_MESSAGE_HIDDEN_S *msg = (MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data;
	MSG_SENDINGOPT_S *sendingOpt = (MSG_SENDINGOPT_S *)pStruct->data;

	try {
		err = pHandle->updateMessage(msg, sendingOpt);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_read_status(msg_handle_t handle, msg_message_id_t msg_id, bool read)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->updateReadStatus(msg_id, read);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_set_conversation_to_read(msg_handle_t handle, msg_thread_id_t thread_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->setConversationToRead(thread_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_protected_status(msg_handle_t handle, msg_message_id_t msg_id, bool is_protected)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->updateProtectedStatus(msg_id, is_protected);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_delete_message(msg_handle_t handle, msg_message_id_t msg_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteMessage(msg_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_all_msgs_in_folder(msg_handle_t handle, msg_folder_id_t folder_id, bool bOnlyDB)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteAllMessagesInFolder(folder_id, bOnlyDB);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_msgs_by_list(msg_handle_t handle, msg_id_list_s *msg_id_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_id_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteMessagesByList(msg_id_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_move_msg_to_folder(msg_handle_t handle, msg_message_id_t msg_id, msg_folder_id_t dest_folder_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->moveMessageToFolder(msg_id, dest_folder_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_move_msg_to_storage(msg_handle_t handle, msg_message_id_t msg_id, msg_storage_id_t storage_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	if (storage_id < MSG_STORAGE_PHONE || storage_id > MSG_STORAGE_SIM2) {
		MSG_FATAL("unsupported storage [%d]", storage_id);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->moveMessageToStorage(msg_id, storage_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_message(msg_handle_t handle, msg_folder_id_t folder_id, msg_struct_t count_info)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || count_info == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)count_info;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_COUNT_INFO);

	try {
		err = pHandle->countMessage(folder_id, (MSG_COUNT_INFO_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_msg_by_type(msg_handle_t handle, msg_message_type_t msg_type, int *msg_count)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_count == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	MSG_MESSAGE_TYPE_S msgType = {0};

	if (msg_type == MSG_TYPE_SMS) {
		msgType.mainType = MSG_SMS_TYPE;
		msgType.subType = MSG_NORMAL_SMS;
	} else if (msg_type == MSG_TYPE_SMS_WAPPUSH) {
		msgType.mainType = MSG_SMS_TYPE;
		msgType.subType = MSG_WAP_SI_SMS;
	} else if (msg_type == MSG_TYPE_MMS) {
		msgType.mainType = MSG_MMS_TYPE;
		msgType.subType = MSG_SENDREQ_MMS;
	} else {
		return MSG_ERR_INVALID_PARAMETER;
	}

	try {
		err = pHandle->countMsgByType(&msgType, msg_count);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_msg_by_contact(msg_handle_t handle, const msg_struct_t addr_info, msg_struct_t msg_thread_count_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || addr_info == NULL || msg_thread_count_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pAddr = (msg_struct_s *)addr_info;
	MSG_TYPE_CHECK(pAddr->type, MSG_STRUCT_THREAD_LIST_INDEX);

	msg_struct_s *pCount = (msg_struct_s *)msg_thread_count_list;
	MSG_TYPE_CHECK(pCount->type, MSG_STRUCT_THREAD_COUNT_INFO);

	try {
		err = pHandle->countMsgByContact((MSG_THREAD_LIST_INDEX_INFO_S *)pAddr->data, (MSG_THREAD_COUNT_INFO_S *)pCount->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_message(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t opq_msg, msg_struct_t send_opt)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	MSG_TYPE_CHECK(pMsgStruct->type, MSG_STRUCT_MESSAGE_INFO);

	msg_struct_s *pStruct = (msg_struct_s *)send_opt;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_SENDOPT);

	try {
		err = pHandle->getMessage(msg_id, (MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data, (MSG_SENDINGOPT_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_vobject_data(msg_handle_t handle, msg_message_id_t msg_id, void** result_data)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || result_data == NULL) {
		MSG_FATAL("handle or result_data is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getVobject(msg_id, result_data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_conversation(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t conv)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || conv == NULL) {
		MSG_FATAL("handle or opq_msg is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)conv;
	MSG_TYPE_CHECK(pMsgStruct->type, MSG_STRUCT_CONV_INFO);

	try {
		err = pHandle->getConversationViewItem(msg_id, (MSG_CONVERSATION_VIEW_S *)pMsgStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_thread_view_list(msg_handle_t handle, const msg_struct_t sort_rule, msg_struct_list_s *msg_thread_view_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_thread_view_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct =(msg_struct_s *)sort_rule;
	if (pStruct)
		MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_SORT_RULE);

	try {
		if (pStruct == NULL) {
			MSG_SORT_RULE_S sortRule = {0};

			sortRule.sortType = MSG_SORT_BY_THREAD_DATE;
			sortRule.bAscending = false;

			err = pHandle->getThreadViewList(&sortRule, msg_thread_view_list);
		} else {
			err = pHandle->getThreadViewList((MSG_SORT_RULE_S *)pStruct->data, msg_thread_view_list);
		}
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_conversation_view_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_conv_view_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_conv_view_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getConversationViewList(thread_id, msg_conv_view_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_thread_message_list(msg_handle_t handle, msg_thread_id_t thread_id, bool include_protected_msg)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteThreadMessageList(thread_id, include_protected_msg);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_folder(msg_handle_t handle, const msg_struct_t folder_info)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)folder_info;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_FOLDER_INFO);

	try {
		err = pHandle->addFolder((MSG_FOLDER_INFO_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_folder(msg_handle_t handle, const msg_struct_t folder_info)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)folder_info;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_FOLDER_INFO);

	try {
		err = pHandle->updateFolder((MSG_FOLDER_INFO_S *)pStruct->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_folder(msg_handle_t handle, msg_folder_id_t folder_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->deleteFolder(folder_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_folder_list(msg_handle_t handle, msg_struct_list_s *folder_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || folder_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getFolderList(folder_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_generate_message(msg_handle_t handle, msg_message_type_t msg_type, msg_folder_id_t folder_id, unsigned int num_msg)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);

	if (handle == NULL) {
		MSG_DEBUG("Handle is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (msg_type >= MSG_MESSAGE_TYPE_MAX) {
		MSG_DEBUG("msg_type is invalid [%d]", msg_type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	if ((unsigned char)folder_id >= (unsigned char)MSG_MAX_FOLDER_ID) {
		MSG_DEBUG("folderId is invalid [%d]", folder_id);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_DEBUG("type : %d, folder : %d, num_msg : %d", msg_type, folder_id, num_msg);

	int err = 0;
	MSG_SENDINGOPT_S sendingOpt = {0};
	sendingOpt.bSetting = false;

	char strMsg[20] = {0};
	char prefix[10] ="0103001";
/*	int postfix = 8111; */
	int postfix = 0;

	srand(getpid());

	msg_struct_s *msg_s = NULL;
	msg_struct_s *addr_s = NULL;
	MSG_MESSAGE_HIDDEN_S *msgInfo = NULL;
	MSG_ADDRESS_INFO_S *addrInfo = NULL;

	for (unsigned int i = 0; i < num_msg; i++) {
		msg_s = (msg_struct_s *)msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
		if (msg_s == NULL)
			return MSG_ERR_NOT_SUPPORTED;

		msgInfo = (MSG_MESSAGE_HIDDEN_S *)msg_s->data;

		msgInfo->folderId = folder_id;

		if (msg_type == MSG_TYPE_MMS) {
			msgInfo->mainType = MSG_MMS_TYPE;
			msgInfo->subType = MSG_SENDREQ_MMS;
		} else {
			msgInfo->mainType = MSG_SMS_TYPE;
			msgInfo->subType = MSG_NORMAL_SMS;

			snprintf(strMsg, sizeof(strMsg), "test msg %d", i);
			msgInfo->dataSize = strlen(strMsg);
			msgInfo->pData = (void*)new char[msgInfo->dataSize+1];
			memcpy((char *)msgInfo->pData, strMsg, msgInfo->dataSize);
			((char*) msgInfo->pData)[msgInfo->dataSize] = '\0';
		}

		msgInfo->storageId = MSG_STORAGE_PHONE;

		msgInfo->addr_list->nCount = 1;

		addr_s = (msg_struct_s *)msgInfo->addr_list->msg_struct_info[0];

		addrInfo = (MSG_ADDRESS_INFO_S *)addr_s->data;

		addrInfo->addressType = MSG_ADDRESS_TYPE_PLMN;
		postfix = random()%10000;
		snprintf(addrInfo->addressVal, MAX_ADDRESS_VAL_LEN+1, "%s%04d", prefix, postfix);

		addrInfo->recipientType = MSG_RECIPIENTS_TYPE_TO;

		time(&(msgInfo->displayTime));

		msgInfo->networkStatus = MSG_NETWORK_NOT_SEND;
		msgInfo->bRead = false;
		msgInfo->bProtected = false;
		msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;

		if (folder_id == MSG_OUTBOX_ID || folder_id == MSG_SENTBOX_ID)
			msgInfo->direction = MSG_DIRECTION_TYPE_MO;
		else
			msgInfo->direction = MSG_DIRECTION_TYPE_MT;

		if (msg_type == MSG_TYPE_MMS) {
			snprintf(msgInfo->subject, MAX_SUBJECT_LEN+1, "subject %d", i);

			if (folder_id == MSG_INBOX_ID)
				msgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;

			msg_struct_t mms_data = msg_create_struct(MSG_STRUCT_MMS);

			msg_set_mms_struct((msg_struct_t)msg_s, mms_data);

			msg_release_struct(&mms_data);
		}

		/*err = msg_add_message(handle, (msg_message_t)&msgInfo, &sendingOpt);*/
		try {
			MsgHandle* pHandle = (MsgHandle*)handle;
			err = pHandle->addMessage(msgInfo, &sendingOpt);
		} catch (MsgException& e) {
			MSG_FATAL("%s", e.what());
			msg_release_struct((msg_struct_t *)&msg_s);
			return MSG_ERR_STORAGE_ERROR;
		}

		msg_release_struct((msg_struct_t *)&msg_s);

		if (err < 0) {
			MSG_DEBUG("err [%d]", err);
			return err;
		}
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_quick_panel_data(msg_handle_t handle, msg_quickpanel_type_t type, msg_struct_t opq_msg)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL) {
		MSG_FATAL("handle or opq_msg is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (type > MSG_QUICKPANEL_MMS_NOTI) {
		MSG_FATAL("unsupported quickpanel type [%d]", type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pMsg = (msg_struct_s *)opq_msg;
	MSG_TYPE_CHECK(pMsg->type, MSG_STRUCT_MESSAGE_INFO);

	try {
		err = pHandle->getQuickPanelData(type, (MSG_MESSAGE_HIDDEN_S *)pMsg->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_reset_database(msg_handle_t handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->resetDatabase();
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_mem_size(msg_handle_t handle, unsigned int* memsize)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || memsize == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getMemSize(memsize);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_backup_message(msg_handle_t handle, msg_message_backup_type_t type, const char *backup_filepath)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || backup_filepath == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->backupMessage(type, backup_filepath);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_restore_message(msg_handle_t handle, const char *backup_filepath)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || backup_filepath == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->restoreMessage(backup_filepath);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_search_message_for_thread_view(msg_handle_t handle, const char *search_string, msg_struct_list_s *msg_thread_view_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || search_string == NULL || msg_thread_view_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	if (strlen(search_string) <= 0 || strlen(search_string) > MAX_MSG_TEXT_LEN)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->searchMessage(search_string, msg_thread_view_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_reject_msg_list(msg_handle_t handle, const char *phone_num, msg_struct_list_s *msg_reject_msg_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_reject_msg_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getRejectMsgList(phone_num, msg_reject_msg_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_storage_change_callback(msg_handle_t handle, msg_storage_change_cb cb, void *user_param)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->regStorageChangeCallback(cb, user_param);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		if (e.errorCode() == MsgException::SERVER_READY_ERROR)
			return MSG_ERR_PERMISSION_DENIED;
		else
			return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_report_status(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_list_s *report_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_id < 1 || report_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getReportStatus(msg_id, report_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_address_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_address_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_address_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getAddressList(thread_id, msg_address_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread_id_by_address(msg_handle_t handle, msg_struct_list_s *msg_address_list, msg_thread_id_t *thread_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_address_list == NULL || thread_id == NULL) {
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (msg_address_list->nCount < 1 )
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getThreadIdByAddress(msg_address_list, thread_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread_id_by_address2(msg_handle_t handle, msg_list_handle_t msg_address_list, msg_thread_id_t *thread_id)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_address_list == NULL || thread_id == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getThreadIdByAddress(msg_address_list, thread_id);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_t msg_thread)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_thread == NULL) {
		MSG_FATAL("handle or msg_thread is NULL");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *msgThread = (msg_struct_s *)msg_thread;
	MSG_TYPE_CHECK(msgThread->type, MSG_STRUCT_THREAD_INFO);

	MSG_THREAD_VIEW_S* pThreadInfo = (MSG_THREAD_VIEW_S *)msgThread->data;

	try {
		err = pHandle->getThread(thread_id, pThreadInfo);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_message_list2(msg_handle_t handle, const msg_struct_t msg_list_conditions, msg_struct_list_s *msg_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_list_conditions == NULL || msg_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)msg_list_conditions;
	MSG_TYPE_CHECK(pStruct->type, MSG_STRUCT_MSG_LIST_CONDITION);

	try {
		err = pHandle->getMessageList((MSG_LIST_CONDITION_S *)pStruct->data, msg_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_media_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_list_handle_t *msg_list)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || msg_list == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	try {
		err = pHandle->getMediaList(thread_id, msg_list);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}


	return err;
}

static int msg_get_msg_type(int mainType, int subType)
{
	if (mainType == MSG_SMS_TYPE) {
		switch (subType) {
		case MSG_CB_SMS:
			return MSG_TYPE_SMS_CB;
		case MSG_JAVACB_SMS:
			return MSG_TYPE_SMS_JAVACB;
		case MSG_WAP_SI_SMS:
		case MSG_WAP_SL_SMS:
			return MSG_TYPE_SMS_WAPPUSH;
		case MSG_MWI_VOICE_SMS:
		case MSG_MWI_FAX_SMS:
		case MSG_MWI_EMAIL_SMS:
		case MSG_MWI_OTHER_SMS:
			return MSG_TYPE_SMS_MWI;
		case MSG_SYNCML_CP:
			return MSG_TYPE_SMS_SYNCML;
		case MSG_REJECT_SMS:
			return MSG_TYPE_SMS_REJECT;
		case MSG_ETWS_SMS:
			return MSG_TYPE_SMS_ETWS_PRIMARY;
		case MSG_CMAS_PRESIDENTIAL:
			return MSG_TYPE_SMS_CMAS_PRESIDENTIAL;
		case MSG_CMAS_EXTREME:
			return MSG_TYPE_SMS_CMAS_EXTREME;
		case MSG_CMAS_SEVERE:
			return MSG_TYPE_SMS_CMAS_SEVERE;
		case MSG_CMAS_AMBER:
			return MSG_TYPE_SMS_CMAS_AMBER;
		case MSG_CMAS_TEST:
			return MSG_TYPE_SMS_CMAS_TEST;
		case MSG_CMAS_EXERCISE:
			return MSG_TYPE_SMS_CMAS_EXERCISE;
		case MSG_CMAS_OPERATOR_DEFINED:
			return MSG_TYPE_SMS_CMAS_OPERATOR_DEFINED;
		default:
			return MSG_TYPE_SMS;
		}
	} else if (mainType == MSG_MMS_TYPE) {
		if (subType == MSG_NOTIFICATIONIND_MMS)
			return MSG_TYPE_MMS_NOTI;
		else if (subType == MSG_SENDREQ_JAVA_MMS)
			return MSG_TYPE_MMS_JAVA;
		else
			return MSG_TYPE_MMS;
	} else {
		return MSG_TYPE_INVALID;
	}
}


int msg_syncml_info_get_int(void *syncml_info, int field, int *value)
{
	if (!syncml_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)syncml_info;

	switch (field) {
	case MSG_SYNCML_INFO_EXTID_INT:
		*value = pSync->extId;
		break;
	case MSG_SYNCML_INFO_PINCODE_INT:
		*value = pSync->pinCode;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_count_info_get_int(void *count_info, int field, int *value)
{
	if (!count_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_COUNT_INFO_S *pCount = (MSG_COUNT_INFO_S *)count_info;

	switch (field) {
	case MSG_COUNT_INFO_READ_INT:
		*value = pCount->nReadCnt;
		break;
	case MSG_COUNT_INFO_UNREAD_INT:
		*value = pCount->nUnreadCnt;
		break;
	case MSG_COUNT_INFO_SMS_INT:
		*value = pCount->nSms;
		break;
	case MSG_COUNT_INFO_MMS_INT:
		*value = pCount->nMms;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_thread_count_get_int(void *count_info, int field, int *value)
{
	if (!count_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_THREAD_COUNT_INFO_S *pCount = (MSG_THREAD_COUNT_INFO_S *)count_info;

	switch (field) {
	case MSG_THREAD_COUNT_TOTAL_INT:
		*value = pCount->totalCount;
		break;
	case MSG_THREAD_COUNT_UNREAD_INT:
		*value = pCount->unReadCount;
		break;
	case MSG_THREAD_COUNT_SMS_INT:
		*value = pCount->smsMsgCount;
		break;
	case MSG_THREAD_COUNT_MMS_INT:
		*value = pCount->mmsMsgCount;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_thread_index_get_int(void *index_info, int field, int *value)
{
	if (!index_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_THREAD_LIST_INDEX_S *pIndex = (MSG_THREAD_LIST_INDEX_S *)index_info;
	switch (field) {
	case MSG_THREAD_LIST_INDEX_CONTACTID_INT:
		*value = pIndex->contactId;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_sortrule_get_int(void *sort_info, int field, int *value)
{
	if (!sort_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_info;

	switch (field) {
	case MSG_SORT_RULE_SORT_TYPE_INT:
		*value = pSort->sortType;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_folder_info_get_int(void *folder_info, int field, int *value)
{
	if (!folder_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;

	switch (field) {
	case MSG_FOLDER_INFO_ID_INT:
		*value = pFolder->folderId;
		break;
	case MSG_FOLDER_INFO_TYPE_INT:
		*value = pFolder->folderType;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_thread_info_get_int(void *data, int field, int *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_THREAD_VIEW_S *pThread = (MSG_THREAD_VIEW_S *)data;

	switch (field) {
	case MSG_THREAD_ID_INT:
		*value = pThread->threadId;
		break;
	case MSG_THREAD_MSG_TYPE_INT:
		*value = msg_get_msg_type(pThread->mainType, pThread->subType);
		break;
	case MSG_THREAD_MSG_TIME_INT:
		*value = pThread->threadTime;
		break;
	case MSG_THREAD_DIRECTION_INT:
		*value = pThread->direction;
		break;
	case MSG_THREAD_UNREAD_COUNT_INT:
		*value = pThread->unreadCnt;
		break;
	case MSG_THREAD_SMS_COUNT_INT:
		*value = pThread->smsCnt;
		break;
	case MSG_THREAD_MMS_COUNT_INT:
		*value = pThread->mmsCnt;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_conv_info_get_int(void *data, int field, int *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;

	switch (field) {
	case MSG_CONV_MSG_ID_INT:
		*value = pConv->msgId;
		break;
	case MSG_CONV_MSG_THREAD_ID_INT:
		*value = pConv->threadId;
		break;
	case MSG_CONV_MSG_FOLDER_ID_INT:
		*value = pConv->folderId;
		break;
	case MSG_CONV_MSG_TYPE_INT:
		*value = msg_get_msg_type(pConv->mainType, pConv->subType);
		break;
	case MSG_CONV_MSG_STORAGE_ID_INT:
		*value = pConv->storageId;
		break;
	case MSG_CONV_MSG_DISPLAY_TIME_INT:
		*value = pConv->displayTime;
		break;
	case MSG_CONV_MSG_SCHEDULED_TIME_INT:
		*value = pConv->scheduledTime;
		break;
	case MSG_CONV_MSG_NETWORK_STATUS_INT:
		*value = pConv->networkStatus;
		break;
	case MSG_CONV_MSG_DIRECTION_INT:
		*value = pConv->direction;
		break;
	case MSG_CONV_MSG_ATTACH_COUNT_INT:
		*value = pConv->attachCount;
		break;
	case MSG_CONV_MSG_TEXT_SIZE_INT:
		*value = pConv->textSize;
		break;
	case MSG_CONV_MSG_PAGE_COUNT_INT:
		*value = pConv->pageCount;
		break;
	case MSG_CONV_MSG_TCS_BC_LEVEL_INT:
		*value = pConv->tcs_bc_level;
		break;
	case MSG_CONV_MSG_SIM_INDEX_INT:
		*value = pConv->simIndex;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_list_condition_get_int(void *condition_info, int field, int *value)
{
	if (!condition_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)condition_info;

	switch (field) {
	case MSG_LIST_CONDITION_FOLDER_ID_INT:
		*value = pCond->folderId;
		break;
	case MSG_LIST_CONDITION_THREAD_ID_INT:
		*value = pCond->threadId;
		break;
	case MSG_LIST_CONDITION_STORAGE_ID_INT:
		*value = pCond->storageId;
		break;
	case MSG_LIST_CONDITION_MSGTYPE_INT:
		*value = pCond->msgType;
		break;
	case MSG_LIST_CONDITION_FROM_TIME_INT:
		*value = pCond->fromTime;
		break;
	case MSG_LIST_CONDITION_TO_TIME_INT:
		*value = pCond->toTime;
		break;
	case MSG_LIST_CONDITION_OFFSET_INT:
		*value = pCond->offset;
		break;
	case MSG_LIST_CONDITION_LIMIT_INT:
		*value = pCond->limit;
		break;
	case MSG_LIST_CONDITION_SIM_INDEX_INT:
		*value = pCond->simIndex;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_report_status_get_int(void *report_info, int field, int *value)
{
	if (!report_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;
	switch (field) {
	case MSG_REPORT_TYPE_INT:
		*value = pReport->type;
		break;
	case MSG_REPORT_STATUS_INT:
		*value = pReport->status;
		break;
	case MSG_REPORT_TIME_INT:
		*value = pReport->statusTime;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_report_status_get_str(void *report_info, int field, char *value, int size)
{
	if (!report_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;

	switch (field) {
	case MSG_REPORT_ADDRESS_STR:
		strncpy(value, pReport->addressVal, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}
	return MSG_SUCCESS;
}

int msg_folder_info_get_str(void *folder_info, int field, char *value, int size)
{
	if (!folder_info)
		return MSG_ERR_NULL_POINTER;

	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;

	switch (field) {
	case MSG_FOLDER_INFO_NAME_STR:
		strncpy(value, pFolder->folderName, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_thread_info_get_str(void *data, int field, char *value, int size)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_VIEW_S *pThread = (MSG_THREAD_VIEW_S *)data;

	switch (field) {
	case MSG_THREAD_NAME_STR:
		strncpy(value, pThread->threadName, size);
		break;
	case MSG_THREAD_MSG_DATA_STR:
		strncpy(value, pThread->threadData, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}


int msg_conv_info_get_str(void *data, int field, char *value, int size)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;

	switch (field) {
	case MSG_CONV_MSG_SUBJECT_STR:
		strncpy(value, pConv->subject, size);
		break;
	case MSG_CONV_MSG_ATTACH_NAME_STR:
		strncpy(value, pConv->attachFileName, size);
		break;
	case MSG_CONV_MSG_AUDIO_NAME_STR:
		strncpy(value, pConv->audioFileName, size);
		break;
	case MSG_CONV_MSG_IMAGE_THUMB_PATH_STR:
		strncpy(value, pConv->imageThumbPath, size);
		break;
	case MSG_CONV_MSG_VIDEO_THUMB_PATH_STR:
		strncpy(value, pConv->videoThumbPath, size);
		break;
	case MSG_CONV_MSG_TEXT_STR:
		if (pConv->pText)
			strncpy(value, pConv->pText, size);
		break;
	case MSG_CONV_MSG_1ST_MEDIA_PATH_STR:
		strncpy(value, pConv->firstMediaPath, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}


int msg_list_condition_get_str(void *condition_info, int field, char *value, int size)
{
	if (!condition_info)
		return MSG_ERR_NULL_POINTER;

	MSG_LIST_CONDITION_S *cond = (MSG_LIST_CONDITION_S *)condition_info;

	switch (field) {
	case MSG_LIST_CONDITION_ADDRESS_VALUE_STR:
		if (cond->pAddressVal)
			strncpy(value, cond->pAddressVal, size);
		break;
	case MSG_LIST_CONDITION_TEXT_VALUE_STR:
		if (cond->pTextVal)
			strncpy(value, cond->pTextVal, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}


int msg_sendopt_get_bool(void *send_opt, int field, bool *value)
{
	if (!send_opt)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)send_opt;

	switch (field) {
	case MSG_SEND_OPT_SETTING_BOOL:
		*value = sendopt->bSetting;
		break;
	case MSG_SEND_OPT_KEEPCOPY_BOOL:
		*value = sendopt->bKeepCopy;
		break;
	case MSG_SEND_OPT_DELIVER_REQ_BOOL:
		*value = sendopt->bDeliverReq;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_sortrule_get_bool(void *sort_rule, int field, bool *value)
{
	if (!sort_rule)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_rule;

	switch (field) {
	case MSG_SORT_RULE_ACSCEND_BOOL:
		*value = pSort->bAscending;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_conv_get_bool(void *data, int field, bool *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;

	switch (field) {
	case MSG_CONV_MSG_READ_BOOL:
		*value = pConv->bRead;
		break;
	case MSG_CONV_MSG_PROTECTED_BOOL:
		*value = pConv->bProtected;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_thread_info_get_bool(void *data, int field, bool *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_THREAD_VIEW_S *pthreadInfo = (MSG_THREAD_VIEW_S *)data;

	switch (field) {
	case MSG_THREAD_PROTECTED_BOOL:
		*value = pthreadInfo->bProtected;
		break;
	case MSG_THREAD_DRAFT_BOOL:
		*value = pthreadInfo->bDraft;
		break;
	case MSG_THREAD_SEND_FAILED_BOOL:
		*value = pthreadInfo->bSendFailed;
		break;
	case MSG_THREAD_SENDING_BOOL:
		*value = pthreadInfo->bSending;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_list_condition_get_bool(void *data, int field, bool *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)data;

	switch (field) {
	case MSG_LIST_CONDITION_PROTECTED_BOOL:
		*value = pCond->bProtected;
		break;
	case MSG_LIST_CONDITION_SCHEDULED_BOOL:
		*value = pCond->bScheduled;
		break;
	case MSG_LIST_CONDITION_AND_OPERATER_BOOL:
		*value = pCond->bAnd;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}


int msg_sendopt_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)msg_struct->data;

	switch (field) {
	case MSG_SEND_OPT_MMS_OPT_HND:
		*value = (void *)sendopt->mmsSendOpt;
		break;
	case MSG_SEND_OPT_SMS_OPT_HND:
		*value = (void *)sendopt->smsSendOpt;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_syncml_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)msg_struct->data;

	switch (field) {
	case MSG_SYNCML_INFO_MESSAGE_HND:
		*value = (void *)pSync->msg;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_thread_index_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_LIST_INDEX_INFO_S *pIndex = (MSG_THREAD_LIST_INDEX_INFO_S *)msg_struct->data;

	switch (field) {
	case MSG_THREAD_LIST_INDEX_ADDR_INFO_HND:
		*value = (void *)pIndex->msgAddrInfo;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_list_condition_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)msg_struct->data;

	switch (field) {
	case MSG_LIST_CONDITION_SORT_RULE_HND:
		*value = (void *)pCond->sortRule;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_address_info_get_int(void *addr_info, int field, int *value)
{
	if (!addr_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;

	switch (field) {
	case MSG_ADDRESS_INFO_ADDRESS_TYPE_INT:
		*value = pAddr->addressType;
		break;
	case MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT:
		*value = pAddr->recipientType;
		break;
	case MSG_ADDRESS_INFO_CONTACT_ID_INT:
		*value = pAddr->contactId;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_mms_sendopt_get_int(void *opt_info, int field, int *value)
{
	if (!opt_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;

	switch (field) {
	case MSG_MMS_SENDOPTION_EXPIRY_TIME_INT:
		*value = pOpt->expiryTime;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_TIME_INT:
		*value = pOpt->deliveryTime;
		break;
	case MSG_MMS_SENDOPTION_PRIORITY_INT:
		*value = pOpt->priority;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_reject_message_get_int(void *msg_info, int field, int *value)
{
	if (!msg_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;

	switch (field) {
	case MSG_REJECT_MESSAGE_MSGID_INT:
		*value = pMsg->msgId;
		break;
	case MSG_REJECT_MESSAGE_DISPLAY_TIME_INT:
		*value = pMsg->displayTime;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_address_info_get_str(void *addr_info, int field, char *value, int size)
{
	if (!addr_info)
		return MSG_ERR_NULL_POINTER;

	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;

	switch (field) {
	case MSG_ADDRESS_INFO_ADDRESS_VALUE_STR:
		strncpy(value, pAddr->addressVal, size);
		break;
	case MSG_ADDRESS_INFO_DISPLAYNAME_STR:
		strncpy(value, pAddr->displayName, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_reject_message_get_str(void *msg_info, int field, char *value, int size)
{
	if (!msg_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;

	switch (field) {
	case MSG_REJECT_MESSAGE_MSGTEXT_STR:
		strncpy(value, pMsg->msgText, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_mms_sendopt_get_bool(void *opt_info, int field, bool *value)
{
	if (!opt_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;

	switch (field) {
	case MSG_MMS_SENDOPTION_READ_REQUEST_BOOL:
		*value = pOpt->bReadReq;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_CUSTOMTIME_BOOL:
		*value = pOpt->bUseDeliveryCustomTime;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_sms_sendopt_get_bool(void *opt_info, int field, bool *value)
{
	if (!opt_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	SMS_SENDINGOPT_S *pOpt = (SMS_SENDINGOPT_S *)opt_info;

	switch (field) {
	case MSG_SMS_SENDOPT_REPLYPATH_BOOL:
		*value = pOpt->bReplyPath;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_syncml_info_set_int(void *syncml_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!syncml_info)
		return MSG_ERR_NULL_POINTER;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)syncml_info;

	switch (field) {
	case MSG_SYNCML_INFO_EXTID_INT:
		pSync->extId = value;
		break;
	case MSG_SYNCML_INFO_PINCODE_INT:
		pSync->pinCode = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_count_info_set_int(void *count_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!count_info)
		return MSG_ERR_NULL_POINTER;

	MSG_COUNT_INFO_S *pCount = (MSG_COUNT_INFO_S *)count_info;

	switch (field) {
	case MSG_COUNT_INFO_READ_INT:
		pCount->nReadCnt = value;
		break;
	case MSG_COUNT_INFO_UNREAD_INT:
		pCount->nUnreadCnt = value;
		break;
	case MSG_COUNT_INFO_SMS_INT:
		pCount->nSms = value;
		break;
	case MSG_COUNT_INFO_MMS_INT:
		pCount->nMms = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_thread_count_set_int(void *count_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!count_info)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_COUNT_INFO_S *pCount = (MSG_THREAD_COUNT_INFO_S *)count_info;

	switch (field) {
	case MSG_THREAD_COUNT_TOTAL_INT:
		pCount->totalCount = value;
		break;
	case MSG_THREAD_COUNT_UNREAD_INT:
		pCount->unReadCount = value;
		break;
	case MSG_THREAD_COUNT_SMS_INT:
		pCount->smsMsgCount = value;
		break;
	case MSG_THREAD_COUNT_MMS_INT:
		pCount->mmsMsgCount = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_thread_index_set_int(void *index_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!index_info)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_LIST_INDEX_S *pIndex = (MSG_THREAD_LIST_INDEX_S *)index_info;

	switch (field) {
	case MSG_THREAD_LIST_INDEX_CONTACTID_INT:
		pIndex->contactId = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_sortrule_set_int(void *sort_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!sort_info)
		return MSG_ERR_NULL_POINTER;

	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_info;

	switch (field) {
	case MSG_SORT_RULE_SORT_TYPE_INT:
		pSort->sortType = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_folder_info_set_int(void *folder_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!folder_info)
		return MSG_ERR_NULL_POINTER;

	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;

	switch (field) {
	case MSG_FOLDER_INFO_ID_INT:
		pFolder->folderId = value;
		break;
	case MSG_FOLDER_INFO_TYPE_INT:
		pFolder->folderType = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_list_condition_set_int(void *condition_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!condition_info)
		return MSG_ERR_NULL_POINTER;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)condition_info;

	switch (field) {
	case MSG_LIST_CONDITION_FOLDER_ID_INT:
		pCond->folderId = value;
		break;
	case MSG_LIST_CONDITION_THREAD_ID_INT:
		pCond->threadId = value;
		break;
	case MSG_LIST_CONDITION_STORAGE_ID_INT:
		pCond->storageId = value;
		break;
	case MSG_LIST_CONDITION_MSGTYPE_INT:
		pCond->msgType = value;
		break;
	case MSG_LIST_CONDITION_FROM_TIME_INT:
		pCond->fromTime = value;
		break;
	case MSG_LIST_CONDITION_TO_TIME_INT:
		pCond->toTime = value;
		break;
	case MSG_LIST_CONDITION_OFFSET_INT:
		pCond->offset = value;
		break;
	case MSG_LIST_CONDITION_LIMIT_INT:
		pCond->limit = value;
		break;
	case MSG_LIST_CONDITION_SIM_INDEX_INT:
		pCond->simIndex = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_report_status_set_int(void *report_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!report_info)
	return MSG_ERR_NULL_POINTER;

	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;

	switch (field) {
	case MSG_REPORT_TYPE_INT:
		pReport->type = value;
		break;
	case MSG_REPORT_STATUS_INT:
		pReport->status = value;
		break;
	case MSG_REPORT_TIME_INT:
		pReport->statusTime = value;
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_folder_info_set_str(void *folder_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!folder_info || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;

	int _len = 0;
	(size > MAX_FOLDER_NAME_SIZE)? _len = MAX_FOLDER_NAME_SIZE : _len = size;

	switch (field) {
	case MSG_FOLDER_INFO_NAME_STR:
		strncpy(pFolder->folderName, value, _len);
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_list_condition_set_str(void *condition_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!condition_info || !value)
		return MSG_ERR_NULL_POINTER;

	if (size <= 0) {
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_LIST_CONDITION_S *cond = (MSG_LIST_CONDITION_S *)condition_info;

	switch (field) {
	case MSG_LIST_CONDITION_ADDRESS_VALUE_STR:
		if (size) {
			if (cond->pAddressVal) {
				delete cond->pAddressVal;
				cond->pAddressVal = NULL;
			}
			cond->pAddressVal = (char *)new char[size+1];
			memset(cond->pAddressVal, 0x00, sizeof(char)*(size+1));
			if (cond->pAddressVal)
				memcpy(cond->pAddressVal, value, sizeof(char)*size);
			else
				return MSG_ERR_MEMORY_ERROR;
		}
		break;
	case MSG_LIST_CONDITION_TEXT_VALUE_STR:
		if (size) {
			if (cond->pTextVal) {
				delete cond->pTextVal;
				cond->pTextVal = NULL;
			}
			cond->pTextVal = (char *)new char[size+1];
			memset(cond->pTextVal, 0x00, sizeof(char)*(size+1));
			if (cond->pTextVal)
				memcpy(cond->pTextVal, value, sizeof(char)*size);
			else
				return MSG_ERR_MEMORY_ERROR;
		}
		break;
	default:
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_sendopt_set_bool(void *send_opt, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!send_opt)
		return MSG_ERR_NULL_POINTER;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)send_opt;

	switch (field) {
	case MSG_SEND_OPT_SETTING_BOOL:
		sendopt->bSetting = value;
		break;
	case MSG_SEND_OPT_KEEPCOPY_BOOL:
		sendopt->bKeepCopy = value;
		break;
	case MSG_SEND_OPT_DELIVER_REQ_BOOL:
		sendopt->bDeliverReq = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_sortrule_set_bool(void *sort_rule, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!sort_rule)
		return MSG_ERR_NULL_POINTER;

	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_rule;

	switch (field) {
	case MSG_SORT_RULE_ACSCEND_BOOL:
		pSort->bAscending = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}


int msg_list_condition_set_bool(void *data, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!data)
		return MSG_ERR_NULL_POINTER;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)data;

	switch (field) {
	case MSG_LIST_CONDITION_PROTECTED_BOOL:
		pCond->bProtected = value;
		break;
	case MSG_LIST_CONDITION_SCHEDULED_BOOL:
		pCond->bScheduled = value;
		break;
	case MSG_LIST_CONDITION_AND_OPERATER_BOOL:
		pCond->bAnd = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}


int msg_sendopt_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch (field) {
	case MSG_SEND_OPT_MMS_OPT_HND:
		pTmp = (msg_struct_s *)sendopt->mmsSendOpt;
		memcpy(pTmp->data, value->data, sizeof(MMS_SENDINGOPT_INFO_S));
		break;
	case MSG_SEND_OPT_SMS_OPT_HND:
		pTmp = (msg_struct_s *)sendopt->smsSendOpt;
		memcpy(pTmp->data, value->data, sizeof(SMS_SENDINGOPT_INFO_S));
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_syncml_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch (field) {
	case MSG_SYNCML_INFO_MESSAGE_HND: {
		pTmp = (msg_struct_s *)pSync->msg;
		MSG_MESSAGE_HIDDEN_S *pSrc = (MSG_MESSAGE_HIDDEN_S *)value->data;
		MSG_MESSAGE_HIDDEN_S *pDst = (MSG_MESSAGE_HIDDEN_S *)pTmp->data;
		msg_message_copy_message(pSrc, pDst);
		break;
	}
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_thread_index_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_LIST_INDEX_INFO_S *pIndex = (MSG_THREAD_LIST_INDEX_INFO_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch (field) {
	case MSG_THREAD_LIST_INDEX_ADDR_INFO_HND:
		pTmp = (msg_struct_s *)pIndex->msgAddrInfo;
		memcpy(pTmp->data, value->data, sizeof(MSG_ADDRESS_INFO_S));
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}


int msg_list_condition_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_LIST_CONDITION_S *pCond = (MSG_LIST_CONDITION_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch (field) {
	case MSG_LIST_CONDITION_SORT_RULE_HND:
		pTmp = (msg_struct_s *)pCond->sortRule;
		memcpy(pTmp->data, value->data, sizeof(MSG_SORT_RULE_S));
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}


int msg_address_info_set_int(void *addrinfo, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!addrinfo)
		return MSG_ERR_NULL_POINTER;

	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addrinfo;

	switch (field) {
	case MSG_ADDRESS_INFO_ADDRESS_TYPE_INT:
		pAddr->addressType = value;
		break;
	case MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT:
		pAddr->recipientType = value;
		break;
	case MSG_ADDRESS_INFO_CONTACT_ID_INT:
		pAddr->contactId = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}


int msg_mms_sendopt_set_int(void *opt_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!opt_info)
		return MSG_ERR_NULL_POINTER;

	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;

	switch (field) {
	case MSG_MMS_SENDOPTION_EXPIRY_TIME_INT:
		pOpt->expiryTime = value;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_TIME_INT:
		pOpt->deliveryTime = value;
		break;
	case MSG_MMS_SENDOPTION_PRIORITY_INT:
		pOpt->priority = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_reject_message_set_int(void *msg_info, int field, int value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;

	switch (field) {
	case MSG_REJECT_MESSAGE_MSGID_INT:
		pMsg->msgId = value;
		break;
	case MSG_REJECT_MESSAGE_DISPLAY_TIME_INT:
		pMsg->displayTime = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_address_info_set_str(void *addr_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!addr_info || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;
	int _len = 0;

	switch (field) {
	case MSG_ADDRESS_INFO_ADDRESS_VALUE_STR:
		(size > MAX_ADDRESS_VAL_LEN)? _len = MAX_ADDRESS_VAL_LEN : _len = size;
		memset(pAddr->addressVal, 0x00, sizeof(pAddr->addressVal));
		strncpy(pAddr->addressVal, value, _len);
		break;
	case MSG_ADDRESS_INFO_DISPLAYNAME_STR:
		(size > MAX_DISPLAY_NAME_LEN)? _len = MAX_DISPLAY_NAME_LEN : _len = size;
		memset(pAddr->displayName, 0x00, sizeof(pAddr->displayName));
		strncpy(pAddr->displayName, value, _len);
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_media_info_set_str(void *media_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!media_info || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_MEDIA_INFO_S *pAddr = (MSG_MEDIA_INFO_S *)media_info;
	int _len = 0;

	switch (field) {
	case MSG_MEDIA_ITEM_STR:
		(size > MSG_FILEPATH_LEN_MAX)? _len = MSG_FILEPATH_LEN_MAX : _len = size;
		memset(pAddr->media_item, 0x00, sizeof(pAddr->media_item));
		strncpy(pAddr->media_item, value, _len);
		break;
	case MSG_MEDIA_MIME_TYPE_STR:
		(size > MAX_MIME_TYPE_LEN)? _len = MAX_MIME_TYPE_LEN : _len = size;
		memset(pAddr->mime_type, 0x00, sizeof(pAddr->mime_type));
		strncpy(pAddr->mime_type, value, _len);
		break;
	case MSG_MEDIA_THUMB_PATH_STR:
		(size > MSG_FILEPATH_LEN_MAX)? _len = MSG_FILEPATH_LEN_MAX : _len = size;
		memset(pAddr->thumb_path, 0x00, sizeof(pAddr->thumb_path));
		strncpy(pAddr->thumb_path, value, _len);
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_reject_message_set_str(void *msg_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!msg_info || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;

	int _len = 0;
	(size > MAX_MSG_TEXT_LEN)? _len = MAX_MSG_TEXT_LEN : _len = size;

	switch (field) {
	case MSG_REJECT_MESSAGE_MSGTEXT_STR:
		strncpy(pMsg->msgText, value, _len);
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_mms_sendopt_set_bool(void *option, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!option)
		return MSG_ERR_NULL_POINTER;

	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)option;

	switch (field) {
	case MSG_MMS_SENDOPTION_READ_REQUEST_BOOL:
		pOpt->bReadReq = value;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_CUSTOMTIME_BOOL:
		pOpt->bUseDeliveryCustomTime = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_sms_sendopt_set_bool(void *option, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!option)
		return MSG_ERR_NULL_POINTER;

	SMS_SENDINGOPT_S *pOpt = (SMS_SENDINGOPT_S *)option;

	switch (field) {
	case MSG_SMS_SENDOPT_REPLYPATH_BOOL:
		pOpt->bReplyPath = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

EXPORT_API int msg_add_push_event(msg_handle_t handle, const msg_struct_t push_event)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || push_event == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pPush = (msg_struct_s *)push_event;
	MSG_TYPE_CHECK(pPush->type, MSG_STRUCT_PUSH_CONFIG_INFO);

	try {
		err = pHandle->addPushEvent((MSG_PUSH_EVENT_INFO_S *)pPush->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_delete_push_event(msg_handle_t handle, const msg_struct_t push_event)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || push_event == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pPush = (msg_struct_s *)push_event;
	MSG_TYPE_CHECK(pPush->type, MSG_STRUCT_PUSH_CONFIG_INFO);

	try {
		err = pHandle->deletePushEvent((MSG_PUSH_EVENT_INFO_S *)pPush->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_update_push_event(msg_handle_t handle, const msg_struct_t src_event, const msg_struct_t dst_event)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_SMS_FEATURE);
	msg_error_t err = MSG_SUCCESS;

	if (handle == NULL || src_event == NULL || dst_event == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pSrc = (msg_struct_s *)src_event;
	MSG_TYPE_CHECK(pSrc->type, MSG_STRUCT_PUSH_CONFIG_INFO);

	msg_struct_s *pDst = (msg_struct_s *)dst_event;
	MSG_TYPE_CHECK(pDst->type, MSG_STRUCT_PUSH_CONFIG_INFO);

	try {
		err = pHandle->updatePushEvent((MSG_PUSH_EVENT_INFO_S *)pSrc->data, (MSG_PUSH_EVENT_INFO_S *)pDst->data);
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

int msg_push_config_get_str(void *event_info, int field, char *value, int size)
{
	if (!event_info)
		return MSG_ERR_NULL_POINTER;

	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;

	switch (field) {
	case MSG_PUSH_CONFIG_CONTENT_TYPE_STR:
		strncpy(value, pEvent->contentType, size);
		break;
	case MSG_PUSH_CONFIG_APPLICATON_ID_STR:
		strncpy(value, pEvent->appId, size);
		break;
	case MSG_PUSH_CONFIG_PACKAGE_NAME_STR:
		strncpy(value, pEvent->pkgName, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_push_config_get_bool(void *event_info, int field, bool *value)
{
	if (!event_info)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;

	switch (field) {
	case MSG_PUSH_CONFIG_LAUNCH_BOOL:
		*value = pEvent->bLaunch;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_push_config_set_str(void *event_info, int field, const char *value, int size)
{
	msg_error_t err = MSG_SUCCESS;

	if (!event_info || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;
	int _len = 0;

	switch (field) {
	case MSG_PUSH_CONFIG_CONTENT_TYPE_STR:
		(size > MAX_WAPPUSH_CONTENT_TYPE_LEN)? _len = MAX_WAPPUSH_CONTENT_TYPE_LEN : _len = size;
		strncpy(pEvent->contentType, value, _len);
		break;
	case MSG_PUSH_CONFIG_APPLICATON_ID_STR:
		(size > MAX_WAPPUSH_ID_LEN)? _len = MAX_WAPPUSH_ID_LEN : _len = size;
		strncpy(pEvent->appId, value, _len);
		break;
	case MSG_PUSH_CONFIG_PACKAGE_NAME_STR:
		(size > MSG_FILEPATH_LEN_MAX)? _len = MSG_FILEPATH_LEN_MAX : _len = size;
		strncpy(pEvent->pkgName, value, _len);
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_push_config_set_bool(void *event, int field, bool value)
{
	msg_error_t err = MSG_SUCCESS;

	if (!event)
		return MSG_ERR_NULL_POINTER;

	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event;

	switch (field) {
	case MSG_PUSH_CONFIG_LAUNCH_BOOL:
		pEvent->bLaunch = value;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_media_item_get_str(void *data, int field, char *value, int size)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	MSG_MEDIA_INFO_S *pMedia = (MSG_MEDIA_INFO_S *)data;

	switch (field) {
	case MSG_MEDIA_ITEM_STR:
		strncpy(value, pMedia->media_item, size);
		break;
	case MSG_MEDIA_MIME_TYPE_STR:
		strncpy(value, pMedia->mime_type, size);
		break;
	case MSG_MEDIA_THUMB_PATH_STR:
		strncpy(value, pMedia->thumb_path, size);
		break;
	default:
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}

int msg_media_item_get_int(void *data, int field, int *value)
{
	if (!data || !value)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MEDIA_INFO_S *pMedia = (MSG_MEDIA_INFO_S *)data;

	switch (field) {
	case MSG_MEDIA_MESSAGE_ID_INT:
		*value = pMedia->msg_id;
		break;
	default:
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}
