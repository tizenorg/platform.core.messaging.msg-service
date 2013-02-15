/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

extern "C"
{
	#include <contacts.h>
}

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgContact.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
__thread bool isContactSvcConnected = false;

MsgDbHandler ContactDbHandle;

MsgContactChangeCB cbFunction = NULL;
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static void MsgContactSvcCallback(const char *view_uri, void *user_data)
{
	MSG_DEBUG("Contact Data is Changed!!!");

	MsgSyncContact();

	if (ContactDbHandle.disconnect() != MSG_SUCCESS)
		MSG_DEBUG("DB Disconnect Fail");
}


msg_error_t MsgOpenContactSvc()
{
	int errCode = CONTACTS_ERROR_NONE;

	if (!isContactSvcConnected) {
		errCode = contacts_connect2();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_DEBUG("Connect to Contact Service Success");
			isContactSvcConnected = true;
		} else {
			MSG_DEBUG("Connect to Contact Service Fail [%d]", errCode);
			isContactSvcConnected = false;
			return MSG_ERR_DB_CONNECT;
		}
	} else {
		MSG_DEBUG("Already connected to Contact Service.");
	}

	return MSG_SUCCESS;
}


msg_error_t MsgCloseContactSvc()
{
	int errCode = CONTACTS_ERROR_NONE;

	if (isContactSvcConnected) {
		errCode = contacts_disconnect2();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_DEBUG("Disconnect to Contact Service Success");
		} else {
			MSG_DEBUG("Disconnect to Contact Service Fail [%d]", errCode);
			return MSG_ERR_DB_DISCONNECT;
		}
	}

	return MSG_SUCCESS;
}


msg_error_t MsgInitContactSvc(MsgContactChangeCB cb)
{
	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return err;
	}

	int errCode = CONTACTS_ERROR_NONE;

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		return MSG_ERR_UNKNOWN;
	}

	if (cb != NULL)
		cbFunction = cb;

	// Register callback function
	errCode = contacts_db_add_changed_cb(_contacts_contact._uri, MsgContactSvcCallback, NULL);

	if (errCode == CONTACTS_ERROR_NONE)
		MSG_DEBUG("Register Contact Service Callback");
	else
		MSG_DEBUG("Fail to Register Contact Service Callback [%d]", errCode);

	return MSG_SUCCESS;
}


