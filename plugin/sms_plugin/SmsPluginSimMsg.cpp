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

#include <errno.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginTpduCodec.h"
#include "SmsPluginTransport.h"
#include "SmsPluginStorage.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginCallback.h"
#include "MsgUtilStorage.h"
#include "MsgNotificationWrapper.h"

#include "SmsPluginSimMsg.h"
#include "SmsPluginDSHandler.h"


/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginSimMsg - Member Functions
==================================================================================================*/
SmsPluginSimMsg* SmsPluginSimMsg::pInstance = NULL;


SmsPluginSimMsg::SmsPluginSimMsg()
{
	// Initialize member variables
	simMsgId = 0;
	delSimMsgId = -1;
	usedCnt = 0;
	totalCnt = 0;
	bTapiResult = false;
	memset(&simMsgDataInfo, 0x00, sizeof(simMsgDataInfo));
	memset(simIdList, 0, sizeof(int) * MAX_SIM_SMS_NUM);
	memset(&simMsgCnt, 0x00, sizeof(simMsgCnt));
	memset(&simMsgInfo, 0x00, sizeof(simMsgInfo));
	memset(&simAddrInfo, 0x00, sizeof(simAddrInfo));
}


SmsPluginSimMsg::~SmsPluginSimMsg()
{


}


SmsPluginSimMsg* SmsPluginSimMsg::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginSimMsg();

	return pInstance;
}


