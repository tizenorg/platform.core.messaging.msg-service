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

#include <ctype.h>

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"
#include "MsgContact.h"
#include "MsgZoneManager.h"

extern "C"
{
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	#include <contacts.h>
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED
}

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
__thread bool isContactSvcConnected = false;

MsgDbHandler ContactDbHandle;

MsgContactChangeCB cbFunction = NULL;

#define CONTACT_CALLBACK_USER_DATA				"contact"
#define ADDRESSBOOK_CALLBACK_USER_DATA	"addressbook"

// phonenumber minimum match digit.
#define PHONENUMBER_MIN_MATCH_DIGIT VCONFKEY_CONTACTS_SVC_PHONENUMBER_MIN_MATCH_DIGIT
#define DEFAULT_MIN_MATCH_DIGIT 8

static int phonenumberMinMatchDigit = -1;

/*==================================================================================================
                                     INTERNAL FUNCTION IMPLEMENTATION
==================================================================================================*/
int countryCodeLength(const char *src)
{
	int ret = 0;
	switch (src[ret++]-'0')
	{
	case 1:
	case 7:
		break;
	case 2:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 3:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 4:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 5:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 6:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 8:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 9:
		switch (src[ret++]-'0')
		{
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
			MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 0:
	default:
		MSG_DEBUG("The parameter(src:%s) has invalid character set", src);
		return 0;
	}

	return ret;
}


void normalizeNumber(const char *orig, char* dest, unsigned int destSize)
{
	unsigned int pos = 0;
	for (unsigned int i=0; (orig[i] && i<destSize); i++) {
		if (isdigit(orig[i]) || (orig[i] == '+')) {
			dest[pos++] = orig[i];
		}
	}
}


//static void MsgContactSvcCallback(const char *view_uri, void *user_data)
//{
//	MSG_DEBUG("MsgContactSvcCallback is called.");
//	if (!strcmp(CONTACT_CALLBACK_USER_DATA, (const char*)user_data)) {
//		MSG_DEBUG("Contact Data is Changed!!!");
//		MsgSyncContact();
//	} else if (!strcmp(ADDRESSBOOK_CALLBACK_USER_DATA, (const char*)user_data)) {
//		MSG_DEBUG("Address Book Data is Changed!!!");
//		MsgSyncAddressbook();
//	}
//
//	if (ContactDbHandle.disconnect() != MSG_SUCCESS)
//		MSG_DEBUG("DB Disconnect Fail");
//}

#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgOpenContactSvc()
{
	int errCode = CONTACTS_ERROR_NONE;

	if (!isContactSvcConnected) {
		errCode = contacts_connect();

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
	MsgZoneChange();

	int errCode = CONTACTS_ERROR_NONE;

	if (isContactSvcConnected) {
		errCode = contacts_disconnect();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_DEBUG("Disconnect to Contact Service Success");
			isContactSvcConnected = false;
		} else {
			MSG_DEBUG("Disconnect to Contact Service Fail [%d]", errCode);
			MsgZoneRevert();
			return MSG_ERR_DB_DISCONNECT;
		}
	}

	MsgZoneRevert();

	return MSG_SUCCESS;
}


msg_error_t MsgInitContactSvc(MsgContactChangeCB cb)
{
	MsgZoneChange();

//	msg_error_t err = MSG_SUCCESS;
//	unsigned int retryCnt = 10;

	if (cb != NULL)
		cbFunction = cb;

	phonenumberMinMatchDigit = MsgSettingGetInt(PHONENUMBER_MIN_MATCH_DIGIT);
	MSG_DEBUG("phonenumberMinMatchDigit [%d]", phonenumberMinMatchDigit);

	if (phonenumberMinMatchDigit < 1) {
		phonenumberMinMatchDigit = DEFAULT_MIN_MATCH_DIGIT;
	}

#if 0
	do {
		if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
			retryCnt--;
			MSG_DEBUG("MsgOpenContactSvc fail. Retry count left [%d]", retryCnt);
			usleep(100 * 1000);
		}
	} while (err != MSG_SUCCESS && retryCnt > 0);

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	// Sync contact first.
	MsgSyncAddressbook();
//	MsgSyncContact();

	int errCode = CONTACTS_ERROR_NONE;

	// Register callback function
	errCode = contacts_db_add_changed_cb(_contacts_contact._uri, MsgContactSvcCallback, (void *)CONTACT_CALLBACK_USER_DATA);

	if (errCode == CONTACTS_ERROR_NONE)
		MSG_DEBUG("Register Contact Service Callback [_contacts_contact]");
	else
		MSG_DEBUG("Fail to Register Contact Service Callback [_contacts_contact] [%d]", errCode);

	// Register callback function
	errCode = contacts_db_add_changed_cb(_contacts_address_book._uri, MsgContactSvcCallback, (void *)ADDRESSBOOK_CALLBACK_USER_DATA);

	if (errCode == CONTACTS_ERROR_NONE)
		MSG_DEBUG("Register Contact Service Callback [_contacts_address_book]");
	else
		MSG_DEBUG("Fail to Register Contact Service Callback [_contacts_address_book] [%d]", errCode);

	if (ContactDbHandle.disconnect() != MSG_SUCCESS)
			MSG_DEBUG("DB Disconnect Fail");
#endif

	MsgZoneRevert();

	return MSG_SUCCESS;
}


