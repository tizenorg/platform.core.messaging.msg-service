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

#include "msg.h"
#include "msg_private.h"
#include "msg_storage.h"


static int msg_get_msg_type(int mainType, int subType);
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_add_message(msg_handle_t handle, msg_struct_t opq_msg, const msg_struct_t send_opt)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	msg_struct_s *pStruct = (msg_struct_s *)send_opt;

	try
	{
		err = pHandle->addMessage((MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data, (MSG_SENDINGOPT_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_syncml_message(msg_handle_t handle, const msg_struct_t syncml_msg)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || syncml_msg == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)syncml_msg;

	try
	{
		err = pHandle->addSyncMLMessage((MSG_SYNCML_MESSAGE_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_message(msg_handle_t handle, const msg_struct_t opq_msg, const msg_struct_t send_opt)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	msg_struct_s* pStruct = (msg_struct_s *)send_opt;

	MSG_MESSAGE_HIDDEN_S *msg = (MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data;

	if (msg->addr_list->nCount > 1)
	{
		MSG_DEBUG("Multiple Address cannot be updated [%d]", msg->addr_list->nCount);
		return -EINVAL;
	}

	try
	{
		err = pHandle->updateMessage(msg, (MSG_SENDINGOPT_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_read_status(msg_handle_t handle, msg_message_id_t msg_id, bool read)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->updateReadStatus(msg_id, read);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_protected_status(msg_handle_t handle, msg_message_id_t msg_id, bool is_protected)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->updateProtectedStatus(msg_id, is_protected);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_message(msg_handle_t handle, msg_message_id_t msg_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteMessage(msg_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_all_msgs_in_folder(msg_handle_t handle, msg_folder_id_t folder_id, bool bOnlyDB)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteAllMessagesInFolder(folder_id, bOnlyDB);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_msgs_by_list(msg_handle_t handle, msg_id_list_s *msg_id_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteMessagesByList(msg_id_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_move_msg_to_folder(msg_handle_t handle, msg_message_id_t msg_id, msg_folder_id_t dest_folder_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->moveMessageToFolder(msg_id, dest_folder_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_move_msg_to_storage(msg_handle_t handle, msg_message_id_t msg_id, msg_storage_id_t storage_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	if (storage_id != MSG_STORAGE_PHONE && storage_id != MSG_STORAGE_SIM)
	{
		MSG_FATAL("unsupported storage [%d]", storage_id);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->moveMessageToStorage(msg_id, storage_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_message(msg_handle_t handle, msg_folder_id_t folder_id, msg_struct_t count_info)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)count_info;
	try
	{
		err = pHandle->countMessage(folder_id, (MSG_COUNT_INFO_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_msg_by_type(msg_handle_t handle, msg_message_type_t msg_type, int *msg_count)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	MSG_MESSAGE_TYPE_S msgType = {0};

	if (msg_type == MSG_TYPE_SMS)
	{
		msgType.mainType = MSG_SMS_TYPE;
		msgType.subType = MSG_NORMAL_SMS;
	}
	else if (msg_type == MSG_TYPE_SMS_WAPPUSH)
	{
		msgType.mainType = MSG_SMS_TYPE;
		msgType.subType = MSG_WAP_SI_SMS;
	}
	else if (msg_type == MSG_TYPE_MMS)
	{
		msgType.mainType = MSG_MMS_TYPE;
		msgType.subType = MSG_SENDREQ_MMS;
	}

	try
	{
		err = pHandle->countMsgByType(&msgType, msg_count);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_msg_by_contact(msg_handle_t handle, const msg_struct_t addr_info, msg_struct_t msg_thread_count_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || addr_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pAddr = (msg_struct_s *)addr_info;
	msg_struct_s *pCount = (msg_struct_s *)msg_thread_count_list;

	try
	{
		err = pHandle->countMsgByContact((MSG_THREAD_LIST_INDEX_INFO_S *)pAddr->data, (MSG_THREAD_COUNT_INFO_S *)pCount->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_message(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_t opq_msg, msg_struct_t send_opt)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || !opq_msg)
	{
		MSG_FATAL("handle or opq_msg is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pMsgStruct = (msg_struct_s *)opq_msg;
	msg_struct_s *pStruct = (msg_struct_s *)send_opt;

	try
	{
		err = pHandle->getMessage(msg_id, (MSG_MESSAGE_HIDDEN_S *)pMsgStruct->data, (MSG_SENDINGOPT_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_folder_view_list(msg_handle_t handle, msg_folder_id_t folder_id, const msg_struct_t sort_rule, msg_struct_list_s *msg_folder_view_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)sort_rule;

	try
	{
		if (sort_rule == NULL)
		{
			MSG_SORT_RULE_S sortRule = {0};

			sortRule.sortType = MSG_SORT_BY_READ_STATUS;
			sortRule.bAscending = true;

			err = pHandle->getFolderViewList(folder_id, &sortRule, msg_folder_view_list);
		}
		else
		{
		err = pHandle->getFolderViewList(folder_id, (MSG_SORT_RULE_S *)pStruct->data, msg_folder_view_list);
	}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread_view_list(msg_handle_t handle, const msg_struct_t sort_rule, msg_struct_list_s *msg_thread_view_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct =(msg_struct_s *)sort_rule;

	try
	{
		if (sort_rule == NULL)
		{
			MSG_SORT_RULE_S sortRule = {0};

			sortRule.sortType = MSG_SORT_BY_THREAD_DATE;
			sortRule.bAscending = false;

			err = pHandle->getThreadViewList(&sortRule, msg_thread_view_list);
		}
		else
		{
			err = pHandle->getThreadViewList((MSG_SORT_RULE_S *)pStruct->data, msg_thread_view_list);
		}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_conversation_view_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_conv_view_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getConversationViewList(thread_id, msg_conv_view_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_thread_message_list(msg_handle_t handle, msg_thread_id_t thread_id, bool include_protected_msg)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteThreadMessageList(thread_id, include_protected_msg);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_folder(msg_handle_t handle, const msg_struct_t folder_info)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)folder_info;

	try
	{
		err = pHandle->addFolder((MSG_FOLDER_INFO_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_folder(msg_handle_t handle, const msg_struct_t folder_info)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)folder_info;

	try
	{
		err = pHandle->updateFolder((MSG_FOLDER_INFO_S *)pStruct->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_folder(msg_handle_t handle, msg_folder_id_t folder_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteFolder(folder_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_folder_list(msg_handle_t handle, msg_struct_list_s *folder_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getFolderList(folder_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_generate_message(msg_handle_t handle, msg_message_type_t msg_type, msg_folder_id_t folder_id, unsigned int num_msg)
{
	if (handle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return -EINVAL;
	}

	if (folder_id >= MSG_MAX_FOLDER_ID)
	{
		MSG_DEBUG("folderId is invalid [%d]", folder_id);
		return -EINVAL;
	}

	MSG_DEBUG("type : %d, folder : %d, num_msg : %d", msg_type, folder_id, num_msg);

	int err = 0;
	MSG_SENDINGOPT_S sendingOpt = {0};
	sendingOpt.bSetting = false;

	char strMsg[20] = {0};
	char prefix[10] ="0103001";
//	int postfix = 8111;
	int postfix = 0;

	srand(getpid());

	msg_struct_s *msg_s = NULL;
	msg_struct_s *addr_s = NULL;
	MSG_MESSAGE_HIDDEN_S *msgInfo = NULL;
	MSG_ADDRESS_INFO_S *addrInfo = NULL;

	for (unsigned int i = 0; i < num_msg; i++)
	{
		msg_s = (msg_struct_s *)msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
		msgInfo = (MSG_MESSAGE_HIDDEN_S *)msg_s->data;

		msgInfo->folderId = folder_id;

		if (msg_type == MSG_TYPE_MMS)
		{
			msgInfo->mainType = MSG_MMS_TYPE;
			msgInfo->subType = MSG_SENDREQ_MMS;
		}
		else
		{
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
		postfix = rand()%10000;
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

		if (msg_type == MSG_TYPE_MMS)
		{
			snprintf(msgInfo->subject, MAX_SUBJECT_LEN+1, "subject %d", i);

			if(folder_id == MSG_INBOX_ID) msgInfo->networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;

			//MMS_MESSAGE_DATA_S* mms_data;
			//MMS_PAGE_S* page[2];
			//MMS_MEDIA_S* media[5];

//			mms_data = msg_mms_create_message();

//			msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
//			msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
//			msg_mms_add_region(mms_data, "Text", 0, 0, 100, 50, 0xffffff);

			//------------>  1st Slide Composing
//			page[0] = msg_mms_add_page(mms_data, 5440);

//			media[0] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_IMG, "Image", (char*)"/opt/etc/msg-service/P091120_104633.jpg");
//			media[1] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_AUDIO, NULL, (char*)"/opt/etc/msg-service/audio.amr");
//			media[2] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp0_2.txt");
//			media[2]->sMedia.sText.nColor = 0x000000;
//			media[2]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
//			media[2]->sMedia.sText.bBold = true;

			//------------>  2nd Slide Composing
//			page[1] = msg_mms_add_page(mms_data, 4544);

//			media[3] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp1_0.txt");
//			media[3]->sMedia.sText.nColor = 0x000000;
//			media[3]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
//			media[3]->sMedia.sText.bItalic = true;
//			media[4] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_VIDEO, "Text", (char*)"/opt/etc/msg-service/V091120_104905.3gp");
//			strncpy(media[4]->szAlt, "Video Load Fail", MAX_SMIL_ALT_LEN-1);

			//FIXME msg_mms_set_message_body((msg_message_t)&msgInfo, mms_data);

//			msg_mms_destroy_message(mms_data);
		}

		//err = msg_add_message(handle, (msg_message_t)&msgInfo, &sendingOpt);
		try
		{
			MsgHandle* pHandle = (MsgHandle*)handle;
			err = pHandle->addMessage(msgInfo, &sendingOpt);
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());
			return MSG_ERR_STORAGE_ERROR;
		}

		if (msg_type == MSG_TYPE_MMS && msgInfo->pMmsData) //free pMmsData directly. It is added to enhance performance
			delete [] static_cast<char*>(msgInfo->pMmsData);

		if (err < 0)
		{
			MSG_DEBUG("err [%d]", err);
			return err;
		}

		msg_release_struct((msg_struct_t *)&msg_s);
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_generate_sms(msg_handle_t handle, msg_folder_id_t folder_id, unsigned int num_msg)
{
	MSG_DEBUG("folder %d, num_msg %d", folder_id, num_msg);

	if (handle == NULL)
	{
		MSG_DEBUG("Handle is NULL");
		return -EINVAL;
	}

	if (folder_id >= MSG_MAX_FOLDER_ID)
	{
		MSG_DEBUG("folderId is invalid");
		return -EINVAL;
	}

	int err = 0;
	char strMsg[20] = {0};
	char prefix[10] ="0103001";
	int postfix = 0;

	MSG_SENDINGOPT_S sendingOpt = {0};
	sendingOpt.bSetting = false;

	srand(getpid());

	msg_struct_s *msg_s = NULL;
	msg_struct_s *addr_s = NULL;
	MSG_MESSAGE_HIDDEN_S *msgInfo = NULL;
	MSG_ADDRESS_INFO_S *addrInfo = NULL;

	for (unsigned int i = 0; i < num_msg; i++)
	{
		msg_s = (msg_struct_s *)msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
		msgInfo = (MSG_MESSAGE_HIDDEN_S *)msg_s->data;

		msgInfo->msgId	= 0; // It should be set 0
		msgInfo->folderId = folder_id;

		msgInfo->mainType = MSG_SMS_TYPE;
		msgInfo->subType = 0;

		msgInfo->storageId = MSG_STORAGE_PHONE;

		snprintf(strMsg, sizeof(strMsg), "test %d", i);
		msgInfo->dataSize = strlen(strMsg);
		msgInfo->pData = strMsg;

		msgInfo->addr_list->nCount = 1;

		addr_s = (msg_struct_s *)msgInfo->addr_list->msg_struct_info[0];

		addrInfo = (MSG_ADDRESS_INFO_S *)addr_s->data;

		addrInfo->addressType = MSG_ADDRESS_TYPE_PLMN;
		postfix = rand()%10000;
		snprintf(addrInfo->addressVal, MAX_ADDRESS_VAL_LEN+1, "%s%04d", prefix, postfix);

		addrInfo->recipientType = MSG_RECIPIENTS_TYPE_TO;

		time(&(msgInfo->displayTime));

		msgInfo->networkStatus = MSG_NETWORK_NOT_SEND;
		msgInfo->bRead = false;
		msgInfo->bProtected = false;
		msgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
		msgInfo->direction = MSG_DIRECTION_TYPE_MO;

//		err = msg_add_message(handle, (msg_message_t) &msgInfo, &sendingOpt);
		try
		{
			MsgHandle* pHandle = (MsgHandle*)handle;
			err = pHandle->addMessage(msgInfo, &sendingOpt);
		}
		catch (MsgException& e)
		{
			MSG_FATAL("%s", e.what());
			return MSG_ERR_STORAGE_ERROR;
		}

		if (err < 0)
		{
			MSG_DEBUG("err [%d]", err);
			return err;
		}

		msg_release_struct((msg_struct_t *)&msg_s);
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_quick_panel_data(msg_handle_t handle, msg_quickpanel_type_t type, msg_struct_t opq_msg)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || !opq_msg )
	{
		MSG_FATAL("handle or opq_msg is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pMsg = (msg_struct_s *)opq_msg;

	try
	{
		err = pHandle->getQuickPanelData(type, (MSG_MESSAGE_HIDDEN_S *)pMsg->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_reset_database(msg_handle_t handle)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->resetDatabase();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_mem_size(msg_handle_t handle, unsigned int* memsize)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMemSize(memsize);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;

}

EXPORT_API int msg_backup_message(msg_handle_t handle)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->backupMessage();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_restore_message(msg_handle_t handle)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->restoreMessage();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_search_message_for_thread_view(msg_handle_t handle, const char *search_string, msg_struct_list_s *msg_thread_view_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || search_string == NULL)
	{
		return -EINVAL;
	}

	if (strlen(search_string) <= 0 || strlen(search_string) > MAX_MSG_TEXT_LEN)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->searchMessage(search_string, msg_thread_view_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_search_message(msg_handle_t handle, const msg_struct_t msg_search_conditions, int offset, int limit, msg_struct_list_s *msg_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_search_conditions == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *pStruct = (msg_struct_s *)msg_search_conditions;

	try
	{
		err = pHandle->searchMessage((MSG_SEARCH_CONDITION_S *)pStruct->data, offset, limit, msg_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_reject_msg_list(msg_handle_t handle, const char *phone_num, msg_struct_list_s *msg_reject_msg_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getRejectMsgList(phone_num, msg_reject_msg_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_reg_storage_change_callback(msg_handle_t handle, msg_storage_change_cb cb, void *user_param)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || cb == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->regStorageChangeCallback(cb, user_param);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_CALLBACK_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_report_status(msg_handle_t handle, msg_message_id_t msg_id, msg_struct_list_s *report_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_id < 1 || report_list == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getReportStatus(msg_id, report_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_get_address_list(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_list_s *msg_address_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getAddressList(thread_id, msg_address_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread_id_by_address(msg_handle_t handle, msg_struct_list_s *msg_address_list, msg_thread_id_t *thread_id)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_address_list->nCount < 1)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getThreadIdByAddress(msg_address_list, thread_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread(msg_handle_t handle, msg_thread_id_t thread_id, msg_struct_t msg_thread)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || msg_thread == NULL ) {
		MSG_FATAL("handle or msg_thread is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	msg_struct_s *msgThread = (msg_struct_s *)msg_thread;
	if (msgThread->type != MSG_STRUCT_THREAD_INFO) {
		MSG_FATAL("Invaild type. type [%d]", msgThread->type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_THREAD_VIEW_S* pThreadInfo = (MSG_THREAD_VIEW_S *)msgThread->data;

	try
	{
		err = pHandle->getThread(thread_id, pThreadInfo);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_message_list(msg_handle_t handle, msg_folder_id_t folder_id, msg_thread_id_t thread_id, msg_message_type_t msg_type, msg_storage_id_t storage_id, msg_struct_list_s *msg_list)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL) {
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMessageList(folder_id, thread_id, msg_type, storage_id, msg_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

static int msg_get_msg_type(int mainType, int subType)
{
	if (mainType == MSG_SMS_TYPE)
	{
		if (subType == MSG_CB_SMS)
			return MSG_TYPE_SMS_CB;
		else if (subType == MSG_JAVACB_SMS)
			return MSG_TYPE_SMS_JAVACB;
		else if (subType == MSG_WAP_SI_SMS || subType == MSG_WAP_SL_SMS)
			return MSG_TYPE_SMS_WAPPUSH;
		else if (subType == MSG_MWI_VOICE_SMS || subType == MSG_MWI_FAX_SMS
				|| subType == MSG_MWI_EMAIL_SMS || subType == MSG_MWI_OTHER_SMS)
			return MSG_TYPE_SMS_MWI;
		else 	if (subType == MSG_SYNCML_CP)
			return MSG_TYPE_SMS_SYNCML;
		else 	if (subType == MSG_REJECT_SMS)
			return MSG_TYPE_SMS_REJECT;
		else
			return MSG_TYPE_SMS;
	}
	else if (mainType == MSG_MMS_TYPE)
	{
		if (subType == MSG_NOTIFICATIONIND_MMS)
			return MSG_TYPE_MMS_NOTI;
		else if (subType == MSG_SENDREQ_JAVA_MMS)
			return MSG_TYPE_MMS_JAVA;
		else
			return MSG_TYPE_MMS;
	}
	else
		return MSG_TYPE_INVALID;
}


int msg_syncml_info_get_int(void *syncml_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)syncml_info;
	switch(field)
	{
	case MSG_SYNCML_INFO_EXTID_INT:
		result = pSync->extId;
		break;
	case MSG_SYNCML_INFO_PINCODE_INT:
		result = pSync->pinCode;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_count_info_get_int(void *count_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_COUNT_INFO_S *pCount = (MSG_COUNT_INFO_S *)count_info;
	switch(field)
	{
	case MSG_COUNT_INFO_READ_INT:
		result = pCount->nReadCnt;
		break;
	case MSG_COUNT_INFO_UNREAD_INT:
		result = pCount->nUnreadCnt;
		break;
	case MSG_COUNT_INFO_SMS_INT:
		result = pCount->nSms;
		break;
	case MSG_COUNT_INFO_MMS_INT:
		result = pCount->nMms;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_thread_count_get_int(void *count_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_THREAD_COUNT_INFO_S *pCount = (MSG_THREAD_COUNT_INFO_S *)count_info;
	switch(field)
	{
	case MSG_THREAD_COUNT_TOTAL_INT:
		result = pCount->totalCount;
		break;
	case MSG_THREAD_COUNT_UNREAD_INT:
		result = pCount->unReadCount;
		break;
	case MSG_THREAD_COUNT_SMS_INT:
		result = pCount->smsMsgCount;
		break;
	case MSG_THREAD_COUNT_MMS_INT:
		result = pCount->mmsMsgCount;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_thread_index_get_int(void *index_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_THREAD_LIST_INDEX_S *pIndex = (MSG_THREAD_LIST_INDEX_S *)index_info;
	switch(field)
	{
	case MSG_THREAD_LIST_INDEX_CONTACTID_INT:
		result = pIndex->contactId;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_sortrule_get_int(void *sort_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_info;
	switch(field)
	{
	case MSG_SORT_RULE_SORT_TYPE_INT:
		result = pSort->sortType;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_folder_info_get_int(void *folder_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;
	switch(field)
	{
	case MSG_FOLDER_INFO_ID_INT:
		result = pFolder->folderId;
		break;
	case MSG_FOLDER_INFO_TYPE_INT:
		result = pFolder->folderType;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_thread_info_get_int(void *data, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_THREAD_VIEW_S *pThread = (MSG_THREAD_VIEW_S *)data;

	switch(field)
	{
	case MSG_THREAD_ID_INT :
		result = pThread->threadId;
		break;
	case MSG_THREAD_MSG_TYPE_INT :
		result = msg_get_msg_type(pThread->mainType, pThread->subType);
		break;
	case MSG_THREAD_MSG_TIME_INT :
		result = pThread->threadTime;
		break;
	case MSG_THREAD_DIRECTION_INT :
		result = pThread->direction;
		break;
	case MSG_THREAD_UNREAD_COUNT_INT :
		result = pThread->unreadCnt;
		break;
	case MSG_THREAD_SMS_COUNT_INT :
		result = pThread->smsCnt;
		break;
	case MSG_THREAD_MMS_COUNT_INT :
		result = pThread->mmsCnt;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}


int msg_conv_info_get_int(void *data, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;

	switch(field)
	{
	case MSG_CONV_MSG_ID_INT :
		result = pConv->msgId;
		break;
	case MSG_CONV_MSG_THREAD_ID_INT :
		result = pConv->threadId;
		break;
	case MSG_CONV_MSG_FOLDER_ID_INT :
		result = pConv->folderId;
		break;
	case MSG_CONV_MSG_TYPE_INT :
		result = msg_get_msg_type(pConv->mainType, pConv->subType);
		break;
	case MSG_CONV_MSG_STORAGE_ID_INT :
		result = pConv->storageId;
		break;
	case MSG_CONV_MSG_DISPLAY_TIME_INT :
		result = pConv->displayTime;
		break;
	case MSG_CONV_MSG_SCHEDULED_TIME_INT :
		result = pConv->scheduledTime;
		break;
	case MSG_CONV_MSG_NETWORK_STATUS_INT :
		result = pConv->networkStatus;
		break;
	case MSG_CONV_MSG_DIRECTION_INT :
		result = pConv->direction;
		break;
	case MSG_CONV_MSG_ATTACH_COUNT_INT :
		result = pConv->attachCount;
		break;
	case MSG_CONV_MSG_TEXT_SIZE_INT :
		result = pConv->textSize;
		break;
	case MSG_CONV_MSG_PAGE_COUNT_INT :
		result = pConv->pageCount;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}


int msg_search_condition_get_int(void *condition_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_SEARCH_CONDITION_S *pCond = (MSG_SEARCH_CONDITION_S *)condition_info;
	switch(field)
	{
	case MSG_SEARCH_CONDITION_FOLDERID_INT:
		result = pCond->folderId;
		break;
	case MSG_SEARCH_CONDITION_MSGTYPE_INT:
		result = pCond->msgType;
		break;
	case MSG_SEARCH_CONDITION_RESERVED_INT:
		result = pCond->reserved;
		break;
	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

int msg_report_status_get_int(void *report_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;
	switch(field)
	{
	case MSG_REPORT_TYPE_INT:
		result = pReport->type;
		break;
	case MSG_REPORT_STATUS_INT:
		result = pReport->status;
		break;
	case MSG_REPORT_TIME_INT:
		result = pReport->statusTime;
		break;

	default:
		result = MSG_ERR_INVALID_PARAMETER;
		break;
	}
	return result;
}

char* msg_report_status_get_str(void *report_info, int field)
{
	char *result = NULL;
	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;
	switch(field)
	{

	case MSG_REPORT_ADDRESS_STR:
		result = pReport->addressVal;
		break;

	default:
		break;
	}
	return result;
}

char* msg_folder_info_get_str(void *folder_info, int field)
{
	char *result = NULL;
	MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;
	switch(field)
	{
	case MSG_FOLDER_INFO_NAME_STR:
		result = pFolder->folderName;
		break;
	default:
		result = NULL;
		break;
	}
	return result;
}

char *msg_thread_info_get_str(void *data, int field)
{
	char *ret_str = NULL;
	MSG_THREAD_VIEW_S *pThread = (MSG_THREAD_VIEW_S *)data;

	switch(field)
	{
	case MSG_THREAD_NAME_STR :
		ret_str = pThread->threadName;
		break;
	case MSG_THREAD_MSG_DATA_STR :
		ret_str = pThread->threadData;
		break;
	default:
		break;
	}

	return ret_str;
}


char *msg_conv_info_get_str(void *data, int field)
{
	char *ret_str = NULL;
	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;

	switch(field)
	{
	case MSG_CONV_MSG_SUBJECT_STR :
		ret_str = pConv->subject;
		break;
	case MSG_CONV_MSG_ATTACH_NAME_STR :
		ret_str = pConv->attachFileName;
		break;
	case MSG_CONV_MSG_AUDIO_NAME_STR :
		ret_str = pConv->audioFileName;
		break;
	case MSG_CONV_MSG_IMAGE_THUMB_PATH_STR :
		ret_str = pConv->imageThumbPath;
		break;
	case MSG_CONV_MSG_VIDEO_THUMB_PATH_STR :
		ret_str = pConv->videoThumbPath;
		break;
	case MSG_CONV_MSG_TEXT_STR :
		ret_str = pConv->pText;
		break;
	default:
		break;
	}

	return ret_str;
}


char* msg_search_condition_get_str(void *condition_info, int field, int size)
{
	char *result = NULL;
	MSG_SEARCH_CONDITION_S *search_cond = (MSG_SEARCH_CONDITION_S *)condition_info;
	switch(field)
	{
	case MSG_SEARCH_CONDITION_ADDRESS_VALUE_STR:
		result = search_cond->pAddressVal;
		break;
	case MSG_SEARCH_CONDITION_SEARCH_VALUE_STR:
		result = search_cond->pSearchVal;
		break;

	default:
		result = NULL;
		break;
	}
	return result;
}

bool msg_sendopt_get_bool(void *send_opt, int field)
{
	bool result = false;
	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)send_opt;
	switch(field)
	{
	case MSG_SEND_OPT_SETTING_BOOL:
		result = sendopt->bDeliverReq;
		break;
	case MSG_SEND_OPT_KEEPCOPY_BOOL:
		result = sendopt->bDeliverReq;
		break;
	case MSG_SEND_OPT_DELIVER_REQ_BOOL:
		result = sendopt->bDeliverReq;
		break;
	default:
		break;
	}
	return result;
}

bool msg_sortrule_get_bool(void *sort_rule, int field)
{
	bool result = false;
	MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_rule;
	switch(field)
	{
	case MSG_SORT_RULE_ACSCEND_BOOL:
		result = pSort->bAscending;
		break;
	default:
		break;
	}
	return result;
}

bool msg_conv_get_bool(void *data, int field)
{
	bool result = false;
	MSG_CONVERSATION_VIEW_S *pConv = (MSG_CONVERSATION_VIEW_S *)data;
	switch(field)
	{
	case MSG_CONV_MSG_READ_BOOL:
		result = pConv->bRead;
		break;
	case MSG_CONV_MSG_PROTECTED_BOOL:
		result = pConv->bProtected;
		break;
	default:
		break;
	}
	return result;
}

bool msg_thread_info_get_bool(void *data, int field)
{
	bool result = false;
	MSG_THREAD_VIEW_S *pthreadInfo = (MSG_THREAD_VIEW_S *)data;
	switch(field)
	{
	case MSG_THREAD_PROTECTED_BOOL:
		result = pthreadInfo->bProtected;
		break;
	default:
		break;
	}
	return result;
}

int msg_sendopt_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)msg_struct->data;

	switch(field)
	{
	case MSG_SEND_OPT_MMS_OPT_HND:
		*value = (void *)sendopt->mmsSendOpt;
		break;
	case MSG_SEND_OPT_SMS_OPT_HND:
		*value = (void *)sendopt->smsSendOpt;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;
	}
	return err;
}

int msg_syncml_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err =  MSG_SUCCESS;

	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)msg_struct->data;
	switch(field)
	{
	case MSG_SYNCML_INFO_MESSAGE_HND:
		*value = (void *)pSync->msg;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;

	}
	return err;
}

int msg_thread_index_get_struct_handle(msg_struct_s *msg_struct, int field, void **value)
{
	msg_error_t err =  MSG_SUCCESS;

	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_LIST_INDEX_INFO_S *pIndex = (MSG_THREAD_LIST_INDEX_INFO_S *)msg_struct->data;
	switch(field)
	{
	case MSG_SYNCML_INFO_MESSAGE_HND:
		*value = (void *)pIndex->msgAddrInfo;
		break;
	default:
		err = MSG_ERR_UNKNOWN;
		break;

	}
	return err;
}

int msg_address_info_get_int(void *addr_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;
	switch(field)
	{
	case MSG_ADDRESS_INFO_ADDRESS_TYPE_INT:
		result = pAddr->addressType;
		break;
	case MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT:
		result = pAddr->recipientType;
		break;
	case MSG_ADDRESS_INFO_CONTACT_ID_INT:
		result = pAddr->contactId;
		break;
	default:
		break;
	}
	return result;
}
int msg_mms_sendopt_get_int(void *opt_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;
	switch(field)
	{
	case MSG_MMS_SENDOPTION_EXPIRY_TIME_INT:
		result = pOpt->expiryTime;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_TIME_INT:
		result = pOpt->deliveryTime;
		break;
	case MSG_MMS_SENDOPTION_PRIORITY_INT:
		result = pOpt->priority;
		break;
	default:
		break;
	}
	return result;
}

int msg_reject_message_get_int(void *msg_info, int field)
{
	int result = MSG_ERR_INVALID_PARAMETER;
	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;
	switch(field)
	{
	case MSG_REJECT_MESSAGE_MSGID_INT:
		result = pMsg->msgId;
		break;
	case MSG_REJECT_MESSAGE_DISPLAY_TIME_INT:
		result = pMsg->displayTime;
		break;
	default:
		break;
	}
	return result;
}

char* msg_address_info_get_str(void *addr_info, int field, int size)
{
	char *result = NULL;
	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;
	switch(field)
	{
	case MSG_ADDRESS_INFO_ADDRESS_VALUE_STR:
		result = pAddr->addressVal;
		break;
	case MSG_ADDRESS_INFO_DISPLAYNAME_STR:
		result = pAddr->displayName;
		break;

	default:
		result = NULL;
		break;
	}
	return result;
}

char* msg_reject_message_get_str(void *msg_info, int field, int size)
{
	char *result = NULL;
	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;
	switch(field)
	{
	case MSG_REJECT_MESSAGE_MSGTEXT_STR:
		result = pMsg->msgText;
		break;
	default:
		result = NULL;
		break;
	}
	return result;
}

bool msg_mms_sendopt_get_bool(void *opt_info, int field)
{
	bool result = false;
	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;
	switch(field)
	{
	case MSG_MMS_SENDOPTION_READ_REQUEST_BOOL:
		result = pOpt->bReadReq;
		break;
	case MSG_MMS_SENDOPTION_DELIVERY_CUSTOMTIME_BOOL:
		result = pOpt->bUseDeliveryCustomTime;
		break;
	default:
		break;
	}
	return result;
}

bool msg_sms_sendopt_get_bool(void *opt_info, int field)
{
	bool result = false;
	SMS_SENDINGOPT_S *pOpt = (SMS_SENDINGOPT_S *)opt_info;
	switch(field)
	{
	case MSG_SMS_SENDOPT_REPLYPATH_BOOL:
		result = pOpt->bReplyPath;
		break;
	default:
		break;
	}
	return result;
}

int msg_syncml_info_set_int(void *syncml_info, int field, int value)
{

	msg_error_t err =  MSG_SUCCESS;
	if(!syncml_info)
		return MSG_ERR_NULL_POINTER;

    MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)syncml_info;
    switch(field)
    {
    case MSG_SYNCML_INFO_EXTID_INT:
		pSync->extId = value;
		break;
    case MSG_SYNCML_INFO_PINCODE_INT:
		pSync->pinCode = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }

	return err;
}

int msg_count_info_set_int(void *count_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!count_info)
		return MSG_ERR_NULL_POINTER;

    MSG_COUNT_INFO_S *pCount = (MSG_COUNT_INFO_S *)count_info;

    switch(field)
    {
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
		err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}

int msg_thread_count_set_int(void *count_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!count_info)
		return MSG_ERR_NULL_POINTER;

    MSG_THREAD_COUNT_INFO_S *pCount = (MSG_THREAD_COUNT_INFO_S *)count_info;
    switch(field)
    {
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
		err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}

int msg_thread_index_set_int(void *index_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!index_info)
		return MSG_ERR_NULL_POINTER;

    MSG_THREAD_LIST_INDEX_S *pIndex = (MSG_THREAD_LIST_INDEX_S *)index_info;
    switch(field)
    {
    case MSG_THREAD_LIST_INDEX_CONTACTID_INT:
		pIndex->contactId = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }

	return err;
}

int msg_sortrule_set_int(void *sort_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;

	if(!sort_info)
		return MSG_ERR_NULL_POINTER;

    MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_info;
    switch(field)
    {
    case MSG_SORT_RULE_SORT_TYPE_INT:
		pSort->sortType = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}

int msg_folder_info_set_int(void *folder_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!folder_info)
		return MSG_ERR_NULL_POINTER;

    MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;
    switch(field)
    {
    case MSG_FOLDER_INFO_ID_INT:
		pFolder->folderId = value;
		break;
    case MSG_FOLDER_INFO_TYPE_INT:
		pFolder->folderType = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }

	return err;
}

int msg_search_condition_set_int(void *condition_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!condition_info)
		return MSG_ERR_NULL_POINTER;

    MSG_SEARCH_CONDITION_S *pCond = (MSG_SEARCH_CONDITION_S *)condition_info;
    switch(field)
    {
    case MSG_SEARCH_CONDITION_FOLDERID_INT:
		pCond->folderId = value;
		break;
    case MSG_SEARCH_CONDITION_MSGTYPE_INT:
		pCond->msgType = value;
		break;
    case MSG_SEARCH_CONDITION_RESERVED_INT:
		pCond->reserved = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }

	return err;
}

int msg_report_status_set_int(void *report_info, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!report_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REPORT_STATUS_INFO_S *pReport = (MSG_REPORT_STATUS_INFO_S *)report_info;
	switch(field)
	{
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
		err = MSG_ERR_UNKNOWN;
		break;
	}

	return err;
}

int msg_folder_info_set_str(void *folder_info, int field, char *value, int size)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!folder_info || !value)
		return MSG_ERR_NULL_POINTER;
    MSG_FOLDER_INFO_S *pFolder = (MSG_FOLDER_INFO_S *)folder_info;
    int _len = 0;
    (size > MAX_FOLDER_NAME_SIZE)? _len = MAX_FOLDER_NAME_SIZE : _len = size;
    switch(field)
    {
    case MSG_FOLDER_INFO_NAME_STR:
		strncpy(pFolder->folderName, value, _len);
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }

	return err;
}

int msg_search_condition_set_str(void *condition_info, int field, char *value, int size)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!condition_info || !value)
		return MSG_ERR_NULL_POINTER;

    MSG_SEARCH_CONDITION_S *search_cond = (MSG_SEARCH_CONDITION_S *)condition_info;

    switch(field)
    {
    case MSG_SEARCH_CONDITION_ADDRESS_VALUE_STR:
		search_cond->pAddressVal = value;
		break;
    case MSG_SEARCH_CONDITION_SEARCH_VALUE_STR:
		search_cond->pSearchVal = value;
		break;

    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}

int msg_sendopt_set_bool(void *send_opt, int field, bool value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!send_opt)
		return MSG_ERR_NULL_POINTER;

    MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)send_opt;
    switch(field)
    {
    case MSG_SEND_OPT_SETTING_BOOL:
		sendopt->bDeliverReq = value;
		break;
    case MSG_SEND_OPT_KEEPCOPY_BOOL:
		sendopt->bDeliverReq = value;
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
	msg_error_t err =  MSG_SUCCESS;
	if(!sort_rule)
		return MSG_ERR_NULL_POINTER;

    MSG_SORT_RULE_S *pSort = (MSG_SORT_RULE_S *)sort_rule;
    switch(field)
    {
    case MSG_SORT_RULE_ACSCEND_BOOL:
		pSort->bAscending = value;
		break;
    default:
		err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}

int msg_sendopt_set_struct_handle(msg_struct_s *msg_struct, int field, msg_struct_s *value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SENDINGOPT_S *sendopt = (MSG_SENDINGOPT_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch(field)
	{
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
	msg_error_t err =  MSG_SUCCESS;

	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_SYNCML_MESSAGE_S *pSync = (MSG_SYNCML_MESSAGE_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch(field)
	{
	case MSG_SYNCML_INFO_MESSAGE_HND:
	{
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
	msg_error_t err =  MSG_SUCCESS;

	if(!msg_struct || !value)
		return MSG_ERR_NULL_POINTER;

	MSG_THREAD_LIST_INDEX_INFO_S *pIndex = (MSG_THREAD_LIST_INDEX_INFO_S *)msg_struct->data;
	msg_struct_s *pTmp = NULL;

	switch(field)
	{
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

int msg_address_info_set_int(void *addrinfo, int field, int value)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!addrinfo)
		return MSG_ERR_NULL_POINTER;

	MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addrinfo;
	switch(field)
	{
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
	msg_error_t err =  MSG_SUCCESS;
	if(!opt_info)
		return MSG_ERR_NULL_POINTER;

	MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)opt_info;
	switch(field)
	{
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
	msg_error_t err =  MSG_SUCCESS;
	if(!msg_info)
		return MSG_ERR_NULL_POINTER;

	MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;
	switch(field)
	{
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

int msg_address_info_set_str(void *addr_info, int field, char *value, int size)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!addr_info || !value)
		return MSG_ERR_NULL_POINTER;
    MSG_ADDRESS_INFO_S *pAddr = (MSG_ADDRESS_INFO_S *)addr_info;
    int _len = 0;

    switch(field)
    {
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
int msg_reject_message_set_str(void *msg_info, int field, char *value, int size)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!msg_info || !value)
		return MSG_ERR_NULL_POINTER;
    MSG_REJECT_MSG_INFO_S *pMsg = (MSG_REJECT_MSG_INFO_S *)msg_info;
    int _len = 0;
    (size > MAX_MSG_TEXT_LEN)? _len = MAX_MSG_TEXT_LEN : _len = size;
    switch(field)
    {
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
	msg_error_t err =  MSG_SUCCESS;
	if(!option)
		return MSG_ERR_NULL_POINTER;

    MMS_SENDINGOPT_S *pOpt = (MMS_SENDINGOPT_S *)option;
    switch(field)
    {
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
	msg_error_t err =  MSG_SUCCESS;
	if(!option)
		return MSG_ERR_NULL_POINTER;

    SMS_SENDINGOPT_S *pOpt = (SMS_SENDINGOPT_S *)option;
    switch(field)
    {
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
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || push_event == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pPush = (msg_struct_s *)push_event;

	try
	{
		err = pHandle->addPushEvent((MSG_PUSH_EVENT_INFO_S *)pPush->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_delete_push_event(msg_handle_t handle, const msg_struct_t push_event)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || push_event == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pPush = (msg_struct_s *)push_event;

	try
	{
		err = pHandle->deletePushEvent((MSG_PUSH_EVENT_INFO_S *)pPush->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

EXPORT_API int msg_update_push_event(msg_handle_t handle, const msg_struct_t src_event, const msg_struct_t dst_event)
{
	msg_error_t err =  MSG_SUCCESS;

	if (handle == NULL || src_event == NULL || dst_event == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	msg_struct_s *pSrc = (msg_struct_s *)src_event;
	msg_struct_s *pDst = (msg_struct_s *)dst_event;

	try
	{
		err = pHandle->updatePushEvent((MSG_PUSH_EVENT_INFO_S *)pSrc->data, (MSG_PUSH_EVENT_INFO_S *)pDst->data);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

char* msg_push_config_get_str(void *event_info, int field, int size)
{
	char *result = NULL;
	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;
	switch(field)
	{
    case MSG_PUSH_CONFIG_CONTENT_TYPE_STR:
		result = pEvent->contentType;
		break;
    case MSG_PUSH_CONFIG_APPLICATON_ID_STR:
		result = pEvent->appId;
		break;
    case MSG_PUSH_CONFIG_PACKAGE_NAME_STR:
		result = pEvent->pkgName;
		break;

	default:
		result = NULL;
		break;
	}
	return result;
}

bool msg_push_config_get_bool(void *event_info, int field)
{
	bool result = false;
	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;
	switch(field)
	{
    case MSG_PUSH_CONFIG_LAUNCH_BOOL:
    	result = pEvent->bLaunch;
		break;
	default:
		break;
	}
	return result;
}

int msg_push_config_set_str(void *event_info, int field, char *value, int size)
{
	msg_error_t err =  MSG_SUCCESS;
	if(!event_info || !value)
		return MSG_ERR_NULL_POINTER;
    MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event_info;
    int _len = 0;

    switch(field)
    {
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
	msg_error_t err =  MSG_SUCCESS;
	if(!event)
		return MSG_ERR_NULL_POINTER;

	MSG_PUSH_EVENT_INFO_S *pEvent = (MSG_PUSH_EVENT_INFO_S *)event;
    switch(field)
    {
    case MSG_PUSH_CONFIG_LAUNCH_BOOL:
    	pEvent->bLaunch = value;
		break;
    default:
    	err = MSG_ERR_UNKNOWN;
		break;
    }
	return err;
}
