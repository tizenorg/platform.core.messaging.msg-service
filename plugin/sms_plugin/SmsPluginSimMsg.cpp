/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
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
#include "SmsPluginSimMsg.h"


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
	bClass2Msg = false;
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

	MSG_SIM_COUNT_S tmpMsgCnt = {};

	getSimMsgCount(&tmpMsgCnt);

	MSG_MESSAGE_INFO_S tmpMsgInfo = {};

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


MSG_ERROR_T SmsPluginSimMsg::saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList)
{
	// Reset Out Parameter
	pSimIdList->count = 0;

	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER;

	// Set SMS TPDU Options
	setSmsOptions(&(tpdu.data.deliver));

	// Set TimeStamp
	convertTimeStamp(pMsgInfo, &(tpdu.data.deliver));

	// Set SMSC Options
	SMS_ADDRESS_S smsc;
	SmsPluginTransport::instance()->setSmscOptions(&smsc);

	// Make SMS_SUBMIT_DATA_S from MSG_REQUEST_INFO_S
	SMS_SUBMIT_DATA_S submitData;
	SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme));

	// Check sim message full.
	if (checkSimMsgFull(submitData.segCount) == true)
	{
		MSG_DEBUG("SIM storage is full.");

		return MSG_ERR_SIM_STORAGE_FULL;
	}

	tpdu.data.deliver.userData.headerCnt = 0;

	if (submitData.segCount > 1)
		tpdu.data.deliver.bHeaderInd = true;

	int bufLen = 0, reqId = 0;

	char buf[MAX_TPDU_DATA_LEN];

	int addLen = strlen(submitData.destAddress.address);

	tpdu.data.deliver.originAddress.ton = submitData.destAddress.ton;
	tpdu.data.deliver.originAddress.npi = submitData.destAddress.npi;

	memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, addLen);
	tpdu.data.deliver.originAddress.address[addLen] = '\0';

	for (unsigned int segCnt = 0; segCnt < submitData.segCount; segCnt++)
	{
		if (submitData.userData[segCnt].header[0].udhType == SMS_UDH_CONCAT_8BIT ||
			submitData.userData[segCnt].header[0].udhType == SMS_UDH_CONCAT_16BIT)
		{
			tpdu.data.deliver.userData.headerCnt = 1;

			SmsPluginTransport::instance()->setConcatHeader(&(submitData.userData[segCnt].header[0]), &(tpdu.data.deliver.userData.header[0]));
		}

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

		MSG_DEBUG("Read Status [%d]", pMsgInfo->bRead);

		if (pMsgInfo->bRead == true)
			simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_READ;
		else
			simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;

		// Save SMS in SIM
		int ret = 0;

		ret = tel_write_sms_in_sim(&simSmsData, &reqId);

		if (ret == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_write_sms_in_sim Success !!! req Id : [%d] #######", reqId);
		}
		else
		{
		 	MSG_DEBUG("########  tel_write_sms_in_sim Fail !!! req Id : [%d] return : [%d] #######", reqId, ret);

			return MSG_ERR_PLUGIN_STORAGE;
		}

		MSG_SIM_ID_T SimId = 0;

		bool bResult = false;

		bResult = getSimEvent(&SimId);

		int usedCnt = 0;

		if (bResult == true)
		{
			MSG_DEBUG("########  Saving Msg was Successful !!! req Id : [%d] SIM ID : [%d] #######", reqId, SimId);

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
		 	MSG_DEBUG("########  Saving Msg was Failed !!! req Id : [%d] SIM ID : [%d] #######", reqId, SimId);

			return MSG_ERR_PLUGIN_STORAGE;
		}
	}

	return MSG_SUCCESS;
}


MSG_ERROR_T SmsPluginSimMsg::saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	// Reset Flag
	bClass2Msg = false;

	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER;

	// Set SMS TPDU Options
	setSmsOptions(&(tpdu.data.deliver));

	// Set TimeStamp
	convertTimeStamp(pMsgInfo, &(tpdu.data.deliver));

	// Set SMSC Options
	SMS_ADDRESS_S smsc;
	SmsPluginTransport::instance()->setSmscOptions(&smsc);

	// Make SMS_SUBMIT_DATA_S from MSG_REQUEST_INFO_S
	SMS_SUBMIT_DATA_S submitData;
	SmsPluginTransport::instance()->msgInfoToSubmitData(pMsgInfo, &submitData, &(tpdu.data.deliver.dcs.codingScheme));

	// Check sim message full.
	if (checkSimMsgFull(submitData.segCount) == true)
	{
		MSG_DEBUG("SIM storage is full.");

		return MSG_ERR_SIM_STORAGE_FULL;
	}

	tpdu.data.deliver.userData.headerCnt = 0;

	int bufLen = 0, reqId = 0;

	char buf[MAX_TPDU_DATA_LEN];

	int addrLen = strlen(submitData.destAddress.address);

	tpdu.data.deliver.originAddress.ton = submitData.destAddress.ton;
	tpdu.data.deliver.originAddress.npi = submitData.destAddress.npi;

	memcpy(tpdu.data.deliver.originAddress.address, submitData.destAddress.address, addrLen);
	tpdu.data.deliver.originAddress.address[addrLen] = '\0';

	tpdu.data.deliver.userData.length = submitData.userData[0].length;
	memcpy(tpdu.data.deliver.userData.data, submitData.userData[0].data, submitData.userData[0].length);

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

	simSmsData.MsgStatus = TAPI_NETTEXT_STATUS_UNREAD;

	// Save Class 2 Msg in SIM
	int tapiRet = TAPI_API_SUCCESS;

	tapiRet = tel_write_sms_in_sim(&simSmsData, &reqId);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_write_sms_in_sim Success !!! req Id : [%d] #######", reqId);

		// Set Flag
		bClass2Msg = true;

		memset(&simMsgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));
		memcpy(&simMsgInfo, pMsgInfo, sizeof(MSG_MESSAGE_INFO_S));

		usedCnt = MsgSettingGetInt(SIM_USED_COUNT);

		usedCnt++;

		if (MsgSettingSetInt(SIM_USED_COUNT, usedCnt) != MSG_SUCCESS)
		{
			MSG_DEBUG("Error to set config data [%s]", SIM_USED_COUNT);
		}
	}
	else
	{
	 	MSG_DEBUG("########  tel_write_sms_in_sim Fail !!! req Id : [%d] return : [%d] #######", reqId, tapiRet);

		SmsPluginTransport::instance()->sendDeliverReport(MSG_ERR_STORAGE_ERROR);

		return MSG_ERR_PLUGIN_STORAGE;
	}

	return MSG_SUCCESS;
}


