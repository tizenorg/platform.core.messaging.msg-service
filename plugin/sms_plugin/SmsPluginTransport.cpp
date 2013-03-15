/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <errno.h>
#include <math.h>

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgGconfWrapper.h"
#include "MsgUtilFile.h"
#include "MsgNotificationWrapper.h"
#include "SmsPluginParamCodec.h"
#include "SmsPluginTpduCodec.h"
#include "SmsPluginEventHandler.h"
#include "SmsPluginStorage.h"
#include "SmsPluginCallback.h"
#include "SmsPluginTransport.h"

extern "C"
{
	#include <ITapiNetText.h>
}


extern struct tapi_handle *pTapiHandle;

/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginTransport - Member Functions
==================================================================================================*/
SmsPluginTransport* SmsPluginTransport::pInstance = NULL;


SmsPluginTransport::SmsPluginTransport()
{
	msgRef 		= 0x00;
	msgRef8bit 	= 0x00;
	msgRef16bit	= 0x0000;
}


SmsPluginTransport::~SmsPluginTransport()
{

}


SmsPluginTransport* SmsPluginTransport::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginTransport();

	return pInstance;
}


void SmsPluginTransport::submitRequest(SMS_REQUEST_INFO_S *pReqInfo)
{
	MSG_BEGIN();

	SMS_TPDU_S tpdu = {0,};

	tpdu.tpduType = SMS_TPDU_SUBMIT;

	// Set SMS Send Options - Setting
	setSmsSendOptions(&(tpdu.data.submit));

	// Set SMS Send Options - Each Message
	if (pReqInfo->sendOptInfo.bSetting == true)
	{
		tpdu.data.submit.bStatusReport = pReqInfo->sendOptInfo.bDeliverReq;
		tpdu.data.submit.bReplyPath = pReqInfo->sendOptInfo.option.smsSendOptInfo.bReplyPath;
	}

	// Set Coding Scheme for apps that use port number
	if (pReqInfo->msgInfo.msgPort.valid == true)
	{
		tpdu.data.submit.dcs.codingScheme = (SMS_CODING_SCHEME_T)pReqInfo->msgInfo.encodeType;

		MSG_DEBUG("DCS is changed by application : [%d]", tpdu.data.submit.dcs.codingScheme);
	}

#ifdef MSG_SMS_REPORT
	// Update Msg Ref into Report Table
	if (tpdu.data.submit.bStatusReport == true)
	{
		MSG_DEBUG("Update Msg Ref [%d] in Report Table", tpdu.data.submit.msgRef);

		SmsPluginStorage::instance()->updateMsgRef(pReqInfo->msgInfo.msgId, tpdu.data.submit.msgRef);
	}
#endif

	// Set SMSC Options
	SMS_ADDRESS_S smsc;
	setSmscOptions(&smsc);
	int i = 0;
	int j = 0;

	MSG_DEBUG("pReqInfo->msgInfo.nAddressCnt [%d]", pReqInfo->msgInfo.nAddressCnt);

	for (i = 0; i < pReqInfo->msgInfo.nAddressCnt; i++)
	{
		// Make SMS_SUBMIT_DATA_S from MSG_REQUEST_INFO_S
		SMS_SUBMIT_DATA_S submitData = {{0},};
		msgInfoToSubmitData(&(pReqInfo->msgInfo), &submitData, &(tpdu.data.submit.dcs.codingScheme), i);

		// Encode SMSC Address
		unsigned char smscAddr[MAX_SMSC_LEN];
		memset(smscAddr, 0x00, sizeof(smscAddr));

		int smscLen = SmsPluginParamCodec::encodeSMSC(&smsc, smscAddr);

		if (smscLen <= 0) {
			MSG_DEBUG("smscLen <= 0");
			return;
		}

		char smscAddrTmp[(smscLen*2)+1];
		memset(smscAddrTmp, 0x00, sizeof(smscAddrTmp));
		for (j = 0; j < smscLen; j++) {
			snprintf(smscAddrTmp+(j*2), sizeof(smscAddrTmp)-(j*2), "%02X", smscAddr[j]);
		}
		MSG_DEBUG("pSCAInfo [%s]", smscAddrTmp);

		int bufLen = 0;

		char buf[MAX_TPDU_DATA_LEN];

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

MSG_DEBUG("ton [%d]", tpdu.data.submit.destAddress.ton);
MSG_DEBUG("npi [%d]", tpdu.data.submit.destAddress.npi);
MSG_DEBUG("address [%s]", tpdu.data.submit.destAddress.address);

		bool bStatusReport = false;

		for (unsigned int segCnt = 0; segCnt < submitData.segCount; segCnt++)
		{
			if (submitData.userData[segCnt].headerCnt > 0) {
				tpdu.data.submit.bHeaderInd = true;
			} else {
				tpdu.data.submit.bHeaderInd = false;
			}

			if (segCnt == 0 && submitData.segCount > 1) {
				bStatusReport = tpdu.data.submit.bStatusReport;
				tpdu.data.submit.bStatusReport = false;
			} else if ((segCnt+1 == submitData.segCount) && submitData.segCount > 1) {
				tpdu.data.submit.bStatusReport = bStatusReport;
			}

			memset(&(tpdu.data.submit.userData), 0x00, sizeof(SMS_USERDATA_S));
			memcpy(&(tpdu.data.submit.userData), &(submitData.userData[segCnt]), sizeof(SMS_USERDATA_S));

			// Encode SMS-SUBMIT TPDU
			memset(buf, 0x00, sizeof(buf));

			bufLen = SmsPluginTpduCodec::encodeTpdu(&tpdu, buf);

			// Make Telephony Structure
			TelSmsDatapackageInfo_t pkgInfo;

			// Set TPDU data
			memset((void*)pkgInfo.szData, 0x00, sizeof(pkgInfo.szData));
			memcpy((void*)pkgInfo.szData, buf, bufLen);

			pkgInfo.szData[bufLen] = 0;
			pkgInfo.MsgLength = bufLen;

			// Set SMSC data
			memset(pkgInfo.Sca, 0x00, sizeof(pkgInfo.Sca));
			memcpy((void*)pkgInfo.Sca, smscAddr, smscLen);
			pkgInfo.Sca[smscLen] = '\0';


			char pkgInfoTmp[(pkgInfo.MsgLength*2)+1];
			memset(pkgInfoTmp, 0x00, sizeof(pkgInfoTmp));
			for (j = 0; j < pkgInfo.MsgLength; j++) {
				snprintf(pkgInfoTmp+(j*2), sizeof(pkgInfoTmp)-(j*2), "%02X", pkgInfo.szData[j]);
			}
			MSG_DEBUG("Submit Request TPDU.");
			MSG_DEBUG("[%s]", pkgInfoTmp);

			SMS_SENT_INFO_S sentInfo;
			memset(&sentInfo, 0x00, sizeof(SMS_SENT_INFO_S));

			bool bMoreMsg = FALSE;

			memcpy(&(sentInfo.reqInfo), pReqInfo, sizeof(SMS_REQUEST_INFO_S));

			if ((segCnt+1) == submitData.segCount && (i+1)==pReqInfo->msgInfo.nAddressCnt)
			{
				sentInfo.bLast = true;

				bMoreMsg = FALSE;
			}
			else
			{
				sentInfo.bLast = false;

				bMoreMsg = TRUE;
			}

			SmsPluginEventHandler::instance()->SetSentInfo(&sentInfo);

			curStatus = MSG_NETWORK_SENDING;

			// Send SMS
			int tapiRet = TAPI_API_SUCCESS;

			tapiRet = tel_send_sms(pTapiHandle, &pkgInfo, bMoreMsg, TapiEventSentStatus, NULL);

			if (tapiRet == TAPI_API_SUCCESS)
			{
				MSG_DEBUG("########  tel_send_sms Success !!! return : [%d] #######", tapiRet);
			}
			else
			{
				SmsPluginEventHandler::instance()->handleSentStatus(MSG_NETWORK_SEND_FAIL);

				THROW(MsgException::SMS_PLG_ERROR, "########  tel_send_sms Fail !!! return : [%d] #######", tapiRet);
			}

			// Tizen Validation System
			char *msisdn = NULL;
			msisdn = MsgSettingGetString(MSG_SIM_MSISDN);

			MSG_SMS_VLD_INFO("%d, SMS Send Start, %s->%s, %s",  pReqInfo->msgInfo.msgId, \
																		(msisdn == NULL)?"ME":msisdn, \
																		pReqInfo->msgInfo.addressList[0].addressVal, \
																		(tapiRet == TAPI_API_SUCCESS)?"Success":"Fail");

			MSG_SMS_VLD_TXT("%d, [%s]", pReqInfo->msgInfo.msgId, pReqInfo->msgInfo.msgText);

			msg_network_status_t retStatus = getNetStatus();

	#ifdef MSG_SMS_REPORT
			if (err == MSG_SUCCESS && tmpInfo.msgInfo.msgPort.valid == false)
			{
				if(pReqInfo->sendOptInfo.bDeliverReq == true)
				{
					MSG_DEBUG("Update Delivery Report Status : [%d] Msg ID : [%d]", err, tmpInfo.msgInfo.msgId);

					// Adding delivery report status info.
					MsgStoAddDeliveryReportStatus( tmpInfo.msgInfo.msgId, (unsigned char)tmpInfo.msgInfo.referenceId);
				}
			}
	#endif

			MSG_SMS_VLD_INFO("%d, SMS Send End, %s->%s, %s",  pReqInfo->msgInfo.msgId, \
																	(msisdn == NULL)?"ME":msisdn, \
																	pReqInfo->msgInfo.addressList[0].addressVal, \
																	(retStatus == MSG_NETWORK_SEND_SUCCESS)?"Success":"Fail");


			if (retStatus == MSG_NETWORK_SEND_SUCCESS)
			{
				if (bMoreMsg == false) {
					MsgInsertTicker("SMS is sent", SMS_MESSAGE_SENT);
				}
				MSG_DEBUG("########  Msg Sent was Successful !!! return : [%d] #######", retStatus);
			}
			else
			{
				MsgInsertTicker("Sending SMS is failed", NULL);
				MsgInsertTicker("Sending SMS is failed", SMS_MESSAGE_SENDING_FAIL);

				SmsPluginEventHandler::instance()->handleSentStatus(MSG_NETWORK_SEND_FAIL);

				THROW(MsgException::SMS_PLG_ERROR, "########  Msg Sent was Failed !!! return : [%d] #######", retStatus);
			}

			if (tpdu.data.submit.userData.headerCnt > 0) tpdu.data.submit.userData.headerCnt--;
		}
	}

	MSG_END();
}