msg_error_t MsgGetContactInfo(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_CONTACT_INFO_S *pContactInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return err;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		return MSG_ERR_UNKNOWN;
	}

	MSG_DEBUG("Address Type [%d], Address Value [%s]", pAddrInfo->addressType, pAddrInfo->addressVal);

	memset(pContactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN && strlen(pAddrInfo->addressVal) > (MAX_PHONE_NUMBER_LEN+1)) {
		MSG_DEBUG("Phone Number is too long [%s]", pAddrInfo->addressVal);
		return MSG_SUCCESS;
	}

	int ret = 0;
	int index = 0;
	unsigned int count = 0;
	contacts_query_h query = NULL;
	contacts_filter_h filter = NULL;
	contacts_list_h contacts = NULL;

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN || pAddrInfo->addressType == MSG_ADDRESS_TYPE_UNKNOWN) {
		ret = contacts_query_create(_contacts_contact_number._uri, &query);
		ret = contacts_filter_create(_contacts_contact_number._uri, &filter);

		ret = contacts_filter_add_str(filter, _contacts_contact_number.number_filter, CONTACTS_MATCH_EXACTLY, pAddrInfo->addressVal);

	} else if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_EMAIL) {
		ret = contacts_query_create(_contacts_contact_email._uri, &query);
		ret = contacts_filter_create(_contacts_contact_email._uri, &filter);

		ret = contacts_filter_add_str(filter, _contacts_contact_email.email, CONTACTS_MATCH_EXACTLY, pAddrInfo->addressVal);

	} else {
		MSG_DEBUG("Invalid pAddrInfo->addressType.");
		return MSG_SUCCESS;
	}

	ret = contacts_query_set_filter(query, filter);
	ret = contacts_db_get_records_with_query(query, 0, 1, &contacts);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_records_with_query() Error [%d]", ret);
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		return MSG_SUCCESS;
	}

	ret = contacts_list_get_count(contacts, &count);

	if (count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("No Serach Data from Contact Service.");
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		return MSG_SUCCESS;
	}

	contacts_record_h contact = NULL;

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN || pAddrInfo->addressType == MSG_ADDRESS_TYPE_UNKNOWN) {
		contacts_record_h number = NULL;

		ret = contacts_list_get_current_record_p(contacts, &number);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return MSG_SUCCESS;
		}

		ret = contacts_record_get_int(number, _contacts_contact_number.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			contacts_record_destroy(number, true);
			return MSG_SUCCESS;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			contacts_record_destroy(contact, true);
			contacts_record_destroy(number, true);
			return MSG_SUCCESS;
		}
	} else if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_EMAIL) {
		contacts_record_h email = NULL;

		ret = contacts_list_get_current_record_p(contacts, &email);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return MSG_SUCCESS;
		}

		ret = contacts_record_get_int(email, _contacts_contact_email.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			contacts_record_destroy(email, true);
			return MSG_SUCCESS;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			contacts_record_destroy(contact, true);
			contacts_record_destroy(email, true);
			return MSG_SUCCESS;
		}
	}

	contacts_list_destroy(contacts, true);

	// Name Info
	contacts_record_h name = NULL;

	ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.name, 0, &name);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return MSG_SUCCESS;
	}

	char* strFirstName = NULL;
	ret = contacts_record_get_str_p(name, _contacts_name.first, &strFirstName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return MSG_SUCCESS;
	}

	char* strLastName = NULL;
	ret = contacts_record_get_str_p(name, _contacts_name.last, &strLastName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return MSG_SUCCESS;
	}

	MSG_DEBUG("First Name : [%s], Last Name : [%s]", strFirstName, strLastName);

	if (strFirstName != NULL)
		strncpy(pContactInfo->firstName, strFirstName, MAX_DISPLAY_NAME_LEN);

	if (strLastName != NULL)
		strncpy(pContactInfo->lastName, strLastName, MAX_DISPLAY_NAME_LEN);

	ret = contacts_record_get_int(contact, _contacts_contact.id, (int*)&pContactInfo->contactId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return MSG_SUCCESS;
	}

	MSG_DEBUG("Contact ID [%d]", pContactInfo->contactId);

	char* strImagePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.image_thumbnail_path, &strImagePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return MSG_SUCCESS;
	}

	if (strImagePath != NULL)
		strncpy(pContactInfo->imagePath , strImagePath, MAX_IMAGE_PATH_LEN);

	MSG_DEBUG("Image Path [%s]", pContactInfo->imagePath);

	contacts_record_destroy(contact, true);

	MSG_END();

	return MSG_SUCCESS;
}


void MsgSyncContact()
{
	int ret = -1;
	unsigned int changed_count = 0;
	int lastSyncTime = 0;
	int finalSyncTime = 0;

	/* get contact sync time */
	lastSyncTime = MsgSettingGetInt(CONTACT_SYNC_TIME);

	if (lastSyncTime < 0) {
		MSG_DEBUG("Fail to get CONTACT_SYNC_TIME.");
		lastSyncTime = 0;
	}

	contacts_list_h contactsList = NULL;

	ret = contacts_db_get_changes_by_version(_contacts_contact_updated_info._uri, -1, lastSyncTime, &contactsList, &finalSyncTime);

	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_changes_by_version() Error [%d]", ret);
		return;
	}

	ret = contacts_list_get_count(contactsList, &changed_count);

	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_list_get_count() Error [%d]", ret);
		contacts_list_destroy(contactsList, true);
		return;
	}

	for (unsigned int i = 0; i < changed_count; i++)
	{
		int index_num = 0;
		int type = 0;
		contacts_record_h event = NULL;

		ret = contacts_list_get_current_record_p(contactsList, &event);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			return;
		}

		ret = contacts_record_get_int(event, _contacts_contact_updated_info.contact_id, &index_num);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			return;
		}

		MSG_DEBUG("index (%d)", index_num);

		ret = contacts_record_get_int(event, _contacts_contact_updated_info.type, &type);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			return;
		}

		if (type == CONTACTS_CHANGE_UPDATED || type == CONTACTS_CHANGE_INSERTED) {
			MsgUpdateContact(index_num, type);
		} else {// Delete
			MSG_DEBUG("Delete Contact");
			MsgDeleteContact(index_num);
		}

		ret = contacts_list_next(contactsList);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_next() Error [%d]", ret);
		}
	}

	if(MsgSettingSetInt(CONTACT_SYNC_TIME, finalSyncTime) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetInt fail : CONTACT_SYNC_TIME");
	MSG_DEBUG("lastSyncTime : %d", finalSyncTime);

	contacts_list_destroy(contactsList, true);

	if(changed_count > 0)
		cbFunction();
}


