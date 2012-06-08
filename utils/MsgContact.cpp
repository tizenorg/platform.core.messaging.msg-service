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

extern "C"
{
	#include <contacts-svc.h>
}

#include "MsgDebug.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgContact.h"


/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
static bool isContactSvcOpened = false;

MsgDbHandler ContactDbHandle;

MsgContactChangeCB cbFunction = NULL;
/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
static void MsgContactSvcCallback(void *pData)
{
	MSG_DEBUG("Contact Data is Changed!!!");

	MsgSyncContact();

	if (ContactDbHandle.disconnect() != MSG_SUCCESS)
	{
		MSG_DEBUG("DB Disconnect Fail");
	}
}


MSG_ERROR_T MsgOpenContactSvc()
{
	int errCode = CTS_SUCCESS;

	if(!isContactSvcOpened)
	{
		errCode = contacts_svc_connect();

		if (errCode == CTS_SUCCESS)
		{
			MSG_DEBUG("Connect to Contact Service Success");
			isContactSvcOpened = true;
		}
		else
		{
			MSG_DEBUG("Connect to Contact Service Fail [%d]", errCode);
			isContactSvcOpened = false;
			return MSG_ERR_DB_CONNECT;
		}
	}
	else
	{
		MSG_DEBUG("Already connected to Contact Service.");
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgCloseContactSvc()
{
	int errCode = CTS_SUCCESS;

	if(isContactSvcOpened)
	{
		errCode = contacts_svc_disconnect();

		if (errCode == CTS_SUCCESS)
		{
			MSG_DEBUG("Disconnect to Contact Service Success");
		}
		else
		{
			MSG_DEBUG("Disconnect to Contact Service Fail [%d]", errCode);
			return MSG_ERR_DB_DISCONNECT;
		}
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgInitContactSvc(MsgContactChangeCB cb)
{
	int errCode = CTS_SUCCESS;

	if (cb != NULL)
		cbFunction = cb;

	// Register callback function
	errCode = contacts_svc_subscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, MsgContactSvcCallback, NULL);

	if (errCode == CTS_SUCCESS)
	{
		MSG_DEBUG("Register Contact Service Callback");
	}
	else
	{
		MSG_DEBUG("Fail to Register Contact Service Callback [%d]", errCode);
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T MsgGetContactInfo(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_CONTACT_INFO_S *pContactInfo)
{
	MSG_BEGIN();

	MSG_DEBUG("Address Type [%d], Address Value [%s]", pAddrInfo->addressType, pAddrInfo->addressVal);

	memset(pContactInfo, 0x00, sizeof(MSG_CONTACT_INFO_S));

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_PLMN && strlen(pAddrInfo->addressVal) > (MAX_PHONE_NUMBER_LEN+1))
	{
		MSG_DEBUG("Phone Number is too long [%s]", pAddrInfo->addressVal);
		return MSG_ERR_INVALID_PARAMETER;
	}

	int index, ret = -1;

	CTSstruct* contact = NULL;

	cts_find_op recordType;

	if (pAddrInfo->addressType == MSG_ADDRESS_TYPE_EMAIL)
		recordType = CTS_FIND_BY_EMAIL;
	else
		recordType = CTS_FIND_BY_NUMBER;

	index = contacts_svc_find_contact_by(recordType, (char*)pAddrInfo->addressVal);

	if (index > CTS_SUCCESS)
	{
		MSG_DEBUG("Index : [%d]", index);
		ret = contacts_svc_get_contact(index, &contact);
	}

	if (ret < 0)
	{
		MSG_DEBUG("No Contact Info");
		return MSG_SUCCESS;
	}

	CTSvalue* name = NULL;

	ret = contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &name);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_struct_get_value() Error [%d]", ret);
		return MSG_SUCCESS;
	}

	const char* strDisplayName = contacts_svc_value_get_str(name, CTS_NAME_VAL_DISPLAY_STR);
	const char* strFirstName = contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR);
	const char* strLastName = contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR);

	MSG_DEBUG("Display Name : [%s], First Name : [%s], Last Name : [%s]", strDisplayName, strFirstName, strLastName);

	if (strDisplayName != NULL)
	{
		strncpy(pContactInfo->displayName, strDisplayName, MAX_DISPLAY_NAME_LEN);
	}

	if (strFirstName != NULL)
	{
		strncpy(pContactInfo->firstName, strFirstName, MAX_DISPLAY_NAME_LEN);
	}

	if (strLastName != NULL)
	{
		strncpy(pContactInfo->lastName, strLastName, MAX_DISPLAY_NAME_LEN);
	}

	CTSvalue* base = NULL;

	ret = contacts_svc_struct_get_value(contact, CTS_CF_BASE_INFO_VALUE, &base);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_struct_get_value() Error [%d]", ret);
		return MSG_SUCCESS;
	}

	pContactInfo->contactId = contacts_svc_value_get_int(base, CTS_BASE_VAL_ID_INT);

	MSG_DEBUG("Contact ID [%d]", pContactInfo->contactId);

	const char* strImagePath = contacts_svc_value_get_str(base, CTS_BASE_VAL_IMG_PATH_STR);

	if (strImagePath != NULL)
	{
		strncpy(pContactInfo->imagePath , strImagePath, MAX_IMAGE_PATH_LEN);
	}

	MSG_DEBUG("Image Path [%s]", pContactInfo->imagePath);

	contacts_svc_value_free(base);
	contacts_svc_value_free(name);

	contacts_svc_struct_free(contact);

	MSG_END();

	return MSG_SUCCESS;
}