void SmsPluginTransport::sendDeliverReport(msg_error_t err)
{
	MSG_BEGIN();

	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER_REP;

	TelSmsResponse_t response;

	int tapiRet = TAPI_API_SUCCESS;

	if (err == MSG_SUCCESS)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_POSITIVE;
		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else if (err == MSG_ERR_SIM_STORAGE_FULL)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_NEGATIVE;
		tpdu.data.deliverRep.failCause = SMS_FC_MSG_CAPA_EXCEEDED;
		response = TAPI_NETTEXT_SIM_FULL;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else if (err == MSG_ERR_MESSAGE_COUNT_FULL)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_NEGATIVE;
		tpdu.data.deliverRep.failCause = SMS_FC_MSG_CAPA_EXCEEDED;
		response = TAPI_NETTEXT_ME_FULL;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_NEGATIVE;
		tpdu.data.deliverRep.failCause = SMS_FC_UNSPEC_ERROR;
		//response = TAPI_NETTEXT_PROTOCOL_ERROR;
		// For gcf test [34.2.5.3 class2 message]
		response = TAPI_NETTEXT_SIM_FULL;

	}

	MSG_DEBUG("err : [%d], response : [%02x]", err, response);

	tpdu.data.deliverRep.bHeaderInd = false;
	tpdu.data.deliverRep.paramInd = 0x00;

	// Encode SMS-DELIVER-REPORT TPDU
	int bufLen = 0;

	char buf[MAX_TPDU_DATA_LEN];
	memset(buf, 0x00, sizeof(buf));

	bufLen = SmsPluginTpduCodec::encodeTpdu(&tpdu, buf);