msg_error_t MsgGetContactInfo(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_CONTACT_INFO_S *pContactInfo)
{
	MSG_BEGIN();

	MsgZoneChange();

	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		MsgZoneRevert();
		return err;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	MSG_SEC_DEBUG("Address Type [%d], Address Value [%s]", pAddrInfo->addressType, pAddrInfo->addressVal);

	memset(pContactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN && strlen(pAddrInfo->addressVal) > (MAX_PHONE_NUMBER_LEN+1)) {
		MSG_SEC_DEBUG("Phone Number is too long [%s]", pAddrInfo->addressVal);
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
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
		MSG_DEBUG("Invalid pAddrInfo->addressType.");
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	ret = contacts_query_set_filter(query, filter);
	ret = contacts_db_get_records_with_query(query, 0, 1, &contacts);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_records_with_query() Error [%d]", ret);
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	ret = contacts_list_get_count(contacts, &count);

	if (count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("No Serach Data from Contact Service.");
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(contacts, true);
		MsgZoneRevert();
		return MSG_SUCCESS;
	}

	contacts_query_destroy(query);
	contacts_filter_destroy(filter);

	contacts_record_h contact = NULL;

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN || pAddrInfo->addressType == MSG_ADDRESS_TYPE_UNKNOWN) {
		contacts_record_h number = NULL;

		ret = contacts_list_get_current_record_p(contacts, &number);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}

		ret = contacts_record_get_int(number, _contacts_contact_number.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}
	} else if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_EMAIL) {
		contacts_record_h email = NULL;

		ret = contacts_list_get_current_record_p(contacts, &email);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}

		ret = contacts_record_get_int(email, _contacts_contact_email.contact_id, &index);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}

		ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
			contacts_list_destroy(contacts, true);
			MsgZoneRevert();
			return MSG_ERR_UNKNOWN;
		}
	}

	contacts_list_destroy(contacts, true);