void SmsPluginSimMsg::initSimMessage(struct tapi_handle *handle)
{
	MSG_BEGIN();

	char keyName[MAX_VCONFKEY_NAME_LEN] = {0,};
	int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);

	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, sim_idx);
	// Set SIM count of vconf to 0
	if (MsgSettingSetInt(keyName, 0) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", keyName);
	}
	memset(keyName, 0, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_TOTAL_COUNT, sim_idx);

	if (MsgSettingSetInt(keyName, 0) != MSG_SUCCESS) {
		MSG_DEBUG("Error to set config data [%s]", keyName);
	}

	MSG_SIM_COUNT_S tmpMsgCnt;
	memset(&tmpMsgCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
	getSimMsgCount(handle, &tmpMsgCnt);

	MSG_MESSAGE_INFO_S tmpMsgInfo;
	int simIdList[MAX_SIM_SMS_NUM];
	int unreadSimMsg = 0;

	for (int i = 0; i < tmpMsgCnt.usedCount; i++)
	{
		memset(&tmpMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
		memset(simIdList, 0, sizeof(int) * MAX_SIM_SMS_NUM);

		// Get SIM Msg
		if (getSimMsg(handle, tmpMsgCnt.indexList[i], &tmpMsgInfo, simIdList) == false)
			continue;

		if (tmpMsgInfo.bRead == false)
			unreadSimMsg++;

		if (SmsPluginEventHandler::instance()->handleSimMsg(&tmpMsgInfo, simIdList, NULL, MAX_SIM_SMS_NUM) < 0) {
			MSG_DEBUG("Fail to handleSimMsg()");
		}

		if (tmpMsgInfo.addressList) {
			free(tmpMsgInfo.addressList);
			tmpMsgInfo.addressList = NULL;
		}
	}

	MSG_DEBUG("Unread SIM message count = [%d]", unreadSimMsg);
#ifndef MSG_NOTI_INTEGRATION
	if (unreadSimMsg > 0) {
		MsgRefreshNotification(MSG_NOTI_TYPE_SIM, true, false);
	}
#endif

	if (SmsPluginEventHandler::instance()->updateIMSI(sim_idx) != MSG_SUCCESS) {
		MSG_ERR("Fail to handleSimMsg()");
	}

	MSG_END();
}


msg_error_t SmsPluginSimMsg::saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	bool bSimSst = true;

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(pMsgInfo->sim_idx);
	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SERVICE_TABLE, pMsgInfo->sim_idx);
	if (MsgSettingGetBool(keyName, &bSimSst) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingGetBool [%s] failed", keyName);
		/* No return, default value is true. */

	if (bSimSst == false)
		return MSG_ERR_STORE_RESTRICT;

	// Reset Out Parameter
	pSimIdList->count = 0;

	SMS_TPDU_S tpdu;
	memset(&tpdu, 0x00, sizeof(SMS_TPDU_S));
	if(pMsgInfo->direction == MSG_DIRECTION_TYPE_MO)
	{
		tpdu.tpduType = SMS_TPDU_SUBMIT;
		tpdu.data.submit.dcs.msgClass = SMS_MSG_CLASS_NONE;
		//SmsPluginTransport::instance()->setSmsSendOptions(&(tpdu.data.submit));
		tpdu.data.submit.vpf = SMS_VPF_NOT_PRESENT;
	}
	else
	{
		tpdu.tpduType = SMS_TPDU_DELIVER;
		setSmsOptions(pMsgInfo, &(tpdu.data.deliver));
		// Set TimeStamp
		convertTimeStamp(pMsgInfo, &(tpdu.data.deliver));
	}

	for(int i=0; i <pMsgInfo->nAddressCnt; ++i)
	{
		SMS_SUBMIT_DATA_S submitData;
		memset(&submitData, 0x00, sizeof(SMS_SUBMIT_DATA_S));
		int bufLen = 0;
		char buf[MAX_TPDU_DATA_LEN];

		if(pMsgInfo->direction == MSG_DIRECTION_TYPE_MO) // SUBMIT MSG
		{
			SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.submit.dcs.codingScheme), i);

			int addLen = strlen(submitData.destAddress.address);

			tpdu.data.submit.destAddress.ton = submitData.destAddress.ton;
			tpdu.data.submit.destAddress.npi = submitData.destAddress.npi;

			if (addLen < MAX_ADDRESS_LEN) {
				memcpy(tpdu.data.submit.destAddress.address, submitData.destAddress.address, addLen);
				tpdu.data.submit.destAddress.address[addLen] = '\0';
			} else {
				memcpy(tpdu.data.submit.destAddress.address, submitData.destAddress.address, MAX_ADDRESS_LEN);
				tpdu.data.submit.destAddress.address[MAX_ADDRESS_LEN] = '\0';
			}

		} else { // DELIVER MSG
			SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme), i);

			int addLen = strlen(submitData.destAddress.address);

			tpdu.data.deliver.originAddress.ton = submitData.destAddress.ton;
			tpdu.data.deliver.originAddress.npi = submitData.destAddress.npi;

			if (addLen < MAX_ADDRESS_LEN) {
				memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, addLen);
				tpdu.data.deliver.originAddress.address[addLen] = '\0';
			} else {
				memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, MAX_ADDRESS_LEN);
				tpdu.data.deliver.originAddress.address[MAX_ADDRESS_LEN] = '\0';
			}
		}

		// Check sim message full.
		if (checkSimMsgFull(pMsgInfo->sim_idx, submitData.segCount) == true)
		{
			MSG_DEBUG("SIM storage is full.");
			MsgInsertTicker("Sim memory full. Delete some items", SMS_MESSAGE_SIM_MESSAGE_FULL, true, 0);

			return MSG_ERR_SIM_STORAGE_FULL;
		}

		if(submitData.segCount > 1)
		{
			if(pMsgInfo->direction == MSG_DIRECTION_TYPE_MO){ // SUBMIT MSG
				tpdu.data.submit.bHeaderInd = true;
			} else {
				tpdu.data.deliver.bHeaderInd = true;
			}

		}

		for (unsigned int segCnt = 0; segCnt < submitData.segCount; segCnt++)
		{
			if(pMsgInfo->direction == MSG_DIRECTION_TYPE_MO){
				memcpy(&(tpdu.data.submit.userData), &(submitData.userData[segCnt]), sizeof(SMS_USERDATA_S));
			} else {
				memcpy(&(tpdu.data.deliver.userData), &(submitData.userData[segCnt]), sizeof(SMS_USERDATA_S));
			}

			memset(buf, 0x00, sizeof(buf));

			// Encode SMS-DELIVER TPDU
			bufLen = SmsPluginTpduCodec::encodeTpdu(&tpdu, buf);

			// Make Telephony Structure
			TelSmsData_t simSmsData;
			memset((void*)&simSmsData, 0x00, sizeof(simSmsData));

			// Set TPDU data
			memcpy((void*)simSmsData.SmsData.szData, buf, bufLen);

			simSmsData.SmsData.szData[bufLen] = 0;
			simSmsData.SmsData.MsgLength = bufLen;
			simSmsData.SmsData.format = TAPI_NETTEXT_NETTYPE_3GPP;

			if(pMsgInfo->direction == MSG_DIRECTION_TYPE_MT) { // MT messages
				if (pMsgInfo->bRead == true)
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_READ;
				else
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;
			} else { // MO messages
				if (pMsgInfo->networkStatus == MSG_NETWORK_SEND_SUCCESS)
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_SENT;
				else if (pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_SUCCESS)
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_DELIVERED;
				else if (pMsgInfo->networkStatus == MSG_NETWORK_DELIVER_FAIL)
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_DELIVERY_UNCONFIRMED;
				else
					simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNSENT;
			}

			// Save SMS in SIM
			int ret = 0;

			ret = tel_write_sms_in_sim(handle, &simSmsData, TapiEventSaveSimMsg, NULL);

			if (ret == TAPI_API_SUCCESS)
			{
				MSG_DEBUG("########  tel_write_sms_in_sim Success !!!#######");
			}
			else
			{
			 	MSG_DEBUG("########  tel_write_sms_in_sim Fail !!! return : [%d] #######", ret);

				return MSG_ERR_PLUGIN_STORAGE;
			}

			msg_sim_id_t SimId = 0;

			bool bResult = false;

			bResult = getSimEvent(&SimId);

			int usedCnt = 0;

			if (bResult == true)
			{
				MSG_DEBUG("########  Saving Msg was Successful !!! SIM ID : [%d] #######", SimId);

				char keyName[MAX_VCONFKEY_NAME_LEN];
				memset(keyName, 0x00, sizeof(keyName));
				snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, pMsgInfo->sim_idx);
				usedCnt = MsgSettingGetInt(keyName);

				usedCnt++;

				if (MsgSettingSetInt(keyName, usedCnt) != MSG_SUCCESS)
				{
					MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
				}

				pSimIdList->simId[pSimIdList->count] = SimId;
				pSimIdList->count++;
			}
			else
			{
			 	MSG_DEBUG("########  Saving Msg was Failed !!! SIM ID : [%d] #######", SimId);

				return MSG_ERR_PLUGIN_STORAGE;
			}
		}
	}
	return MSG_SUCCESS;
}