#ifdef MSG_FOR_DEBUG
	/////// print DeliverReport tpdu
	printf("\n\n######## DeliverReport tpdu #########\n");
	for(int i=0; i < bufLen; i++)
	{
		printf("[%02x] ", buf[i]);
	}
	printf("\n#################################\n\n");
	//////
#endif

	// Make Telephony Structure
	TelSmsDatapackageInfo_t pkgInfo;

	// Set TPDU data
	memset((void*)pkgInfo.szData, 0x00, sizeof(pkgInfo.szData));
	memcpy((void*)pkgInfo.szData, buf, bufLen);

	pkgInfo.szData[bufLen] = 0;
	pkgInfo.MsgLength = bufLen;

	// Set SMSC Address
	SMS_ADDRESS_S smsc;

	// Set SMSC Options
	setSmscOptions(&smsc);

	// Encode SMSC Address
	unsigned char smscAddr[MAX_SMSC_LEN];
	memset(smscAddr, 0x00, sizeof(smscAddr));

	int smscLen = SmsPluginParamCodec::encodeSMSC(&smsc, smscAddr);

	if (smscLen <= 0) return;

	// Set SMSC data
	memset(pkgInfo.Sca, 0x00, sizeof(pkgInfo.Sca));
	memcpy((void*)pkgInfo.Sca, smscAddr, smscLen);
	pkgInfo.Sca[smscLen] = '\0';

	// Send Deliver Report
	tapiRet = tel_send_sms_deliver_report(pTapiHandle, &pkgInfo, response, TapiEventDeliveryReportCNF, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_send_sms_deliver_report() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("########  tel_send_sms_deliver_report() Fail !!! return : [%d] #######", tapiRet);
	}

	MSG_END();
}

