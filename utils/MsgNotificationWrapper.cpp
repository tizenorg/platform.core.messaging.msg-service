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
#include "MsgNotificationWrapper.h"

extern "C"
{
	#include <notification.h>
}

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgInsertNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg)
{

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	bundle* args;

	int contactId = 0;
	int threadId = 0;
	time_t msgTime = 0;
	char tempId[6];
	char addressVal[MAX_ADDRESS_VAL_LEN+1];
	char firstName[MAX_DISPLAY_NAME_LEN+1], lastName[MAX_DISPLAY_NAME_LEN+1];
	char displayName[MAX_DISPLAY_NAME_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(tempId, 0x00, sizeof(tempId));
	memset(addressVal, 0x00, sizeof(addressVal));
	memset(firstName, 0x00, sizeof(firstName));
	memset(lastName, 0x00, sizeof(lastName));
	memset(displayName, 0x00, sizeof(displayName));
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_ID, A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, \
						B.DISPLAY_TIME, A.CONTACT_ID \
						FROM %s A, %s B \
				             WHERE B.MSG_ID=%d AND A.ADDRESS_ID=B.ADDRESS_ID;",
		MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		threadId = pDbHandle->columnInt(0);

		if (pDbHandle->columnText(1) != NULL)
			strncpy(addressVal, (char*)pDbHandle->columnText(1), MAX_ADDRESS_VAL_LEN);


		char *pTempDisplayName = (char *)pDbHandle->columnText(2);
		if (pTempDisplayName != NULL && pTempDisplayName[0] != '\0') {
			strncpy(displayName, pTempDisplayName, MAX_DISPLAY_NAME_LEN);
		} else {
			if (pDbHandle->columnText(3) != NULL)
				strncpy(firstName, (char*)pDbHandle->columnText(3), MAX_DISPLAY_NAME_LEN);

			if (pDbHandle->columnText(4) != NULL)
				strncpy(lastName, (char*)pDbHandle->columnText(4), MAX_DISPLAY_NAME_LEN);

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

		msgTime = (time_t)pDbHandle->columnInt(5);

		contactId = pDbHandle->columnInt(6);
	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();


	args = bundle_create();

	if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->msgType.subType == MSG_CB_SMS) {

		noti = notification_new(NOTIFICATION_TYPE_NOTI, 1, pMsg->msgId);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			bundle_free(args);
			return MSG_ERR_UNKNOWN;
		}

		noti_err = notification_set_application(noti, "org.tizen.message");
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, CB_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		memset(&tempId, 0x00, sizeof(tempId));

		snprintf(tempId, 5, "%d", threadId);
		bundle_add(args, "threadId", tempId);
	} else if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->msgType.classType == MSG_CLASS_0) {

		noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			bundle_free(args);
			return MSG_ERR_UNKNOWN;
		}

		noti_err = notification_set_application(noti, "org.tizen.msg-ui-class0");
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
		}

		snprintf(tempId, 5, "%d", pMsg->msgId);
		bundle_add(args, "msg_id", tempId);
	} else if (pMsg->msgType.mainType == MSG_SMS_TYPE && pMsg->msgType.subType == MSG_MWI_VOICE_SMS) {

		noti = notification_new(NOTIFICATION_TYPE_NOTI, 1, pMsg->msgId);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			bundle_free(args);
			return MSG_ERR_UNKNOWN;
		}

		noti_err = notification_set_application(noti, "org.tizen.message");
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_application : %d", noti_err);
		}

		noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, VOICE_MSG_ICON_PATH);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
		}

		memset(&tempId, 0x00, sizeof(tempId));

		snprintf(tempId, 5, "%d", threadId);
		bundle_add(args, "threadId", tempId);
	} else {
		MSG_DEBUG("notification_new pMsg->msgId [%d]", pMsg->msgId);
		noti = notification_new(NOTIFICATION_TYPE_NOTI, 1, pMsg->msgId);
		if (noti == NULL) {
			MSG_DEBUG("notification_new is failed.");
			bundle_free(args);
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

		memset(&tempId, 0x00, sizeof(tempId));

		snprintf(tempId, 5, "%d", threadId);
		bundle_add(args, "threadId", tempId);
	}

	bundle_add(args, "type", "msg_id");

 	snprintf(tempId, 5, "%d", pMsg->msgId);
	bundle_add(args, "msgId", tempId);

	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (pMsg->msgType.mainType == MSG_SMS_TYPE)
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, pMsg->msgText, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, pMsg->subject, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);


	if (args != NULL) {
		noti_err = notification_set_args(noti, args, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE) {
			MSG_DEBUG("Fail to notification_set_args : %d", noti_err);
		}
	}

	noti_err = notification_set_time(noti, msgTime);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_time : %d", noti_err);
	}

	noti_err = notification_insert(noti, NULL);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_insert");
	}

	noti_err = notification_free(noti);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_free");
	}

	bundle_free(args);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgInsertSmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId, MSG_DELIVERY_REPORT_STATUS_T Status)
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

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME \
								       FROM %s A, %s B \
								    WHERE B.MSG_ID = %d \
								         AND A.ADDRESS_ID = B.ADDRESS_ID;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, MsgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (pDbHandle->columnText(0) != NULL)
			strncpy(addressVal, (char*)pDbHandle->columnText(0), MAX_ADDRESS_VAL_LEN);

		if (pDbHandle->columnText(1) != NULL) {
			strncpy(displayName, (char*)pDbHandle->columnText(1), MAX_DISPLAY_NAME_LEN);
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
	} else {
		MSG_DEBUG("query : %s", sqlQuery);

		pDbHandle->finalizeQuery();

		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();


	noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		MSG_DEBUG("notification_new is failed.");
		return MSG_ERR_UNKNOWN;
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NOTI_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);


	if (Status == MSG_DELIVERY_REPORT_SUCCESS)
		snprintf(contents, MAX_DISPLAY_NAME_LEN, "Delivered.");
	else
		snprintf(contents, MAX_DISPLAY_NAME_LEN, "Deliver Failed.");

	notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

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