bool MsgInsertContact(MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber)
{
	if (!pNumber || strlen(pNumber) <= 0)
		return false;

	if (MsgStoAddContactInfo(&ContactDbHandle, pContactInfo, pNumber) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to add contact info.");
		return false;
	}

	return true;
}


bool MsgUpdateContact(int index, int type)
{
	int ret = CONTACTS_ERROR_NONE;

	contacts_record_h contact = NULL;

	ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	MSG_CONTACT_INFO_S contactInfo;
	memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	ret = contacts_record_get_int(contact, _contacts_contact.id, (int*)&contactInfo.contactId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	MSG_DEBUG("Contact ID [%d]", contactInfo.contactId);

	char* strImagePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.image_thumbnail_path, &strImagePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	if (strImagePath != NULL)
		strncpy(contactInfo.imagePath , strImagePath, MAX_IMAGE_PATH_LEN);

	MSG_DEBUG("Image Path [%s]", contactInfo.imagePath);

	// Name Info
	contacts_record_h name = NULL;

	ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.name, 0, &name);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	char* strFirstName = NULL;
	ret = contacts_record_get_str_p(name, _contacts_name.first, &strFirstName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	char* strLastName = NULL;
	ret = contacts_record_get_str_p(name, _contacts_name.last, &strLastName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	MSG_DEBUG("First Name : [%s], Last Name : [%s]", strFirstName, strLastName);

	if (strFirstName != NULL)
		strncpy(contactInfo.firstName, strFirstName, MAX_DISPLAY_NAME_LEN);

	if (strLastName != NULL)
		strncpy(contactInfo.lastName, strLastName, MAX_DISPLAY_NAME_LEN);

	MsgStoClearContactInfo(&ContactDbHandle, index);

	unsigned int count = 0;
	ret = contacts_record_get_child_record_count(contact, _contacts_contact.number, &count);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return false;
	}

	if (count > 0) {
		for(unsigned int i=0; i < count; i++)
		{
			MSG_DEBUG("Add Contact Data");

			contacts_record_h number = NULL;

			ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.number, i, &number);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
				contacts_record_destroy(contact, true);
				return false;
			}

			char* strNumber = NULL;
			ret = contacts_record_get_str_p(number, _contacts_number.number, &strNumber);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
				contacts_record_destroy(contact, true);
				return false;
			}

			if (strNumber != NULL) {
				MSG_DEBUG("Number = %s", strNumber);
				if (!MsgInsertContact(&contactInfo, strNumber)) {
					MSG_DEBUG("MsgInsertContact fail.");
				}
			}
		}
	} else {// No phone number in contact
		contacts_record_destroy(contact, true);
		return true;
	}

	MsgStoSetConversationDisplayName(&ContactDbHandle, index);

	contacts_record_destroy(contact, true);

	return true;
}


bool MsgDeleteContact(int index)
{
	if (MsgStoClearContactInfo(&ContactDbHandle, index) != MSG_SUCCESS)
		return false;

	return true;
}


int MsgGetContactNameOrder()
{
	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return 0;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		return 0; // return default value : FIRSTLAST
	}

	int ret = CONTACTS_ERROR_NONE;

	contacts_name_display_order_e order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;

	ret = contacts_setting_get_name_display_order(&order);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_setting_get_name_display_order() Error [%d]", ret);
		return 0;
	}

	if (order == CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST)
		return 0;
	else
		return 1;
}