void SmsPluginTransport::sendClass0DeliverReport(msg_error_t err)
{
	MSG_BEGIN();

	SMS_TPDU_S tpdu;

	tpdu.tpduType = SMS_TPDU_DELIVER_REP;

	TelSmsResponse_t response;

	int tapiRet = TAPI_API_SUCCESS;

	if (err == MSG_SUCCESS)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_POSITIVE;
		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_AVAILABLE, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else if (err == MSG_ERR_SIM_STORAGE_FULL)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_POSITIVE;
		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else if (err == MSG_ERR_MESSAGE_COUNT_FULL)
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_POSITIVE;
		response = TAPI_NETTEXT_SENDSMS_SUCCESS;

		tapiRet = tel_set_sms_memory_status(pTapiHandle, TAPI_NETTEXT_PDA_MEMORY_STATUS_FULL, TapiEventMemoryStatus, NULL);

		if (tapiRet == TAPI_API_SUCCESS)
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Success !!! #######");
		}
		else
		{
			MSG_DEBUG("########  tel_set_sms_memory_status() Failed !!! return : [%d] #######", tapiRet);
		}
	}
	else
	{
		tpdu.data.deliverRep.reportType = SMS_REPORT_NEGATIVE;
		tpdu.data.deliverRep.failCause = SMS_FC_UNSPEC_ERROR;
		//response = TAPI_NETTEXT_PROTOCOL_ERROR;
		// For gcf test [34.2.5.3 class2 message]
		response = TAPI_NETTEXT_SIM_FULL;

	}

	MSG_DEBUG("err : [%d], response : [%02x]", err, response);

	tpdu.data.deliverRep.bHeaderInd = false;
	tpdu.data.deliverRep.paramInd = 0x00;

	// Encode SMS-DELIVER-REPORT TPDU
	int bufLen = 0;

	char buf[MAX_TPDU_DATA_LEN];
	memset(buf, 0x00, sizeof(buf));

	bufLen = SmsPluginTpduCodec::encodeTpdu(&tpdu, buf);

	// Make Telephony Structure
	TelSmsDatapackageInfo_t pkgInfo;

	// Set TPDU data
	memset((void*)pkgInfo.szData, 0x00, sizeof(pkgInfo.szData));
	memcpy((void*)pkgInfo.szData, buf, bufLen);

	pkgInfo.szData[bufLen] = 0;
	pkgInfo.MsgLength = bufLen;

	// Set SMSC Address
	SMS_ADDRESS_S smsc;

	// Set SMSC Options
	setSmscOptions(&smsc);

	// Encode SMSC Address
	unsigned char smscAddr[MAX_SMSC_LEN];
	memset(smscAddr, 0x00, sizeof(smscAddr));

	int smscLen = SmsPluginParamCodec::encodeSMSC(&smsc, smscAddr);

	if (smscLen <= 0) return;

	// Set SMSC data
	memset(pkgInfo.Sca, 0x00, sizeof(pkgInfo.Sca));
	memcpy((void*)pkgInfo.Sca, smscAddr, smscLen);
	pkgInfo.Sca[smscLen] = '\0';

	// Send Deliver Report
	tapiRet = tel_send_sms_deliver_report(pTapiHandle, &pkgInfo, response, TapiEventDeliveryReportCNF, NULL);

	if (tapiRet == TAPI_API_SUCCESS)
	{
		MSG_DEBUG("########  tel_send_sms_deliver_report() Success !!! #######");
	}
	else
	{
		MSG_DEBUG("########  tel_send_sms_deliver_report() Fail !!! return : [%d] #######", tapiRet);
	}

	MSG_END();
}



