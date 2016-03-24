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

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgSpamFilter.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
msg_error_t MsgSetFilterOperation(bool bSetFlag)
{
	MSG_BEGIN();

	if (MsgSettingSetBool(MSG_BLOCK_MESSAGE, bSetFlag) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", MSG_BLOCK_MESSAGE);
		return MSG_ERR_SET_SETTING;
	}

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgGetFilterOperation(bool *pSetFlag)
{
	MSG_BEGIN();

	if (MsgSettingGetBool(MSG_BLOCK_MESSAGE, pSetFlag) != MSG_SUCCESS)
		MSG_INFO("MsgSettingGetBool() is failed");

	MSG_END();

	return MSG_SUCCESS;
}


bool MsgCheckFilter(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	/*========================================================================
	   Check Unknown Sender
	========================================================================*/
	bool bBlockUnknown = false;

	if (MsgSettingGetBool(MSG_BLOCK_UNKNOWN_MSG, &bBlockUnknown) == MSG_SUCCESS) {
		if (bBlockUnknown == true && pMsgInfo->addressList[0].addressVal[0] == '\0') {
			MSG_DEBUG("Unknown sender message!!");
			return true;
		}
	}

	/*========================================================================
	   Check Filter Operation
	========================================================================*/
	bool filterFlag = false;

	MsgGetFilterOperation(&filterFlag);

	if (filterFlag == false) {
		MSG_DEBUG("filter operation is not working");
		return false;
	}

	/*========================================================================
	   Check Filter by Address
	========================================================================*/
	int rowCnt = 0;

	MSG_SEC_DEBUG("pMsg->addressList[0].addressVal [%s]", pMsgInfo->addressList[0].addressVal);

	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND '%s' LIKE (CASE WHEN LENGTH(FILTER_VALUE) > %d-1 THEN '%%' || SUBSTR(FILTER_VALUE, LENGTH(FILTER_VALUE)-%d+1) ELSE FILTER_VALUE END) AND FILTER_ACTIVE = 1 \
			UNION SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND '%s' LIKE SUBSTR(FILTER_VALUE, 1) || '%%' AND FILTER_ACTIVE = 1 \
			UNION SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND '%s' LIKE '%%' || SUBSTR(FILTER_VALUE, 1) || '%%' AND FILTER_ACTIVE = 1 \
			UNION SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND '%s' LIKE '%%' || SUBSTR(FILTER_VALUE, 1) AND FILTER_ACTIVE = 1;",
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS_SAME, pMsgInfo->addressList[0].addressVal, MsgContactGetMinMatchDigit(), MsgContactGetMinMatchDigit(),
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS_START, pMsgInfo->addressList[0].addressVal,
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS_INCLUDE, pMsgInfo->addressList[0].addressVal,
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS_END, pMsgInfo->addressList[0].addressVal);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (rowCnt > 0) {
		MSG_SEC_DEBUG("Msg is Filtered by Address : [%s]", pMsgInfo->addressList[0].addressVal);
		pDbHandle->freeTable();
		pMsgInfo->folderId = MSG_SPAMBOX_ID;
		return true;
	} else {
		MSG_SEC_DEBUG("Msg is NOT Filtered by Address : [%s]", pMsgInfo->addressList[0].addressVal);
		pDbHandle->freeTable();
	}

	/*========================================================================
	   Check Filter by Subject
	========================================================================*/
	/* Get Filter List */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_VALUE FROM %s WHERE FILTER_TYPE = %d;",
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_WORD);

	rowCnt = 0;

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return false;
	}

	char filterValue[MAX_FILTER_VALUE_LEN+1];

	char* pData = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&pData, unique_ptr_deleter);

	int fileSize = 0;
	bool bFiltered = false;

	for (int i = 1; i <= rowCnt; i++) {
		memset(filterValue, 0x00, sizeof(filterValue));

		pDbHandle->getColumnToString(i, MAX_FILTER_VALUE_LEN, filterValue);

		MSG_DEBUG("filterValue [%s]", filterValue);

		if (strlen(filterValue) <= 0)
			continue;

		if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE && pMsgInfo->msgType.subType == MSG_NORMAL_SMS) {
			if (pMsgInfo->bTextSms == false) {
				if (pData) {
					delete[] pData;
					pData = NULL;
				}

				if (MsgOpenAndReadFile(pMsgInfo->msgData, &pData, &fileSize) == false) {
					pDbHandle->freeTable();
					return false;
				}
				MSG_DEBUG("file data [%s]", pData);
			} else {
				if (pMsgInfo->dataSize > 0) {
					if (pData) {
						delete[] pData;
						pData = NULL;
					}

					pData = new char[pMsgInfo->dataSize+1];

					strncpy(pData, pMsgInfo->msgText, pMsgInfo->dataSize);
					pData[strlen(pMsgInfo->msgText)] = '\0';
				}
			}
		} else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE) {
			if (strlen(pMsgInfo->subject) > 0) {
				if (pData) {
					delete[] pData;
					pData = NULL;
				}

				pData = new char[strlen(pMsgInfo->subject)+1];

				strncpy(pData, pMsgInfo->subject, strlen(pMsgInfo->subject));
				pData[strlen(pMsgInfo->subject)] = '\0';
			}
		}

		/* NULL value check */
		if (pData == NULL) {
			MSG_DEBUG("pData is NULL");

			bFiltered = false;
			break;
		}

		MSG_DEBUG("pData [%s]", pData);

		if (strcasestr(pData, filterValue) != NULL) {
#if 0
			MSG_CONTACT_INFO_S contactInfo;
			memset(&contactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

			/* Get Contact Info */
			if (MsgGetContactInfo(&(pMsgInfo->addressList[0]), &contactInfo) == MSG_SUCCESS) {
				if (contactInfo.contactId > 0) {
					MSG_SEC_DEBUG("Msg is Filtered by Subject [%s] Data [%s], but address is in contact. Skip.", filterValue, pData);
				} else {
					MSG_DEBUG("Msg is Filtered by Subject [%s] Data [%s]", filterValue, pData);
					bFiltered = true;
					break;
				}
			} else {
				MSG_DEBUG("MsgGetContactInfo() fail.");
			}
#else
			MSG_DEBUG("Msg is Filtered by Subject [%s] Data [%s]", filterValue, pData);
			bFiltered = true;
			break;
#endif
		}
	}

	pDbHandle->freeTable();

	if (bFiltered == true) {
		MSG_DEBUG("Msg is Filtered by Subject");

		pMsgInfo->folderId = MSG_SPAMBOX_ID;

		return true;
	} else {
		MSG_DEBUG("Msg is NOT Filtered by Subject");
	}

	MSG_END();

	return false;
}


