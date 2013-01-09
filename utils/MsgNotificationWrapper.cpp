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

#include "MsgDebug.h"
#include "MsgContact.h"
#include "MsgStorageTypes.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgNotificationWrapper.h"

extern "C"
{
	#include <notification.h>
}

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
void MsgSmsClass0Noti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	char tempId[6];

	memset(tempId, 0x00, sizeof(tempId));

	noti_err = notification_set_application(noti, "org.tizen.msg-ui-class0");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "CLASS 0 Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New CLASS 0 Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (pMsg->addressList[0].displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, pMsg->msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

	bundle_add(args, "type", "msg_id");

	snprintf(tempId, 5, "%d", pMsg->msgId);
	bundle_add(args, "msgId", tempId);

	MSG_END();
}

void MsgSmsCBNoti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	char tempId[6];

	memset(tempId, 0x00, sizeof(tempId));

	noti_err = notification_set_application(noti, "org.tizen.message");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, CB_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "CB Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New CB Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (pMsg->addressList[0].displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, pMsg->msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

	memset(&tempId, 0x00, sizeof(tempId));

	bundle_add(args, "type", "msg_id");

	snprintf(tempId, 5, "%d", pMsg->msgId);
	bundle_add(args, "msgId", tempId);

	MSG_END();
}

void MsgSmsVoiceNoti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_application(noti, "org.tizen.call");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Voice Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Voice Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, VOICE_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	if (pMsg->addressList[0].displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, pMsg->msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

	//FIXME :: Temp code for voice number, 2012.08.16 sangkoo.kim
	bundle_add(args, "launch-type", "MO");
	bundle_add(args, "number", pMsg->addressList[0].addressVal);

	MSG_END();
}


void MsgSmsReportNoti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	char tempId[6];

	memset(tempId, 0x00, sizeof(tempId));

	noti_err = notification_set_application(noti, "org.tizen.message");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}
	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Delivery Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Delivery Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NOTI_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	if (pMsg->addressList[0].displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, pMsg->msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

	// get msg id
	MsgDbHandler dbhandler;
	char sqlQuery[MAX_QUERY_LEN+1];
	memset(sqlQuery, 0x00, sizeof(sqlQuery));
	snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT MSG_ID "
			"FROM %s "
			"WHERE CONV_ID IN (SELECT CONV_ID FROM %s WHERE ADDRESS_VAL LIKE '%%%s') AND FOLDER_ID=%d "
			"ORDER BY DISPLAY_TIME DESC;"
			, MSGFW_MESSAGE_TABLE_NAME, MSGFW_ADDRESS_TABLE_NAME, pMsg->addressList[0].addressVal, MSG_SENTBOX_ID);

	MSG_DEBUG("sqlQuery - %s", sqlQuery);

	if (dbhandler.prepareQuery(sqlQuery) == MSG_SUCCESS) {

		if (dbhandler.stepQuery() == MSG_ERR_DB_ROW) {
			memset(&tempId, 0x00, sizeof(tempId));

			bundle_add(args, "type", "report");

			snprintf(tempId, 5, "%d", dbhandler.columnInt(0));
			bundle_add(args, "msgId", tempId);

			MSG_DEBUG("msgId [%s] add.", tempId);
		}

		dbhandler.finalizeQuery();
	}

	MSG_END();
}


msg_error_t MsgInsertNoti(MSG_MESSAGE_INFO_S* pMsg)
{
	MSG_DEBUG("Start to Insert Notification.");
	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	bundle* args = NULL;

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		MSG_DEBUG("notification_create is failed.");
		return MSG_ERR_UNKNOWN;
	}

	if (pMsg->msgType.mainType == MSG_SMS_TYPE) {
		switch(pMsg->msgType.subType)
		{
		case MSG_CB_SMS :
			args = bundle_create();
			MsgSmsCBNoti(pMsg, noti, args);
			break;
		case MSG_MWI_VOICE_SMS :
		case MSG_MWI_FAX_SMS :
		case MSG_MWI_EMAIL_SMS :
		case MSG_MWI_OTHER_SMS :
			args = bundle_create();
			MsgSmsVoiceNoti(pMsg, noti, args);
			break;
		case MSG_STATUS_REPORT_SMS :
			args = bundle_create();
			MsgSmsReportNoti(pMsg, noti, args);
			break;
		default :
			MsgRefreshNoti(true);
			noti_err = notification_free(noti);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_free");
			}
			return MSG_SUCCESS;
			break;
		}
	} else if (pMsg->msgType.mainType == MSG_MMS_TYPE) {
		switch(pMsg->msgType.subType)
		{
		default :
			MsgRefreshNoti(true);
			noti_err = notification_free(noti);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_free");
			}
			return MSG_SUCCESS;
			break;
		}
	} else {
		MSG_DEBUG("Message type does not match.");

		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_free");
		}

		return MSG_ERR_INVALID_PARAMETER;
	}

	if (args != NULL) {
		noti_err = notification_set_args(noti, args, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_args : %d", noti_err);
		}
	}

	noti_err = notification_insert(noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_insert");
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	if (args != NULL) {
		bundle_free(args);
	}

	MsgChangePmState();

	return MSG_SUCCESS;
}