void SmsPluginTransport::setSmsSendOptions(SMS_SUBMIT_S *pSubmit)
{
	// Set SMS Send Options
	pSubmit->bRejectDup = false;
	pSubmit->bHeaderInd = false;

	MsgSettingGetBool(SMS_SEND_DELIVERY_REPORT, &pSubmit->bStatusReport);
	MsgSettingGetBool(SMS_SEND_REPLY_PATH, &pSubmit->bReplyPath);

	pSubmit->msgRef = msgRef++;

	pSubmit->dcs.bCompressed = false;
	pSubmit->dcs.msgClass = SMS_MSG_CLASS_NONE;
	pSubmit->dcs.codingGroup = SMS_GROUP_GENERAL;

	pSubmit->dcs.codingScheme = (SMS_CODING_SCHEME_T)MsgSettingGetInt(SMS_SEND_DCS);

	MSG_DEBUG("DCS : %d", pSubmit->dcs.codingScheme);

	int selectIdx = MsgSettingGetInt(SMSC_SELECTED);

	char keyName[128];

	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", SMSC_PID, selectIdx);
	MSG_SMS_PID_T pid = (MSG_SMS_PID_T)MsgSettingGetInt(keyName);

	pSubmit->pid = convertPid(pid);
	MSG_DEBUG("PID : %d", pSubmit->pid);

	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", SMSC_VAL_PERIOD, selectIdx);
	int valPeriod = MsgSettingGetInt(keyName);

	MSG_DEBUG("valPeriod : %d", valPeriod);

	if (valPeriod == 0)
	{
		pSubmit->vpf = SMS_VPF_NOT_PRESENT;
	}
	else
	{
		pSubmit->vpf = SMS_VPF_RELATIVE;
		pSubmit->validityPeriod.format = SMS_TIME_RELATIVE;
		pSubmit->validityPeriod.time.relative.time = valPeriod;
	}
}


void SmsPluginTransport::setSmscOptions(SMS_ADDRESS_S *pSmsc)
{
	// Set SMSC Options
	int selectIdx = MsgSettingGetInt(SMSC_SELECTED);

	char keyName[128];

	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", SMSC_ADDRESS, selectIdx);

	char* tmpValue = NULL;

	tmpValue = MsgSettingGetString(keyName);

	if (tmpValue != NULL)
	{
		memset(pSmsc->address, 0x00, sizeof(pSmsc->address));
		strncpy(pSmsc->address, tmpValue, MAX_ADDRESS_LEN);

		MSG_DEBUG("address : %s", pSmsc->address);
	}
	else
	{
		strncpy(pSmsc->address, "+8210911111", MAX_ADDRESS_LEN);
	}

	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", SMSC_TON, selectIdx);
	pSmsc->ton = (SMS_TON_T)MsgSettingGetInt(keyName);

	MSG_DEBUG("ton : %d", pSmsc->ton);

	memset(keyName, 0x00, sizeof(keyName));
	sprintf(keyName, "%s/%d", SMSC_NPI, selectIdx);
	pSmsc->npi = (SMS_NPI_T)MsgSettingGetInt(keyName);

	MSG_DEBUG("npi : %d", pSmsc->npi);

	if (tmpValue != NULL)
	{
		free(tmpValue);
		tmpValue = NULL;
	}
}


