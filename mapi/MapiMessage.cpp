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

#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MsgMmsMessage.h"
#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgHandle.h"
#include "MsgException.h"
#include "MapiMessage.h"
#include "MsgInternalTypes.h"
#include "MsgDrmWrapper.h"


static MSG_ERROR_T msg_mms_release_message(MMS_MESSAGE_DATA_S* msg_data);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
EXPORT_API msg_message_t msg_new_message(void)
{
	MSG_MESSAGE_S *msg = new MSG_MESSAGE_S;

	memset(msg, 0x00, sizeof(MSG_MESSAGE_S));

	/* set default value to message*/
	msg->msgId = 0;
	msg->folderId = MSG_DRAFT_ID;
	msg->referenceId = 0;
	msg->msgType.mainType= MSG_SMS_TYPE;
	msg->msgType.subType = MSG_NORMAL_SMS;
	msg->msgType.classType = MSG_CLASS_NONE;
	msg->storageId = MSG_STORAGE_PHONE;
	msg->nAddressCnt = 0;
	time_t curTime = time(NULL);
	msg->displayTime = curTime;
	msg->networkStatus = MSG_NETWORK_NOT_SEND;
	msg->encodeType = MSG_ENCODE_AUTO;
	msg->bRead = false;
	msg->bProtected = false;
	msg->bBackup = false;
	msg->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	msg->direction = MSG_DIRECTION_TYPE_MO;
	msg->msgPort.valid = false;
	msg->dataSize = 0;
	msg->pData = NULL;
	msg->pMmsData = NULL;
	msg->scheduledTime = 0;

	return (msg_message_t) msg;
}