msg_error_t MsgInsertMmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg)
{

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char contents[MAX_DISPLAY_NAME_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(addressVal, 0x00, sizeof(addressVal));
	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));
	memset(displayName, 0x00, sizeof(displayName));
	memset(contents, 0x00, sizeof(contents));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	int report_status_type;
	int report_status_value;

	if (pMsg->msgType.subType == MSG_DELIVERYIND_MMS) {
		report_status_type = MSG_REPORT_TYPE_DELIVERY;
		MSG_DEBUG("mms subtype is Delivery Report type");
	} else if (pMsg->msgType.subType == MSG_READORGIND_MMS) {
		report_status_type = MSG_REPORT_TYPE_READ;
		MSG_DEBUG("mms subtype is Read Report type");
	} else {
		MSG_DEBUG("No matching subtype. subtype [%d]", pMsg->msgType.subType);
		return MSG_SUCCESS;
	}

	MSG_ADDRESS_INFO_S *address_info_s = &pMsg->addressList[0];
	MSG_DEBUG("address info : %s, type : %d", address_info_s->addressVal, address_info_s->addressType);

	if (address_info_s->addressType == MSG_ADDRESS_TYPE_PLMN) {

		if (strlen(address_info_s->addressVal) > MAX_PRECONFIG_NUM) {
			char newPhoneNum[MAX_PRECONFIG_NUM+1];

			memset(newPhoneNum, 0x00, sizeof(newPhoneNum));

			MsgConvertNumber(address_info_s->addressVal, newPhoneNum);

			snprintf(sqlQuery, sizeof(sqlQuery),
						"SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, B.STATUS "
						"FROM %s A, %s B "
						"WHERE B.MSG_ID=%d AND B.STATUS_TYPE=%d AND A.ADDRESS_VAL LIKE '%%%s' AND B.ADDRESS_VAL LIKE \'%%%s\';"
						, MSGFW_ADDRESS_TABLE_NAME, MSGFW_REPORT_TABLE_NAME, pMsg->msgId, report_status_type, newPhoneNum, newPhoneNum);
		} else {

			snprintf(sqlQuery, sizeof(sqlQuery),
				"SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, B.STATUS "
				"FROM %s A, %s B "
				"WHERE B.MSG_ID=%d AND B.STATUS_TYPE=%d AND A.ADDRESS_VAL LIKE '%s' AND B.ADDRESS_VAL LIKE '%s';"
				, MSGFW_ADDRESS_TABLE_NAME, MSGFW_REPORT_TABLE_NAME, pMsg->msgId, report_status_type, address_info_s->addressVal, address_info_s->addressVal);
		}

	} else if (address_info_s->addressType == MSG_ADDRESS_TYPE_EMAIL) {//check full
		snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, B.STATUS "
			"FROM %s A, %s B "
			"WHERE B.MSG_ID=%d AND B.STATUS_TYPE=%d AND A.ADDRESS_VAL=\'%s\' AND B.ADDRESS_VAL LIKE \'%s\';"
			, MSGFW_ADDRESS_TABLE_NAME, MSGFW_REPORT_TABLE_NAME, pMsg->msgId, report_status_type, address_info_s->addressVal, address_info_s->addressVal);
	} else {
		snprintf(sqlQuery, sizeof(sqlQuery),
			"SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, B.STATUS "
			"FROM %s A, %s B "
			"WHERE B.MSG_ID=%d B.STATUS_TYPE=%d;"
			, MSGFW_ADDRESS_TABLE_NAME, MSGFW_REPORT_TABLE_NAME, pMsg->msgId, report_status_type);
	}

	MSG_DEBUG("sqlQuery = [%s]", sqlQuery);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (pDbHandle->columnText(0) != NULL) {
			strncpy(addressVal, (char*)pDbHandle->columnText(0), MAX_ADDRESS_VAL_LEN);
			MSG_DEBUG("addressVal is [%s]",addressVal);
		} else {
			MSG_DEBUG("address Val is Null");
		}

		char *pTempDisplayName = (char *)pDbHandle->columnText(1);
		if (pTempDisplayName != NULL && pTempDisplayName[0] != '\0') {
			strncpy(displayName, pTempDisplayName, MAX_DISPLAY_NAME_LEN);
		} else {
			if (pDbHandle->columnText(2) != NULL)
				strncpy(firstName, (char*)pDbHandle->columnText(2), MAX_DISPLAY_NAME_LEN);

			if (pDbHandle->columnText(3) != NULL)
				strncpy(lastName, (char*)pDbHandle->columnText(3), MAX_DISPLAY_NAME_LEN);

			int order = MsgGetContactNameOrder();

			if (order == 0) {
				if (firstName[0] != '\0') {
					strncpy(displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (lastName[0] != '\0') {
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
					strncat(displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			} else if (order == 1) {
				if (lastName[0] != '\0') {
					strncpy(displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}

				if (firstName[0] != '\0') {
					strncat(displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			}
		}

		report_status_value = pDbHandle->columnInt(4);
		MSG_DEBUG("report status [type = %d, value = %d]", report_status_type, report_status_value);
	} else {
		MSG_DEBUG("DB Query Result Fail");
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	noti = notification_create(NOTIFICATION_TYPE_NOTI);
	if (noti == NULL) {
		MSG_DEBUG("notification_create is failed.");
		return MSG_ERR_UNKNOWN;
	}

	noti_err = notification_set_application(noti, "org.tizen.message");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}
	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NOTI_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
//	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, msgTime);


	if (pMsg->msgType.subType == MSG_DELIVERYIND_MMS) {

		switch(report_status_value) {
		case MSG_DELIVERY_REPORT_NONE:
			noti_err = notification_free(noti);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_free");
			}

			return MSG_ERR_UNKNOWN;

		case MSG_DELIVERY_REPORT_EXPIRED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Expired.");
			break;

		case MSG_DELIVERY_REPORT_REJECTED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Rejected.");
			break;

		case MSG_DELIVERY_REPORT_DEFERRED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Deferred.");
			break;

		case MSG_DELIVERY_REPORT_UNRECOGNISED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Unrecognised.");
			break;

		case MSG_DELIVERY_REPORT_INDETERMINATE:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Indeterminate.");
			break;

		case MSG_DELIVERY_REPORT_FORWARDED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Forwarded.");
			break;

		case MSG_DELIVERY_REPORT_UNREACHABLE:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Unreachable.");
			break;

		case MSG_DELIVERY_REPORT_ERROR:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Error.");
			break;

		default :
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Delivered.");
			break;
		}

		MSG_DEBUG("content text : %s", contents);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		noti_err = notification_insert(noti, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE)
			MSG_DEBUG("Fail to notification_insert");

	} else if (pMsg->msgType.subType == MSG_READORGIND_MMS) {

		switch(report_status_value) {
		case MSG_READ_REPORT_NONE:
			noti_err = notification_free(noti);
			if (noti_err != NOTIFICATION_ERROR_NONE)
				MSG_DEBUG("Fail to notification_free");

			return MSG_ERR_UNKNOWN;

		case MSG_READ_REPORT_IS_DELETED:
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Deleted.");
			break;

		default :
			snprintf(contents, MAX_DISPLAY_NAME_LEN, "Read.");
			break;
		}

		MSG_DEBUG("content text : %s", contents);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		noti_err = notification_insert(noti, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_insert");
		}
	} else {
		MSG_DEBUG("No matching subtype. subtype [%d]", pMsg->msgType.subType);

		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_free");
		}
		return MSG_SUCCESS;
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	return MSG_SUCCESS;
}


msg_error_t MsgRefreshNoti(bool bWithTicker)
{

	MsgDbHandler dbhandler;
	MSG_MESSAGE_INFO_S msg = {0,};

	int notiPrivId = MsgSettingGetInt(NOTIFICATION_PRIV_ID);

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	bundle* args;

	int contactId = 0;
	msg_thread_id_t threadId = 0;
	time_t msgTime = 0;
	char tempId[6];
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char thumbPath[MAX_IMAGE_PATH_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(tempId, 0x00, sizeof(tempId));
	memset(addressVal, 0x00, sizeof(addressVal));
	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));
	memset(displayName, 0x00, sizeof(displayName));
	memset(thumbPath, 0x00, sizeof(thumbPath));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.CONV_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, \
			B.DISPLAY_TIME, A.CONTACT_ID, A.IMAGE_PATH, B.MSG_ID, B.MSG_TEXT, B.SUBJECT, B.MAIN_TYPE \
			FROM %s A, %s B WHERE A.CONV_ID=B.CONV_ID AND B.READ_STATUS=0 AND B.FOLDER_ID=%d ORDER BY B.DISPLAY_TIME DESC;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID);

	if (dbhandler.prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (dbhandler.stepQuery() == MSG_ERR_DB_ROW) {
		threadId = dbhandler.columnInt(0);

		if (dbhandler.columnText(1) != NULL)
			strncpy(addressVal, (char*)dbhandler.columnText(1), MAX_ADDRESS_VAL_LEN);


		char *pTempDisplayName = (char *)dbhandler.columnText(2);
		if (pTempDisplayName != NULL && pTempDisplayName[0] != '\0') {
			strncpy(displayName, pTempDisplayName, MAX_DISPLAY_NAME_LEN);
		} else {
			if (dbhandler.columnText(3) != NULL)
				strncpy(firstName, (char*)dbhandler.columnText(3), MAX_DISPLAY_NAME_LEN);

			if (dbhandler.columnText(4) != NULL)
				strncpy(lastName, (char*)dbhandler.columnText(4), MAX_DISPLAY_NAME_LEN);

			int order = MsgGetContactNameOrder();

			if (order == 0) {
				if (firstName[0] != '\0') {
					strncpy(displayName, firstName, MAX_DISPLAY_NAME_LEN);
				}

				if (lastName[0] != '\0') {
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
					strncat(displayName, lastName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			} else if (order == 1) {
				if (lastName[0] != '\0') {
					strncpy(displayName, lastName, MAX_DISPLAY_NAME_LEN);
					strncat(displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}

				if (firstName[0] != '\0') {
					strncat(displayName, firstName, MAX_DISPLAY_NAME_LEN-strlen(displayName));
				}
			}
		}

		msgTime = (time_t)dbhandler.columnInt(5);

		contactId = dbhandler.columnInt(6);

		strncpy(thumbPath, (char*)dbhandler.columnText(7), MAX_IMAGE_PATH_LEN);

		msg.msgId = dbhandler.columnInt(8);

		strncpy(msg.msgText, (char*)dbhandler.columnText(9), MAX_MSG_TEXT_LEN);

		strncpy(msg.subject, (char*)dbhandler.columnText(10), MAX_SUBJECT_LEN);

		msg.msgType.mainType = dbhandler.columnInt(11);

		MSG_DEBUG("unread message [%d].", msg.msgId);
	} else {

		MSG_DEBUG("No unread message.");
		MSG_DEBUG("notiPrivId [%d]", notiPrivId);

		dbhandler.finalizeQuery();

		// No unread message.
		if (notiPrivId > 0) {
			noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, notiPrivId);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
			}
		}

		notiPrivId = 0;

		if(MsgSettingSetInt(NOTIFICATION_PRIV_ID, notiPrivId) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt fail : NOTIFICATION_PRIV_ID");

		return MSG_ERR_DB_STEP;
	}

	dbhandler.finalizeQuery();


	args = bundle_create();

	int unreadMsgCnt = MsgStoGetUnreadCnt(&dbhandler, MSG_SMS_TYPE);
	unreadMsgCnt += MsgStoGetUnreadCnt(&dbhandler, MSG_MMS_TYPE);

	MSG_DEBUG("notiPrivId [%d], unreadMsgCnt [%d]", notiPrivId, unreadMsgCnt);

	if (notiPrivId > 0) {
		noti = notification_load(NULL, notiPrivId);
		if (noti == NULL)
			MSG_DEBUG("notification_load is failed.");
	}

	if (noti == NULL) {
		noti = notification_create(NOTIFICATION_TYPE_NOTI);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			bundle_free(args);
			return MSG_ERR_UNKNOWN;
		}

		notiPrivId = 0;
	}
	noti_err = notification_set_application(noti, "org.tizen.message");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}
	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}


	if (unreadMsgCnt > 1) {

		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_MULTIPLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		noti_err = notification_set_text_domain(noti, MSG_SYS_PACKAGE_NAME, MSG_SYS_LOCALEDIR);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_text_domain.");
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Message", MESSAGE, NOTIFICATION_VARIABLE_TYPE_NONE);

		char unreadMsgCntStr[5] = {0,};
		snprintf(unreadMsgCntStr, 5, "%d", unreadMsgCnt);
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Messages", NEW_MESSAGES, NOTIFICATION_VARIABLE_TYPE_NONE);

		if (displayName[0] == '\0')
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		else
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		if (msg.msgType.mainType == MSG_SMS_TYPE)
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2,msg.msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		else
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, msg.subject, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		// set time.
		notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, msgTime);

	} else {

		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		noti_err = notification_set_text_domain(noti, MSG_SYS_PACKAGE_NAME, MSG_SYS_LOCALEDIR);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_text_domain.");
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Message", MESSAGE, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Message", NEW_MESSAGE, NOTIFICATION_VARIABLE_TYPE_NONE);

		if (displayName[0] == '\0')
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		else
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		if (msg.msgType.mainType == MSG_SMS_TYPE)
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2,msg.msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		else
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, msg.subject, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		// set time.
		notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, msgTime);

	}
	{
		if (bWithTicker)
			noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_ALL);
		else
			noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER);

		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_display_applist : %d", noti_err);
		}
	}

	memset(&tempId, 0x00, sizeof(tempId));

	bundle_add(args, "type", "msg_id");

	snprintf(tempId, 5, "%d", msg.msgId);
	bundle_add(args, "msgId", tempId);

	if (args != NULL) {
		noti_err = notification_set_args(noti, args, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_args : %d", noti_err);
		}
	}

	if (notiPrivId > 0) {
		noti_err = notification_update(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_update");
		}
	} else {
		noti_err = notification_insert(noti, &notiPrivId);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_insert");
		}

		if (MsgSettingSetInt(NOTIFICATION_PRIV_ID, notiPrivId) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt fail: NOTIFICATION_PRIV_ID");
		MSG_DEBUG("Insert notiPrivId [%d]", notiPrivId);
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	bundle_free(args);

	return MSG_SUCCESS;
}

