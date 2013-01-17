/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"
#include "MsgSpamFilter.h"
#include "MsgStorageHandler.h"
#include "MsgCmdHandler.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgAddFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Filter Structure
	MSG_FILTER_S* pFilter = (MSG_FILTER_S*)pCmd->cmdData;

	// Add Filter
	err = MsgStoAddFilter(pFilter);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoAddFilter()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoAddFilter()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_ADD_FILTER, err, (void**)ppEvent);

	return eventSize;
}


int MsgUpdateFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Filter Structure
	MSG_FILTER_S* pFilter = (MSG_FILTER_S*)pCmd->cmdData;

	// Update Filter
	err = MsgStoUpdateFilter(pFilter);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoUpdateFilter()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoUpdateFilter()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_UPDATE_FILTER, err, (void**)ppEvent);

	return eventSize;
}


int MsgDeleteFilterHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Filter Structure
	msg_filter_id_t  *pFilterId = (msg_filter_id_t *)pCmd->cmdData;

	MSG_DEBUG("Delete Filter id : %d", *pFilterId);

	// Delete Filter
	err = MsgStoDeleteFilter(*pFilterId);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoDeleteFilter()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoDeleteFilter()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_DELETE_FILTER, err, (void**)ppEvent);

	return eventSize;
}


int MsgSetFilterActivationHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Filter Structure
	msg_filter_id_t  *pFilterId = (msg_filter_id_t *)pCmd->cmdData;

	bool setFlag = false;

	memcpy(&setFlag, pCmd->cmdData+sizeof(msg_filter_id_t), sizeof(bool));

	MSG_DEBUG("Filter id : %d", *pFilterId);

	// Delete Filter
	err = MsgStoSetFilterActivation(*pFilterId, setFlag);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoSetFilterActivation()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoSetFilterActivation()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_SET_FILTER_ACTIVATION, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetFilterListHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Filter List
	msg_struct_list_s filterList;

	err = MsgStoGetFilterList(&filterList);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgStoGetFilterList()");

		// Encoding Filter List Data
		dataSize = MsgEncodeFilterList(&filterList, &encodedData);

		delete [] filterList.msg_struct_info;
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgStoGetFilterList()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_FILTERLIST, err, (void**)ppEvent);

	return eventSize;
}


int MsgSetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;

	// Get Filter Flag
	bool setFlag = false;

	memcpy(&setFlag, pCmd->cmdData, sizeof(bool));

	// Add Filter
	err = MsgSetFilterOperation(setFlag);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgSetFilterOperation()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgSetFilterOperation()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(NULL, 0, MSG_EVENT_SET_FILTER_OPERATION, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetFilterOperationHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0, eventSize = 0;

	// Get Filter List
	bool setFlag = false;

	err = MsgGetFilterOperation(&setFlag);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgGetFilterOperation()");

		// Encoding Filter List Data
		dataSize = MsgEncodeFilterFlag(&setFlag, &encodedData);
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgFilterGetFilterList()");
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, MSG_EVENT_GET_FILTER_OPERATION, err, (void**)ppEvent);

	return eventSize;
}
