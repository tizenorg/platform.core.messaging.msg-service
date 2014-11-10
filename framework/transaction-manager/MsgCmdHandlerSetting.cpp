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
#include "MsgCmdHandler.h"
#include "MsgSettingHandler.h"
#include "MsgUtilFunction.h"
#include "MsgCppTypes.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
int MsgSetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	int eventSize = 0;
	int eventType = -1;

	// Get Setting Structure
	MSG_SETTING_S* pSetting = (MSG_SETTING_S*)pCmd->cmdData;

	// Set Config Data
	err = MsgSetConfigData(pSetting);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgSetConfigData()");
	}
	else
	{
		MSG_DEBUG("Command Handle Fail : MsgSetConfigData()");
	}

	// Make Event Data
	switch (pCmd->cmdType) {
	case MSG_CMD_SET_SMSC_OPT :
		eventType = MSG_EVENT_SET_SMSC_OPT;
		break;
	case MSG_CMD_SET_CB_OPT :
		eventType = MSG_EVENT_SET_CB_OPT;
		break;
	case MSG_CMD_SET_SMS_SEND_OPT :
		eventType = MSG_EVENT_SET_SMS_SEND_OPT;
		break;
	case MSG_CMD_SET_MMS_SEND_OPT :
		eventType = MSG_EVENT_SET_MMS_SEND_OPT;
		break;
	case MSG_CMD_SET_MMS_RECV_OPT :
		eventType = MSG_EVENT_SET_MMS_RECV_OPT;
		break;
	case MSG_CMD_SET_PUSH_MSG_OPT :
		eventType = MSG_EVENT_SET_PUSH_MSG_OPT;
		break;
	case MSG_CMD_SET_VOICE_MSG_OPT :
		eventType = MSG_EVENT_SET_VOICE_MSG_OPT;
		break;
	case MSG_CMD_SET_GENERAL_MSG_OPT :
		eventType = MSG_EVENT_SET_GENERAL_MSG_OPT;
		break;
	case MSG_CMD_SET_MSG_SIZE_OPT :
			eventType = MSG_EVENT_SET_MSG_SIZE_OPT;
			break;
	default :
		break;
	}

	eventSize = MsgMakeEvent(NULL, 0, eventType, err, (void**)ppEvent);

	return eventSize;
}


int MsgGetConfigHandler(const MSG_CMD_S *pCmd, char **ppEvent)
{
	msg_error_t err = MSG_SUCCESS;

	char* encodedData = NULL;
	AutoPtr<char> buf(&encodedData);

	int dataSize = 0;
	int eventSize = 0;
	int eventType = -1;

	// Get Option Type
	MSG_OPTION_TYPE_T* type = (MSG_OPTION_TYPE_T*)pCmd->cmdData;

	// Get Config Data
	MSG_SETTING_S setting;
	setting.type = *type;

	err = MsgGetConfigData(&setting);

	if (err == MSG_SUCCESS)
	{
		MSG_DEBUG("Command Handle Success : MsgGetConfigData()");

		// Encoding Config Data
		switch (setting.type)
			{
				case MSG_GENERAL_OPT :
					dataSize += sizeof(MSG_GENERAL_OPT_S);
					break;
				case MSG_SMS_SENDOPT :
					dataSize += sizeof(MSG_SMS_SENDOPT_S);
					break;
				case MSG_SMSC_LIST :
					dataSize += sizeof(MSG_SMSC_LIST_S);
					break;
				case MSG_MMS_SENDOPT :
					dataSize += sizeof(MSG_MMS_SENDOPT_S);
					break;
				case MSG_MMS_RECVOPT :
					dataSize += sizeof(MSG_MMS_RECVOPT_S);
					break;
				case MSG_MMS_STYLEOPT :
					dataSize += sizeof(MSG_MMS_STYLEOPT_S);
					break;
				case MSG_PUSHMSG_OPT :
					dataSize += sizeof(MSG_PUSHMSG_OPT_S);
					break;
				case MSG_CBMSG_OPT :
					dataSize += sizeof(MSG_CBMSG_OPT_S);
					break;
				case MSG_VOICEMAIL_OPT :
					dataSize += sizeof(MSG_VOICEMAIL_OPT_S);
					break;
				case MSG_MSGSIZE_OPT :
					dataSize += sizeof(MSG_MSGSIZE_OPT_S);
					break;
				default:
					break;
			}

			encodedData = (char*)new char[dataSize];
			void* p = (void*)encodedData;

			switch (setting.type)
			{
			case MSG_GENERAL_OPT :
				memcpy(p, &(setting.option.generalOpt), dataSize);
				break;
			case MSG_SMS_SENDOPT :
				memcpy(p, &(setting.option.smsSendOpt), dataSize);
				break;
			case MSG_SMSC_LIST :
				memcpy(p, &(setting.option.smscList), dataSize);
				break;
			case MSG_MMS_SENDOPT :
				memcpy(p, &(setting.option.mmsSendOpt), dataSize);
				break;
			case MSG_MMS_RECVOPT :
				memcpy(p, &(setting.option.mmsRecvOpt), dataSize);
				break;
			case MSG_MMS_STYLEOPT :
				memcpy(p, &(setting.option.mmsStyleOpt), dataSize);
				break;
			case MSG_PUSHMSG_OPT :
				memcpy(p, &(setting.option.pushMsgOpt), dataSize);
				break;
			case MSG_CBMSG_OPT :
				memcpy(p, &(setting.option.cbMsgOpt), dataSize);
				break;
			case MSG_VOICEMAIL_OPT :
				memcpy(p, &(setting.option.voiceMailOpt), dataSize);
				break;
			case MSG_MSGSIZE_OPT :
				memcpy(p, &(setting.option.msgSizeOpt), dataSize);
				break;
			default:
				break;
			}
	} else {
		MSG_DEBUG("Command Handle Fail : MsgGetConfigData()");
	}

	MSG_DEBUG("dataSize [%d]", dataSize);

	switch (pCmd->cmdType) {
	case MSG_CMD_GET_SMSC_OPT :
		eventType = MSG_EVENT_GET_SMSC_OPT;
		break;
	case MSG_CMD_GET_CB_OPT :
		eventType = MSG_EVENT_GET_CB_OPT;
		break;
	case MSG_CMD_GET_SMS_SEND_OPT :
		eventType = MSG_EVENT_GET_SMS_SEND_OPT;
		break;
	case MSG_CMD_GET_MMS_SEND_OPT :
		eventType = MSG_EVENT_GET_MMS_SEND_OPT;
		break;
	case MSG_CMD_GET_MMS_RECV_OPT :
		eventType = MSG_EVENT_GET_MMS_RECV_OPT;
		break;
	case MSG_CMD_GET_PUSH_MSG_OPT :
		eventType = MSG_EVENT_GET_PUSH_MSG_OPT;
		break;
	case MSG_CMD_GET_VOICE_MSG_OPT :
		eventType = MSG_EVENT_GET_VOICE_MSG_OPT;
		break;
	case MSG_CMD_GET_GENERAL_MSG_OPT :
		eventType = MSG_EVENT_GET_GENERAL_MSG_OPT;
		break;
	case MSG_CMD_GET_MSG_SIZE_OPT :
			eventType = MSG_EVENT_GET_MSG_SIZE_OPT;
		break;
	default :
		break;
	}

	// Make Event Data
	eventSize = MsgMakeEvent(encodedData, dataSize, eventType, err, (void**)ppEvent);

	return eventSize;
}
