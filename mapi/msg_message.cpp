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

#include "MsgTypes.h"
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"
#include "MsgMmsMessage.h"
#include "MsgSerialize.h"
#include "msg.h"
#include "msg_private.h"

msg_struct_s *__msg_message_create_address_struct()
{
	msg_struct_s *msg_struct = new msg_struct_s;
	bzero(msg_struct, sizeof(msg_struct_s));

	msg_struct->type = MSG_STRUCT_ADDRESS_INFO;
	msg_struct->data = new MSG_ADDRESS_INFO_S;
	memset(msg_struct->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	return msg_struct;
}

void __msg_message_release_address_struct(gpointer data)
{
	if (data == NULL)
		return;

	msg_struct_s *addr_struct = (msg_struct_s *)data;
	MSG_ADDRESS_INFO_S *addrInfo = (MSG_ADDRESS_INFO_S *)addr_struct->data;

	delete (MSG_ADDRESS_INFO_S *)addrInfo;

	addr_struct->data = NULL;

	delete addr_struct;
}


void msg_message_create_struct(msg_struct_s *msg_struct)
{
	MSG_MESSAGE_HIDDEN_S *msg = (MSG_MESSAGE_HIDDEN_S *)new MSG_MESSAGE_HIDDEN_S;
	memset(msg, 0x00, sizeof(MSG_MESSAGE_HIDDEN_S));

	/* set default value to message*/
	msg->msgId = 0;
	msg->threadId = 0;
	msg->folderId = MSG_DRAFT_ID;
	msg->mainType= MSG_SMS_TYPE;
	msg->subType = MSG_NORMAL_SMS;
	msg->classType = MSG_CLASS_NONE;
	msg->storageId = MSG_STORAGE_PHONE;
	time_t curTime = time(NULL);
	msg->displayTime = curTime;
	msg->networkStatus = MSG_NETWORK_NOT_SEND;
	msg->encodeType = MSG_ENCODE_AUTO;
	msg->bRead = false;
	msg->bProtected = false;
	msg->bBackup = false;
	msg->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msg->direction = MSG_DIRECTION_TYPE_MO;
	msg->bPortValid = false;
	msg->dataSize = 0;
	msg->pData = NULL;
	msg->pMmsData = NULL;
	msg->mmsDataSize = 0;
	msg->simIndex = MSG_SIM_SLOT_ID_1;

	/* Allocate memory for address list of message */
	msg_struct_list_s *addr_list = (msg_struct_list_s *)new msg_struct_list_s;

	addr_list->nCount = 0;
	addr_list->msg_struct_info = (msg_struct_t *)calloc(MAX_TO_ADDRESS_CNT, sizeof(msg_struct_s *));

	if (addr_list->msg_struct_info != NULL) {
		msg_struct_s *pTmp = NULL;

		for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
			addr_list->msg_struct_info[i] = (msg_struct_t)new msg_struct_s;
			pTmp = (msg_struct_s *)addr_list->msg_struct_info[i];
			memset(pTmp, 0x00, sizeof(msg_struct_s));
			pTmp->type = MSG_STRUCT_ADDRESS_INFO;
			pTmp->data = new MSG_ADDRESS_INFO_S;
			memset(pTmp->data, 0x00, sizeof(MSG_ADDRESS_INFO_S));
		}
	}

	msg->addr_list = addr_list;

	msg->addressList = NULL;

	msg_struct->data = (int *)msg;
}