msg_error_t SmsPluginSimMsg::saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo)
{

	msg_error_t err = MSG_SUCCESS;
	bool bSimSst = true;
	int tapiRet = TAPI_API_SUCCESS;
	int simId = -1;
	int replaceSimId = -1;
	int replaceMsgId = 0;

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(pMsgInfo->sim_idx);

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", MSG_SIM_SERVICE_TABLE, pMsgInfo->sim_idx);
	if (MsgSettingGetBool(keyName, &bSimSst) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingGetBool [%s] failed", keyName);
	/* No return, default value is true. */

	if (bSimSst == false)
	{
		SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_SUCCESS);
		return MSG_SUCCESS;
	}

	// Reset Flag
	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER;

	convertTimeStamp(pMsgInfo, &(tpdu.data.deliver));

	// Set SMS TPDU Options
	setSmsOptions(pMsgInfo, &(tpdu.data.deliver));

	SMS_SUBMIT_DATA_S submitData;
	int bufLen = 0;
	char buf[MAX_TPDU_DATA_LEN];

	SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme), 0);

	if (pMsgInfo->msgType.subType >= MSG_REPLACE_TYPE1_SMS && pMsgInfo->msgType.subType <= MSG_REPLACE_TYPE7_SMS) {
		err = SmsPluginStorage::instance()->getReplaceSimMsg(pMsgInfo, &replaceMsgId, &replaceSimId);
		MSG_DEBUG("getReplaceSimMsg(): err=[%d], Replace Sim Id = [%d], Replace message id = [%d]", err, replaceSimId, replaceMsgId);
		if (replaceMsgId < 0)
			replaceMsgId = 0;
	}

	// Check SIM ID
	if (replaceSimId < 0) { // Normal message type
		// Check sim message full.
		if (checkSimMsgFull(pMsgInfo->sim_idx, submitData.segCount) == true)
		{
			MSG_DEBUG("SIM storage is full.");

			SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_ERR_SIM_STORAGE_FULL);

			return MSG_ERR_SIM_STORAGE_FULL;
		}

		int addLen = strlen(submitData.destAddress.address);

		tpdu.data.deliver.originAddress.ton = submitData.destAddress.ton;
		tpdu.data.deliver.originAddress.npi = submitData.destAddress.npi;

		if (addLen < MAX_ADDRESS_LEN) {
			memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, addLen);
			tpdu.data.deliver.originAddress.address[addLen] = '\0';
		} else {
			memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, MAX_ADDRESS_LEN);
			tpdu.data.deliver.originAddress.address[MAX_ADDRESS_LEN] = '\0';
		}

		if (submitData.segCount > 1)
			tpdu.data.deliver.bHeaderInd = true;
	} else { // Replace message type
		tapiRet = tel_delete_sms_in_sim(handle, replaceSimId, TapiEventDeleteSimMsg, NULL);

		if (tapiRet == TAPI_API_SUCCESS) {
			MSG_DEBUG("########  tel_delete_sms_in_sim Success !!! #######");

			simId = -1;
			if(getDelSimEvent(&simId) == true) {
				err = SmsPluginStorage::instance()->deleteSimMessage(pMsgInfo->sim_idx, replaceSimId);
			}
			MSG_DEBUG("tel_delete_sms_in_sim() : Err=[%d], Replace Sim Id=[%d], Result Sim id=[%d]", err, replaceSimId, simId);

		} else {
			MSG_DEBUG("########  tel_delete_sms_in_sim Fail !!! return : [%d] #######", tapiRet);
		}
	}

	for (unsigned int segCnt = 0; segCnt < submitData.segCount; segCnt++) {
		// Create TelSmsData_t data
		TelSmsData_t simSmsData = {0,};

		if (submitData.segCount == 1) {
			memcpy(&simSmsData.SmsData.Sca, &simMsgDataInfo.sca, sizeof(simSmsData.SmsData.Sca));
			memcpy(&simSmsData.SmsData.szData, &simMsgDataInfo.szData, sizeof(simSmsData.SmsData.szData)-1);
			simSmsData.SmsData.MsgLength = simMsgDataInfo.msgLength;

		} else {
			memcpy(&(tpdu.data.deliver.userData), &(submitData.userData[segCnt]), sizeof(SMS_USERDATA_S));

			memset(buf, 0x00, sizeof(buf));

			// Encode SMS-DELIVER TPDU
			bufLen = SmsPluginTpduCodec::encodeTpdu(&tpdu, buf);

			// Set TPDU data
			memcpy((void*)simSmsData.SmsData.Sca, &simMsgDataInfo.sca, sizeof(simSmsData.SmsData.Sca));
			memcpy((void*)simSmsData.SmsData.szData, buf, bufLen);
			simSmsData.SmsData.szData[bufLen] = 0;
			simSmsData.SmsData.MsgLength = bufLen;
		}

		simSmsData.SmsData.format = TAPI_NETTEXT_NETTYPE_3GPP;
		simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;

		MSG_MESSAGE_INFO_S *tmpSimMsgInfo = (MSG_MESSAGE_INFO_S *)calloc(1, sizeof(MSG_MESSAGE_INFO_S));
		if (tmpSimMsgInfo) {
			memcpy(tmpSimMsgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

			tmpSimMsgInfo->msgId = replaceMsgId;

			tmpSimMsgInfo->addressList = NULL;
			tmpSimMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
			memcpy(&tmpSimMsgInfo->addressList[0], &pMsgInfo->addressList[0], sizeof(MSG_ADDRESS_INFO_S));

			tapiRet = tel_write_sms_in_sim(handle, &simSmsData, TapiEventSaveClass2Msg, tmpSimMsgInfo);

			if (tapiRet == TAPI_API_SUCCESS) {
				MSG_DEBUG("########  tel_write_sms_in_sim Success !!!, segNum = [%d] #######", segCnt);
			} else {
			 	MSG_DEBUG("########  tel_write_sms_in_sim Fail !!! return : [%d] #######", tapiRet);

				SmsPluginTransport::instance()->sendDeliverReport(handle, MSG_ERR_STORAGE_ERROR);

				return MSG_ERR_PLUGIN_STORAGE;
			}
		}
		msg_sim_id_t retSimId;
		if (!getSimEvent(&retSimId))
			return MSG_ERR_PLUGIN_STORAGE;
	}

	return MSG_SUCCESS;
}