void SmsPluginSimMsg::deleteSimMessage(MSG_SIM_ID_T SimMsgId)
{
	int tapiRet = TAPI_API_SUCCESS;
	int reqId = 0;

	tapiRet = tel_delete_sms_in_sim((int)SimMsgId, &reqId);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_delete_sms_in_sim Success !!! req Id : [%d] #######", reqId);
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "########  tel_delete_sms_in_sim Fail !!! req Id : [%d] return : [%d] #######", reqId, tapiRet);
	}

	MSG_SIM_ID_T SimId = 0;
	bool bResult = false;

	bResult = getSimEvent(&SimId);

	int usedCnt = 0, totalCnt = 0;

	if (bResult == true)
	{
		MSG_DEBUG("########  Deleting Msg was Successful !!! req Id : [%d] SIM ID : [%d] #######", reqId, SimId);

		usedCnt = MsgSettingGetInt(SIM_USED_COUNT);
		totalCnt = MsgSettingGetInt(SIM_TOTAL_COUNT);

		if (usedCnt == totalCnt)
		{
			tapiRet = tel_set_sms_memory_status(TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, &reqId);

			if (tapiRet == TAPI_API_SUCCESS)
			{
				MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! req Id : [%d] #######", reqId);
			}
			else
			{
				MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! req Id : [%d] return : [%d] #######", reqId, tapiRet);
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
	 	THROW(MsgException::SMS_PLG_ERROR, "########  Deleting Msg was Failed !!! req Id : [%d] SIM ID : [%d] #######", reqId, SimId);
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


void SmsPluginSimMsg::setReadStatus(MSG_SIM_ID_T SimMsgId)
{
	MSG_DEBUG("Sim Message ID [%d]", SimMsgId);

	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_set_sms_message_status((int)SimMsgId, TAPI_NETTEXT_STATUS_READ, &reqId);

	if (ret == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_set_sms_message_status Success !!! return : %d #######", ret);
	}
	else
	{
		THROW(MsgException::SMS_PLG_ERROR, "########  tel_set_sms_message_status Fail !!! return : %d #######", ret);
	}

	MSG_SIM_ID_T SimId = 0;
	bool bResult = false;

	bResult = getSimEvent(&SimId);

	if (bResult == true)
	{
		MSG_DEBUG("######## Setting Read Status was Successful !!! req Id : [%d] #######", reqId);
	}
	else
	{
	 	THROW(MsgException::SMS_PLG_ERROR, "######## Setting Read Status was Failed !!! req Id : [%d] #######", reqId);
	}
}


void SmsPluginSimMsg::getSimMsgCount(MSG_SIM_COUNT_S *pSimMsgCnt)
{
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_get_sms_count(&reqId);

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


bool SmsPluginSimMsg::getSimMsg(MSG_SIM_ID_T SimMsgId, MSG_MESSAGE_INFO_S *pMsgInfo)
{
	int reqId = 0;

	int ret = TAPI_API_SUCCESS;

	ret = tel_read_sms_in_sim(SimMsgId, &reqId);

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

	bTapiResult = false;

	mx.lock();

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


void SmsPluginSimMsg::setSimEvent(MSG_SIM_ID_T SimId, bool bResult)
{
	if (bClass2Msg == true)
	{
		MSG_ERROR_T err = MSG_SUCCESS;

		simMsgInfo.msgId = SimId;

		// Add Deliver Class2 Msg into DB
		err = SmsPluginStorage::instance()->addSmsMessage(&simMsgInfo);

		if (err == MSG_SUCCESS)
		{
			MSG_DEBUG("addMessage() Success !!");

			// Callback
			err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&simMsgInfo);

			if (err != MSG_SUCCESS)
			{
				MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
			}
		}
		else
		{
			MSG_DEBUG("addMessage() Error !! [%d]", err);
		}

		// Send Deliver Report
		SmsPluginTransport::instance()->sendDeliverReport(err);

		bClass2Msg = false;
	}
	else
	{
		mx.lock();

		simMsgId = SimId;
		bTapiResult = bResult;

		cv.signal();

		mx.unlock();
	}
}


bool SmsPluginSimMsg::getSimEvent(MSG_SIM_ID_T *pSimId)
{
	int ret = 0;

	bTapiResult = false;

	mx.lock();

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