bool MsgCheckFilterByWord(MsgDbHandler *pDbHandle, const char *pMsgText)
{
	MSG_BEGIN();

	if (!pMsgText) {
		MSG_DEBUG("pMsgText is NULL");
		return false;
	}

	msg_error_t err = MSG_SUCCESS;

	/*========================================================================
	   Check Filter Operation
	========================================================================*/
	bool filterFlag = false;

	MsgGetFilterOperation(&filterFlag);

	if (filterFlag == false) {
		MSG_DEBUG("filter operation is not working");
		return false;
	}

	int rowCnt = 0;
	char sqlQuery[MAX_QUERY_LEN+1];

	memset(sqlQuery, 0x00, sizeof(sqlQuery));


	/*========================================================================
	   Check Filter by Word
	========================================================================*/
	/* Get Filter List */
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_VALUE FROM %s WHERE FILTER_TYPE = %d;",
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_WORD);

	err = pDbHandle->getTable(sqlQuery, &rowCnt, NULL);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to getTable().");
		pDbHandle->freeTable();
		return false;
	}

	char filterValue[MAX_FILTER_VALUE_LEN+1];
	bool bFiltered = false;

	for (int i = 1; i <= rowCnt; i++) {
		memset(filterValue, 0x00, sizeof(filterValue));

		pDbHandle->getColumnToString(i, MAX_FILTER_VALUE_LEN, filterValue);

		MSG_DEBUG("filterValue [%s]", filterValue);

		if (strlen(filterValue) <= 0)
			continue;

		if (strcasestr(pMsgText, filterValue) != NULL) {
			MSG_DEBUG("Msg is Filtered by Word [%s] ", filterValue);
			bFiltered = true;
			break;
		}
	}

	pDbHandle->freeTable();

	MSG_END();

	return bFiltered;
}


