/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
	#include <appsvc.h>
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

	appsvc_set_appid(args, "org.tizen.msg-ui-class0");

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

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	bundle_add(args, "type", "new_msg");

	snprintf(tempId, 5, "%d", pMsg->msgId);
	bundle_add(args, "msgId", tempId);

	MSG_END();
}


void MsgSmsMWINoti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	appsvc_set_appid(args, "org.tizen.call");

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "MWI Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New MWI Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
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

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	bundle_add(args, "launch-type", "MO");
	bundle_add(args, "number", pMsg->addressList[0].addressVal);

	MSG_END();
}


void MsgSmsVoiceNoti(MSG_MESSAGE_INFO_S* pMsg)
{
	MSG_BEGIN();

	int notiId = 0;
	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	bundle* args;

	args = bundle_create();

	notiId = MsgSettingGetInt(VOICE_NOTI_ID_1);

	if (notiId > 0) {
		noti = notification_load(NULL, notiId);
		if (noti == NULL)
			MSG_DEBUG("notification_load is failed.");
	}

	if (noti == NULL) {
		noti = notification_create(NOTIFICATION_TYPE_NOTI);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			if (args != NULL) bundle_free(args);
			return;
		}

		notiId = 0;
	}

	appsvc_set_appid(args, "org.tizen.call");

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Voicemail", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Voicemail", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
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

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	bundle_add(args, "launch-type", "MO");
	bundle_add(args, "number", pMsg->addressList[0].addressVal);

	if (args != NULL) {
		noti_err = notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, args);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_execute_option : %d", noti_err);
		}
	}

	if (notiId > 0) {
		noti_err = notification_update(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_update");
		}
	} else {
		noti_err = notification_insert(noti, &notiId);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_insert");
		}

		if (MsgSettingSetInt(VOICE_NOTI_ID_1, notiId) == MSG_SUCCESS)
			MSG_DEBUG("Insert VOICE_NOTI_ID_1 [%d]", notiId);
		else
			MSG_DEBUG("MsgSettingSetInt fail: VOICE_NOTI_ID_1");
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	if (args != NULL) {
		bundle_free(args);
	}

	MsgChangePmState();

	MSG_END();
}


