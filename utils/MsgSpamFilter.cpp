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
#include "MsgUtilFile.h"
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

	MsgSettingGetBool(MSG_BLOCK_MESSAGE, pSetFlag);

	MSG_END();

	return MSG_SUCCESS;
}


void GetValidNumber(const char *pSrcNum, char *pDestNum)
{
	int overLen = 0, i = 0;

	overLen = strlen(pSrcNum) - VALID_ADDRESS_LEN;

	for (i = 0; i < VALID_ADDRESS_LEN; i++)
		pDestNum[i] = pSrcNum[i+overLen];

	pDestNum[i] = '\0';
}


bool MsgCheckFilter(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	// =======================================================================
	// Check Block List of Call-log (Auto Reject)
	// =======================================================================
	int ret = 0;

	MSG_DEBUG("bAutoReject = [%d], bUnknownAutoReject = [%d]", MsgSettingGetAutoReject(), MsgSettingGetUnknownAutoReject());

	if (MsgSettingGetAutoReject()) {
		char checkNumber[MAX_ADDRESS_VAL_LEN+1] = {0,};
		snprintf(checkNumber, MAX_ADDRESS_VAL_LEN, "%s", pMsgInfo->addressList[0].addressVal);

		if (checkNumber[0] == '\0') {
			if (MsgSettingGetUnknownAutoReject())
				return true;
		}

		if (ret > 0) {
			MSG_DEBUG("Message is Rejected by auto-reject option.");
			return true;
		}
	}

	// =======================================================================
	// Check Filter Operation
	// =======================================================================
	bool filterFlag = false;

	MsgGetFilterOperation(&filterFlag);

	if (filterFlag == false) {
		MSG_DEBUG("filter operation is not working");
		return false;
	}

	// =======================================================================
	// Check Filter by Address
	// =======================================================================
	int rowCnt = 0;

	MSG_DEBUG("pMsg->addressList[0].addressVal [%s]", pMsgInfo->addressList[0].addressVal);

	char newNumber[VALID_ADDRESS_LEN+1];
	char sqlQuery[MAX_QUERY_LEN+1];

	if (strlen(pMsgInfo->addressList[0].addressVal) > VALID_ADDRESS_LEN) {
		memset(newNumber, 0x00, sizeof(newNumber));
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		GetValidNumber(pMsgInfo->addressList[0].addressVal, newNumber);

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND FILTER_VALUE LIKE '%%%s';",
				MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS, newNumber);
	} else {
		memset(sqlQuery, 0x00, sizeof(sqlQuery));

		snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_ID FROM %s WHERE FILTER_TYPE = %d AND FILTER_VALUE = '%s';",
				MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_ADDRESS, pMsgInfo->addressList[0].addressVal);
	}

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (rowCnt > 0) {
		MSG_DEBUG("Msg is Filtered by Address : [%s]", pMsgInfo->addressList[0].addressVal);

		pDbHandle->freeTable();

		pMsgInfo->folderId = MSG_SPAMBOX_ID;

		return true;
	} else {
		MSG_DEBUG("Msg is NOT Filtered by Address : [%s]", pMsgInfo->addressList[0].addressVal);

		pDbHandle->freeTable();
	}

	// =======================================================================
	// Check Filter by Subject
	// =======================================================================
	// Get Filter List
	memset(sqlQuery, 0x00, sizeof(sqlQuery));

	snprintf(sqlQuery, sizeof(sqlQuery), "SELECT FILTER_VALUE FROM %s WHERE FILTER_TYPE = %d;",
			MSGFW_FILTER_TABLE_NAME, MSG_FILTER_BY_SUBJECT);

	rowCnt = 0;

	err = pDbHandle->getTable(sqlQuery, &rowCnt);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("MsgGetTable() Error [%d] : [%s]", err, sqlQuery);

		pDbHandle->freeTable();

		return false;
	}

	char filterValue[MAX_FILTER_VALUE_LEN+1];

	char* pData = NULL;
	AutoPtr<char> buf(&pData);

	int fileSize = 0;
	bool bFiltered = false;

	for (int i = 1; i <= rowCnt; i++)
	{
		memset(filterValue, 0x00, sizeof(filterValue));

		pDbHandle->getColumnToString(i, MAX_FILTER_VALUE_LEN, filterValue);

		MSG_DEBUG("filterValue [%s]", filterValue);

		if (strlen(filterValue) <= 0) continue;

		if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE && pMsgInfo->msgType.subType == MSG_NORMAL_SMS) {
			if (pMsgInfo->bTextSms == false) {
				if (MsgOpenAndReadFile(pMsgInfo->msgData, &pData, &fileSize) == false) {
					pDbHandle->freeTable();
					return false;
				}
				MSG_DEBUG("file data [%s]", pData);
			} else {
				if (pMsgInfo->dataSize > 0) {
					pData = new char[pMsgInfo->dataSize+1];

					strncpy(pData, pMsgInfo->msgText, pMsgInfo->dataSize);
					pData[strlen(pMsgInfo->msgText)] = '\0';
				}
			}
		} else if(pMsgInfo->msgType.mainType == MSG_MMS_TYPE) {
			if (strlen(pMsgInfo->subject) > 0) {
				pData = new char[strlen(pMsgInfo->subject)+1];

				strncpy(pData, pMsgInfo->subject, strlen(pMsgInfo->subject));
				pData[strlen(pMsgInfo->subject)] = '\0';
			}
		}

		// NULL value check
		if (pData == NULL) {
			MSG_DEBUG("pData is NULL");

			bFiltered = false;
			break;
		}

		MSG_DEBUG("pData [%s]", pData);

		if (strcasestr(pData, filterValue) != NULL) {
			MSG_DEBUG("Msg is Filtered by Subject [%s] Data [%s]", filterValue, pData);

			bFiltered = true;
			break;
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
