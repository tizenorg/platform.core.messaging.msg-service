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
#include "SmsPluginSimMsg.h"

extern struct tapi_handle *pTapiHandle;

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginSimMsg - Member Functions
==================================================================================================*/
SmsPluginSimMsg* SmsPluginSimMsg::pInstance = NULL;


SmsPluginSimMsg::SmsPluginSimMsg()
{
	// Initialize member variables
	simMsgId = 0;
	usedCnt = 0;
	totalCnt = 0;
	bTapiResult = false;
	memset(&simMsgDataInfo, 0x00, sizeof(simMsgDataInfo));
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


void SmsPluginSimMsg::initSimMessage()
{
	MSG_BEGIN();

	MSG_SIM_COUNT_S tmpMsgCnt;
	memset(&tmpMsgCnt, 0x00, sizeof(MSG_SIM_COUNT_S));

	getSimMsgCount(&tmpMsgCnt);

	MSG_MESSAGE_INFO_S tmpMsgInfo;
	memset(&tmpMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	for (int i = 0; i < tmpMsgCnt.usedCount; i++)
	{
		// Get SIM Msg
		if (getSimMsg(tmpMsgCnt.indexList[i], &tmpMsgInfo) == false)
			continue;

		if (SmsPluginStorage::instance()->addSimMessage(&tmpMsgInfo) != MSG_SUCCESS)
		{
			MSG_DEBUG("Fail to addSimMessage()");
		}
	}

	MSG_END();
}


msg_error_t SmsPluginSimMsg::saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	// Reset Out Parameter
	pSimIdList->count = 0;

	SMS_TPDU_S tpdu;
	memset(&tpdu, 0x00, sizeof(SMS_TPDU_S));

	tpdu.tpduType = SMS_TPDU_DELIVER;

	// Set SMS TPDU Options
	setSmsOptions(&(tpdu.data.deliver));

	// Set TimeStamp
	convertTimeStamp(pMsgInfo, &(tpdu.data.deliver));

	// Set SMSC Options
	SMS_ADDRESS_S smsc;
	memset(&smsc, 0x00, sizeof(SMS_ADDRESS_S));
	SmsPluginTransport::instance()->setSmscOptions(&smsc);

	// Make SMS_SUBMIT_DATA_S from MSG_REQUEST_INFO_S
	SMS_SUBMIT_DATA_S submitData;
	memset(&submitData, 0x00, sizeof(SMS_SUBMIT_DATA_S));
	SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme), 0);

	// Check sim message full.
	if (checkSimMsgFull(submitData.segCount) == true)
	{
		MSG_DEBUG("SIM storage is full.");

		return MSG_ERR_SIM_STORAGE_FULL;
	}

	tpdu.data.deliver.userData.headerCnt = 0;

	if (submitData.segCount > 1)
		tpdu.data.deliver.bHeaderInd = true;

	int bufLen = 0;

	char buf[MAX_TPDU_DATA_LEN];

	int addLen = strlen(submitData.destAddress.address);

	tpdu.data.deliver.originAddress.ton = submitData.destAddress.ton;
	tpdu.data.deliver.originAddress.npi = submitData.destAddress.npi;

	memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, addLen);
	tpdu.data.deliver.originAddress.address[addLen] = '\0';

	for (unsigned int segCnt = 0; segCnt < submitData.segCount; segCnt++)
	{
		tpdu.data.deliver.userData.length = submitData.userData[segCnt].length;
		memcpy(tpdu.data.deliver.userData.data, submitData.userData[segCnt].data, submitData.userData[segCnt].length);

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

#ifdef MSG_FOR_DEBUG
		MSG_DEBUG("Sim Message.");
		for (int j = 0; j < simSmsData.SmsData.MsgLength; j++)
		{
			MSG_DEBUG("[%02x]", simSmsData.SmsData.szData[j]);
		}
#endif

		MSG_DEBUG("Read Status [%d]", pMsgInfo->bRead);

		if (pMsgInfo->bRead == true)
			simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_READ;
		else
			simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;

		// Save SMS in SIM
		int ret = 0;

		ret = tel_write_sms_in_sim(pTapiHandle, &simSmsData, TapiEventSaveSimMsg, NULL);

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

			usedCnt = MsgSettingGetInt(SIM_USED_COUNT);

			usedCnt++;

			if (MsgSettingSetInt(SIM_USED_COUNT, usedCnt) != MSG_SUCCESS)
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

	return MSG_SUCCESS;
}