msg_error_t MsgCleanAndResetNoti()
{
	msg_error_t err = MSG_SUCCESS;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_delete_all_by_type("/usr/bin/msg-server", NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_delete_all_by_type noti_err [%d]", noti_err);
		return MSG_ERR_UNKNOWN;
	}

	err = MsgRefreshNoti(false);
	if (err != MSG_SUCCESS) {
			MSG_DEBUG("Fail to MsgRefreshNoti");
	}

	return err;
}


msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg)
{

	MSG_DEBUG("pTickerMsg [%s]", pTickerMsg);
	MSG_DEBUG("pLocaleTickerMsg [%s]", pLocaleTickerMsg);

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		MSG_DEBUG("notification_new is failed.");
		return MSG_ERR_UNKNOWN;
	}

	noti_err = notification_set_application(noti, "org.tizen.message");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	noti_err = notification_set_text_domain(noti, MSG_APP_PACKAGE_NAME, MSG_APP_LOCALEDIR);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_text_domain.");
	}

	noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, pTickerMsg, pLocaleTickerMsg, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_text : %d", noti_err);
	}

	noti_err = notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, pTickerMsg, pLocaleTickerMsg, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_text : %d", noti_err);
	}

	noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_TICKER);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_display_applist : %d", noti_err);
	}

	noti_err = notification_insert(noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_text_domain");
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_text_domain");
	}

	return MSG_SUCCESS;
}
msg_error_t MsgInsertBadge(unsigned int unreadMsgCnt)
{
	return MSG_SUCCESS;
}
