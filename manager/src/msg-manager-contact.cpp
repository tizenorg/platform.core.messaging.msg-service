/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved
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

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>

#include <msg_storage.h>

#include <msg-manager-contact.h>
#include <msg-manager-debug.h>

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

static bool isContactSvcConnected = false;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

int MsgMgrOpenContactSvc()
{
	int errCode = CONTACTS_ERROR_NONE;

	if (!isContactSvcConnected) {
		errCode = contacts_connect();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("Connect to Contact Service Success");
			isContactSvcConnected = true;
		} else {
			MSG_MGR_DEBUG("Connect to Contact Service Fail [%d]", errCode);
			isContactSvcConnected = false;
			return -1;
		}
	} else {
		MSG_MGR_DEBUG("Already connected to Contact Service.");
	}
	return 0;
}


int MsgMgrCloseContactSvc()
{
	int errCode = CONTACTS_ERROR_NONE;

	if (isContactSvcConnected) {
		errCode = contacts_disconnect();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("Disconnect to Contact Service Success");
			isContactSvcConnected = false;
		} else {
			MSG_MGR_DEBUG("Disconnect to Contact Service Fail [%d]", errCode);
			return -1;
		}
	}
	return 0;
}


void MsgMgrAddPhoneLog(contactInfo *contact_info)
{
	int err = 0;

	if ((err = MsgMgrOpenContactSvc()) != 0) {
		MSG_MGR_DEBUG("MsgMgrOpenContactSvc fail.");
		return;
	}

	if (!isContactSvcConnected) {
		MSG_MGR_DEBUG("Contact Service Not Opened.");
		return;
	}

	int addr_cnt = msg_list_length(contact_info->addrList);
	if (addr_cnt < 1) {
		MSG_MGR_DEBUG("address count is [%d]", addr_cnt);
		return;
	}

	for (int i = 0; i < addr_cnt; i++) {
		int ret = 0;
		contacts_record_h plog = NULL;
		char addressVal[MAX_ADDRESS_VAL_LEN + 1] = {0};

		msg_struct_t addr_data = msg_list_nth_data(contact_info->addrList, i);

		ret = contacts_record_create(_contacts_phone_log._uri, &plog);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_record_create() Error [%d]", ret);
			contacts_record_destroy(plog, true);
			break;
		}

		contacts_record_set_int(plog, _contacts_phone_log.sim_slot_no, contact_info->simIndex-1);
		msg_get_str_value(addr_data, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, addressVal, MAX_ADDRESS_VAL_LEN);
		MSG_MGR_SEC_DEBUG("addressVal : %s", addressVal);
		contacts_record_set_str(plog, _contacts_phone_log.address, addressVal);
		contacts_record_set_int(plog, _contacts_phone_log.log_time, (int)time(NULL));

		char strText[MAX_CONTACT_TEXT_LEN + 1];
		memset(strText, 0x00, sizeof(strText));

		if (contact_info->msgType != MSG_TYPE_MMS && contact_info->msgType != MSG_TYPE_MMS_NOTI && contact_info->msgType != MSG_TYPE_MMS_JAVA) {
			strncpy(strText, contact_info->msgText, MAX_CONTACT_TEXT_LEN);
			MSG_MGR_SEC_DEBUG("msgText : %s", strText);
		} else {
			if (strlen(contact_info->subject) > 0 || contact_info->msgType == MSG_TYPE_MMS_NOTI) {
				strncpy(strText, contact_info->subject, MAX_CONTACT_TEXT_LEN);
				MSG_MGR_SEC_DEBUG("subject : %s", strText);
			} else {
				char *pFileData = NULL;
				gsize fileSize = 0;

				if (contact_info->msgText[0] != '\0' && g_file_get_contents(contact_info->msgText, &pFileData, &fileSize, NULL) == true) {
					if (pFileData)
						strncpy(strText, pFileData, 100);
				}

				if (pFileData)
					g_free(pFileData);

				MSG_MGR_SEC_DEBUG("msgText : %s", strText);
			}
		}

		contacts_record_set_str(plog, _contacts_phone_log.extra_data2, strText);
		contacts_record_set_int(plog, _contacts_phone_log.extra_data1, contact_info->msgId);

		if (contact_info->folderId == MSG_INBOX_ID) {
			if (contact_info->msgType != MSG_TYPE_MMS && contact_info->msgType != MSG_TYPE_MMS_NOTI && contact_info->msgType != MSG_TYPE_MMS_JAVA)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_INCOMING);
			else
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_INCOMING);
		} else if (contact_info->folderId == MSG_OUTBOX_ID || contact_info->folderId == MSG_SENTBOX_ID) {
			if (contact_info->msgType != MSG_TYPE_MMS && contact_info->msgType != MSG_TYPE_MMS_NOTI && contact_info->msgType != MSG_TYPE_MMS_JAVA)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_OUTGOING);
			else
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_OUTGOING);
		} else if (contact_info->folderId == MSG_SPAMBOX_ID) {
			if (contact_info->msgType != MSG_TYPE_MMS && contact_info->msgType != MSG_TYPE_MMS_NOTI && contact_info->msgType != MSG_TYPE_MMS_JAVA)
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_SMS_BLOCKED);
			else
				contacts_record_set_int(plog, _contacts_phone_log.log_type, CONTACTS_PLOG_TYPE_MMS_BLOCKED);
		}

		ret = contacts_db_insert_record(plog, NULL);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_db_insert_record() Error [%d]", ret);
		}

		contacts_record_destroy(plog, true);
	}
}