void SmsPluginTransport::msgInfoToSubmitData(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SUBMIT_DATA_S *pData, SMS_CODING_SCHEME_T *pCharType, int addrIndex)
{
	// Destination Address
	pData->destAddress.ton = SMS_TON_UNKNOWN;
	pData->destAddress.npi = SMS_NPI_ISDN;

	memset(pData->destAddress.address, 0x00, MAX_ADDRESS_LEN+1);
	memcpy(pData->destAddress.address, pMsgInfo->addressList[addrIndex].addressVal, MAX_ADDRESS_LEN);

	MSG_DEBUG("ton [%d]", pData->destAddress.ton);
	MSG_DEBUG("npi [%d]", pData->destAddress.npi);
	MSG_DEBUG("address [%s]", pData->destAddress.address);

	int decodeLen = 0, bufSize = (MAX_GSM_7BIT_DATA_LEN*MAX_SEGMENT_NUM) + 1; 	// SMS_CHARSET_7BIT

	unsigned char decodeData[bufSize];
	memset(decodeData, 0x00, sizeof(decodeData));

	msg_encode_type_t encodeType = MSG_ENCODE_GSM7BIT;

	MSG_LANGUAGE_ID_T langId = MSG_LANG_ID_RESERVED;

	bool bAbnormal = false;

	// User Data
	if (pMsgInfo->bTextSms == true)
	{
		if (*pCharType == SMS_CHARSET_7BIT)
		{
			decodeLen = textCvt.convertUTF8ToGSM7bit(decodeData, bufSize, (unsigned char*)pMsgInfo->msgText, (int)pMsgInfo->dataSize, &langId, &bAbnormal);
		}
		else if (*pCharType == SMS_CHARSET_8BIT)
		{
			memcpy(decodeData, pMsgInfo->msgText, pMsgInfo->dataSize);
			decodeLen = pMsgInfo->dataSize;
		}
		else if (*pCharType == SMS_CHARSET_UCS2)
		{
			decodeLen = textCvt.convertUTF8ToUCS2(decodeData, bufSize, (unsigned char*)pMsgInfo->msgText, (int)pMsgInfo->dataSize);
		}
		else if (*pCharType == SMS_CHARSET_AUTO)
		{
			decodeLen = textCvt.convertUTF8ToAuto(decodeData, bufSize, (unsigned char*)pMsgInfo->msgText, (int)pMsgInfo->dataSize, &encodeType);
			*pCharType = encodeType;
		}
	}
	else
	{
		int fileSize = 0;

		char* pFileData = NULL;
		AutoPtr<char> FileBuf(&pFileData);

		// Read Message Data from File
		if (MsgOpenAndReadFile(pMsgInfo->msgData, &pFileData, &fileSize) == false)
			THROW(MsgException::FILE_ERROR, "MsgOpenAndReadFile error");

		MSG_DEBUG("file size : [%d] file data : [%s]", fileSize, pFileData);

		if (*pCharType == SMS_CHARSET_7BIT)
		{
			decodeLen = textCvt.convertUTF8ToGSM7bit(decodeData, bufSize, (unsigned char*)pFileData, fileSize, &langId, &bAbnormal);
		}
		else if (*pCharType == SMS_CHARSET_8BIT)
		{
			memcpy(decodeData, pFileData, fileSize);
			decodeLen = fileSize;
		}
		else if (*pCharType == SMS_CHARSET_UCS2)
		{
			decodeLen = textCvt.convertUTF8ToUCS2(decodeData, bufSize, (unsigned char*)pFileData, fileSize);
		}
		else if (*pCharType == SMS_CHARSET_AUTO)
		{
			decodeLen = textCvt.convertUTF8ToAuto(decodeData, bufSize, (unsigned char*)pFileData, fileSize, &encodeType);
			*pCharType = encodeType;
		}

		// Delete File
		if (pMsgInfo->nAddressCnt == (addrIndex + 1))
			MsgDeleteFile(pMsgInfo->msgData);
	}

MSG_DEBUG("decode length : [%d]", decodeLen);
MSG_DEBUG("character type : [%d]", *pCharType);
MSG_DEBUG("Language Identifier : [%d]", langId);
MSG_DEBUG("reply address : [%s]", pMsgInfo->replyAddress);

	int addrLen = 0;

	char* encodedAddr = NULL;
	AutoPtr<char> addressBuf(&encodedAddr);

	if (strlen(pMsgInfo->replyAddress) > 0)
	{
		SMS_ADDRESS_S replyAddr = {};

		replyAddr.ton = SMS_TON_NATIONAL;
		replyAddr.npi = SMS_NPI_ISDN;

		memset(replyAddr.address, 0x00, MAX_ADDRESS_LEN+1);
		memcpy(replyAddr.address, pMsgInfo->replyAddress, MAX_ADDRESS_LEN);

		addrLen = SmsPluginParamCodec::encodeAddress(&replyAddr, &encodedAddr);

		MSG_DEBUG("reply addr length : [%d]", addrLen);
	}

	int segSize = 0, index = 0;

	segSize = getSegmentSize(*pCharType, decodeLen, pMsgInfo->msgPort.valid, langId, addrLen);

	pData->segCount = ceil((double)decodeLen/(double)segSize);

MSG_DEBUG("segment size : [%d], pData->segCount : [%d]", segSize, pData->segCount);

	if (pData->segCount > MAX_SEGMENT_NUM)
	 	THROW(MsgException::SMS_PLG_ERROR, "Segment Count is over maximum : %d", pData->segCount);

	int headerCnt = 0;

	for (unsigned int i = 0; i < pData->segCount; i++)
	{
		headerCnt = 0;

		if ((i + 1) == pData->segCount)
			pData->userData[i].length = decodeLen - (i*segSize);
		else
			pData->userData[i].length = segSize;

		memset(pData->userData[i].data, 0x00, MAX_USER_DATA_LEN+1);
		memcpy(pData->userData[i].data, &(decodeData[index]), pData->userData[i].length);
		pData->userData[i].data[pData->userData[i].length] = 0;

MSG_DEBUG("user data len [%d]", pData->userData[i].length);
MSG_DEBUG("user data [%s]", pData->userData[i].data);

		index += segSize;

		// Set User Data Header for Concatenated Message
		if (pData->segCount > 1)
		{
			pData->userData[i].header[headerCnt].udhType = SMS_UDH_CONCAT_8BIT;
			pData->userData[i].header[headerCnt].udh.concat8bit.msgRef = msgRef8bit;
			pData->userData[i].header[headerCnt].udh.concat8bit.totalSeg = pData->segCount;
			pData->userData[i].header[headerCnt].udh.concat8bit.seqNum = i + 1;

			headerCnt++;
		}

		// Set User Data Header Port Information
		if (pMsgInfo->msgPort.valid == true)
		{
			pData->userData[i].header[headerCnt].udhType = SMS_UDH_APP_PORT_16BIT;
			pData->userData[i].header[headerCnt].udh.appPort16bit.destPort = pMsgInfo->msgPort.dstPort;
			pData->userData[i].header[headerCnt].udh.appPort16bit.originPort = pMsgInfo->msgPort.srcPort;

			headerCnt++;
		}

		// Set User Data Header for Alternate Reply Address
		if (strlen(pMsgInfo->replyAddress) > 0)
		{
			pData->userData[i].header[headerCnt].udhType = SMS_UDH_ALTERNATE_REPLY_ADDRESS;

			pData->userData[i].header[headerCnt].udh.alternateAddress.ton = SMS_TON_NATIONAL;
			pData->userData[i].header[headerCnt].udh.alternateAddress.npi = SMS_NPI_ISDN;

			memset(pData->userData[i].header[headerCnt].udh.alternateAddress.address, 0x00, MAX_ADDRESS_LEN+1);
			memcpy(pData->userData[i].header[headerCnt].udh.alternateAddress.address, pMsgInfo->replyAddress, MAX_ADDRESS_LEN);

			headerCnt++;
		}

		// Set User Data Header for National Language Single Shift
		if (*pCharType == SMS_CHARSET_7BIT && langId != MSG_LANG_ID_RESERVED)
		{
			pData->userData[i].header[headerCnt].udhType = SMS_UDH_SINGLE_SHIFT;
			pData->userData[i].header[headerCnt].udh.singleShift.langId = langId;

			headerCnt++;
		}

		pData->userData[i].headerCnt = headerCnt;
	}

	msgRef8bit++;
}