void MsgSmsReportNoti(MSG_MESSAGE_INFO_S* pMsg, notification_h noti, bundle* args)
{
	MSG_BEGIN();

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Delivery Report", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if(pMsg->networkStatus == MSG_NETWORK_DELIVER_SUCCESS)
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "Message Delivered", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "Message delivery failed", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	// get contact info.
	MSG_CONTACT_INFO_S contactInfo;
	memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));
	MsgGetContactInfo(&(pMsg->addressList[0]), &contactInfo);

	int order = MsgGetContactNameOrder();

	if (order == 0) {
		if (contactInfo.firstName[0] != '\0') {
			strncpy(pMsg->addressList[0].displayName, contactInfo.firstName, MAX_DISPLAY_NAME_LEN);
		}

		if (contactInfo.lastName[0] != '\0') {
			strncat(pMsg->addressList[0].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
			strncat(pMsg->addressList[0].displayName, contactInfo.lastName, MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
		}
	} else if (order == 1) {
		if (contactInfo.lastName[0] != '\0') {
			strncpy(pMsg->addressList[0].displayName, contactInfo.lastName, MAX_DISPLAY_NAME_LEN);
			strncat(pMsg->addressList[0].displayName, " ", MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
		}

		if (contactInfo.firstName[0] != '\0') {
			strncat(pMsg->addressList[0].displayName, contactInfo.firstName, MAX_DISPLAY_NAME_LEN-strlen(pMsg->addressList[0].displayName));
		}
	}

	if (pMsg->addressList[0].displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, pMsg->addressList[0].displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message delivered.", pMsg->msgText, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	// set launch type
	noti_err = notification_set_property(noti, NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_set_display_applist");

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

	switch(pMsg->msgType.subType)
	{
	case MSG_CB_SMS :
		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_free");
		}
		MsgRefreshCBNoti(true);
		return MSG_SUCCESS;
		break;
	case MSG_MWI_FAX_SMS :
	case MSG_MWI_EMAIL_SMS :
	case MSG_MWI_OTHER_SMS :
		args = bundle_create();
		MsgSmsMWINoti(pMsg, noti, args);
		break;
	case MSG_MWI_VOICE_SMS :
		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_free");
		}
		MsgSmsVoiceNoti(pMsg);
		return MSG_SUCCESS;
		break;
	case MSG_STATUS_REPORT_SMS :
		//args = bundle_create();
		MsgSmsReportNoti(pMsg, noti, NULL);
		break;
	case MSG_NORMAL_SMS :
		if (pMsg->msgType.classType == MSG_CLASS_0) {
			args = bundle_create();
			MsgSmsClass0Noti(pMsg, noti, args);
			break;
		} // Do not break here.
	default :
		noti_err = notification_free(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_free");
		}
		MsgRefreshNoti(true);
		return MSG_SUCCESS;
		break;
	}

	if (args != NULL) {
		// set execute option
		noti_err = notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, args);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_execute_option : %d", noti_err);
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
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(addressVal, 0x00, sizeof(addressVal));
	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));
	memset(displayName, 0x00, sizeof(displayName));
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

	noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
	}

	if (pMsg->msgType.subType == MSG_DELIVERYIND_MMS) { // MMS delivery report

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "New message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "Delivery Report", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	} else if (pMsg->msgType.subType == MSG_READORGIND_MMS) {  // MMS read report

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "New message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "Read Report", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	}

	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set launch type
	noti_err = notification_set_property(noti, NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_set_display_applist");

	if (pMsg->msgType.subType == MSG_DELIVERYIND_MMS) {

		switch(report_status_value) {
		case MSG_DELIVERY_REPORT_NONE:
			noti_err = notification_free(noti);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_free");
			}

			return MSG_ERR_UNKNOWN;

		case MSG_DELIVERY_REPORT_EXPIRED:
			MSG_DEBUG("Message expired.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message expired", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message expired", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_REJECTED:
			MSG_DEBUG("Message rejected.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message rejected", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message rejected", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_DEFERRED:
			MSG_DEBUG("Message deferred.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message deferred", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message deferred", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_UNRECOGNISED:
			MSG_DEBUG("Message unrecognised.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message unrecognised", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message unrecognised", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_INDETERMINATE:
			MSG_DEBUG("Message indeterminate.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message indeterminate", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message indeterminate", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_FORWARDED:
			MSG_DEBUG("Message forwarded.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message forwarded", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message forwarded", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_UNREACHABLE:
			MSG_DEBUG("Message unreachable.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message unreachable", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message unreachable", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		case MSG_DELIVERY_REPORT_ERROR:
			MSG_DEBUG("Message error.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message error", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message error", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		default :
			MSG_DEBUG("Message delivered.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message delivered", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, "Message delivered", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;
		}

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
			MSG_DEBUG("Message deleted.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message deleted", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;

		default :
			MSG_DEBUG("Message read.");
			notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, "Message read", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
			break;
		}

		// set time.
		notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, pMsg->displayTime);

		// set led.
		notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

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
	bundle* args = NULL;
	bundle* reply = NULL;

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

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.CONV_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, "
			"B.DISPLAY_TIME, A.CONTACT_ID, A.IMAGE_PATH, B.MSG_ID, B.MSG_TEXT, B.SUBJECT, B.MAIN_TYPE "
			"FROM %s A, %s B WHERE A.CONV_ID=B.CONV_ID AND B.READ_STATUS=0 AND B.FOLDER_ID=%d AND "
			"(B.STORAGE_ID = %d OR B.STORAGE_ID = %d) "
			"ORDER BY B.DISPLAY_TIME DESC;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_INBOX_ID, MSG_STORAGE_PHONE, MSG_STORAGE_SIM);

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
			noti_err = notification_delete_by_priv_id("8r4r5ddzzn.Messages", NOTIFICATION_TYPE_NOTI, notiPrivId);
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

	int unreadMsgCnt = 0;
#if 0
	unreadMsgCnt = MsgStoGetUnreadCnt(&dbhandler, MSG_SMS_TYPE);
	unreadMsgCnt += MsgStoGetUnreadCnt(&dbhandler, MSG_MMS_TYPE);
#else
	if (dbhandler.getTable(sqlQuery , &unreadMsgCnt) != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable");
		dbhandler.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	dbhandler.freeTable();
#endif

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
			return MSG_ERR_UNKNOWN;
		}

		notiPrivId = 0;
	}

	// set pkg name.
	noti_err = notification_set_pkgname(noti, "8r4r5ddzzn.Messages");
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_pkgname : %d", noti_err);
	}

	// create bundle
	args = bundle_create();

	appsvc_set_appid(args, "8r4r5ddzzn.Messages");

	// Set bundle values
	memset(&tempId, 0x00, sizeof(tempId));
	bundle_add(args, "type", "new_msg");
	snprintf(tempId, 5, "%d", msg.msgId);
	bundle_add(args, "msgId", tempId);

	if (unreadMsgCnt > 1) {

		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_MULTIPLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		char unreadMsgCntStr[5] = {0,};
		snprintf(unreadMsgCntStr, 5, "%d", unreadMsgCnt);
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadMsgCntStr, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Messages", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		// set execute option
		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, NULL, NULL, args);

		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_RESPONDING, NULL, NULL, NULL);

	} else {

		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_FOR_LOCK, NORMAL_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		// set execute option
		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, args);

		// Set responding option
		reply = bundle_create();
		appsvc_set_appid(reply, "8r4r5ddzzn.Messages");

		bundle_add(reply, "type", "reply");
		bundle_add(reply, "show_list", "list_show");

		memset(&tempId, 0x00, sizeof(tempId));
		snprintf(tempId, 5, "%d", msg.msgId);
		bundle_add(reply, "msgId", tempId);

		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_RESPONDING, NULL, NULL, reply);
	}


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

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	notification_set_property(noti, NOTIFICATION_PROP_DISABLE_AUTO_DELETE);

	if (bWithTicker)
		noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_ALL);
	else
		noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_TICKER);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_display_applist : %d", noti_err);
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

	if (args != NULL) {
		bundle_free(args);
	}

	if (reply != NULL) {
		bundle_free(reply);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgRefreshCBNoti(bool bWithTicker)
{
	MsgDbHandler dbhandler;

	int notiCbId = MsgSettingGetInt(CB_NOTI_PRIV_ID);

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	bundle* args = NULL;

	int contactId = 0;
	msg_thread_id_t threadId = 0;
	msg_message_id_t msgId = 0;
	time_t msgTime = 0;
	char tempId[6];
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char thumbPath[MAX_IMAGE_PATH_LEN+1];
	char msgText[MAX_MSG_TEXT_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(tempId, 0x00, sizeof(tempId));
	memset(addressVal, 0x00, sizeof(addressVal));
	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));
	memset(displayName, 0x00, sizeof(displayName));
	memset(thumbPath, 0x00, sizeof(thumbPath));
	memset(msgText, 0x00, sizeof(msgText));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.CONV_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, "
			"B.DISPLAY_TIME, A.CONTACT_ID, A.IMAGE_PATH, B.MSG_ID, B.MSG_TEXT "
			"FROM %s A, %s B WHERE A.CONV_ID=B.CONV_ID AND B.READ_STATUS=0 AND B.FOLDER_ID=%d AND B.STORAGE_ID = %d "
			"ORDER BY B.DISPLAY_TIME DESC;",
			MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MSG_CBMSGBOX_ID, MSG_STORAGE_PHONE);

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

		msgId = dbhandler.columnInt(8);

		strncpy(msgText, (char*)dbhandler.columnText(9), MAX_MSG_TEXT_LEN);

		MSG_DEBUG("unread CB message [%d].", msgId);
	} else {

		MSG_DEBUG("No unread CB message.");
		MSG_DEBUG("notiCbId [%d]", notiCbId);

		dbhandler.finalizeQuery();

		// No unread message.
		if (notiCbId > 0) {
			noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, notiCbId);
			if (noti_err != NOTIFICATION_ERROR_NONE) {
				MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
			}
		}

		notiCbId = 0;

		if(MsgSettingSetInt(CB_NOTI_PRIV_ID, notiCbId) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt fail : CB_NOTI_PRIV_ID");

		return MSG_ERR_DB_STEP;
	}

	dbhandler.finalizeQuery();

	int unreadCbMsgCnt = 0;

	if (dbhandler.getTable(sqlQuery, &unreadCbMsgCnt) != MSG_SUCCESS) {
		MSG_DEBUG("getTable is failed");
		dbhandler.freeTable();
		return MSG_ERR_DB_GETTABLE;
	}

	MSG_DEBUG("notiCbId [%d], unreadCbMsgCnt [%d]", notiCbId, unreadCbMsgCnt);

	if (notiCbId > 0) {
		noti = notification_load(NULL, notiCbId);
		if (noti == NULL)
			MSG_DEBUG("notification_load is failed.");
	}

	if (noti == NULL) {
		noti = notification_create(NOTIFICATION_TYPE_NOTI);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			return MSG_ERR_UNKNOWN;
		}
		notiCbId = 0;
	}

	args = bundle_create();

	appsvc_set_appid(args, "8r4r5ddzzn.Messages");

	// Set bundle values
	memset(&tempId, 0x00, sizeof(tempId));
	bundle_add(args, "type", "new_msg");
	snprintf(tempId, 5, "%d", msgId);
	bundle_add(args, "msgId", tempId);

	if (unreadCbMsgCnt > 1) {
		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_MULTIPLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Broadcast message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Messages", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		char unreadCbMsgCntStr[5] = {0,};
		snprintf(unreadCbMsgCntStr, 5, "%d", unreadCbMsgCnt);
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, unreadCbMsgCntStr, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, NULL, NULL, args);
	} else {
		noti_err = notification_set_layout(noti, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_layout : %d", noti_err);
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, "Broadcast message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, "New Message", NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		notification_set_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, args);
	}


	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_1, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_INFO_2, msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	// set time.
	notification_set_time_to_text(noti, NOTIFICATION_TEXT_TYPE_INFO_SUB_1, msgTime);

	// set icon image
	notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);

	// set led.
	notification_set_led(noti, NOTIFICATION_LED_OP_ON, 0x00);

	if (bWithTicker)
		noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_ALL^NOTIFICATION_DISPLAY_APP_LOCK);
	else
		noti_err = notification_set_display_applist(noti, NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY|NOTIFICATION_DISPLAY_APP_INDICATOR);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_display_applist : %d", noti_err);
	}

	if (notiCbId > 0) {
		noti_err = notification_update(noti);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_update");
		}
	} else {
		noti_err = notification_insert(noti, &notiCbId);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_insert");
		}

		if (MsgSettingSetInt(CB_NOTI_PRIV_ID, notiCbId) != MSG_SUCCESS)
			MSG_DEBUG("MsgSettingSetInt fail: CB_NOTI_PRIV_ID");
		MSG_DEBUG("Insert notiCbId [%d]", notiCbId);
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	if (args != NULL) {
		bundle_free(args);
	}

	return MSG_SUCCESS;
}