void SmsPluginSimMsg::deleteSimMessage(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	int tapiRet = TAPI_API_SUCCESS;

	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(sim_idx);
	tapiRet = tel_delete_sms_in_sim(handle, (int)SimMsgId, TapiEventDeleteSimMsg, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_delete_sms_in_sim Success !!! #######");
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "########  tel_delete_sms_in_sim Fail !!! return : [%d] #######", tapiRet);
	}

	int SimId = 0;
	bool bResult = false;

	bResult = getDelSimEvent(&SimId);

	int usedCnt = 0, totalCnt = 0;

	if (bResult == true)
	{
		MSG_DEBUG("########  Deleting Msg was Successful !!! SIM ID : [%d] #######", SimId);
		char keyName[MAX_VCONFKEY_NAME_LEN];
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, sim_idx);
		usedCnt = MsgSettingGetInt(keyName);
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", SIM_TOTAL_COUNT, sim_idx);
		totalCnt = MsgSettingGetInt(keyName);

		if (usedCnt == totalCnt)
		{
			tapiRet = tel_set_sms_memory_status(handle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, NULL, NULL);

			if (tapiRet == TAPI_API_SUCCESS)
			{
				MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
			}
			else
			{
				MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! return : [%d] #######", tapiRet);
			}
		}

		usedCnt--;

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, sim_idx);

		if (MsgSettingSetInt(keyName, usedCnt) != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data [%s]", keyName);
		}
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "########  Deleting Msg was Failed !!! SIM ID : [%d] #######", SimId);
	}
}


