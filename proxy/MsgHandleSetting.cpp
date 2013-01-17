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
#include "MsgException.h"
#include "MsgHandle.h"


#define MSG_TYPE_CHECK(a, b) \
		({\
			if(a != b) return MSG_ERR_INVALID_PARAMETER; \
		})\

#define MSG_NULL_CHECK(a) \
		({\
			if(a == NULL) return MSG_ERR_NULL_POINTER; \
		})\

/*==================================================================================================
                                     IMPLEMENTATION OF MsgHandle - Setting Member Functions
==================================================================================================*/

msg_error_t MsgHandle::getSMSCOption(msg_struct_t msg_struct)
{
	msg_struct_s *smsc_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(smsc_opt->type, MSG_STRUCT_SETTING_SMSC_OPT);
	MSG_NULL_CHECK(smsc_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_SMSC_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_SMSC_LIST;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_SMSC_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		MSG_SMSC_LIST_S smsc_list_tmp = {0,};
		int dataSize = 0;

		dataSize = sizeof(MSG_SMSC_LIST_S);
		memcpy(&smsc_list_tmp, pEvent->data, dataSize);

		MSG_SMSC_LIST_HIDDEN_S *pTmp = (MSG_SMSC_LIST_HIDDEN_S *)smsc_opt->data;

		pTmp->selected = smsc_list_tmp.selected;
		pTmp->smsc_list->nCount = smsc_list_tmp.totalCnt;

		msg_struct_s *pStructTmp = NULL;

		for (int i = 0; i < pTmp->smsc_list->nCount; i++) {
			pStructTmp = (msg_struct_s *)pTmp->smsc_list->msg_struct_info[i];
			memset(pStructTmp->data, 0x00, sizeof(MSG_SMSC_DATA_S));
			memcpy(pStructTmp->data, &(smsc_list_tmp.smscData[i]), sizeof(MSG_SMSC_DATA_S));
		}
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setSMSCOption(msg_struct_t msg_struct)
{
	msg_struct_s *smsc_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(smsc_opt->type, MSG_STRUCT_SETTING_SMSC_OPT);
	MSG_NULL_CHECK(smsc_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_SMSC_LIST;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;
	int i = 0;

	msg_struct_s *pTmp = (msg_struct_s *)msg_struct;
	MSG_SMSC_LIST_HIDDEN_S *pTmpList = (MSG_SMSC_LIST_HIDDEN_S *)pTmp->data;

	MSG_SMSC_LIST_S smsc_list_tmp = {0,};

	smsc_list_tmp.totalCnt = pTmpList->smsc_list->nCount;
	smsc_list_tmp.selected = pTmpList->selected;

	msg_struct_s *pStructTmp = NULL;

	for (i=0; i < smsc_list_tmp.totalCnt; i++) {
		pStructTmp = (msg_struct_s *)pTmpList->smsc_list->msg_struct_info[i];
		MSG_TYPE_CHECK(pStructTmp->type, MSG_STRUCT_SETTING_SMSC_INFO);
		memcpy(&(smsc_list_tmp.smscData[i]), pStructTmp->data, sizeof(MSG_SMSC_DATA_S));
	}

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_SMSC_OPT;

    // Copy Cookie
    memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	MSG_SETTING_S pSetting = {0,};

	pSetting.type = optionType;
	memcpy(&(pSetting.option.smscList), &smsc_list_tmp, sizeof(MSG_SMSC_LIST_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_SMSC_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getCBOption(msg_struct_t msg_struct)
{
	msg_struct_s *cb_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(cb_opt->type, MSG_STRUCT_SETTING_CB_OPT);
	MSG_NULL_CHECK(cb_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_CB_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_CBMSG_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_CB_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		int i = 0;
		MSG_CBMSG_OPT_S cb_opt_tmp = {0,};
		memcpy(&cb_opt_tmp, pEvent->data, sizeof(MSG_CBMSG_OPT_S));

		MSG_CBMSG_OPT_HIDDEN_S *pTmp = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt->data;
		pTmp->bReceive = cb_opt_tmp.bReceive;
		pTmp->maxSimCnt = cb_opt_tmp.maxSimCnt;

		for (i = 0; i < CB_LANG_TYPE_MAX; i++)
			pTmp->bLanguage[i] = cb_opt_tmp.bLanguage[i];

		pTmp->channelData->nCount = cb_opt_tmp.channelData.channelCnt;

		msg_struct_s *pStructTmp = NULL;

		for (i = 0; i < pTmp->channelData->nCount; i++) {
			pStructTmp = (msg_struct_s *)pTmp->channelData->msg_struct_info[i];
			memset(pStructTmp->data, 0x00, sizeof(MSG_CB_CHANNEL_INFO_S));
			memcpy(pStructTmp->data, &(cb_opt_tmp.channelData.channelInfo[i]), sizeof(MSG_CB_CHANNEL_INFO_S));
		}
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setCBOption(msg_struct_t msg_struct)
{
	msg_struct_s *cb_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(cb_opt->type, MSG_STRUCT_SETTING_CB_OPT);
	MSG_NULL_CHECK(cb_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_CBMSG_OPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;
	int i = 0;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_CB_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_CBMSG_OPT_S cb_opt_tmp = {0,};
	MSG_CBMSG_OPT_HIDDEN_S *cb_msg_opt = (MSG_CBMSG_OPT_HIDDEN_S *)cb_opt->data;

	cb_opt_tmp.bReceive = cb_msg_opt->bReceive;
	cb_opt_tmp.maxSimCnt = cb_msg_opt->maxSimCnt;
	for (int i = 0; i < CB_LANG_TYPE_MAX; i++)
		cb_opt_tmp.bLanguage[i] = cb_msg_opt->bLanguage[i];

	msg_struct_list_s *cb_ch_list = cb_msg_opt->channelData;
	msg_struct_s *pStructTmp = NULL;

	cb_opt_tmp.channelData.channelCnt = cb_ch_list->nCount;

	for (i = 0; i < cb_opt_tmp.channelData.channelCnt; i++) {
		pStructTmp = (msg_struct_s *)cb_ch_list->msg_struct_info[i];
		memcpy(&(cb_opt_tmp.channelData.channelInfo[i]), pStructTmp->data, sizeof(MSG_CB_CHANNEL_INFO_S));
	}

	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.cbMsgOpt), &cb_opt_tmp, sizeof(MSG_CBMSG_OPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_CB_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getSmsSendOpt(msg_struct_t msg_struct)
{
	msg_struct_s *sms_send_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(sms_send_opt->type, MSG_STRUCT_SETTING_SMS_SEND_OPT);
	MSG_NULL_CHECK(sms_send_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_SMS_SEND_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_SMS_SENDOPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_SMS_SEND_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(sms_send_opt->data, pEvent->data, sizeof(MSG_SMS_SENDOPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setSmsSendOpt(msg_struct_t msg_struct)
{
	msg_struct_s *sms_send_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(sms_send_opt->type, MSG_STRUCT_SETTING_SMS_SEND_OPT);
	MSG_NULL_CHECK(sms_send_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_SMS_SENDOPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_SMS_SEND_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.smsSendOpt), sms_send_opt->data, sizeof(MSG_SMS_SENDOPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_SMS_SEND_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getMmsSendOpt(msg_struct_t msg_struct)
{
	msg_struct_s *mms_send_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(mms_send_opt->type, MSG_STRUCT_SETTING_MMS_SEND_OPT);
	MSG_NULL_CHECK(mms_send_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MMS_SEND_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_MMS_SENDOPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_MMS_SEND_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(mms_send_opt->data, pEvent->data, sizeof(MSG_MMS_SENDOPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setMmsSendOpt(msg_struct_t msg_struct)
{
	msg_struct_s *mms_send_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(mms_send_opt->type, MSG_STRUCT_SETTING_MMS_SEND_OPT);
	MSG_NULL_CHECK(mms_send_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_MMS_SENDOPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_MMS_SEND_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.mmsSendOpt), mms_send_opt->data, sizeof(MSG_MMS_SENDOPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_MMS_SEND_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getMmsRecvOpt(msg_struct_t msg_struct)
{
	msg_struct_s *mms_recv_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(mms_recv_opt->type, MSG_STRUCT_SETTING_MMS_RECV_OPT);
	MSG_NULL_CHECK(mms_recv_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MMS_RECV_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_MMS_RECVOPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_MMS_RECV_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(mms_recv_opt->data, pEvent->data, sizeof(MSG_MMS_RECVOPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setMmsRecvOpt(msg_struct_t msg_struct)
{
	msg_struct_s *mms_recv_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(mms_recv_opt->type, MSG_STRUCT_SETTING_MMS_RECV_OPT);
	MSG_NULL_CHECK(mms_recv_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_MMS_RECVOPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_MMS_RECV_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.mmsRecvOpt), mms_recv_opt->data, sizeof(MSG_MMS_RECVOPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_MMS_RECV_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getPushMsgOpt(msg_struct_t msg_struct)
{
	msg_struct_s *push_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(push_opt->type, MSG_STRUCT_SETTING_PUSH_MSG_OPT);
	MSG_NULL_CHECK(push_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_PUSH_MSG_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_PUSHMSG_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_PUSH_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(push_opt->data, pEvent->data, sizeof(MSG_PUSHMSG_OPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setPushMsgOpt(msg_struct_t msg_struct)
{
	msg_struct_s *push_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(push_opt->type, MSG_STRUCT_SETTING_PUSH_MSG_OPT);
	MSG_NULL_CHECK(push_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_PUSHMSG_OPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_PUSH_MSG_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.pushMsgOpt), push_opt->data, sizeof(MSG_PUSHMSG_OPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_PUSH_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getVoiceMsgOpt(msg_struct_t msg_struct)
{
	msg_struct_s *voice_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(voice_opt->type, MSG_STRUCT_SETTING_VOICE_MSG_OPT);
	MSG_NULL_CHECK(voice_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_VOICE_MSG_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_VOICEMAIL_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_VOICE_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(voice_opt->data, pEvent->data, sizeof(MSG_VOICEMAIL_OPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setVoiceMsgOpt(msg_struct_t msg_struct)
{
	msg_struct_s *voice_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(voice_opt->type, MSG_STRUCT_SETTING_VOICE_MSG_OPT);
	MSG_NULL_CHECK(voice_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_VOICEMAIL_OPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_VOICE_MSG_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.voiceMailOpt), voice_opt->data, sizeof(MSG_VOICEMAIL_OPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_VOICE_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getGeneralOpt(msg_struct_t msg_struct)
{
	msg_struct_s *general_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(general_opt->type, MSG_STRUCT_SETTING_GENERAL_OPT);
	MSG_NULL_CHECK(general_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_GENERAL_MSG_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_GENERAL_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_GENERAL_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(general_opt->data, pEvent->data, sizeof(MSG_GENERAL_OPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setGeneralOpt(msg_struct_t msg_struct)
{
	msg_struct_s *general_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(general_opt->type, MSG_STRUCT_SETTING_GENERAL_OPT);
	MSG_NULL_CHECK(general_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_GENERAL_OPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_GENERAL_MSG_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.generalOpt), general_opt->data, sizeof(MSG_GENERAL_OPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_GENERAL_MSG_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}

msg_error_t MsgHandle::getMsgSizeOpt(msg_struct_t msg_struct)
{
	msg_struct_s *msg_size_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(msg_size_opt->type, MSG_STRUCT_SETTING_MSGSIZE_OPT);
	MSG_NULL_CHECK(msg_size_opt->data);

	// Allocate Memory to Command Data
	int cmdSize = sizeof(MSG_CMD_S) + sizeof(MSG_OPTION_TYPE_T);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_GET_MSG_SIZE_OPT;

	// Set option type
	MSG_OPTION_TYPE_T opt_type = MSG_MSGSIZE_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	memcpy((void*)((char*)pCmd+sizeof(MSG_CMD_TYPE_T)+MAX_COOKIE_LEN), &opt_type, sizeof(MSG_OPTION_TYPE_T));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_GET_MSG_SIZE_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	if (pEvent->result == MSG_SUCCESS) {
		memcpy(msg_size_opt->data, pEvent->data, sizeof(MSG_MSGSIZE_OPT_S));
	}

	return pEvent->result;
}

msg_error_t MsgHandle::setMsgSizeOpt(msg_struct_t msg_struct)
{
	msg_struct_s *msg_size_opt = (msg_struct_s *)msg_struct;
	MSG_TYPE_CHECK(msg_size_opt->type, MSG_STRUCT_SETTING_MSGSIZE_OPT);
	MSG_NULL_CHECK(msg_size_opt->data);

	// Allocate Memory to Command Data
	MSG_OPTION_TYPE_T optionType = MSG_MSGSIZE_OPT;
	int cmdSize = getSettingCmdSize(optionType);

	char cmdBuf[cmdSize];
	bzero(cmdBuf, cmdSize);
	MSG_CMD_S* pCmd = (MSG_CMD_S*)cmdBuf;

	// Set Command Parameters
	pCmd->cmdType = MSG_CMD_SET_MSG_SIZE_OPT;

	// Copy Cookie
	memcpy(pCmd->cmdCookie, mCookie, MAX_COOKIE_LEN);

	// Copy Command Data
	MSG_SETTING_S pSetting = {0,};
	pSetting.type = optionType;

	memcpy(&(pSetting.option.msgSizeOpt), msg_size_opt->data, sizeof(MSG_MSGSIZE_OPT_S));

    // Copy Command Data
    memcpy(pCmd->cmdData, &pSetting, cmdSize-sizeof(MSG_CMD_S));

	// Send Command to Messaging FW
	char* pEventData = NULL;
	AutoPtr<char> eventBuf(&pEventData);

	write((char*)pCmd, cmdSize, &pEventData);

	// Get Return Data
	MSG_EVENT_S* pEvent = (MSG_EVENT_S*)pEventData;

	if (pEvent->eventType != MSG_EVENT_SET_MSG_SIZE_OPT)
	{
		THROW(MsgException::INVALID_RESULT, "Event Data Error");
	}

	return pEvent->result;
}
