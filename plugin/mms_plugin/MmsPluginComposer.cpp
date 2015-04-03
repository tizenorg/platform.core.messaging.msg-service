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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "MsgCppTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"
#include "MsgMmsMessage.h"
#include "MmsPluginTypes.h"
#include "MmsPluginDebug.h"
#include "MmsPluginMessage.h"
#include "MmsPluginMIME.h"
#include "MmsPluginStorage.h"
#include "MmsPluginUtil.h"
#include "MmsPluginTcs.h"
#include "MsgSmil.h"


#include "MmsPluginComposer.h"
#include "MsgSerialize.h"

bool composeSendReqHeader(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_DATA_S *pMsgData);

MmsPluginComposer *MmsPluginComposer::pInstance = NULL;

MmsPluginComposer *MmsPluginComposer::instance()
{
	if (!MmsPluginComposer::pInstance)
		MmsPluginComposer::pInstance = new MmsPluginComposer();

	return MmsPluginComposer::pInstance;
}

MmsPluginComposer::MmsPluginComposer(){}
MmsPluginComposer::~MmsPluginComposer(){}

void MmsPluginComposer::composeSendReq(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_DATA_S *pMmsData)
{

	if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS) {

		if (pMmsData->header == NULL) {//send req from user
			pMmsData->header = MsgMmsCreateHeader();
		}

		composeSendReqHeader(pMsgInfo, pSendOptInfo, pMmsData);
		//TODO:: apply MmsReplaceNonAsciiUtf8 to all multipart FileName;

		int len = g_list_length(pMmsData->multipartlist);

		for (int i = 0; i < len; i++) {

			MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMmsData->multipartlist, i);

			if (multipart) {
				if (multipart->type == MIME_UNKNOWN && strlen(multipart->szContentType) == 0) {
					const char *content_type = NULL;
					MmsGetMimeTypeFromFileName(MIME_MAINTYPE_UNKNOWN, multipart->szFileName, &multipart->type, &content_type);
					snprintf(multipart->szContentType, sizeof(multipart->szContentType), "%s", content_type);
				}
			}
		} //end for
	}
}

MMSList *getAddressList(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType)
{
	MSG_BEGIN();

	MMSList * addressList = NULL;

	int nAddressCnt = 0;

	nAddressCnt = pMsgInfo->nAddressCnt;

	// Calculate allocated buffer size
	for (int i = 0; i < nAddressCnt; ++i) {

		MSG_SEC_DEBUG("recipientType: %d, address value: %s", pMsgInfo->addressList[i].recipientType, pMsgInfo->addressList[i].addressVal);

		if (pMsgInfo->addressList[i].recipientType == MSG_RECIPIENTS_TYPE_UNKNOWN)
			pMsgInfo->addressList[i].recipientType = MSG_RECIPIENTS_TYPE_TO;

		if (pMsgInfo->addressList[i].recipientType == recipientType) {

			MMS_ADDRESS_DATA_S * pAddressData = NULL;
			if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				pAddressData = MsgMmsCreateAddress(MSG_ADDRESS_TYPE_PLMN, pMsgInfo->addressList[i].addressVal);
			} else if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_EMAIL) {
				pAddressData = MsgMmsCreateAddress(MSG_ADDRESS_TYPE_EMAIL, pMsgInfo->addressList[i].addressVal);
			} else
				; // Need to consider IPV4, IPV6, and Alias formatted address

			if (pAddressData)
				addressList = g_list_append(addressList, pAddressData);
		}
	}

	MSG_END();
	return addressList;
}

bool composeSendReqHeader(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_DATA_S *pMsgData)
{
	MSG_BEGIN();

	bool bAskDeliveryReport = false;
	bool bAskReadReply = false;
	MmsPriority priority;
	MmsTimeStruct expiryTime;
	MmsTimeStruct deliveryTime;
	MmsMsgClass msgClass;

	struct tm *timeInfo = NULL;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	MMS_HEADER_DATA_S *pHeaderData = pMsgData->header;
	if (pSendOptInfo) {
		if (pSendOptInfo->bSetting == false) {
			unsigned int settingTime;

			priority = (MmsPriority)MsgSettingGetInt(MMS_SEND_PRIORITY);

			settingTime = (unsigned int)MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);
			if (settingTime == 0) {
				expiryTime.type = MMS_TIMETYPE_NONE;
				expiryTime.time = 0;
			} else {
				expiryTime.type = MMS_TIMETYPE_RELATIVE;
				expiryTime.time = settingTime;
			}

			settingTime = (unsigned int)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);
			if (settingTime == 0) {
				deliveryTime.type = MMS_TIMETYPE_NONE;
				deliveryTime.time = 0;
			} else {
				deliveryTime.type = MMS_TIMETYPE_RELATIVE;
				deliveryTime.time = (unsigned int)settingTime;
			}

			MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &bAskDeliveryReport);
			MsgSettingGetBool(MMS_SEND_READ_REPLY, &bAskReadReply);
		} else {
			priority = (MmsPriority)pSendOptInfo->option.mmsSendOptInfo.priority;

			expiryTime.type = pSendOptInfo->option.mmsSendOptInfo.expiryTime.type;
			expiryTime.time = pSendOptInfo->option.mmsSendOptInfo.expiryTime.time;

			deliveryTime.type = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.type;
			deliveryTime.time = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time;

			bAskDeliveryReport = pSendOptInfo->bDeliverReq;
			bAskReadReply = pSendOptInfo->option.mmsSendOptInfo.bReadReq;
		}

		msgClass = (MmsMsgClass)MsgSettingGetInt(MMS_SEND_MSG_CLASS);

		//set Header
		time(&RawTime);
		timeInfo = localtime(&RawTime);
		nTimeInSecs = mktime(timeInfo);
		pHeaderData->date = nTimeInSecs;

		pHeaderData->bDeliveryReport = bAskDeliveryReport;
		pHeaderData->delivery.type = deliveryTime.type;
		pHeaderData->delivery.time = deliveryTime.time;

		pHeaderData->expiry.type = expiryTime.type;
		pHeaderData->expiry.time = expiryTime.time;

		pHeaderData->messageClass = msgClass;
		pHeaderData->messageType = MMS_MSGTYPE_SEND_REQ;
		pHeaderData->mmsVersion = MMS_VERSION;
		pHeaderData->mmsPriority = priority;
		pHeaderData->bReadReport = bAskReadReply;
		pHeaderData->bHideAddress = false;
	}

	if (pMsgData->smil) {
		pHeaderData->contentType = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;
	} else {
		pHeaderData->contentType = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
	}

	char *content_type = MimeGetMimeStringFromMimeInt(pHeaderData->contentType);
	if (content_type) {
		snprintf(pHeaderData->szContentType, sizeof(pHeaderData->szContentType), "%s", content_type);
	}

	snprintf(pHeaderData->szSubject, sizeof(pHeaderData->szSubject), "%s", pMsgInfo->subject);

	//setting adddress
	pHeaderData->to = getAddressList(pMsgInfo, MSG_RECIPIENTS_TYPE_TO);
	pHeaderData->cc = getAddressList(pMsgInfo, MSG_RECIPIENTS_TYPE_CC);
	pHeaderData->bcc = getAddressList(pMsgInfo, MSG_RECIPIENTS_TYPE_BCC);

	//snprintf(pHeaderData->szFrom, sizeof(pHeaderData->szFrom), "%s", pMmsMsg->mmsAttrib.szFrom);

	return true;
}