int SmsPluginTransport::getSegmentSize(SMS_CODING_SCHEME_T CodingScheme, int DataLen, bool bPortNum, MSG_LANGUAGE_ID_T LangId, int ReplyAddrLen)
{
	int headerLen = 1, concat = 5, port = 6, lang = 3, reply = 2;
	int headerSize = 0, segSize = 0, maxSize = 0;

	if (CodingScheme == SMS_CHARSET_7BIT)
	{
		MSG_DEBUG("SMS_CHARSET_7BIT");
		maxSize = MAX_GSM_7BIT_DATA_LEN;
	}
	else if (CodingScheme == SMS_CHARSET_8BIT || CodingScheme == SMS_CHARSET_UCS2)
	{
		MSG_DEBUG("SMS_CHARSET_8BIT or SMS_CHARSET_UCS2 [%d]", CodingScheme);
		maxSize = MAX_UCS2_DATA_LEN;
	}

	if (bPortNum == true)
	{
		MSG_DEBUG("Port Number Exists");
		headerSize += port;
	}

	if (LangId != MSG_LANG_ID_RESERVED)
	{
		MSG_DEBUG("National Language Exists");
		headerSize += lang;
	}

	if (ReplyAddrLen > 0)
	{
		MSG_DEBUG("Reply Address Exists");
		headerSize += reply;
		headerSize += ReplyAddrLen;
	}

	if (CodingScheme == SMS_CHARSET_7BIT)
	{
		if (((DataLen+headerSize)/maxSize) >= 1)
			segSize = ((140*8) - ((headerLen + concat + headerSize)*8)) / 7;
		else
			segSize = DataLen;
	}
	else if (CodingScheme == SMS_CHARSET_8BIT || CodingScheme == SMS_CHARSET_UCS2)
	{
		if (((DataLen+headerSize)/maxSize) >= 1)
			segSize = 140 - (headerLen + concat + headerSize);
		else
			segSize = DataLen;
	}

	return segSize;
}