msg_error_t SmsPluginSimMsg::saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	// Reset Flag
	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER;

	// Set SMS TPDU Options
	setSmsOptions(&(tpdu.data.deliver));

	// Make SMS_SUBMIT_DATA_S from MSG_REQUEST_INFO_S to get segment count
	SMS_SUBMIT_DATA_S submitData;
	SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme), 0);

	// Check sim message full.
	if (checkSimMsgFull(submitData.segCount) == true)
	{
		MSG_DEBUG("SIM storage is full.");

		SmsPluginTransport::instance()->sendDeliverReport(MSG_ERR_SIM_STORAGE_FULL);

		return MSG_ERR_SIM_STORAGE_FULL;
	}

	// Create TelSmsData_t data
	TelSmsData_t simSmsData = {0,};

	memcpy(&simSmsData.SmsData.Sca, &simMsgDataInfo.sca, sizeof(simSmsData.SmsData.Sca));
	memcpy(&simSmsData.SmsData.szData, &simMsgDataInfo.szData, sizeof(simSmsData.SmsData.szData)-1);
	simSmsData.SmsData.MsgLength = simMsgDataInfo.msgLength;

	// Set message status
	simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;

	// Save Class 2 Msg in SIM
	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_write_sms_in_sim(pTapiHandle, &simSmsData, TapiEventSaveClass2Msg, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		memset(&simMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
		memcpy(&simMsgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		MSG_DEBUG("########  tel_write_sms_in_sim Success !!! #######");
	}
	else
	{
	 	MSG_DEBUG("########  tel_write_sms_in_sim Fail !!! return : [%d] #######", tapiRet);

		SmsPluginTransport::instance()->sendDeliverReport(MSG_ERR_STORAGE_ERROR);

		return MSG_ERR_PLUGIN_STORAGE;
	}

	return MSG_SUCCESS;
}


void SmsPluginSimMsg::deleteSimMessage(msg_sim_id_t SimMsgId)
{
	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_delete_sms_in_sim(pTapiHandle, (int)SimMsgId, TapiEventDeleteSimMsg, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_delete_sms_in_sim Success !!! #######");
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "########  tel_delete_sms_in_sim Fail !!! return : [%d] #######", tapiRet);
	}

	msg_sim_id_t SimId = 0;
	bool bResult = false;

	bResult = getSimEvent(&SimId);

	int usedCnt = 0, totalCnt = 0;

	if (bResult == true)
	{
		MSG_DEBUG("########  Deleting Msg was Successful !!! SIM ID : [%d] #######", SimId);

		usedCnt = MsgSettingGetInt(SIM_USED_COUNT);
		totalCnt = MsgSettingGetInt(SIM_TOTAL_COUNT);

		if (usedCnt == totalCnt)
		{
			tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, NULL, NULL);

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

		if (MsgSettingSetInt(SIM_USED_COUNT, usedCnt) != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
		}
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "########  Deleting Msg was Failed !!! SIM ID : [%d] #######", SimId);
	}
}


bool SmsPluginSimMsg::checkSimMsgFull(unsigned int SegCnt)
{
	int usedCnt = 0, totalCnt = 0;

	usedCnt = MsgSettingGetInt(SIM_USED_COUNT);
	totalCnt = MsgSettingGetInt(SIM_TOTAL_COUNT);

	MSG_DEBUG("Segment Count [%d]", SegCnt);
	MSG_DEBUG("usedCnt [%d], totalCnt [%d]", usedCnt, totalCnt);

	if ((usedCnt + (int)SegCnt) <= totalCnt)
		return false;
	else
		return true;
}


void SmsPluginSimMsg::setReadStatus(msg_sim_id_t SimMsgId)
{
	MSG_DEBUG("Sim Message ID [%d]", SimMsgId);

	int ret = TAPI_API_SUCCESS;

	ret = tel_set_sms_message_status(pTapiHandle, (int)SimMsgId, TAPI_NETTEXT_STATUS_READ, TapiEventSetMsgStatus, (void *)&SimMsgId);

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


void SmsPluginSimMsg::getSimMsgCount(MSG_SIM_COUNT_S *pSimMsgCnt)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_count(pTapiHandle, TapiEventGetSimMsgCnt, NULL);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_get_sms_count() Success !!! #######");
	}
	else
	{
		THROW(MsgException::SMS_PLG_ERROR, "########  tel_get_sms_count() Fail !!! return : %d #######", ret);
	}

	if (getSimMsgCntEvent(pSimMsgCnt) == true)
	{
		MSG_DEBUG("######## Get Sim Msg Count was Successful !!! #######");
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "######## Get Sim Msg Count was Failed !!! #######");
	}
}


bool SmsPluginSimMsg::getSimMsg(msg_sim_id_t SimMsgId, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	int ret = TAPI_API_SUCCESS;

	ret = tel_read_sms_in_sim(pTapiHandle, SimMsgId, TapiEventGetSimMsg, NULL);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("######## tel_read_sms_in_sim() Success !!! Sim ID : [%d] #######", SimMsgId);
	}
	else
	{
		MSG_DEBUG("########  tel_read_sms_in_sim() Fail !!! return : %d #######", ret);
		return false;
	}

	if (getSimMsgEvent(pMsgInfo) == true)
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


