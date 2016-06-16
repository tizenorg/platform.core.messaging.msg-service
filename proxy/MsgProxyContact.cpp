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

#include <MsgProxyContact.h>
#include <MsgContact.h>
#include <MsgDebug.h>

#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
extern "C"
{
	#include <contacts.h>
}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
__thread bool isContactSvcConnected = false;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgOpenContactSvc()
{
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
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
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	return MSG_SUCCESS;
}


msg_error_t MsgCloseContactSvc()
{
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	int errCode = CONTACTS_ERROR_NONE;

	if (isContactSvcConnected) {
		errCode = contacts_disconnect();

		if (errCode == CONTACTS_ERROR_NONE) {
			MSG_DEBUG("Disconnect to Contact Service Success");
			isContactSvcConnected = false;
		} else {
			MSG_DEBUG("Disconnect to Contact Service Fail [%d]", errCode);
			return MSG_ERR_DB_DISCONNECT;
		}
	}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	return MSG_SUCCESS;
}

msg_error_t MsgGetContactSearchList(const char *pSearchVal, MSG_ADDRESS_INFO_S **pAddrInfo, int *count)
{
	MSG_BEGIN();
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED

	msg_error_t err = MSG_SUCCESS;

	*count = 0;

	if (pSearchVal == NULL) {
		MSG_DEBUG("pSearchVal is NULL.");
		return MSG_ERR_NULL_POINTER;
	}

	if (pAddrInfo == NULL) {
		MSG_DEBUG("pAddrInfo is NULL.");
		return MSG_ERR_NULL_POINTER;
	}

	if ((err = MsgOpenContactSvc()) != MSG_SUCCESS) {
		MSG_DEBUG("MsgOpenContactSvc fail.");
		return err;
	}

	if (!isContactSvcConnected) {
		MSG_DEBUG("Contact Service Not Opened.");
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
		return MSG_ERR_UNKNOWN;
	}

	ret = contacts_list_get_count(personNumbers, count);
	if (*count == 0 || ret != CONTACTS_ERROR_NONE) {
		MSG_DEBUG("No Serach Data from Contact Service.");
		*count = 0;
		contacts_query_destroy(query);
		contacts_filter_destroy(filter);
		contacts_list_destroy(personNumbers, true);
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
		} else if (normalizedNumber) {
			MSG_DEBUG("normalizedNumber [%s]", normalizedNumber);
			strncpy((*pAddrInfo)[index].addressVal, normalizedNumber, MAX_ADDRESS_VAL_LEN);
		}

		contacts_list_next(personNumbers);
		index++;
	}

	contacts_list_destroy(personNumbers, true);

#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
	MSG_END();

	return MSG_SUCCESS;
}