void SmsPluginTransport::setConcatHeader(SMS_UDH_S *pSrcHeader, SMS_UDH_S *pDstHeader)
{
	pDstHeader->udhType = pSrcHeader->udhType;

	switch (pDstHeader->udhType)
	{
		case SMS_UDH_CONCAT_8BIT :
		{
			pDstHeader->udh.concat8bit.msgRef = pSrcHeader->udh.concat8bit.msgRef;
			pDstHeader->udh.concat8bit.totalSeg = pSrcHeader->udh.concat8bit.totalSeg;
			pDstHeader->udh.concat8bit.seqNum = pSrcHeader->udh.concat8bit.seqNum;
		}
		break;

		case SMS_UDH_CONCAT_16BIT :
		{
			pDstHeader->udh.concat16bit.msgRef = pSrcHeader->udh.concat16bit.msgRef;
			pDstHeader->udh.concat16bit.totalSeg = pSrcHeader->udh.concat16bit.totalSeg;
			pDstHeader->udh.concat16bit.seqNum = pSrcHeader->udh.concat16bit.seqNum;
		}
		break;
	}
}


void SmsPluginTransport::setNetStatus(msg_network_status_t netStatus)
{
	mx.lock();
	curStatus = netStatus;
	cv.signal();
	mx.unlock();
}


msg_network_status_t SmsPluginTransport::getNetStatus()
{
	mx.lock();

	int ret = 0;

	if (curStatus == MSG_NETWORK_SENDING)
		ret = cv.timedwait(mx.pMutex(), 125);

	mx.unlock();

	if (ret == ETIMEDOUT)
	{
		MSG_DEBUG("WARNING: SENT STATUS TIME-OUT");
		curStatus = MSG_NETWORK_SEND_TIMEOUT;
	}

	return curStatus;
}


unsigned char SmsPluginTransport::getMsgRef()
{
	return msgRef++;
}


SMS_PID_T SmsPluginTransport::convertPid(MSG_SMS_PID_T pid)
{
	SMS_PID_T retPid;

	switch (pid)
	{
		case MSG_PID_TEXT :
			retPid = SMS_PID_NORMAL;
		break;
		case MSG_PID_VOICE :
			retPid = SMS_PID_VOICE;
		break;
		case MSG_PID_FAX :
			retPid = SMS_PID_TELEX;
		break;
		case MSG_PID_X400 :
			retPid = SMS_PID_x400;
		break;
		case MSG_PID_ERMES :
			retPid = SMS_PID_ERMES;
		break;
		case MSG_PID_EMAIL :
			retPid = SMS_PID_EMAIL;
		break;
		default :
			retPid = SMS_PID_NORMAL;
		break;
	}

	return retPid;
}
