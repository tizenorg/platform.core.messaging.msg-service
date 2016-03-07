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
#include <stdbool.h>
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