#if 0
	// Name Info
	contacts_record_h name = NULL;

	ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.name, 0, &name);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
	} else {
		char* strName = NULL;
		ret = contacts_record_get_str_p(name, _contacts_name.first, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(pContactInfo->firstName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.last, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(pContactInfo->lastName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.addition, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(pContactInfo->middleName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.prefix, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(pContactInfo->prefix, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.suffix, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(pContactInfo->suffix, strName, MAX_DISPLAY_NAME_LEN);
			}
		}
	}
#endif

	ret = contacts_record_get_int(contact, _contacts_contact.id, (int*)&pContactInfo->contactId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	MSG_DEBUG("Contact ID [%d]", pContactInfo->contactId);

	ret = contacts_record_get_int(contact, _contacts_contact.address_book_id, (int*)&pContactInfo->addrbookId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	MSG_DEBUG("Address Book ID [%d]", pContactInfo->addrbookId);

	char* strImagePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.image_thumbnail_path, &strImagePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (strImagePath != NULL) {
		strncpy(pContactInfo->imagePath , strImagePath, MAX_IMAGE_PATH_LEN);
		MSG_DEBUG("Image Path [%s]", pContactInfo->imagePath);
	}

	char* alerttonePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.message_alert, &alerttonePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}

	if (MsgAccessFile(alerttonePath, F_OK) == false) {
		alerttonePath = NULL;
	}

	if (alerttonePath != NULL) {
		MSG_DEBUG("alert tone Path [%s]", alerttonePath);
		strncpy(pContactInfo->alerttonePath , alerttonePath, MSG_FILEPATH_LEN_MAX);
	} else {
		MSG_DEBUG("alert tone Path for this contact is default");
		count = 0;
		ret = contacts_record_get_child_record_count(contact, _contacts_contact.group_relation, &count);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
		}

		contacts_record_h group_relation_record;

		for (int i = 0; i < count; i++) {
			int group_id = 0;
			contacts_record_get_child_record_at_p(contact, _contacts_contact.group_relation, i, &group_relation_record);
			contacts_record_get_int(group_relation_record, _contacts_group_relation.group_id, &group_id);

			contacts_record_h group_record;
			contacts_db_get_record(_contacts_group._uri, group_id, &group_record);

			MSG_DEBUG("Group ID = [%d]", group_id);

			char *group_ringtone_path;
			ret = contacts_record_get_str_p(group_record, _contacts_group.message_alert, &group_ringtone_path);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
			} else {
				if (group_ringtone_path) {
					MSG_DEBUG("Msg alert_tone is change to [%s] as contact group", group_ringtone_path);
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
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (vibrationPath != NULL) {
		MSG_DEBUG("vibration Path [%s]", vibrationPath);
		strncpy(pContactInfo->vibrationPath , vibrationPath, MSG_FILEPATH_LEN_MAX);
	}

	char* displayName = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.display_name, &displayName);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}
	if (displayName != NULL) {
		MSG_DEBUG("displayName [%s]", displayName);
		strncpy(pContactInfo->firstName , displayName, MAX_DISPLAY_NAME_LEN);
	}

	contacts_record_destroy(contact, true);

	MSG_END();

	MsgZoneRevert();

	return MSG_SUCCESS;
}


msg_error_t MsgGetContactSearchList(const char *pSearchVal, MSG_ADDRESS_INFO_S **pAddrInfo, int *count)
{
	MSG_BEGIN();

	MsgZoneChange();

	msg_error_t err = MSG_SUCCESS;

	*count = 0;

	if (pSearchVal == NULL) {
		MSG_DEBUG("pSearchVal is NULL.");
		MsgZoneRevert();
		return MSG_ERR_NULL_POINTER;
	}

	if (pAddrInfo == NULL) {
		MSG_DEBUG("pAddrInfo is NULL.");
		MsgZoneRevert();
		return MSG_ERR_NULL_POINTER;
	}

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		MsgZoneRevert();
		return err;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	MSG_SEC_DEBUG("pSearchVal [%s]", pSearchVal);

	int ret = 0;
	unsigned int index = 0;
	contacts_query_h query = NULL;
	contacts_filter_h filter = NULL;
	contacts_list_h personNumbers = NULL;

	ret = contacts_query_create(_contacts_person_number._uri, &query);
	ret = contacts_filter_create(_contacts_person_number._uri, &filter);

	ret = contacts_filter_add_str(filter, _contacts_person_number.display_name, CONTACTS_MATCH_CONTAINS, pSearchVal);
	ret = contacts_query_set_filter(query, filter);

	ret = contacts_db_get_records_with_query(query, 0, 0, &personNumbers);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_records_with_query() Error [%d]", ret);
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(personNumbers, true);
		MsgZoneRevert();
		return MSG_ERR_UNKNOWN;
	}

	ret = contacts_list_get_count(personNumbers, count);
	if (*count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("No Serach Data from Contact Service.");
		*count = 0;
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(personNumbers, true);
		MsgZoneRevert();
		return MSG_SUCCESS;
	}

	contacts_query_destroy(query);
	contacts_filter_destroy(filter);

	MSG_DEBUG(" *count [%d]", *count);

	*pAddrInfo = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S) * (*count)];
	memset(*pAddrInfo, 0x00, (sizeof(MSG_ADDRESS_INFO_S) * (*count)));

	contacts_record_h personNumber = NULL;

	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(personNumbers, &personNumber)) {
		char* normalizedNumber = NULL;
		ret = contacts_record_get_str(personNumber, _contacts_person_number.normalized_number, &normalizedNumber);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
		}
		else if (normalizedNumber) {
			MSG_DEBUG("normalizedNumber [%s]", normalizedNumber);
			strncpy((*pAddrInfo)[index].addressVal, normalizedNumber, MAX_ADDRESS_VAL_LEN);
		}

		contacts_list_next(personNumbers);
		index++;
	}

	contacts_list_destroy(personNumbers, true);

	MSG_END();

	MsgZoneRevert();

	return MSG_SUCCESS;
}