int msg_message_release(msg_struct_s **msg_struct)
{
	MSG_MESSAGE_HIDDEN_S *msg = (MSG_MESSAGE_HIDDEN_S *)(*msg_struct)->data;

	if (msg->pData) {
		delete [] static_cast<char*>(msg->pData);
		msg->pData = NULL;
	}

	if (msg->pMmsData) {
		delete [] static_cast<char*>(msg->pMmsData);
		msg->pMmsData = NULL;
		msg->mmsDataSize = 0;
	}

	/* Memory Free */
	if (msg->addr_list != NULL) {
		for (int i = 0; i < MAX_TO_ADDRESS_CNT; i++) {
			msg_struct_s * addrInfo = (msg_struct_s *)msg->addr_list->msg_struct_info[i];
			delete (MSG_ADDRESS_INFO_S *)addrInfo->data;
			addrInfo->data = NULL;
			delete (msg_struct_s *)msg->addr_list->msg_struct_info[i];
			msg->addr_list->msg_struct_info[i] = NULL;
		}

		g_free(msg->addr_list->msg_struct_info);

		delete msg->addr_list;
		msg->addr_list = NULL;
	}

	/* Free GList *addressList */
	if (msg->addressList) {
		g_list_free_full(msg->addressList, __msg_message_release_address_struct);
		msg->addressList = NULL;
	}

	delete msg;
	(*msg_struct)->data = NULL;

	delete (msg_struct_s *)*msg_struct;
	*msg_struct = NULL;

	return MSG_SUCCESS;
}