EXPORT_API int msg_release_message(msg_message_t *opq_msg)
{
	if (opq_msg == NULL)
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S* msg = NULL;

	msg = (MSG_MESSAGE_S*)(*opq_msg);

	if (msg != NULL)
	{
		MSG_DEBUG("main type : %d, data size : %d", msg->msgType.mainType, msg->dataSize);

		if (msg->pData)
		{
			delete [] static_cast<char*>(msg->pData);
			msg->pData = NULL;
		}

		if (msg->pMmsData)
		{
			delete [] static_cast<char*>(msg->pMmsData);
			msg->pMmsData = NULL;
		}

		delete (MSG_MESSAGE_S*)(*opq_msg);
		*opq_msg = NULL;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_set_message_id(msg_message_t opq_msg, int msg_id)
{
	if ( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->msgId = msg_id;
	return MSG_SUCCESS;
}


EXPORT_API int msg_get_message_id(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}
	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->msgId;
}


EXPORT_API int msg_set_message_type(msg_message_t opq_msg, MSG_MESSAGE_TYPE_T msg_type)
{
	if (!opq_msg)
	{
		MSG_FATAL("msg or text is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*)opq_msg;

	if (msg_type == MSG_TYPE_SMS)
	{
		msg->msgType.mainType = MSG_SMS_TYPE;
		msg->msgType.subType = MSG_NORMAL_SMS;
	}
	else if (msg_type == MSG_TYPE_MMS)
	{
		msg->msgType.mainType = MSG_MMS_TYPE;
		msg->msgType.subType = MSG_SENDREQ_MMS;
	}
	else if (msg_type == MSG_TYPE_MMS_JAVA)
	{
		msg->msgType.mainType = MSG_MMS_TYPE;
		msg->msgType.subType = MSG_SENDREQ_JAVA_MMS;
	}
	else if (msg_type == MSG_TYPE_SMS_SYNCML)
	{
		msg->msgType.mainType = MSG_SMS_TYPE;
		msg->msgType.subType = MSG_SYNCML_CP;
	}
	else if (msg_type == MSG_TYPE_SMS_REJECT)
	{
		msg->msgType.mainType = MSG_SMS_TYPE;
		msg->msgType.subType = MSG_REJECT_SMS;
	}
	else
	{
		MSG_FATAL("invalid msg_type [%d]", msg_type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_message_type(msg_message_t opq_msg)
{
	if (!opq_msg)
	{
		MSG_FATAL("msg or msg_type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*)opq_msg;

	if (msg->msgType.mainType == MSG_SMS_TYPE)
	{
		if (msg->msgType.subType == MSG_CB_SMS)
			return MSG_TYPE_SMS_CB;
		else 	if (msg->msgType.subType == MSG_JAVACB_SMS)
			return MSG_TYPE_SMS_JAVACB;
		else 	if (msg->msgType.subType == MSG_WAP_SI_SMS || msg->msgType.subType == MSG_WAP_SL_SMS)
			return MSG_TYPE_SMS_WAPPUSH;
		else 	if (msg->msgType.subType == MSG_MWI_VOICE_SMS || msg->msgType.subType == MSG_MWI_FAX_SMS
				|| msg->msgType.subType == MSG_MWI_EMAIL_SMS || msg->msgType.subType == MSG_MWI_OTHER_SMS)
			return MSG_TYPE_SMS_MWI;
		else 	if (msg->msgType.subType == MSG_SYNCML_CP)
			return MSG_TYPE_SMS_SYNCML;
		else 	if (msg->msgType.subType == MSG_REJECT_SMS)
			return MSG_TYPE_SMS_REJECT;
		else
			return MSG_TYPE_SMS;
	}
	else if (msg->msgType.mainType == MSG_MMS_TYPE)
	{
		if (msg->msgType.subType == MSG_NOTIFICATIONIND_MMS)
			return MSG_TYPE_MMS_NOTI;
		else if (msg->msgType.subType == MSG_SENDREQ_JAVA_MMS)
			return MSG_TYPE_MMS_JAVA;
		else
			return MSG_TYPE_MMS;
	}
	else
		return MSG_TYPE_INVALID;
}


EXPORT_API bool msg_is_sms(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or msg_type is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return (msg->msgType.mainType == MSG_SMS_TYPE);
}


EXPORT_API bool msg_is_mms(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or msg_type is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return (msg->msgType.mainType == MSG_MMS_TYPE);
}


EXPORT_API int msg_set_storage_id(msg_message_t opq_msg, MSG_STORAGE_ID_T storage_id)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg or storage is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (storage_id != MSG_STORAGE_PHONE && storage_id != MSG_STORAGE_SIM)
	{
		MSG_FATAL("unsupported storage [%d]", storage_id);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	msg->storageId = storage_id;

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_storage_id(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->storageId;
}


EXPORT_API bool msg_is_in_sim(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or folder is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return (msg->storageId == MSG_STORAGE_SIM);
}


EXPORT_API int msg_set_folder_id(msg_message_t opq_msg, MSG_FOLDER_ID_T folder_id)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg or is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (folder_id < MSG_INBOX_ID || folder_id > MSG_MAX_FOLDER_ID)
	{
		MSG_FATAL("wrong folder ID [%d]", folder_id);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	msg->folderId = folder_id;

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_folder_id(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->folderId;
}


EXPORT_API int msg_reset_address(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	msg->nAddressCnt = 0;

	for (int i=0 ; i < MAX_TO_ADDRESS_CNT ; i++)
	{
		msg->addressList[i].threadId = 0;
		msg->addressList[i].addressType = MSG_ADDRESS_TYPE_UNKNOWN;
		msg->addressList[i].recipientType = MSG_RECIPIENTS_TYPE_UNKNOWN;
		msg->addressList[i].contactId = 0;
		msg->addressList[i].addressVal[0] = '\0';
		msg->addressList[i].displayName[0] = '\0';
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_add_address(msg_message_t opq_msg, const char* phone_num_list, MSG_RECIPIENT_TYPE_T to_type )
{
	if( !opq_msg || !phone_num_list)
	{
		MSG_FATAL("msg or text or type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (to_type < MSG_RECIPIENTS_TYPE_TO || to_type > MSG_RECIPIENTS_TYPE_BCC)
	{
		MSG_FATAL("unsupported recipient type [%d]", to_type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;


	if(msg->msgType.mainType == MSG_SMS_TYPE)
	{
		char trimmed_num[strlen(phone_num_list)+1];
		bzero(trimmed_num, strlen(phone_num_list)+1);

		MSG_ERROR_T retVal = msg_verify_number(phone_num_list, trimmed_num);

		if ( retVal != MSG_SUCCESS )
			return retVal;

		int i = msg->nAddressCnt;
		for( char* cur_num = strtok(trimmed_num,", "); cur_num && (i < MAX_TO_ADDRESS_CNT);
															cur_num = strtok(NULL,", "), i++)
		{
			if (strlen(cur_num) > MAX_ADDRESS_VAL_LEN)
			{
				MSG_DEBUG("Phone number is too long [%s], and sending is skipped", cur_num);
				continue;
			}

			/* fill the destination number in msgReq */
			msg->nAddressCnt++;
			msg->addressList[i].addressType = MSG_ADDRESS_TYPE_PLMN; // telephone number
			msg->addressList[i].recipientType = to_type; // recipient type
			snprintf(msg->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN, "%s", cur_num);
			MSG_DEBUG("SMS AddressVal = [%s]", msg->addressList[i].addressVal);
		}

	}
	else
	{
		char phone_num_list_tmp[strlen(phone_num_list)+1];
		bzero(phone_num_list_tmp, strlen(phone_num_list)+1);

		strncpy(phone_num_list_tmp, phone_num_list, strlen(phone_num_list));

		MSG_ERROR_T retVal = msg_verify_email(phone_num_list_tmp);

		if ( retVal != MSG_SUCCESS )
			return retVal;

		int i = msg->nAddressCnt;
		for( char* cur_num = strtok(phone_num_list_tmp,", "); cur_num && (i < MAX_TO_ADDRESS_CNT);
															cur_num = strtok(NULL,", "), i++)
		{
			if (strlen(cur_num) > MAX_ADDRESS_VAL_LEN)
			{
				MSG_DEBUG("Phone number is too long [%s], and sending is skipped", cur_num);
				continue;
			}

			char trimmed_num[strlen(cur_num)+1];
			bzero(trimmed_num, strlen(cur_num)+1);

			retVal = msg_verify_number(cur_num, trimmed_num);

			if(retVal == MSG_SUCCESS)
			{
				/* fill the destination number in msgReq */
				msg->nAddressCnt++;
				msg->addressList[i].addressType = MSG_ADDRESS_TYPE_PLMN; // telephone number
				msg->addressList[i].recipientType = to_type; // recipient type
				snprintf(msg->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN, "%s", trimmed_num);
				MSG_DEBUG("MMS Number AddressVal = [%s]", msg->addressList[i].addressVal);
			}
			else
			{
				/* fill the destination number in msgReq */
				msg->nAddressCnt++;
				msg->addressList[i].addressType = MSG_ADDRESS_TYPE_EMAIL; // telephone number
				msg->addressList[i].recipientType = to_type; // recipient type
				snprintf(msg->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN, "%s", cur_num);
				MSG_DEBUG("MMS Email AddressVal = [%s]", msg->addressList[i].addressVal);
			}
		}
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_address_count(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or count is NULL");
		return MSG_ERR_NULL_POINTER;
	}
	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->nAddressCnt;
}


EXPORT_API int msg_get_ith_thread_id(msg_message_t opq_msg, int ith)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg or text or type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if( ith < msg->nAddressCnt )
	{
		return msg->addressList[ith].threadId;
	}
	else
	{
		MSG_FATAL("Requested address count %d exceeds max %d in msg", ith, msg->nAddressCnt);
		return MSG_ERR_UNKNOWN;
	}
}


EXPORT_API const char* msg_get_ith_address(msg_message_t opq_msg, int ith)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or addr is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if( ith < msg->nAddressCnt )
	{
		return msg->addressList[ith].addressVal;
	}
	else
	{
		MSG_FATAL("Requested address count %d exceeds max %d in msg", ith, msg->nAddressCnt);
		return NULL;
	}
}


EXPORT_API int msg_get_ith_recipient_type(msg_message_t opq_msg, int ith)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg or text or type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if( ith < msg->nAddressCnt )
	{
		return msg->addressList[ith].recipientType;
	}
	else
	{
		MSG_FATAL("Requested address count %d exceeds max %d in msg", ith, msg->nAddressCnt);
		return MSG_ERR_UNKNOWN;
	}
}


EXPORT_API const char* msg_get_ith_name(msg_message_t opq_msg, int ith)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or addr is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if( ith < msg->nAddressCnt )
	{
		return msg->addressList[ith].displayName;
	}
	else
	{
		MSG_FATAL("Requested address count %d exceeds max %d in msg", ith, msg->nAddressCnt);
		return NULL;
	}
}


EXPORT_API int msg_get_ith_contact_id(msg_message_t opq_msg, int ith)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or addr is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if( ith < msg->nAddressCnt )
	{
		return msg->addressList[ith].contactId;
	}
	else
	{
		MSG_FATAL("Requested address count %d exceeds max %d in msg", ith, msg->nAddressCnt);
		return MSG_ERR_UNKNOWN;
	}
}


EXPORT_API int msg_set_reply_address(msg_message_t opq_msg, const char* phone_num)
{
	if( !opq_msg || !phone_num)
	{
		MSG_FATAL("msg or text or type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (strlen(phone_num) > MAX_PHONE_NUMBER_LEN)
	{
		MSG_DEBUG("Phone number is too long [%s]", phone_num);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	char trimmed_num[strlen(phone_num)+1];
	bzero(trimmed_num, strlen(phone_num)+1);

	MSG_ERROR_T retVal = msg_verify_number(phone_num, trimmed_num);

	if (retVal != MSG_SUCCESS)
		return retVal;

	snprintf(msg->replyAddress, MAX_PHONE_NUMBER_LEN, "%s", phone_num);

	MSG_DEBUG("Reply Address Number : [%s]", msg->replyAddress);

	return MSG_SUCCESS;
}


EXPORT_API int msg_sms_set_message_body(msg_message_t opq_msg, const char* mdata, int size)
{
	if (!opq_msg || !mdata) {
		MSG_FATAL("msg or text is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if (size <= 0) {
		MSG_FATAL("msg size is invalid");
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if(msg->msgType.mainType == MSG_MMS_TYPE) {
/** Temporary added codes for Binary MMS. */
		if (msg->pMmsData)
			delete [] static_cast<char*>(msg->pMmsData);

		msg->dataSize = size;

		msg->pMmsData = (void*)new char[msg->dataSize+1];

		memset(msg->pMmsData, 0x00, msg->dataSize+1);

		memcpy((char *)msg->pMmsData, mdata, msg->dataSize);

/** End. */
	} else if (msg->msgType.mainType == MSG_SMS_TYPE) {
		if (msg->pData)
			delete [] static_cast<char*>(msg->pData);

		msg->dataSize = size;

		msg->pData = (void*)new char[msg->dataSize+1];

		memcpy((char *)msg->pData, mdata, msg->dataSize);

		((char*) msg->pData)[msg->dataSize] = '\0';

	} else {
		return MSG_ERR_INVALID_MSG_TYPE;
	}

	return MSG_SUCCESS;

}


EXPORT_API const char* msg_sms_get_message_body(msg_message_t opq_msg)
{
	if (!opq_msg) {
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;


	if (msg->msgType.mainType == MSG_SMS_TYPE) {

		return (char*) msg->pData;

	} else if (msg->msgType.mainType == MSG_MMS_TYPE) {

		return (char*) msg->pMmsData;

	} else {
		MSG_FATAL("msg type is invalid.");
		return NULL;
	}

}


EXPORT_API const char* msg_mms_get_text_contents(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if(msg->msgType.mainType != MSG_MMS_TYPE)
	{
		MSG_FATAL("msg is not mms type.");
		return NULL;
	}

	return (char*) msg->pData;
}


EXPORT_API int msg_get_message_body_size(msg_message_t opq_msg )
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->dataSize;
}


EXPORT_API int msg_set_subject(msg_message_t opq_msg, const char* subject )
{
	if( !opq_msg || !subject )
	{
		MSG_FATAL("msg or subject is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	snprintf(msg->subject, MAX_SUBJECT_LEN+1,"%s", subject);

	return MSG_SUCCESS;
}


EXPORT_API const char* msg_get_subject(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->subject;
}


EXPORT_API int msg_set_time(msg_message_t opq_msg, time_t msg_time)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	msg->displayTime = msg_time;
	return MSG_SUCCESS;
}


EXPORT_API time_t* msg_get_time(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return &(msg->displayTime);
}

EXPORT_API int msg_set_network_status(msg_message_t opq_msg, MSG_NETWORK_STATUS_T status)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->networkStatus = status;
	return MSG_SUCCESS;
}


EXPORT_API int msg_get_network_status(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or status is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->networkStatus;
}


EXPORT_API int msg_set_encode_type(msg_message_t opq_msg, MSG_ENCODE_TYPE_T encoding_type)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg or encoding_type is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if( encoding_type != MSG_ENCODE_GSM7BIT &&
		encoding_type != MSG_ENCODE_8BIT &&
		encoding_type != MSG_ENCODE_UCS2 &&
		encoding_type != MSG_ENCODE_AUTO )
	{
		MSG_FATAL("Encoding_type has invalid value [%d]", encoding_type);
		return MSG_ERR_INVALID_PARAMETER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->encodeType = encoding_type;
	return MSG_SUCCESS;
}


EXPORT_API int msg_get_encode_type(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->encodeType;
}


EXPORT_API int msg_set_read_status(msg_message_t opq_msg, bool read_flag)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->bRead = read_flag;
	return MSG_SUCCESS;
}


EXPORT_API bool msg_is_read(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->bRead;
}


EXPORT_API int msg_set_protect_status(msg_message_t opq_msg, bool protect_flag)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->bProtected = protect_flag;
	return MSG_SUCCESS;
}


EXPORT_API bool msg_is_protected(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or protected is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->bProtected;
}


EXPORT_API int msg_set_backup_status(msg_message_t opq_msg, bool backup_flag)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->bBackup = backup_flag;
	return MSG_SUCCESS;
}


EXPORT_API bool msg_is_backup(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or protected is NULL");
		return false;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->bBackup;
}


EXPORT_API int msg_set_priority_info(msg_message_t opq_msg, MSG_PRIORITY_TYPE_T priority)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->priority = priority;
	return MSG_SUCCESS;
}


EXPORT_API int msg_get_priority_info(msg_message_t opq_msg)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->priority;
}


EXPORT_API int msg_set_direction_info(msg_message_t opq_msg, MSG_DIRECTION_TYPE_T direction)
{
	if( !opq_msg)
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->direction= direction;
	return MSG_SUCCESS;
}


EXPORT_API int msg_get_direction_info(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return msg->direction;
}


EXPORT_API int msg_set_port(msg_message_t opq_msg, unsigned short dst_port, unsigned short src_port)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if( !dst_port && !src_port )
		return MSG_SUCCESS;

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->msgPort.valid = true;
	msg->msgPort.dstPort = dst_port;
	msg->msgPort.srcPort = src_port;

	return MSG_SUCCESS;
}


EXPORT_API int msg_get_dest_port(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or dst_port or src_port is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	if( msg->msgPort.valid )
		return msg->msgPort.dstPort;
	else
		return 0;
}


EXPORT_API int msg_get_src_port(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg or dst_port or src_port is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	if( msg->msgPort.valid )
		return msg->msgPort.srcPort;
	else
		return 0;
}


EXPORT_API int msg_set_scheduled_time(msg_message_t opq_msg, time_t time_to_send)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	if( !time_to_send )
		return MSG_SUCCESS;

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	msg->scheduledTime = time_to_send;

	return MSG_SUCCESS;
}


EXPORT_API time_t* msg_get_scheduled_time(msg_message_t opq_msg)
{
	if( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;
	return &(msg->scheduledTime);
}


EXPORT_API int msg_get_attachment_count(msg_message_t opq_msg)
{
	if ( !opq_msg )
	{
		MSG_FATAL("msg is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S* msg = (MSG_MESSAGE_S*) opq_msg;

	return msg->attachCount;
}

EXPORT_API const char* msg_get_thumbnail_path(msg_message_t opq_msg)
{
	if (!opq_msg)
	{
		MSG_FATAL("msg is NULL");
		return NULL;
	}

	MSG_MESSAGE_S* pMsg = (MSG_MESSAGE_S*)opq_msg;

	return pMsg->thumbPath;
}


static MSG_ERROR_T msg_mms_release_message(MMS_MESSAGE_DATA_S* msg_data)
{
	_MsgMmsReleasePageList(msg_data);
	_MsgMmsReleaseRegionList(msg_data);
	_MsgMmsReleaseAttachList(msg_data);
	_MsgMmsReleaseTransitionList(msg_data);
	_MsgMmsReleaseMetaList(msg_data);
	return MSG_SUCCESS;
}


EXPORT_API int msg_mms_set_message_body(msg_message_t opq_msg, const MMS_MESSAGE_DATA_S *msg_data)
{
	if (!opq_msg || !msg_data) {
		MSG_FATAL("msg or msg_data is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S* msg = (MSG_MESSAGE_S*) opq_msg;

	if (msg->msgType.mainType != MSG_MMS_TYPE) {
		MSG_FATAL("msg is not mms type.");
		return MSG_ERR_INVALID_MSG_TYPE;
	}

	if (msg->pMmsData) {
		if (msg->dataSize > 0)
			delete[] ((char*) msg->pMmsData);
	}

	msg->dataSize = 0;

	msg->pMmsData = _MsgMmsSerializeMessageData(msg_data, &(msg->dataSize));

	MSG_DEBUG("msg->dataSize : [%d]", msg->dataSize);

	if (msg->dataSize <= 0) {
		MSG_FATAL("msg size is invalid");
		return MSG_ERR_INVALID_PARAMETER;
	}

	return MSG_SUCCESS;
}


EXPORT_API int msg_mms_get_message_body(msg_message_t opq_msg, MMS_MESSAGE_DATA_S *body )
{
	if (!opq_msg || !body) {
		MSG_FATAL("msg or body is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	MSG_MESSAGE_S *msg = (MSG_MESSAGE_S*) opq_msg;

	if (msg->msgType.mainType != MSG_MMS_TYPE) {
		MSG_FATAL("msg is not mms type.");
		return MSG_ERR_INVALID_MSG_TYPE;
	}

	if (msg->pMmsData == NULL) {
		MSG_FATAL("msg->pMmsData is NULL");
		return MSG_ERR_NULL_POINTER;
	}

	_MsgMmsDeserializeMessageData(body, (char*)msg->pMmsData);

	return MSG_SUCCESS;
}


EXPORT_API MMS_PAGE_S* msg_mms_add_page(MMS_MESSAGE_DATA_S *msg_data, const int duration)
{
	if(msg_data == NULL)
		return NULL;

	MMS_PAGE_S* page = (MMS_PAGE_S*)calloc(sizeof(MMS_PAGE_S), 1);
	page->nDur = duration;

	_MsgMmsAddPage(msg_data, page);

	return page;
}


EXPORT_API MMS_SMIL_REGION* msg_mms_add_region(MMS_MESSAGE_DATA_S *msg_data, const char* szID, const int x, const int y, const int width, const int height, const int bgcolor)
{
	if(msg_data == NULL || szID == NULL)
		return NULL;

	MMS_SMIL_REGION* region = (MMS_SMIL_REGION *)calloc( sizeof(MMS_SMIL_REGION), 1 );

	strncpy(region->szID, szID, MAX_SMIL_REGION_ID-1);
	region->bgColor = bgcolor;
	region->nLeft.value = x;
	region->nLeft.bUnitPercent = true;
	region->nTop.value = y;
	region->nTop.bUnitPercent = true;
	region->width.value = width;
	region->width.bUnitPercent = true;
	region->height.value = height;
	region->height.bUnitPercent = true;
	region->fit = MMSUI_IMAGE_REGION_FIT_MEET;

	_MsgMmsAddRegion(msg_data, region);

	return region;
}


EXPORT_API MMS_MEDIA_S* msg_mms_add_media(MMS_PAGE_S *page, const MmsSmilMediaType mediatype, const char* regionid, char* filepath)
{
	if(page == NULL || filepath == NULL)
		return NULL;

	if (MsgDrmIsDrmFile(filepath)) {
		MSG_DRM_TYPE drm_type = MSG_DRM_NONE;

		if (MsgDrmGetDrmType(filepath, &drm_type)) {
			if (drm_type == MSG_DRM_FORWARD_LOCK || drm_type == MSG_DRM_COMBINED_DELIVERY) {
				MSG_FATAL("file is a FL content");
				return NULL;
			}
		}
	}

	MMS_MEDIA_S* media = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
	char *filename = NULL;

	media->mediatype = mediatype;

	if(regionid)
		strncpy(media->regionId, regionid, MAX_SMIL_REGION_ID-1);

	strncpy(media->szFilePath, filepath, MSG_FILEPATH_LEN_MAX-1);

	filename = strrchr(filepath, '/');
	strncpy(media->szFileName, filename + 1, MSG_FILENAME_LEN_MAX-1);
	strncpy(media->szContentID, filename + 1, MSG_MSG_ID_LEN);

	_MsgMmsAddMedia(page, media);

	return media;
}


EXPORT_API MMS_ATTACH_S* msg_mms_add_attachment(MMS_MESSAGE_DATA_S *msg_data, char *filepath)
{
	if(msg_data == NULL || filepath == NULL)
		return NULL;

	if (MsgDrmIsDrmFile(filepath)) {
		MSG_DRM_TYPE drm_type = MSG_DRM_NONE;

		if (MsgDrmGetDrmType(filepath, &drm_type)) {
			if (drm_type == MSG_DRM_FORWARD_LOCK || drm_type == MSG_DRM_COMBINED_DELIVERY) {
				MSG_FATAL("file is a FL content");
				return NULL;
			}
		}
	}

	MMS_ATTACH_S* attach = (MMS_ATTACH_S*)calloc(sizeof(MMS_ATTACH_S), 1);
	char *filename = NULL;

	attach->mediatype = MIME_UNKNOWN;
	attach->fileSize = -1;
	strncpy(attach->szFilePath, filepath, MSG_FILEPATH_LEN_MAX-1);
	filename = strrchr(filepath, '/');
	strncpy(attach->szFileName, filename + 1, MSG_FILENAME_LEN_MAX-1);

	_MsgMmsAddAttachment(msg_data, attach);

	return attach;
}


EXPORT_API int msg_mms_add_transition(MMS_MESSAGE_DATA_S *msg_data, MMS_SMIL_TRANSITION *transition)
{
	if(msg_data == NULL || transition == NULL)
		return MSG_ERR_NULL_POINTER;

	MMS_SMIL_TRANSITION *pTransition = (MMS_SMIL_TRANSITION *)calloc(sizeof(MMS_SMIL_TRANSITION), 1);

	if (transition->szID[0] != '0')
		strncpy(pTransition->szID, transition->szID, MAX_SMIL_TRANSITION_ID);

	pTransition->nType = transition->nType;
	pTransition->nSubType = transition->nSubType;
	pTransition->nDur = transition->nDur;

	_MsgMmsAddTransition(msg_data, pTransition);

	return MSG_SUCCESS;
}


EXPORT_API int msg_mms_add_meta(MMS_MESSAGE_DATA_S *msg_data, MMS_SMIL_META *meta)
{
	if(msg_data == NULL || meta == NULL)
		return MSG_ERR_NULL_POINTER;

	MMS_SMIL_META *pMeta = (MMS_SMIL_META *)calloc(sizeof(MMS_SMIL_META), 1);

	if (meta->szID[0] != '0')
		strncpy(pMeta->szID, meta->szID, MAX_SMIL_TRANSITION_ID);

	if (meta->szName[0] != '0')
		strncpy(pMeta->szName, meta->szName, MAX_SMIL_META_NAME);

	if (meta->szContent[0] != '0')
		strncpy(pMeta->szContent, meta->szContent, MAX_SMIL_META_NAME);

	_MsgMmsAddMeta(msg_data, pMeta);

	return MSG_SUCCESS;
}


EXPORT_API MMS_PAGE_S *msg_mms_get_page(MMS_MESSAGE_DATA_S *msg_data, int page_idx)
{
	if (msg_data == NULL)
		return NULL;

	if (page_idx < 0)
		return NULL;

	return _MsgMmsGetPage(msg_data, page_idx);
}


EXPORT_API MMS_SMIL_REGION* msg_mms_get_smil_region(MMS_MESSAGE_DATA_S *msg_data, int region_idx)
{
	if (msg_data == NULL)
		return NULL;

	if (region_idx < 0)
		return NULL;

	return _MsgMmsGetSmilRegion(msg_data, region_idx);
}


EXPORT_API MMS_MEDIA_S* msg_mms_get_media(MMS_PAGE_S *page, int media_idx)
{
	if (page == NULL || media_idx < 0)
		return NULL;

	return _MsgMmsGetMedia(page, media_idx);
}


EXPORT_API MMS_ATTACH_S* msg_mms_get_attachment(MMS_MESSAGE_DATA_S *msg_data, int attach_idx)
{
	if (msg_data == NULL)
		return NULL;

	if ( attach_idx <0 )
		return NULL;

	return _MsgMmsGetAttachment(msg_data, attach_idx);
}


EXPORT_API MMS_SMIL_TRANSITION* msg_mms_get_transition(MMS_MESSAGE_DATA_S *msg_data, int transition_idx)
{
	if (msg_data == NULL)
		return NULL;

	if (transition_idx < 0)
		return NULL;

	return _MsgMmsGetTransition(msg_data, transition_idx);
}


EXPORT_API MMS_SMIL_META* msg_mms_get_meta(MMS_MESSAGE_DATA_S *msg_data, int meta_idx)
{
	if (msg_data == NULL)
		return NULL;

	if (meta_idx < 0)
		return NULL;

	return _MsgMmsGetMeta(msg_data, meta_idx);
}


EXPORT_API int msg_mms_release_page_list(MMS_MESSAGE_DATA_S *msg_data)
{
	if (msg_data == NULL)
		return MSG_ERR_NULL_POINTER;

	MSG_ERROR_T err = MSG_SUCCESS;

	err = _MsgMmsReleasePageList(msg_data);

	return err;
}


EXPORT_API int msg_mms_release_region_list(MMS_MESSAGE_DATA_S *msg_data)
{
	if (msg_data == NULL)
		return MSG_ERR_NULL_POINTER;

	MSG_ERROR_T err = MSG_SUCCESS;

	err = _MsgMmsReleaseRegionList(msg_data);

	return err;
}


EXPORT_API int msg_mms_release_attachment_list(MMS_MESSAGE_DATA_S *msg_data)
{
	if (msg_data == NULL)
		return MSG_ERR_NULL_POINTER;

	MSG_ERROR_T err = MSG_SUCCESS;

	err = _MsgMmsReleaseAttachList(msg_data);

	return err;
}


EXPORT_API int msg_mms_release_transition_list(MMS_MESSAGE_DATA_S *msg_data)
{
	if (msg_data == NULL)
		return MSG_ERR_NULL_POINTER;

	MSG_ERROR_T err = MSG_SUCCESS;

	err = _MsgMmsReleaseTransitionList(msg_data);

	return err;
}


EXPORT_API int msg_mms_release_meta_list(MMS_MESSAGE_DATA_S *msg_data)
{
	if (msg_data == NULL)
		return MSG_ERR_NULL_POINTER;

	MSG_ERROR_T err = MSG_SUCCESS;

	err = _MsgMmsReleaseMetaList(msg_data);

	return err;
}


EXPORT_API MMS_MESSAGE_DATA_S* msg_mms_create_message(void)
{
	MMS_MESSAGE_DATA_S *mmsmsg = (MMS_MESSAGE_DATA_S *)calloc(sizeof(MMS_MESSAGE_DATA_S), 1);

	return mmsmsg;
}


EXPORT_API MMS_SMIL_ROOTLAYOUT* msg_mms_set_rootlayout(MMS_MESSAGE_DATA_S* msg, const int width, const int height, const int bgcolor)
{
	if (msg == NULL)
		return NULL;

	msg->rootlayout.width.value = width;
	msg->rootlayout.width.bUnitPercent = true;
	msg->rootlayout.height.value = height;
	msg->rootlayout.height.bUnitPercent = true;
	msg->rootlayout.bgColor = bgcolor;

	return (MMS_SMIL_ROOTLAYOUT *)&(msg->rootlayout);
}


EXPORT_API int msg_mms_destroy_message(MMS_MESSAGE_DATA_S* msg)
{
	if (msg == NULL)
		return MSG_ERR_NULL_POINTER;

	msg_mms_release_message(msg);

	free(msg);

	msg = NULL;

	return MSG_SUCCESS;
}