void MsgAddPhoneLog(const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		return;
	}

	if(pMsgInfo->nAddressCnt < 1) {
		MSG_DEBUG("address count is [%d]", pMsgInfo->nAddressCnt);
		return;
	}

	for (int i = 0; pMsgInfo->nAddressCnt > i; i++) {
		int ret = 0;
		contacts_record_h plog = NULL;

		ret = contacts_record_create(_contacts_phone_log._uri, &plog);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_create() Error [%d]", ret);
			contacts_record_destroy(plog, true);
			break;
		}

		contacts_record_set_str(plog, _contacts_phone_log.address, (char*)pMsgInfo->addressList[i].addressVal);
		contacts_record_set_int(plog, _contacts_phone_log.log_time, (int)time(NULL));

		char strText[101];
		memset(strText, 0x00, sizeof(strText));

		if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE) {
			strncpy(strText, pMsgInfo->msgText, 100);
			MSG_DEBUG("msgText : %s", strText);
		} else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE) {
			if (strlen(pMsgInfo->subject) > 0 || pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
				strncpy(strText, pMsgInfo->subject, 100);
				MSG_DEBUG("subject : %s", strText);
			} else {
				strncpy(strText, pMsgInfo->msgText, 100);
				MSG_DEBUG("msgText : %s", strText);
			}
		}

		contacts_record_set_str(plog, _contacts_phone_log.extra_data2, strText);
		contacts_record_set_int(plog, _contacts_phone_log.extra_data1, (int)pMsgInfo->msgId);

		if (pMsgInfo->folderId == MSG_INBOX_ID) {
			if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_INCOMMING);
			else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_INCOMMING);
		} else if (pMsgInfo->folderId == MSG_OUTBOX_ID) {
			if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_OUTGOING);
			else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_OUTGOING);
		} else if (pMsgInfo->folderId == MSG_SPAMBOX_ID) {
			if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_BLOCKED);
			else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_BLOCKED);
		}

		ret = contacts_db_insert_record(plog, NULL);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_db_insert_record() Error [%d]", ret);
		}

		contacts_record_destroy(plog, true);
	}
}


void MsgDeletePhoneLog(msg_message_id_t msgId)
{
	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return;
	}

	MSG_DEBUG("MsgDeletePhoneLog [%d]", msgId);

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		return;
	}

	int ret = CONTACTS_ERROR_NONE;
	int index = 0;
	unsigned int count = 0;
	contacts_query_h query;
	contacts_filter_h filter;
	contacts_list_h plogs = NULL;

	ret = contacts_query_create(_contacts_phone_log._uri, &query);
	ret = contacts_filter_create(_contacts_phone_log._uri, &filter);

	ret = contacts_filter_add_int(filter, _contacts_phone_log.extra_data1, CONTACTS_MATCH_EQUAL, (int)msgId);


	ret = contacts_query_set_filter(query, filter);
	ret = contacts_db_get_records_with_query(query, 0, 1, &plogs);

	ret = contacts_list_get_count(plogs, &count);

	if (count == 0) {
		MSG_DEBUG("No Serach Data from Contact Service.");
	} else {
		contacts_record_h plog = NULL;

		ret = contacts_list_get_current_record_p(plogs, &plog);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
		}

		ret = contacts_record_get_int(plog, _contacts_phone_log.id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
		}

		ret = contacts_db_delete_record(_contacts_phone_log._uri, index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
		} else {
			MSG_DEBUG("contacts_db_delete_record() Success.");
		}
	}

	contacts_query_destroy(query);
	contacts_filter_destroy(filter);
	contacts_list_destroy(plogs, true);

}


int MsgContactSVCBeginTrans()
{
	//return contacts_svc_begin_trans();
	return 0;
}


int MsgContactSVCEndTrans(bool bSuccess)
{
	//return contacts_svc_end_trans(bSuccess);
	return 0;
}
