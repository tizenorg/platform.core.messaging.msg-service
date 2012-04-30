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

#include <errno.h>

#include "MsgHandle.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MapiMessage.h"
#include "MapiStorage.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API int msg_add_message(MSG_HANDLE_T handle, const msg_message_t opq_msg, const MSG_SENDINGOPT_S *send_opt)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	MSG_MESSAGE_S* pMsg = (MSG_MESSAGE_S*) opq_msg;

	try
	{
		err = pHandle->addMessage(pMsg, send_opt);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_syncml_message(MSG_HANDLE_T handle, const MSG_SYNCML_MESSAGE_S *syncml_msg)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || syncml_msg == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->addSyncMLMessage(syncml_msg);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_message(MSG_HANDLE_T handle, const msg_message_t opq_msg, const MSG_SENDINGOPT_S *send_opt)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || opq_msg == NULL || send_opt == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	MSG_MESSAGE_S* pMsg = (MSG_MESSAGE_S*) opq_msg;

	if (pMsg->nAddressCnt > 1)
	{
		MSG_DEBUG("Multiple Address cannot be updated [%d]", pMsg->nAddressCnt);
		return -EINVAL;
	}

	try
	{
		err = pHandle->updateMessage(pMsg, send_opt);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_read_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, bool read)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_update_protected_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, bool is_protected)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_delete_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_delete_all_msgs_in_folder(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, bool bOnlyDB)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_move_msg_to_folder(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_FOLDER_ID_T dest_folder_id)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_move_msg_to_storage(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_STORAGE_ID_T storage_id)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_count_message(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, MSG_COUNT_INFO_S *count_info)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->countMessage(folder_id, count_info);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_count_msg_by_type(MSG_HANDLE_T handle, MSG_MESSAGE_TYPE_T msg_type, int *msg_count)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_count_msg_by_contact(MSG_HANDLE_T handle, const MSG_THREAD_LIST_INDEX_S *addr_info, MSG_THREAD_COUNT_INFO_S *msg_thread_count_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || addr_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->countMsgByContact(addr_info, msg_thread_count_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, msg_message_t opq_msg, MSG_SENDINGOPT_S *send_opt)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || !opq_msg)
	{
		MSG_FATAL("handle or opq_msg is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	MSG_MESSAGE_S* pMsg = (MSG_MESSAGE_S*) opq_msg;

	try
	{
		err = pHandle->getMessage(msg_id, pMsg, send_opt);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_folder_view_list(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, const MSG_SORT_RULE_S *sort_rule, MSG_LIST_S *msg_folder_view_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

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
		err = pHandle->getFolderViewList(folder_id, sort_rule, msg_folder_view_list);
	}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_get_thread_view_list(MSG_HANDLE_T handle, const MSG_SORT_RULE_S *sort_rule, MSG_THREAD_VIEW_LIST_S *msg_thread_view_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

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
			err = pHandle->getThreadViewList(sort_rule, msg_thread_view_list);
		}
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API void msg_release_thread_view_list(MSG_THREAD_VIEW_LIST_S *msg_thread_view_list)
{
	if (msg_thread_view_list == NULL)
	{
		return;
	}

	// Memory Free
	if(msg_thread_view_list->msgThreadInfo != NULL)
	{
		if(msg_thread_view_list->nCount > 0)
		{
			for(int i=0; i<msg_thread_view_list->nCount; i++)
				delete [] (MSG_THREAD_VIEW_S*)msg_thread_view_list->msgThreadInfo[i];
		}

		//free peer info list
		delete [] msg_thread_view_list->msgThreadInfo;
		msg_thread_view_list->msgThreadInfo = NULL;
	}

	msg_thread_view_list->nCount = 0;
}


EXPORT_API int msg_get_conversation_view_list(MSG_HANDLE_T handle, MSG_THREAD_ID_T thread_id, MSG_LIST_S *msg_conv_view_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_delete_thread_message_list(MSG_HANDLE_T handle, MSG_THREAD_ID_T thread_id)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->deleteThreadMessageList(thread_id);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_add_folder(MSG_HANDLE_T handle, const MSG_FOLDER_INFO_S *folder_info)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->addFolder(folder_info);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_update_folder(MSG_HANDLE_T handle, const MSG_FOLDER_INFO_S *folder_info)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || folder_info == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->updateFolder(folder_info);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_delete_folder(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_get_folder_list(MSG_HANDLE_T handle, MSG_FOLDER_LIST_S *folder_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API void msg_release_folder_list(MSG_FOLDER_LIST_S *folder_list)
{
	if (folder_list == NULL)
	{
		return;
	}

	// Memory Free
	if (folder_list->folderInfo != NULL)
	{
		free(folder_list->folderInfo);
		folder_list->folderInfo = NULL;
	}

	folder_list->nCount = 0;
}


EXPORT_API int msg_generate_message(MSG_HANDLE_T handle, MSG_MESSAGE_TYPE_T msg_type, MSG_FOLDER_ID_T folder_id, unsigned int num_msg)
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
	// Make Message
	MSG_MESSAGE_S msgInfo = {0, };	//structure is used to enhance performance

	for (unsigned int i = 0; i < num_msg; i++)
	{
		bzero(&msgInfo, sizeof(MSG_MESSAGE_S));

		msgInfo.msgId	= 0; // It should be set 0
		msgInfo.folderId = folder_id;

		if (msg_type == MSG_TYPE_MMS)
		{
			msgInfo.msgType.mainType = MSG_MMS_TYPE;
			//msgInfo.msgType.subType = MSG_RETRIEVE_MMS;
			msgInfo.msgType.subType = MSG_SENDREQ_MMS;
		}
		else
		{
			msgInfo.msgType.mainType = MSG_SMS_TYPE;
			msgInfo.msgType.subType = MSG_NORMAL_SMS;

			snprintf(strMsg, sizeof(strMsg), "test msg %d", i);
			msgInfo.dataSize = strlen(strMsg);
			msgInfo.pData = strMsg;
		}

		msgInfo.storageId = MSG_STORAGE_PHONE;

		msgInfo.nAddressCnt = 1;

		msgInfo.addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;

		postfix = rand()%10000;
		snprintf(msgInfo.addressList[0].addressVal, MAX_ADDRESS_VAL_LEN+1, "%s%04d", prefix, postfix);

		msgInfo.addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;

		time(&(msgInfo.displayTime));

		msgInfo.networkStatus = MSG_NETWORK_NOT_SEND;
		msgInfo.bRead = false;
		msgInfo.bProtected = false;
		msgInfo.priority = MSG_MESSAGE_PRIORITY_NORMAL;

		if (folder_id == MSG_OUTBOX_ID || folder_id == MSG_SENTBOX_ID)
			msgInfo.direction = MSG_DIRECTION_TYPE_MO;
		else
			msgInfo.direction = MSG_DIRECTION_TYPE_MT;

		if (msg_type == MSG_TYPE_MMS)
		{
			snprintf(msgInfo.subject, MAX_SUBJECT_LEN+1, "subject %d", i);

			if(folder_id == MSG_INBOX_ID) msgInfo.networkStatus = MSG_NETWORK_RETRIEVE_SUCCESS;

			MMS_MESSAGE_DATA_S* mms_data;
			MMS_PAGE_S* page[2];
			MMS_MEDIA_S* media[5];

			mms_data = msg_mms_create_message();

			msg_mms_set_rootlayout(mms_data, 100, 100, 0xffffff);
			msg_mms_add_region(mms_data, "Image", 0, 50, 100, 50, 0xffffff);
			msg_mms_add_region(mms_data, "Text", 0, 0, 100, 50, 0xffffff);

			//------------>  1st Slide Composing
			page[0] = msg_mms_add_page(mms_data, 5440);

			media[0] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_IMG, "Image", (char*)"/opt/etc/msg-service/P091120_104633.jpg");
			media[1] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_AUDIO, NULL, (char*)"/opt/etc/msg-service/audio.amr");
			media[2] = msg_mms_add_media(page[0], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp0_2.txt");
			media[2]->sMedia.sText.nColor = 0x000000;
			media[2]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
			media[2]->sMedia.sText.bBold = true;

			//------------>  2nd Slide Composing
			page[1] = msg_mms_add_page(mms_data, 4544);

			media[3] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_TEXT, "Text", (char*)"/opt/etc/msg-service/Temp1_0.txt");
			media[3]->sMedia.sText.nColor = 0x000000;
			media[3]->sMedia.sText.nSize = MMS_SMIL_FONT_SIZE_NORMAL;
			media[3]->sMedia.sText.bItalic = true;
			media[4] = msg_mms_add_media(page[1], MMS_SMIL_MEDIA_VIDEO, "Text", (char*)"/opt/etc/msg-service/V091120_104905.3gp");
			strncpy(media[4]->szAlt, "Video Load Fail", MAX_SMIL_ALT_LEN-1);

			msg_mms_set_message_body((msg_message_t)&msgInfo, mms_data);

			msg_mms_destroy_message(mms_data);
		}

		err = msg_add_message(handle, (msg_message_t)&msgInfo, &sendingOpt);

		if (msg_type == MSG_TYPE_MMS && msgInfo.pMmsData) //free pMmsData directly. It is added to enhance performance
			delete [] static_cast<char*>(msgInfo.pMmsData);

		if (err < 0)
		{
			MSG_DEBUG("err [%d]", err);
			return err;
		}
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_generate_sms(MSG_HANDLE_T handle, MSG_FOLDER_ID_T folder_id, unsigned int num_msg)
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

	// Make Message
	MSG_MESSAGE_S msgInfo = {0};
	char strMsg[20] = {0};

	char prefix[10] ="0103001";
	int postfix = 0;

	MSG_SENDINGOPT_S sendingOpt = {0};
	sendingOpt.bSetting = false;

	srand(getpid());

	for (unsigned int i = 0; i < num_msg; i++)
	{
		bzero(&msgInfo, sizeof(msgInfo));
		msgInfo.msgId	= 0; // It should be set 0
		msgInfo.folderId = folder_id;

		msgInfo.msgType.mainType = MSG_SMS_TYPE;
		msgInfo.msgType.subType = 0;

		msgInfo.storageId = MSG_STORAGE_PHONE;

		snprintf(strMsg, sizeof(strMsg), "test %d", i);
		msgInfo.dataSize = strlen(strMsg);
		msgInfo.pData = strMsg;

		msgInfo.addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
		postfix = rand()%10000;
		snprintf(msgInfo.addressList[0].addressVal, MAX_ADDRESS_VAL_LEN+1, "%s%04d", prefix, postfix);
		msgInfo.addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;
		msgInfo.nAddressCnt = 1;

		time(&(msgInfo.displayTime));

		msgInfo.networkStatus = MSG_NETWORK_NOT_SEND;
		msgInfo.bRead = false;
		msgInfo.bProtected = false;
		msgInfo.priority = MSG_MESSAGE_PRIORITY_NORMAL;
		msgInfo.direction = MSG_DIRECTION_TYPE_MO;

		err = msg_add_message(handle, (msg_message_t) &msgInfo, &sendingOpt);

		if (err < 0)
		{
			MSG_DEBUG("err [%d]", err);
			return err;
		}
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_quick_panel_data(MSG_HANDLE_T handle, MSG_QUICKPANEL_TYPE_T type, msg_message_t opq_msg)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || !opq_msg )
	{
		MSG_FATAL("handle or opq_msg is NULL");
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;
	MSG_MESSAGE_S* pMsg = (MSG_MESSAGE_S*) opq_msg;

	try
	{
		err = pHandle->getQuickPanelData(type, pMsg);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_reset_database(MSG_HANDLE_T handle)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_get_mem_size(MSG_HANDLE_T handle, unsigned int* memsize)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_thread_view_get_thread_id(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->threadId;
}


EXPORT_API const char* msg_thread_view_get_address(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return NULL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->threadAddr;
}


EXPORT_API const char* msg_thread_view_get_name(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return NULL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->threadName;
}


EXPORT_API const char* msg_thread_view_get_image_path(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return NULL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->threadImagePath;
}


EXPORT_API int msg_thread_view_get_message_type(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	if (pMsg->threadType.mainType == MSG_SMS_TYPE)
	{
		if (pMsg->threadType.subType == MSG_CB_SMS)
			return MSG_TYPE_SMS_CB;
		else 	if (pMsg->threadType.subType == MSG_JAVACB_SMS)
			return MSG_TYPE_SMS_JAVACB;
		else 	if (pMsg->threadType.subType == MSG_WAP_SI_SMS || pMsg->threadType.subType == MSG_WAP_SL_SMS)
			return MSG_TYPE_SMS_WAPPUSH;
		else 	if (pMsg->threadType.subType == MSG_MWI_VOICE_SMS || pMsg->threadType.subType == MSG_MWI_FAX_SMS
				|| pMsg->threadType.subType == MSG_MWI_EMAIL_SMS || pMsg->threadType.subType == MSG_MWI_OTHER_SMS)
			return MSG_TYPE_SMS_MWI;
		else 	if (pMsg->threadType.subType == MSG_SYNCML_CP)
			return MSG_TYPE_SMS_SYNCML;
		else 	if (pMsg->threadType.subType == MSG_REJECT_SMS)
			return MSG_TYPE_SMS_REJECT;
		else
			return MSG_TYPE_SMS;
	}
	else if (pMsg->threadType.mainType == MSG_MMS_TYPE)
	{
		if (pMsg->threadType.subType == MSG_NOTIFICATIONIND_MMS)
			return MSG_TYPE_MMS_NOTI;
		else if (pMsg->threadType.subType == MSG_SENDREQ_JAVA_MMS)
			return MSG_TYPE_MMS_JAVA;
		else
			return MSG_TYPE_MMS;
	}
	else
		return MSG_TYPE_INVALID;
}


EXPORT_API const char* msg_thread_view_get_data(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return NULL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->threadData;
}


EXPORT_API time_t* msg_thread_view_get_time(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return NULL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return &(pMsg->threadTime);
}


EXPORT_API int msg_thread_view_get_direction(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->direction;
}


EXPORT_API int msg_thread_view_get_contact_id(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->contactId;
}


EXPORT_API int msg_thread_view_get_unread_cnt(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->unreadCnt;
}


EXPORT_API int msg_thread_view_get_sms_cnt(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->smsCnt;
}


EXPORT_API int msg_thread_view_get_mms_cnt(msg_thread_view_t msg_thread)
{
	if (msg_thread == NULL)
	{
		MSG_FATAL("msg_thread is NULL");
		return -EINVAL;
	}

	MSG_THREAD_VIEW_S* pMsg = (MSG_THREAD_VIEW_S*)msg_thread;

	return pMsg->mmsCnt;
}


EXPORT_API int msg_search_message_for_thread_view(MSG_HANDLE_T handle, const char *search_string, MSG_THREAD_VIEW_LIST_S *msg_thread_view_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_search_message(MSG_HANDLE_T handle, const MSG_SEARCH_CONDITION_S *msg_search_conditions, int offset, int limit, MSG_LIST_S *msg_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || msg_search_conditions == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->searchMessage(msg_search_conditions, offset, limit, msg_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API int msg_release_message_list(MSG_LIST_S *msg_list)
{
	if (msg_list == NULL)
	{
		MSG_FATAL("msg_list is NULL");
		return MSG_ERR_NULL_POINTER;
	}


	// Memory Free
	if (msg_list->msgInfo!= NULL)
	{
		if(msg_list->nCount > 0)
		{
			for(int i=0; i<msg_list->nCount; i++)
				msg_release_message(&(msg_list->msgInfo[i]));
		}

		delete [] msg_list->msgInfo;
		msg_list->msgInfo = NULL;
	}

	msg_list->nCount = 0;

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_msgid_list(MSG_HANDLE_T handle, MSG_REFERENCE_ID_T ref_id, MSG_MSGID_LIST_S *msg_msgid_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getMsgIdList(ref_id, msg_msgid_list);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}


EXPORT_API void msg_release_msgid_list(MSG_MSGID_LIST_S *msg_msgid_list)
{
	if (msg_msgid_list == NULL)
	{
		return;
	}

	// Memory Free
	if(msg_msgid_list->msgIdList != NULL)
	{
		//free peer info list
		delete [] msg_msgid_list->msgIdList;
		msg_msgid_list->msgIdList = NULL;
	}

	msg_msgid_list->nCount = 0;
}


EXPORT_API int msg_get_reject_msg_list(MSG_HANDLE_T handle, const char *phone_num, MSG_REJECT_MSG_LIST_S *msg_reject_msg_list)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API void msg_release_reject_msg_list(MSG_REJECT_MSG_LIST_S *msg_reject_msg_list)
{
	if (msg_reject_msg_list == NULL)
	{
		return;
	}

	// Memory Free
	if(msg_reject_msg_list->rejectMsgInfo != NULL)
	{
		//free peer info list
		delete [] msg_reject_msg_list->rejectMsgInfo;
		msg_reject_msg_list->rejectMsgInfo = NULL;
	}

	msg_reject_msg_list->nCount = 0;
}


EXPORT_API int msg_reg_storage_change_callback(MSG_HANDLE_T handle, msg_storage_change_cb cb, void *user_param)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

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


EXPORT_API int msg_get_report_status(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, MSG_REPORT_STATUS_INFO_S *report_status)
{
	MSG_ERROR_T err =  MSG_SUCCESS;

	if (handle == NULL || msg_id < 1)
	{
		return -EINVAL;
	}

	MsgHandle* pHandle = (MsgHandle*)handle;

	try
	{
		err = pHandle->getReportStatus(msg_id, report_status);
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
		return MSG_ERR_STORAGE_ERROR;
	}

	return err;
}