bool SmsPluginSimMsg::checkSimMsgFull(msg_sim_slot_id_t sim_idx, unsigned int SegCnt)
{
	int usedCnt = 0, totalCnt = 0;

	char keyName[MAX_VCONFKEY_NAME_LEN];
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, sim_idx);
	usedCnt = MsgSettingGetInt(keyName);
	memset(keyName, 0x00, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_TOTAL_COUNT, sim_idx);
	totalCnt = MsgSettingGetInt(keyName);

	MSG_DEBUG("Segment Count [%d]", SegCnt);
	MSG_DEBUG("usedCnt [%d], totalCnt [%d]", usedCnt, totalCnt);

	if ((usedCnt + (int)SegCnt) <= totalCnt)
		return false;
	else
		return true;
}


void SmsPluginSimMsg::setReadStatus(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId)
{
	MSG_DEBUG("Sim Message ID [%d]", SimMsgId);

	int ret = TAPI_API_SUCCESS;
	struct tapi_handle *handle = SmsPluginDSHandler::instance()->getTelHandle(sim_idx);

	ret = tel_set_sms_message_status(handle, (int)SimMsgId, TAPI_NETTEXT_STATUS_READ, TapiEventSetMsgStatus, (void *)&SimMsgId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_set_sms_message_status Success !!! return : %d #######", ret);
	}
	else
	{
		THROW(MsgException::SMS_PLG_ERROR, "########  tel_set_sms_message_status Fail !!! return : %d #######", ret);
	}

	msg_sim_id_t SimId = 0;
	bool bResult = false;

	bResult = getSimEvent(&SimId);

	if (bResult == true)
	{
		MSG_DEBUG("######## Setting Read Status was Successful !!!, sim id=[%d] #######", SimId);
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "######## Setting Read Status was Failed !!! #######");
	}
}