MSG_ERROR_T MsgInsertMmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg)
{

	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	MSG_DELIVERY_REPORT_STATUS_T	deliveryStatus;
	MSG_READ_REPORT_STATUS_T		readStatus;

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

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT A.ADDRESS_VAL, A.DISPLAY_NAME, A.FIRST_NAME, A.LAST_NAME, B.DELIVERY_REPORT_STATUS, B.READ_REPORT_STATUS \
								       FROM %s A, %s B \
								    	WHERE B.MSG_ID=%d AND A.ADDRESS_ID=B.ADDRESS_ID;",
				MSGFW_ADDRESS_TABLE_NAME, MSGFW_MESSAGE_TABLE_NAME, pMsg->msgId);

	if (pDbHandle->prepareQuery(sqlQuery) != MSG_SUCCESS)
		return MSG_ERR_DB_PREPARE;

	if (pDbHandle->stepQuery() == MSG_ERR_DB_ROW) {
		if (pDbHandle->columnText(0) != NULL)
			strncpy(addressVal, (char*)pDbHandle->columnText(0), MAX_ADDRESS_VAL_LEN);


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

		deliveryStatus = (MSG_DELIVERY_REPORT_STATUS_T)pDbHandle->columnInt(4);
		readStatus = (MSG_READ_REPORT_STATUS_T)pDbHandle->columnInt(5);

	} else {
		pDbHandle->finalizeQuery();
		return MSG_ERR_DB_STEP;
	}

	pDbHandle->finalizeQuery();

	noti = notification_new(NOTIFICATION_TYPE_NOTI, NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE);
	if (noti == NULL) {
		MSG_DEBUG("notification_new is failed.");
		return MSG_ERR_UNKNOWN;
	}

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, NOTI_MSG_ICON_PATH);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_set_image : %d", noti_err);
	}

	if (displayName[0] == '\0')
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, addressVal, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, displayName, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (pMsg->msgType.subType == MSG_DELIVERYIND_MMS) {

		switch(deliveryStatus) {
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
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT_FOR_DISPLAY_OPTION_IS_OFF, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

		noti_err = notification_insert(noti, NULL);
		if (noti_err != NOTIFICATION_ERROR_NONE)
			MSG_DEBUG("Fail to notification_insert");

	} else if (pMsg->msgType.subType == MSG_READORGIND_MMS) {

		switch(readStatus) {
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
		}

		notification_set_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, contents, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

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


MSG_ERROR_T MsgDeleteNotiByMsgId(MSG_MESSAGE_ID_T msgId)
{

	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	MSG_DEBUG("notification_delete_by_priv_id msgId [%d]", msgId);

	noti_err = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NOTI, msgId);

	if (noti_err != NOTIFICATION_ERROR_NONE) {
		MSG_DEBUG("Fail to notification_delete_by_priv_id noti_err [%d]", noti_err);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgDeleteNotiByThreadId(MSG_THREAD_ID_T ThreadId)
{
	notification_delete_group_by_group_id(NULL, NOTIFICATION_TYPE_NOTI, ThreadId);

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg)
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

	noti_err = notification_set_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, CB_MSG_ICON_PATH);
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