void MsgSyncContact()
{
	int ret = -1;
	int index_num = 0;
	int changed_count = 0;
	int lastSyncTime = 0;

	/* get contact sync time */
	lastSyncTime = MsgSettingGetInt(CONTACT_SYNC_TIME);

	if (lastSyncTime < 0) {
		MSG_DEBUG("Fail to get CONTACT_SYNC_TIME.");
		lastSyncTime = 0;
	}

	CTSiter* pIter;

	ret = contacts_svc_get_updated_contacts(0, lastSyncTime, &pIter);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_get_updated_contacts() Error [%d]", ret);
		return;
	}

	while (contacts_svc_iter_next(pIter) == CTS_SUCCESS)
	{
		CTSvalue *row_info = NULL;

		row_info = contacts_svc_iter_get_info(pIter);

		index_num = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);

		MSG_DEBUG("index (%d)", index_num);

		int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);

		if (type == CTS_OPERATION_UPDATED || type == CTS_OPERATION_INSERTED)
		{
			MsgUpdateContact(index_num, type);
		}
		else // Delete
		{
			MSG_DEBUG("Delete Contact");

			MsgDeleteContact(index_num);
		}

		if(lastSyncTime < contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT))
			lastSyncTime = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT);

		contacts_svc_value_free(row_info);

		changed_count++;

	}

	MsgSettingSetInt(CONTACT_SYNC_TIME, lastSyncTime);
	MSG_DEBUG("lastSyncTime : %d", lastSyncTime);

	contacts_svc_iter_remove(pIter);

	if(changed_count > 0)
		cbFunction();
}


bool MsgInsertContact(MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber)
{
	if (!pNumber || strlen(pNumber) <= 0)
		return false;

	if (MsgStoAddContactInfo(&ContactDbHandle, pContactInfo, pNumber) != MSG_SUCCESS)
	{
		MSG_DEBUG("Fail to add contact info.");
		return false;
	}

	return true;
}


bool MsgUpdateContact(int index, int type)
{
	int ret = -1;

	CTSstruct *contact = NULL;

	ret = contacts_svc_get_contact(index, &contact);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_get_contact() Error [%d]", ret);
		return false;
	}

	// Base Info
	CTSvalue *base = NULL;

	ret = contacts_svc_struct_get_value(contact, CTS_CF_BASE_INFO_VALUE, &base);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_struct_get_value() Error [%d]", ret);
		return false;
	}

	MSG_CONTACT_INFO_S contactInfo = {0};

	contactInfo.contactId = contacts_svc_value_get_int(base, CTS_BASE_VAL_ID_INT);

	MSG_DEBUG("Contact ID [%d]", contactInfo.contactId);

	const char* strImagePath = contacts_svc_value_get_str(base, CTS_BASE_VAL_IMG_PATH_STR);

	if (strImagePath != NULL)
	{
		strncpy(contactInfo.imagePath , strImagePath, MAX_IMAGE_PATH_LEN);
	}

	MSG_DEBUG("Image Path [%s]", contactInfo.imagePath);

	// Name Info
	CTSvalue* name = NULL;

	ret = contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &name);

	if (ret != CTS_SUCCESS)
	{
		MSG_DEBUG("contacts_svc_struct_get_value() Error [%d]", ret);
		return MSG_SUCCESS;
	}

	const char* strDisplayName = contacts_svc_value_get_str(name, CTS_NAME_VAL_DISPLAY_STR);
	const char* strFirstName = contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR);
	const char* strLastName = contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR);

	MSG_DEBUG("Display Name : [%s], First Name : [%s], Last Name : [%s]", strDisplayName, strFirstName, strLastName);

	if (strDisplayName != NULL)
	{
		strncpy(contactInfo.displayName, strDisplayName, MAX_DISPLAY_NAME_LEN);
	}

	if (strFirstName != NULL)
	{
		strncpy(contactInfo.firstName, strFirstName, MAX_DISPLAY_NAME_LEN);
	}

	if (strLastName != NULL)
	{
		strncpy(contactInfo.lastName, strLastName, MAX_DISPLAY_NAME_LEN);
	}

	MsgStoClearContactInfo(&ContactDbHandle, index);

	GSList *get_list, *cursor;

	get_list = NULL;

	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

	cursor = get_list;

	// No phone number in contact
	if (cursor == NULL)
	{
		 contacts_svc_struct_free(contact);

		return true;
	}

	for(; cursor; cursor = g_slist_next(cursor))
	{
		MSG_DEBUG("Add Contact Data");

		const char* strNumber = contacts_svc_value_get_str((CTSvalue*)cursor->data, CTS_NUM_VAL_NUMBER_STR);

		MSG_DEBUG("Number = %s", strNumber);

		if (MsgInsertContact(&contactInfo, strNumber) == false)
		{
			continue;
		}
 	}

	contacts_svc_struct_free(contact);

	return true;
}


bool MsgDeleteContact(int index)
{
	if (MsgStoClearContactInfo(&ContactDbHandle, index) != MSG_SUCCESS)
	{
		return false;
	}

	return true;
}


int MsgGetContactNameOrder()
{
	int ret = 0;

	cts_order_type order = CTS_ORDER_NAME_FIRSTLAST;

	order = contacts_svc_get_order(CTS_ORDER_OF_DISPLAY);

	if (order == CTS_ORDER_NAME_FIRSTLAST)
		ret = 0;
	else if (order == CTS_ORDER_NAME_LASTFIRST)
		ret = 1;

	return ret;
}


int MsgContactSVCBeginTrans()
{
	return contacts_svc_begin_trans();
}


int MsgContactSVCEndTrans(bool bSuccess)
{
	return contacts_svc_end_trans(bSuccess);
}