void MsgSyncAddressbook()
{
	MsgZoneChange();

	int ret = -1;
	int changed_count = 0;

	contacts_list_h addrbookListHnd = NULL;

	ret = contacts_db_get_all_records(_contacts_address_book._uri, 0, 0, &addrbookListHnd);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_all_records() Error [%d]", ret);
		contacts_list_destroy(addrbookListHnd, true);
		MsgZoneRevert();
		return;
	}

	ret = contacts_list_get_count(addrbookListHnd, &changed_count);

	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_list_get_count() Error [%d]", ret);
		contacts_list_destroy(addrbookListHnd, true);
		MsgZoneRevert();
		return;
	}

	if (changed_count>0) {
		int addrbookList[changed_count];

		for (int i = 0; i < changed_count; i++) {
			contacts_record_h addrbook = NULL;
			int addrbookId = 0;

			ret = contacts_list_get_current_record_p(addrbookListHnd, &addrbook);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
				contacts_list_destroy(addrbookListHnd, true);
				MsgZoneRevert();
				return;
			}

			ret = contacts_record_get_int(addrbook, _contacts_address_book.id, &addrbookId);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
				contacts_list_destroy(addrbookListHnd, true);
				MsgZoneRevert();
				return;
			}

			addrbookList[i] = addrbookId;

			ret = contacts_list_next(addrbookListHnd);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_list_next() Error [%d]", ret);
			}

		}

		MsgStoClearContactInfoByAddrbookIdList(&ContactDbHandle, addrbookList, (int)changed_count);
	}

	contacts_list_destroy(addrbookListHnd, true);

	MsgZoneRevert();
}


void MsgSyncContact()
{
	MsgZoneChange();

	int ret = -1;
	int changed_count = 0;
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
		contacts_list_destroy(contactsList, true);
		MsgZoneRevert();
		return;
	}

	ret = contacts_list_get_count(contactsList, &changed_count);

	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_list_get_count() Error [%d]", ret);
		contacts_list_destroy(contactsList, true);
		MsgZoneRevert();
		return;
	}

	for (int i = 0; i < changed_count; i++)
	{
		int index_num = 0;
		int type = 0;
		contacts_record_h event = NULL;

		ret = contacts_list_get_current_record_p(contactsList, &event);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			MsgZoneRevert();
			return;
		}

		ret = contacts_record_get_int(event, _contacts_contact_updated_info.contact_id, &index_num);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			MsgZoneRevert();
			return;
		}

		MSG_DEBUG("index (%d)", index_num);

		ret = contacts_record_get_int(event, _contacts_contact_updated_info.type, &type);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
			contacts_list_destroy(contactsList, true);
			MsgZoneRevert();
			return;
		}

		MSG_DEBUG("type [%d]", type);

		if (type != CONTACTS_CHANGE_INSERTED) {
			MsgDeleteContact(index_num);
		}

		if (type != CONTACTS_CHANGE_DELETED) {
			MsgUpdateContact(index_num, type);
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

	if(changed_count > 0) {
		if(cbFunction) cbFunction();
	}

	MsgZoneRevert();
}


bool MsgInsertContact(MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber)
{
	MsgZoneChange();

	if (!pNumber || strlen(pNumber) <= 0)
		MsgZoneRevert();
		return false;

	if (MsgStoAddContactInfo(&ContactDbHandle, pContactInfo, pNumber) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to add contact info.");
		MsgZoneRevert();
		return false;
	}

	MsgZoneRevert();

	return true;
}


bool MsgUpdateContact(int index, int type)
{
	MsgZoneChange();

	int ret = CONTACTS_ERROR_NONE;

	contacts_record_h contact = NULL;

	ret = contacts_db_get_record(_contacts_contact._uri, index, &contact);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return false;
	}