void SmsPluginSimMsg::setSmsOptions(SMS_DELIVER_S *pDeliver)
{
	pDeliver->bMoreMsg = false;
	pDeliver->bStatusReport = false;
	pDeliver->bHeaderInd = false;
	pDeliver->bReplyPath = false;

	pDeliver->dcs.bCompressed = false;
	pDeliver->dcs.msgClass = SMS_MSG_CLASS_NONE;
	pDeliver->dcs.codingGroup = SMS_GROUP_GENERAL;

	pDeliver->dcs.codingScheme = (SMS_CODING_SCHEME_T)MsgSettingGetInt(SMS_SEND_DCS);

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
	gmtime_r(&pMsgInfo->displayTime, &timeinfo);

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


void SmsPluginSimMsg::setSimMsgCntEvent(const MSG_SIM_COUNT_S *pSimMsgCnt)
{
	mx.lock();

	MSG_DEBUG("Sim Message Count is %d.", pSimMsgCnt->usedCount);

	for (int i=0; i < pSimMsgCnt->usedCount; i++)
	{
		MSG_DEBUG("Sim Message Index is %d.", pSimMsgCnt->indexList[i]);
	}

	if (MsgSettingSetInt(SIM_USED_COUNT, pSimMsgCnt->usedCount) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
	}

	if (MsgSettingSetInt(SIM_TOTAL_COUNT, (int)pSimMsgCnt->totalCount) != MSG_SUCCESS)
	{
		MSG_DEBUG("Error to set config data [%s]", SIM_TOTAL_COUNT);
	}

	memset(&simMsgCnt, 0x00, sizeof(MSG_SIM_COUNT_S));
	memcpy(&simMsgCnt, pSimMsgCnt, sizeof(MSG_SIM_COUNT_S));

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getSimMsgCntEvent(MSG_SIM_COUNT_S *pSimMsgCnt)
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

void SmsPluginSimMsg::setSimMsgEvent(const MSG_MESSAGE_INFO_S *pMsgInfo, bool bSuccess)
{
	mx.lock();

	bTapiResult = bSuccess;

	memset(&simMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	if (bTapiResult  == true)
	{
		MSG_DEBUG("Success to get sim msg - Id : [%d]", pMsgInfo->msgId);

		memcpy(&simMsgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));
	}

	cv.signal();

	mx.unlock();
}


bool SmsPluginSimMsg::getSimMsgEvent(MSG_MESSAGE_INFO_S *pMsgInfo)
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
	}

	return bTapiResult;
}


void SmsPluginSimMsg::setSaveSimMsgEvent(int simMsgId, int result)
{
	msg_error_t err = MSG_SUCCESS;

	mx.lock();

	if (result != TAPI_NETTEXT_SENDSMS_SUCCESS) {
		if (result == TAPI_NETTEXT_ROUTING_NOT_AVAILABLE)
			err = MSG_ERR_SIM_STORAGE_FULL;
		else
			err = MSG_ERR_UNKNOWN;
	}

	if (err == MSG_SUCCESS)
		bTapiResult = true;
	else
		bTapiResult = false;

	cv.signal();

	mx.unlock();

	// Send Deliver Report
	SmsPluginTransport::instance()->sendDeliverReport(err);

}


void SmsPluginSimMsg::setSaveClass2MsgEvent(int simMsgId, int result)
{
	msg_error_t err = MSG_SUCCESS;

	if (result == TAPI_NETTEXT_SENDSMS_SUCCESS && simMsgId >= 0) {

		simMsgInfo.msgId = simMsgId;

		err = SmsPluginStorage::instance()->addSimMessage(&simMsgInfo);

		if (err == MSG_SUCCESS)
		{
			MSG_DEBUG("addSimMessage() Success !!");

			// Callback
			err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&simMsgInfo);

			if (err != MSG_SUCCESS)
			{
				MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
			}

			usedCnt = MsgSettingGetInt(SIM_USED_COUNT);

			usedCnt++;

			if (MsgSettingSetInt(SIM_USED_COUNT, usedCnt) != MSG_SUCCESS)
			{
				MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
			}
		} else	{
			MSG_DEBUG("addMessage() Error !! [%d]", err);
		}
	} else {
			if (result == TAPI_NETTEXT_ROUTING_NOT_AVAILABLE)
				err = MSG_ERR_SIM_STORAGE_FULL;
			else
				err = MSG_ERR_UNKNOWN;
	}

	// Send Deliver Report
	SmsPluginTransport::instance()->sendDeliverReport(err);
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

void SmsPluginSimMsg::setSmsData(const char *sca, const char *szData, int msgLength)
{
	MSG_DEBUG("Set SMS data(class2 message)");

	memset(&simMsgDataInfo, 0x00, sizeof(simMsgDataInfo));

	memcpy(&simMsgDataInfo.sca, sca, sizeof(simMsgDataInfo.sca)-1);
	memcpy(&simMsgDataInfo.szData, szData, sizeof(simMsgDataInfo.szData)-1);
	simMsgDataInfo.msgLength = msgLength;
}