msg_error_t MsgCleanAndResetNoti()
{
	msg_error_t err = MSG_SUCCESS;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti_err = notification_delete_all_by_type("8r4r5ddzzn.Messages", NOTIFICATION_TYPE_NOTI);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_delete_all_by_type noti_err [%d]", noti_err);
	}

	err = MsgRefreshNoti(false);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to MsgRefreshNoti : [err=%d]", err);
	}

	err = MsgRefreshCBNoti(false);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to MsgRefreshCBNoti : [err=%d]", err);
	}

	return err;
}


msg_error_t MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg)
{
	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		MSG_DEBUG("notification_new is failed.");
		return MSG_ERR_UNKNOWN;
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NORMAL_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
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

	noti_err = notification_set_property(noti, NOTIFICATION_PROP_DISABLE_APP_LAUNCH);
	if (noti_err != NOTIFICATION_ERROR_NONE)
		MSG_DEBUG("Fail to notification_set_display_applist");

	noti_err = notification_insert(noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_insert");
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	return MSG_SUCCESS;
}


msg_error_t MsgInsertBadge(unsigned int unreadMsgCnt)
{
	return MSG_SUCCESS;
}


msg_error_t MsgClearVoiceNoti(MSG_SUB_TYPE_T subType)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	int notiId = 0;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	if (subType == MSG_MWI_VOICE_SMS) {
		notiId = MsgSettingGetInt(VOICE_NOTI_ID_1);
	} else {
		return MSG_ERR_UNKNOWN;
	}

	if (notiId>0) {
		noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, notiId);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_delete_by_priv_id : %d", noti_err);
			err = MSG_ERR_UNKNOWN;
		}
	}

	notiId = 0;

	if (subType == MSG_MWI_VOICE_SMS) {
		if (MsgSettingSetInt(VOICE_NOTI_ID_1, notiId) == MSG_SUCCESS)
			MSG_DEBUG("Insert VOICE_NOTI_ID_1 [%d]", notiId);
		else
			MSG_DEBUG("MsgSettingSetInt fail: VOICE_NOTI_ID_1");
	}

	MSG_END();

	return err;
}