#if 0
	MSG_CONTACT_INFO_S contactInfo;
	memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	ret = contacts_record_get_int(contact, _contacts_contact.id, (int*)&contactInfo.contactId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return false;
	}

	MSG_DEBUG("Contact ID [%d]", contactInfo.contactId);

	ret = contacts_record_get_int(contact, _contacts_contact.address_book_id, (int*)&contactInfo.addrbookId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_record() Error [%d]", ret);
	}

	MSG_DEBUG("Addressbook ID [%d]", contactInfo.addrbookId);

	char* strImagePath = NULL;
	ret = contacts_record_get_str_p(contact, _contacts_contact.image_thumbnail_path, &strImagePath);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
	}

	if (strImagePath != NULL)
		strncpy(contactInfo.imagePath , strImagePath, MAX_IMAGE_PATH_LEN);

	MSG_DEBUG("Image Path [%s]", contactInfo.imagePath);

	// Name Info
	contacts_record_h name = NULL;

	ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.name, 0, &name);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
	} else {
		char* strName = NULL;
		ret = contacts_record_get_str_p(name, _contacts_name.first, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(contactInfo.firstName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.last, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(contactInfo.lastName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.addition, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(contactInfo.middleName, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.prefix, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(contactInfo.prefix, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		ret = contacts_record_get_str_p(name, _contacts_name.suffix, &strName);
		if (ret != CONTACTS_ERROR_NONE) {
			MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
		} else {
			if (strName != NULL) {
				MSG_DEBUG("strName [%s]", strName);
				strncpy(contactInfo.suffix, strName, MAX_DISPLAY_NAME_LEN);
			}
		}

		MSG_SEC_DEBUG("First Name : [%s], Last Name : [%s]", contactInfo.firstName, contactInfo.lastName);
	}

	MsgStoClearContactInfo(&ContactDbHandle, index);
#endif

	int count = 0;
	ret = contacts_record_get_child_record_count(contact, _contacts_contact.number, &count);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_child_record_count() Error [%d]", ret);
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return false;
	}

	if (count > 0) {
		for (int i=0; i < count; i++) {
			MSG_DEBUG("Add Contact Data");

			contacts_record_h number = NULL;

			ret = contacts_record_get_child_record_at_p(contact, _contacts_contact.number, i, &number);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_child_record_at_p() Error [%d]", ret);
				contacts_record_destroy(contact, true);
				MsgZoneRevert();
				return false;
			}

			char* strNumber = NULL;
			ret = contacts_record_get_str_p(number, _contacts_number.number, &strNumber);
			if (ret != CONTACTS_ERROR_NONE) {
				MSG_DEBUG("contacts_record_get_str_p() Error [%d]", ret);
				contacts_record_destroy(contact, true);
				MsgZoneRevert();
				return false;
			}

			if (strNumber != NULL) {
				MSG_DEBUG("Number [%s]", strNumber);
				int strNumberLen = strlen(strNumber);
				char normalizeNum[strNumberLen+1];
				memset(normalizeNum, 0x00, sizeof(normalizeNum));
				if (strNumberLen > 0) {
					normalizeNumber(strNumber, normalizeNum, strNumberLen);
					MSG_DEBUG("normalizeNum [%s]", normalizeNum);
				}

				MSG_CONTACT_INFO_S contactInfo;
				memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

				MSG_ADDRESS_INFO_S addrInfo;
				memset(&addrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));
				strncpy(addrInfo.addressVal, normalizeNum, MAX_ADDRESS_VAL_LEN);

				if(MsgGetContactInfo(&addrInfo, &contactInfo) == MSG_SUCCESS) {
					if (MsgInsertContact(&contactInfo, normalizeNum)) {
						MsgStoSetConversationDisplayName(&ContactDbHandle, (int)contactInfo.contactId);
					} else {
						MSG_DEBUG("MsgInsertContact fail.");
					}
				}
			}
		}
	} else {// No phone number in contact
		contacts_record_destroy(contact, true);
		MsgZoneRevert();
		return true;
	}

//	MsgStoSetConversationDisplayName(&ContactDbHandle, index);

	contacts_record_destroy(contact, true);

	MsgZoneRevert();

	return true;
}


bool MsgDeleteContact(int index)
{
//	if (MsgStoClearContactInfo(&ContactDbHandle, index) != MSG_SUCCESS)
	if (MsgStoResetContactInfo(&ContactDbHandle, index) != MSG_SUCCESS)
		return false;

	return true;
}

int MsgGetContactNameOrder()
{
	MsgZoneChange();

//	int ret = CONTACTS_ERROR_NONE;
	contacts_name_display_order_e order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;

//	if (MsgOpenContactSvc() != MSG_SUCCESS) {
//		MSG_DEBUG("MsgOpenContactSvc fail.");
//	}
//
//	if (!isContactSvcConnected) {
//		MSG_DEBUG("Contact Service Not Opened.");
//	} else {
//		ret = contacts_setting_get_name_display_order(&order);
//		if (ret != CONTACTS_ERROR_NONE) {
//			MSG_DEBUG("contacts_setting_get_name_display_order() Error [%d]", ret);
//		}
//	}

	MsgZoneRevert();

	return (int)order;
}


msg_error_t MsgGetContactStyleDisplayName(const char *first, const char *last, const char *middle, const char *prefix, const char *suffix, int contactNameOrder, char *displayName, unsigned int size)
{
	if (first == NULL || last == NULL || middle == NULL || prefix == NULL || suffix == NULL || displayName == NULL || size ==0) {
		MSG_DEBUG("Invalid parameter.");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (contactNameOrder == CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST) {
		if (strlen(prefix) > 0) {
			strncpy(displayName, prefix, size);
		}

		if (strlen(first) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));
			strncat(displayName, first, size-strlen(displayName));
		}

		if (strlen(middle) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));

			strncat(displayName, middle, size-strlen(displayName));
		}

		if (strlen(last) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));
			strncat(displayName, last, size-strlen(displayName));
		}

		if (strlen(suffix) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, ", ", size-strlen(displayName));
			strncat(displayName, suffix, size-strlen(displayName));
		}
	} else {
		if (strlen(prefix) > 0) {
			strncpy(displayName, prefix, size);
		}

		if (strlen(last) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));
			strncat(displayName, last, size-strlen(displayName));

			if (strlen(first) > 0 || strlen(middle) > 0 || strlen(suffix) > 0)
				strncat(displayName, ",", size-strlen(displayName));
		}

		if (strlen(first) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));
			strncat(displayName, first, size-strlen(displayName));
		}

		if (strlen(middle) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, " ", size-strlen(displayName));
			strncat(displayName, middle, size-strlen(displayName));
		}

		if (strlen(suffix) > 0) {
			if (strlen(displayName) > 0) strncat(displayName, ", ", size-strlen(displayName));
			strncat(displayName, suffix, size-strlen(displayName));
		}
	}

	MSG_SEC_DEBUG("displayName [%s]", displayName);

	return MSG_SUCCESS;
}