int msg_message_get_int_value(void *data, int field, int *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_ID_INT :
		*value = msg_data->msgId;
		break;
	case MSG_MESSAGE_THREAD_ID_INT :
		*value = msg_data->threadId;
		break;
	case MSG_MESSAGE_FOLDER_ID_INT :
		*value = msg_data->folderId;
		break;
	case MSG_MESSAGE_TYPE_INT :
	{
		if (msg_data->mainType == MSG_SMS_TYPE) {
			switch (msg_data->subType) {
				case MSG_CB_SMS :
					*value = MSG_TYPE_SMS_CB;
					break;
				case MSG_JAVACB_SMS :
					*value = MSG_TYPE_SMS_JAVACB;
					break;
				case MSG_WAP_SI_SMS :
				case MSG_WAP_SL_SMS :
					*value = MSG_TYPE_SMS_WAPPUSH;
					break;
				case MSG_MWI_VOICE_SMS :
				case MSG_MWI_FAX_SMS :
				case MSG_MWI_EMAIL_SMS :
				case MSG_MWI_OTHER_SMS :
					*value = MSG_TYPE_SMS_MWI;
					break;
				case MSG_SYNCML_CP :
					*value = MSG_TYPE_SMS_SYNCML;
					break;
				case MSG_REJECT_SMS :
					*value = MSG_TYPE_SMS_REJECT;
					break;
				case MSG_ETWS_SMS :
					*value = MSG_TYPE_SMS_ETWS_PRIMARY;
					break;
				case MSG_CMAS_PRESIDENTIAL :
					*value = MSG_TYPE_SMS_CMAS_PRESIDENTIAL;
					break;
				case MSG_CMAS_EXTREME :
					*value = MSG_TYPE_SMS_CMAS_EXTREME;
					break;
				case MSG_CMAS_SEVERE :
					*value = MSG_TYPE_SMS_CMAS_SEVERE;
					break;
				case MSG_CMAS_AMBER :
					*value = MSG_TYPE_SMS_CMAS_AMBER;
					break;
				case MSG_CMAS_TEST :
					*value = MSG_TYPE_SMS_CMAS_TEST;
					break;
				case MSG_CMAS_EXERCISE :
					*value = MSG_TYPE_SMS_CMAS_EXERCISE;
					break;
				case MSG_CMAS_OPERATOR_DEFINED :
					*value = MSG_TYPE_SMS_CMAS_OPERATOR_DEFINED;
					break;
				default :
					*value = MSG_TYPE_SMS;
					break;
			}
		}
		else if (msg_data->mainType == MSG_MMS_TYPE) {
			if (msg_data->subType == MSG_NOTIFICATIONIND_MMS)
				*value = MSG_TYPE_MMS_NOTI;
			else if (msg_data->subType == MSG_SENDREQ_JAVA_MMS)
				*value = MSG_TYPE_MMS_JAVA;
			else
				*value = MSG_TYPE_MMS;
		} else {
			*value = MSG_TYPE_INVALID;
		}

		break;
	}
	case MSG_MESSAGE_CLASS_TYPE_INT :
		*value = msg_data->classType;
		break;
	case MSG_MESSAGE_STORAGE_ID_INT :
		*value = msg_data->storageId;
		break;
	case MSG_MESSAGE_DISPLAY_TIME_INT :
		*value = msg_data->displayTime;
		break;
	case MSG_MESSAGE_NETWORK_STATUS_INT :
		*value = msg_data->networkStatus;
		break;
	case MSG_MESSAGE_ENCODE_TYPE_INT :
		*value = msg_data->encodeType;
		break;
	case MSG_MESSAGE_PRIORITY_INT :
		*value = msg_data->priority;
		break;
	case MSG_MESSAGE_DIRECTION_INT :
		*value = msg_data->direction;
		break;
	case MSG_MESSAGE_DEST_PORT_INT :
		*value = msg_data->dstPort;
		break;
	case MSG_MESSAGE_SRC_PORT_INT :
		*value = msg_data->srcPort;
		break;
	case MSG_MESSAGE_ATTACH_COUNT_INT :
		*value = msg_data->attachCount;
		break;
	case MSG_MESSAGE_DATA_SIZE_INT :
		*value = msg_data->dataSize;
		break;
	case MSG_MESSAGE_SIM_INDEX_INT :
		*value = msg_data->simIndex;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_get_bool_value(void *data, int field, bool *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_READ_BOOL :
		*value = msg_data->bRead;
		break;
	case MSG_MESSAGE_PROTECTED_BOOL :
		*value = msg_data->bProtected;
		break;
	case MSG_MESSAGE_BACKUP_BOOL :
		*value = msg_data->bBackup;
		break;
	case MSG_MESSAGE_PORT_VALID_BOOL :
		*value = msg_data->bPortValid;
		break;
	case MSG_MESSAGE_REPLACE_BOOL :
	{
		if (msg_data->subType >= MSG_REPLACE_TYPE1_SMS && msg_data->subType <= MSG_REPLACE_TYPE7_SMS)
			*value = true;
		else
			*value = false;
		break;
	}
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_get_str_value(void *data, int field, char *value, int size)
{
	if (!data || !value)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_REPLY_ADDR_STR :
		strncpy(value, msg_data->replyAddress, size);
		break;
	case MSG_MESSAGE_SUBJECT_STR :
		strncpy(value, msg_data->subject, size);
		break;
	case MSG_MESSAGE_SMS_DATA_STR :
	case MSG_MESSAGE_MMS_TEXT_STR :
		if (msg_data->pData)
		{
			if (msg_data->mainType == MSG_SMS_TYPE) {
				int data_len = 0;
				((size_t)size >= msg_data->dataSize)? (data_len = msg_data->dataSize) : data_len = size;
				memset(value, 0, size);
				memcpy(value, msg_data->pData, data_len);
			} else if (msg_data->mainType == MSG_MMS_TYPE) {
				memset(value, 0, size);
				strncpy(value, (char *)msg_data->pData, size);
			}
		}
		break;

	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_get_struct_hnd(void *data, int field, void **value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	switch (field) {
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_get_list_hnd(void *data, int field, void **value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_ADDR_LIST_STRUCT :
		*value = (void *)msg_data->addr_list;
		break;
	case MSG_MESSAGE_ADDR_LIST_HND :
		*value = (msg_list_handle_t)msg_data->addressList;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_set_int_value(void *data, int field, int value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_ID_INT :
		msg_data->msgId = value;
		break;
	case MSG_MESSAGE_THREAD_ID_INT :
		msg_data->threadId = value;
		break;
	case MSG_MESSAGE_FOLDER_ID_INT :
		msg_data->folderId = value;
		break;
	case MSG_MESSAGE_TYPE_INT :
	{
		if (value == MSG_TYPE_SMS) {
			msg_data->mainType = MSG_SMS_TYPE;
			msg_data->subType = MSG_NORMAL_SMS;
		} else if (value == MSG_TYPE_MMS) {
			msg_data->mainType = MSG_MMS_TYPE;
			msg_data->subType = MSG_SENDREQ_MMS;
		} else if (value == MSG_TYPE_MMS_JAVA) {
			msg_data->mainType = MSG_MMS_TYPE;
			msg_data->subType = MSG_SENDREQ_JAVA_MMS;
		} else if (value == MSG_TYPE_SMS_SYNCML) {
			msg_data->mainType = MSG_SMS_TYPE;
			msg_data->subType = MSG_SYNCML_CP;
		} else if (value == MSG_TYPE_SMS_REJECT) {
			msg_data->mainType = MSG_SMS_TYPE;
			msg_data->subType = MSG_REJECT_SMS;
		} else if (value == MSG_TYPE_SMS_ETWS_PRIMARY) {
			msg_data->mainType = MSG_SMS_TYPE;
			msg_data->subType = MSG_ETWS_SMS;
		} else {
			ret = MSG_ERR_INVALID_PARAMETER;
		}
		break;
	}
	case MSG_MESSAGE_CLASS_TYPE_INT :
		msg_data->classType = value;
		break;
	case MSG_MESSAGE_STORAGE_ID_INT :
		msg_data->storageId = value;
		break;
	case MSG_MESSAGE_DISPLAY_TIME_INT :
		msg_data->displayTime = value;
		break;
	case MSG_MESSAGE_NETWORK_STATUS_INT :
		msg_data->networkStatus = value;
		break;
	case MSG_MESSAGE_ENCODE_TYPE_INT :
		msg_data->encodeType = value;
		break;
	case MSG_MESSAGE_PRIORITY_INT :
		msg_data->priority = value;
		break;
	case MSG_MESSAGE_DIRECTION_INT :
		msg_data->direction = value;
		break;
	case MSG_MESSAGE_DEST_PORT_INT :
		msg_data->dstPort = value;
		break;
	case MSG_MESSAGE_SRC_PORT_INT :
		msg_data->srcPort = value;
		break;
	case MSG_MESSAGE_ATTACH_COUNT_INT :
		 msg_data->attachCount = value;
		break;
	case MSG_MESSAGE_DATA_SIZE_INT :
		msg_data->dataSize = value;
		break;
	case MSG_MESSAGE_SIM_INDEX_INT :
		msg_data->simIndex = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_set_bool_value(void *data, int field, bool value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_READ_BOOL :
		msg_data->bRead = value;
		break;
	case MSG_MESSAGE_PROTECTED_BOOL :
		msg_data->bProtected = value;
		break;
	case MSG_MESSAGE_BACKUP_BOOL :
		msg_data->bBackup = value;
		break;
	case MSG_MESSAGE_PORT_VALID_BOOL :
		msg_data->bPortValid = value;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_set_str_value(void *data, int field, char *value, int size)
{
	if (!data || !value)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)data;

	switch (field) {
	case MSG_MESSAGE_REPLY_ADDR_STR :
		snprintf(msg_data->replyAddress, sizeof(msg_data->replyAddress), "%s", value);
		break;
	case MSG_MESSAGE_SUBJECT_STR :
		snprintf(msg_data->subject, sizeof(msg_data->subject), "%s",value);
		break;
	case MSG_MESSAGE_SMS_DATA_STR :
	{
		if (msg_data->pData)
			delete [] static_cast<char*>(msg_data->pData);

		msg_data->dataSize = size;
		msg_data->pData = (void*)new char[msg_data->dataSize+1];
		memcpy((char *)msg_data->pData, value, msg_data->dataSize);
		((char*) msg_data->pData)[msg_data->dataSize] = '\0';
	}
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_message_set_struct_hnd(void *data, int field, void *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	switch (field) {
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

void msg_message_copy_message(MSG_MESSAGE_HIDDEN_S *pSrc, MSG_MESSAGE_HIDDEN_S *pDst)
{

	pDst->msgId = pSrc->msgId;
	pDst->threadId = pSrc->msgId;
	pDst->folderId = pSrc->folderId;
	pDst->mainType = pSrc->mainType;
	pDst->subType = pSrc->subType;
	pDst->classType = pSrc->classType;
	pDst->storageId = pSrc->storageId;
	pDst->displayTime = pSrc->displayTime;
	pDst->networkStatus = pSrc->networkStatus;
	pDst->encodeType = pSrc->encodeType;
	pDst->bRead = pSrc->bRead;
	pDst->bProtected = pSrc->bProtected;
	pDst->bBackup = pSrc->bBackup;
	pDst->priority = pSrc->priority;
	pDst->direction = pSrc->direction;
	pDst->bPortValid = pSrc->bPortValid;
	pDst->dataSize = pSrc->dataSize;
	pDst->mmsDataSize = pSrc->mmsDataSize;
	pDst->simIndex = pSrc->simIndex;
	memcpy(pDst->subject, pSrc->subject, sizeof(pDst->subject));

	if (pSrc->pMmsData && pSrc->mmsDataSize) {
		pDst->pMmsData = new char[pSrc->mmsDataSize];
		memcpy(pDst->pMmsData, pSrc->pMmsData, pSrc->mmsDataSize);
	}

	if (pSrc->dataSize && pSrc->pData) {
		int data_len = strlen((const char *)pSrc->pData);
		pDst->pData = new char[data_len + 1];
		memset(pDst->pData, 0x00, data_len + 1);
		strncpy((char *)pDst->pData, (const char *)pSrc->pData, data_len);
	}

	msg_struct_list_s *src_addrlist = pSrc->addr_list;
	msg_struct_list_s *dst_addrlist = pDst->addr_list;
	dst_addrlist->nCount = src_addrlist->nCount;

	for (int i = 0; i < MAX_TO_ADDRESS_CNT; ++i) {
		msg_struct_s *src_addr = (msg_struct_s *)src_addrlist->msg_struct_info[i];
		msg_struct_s *dst_addr = (msg_struct_s *)dst_addrlist->msg_struct_info[i];
		memcpy(dst_addr->data, src_addr->data, sizeof(MSG_ADDRESS_INFO_S));
	}

	if (strlen(pSrc->thumbPath) > 0) {
		memset(pDst->thumbPath, 0x00, sizeof(pDst->thumbPath));
		memcpy(pDst->thumbPath, pSrc->thumbPath, sizeof(pDst->thumbPath));
	}

	for (unsigned int i = 0; i < g_list_length(pSrc->addressList); i++) {
		msg_struct_s *addr_struct = __msg_message_create_address_struct();
		MSG_ADDRESS_INFO_S *addrInfo = (MSG_ADDRESS_INFO_S *)addr_struct->data;

		msg_struct_s *addr_info = (msg_struct_s *)g_list_nth_data(pSrc->addressList,(guint)i);
		MSG_ADDRESS_INFO_S *address = (MSG_ADDRESS_INFO_S *)addr_info->data;

		addrInfo->addressType = address->addressType;
		addrInfo->recipientType = address->recipientType;
		addrInfo->contactId = address->contactId;
		strncpy(addrInfo->addressVal, address->addressVal, MAX_ADDRESS_VAL_LEN);
		strncpy(addrInfo->displayName, address->displayName, MAX_DISPLAY_NAME_LEN);
		addrInfo->displayName[MAX_DISPLAY_NAME_LEN] = '\0';

		pDst->addressList = g_list_append(pDst->addressList, addr_struct);
	}
}

int msg_cb_message_get_int_value(void *data, int field, int *value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_MSG_S *cb_msg = (MSG_CB_MSG_S *)data;

	*value = 0;

	switch (field) {
	case MSG_CB_MSG_TYPE_INT :
		{
			switch (cb_msg->type) {
			case MSG_ETWS_SMS :
				*value = MSG_TYPE_SMS_ETWS_PRIMARY;
				break;
			case MSG_CB_SMS:
				*value = ((cb_msg->messageId & 0xFFF8) == 0x1100 ) ? MSG_TYPE_SMS_ETWS_SECONDARY : MSG_TYPE_SMS_CB;
				break;
			case MSG_CMAS_PRESIDENTIAL :
				*value = MSG_TYPE_SMS_CMAS_PRESIDENTIAL;
				break;
			case MSG_CMAS_EXTREME :
				*value = MSG_TYPE_SMS_CMAS_EXTREME;
				break;
			case MSG_CMAS_SEVERE :
				*value = MSG_TYPE_SMS_CMAS_SEVERE;
				break;
			case MSG_CMAS_AMBER :
				*value = MSG_TYPE_SMS_CMAS_AMBER;
				break;
			case MSG_CMAS_TEST :
				*value = MSG_TYPE_SMS_CMAS_TEST;
				break;
			case MSG_CMAS_EXERCISE :
				*value = MSG_TYPE_SMS_CMAS_EXERCISE;
				break;
			case MSG_CMAS_OPERATOR_DEFINED :
				*value = MSG_TYPE_SMS_CMAS_OPERATOR_DEFINED;
				break;
			default :
				ret = MSG_ERR_UNKNOWN;
			}
		}
		break;
	case MSG_CB_MSG_RECV_TIME_INT :
		*value = cb_msg->receivedTime;
		break;
	case MSG_CB_MSG_SERIAL_NUM_INT :
		*value = cb_msg->serialNum;
		break;
	case MSG_CB_MSG_MSG_ID_INT :
		*value = cb_msg->messageId;
		break;
	case MSG_CB_MSG_DCS_INT :
		*value = (int)cb_msg->dcs;
		break;
	case MSG_CB_MSG_CB_TEXT_LEN_INT :
		*value = cb_msg->cbTextLen;
		break;
	case MSG_CB_MSG_ETWS_WARNING_TYPE_INT :
		*value = cb_msg->etwsWarningType;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

int msg_cb_message_get_str_value(void *data, int field, char *value, int size)
{
	if (!data || !value)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CB_MSG_S *cb_msg = (MSG_CB_MSG_S *)data;

	switch (field) {
	case MSG_CB_MSG_CB_TEXT_STR:
	{
		int	copylen = 0;
		copylen = (size > cb_msg->cbTextLen) ? cb_msg->cbTextLen : size - 1;
		memcpy (value, cb_msg->cbText, copylen);
		value[copylen] = '\0';
	}
	break;
	case MSG_CB_MSG_LANGUAGE_TYPE_STR:
	{
		int	copylen = 0;
		copylen = ((size_t)size > strlen((const char*)cb_msg->language_type)) ? strlen((const char*)cb_msg->language_type) : size - 1;
		memcpy (value, cb_msg->language_type, copylen);
		value[copylen] = '\0';
	}
	break;
	case MSG_CB_MSG_ETWS_WARNING_SECU_INFO_STR:
	{
		if ((size_t)size < sizeof(cb_msg->etwsWarningSecurityInfo))
			ret = MSG_ERR_INVALID_PARAMETER;
		else
			memcpy (value, cb_msg->etwsWarningSecurityInfo, sizeof(cb_msg->etwsWarningSecurityInfo));
	}
	break;

	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}



EXPORT_API int msg_get_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);
	/* TODO :: check message type is MMS */
	int ret = MSG_SUCCESS;

	if (msg_struct_handle == NULL || mms_struct_handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;
	msg_struct_s *mms_struct = (msg_struct_s *)mms_struct_handle;

	MSG_TYPE_CHECK(msg_struct->type, MSG_STRUCT_MESSAGE_INFO);
	MSG_TYPE_CHECK(mms_struct->type, MSG_STRUCT_MMS);

	if (msg_struct->data == NULL || mms_struct->data == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)msg_struct->data;

	if (msg_data->pMmsData == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MMS_DATA_S *mms_data = NULL;

	MsgDeserializeMmsData((char*)msg_data->pMmsData, msg_data->mmsDataSize, &mms_data);

	if (mms_data) {
		convert_to_hidden_mmsdata(mms_data, mms_struct);

		MsgMmsRelease(&mms_data);
	} else {
		return MSG_ERR_INVALID_PARAMETER;
	}

	return ret;
}

EXPORT_API int msg_set_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle)
{
	CHECK_MSG_SUPPORTED(MSG_TELEPHONY_MMS_FEATURE);
	/* TODO :: check message type is MMS */
	int ret = MSG_SUCCESS;

	if (msg_struct_handle == NULL || mms_struct_handle == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;
	msg_struct_s *mms_struct = (msg_struct_s *)mms_struct_handle;

	MSG_TYPE_CHECK(msg_struct->type, MSG_STRUCT_MESSAGE_INFO);
	MSG_TYPE_CHECK(mms_struct->type, MSG_STRUCT_MMS);

	if (msg_struct->data == NULL || mms_struct->data == NULL)
		return MSG_ERR_INVALID_PARAMETER;

	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)msg_struct->data;

	MMS_DATA_S *mms_data = MsgMmsCreate();
	if (mms_data == NULL)
		return MSG_ERR_MEMORY_ERROR;

	convert_from_hidden_mmsdata(mms_struct, mms_data);

	int mmsDataSize = MsgSerializeMms(mms_data, (char **)&msg_data->pMmsData);

	if (mmsDataSize > 0)
		msg_data->mmsDataSize = mmsDataSize;

	/* TODO:: set subtype */

	MsgMmsRelease(&mms_data);

	return ret;
}


int msg_message_list_append(msg_struct_t msg_struct_handle, int field, msg_struct_t *item)
{
	msg_error_t err = MSG_SUCCESS;
	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;
	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)msg_struct->data;

	msg_struct_s *msg_struct_item = NULL;
	switch (field) {
	case MSG_MESSAGE_ADDR_LIST_HND:
	{
		msg_struct_item = __msg_message_create_address_struct();
		msg_data->addressList = g_list_append(msg_data->addressList, msg_struct_item);
		*item = (msg_struct_t)msg_struct_item;
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}


int msg_message_list_clear(msg_struct_t msg_struct_handle, int field)
{
	msg_error_t err = MSG_SUCCESS;
	msg_struct_s *msg_struct = (msg_struct_s *)msg_struct_handle;
	MSG_MESSAGE_HIDDEN_S *msg_data = (MSG_MESSAGE_HIDDEN_S *)msg_struct->data;

	switch (field) {
	case MSG_MESSAGE_ADDR_LIST_HND:
	{
		/* Free GList *addressList */
		if (msg_data->addressList) {
			g_list_free_full(msg_data->addressList, __msg_message_release_address_struct);
			msg_data->addressList = NULL;
		}
	}
	break;
	default :
		err = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return err;
}

int msg_conversation_get_list_hnd(void *data, int field, void **value)
{
	if (!data)
		return MSG_ERR_NULL_POINTER;

	int ret = MSG_SUCCESS;

	MSG_CONVERSATION_VIEW_S *msg_data = (MSG_CONVERSATION_VIEW_S *)data;

	switch (field) {
	case MSG_CONV_MSG_MULTIPART_HND :
		*value = (void *)msg_data->multipart_list;
		break;
	default :
		ret = MSG_ERR_INVALID_PARAMETER;
		break;
	}

	return ret;
}