void SmsPluginSimMsg::getSimMsgCount(struct tapi_handle *handle, MSG_SIM_COUNT_S *pSimMsgCnt)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_count(handle, TapiEventGetSimMsgCnt, NULL);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_get_sms_count() Success !!! #######");
	}
	else
	{
		THROW(MsgException::SMS_PLG_ERROR, "########  tel_get_sms_count() Fail !!! return : %d #######", ret);
	}

	if (getSimMsgCntEvent(handle, pSimMsgCnt) == true)
	{
		MSG_DEBUG("######## Get Sim Msg Count was Successful !!! #######");
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "######## Get Sim Msg Count was Failed !!! #######");
	}
}


bool SmsPluginSimMsg::getSimMsg(struct tapi_handle *handle, msg_sim_id_t SimMsgId, MSG_MESSAGE_INFO_S *pMsgInfo, int *simIdList)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_read_sms_in_sim(handle, SimMsgId, TapiEventGetSimMsg, simIdList);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_read_sms_in_sim() Success !!! Sim ID : [%d] #######", SimMsgId);
	}
	else
	{
		MSG_DEBUG("########  tel_read_sms_in_sim() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getSimMsgEvent(handle, pMsgInfo) == true)
	{
		MSG_DEBUG("######## Get Sim Msg was Successful !!! #######");
	}
	else
	{
	 	MSG_DEBUG("######## Get Sim Msg was Failed !!! #######");
		return false;
	}

	return true;
}


void SmsPluginSimMsg::setSmsOptions(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver)
{
	pDeliver->bMoreMsg = false;
	pDeliver->bStatusReport = false;
	pDeliver->bHeaderInd = false;
	pDeliver->bReplyPath = false;

	pDeliver->dcs.bCompressed = false;
	pDeliver->dcs.msgClass = SMS_MSG_CLASS_NONE;
	pDeliver->dcs.codingGroup = SMS_GROUP_GENERAL;

	/* use encoding type of received message instead of message settings */
	//pDeliver->dcs.codingScheme = (SMS_CODING_SCHEME_T)MsgSettingGetInt(SMS_SEND_DCS);
	pDeliver->dcs.codingScheme = pMsgInfo->encodeType;

	MSG_DEBUG("DCS : %d", pDeliver->dcs.codingScheme);

	pDeliver->pid = SMS_PID_NORMAL;

	MSG_DEBUG("PID : %d", pDeliver->pid);
}


void SmsPluginSimMsg::convertTimeStamp(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver)
{
	MSG_BEGIN();

	// encode time stamp
	pDeliver->timeStamp.format = SMS_TIME_ABSOLUTE;

	// encode absolute time
	struct tm timeinfo = {0,};
	tzset();
	localtime_r(&pMsgInfo->displayTime, &timeinfo);

	pDeliver->timeStamp.time.absolute.year = timeinfo.tm_year - 100;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.year is %d",pDeliver->timeStamp.time.absolute.year);

	pDeliver->timeStamp.time.absolute.month = timeinfo.tm_mon + 1;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.month is %d",pDeliver->timeStamp.time.absolute.month);

	pDeliver->timeStamp.time.absolute.day = timeinfo.tm_mday;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.day is %d",pDeliver->timeStamp.time.absolute.day);

	pDeliver->timeStamp.time.absolute.hour = timeinfo.tm_hour;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.hour is %d",pDeliver->timeStamp.time.absolute.hour);

	pDeliver->timeStamp.time.absolute.minute = timeinfo.tm_min;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.minute is %d",pDeliver->timeStamp.time.absolute.minute);

	pDeliver->timeStamp.time.absolute.second = timeinfo.tm_sec;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.second is %d",pDeliver->timeStamp.time.absolute.second);

	pDeliver->timeStamp.time.absolute.timeZone = 0;
	MSG_DEBUG("pDeliver->timeStamp.time.absolute.timeZone is %d",pDeliver->timeStamp.time.absolute.timeZone);

	MSG_END();
}


void SmsPluginSimMsg::setSimMsgCntEvent(struct tapi_handle *handle, const MSG_SIM_COUNT_S *pSimMsgCnt)
{
	mx.lock();

	MSG_DEBUG("Sim Message Count is %d.", pSimMsgCnt->usedCount);

	int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);
	char keyName[MAX_VCONFKEY_NAME_LEN]= {0,};

	for (int i=0; i < pSimMsgCnt->usedCount; i++)
	{
		MSG_DEBUG("Sim Message Index is %d.", pSimMsgCnt->indexList[i]);
	}

	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, sim_idx);
	if (MsgSettingSetInt(keyName, pSimMsgCnt->usedCount) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
	}

	memset(keyName, 0, sizeof(keyName));
	snprintf(keyName, sizeof(keyName), "%s/%d", SIM_TOTAL_COUNT, sim_idx);
	if (MsgSettingSetInt(keyName, (int)pSimMsgCnt->totalCount) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_TOTAL_COUNT);
	}

	memset(&simMsgCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
	memcpy(&simMsgCnt, pSimMsgCnt, sizeof(MSG_SIM_COUNT_S));

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getSimMsgCntEvent(struct tapi_handle *handle, MSG_SIM_COUNT_S *pSimMsgCnt)
{
	int ret = 0;

	mx.lock();

	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memcpy(pSimMsgCnt, &simMsgCnt, sizeof(MSG_SIM_COUNT_S));

	return true;
}

void SmsPluginSimMsg::setSimMsgEvent(struct tapi_handle *handle, const MSG_MESSAGE_INFO_S *pMsgInfo, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&simMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
	memset(&simAddrInfo, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	if (bTapiResult  == true)
	{
		MSG_DEBUG("Success to get sim msg - Id : [%d]", pMsgInfo->msgId);

		memcpy(&simMsgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
		simMsgInfo.addressList = &simAddrInfo;
		memcpy(&simAddrInfo,pMsgInfo->addressList, sizeof(MSG_ADDRESS_INFO_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getSimMsgEvent(struct tapi_handle *handle, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	memset(pMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	if (bTapiResult == true)
	{
		memcpy(pMsgInfo, &simMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
		pMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)calloc(1, sizeof(MSG_ADDRESS_INFO_S));
		memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));
		memcpy(pMsgInfo->addressList, simMsgInfo.addressList, sizeof(MSG_ADDRESS_INFO_S));
		pMsgInfo->sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);
	}


	return bTapiResult;
}


void SmsPluginSimMsg::setSaveSimMsgEvent(struct tapi_handle *handle, int simId, int result)
{
	msg_error_t err = MSG_SUCCESS;

	mx.lock();

	if (result != TAPI_NETTEXT_SENDSMS_SUCCESS) {
		if (result == TAPI_NETTEXT_ROUTING_NOT_AVAILABLE || result == TAPI_NETTEXT_SIM_FULL)
			err = MSG_ERR_SIM_STORAGE_FULL;
		else
			err = MSG_ERR_UNKNOWN;
	}

	if (err == MSG_SUCCESS)
		bTapiResult = true;
	else
		bTapiResult = false;

	simMsgId = simId;

	cv.signal();

	mx.unlock();

	int tapiRet = TAPI_API_SUCCESS;

	if (err == MSG_SUCCESS) {
		tapiRet = tel_set_sms_memory_status(handle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, TapiEventMemoryStatus, NULL);
	} else if (err == MSG_ERR_SIM_STORAGE_FULL) {
		tapiRet = tel_set_sms_memory_status(handle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);
	} else {
		return;
	}

	if (tapiRet == TAPI_API_SUCCESS) {
		MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
	} else {
		MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
	}
}


void SmsPluginSimMsg::setSaveClass2MsgEvent(struct tapi_handle *handle, int simId, int result, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	msg_error_t err = MSG_SUCCESS;
//	int sim_idx = SmsPluginDSHandler::instance()->getSimIndex(handle);

	if (result == TAPI_NETTEXT_SENDSMS_SUCCESS && simId >= 0 && pMsgInfo) {
		bool isNewSimMsg = true;

		for (int i = 0; i < MAX_SIM_SMS_NUM; i++) {
			if (simIdList[i] != 0) {
				MSG_DEBUG("simIdList[%d] is exist [%d]", i, simIdList[i]);
				continue;
			} else {
				simIdList[i] = simId + 1;
				MSG_DEBUG("simIdList[%d] is assigned [%d]", i, simId + 1);
				break;
			}
		}

		if (pMsgInfo->msgType.subType >= MSG_REPLACE_TYPE1_SMS && pMsgInfo->msgType.subType <= MSG_REPLACE_TYPE7_SMS) {
			if (pMsgInfo->msgId > 0) {
				isNewSimMsg = false;
			}
		}

		if (simMsgDataInfo.totalSegment >= 1 && simIdList[simMsgDataInfo.totalSegment-1] != 0) {
			msg_message_id_t saved_msg_id = 0;
			SmsPluginEventHandler::instance()->handleSimMsg(pMsgInfo, simIdList, &saved_msg_id, MAX_SIM_SMS_NUM);

			MSG_DEBUG("Saved message ID = [%d]", saved_msg_id);

			if (saved_msg_id > 0)
				pMsgInfo->msgId = saved_msg_id;

			err = SmsPluginEventHandler::instance()->callbackMsgIncoming(pMsgInfo);

			if (err != MSG_SUCCESS)
			{
				MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
			}
		}

		if (isNewSimMsg == true) {
			char keyName[MAX_VCONFKEY_NAME_LEN];
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", SIM_USED_COUNT, pMsgInfo->sim_idx);
			usedCnt = MsgSettingGetInt(keyName);
			usedCnt++;

			if (MsgSettingSetInt(keyName, usedCnt) != MSG_SUCCESS)
			{
				MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
			}
		}

		if (simMsgDataInfo.totalSegment >= 1 && simIdList[simMsgDataInfo.totalSegment-1] != 0) {
			memset(simIdList, 0, sizeof(int) * MAX_SIM_SMS_NUM);
		}
	} else {
		if (result == TAPI_NETTEXT_SIM_FULL)
			err = MSG_ERR_SIM_STORAGE_FULL;
		else
			err = MSG_ERR_UNKNOWN;
	}

	// Send Deliver Report
	SmsPluginTransport::instance()->sendDeliverReport(handle, err);
}


void SmsPluginSimMsg::setSimEvent(msg_sim_id_t SimId, bool bResult)
{
	mx.lock();

	simMsgId = SimId;
	bTapiResult = bResult;

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getSimEvent(msg_sim_id_t *pSimId)
{
	int ret = 0;

	mx.lock();

	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	*pSimId = simMsgId;

	MSG_DEBUG("Returned SimMsgId is %d.", simMsgId);

	return bTapiResult;
}


void SmsPluginSimMsg::setDelSimEvent(int SimId, bool bResult)
{
	mx.lock();

	delSimMsgId = SimId;
	bTapiResult = bResult;

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getDelSimEvent(int *pSimId)
{
	int ret = 0;

	mx.lock();

	delSimMsgId = -1;
	bTapiResult = false;
	ret = cv.timedwait(mx.pMutex(), 10);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: TAPI callback TIME-OUT");
		return false;
	}

	*pSimId = delSimMsgId;

	MSG_DEBUG("Returned delSimMsgId is %d.", delSimMsgId);

	return bTapiResult;
}


void SmsPluginSimMsg::setSmsData(const char *sca, const char *szData, int msgLength)
{
	MSG_DEBUG("Set SMS data(class2 message)");

	memset(&simMsgDataInfo, 0x00, sizeof(simMsgDataInfo));

	memcpy(&simMsgDataInfo.sca, sca, sizeof(simMsgDataInfo.sca)-1);
	memcpy(&simMsgDataInfo.szData, szData, sizeof(simMsgDataInfo.szData)-1);
	simMsgDataInfo.msgLength = msgLength;
}

void SmsPluginSimMsg::setSmsTpduTotalSegCount(int totalSeg)
{
	MSG_DEBUG("Set SMS Segements Info");

	simMsgDataInfo.totalSegment = totalSeg;
}