void MsgAddPhoneLog(const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MsgZoneChange();

	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		MsgZoneRevert();
		return;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return;
	}

	if (pMsgInfo->nAddressCnt < 1) {
		MSG_DEBUG("address count is [%d]", pMsgInfo->nAddressCnt);
		MsgZoneRevert();
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
			MSG_SEC_DEBUG("msgText : %s", strText);
		} else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE) {
			if (strlen(pMsgInfo->subject) > 0 || pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
				strncpy(strText, pMsgInfo->subject, 100);
				MSG_SEC_DEBUG("subject : %s", strText);
			} else {
				strncpy(strText, pMsgInfo->msgText, 100);
				MSG_SEC_DEBUG("msgText : %s", strText);
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

	MsgZoneRevert();
}


void MsgDeletePhoneLog(msg_message_id_t msgId)
{
	MsgZoneChange();

	msg_error_t err = MSG_SUCCESS;

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		MsgZoneRevert();
		return;
	}

	MSG_DEBUG("MsgDeletePhoneLog [%d]", msgId);

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return;
	}

	int ret = CONTACTS_ERROR_NONE;
	int index = 0;
	int count = 0;
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

	MsgZoneRevert();
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


bool checkBlockingMode(char *address, bool *pisFavorites)
{
	MsgZoneChange();

	msg_error_t err = MSG_SUCCESS;

	if (pisFavorites != NULL) *pisFavorites = false;

	bool isBlockModeOn = false;
	bool isblock = true;

	MsgSettingGetBool(VCONFKEY_SETAPPL_BLOCKINGMODE_NOTIFICATIONS, &isBlockModeOn);

	int blockModeType = -1;

	blockModeType = MsgSettingGetInt(VCONFKEY_SETAPPL_BLOCKINGMODE_ALLOWED_CONTACT_TYPE);

	if (!isBlockModeOn)
		isblock = false;
	else if (blockModeType < 0)
		isblock = false;


	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		MsgZoneRevert();
		return isblock;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
		MsgZoneRevert();
		return isblock;
	}

	MSG_SEC_DEBUG("Address Value [%s]", address);

	if (strlen(address) > (MAX_PHONE_NUMBER_LEN+1)) {
		MSG_SEC_DEBUG("Phone Number is too long [%s]", address);
		MsgZoneRevert();
		return isblock;
	}

	int ret = 0;
	int personId = 0;
	bool isFavorites = false;
	int count = 0;
	contacts_query_h query = NULL;
	contacts_filter_h filter = NULL;
	contacts_list_h personList = NULL;

	ret = contacts_query_create(_contacts_person_number._uri, &query);
	ret = contacts_filter_create(_contacts_person_number._uri, &filter);

	ret = contacts_filter_add_str(filter, _contacts_person_number.number_filter, CONTACTS_MATCH_EXACTLY, address);

	ret = contacts_query_set_filter(query, filter);
	ret = contacts_db_get_records_with_query(query, 0, 1, &personList);

	contacts_query_destroy(query);
	contacts_filter_destroy(filter);

	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_db_get_records_with_query() Error [%d]", ret);
		contacts_list_destroy(personList, true);
		MsgZoneRevert();
		return isblock;
	}

	ret = contacts_list_get_count(personList, &count);

	if (count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("No Serach Data from Contact Service.");
		contacts_list_destroy(personList, true);
		MsgZoneRevert();
		return isblock;
	} else if (ret == CONTACTS_ERROR_NONE && count > 0
			&& blockModeType == 1) { // For All contacts allow in blocking mode.
		isblock = false;
	}

	contacts_record_h person = NULL;

	ret = contacts_list_get_current_record_p(personList, &person);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_list_get_current_record_p() Error [%d]", ret);
		contacts_list_destroy(personList, true);
		MsgZoneRevert();
		return isblock;
	}

	ret = contacts_record_get_int(person, _contacts_person_number.person_id, &personId);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
		contacts_list_destroy(personList, true);
		MsgZoneRevert();
		return isblock;
	}

	MSG_DEBUG("personId [%d]", personId);

	ret = contacts_record_get_bool(person, _contacts_person_number.is_favorite, &isFavorites);
	if (ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("contacts_record_get_int() Error [%d]", ret);
		contacts_list_destroy(personList, true);
		MsgZoneRevert();
		return isblock;
	}

	contacts_list_destroy(personList, true);

	switch (blockModeType)
	{
	case 2: // For Favorites allow in blocking mode.
	{
		if (isFavorites) isblock = false;
		break;
	}
	case 3: // For Custom allow in blocking mode.
	{
		char *allowList = MsgSettingGetString(VCONFKEY_SETAPPL_BLOCKINGMODE_ALLOWED_CONTACT_LIST);
		char *personIdStr = strtok (allowList," ,");
		while (personIdStr != NULL)
		{
			MSG_DEBUG("personIdStr [%s]", personIdStr);
			if (personId == atoi(personIdStr)) {
				MSG_DEBUG("In allow list.");
				isblock = false;
				break;
			}
			personIdStr = strtok (NULL, " ,");
		}

		if (allowList) {
			free(allowList);
			allowList = NULL;
		}

		break;
	}
	default: // Wrong blocking mode type.
		break;
	}

	if (pisFavorites != NULL) *pisFavorites = isFavorites;

	MsgZoneRevert();

	return isblock;
}
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED

int MsgContactGetMinMatchDigit()
{
	return phonenumberMinMatchDigit;
}


void MsgConvertNumber(const char* pSrcNum, char* pDestNum, int destSize)
{
	int len;
	const char *temp_number;

	if ('+' == pSrcNum[0]) {
		len = countryCodeLength(&pSrcNum[1]);
		temp_number = pSrcNum + len +1;
	} else if ('0' == pSrcNum[0]) {
		if ('0' == pSrcNum[1]) {
			len = countryCodeLength(&pSrcNum[2]);
			temp_number = pSrcNum + len +2;
		} else {
			temp_number = pSrcNum+1;
		}
	} else {
		temp_number = pSrcNum;
	}

	strncpy(pDestNum, temp_number, destSize);
}


bool MsgIsNumber(const char* pSrc)
{
	int len = strlen(pSrc);

	for(int i = 0; i < len; ++i){

		if(i == 0 && pSrc[i] == '+')
			continue;

		if(pSrc[i] < '0' || pSrc[i] >'9'){
			return false;
		}
	}

	return true;
}