int MsgMgrGetContactInfo(const MSG_MGR_ADDRESS_INFO_S *pAddrInfo, MSG_MGR_CONTACT_INFO_S *pContactInfo)
{
	MSG_MGR_BEGIN();
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	int err = 0;

	if ((err = MsgMgrOpenContactSvc()) != 0) {
		MSG_MGR_DEBUG("MsgMgrOpenContactSvc fail.");
		return err;
	}

	if (!isContactSvcConnected) {
		MSG_MGR_DEBUG("Contact Service Not Opened.");
		return -1;
	}

	MSG_MGR_SEC_DEBUG("Address Type [%d], Address Value [%s]", pAddrInfo->addressType, pAddrInfo->addressVal);

	memset(pContactInfo, 0x00, sizeof(MSG_MGR_CONTACT_INFO_S));

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN && strlen(pAddrInfo->addressVal) > (MAX_PHONE_NUMBER_LEN+1)) {
		MSG_MGR_SEC_DEBUG("Phone Number is too long [%s]", pAddrInfo->addressVal);
		return -1;
	}

	int ret = 0;
	int index = 0;
	int count = 0;
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
		MSG_MGR_DEBUG("Invalid pAddrInfo->addressType.");
		return -1;
	}

	ret = contacts_query_set_filter(query, filter);
	ret = contacts_db_get_records_with_query(query, 0, 1, &contacts);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_db_get_records_with_query() Error [%d]", ret);
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		return -1;
	}

	ret = contacts_list_get_count(contacts, &count);

	if (count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("No Serach Data from Contact Service.");
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		return 0;
	}

	contacts_query_destroy(query);
	contacts_filter_destroy(filter);

	contacts_record_h contact = NULL;

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN || pAddrInfo->addressType == MSG_ADDRESS_TYPE_UNKNOWN) {
		contacts_record_h number = NULL;

		ret = contacts_list_get_current_record_p(contacts, &number);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}

		ret = contacts_record_get_int(number, _contacts_contact_number.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}
	} else if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_EMAIL) {
		contacts_record_h email = NULL;

		ret = contacts_list_get_current_record_p(contacts, &email);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}

		ret = contacts_record_get_int(email, _contacts_contact_email.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			return -1;
		}
	}

	contacts_list_destroy(contacts, true);

	ret = contacts_record_get_int(contact, _contacts_contact.id, (int*)&pContactInfo->contactId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return -1;
	}

	MSG_MGR_DEBUG("Contact ID [%d]", pContactInfo->contactId);

	ret = contacts_record_get_int(contact, _contacts_contact.address_book_id, (int*)&pContactInfo->addrbookId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		return -1;
	}

	MSG_MGR_DEBUG("Address Book ID [%d]", pContactInfo->addrbookId);

	char* strImagePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.image_thumbnail_path, &strImagePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (strImagePath != NULL) {
		strncpy(pContactInfo->imagePath , strImagePath, MAX_IMAGE_PATH_LEN);
		MSG_MGR_DEBUG("Image Path [%s]", pContactInfo->imagePath);
	}

	char* alerttonePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.message_alert, &alerttonePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}

	if (alerttonePath != NULL && access(alerttonePath, F_OK) != 0) {
		alerttonePath = NULL;
	}

	if (alerttonePath != NULL) {
		MSG_MGR_DEBUG("alert tone Path [%s]", alerttonePath);
		strncpy(pContactInfo->alerttonePath , alerttonePath, MSG_FILEPATH_LEN_MAX);
	} else {
		MSG_MGR_DEBUG("alert tone Path for this contact is default");
		count = 0;
		ret = contacts_record_get_child_record_count(contact, _contacts_contact.group_relation, &count);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_MGR_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
		}

		contacts_record_h group_relation_record;

		for (int i = 0; i < count; i++) {
			int group_id = 0;
			contacts_record_get_child_record_at_p(contact, _contacts_contact.group_relation, i, &group_relation_record);
			contacts_record_get_int(group_relation_record, _contacts_group_relation.group_id, &group_id);

			contacts_record_h group_record;
			contacts_db_get_record(_contacts_group._uri, group_id, &group_record);

			MSG_MGR_DEBUG("Group ID = [%d]", group_id);

			char *group_ringtone_path;
			ret = contacts_record_get_str_p(group_record, _contacts_group.message_alert, &group_ringtone_path);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_MGR_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
			} else {
				if (group_ringtone_path) {
					MSG_MGR_DEBUG("Msg alert_tone is change to [%s] as contact group", group_ringtone_path);
					memset(pContactInfo->alerttonePath, 0x00, sizeof(pContactInfo->alerttonePath));
					snprintf(pContactInfo->alerttonePath, sizeof(pContactInfo->alerttonePath), "%s", group_ringtone_path);
					contacts_record_destroy(group_record, true);
					break;
				}
			}
			contacts_record_destroy(group_record, true);
		}
	}

	char* vibrationPath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.vibration, &vibrationPath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (vibrationPath != NULL) {
		MSG_MGR_DEBUG("vibration Path [%s]", vibrationPath);
		strncpy(pContactInfo->vibrationPath , vibrationPath, MSG_FILEPATH_LEN_MAX);
	}

	char* displayName = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.display_name, &displayName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_MGR_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (displayName != NULL) {
		MSG_MGR_DEBUG("displayName [%s]", displayName);
		strncpy(pContactInfo->firstName , displayName, MAX_DISPLAY_NAME_LEN);
	}

	contacts_record_destroy(contact, true);

#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	MSG_MGR_END();

	return 0;
}

char* msg_mgr_clean_country_code(char *src)
{
	int ret = 1;

	switch (src[ret++]-'0') {
		case 1:
		case 7:
			break;
		case 2:
			switch (src[ret++]-'0') {
				case 0:
				case 7:
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 3:
			switch (src[ret++]-'0') {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 9:
					break;
				case 5:
				case 7:
				case 8:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 4:
			switch (src[ret++]-'0') {
				case 0:
				case 1:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					break;
				case 2:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 5:
			switch (src[ret++]-'0') {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					break;
				case 0:
				case 9:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 6:
			switch (src[ret++]-'0') {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
					break;
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 8:
			switch (src[ret++]-'0') {
				case 1:
				case 2:
				case 4:
				case 6:
					break;
				case 0:
				case 3:
				case 5:
				case 7:
				case 8:
				case 9:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 9:
			switch (src[ret++]-'0') {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 8:
					break;
				case 6:
				case 7:
				case 9:
					ret += 1;
					break;
				default:
					MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
					break;
			}
			break;
		case 0:
		default:
			MSG_MGR_DEBUG("The parameter(src:%s) has invalid character set", src);
			return src;
	}

	return &src[ret];
}


char* msg_mgr_normalize_number(char *src)
{
	char *normalized_number;

	if ('+' == src[0])
		normalized_number = msg_mgr_clean_country_code(src);
	else if ('0' == src[0])
		normalized_number = src+1;
	else
		normalized_number = src;

	MSG_MGR_DEBUG("src = %s, normalized = %s", src, normalized_number);

	return normalized_number;
}
