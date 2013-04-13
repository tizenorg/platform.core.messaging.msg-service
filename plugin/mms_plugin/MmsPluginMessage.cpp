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
#include "MmsPluginAvCodec.h"
#include "MmsPluginStorage.h"
#include "MmsPluginSMILValidate.h"
#include "MmsPluginSmil.h"
#include "MmsPluginUtil.h"

#define PRINT_KEY_VAL_STR(key, val)\
if (val) {\
MSG_DEBUG("%-20s: %s", key, val);\
}\

#define PRINT_KEY_VAL_INT(key, val)\
if (val) {\
MSG_DEBUG("%-20s: %d", key, val);\
}\

static MsgMultipart *MmsGetNthMultipart(MmsMsg *pMsg, int index);
static bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S *pMsgInfo);
static char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType);
static bool MmsGetSmilRawData(MMS_MESSAGE_DATA_S *pMsgBody, char *pRawData, int *nSize);
static bool MmsInsertPresentation(MmsMsg *pMsg, MimeType mimeType, const char *content_id, char *pData, int size);
static bool MmsInsertPartFromFile(MmsMsg *pMsg, char *szTitleName, char *szOrgFilePath, char *szContentID);
static bool MmsInsertPartFromMultipart(MmsMsg *pMsg, MMS_MULTIPART_DATA_S *pNewMultipart);
static bool MmsGetTypeByFileName(int *type, char *szFileName);
static bool MmsInsertPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart);

bool MmsSetMsgAddressList(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S * pMsgInfo)
{
	MSG_DEBUG("MmsSetMsgAddressList");
	pAttrib->szTo = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_TO);
	MSG_DEBUG("To address: %s", pAttrib->szTo);
	pAttrib->szCc = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_CC);
	MSG_DEBUG("Cc address: %s", pAttrib->szCc);
	pAttrib->szBcc = MmsComposeAddress(pMsgInfo, MSG_RECIPIENTS_TYPE_BCC);
	MSG_DEBUG("Bcc address: %s", pAttrib->szBcc);

	return true;
}

void MmsSetMsgMultiStatus(MmsAttrib *pAttrib, const MSG_MESSAGE_INFO_S *pMsgInfo)
{
	int	nAddressCnt = 0;

	nAddressCnt = pMsgInfo->nAddressCnt;

	for (int i = 0; i < nAddressCnt; ++i) {
		pAttrib->pMultiStatus = (MmsMsgMultiStatus *)malloc(sizeof(MmsMsgMultiStatus));

		memset(pAttrib->pMultiStatus->szTo, 0, MAX_ADDRESS_VAL_LEN + 1);
		strncpy(pAttrib->pMultiStatus->szTo, pMsgInfo->addressList[i].addressVal, MAX_ADDRESS_VAL_LEN);

		MSG_DEBUG("### pMultistatus->szTo = %s ####", pAttrib->pMultiStatus->szTo);
		pAttrib->pMultiStatus->bDeliveryReportIsRead = false;
		pAttrib->pMultiStatus->bDeliveyrReportIsLast = false;
		pAttrib->pMultiStatus->msgStatus = MMS_MSGSTATUS_NONE;
		pAttrib->pMultiStatus->handledTime = 0;
		pAttrib->pMultiStatus->bReadReplyIsRead = false;
		pAttrib->pMultiStatus->bReadReplyIsLast = false;
		pAttrib->pMultiStatus->readStatus = MMS_READSTATUS_NONE;
		pAttrib->pMultiStatus->readTime = 0;

		pAttrib->pMultiStatus = pAttrib->pMultiStatus->pNext;
	}
}

char *MmsComposeAddress(const MSG_MESSAGE_INFO_S *pMsgInfo, int recipientType)
{
	MSG_DEBUG("MmsComposeAddress");
	int	addrLen = 0;
	int	nAddressCnt = 0;
	int nRecpCnt = 0;
	char pString[MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3] = {0, };
	char *szCompose;

	nAddressCnt = pMsgInfo->nAddressCnt;

	// Calculate allocated buffer size
	for (int i = 0; i < nAddressCnt; ++i) {
		MSG_DEBUG("recipientType: %d, address value: %s", pMsgInfo->addressList[i].recipientType, pMsgInfo->addressList[i].addressVal);
		if (pMsgInfo->addressList[i].recipientType == recipientType) {
			if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				addrLen += strlen(MsgGetString(MSG_ADDR_TYPE, MSG_ADDR_TYPE_PHONE));
				addrLen += strlen(pMsgInfo->addressList[i].addressVal);
			} else if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_EMAIL) {
				addrLen += strlen(pMsgInfo->addressList[i].addressVal);
			} else
				; // Need to consider IPV4, IPV6, and Alias formatted address

			nRecpCnt++;
		}
	}

	if (nRecpCnt > 1)
		addrLen = addrLen + nRecpCnt - 1;
	szCompose = (char *)calloc(addrLen + 1, 1);

	// Address String copy
	for (int i = 0; i < nAddressCnt; ++i) {
		if (pMsgInfo->addressList[i].recipientType == recipientType) {
			if (strlen(szCompose) > 0)
				strcat(szCompose, MSG_STR_ADDR_DELIMETER);

			memset(pString, 0x00, (MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3) * sizeof(char));
			if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_PLMN) {
				snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s%s", pMsgInfo->addressList[i].addressVal, MsgGetString(MSG_ADDR_TYPE, MSG_ADDR_TYPE_PHONE));
				MSG_DEBUG("%s", pString);
			} else if (pMsgInfo->addressList[i].addressType == MSG_ADDRESS_TYPE_EMAIL) {
				snprintf(pString, MSG_LOCALE_NAME_LEN + MSG_ADDR_LEN + 3, "%s", pMsgInfo->addressList[i].addressVal);
			} else
				; // Need to consider IPV4, IPV6, and Alias formatted address

			strcat(szCompose, pString);
		}
	}

	return szCompose;
}


bool MmsGetMsgBodyfromMsgInfo(const MSG_MESSAGE_INFO_S *pMsgInfo, MMS_MESSAGE_DATA_S *pMsgBody, char *pFileData)
{
	MSG_DEBUG("MmsGetMsgBodyfromMsgInfo");
	memset(pMsgBody, 0, sizeof(MMS_MESSAGE_DATA_S));

	if (pMsgInfo->bTextSms == false) {	 //if  the message body was stored in file.
		_MsgMmsDeserializeMessageData(pMsgBody, pFileData);
	}

	return true;
}

bool MmsGetSmilRawData(MMS_MESSAGE_DATA_S *pMsgBody, char **pRawdata)
{
	MSG_BEGIN();

	if (MsgReadSmilFile(pMsgBody->szSmilFilePath, pRawdata) < 0)
		return false;

	MsgDeleteSmilFile(pMsgBody->szSmilFilePath);

	MSG_END();

	return true;
}

bool MmsInsertPresentation(MmsMsg *pMsg, MimeType mimeType, const char *content_id, char *pData, int size)
{
	MSG_BEGIN();

	if (pMsg == NULL) {
		MSG_DEBUG("pMsg is NULL");
		return false;
	}

	if (pMsg->msgBody.pPresentationBody != NULL)
		goto __CATCH;

	memset(&pMsg->msgBody.presentationType, 0, sizeof(MsgType));
	pMsg->msgBody.pPresentationBody = (MsgBody *)malloc(sizeof(MsgBody));
	if (pMsg->msgBody.pPresentationBody == NULL)
		goto __CATCH;

	MmsInitMsgBody(pMsg->msgBody.pPresentationBody);

	pMsg->msgBody.pPresentationBody->body.pText = (char *)malloc(size + 1);
	if (pMsg->msgBody.pPresentationBody->body.pText == NULL)
		goto __CATCH;

	pMsg->msgBody.pPresentationBody->size = size;
	pMsg->msgBody.presentationType.type = mimeType;
	pMsg->msgBody.presentationType.param.charset = MSG_CHARSET_UTF8;

	if (content_id && strlen(content_id) > 0) {
		snprintf(pMsg->msgBody.presentationType.szContentID, MSG_MSG_ID_LEN + 1, "%s", content_id);
	} else {
		snprintf(pMsg->msgBody.presentationType.szContentID, MSG_MSG_ID_LEN + 1, "<_S_>");//default
	}

	snprintf(pMsg->msgType.param.szStart, MSG_MSG_ID_LEN + 1, "%s", pMsg->msgBody.presentationType.szContentID);

	pMsg->msgType.param.type = mimeType;

	memset(pMsg->msgBody.pPresentationBody->body.pText, 0, size + 1);
	strncpy(pMsg->msgBody.pPresentationBody->body.pText, pData, size);

	MSG_END();
	return true;

__CATCH:

	if (pMsg->msgBody.pPresentationBody != NULL) {
		if (pMsg->msgBody.pPresentationBody->body.pText != NULL) {
			free(pMsg->msgBody.pPresentationBody->body.pText);
			pMsg->msgBody.pPresentationBody->body.pText = NULL;
		}

		free(pMsg->msgBody.pPresentationBody);
		pMsg->msgBody.pPresentationBody = NULL;
	}

	return false;
}

bool MmsInsertPartFromFile(MmsMsg *pMsg, char *szTitleName, char *szOrgFilePath, char *szContentID)
{
	MSG_DEBUG("MmsInsertPartFromFile");

	MsgMultipart *pMultipart = NULL;
	MsgMultipart *pLastPart = NULL;
	int nFileSize;
	MimeType mimeType = MIME_UNKNOWN;
	char *pExt = NULL;

	pExt = strrchr(szOrgFilePath, '.');

	if (pExt == NULL || pExt[0] == '\0' || strrchr(pExt, '/'))
		mimeType = MIME_UNKNOWN;
	else {
		if (strcasecmp(pExt, ".dcf") == 0)
			mimeType = MIME_APPLICATION_VND_OMA_DRM_CONTENT;
		else {
			if (MmsGetTypeByFileName((int *)&mimeType, szOrgFilePath) == false)
				goto __CATCH;
		}
	}

	MSG_DEBUG("MmsInsertPartFromFile: type = %d, name = %s, filepath = %s, cid = %s", mimeType, szTitleName, szOrgFilePath, szContentID);

	if (mimeType == MIME_UNKNOWN)
		mimeType = MIME_APPLICATION_OCTET_STREAM;

	if (MmsIsMultipart(pMsg->msgType.type) == true) {
		/* Insert as a multipart */
		if (MsgGetFileSize(szOrgFilePath, &nFileSize) == false) {
			MSG_DEBUG("MsgGetFileSize: failed");
			goto __CATCH;
		}

		pMultipart = MmsMakeMultipart(mimeType, szTitleName, szOrgFilePath, szContentID, NULL);
		if (pMultipart == NULL)
			goto __CATCH;

		if (pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED ||
			pMsg->mmsAttrib.contentType == MIME_MULTIPART_MIXED)
			pMultipart->type.disposition = MSG_DISPOSITION_ATTACHMENT;

		if (pMsg->msgBody.body.pMultipart == NULL) {
			pMsg->msgBody.body.pMultipart = pMultipart;
		} else {
			pLastPart = pMsg->msgBody.body.pMultipart;
			while (pLastPart->pNext) {
				pLastPart = pLastPart->pNext;
			}

			pLastPart->pNext = pMultipart;
		}

		pMsg->msgBody.size += pMultipart->pBody->size;
		pMsg->msgType.contentSize += pMultipart->pBody->size;
	} else {
		/* Single part - Insert as a message body */
		if (pMsg->mmsAttrib.contentType != mimeType || pMsg->msgType.type != mimeType)
			goto __CATCH;

		strncpy(pMsg->msgType.param.szName, szTitleName, MSG_LOCALE_FILENAME_LEN_MAX);

		if (MmsIsText(pMsg->msgType.type) == true) {
			pMsg->msgType.param.charset = MSG_CHARSET_UTF8;
		}

		strncpy(pMsg->msgBody.szOrgFilePath, szOrgFilePath, MSG_FILEPATH_LEN_MAX - 1);
		if (MsgGetFileSize(szOrgFilePath, &nFileSize) == false) {
			MSG_DEBUG("MsgGetFileSize: failed");
			goto __CATCH;
		}

		pMsg->msgBody.offset = 0;
		pMsg->msgBody.size = nFileSize;
		pMsg->msgType.contentSize = nFileSize;
	}

	pMsg->nPartCount++;

	return true;

__CATCH:
	return false;

}

bool MmsIsMultipart(int type)
{
	MSG_DEBUG("MmsIsMultipart");
	if (type == MIME_MULTIPART_RELATED ||
		type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED ||
		type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED ||
		type == MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC ||
		type == MIME_MULTIPART_MIXED ||
		type == MIME_MULTIPART_REPORT) {
		return true;
	} else {
		return false;
	}
}

bool MmsIsText(int type)
{
	if (type == MIME_TEXT_PLAIN ||
		type == MIME_TEXT_HTML ||
		type == MIME_TEXT_VND_WAP_WML ||
		type == MIME_TEXT_X_VCARD ||
		type == MIME_TEXT_X_VCALENDAR ||
		type == MIME_TEXT_X_VNOTE ||
		type == MIME_APPLICATION_SMIL ||
		type == MIME_TEXT_X_IMELODY) {
		MSG_DEBUG("MmsIsText true.");
		return true;
	} else {
		MSG_DEBUG("MmsIsText false.");
		return false;
	}
}

bool MmsIsVitemContent (int type, char *pszName)
{
	switch (type) {

/*
*	To make Encoding information right.
*		case MIME_TEXT_X_VCARD :
*		case MIME_TEXT_X_VCALENDAR :
*		case MIME_TEXT_X_VNOTE :	// vnt
*		{
*			MSG_DEBUG("MmsIsVitemContent true.");
*			return true;
*		}
*
*/
	case MIME_TEXT_X_VCARD:
	case MIME_TEXT_X_VCALENDAR:
	case MIME_TEXT_X_VNOTE:	// vnt
	case MIME_TEXT_PLAIN:		// vbm - It SHOULD be distinguished from a normal text file.
		{
			char *pszExt = NULL;

			if (!pszName)
				break;

			// search file extension.
			if ((pszExt = strrchr(pszName, '.')) == NULL)
				break;

			if (!strcasecmp(pszExt, ".vbm")) {
				MSG_DEBUG("MmsIsVitemContent true.");
				return true;
			}
		}
		break;

	default:
		break;
	}

	MSG_DEBUG("MmsIsVitemContent false.");
	return false;
}



MsgMultipart *MmsAllocMultipart(void)
{
	MsgMultipart *pMultipart = NULL;

	pMultipart = (MsgMultipart *)malloc(sizeof(MsgMultipart));

	if (pMultipart == NULL)
		goto __CATCH;

	pMultipart->pBody = (MsgBody *)malloc(sizeof(MsgBody));

	if (pMultipart->pBody == NULL)
		goto __CATCH;

	MmsInitMsgType(&pMultipart->type);
	MmsInitMsgBody(pMultipart->pBody);

	pMultipart->pNext = NULL;

	return pMultipart;

__CATCH:

	if (pMultipart) {
		if (pMultipart->pBody) {
			free(pMultipart->pBody);
			pMultipart->pBody = NULL;
		}

		free(pMultipart);
		pMultipart = NULL;
	}

	return NULL;
}

MsgMultipart *MmsMakeMultipart(MimeType mimeType, char *szTitleName, char *szOrgFilePath, char *szContentID, char *szContentLocation)
{
	MsgMultipart *pMultipart = NULL;

	if ((pMultipart = MmsAllocMultipart()) == NULL)
		return NULL;

	pMultipart->type.type = mimeType;

	if (szTitleName && szTitleName[0]) {
		memset(pMultipart->type.param.szName, 0, MSG_LOCALE_FILENAME_LEN_MAX + 1);
		strncpy(pMultipart->type.param.szName, szTitleName, MSG_LOCALE_FILENAME_LEN_MAX);
	}

	if (szContentID && szContentID[0]) {
		memset(pMultipart->type.szContentID, 0, MSG_MSG_ID_LEN + 1);
		snprintf(pMultipart->type.szContentID, MSG_MSG_ID_LEN + 1, "<%s>", szContentID);
	}

	if (szContentLocation && szContentLocation[0]) {
		memset(pMultipart->type.szContentLocation, 0, MSG_MSG_ID_LEN + 1);
		snprintf(pMultipart->type.szContentLocation, MSG_MSG_ID_LEN + 1, "%s", szContentLocation);
	}

	if (MmsIsText(mimeType) == true) {
		if (!MmsIsVitemContent (mimeType, pMultipart->type.param.szName)) {
			pMultipart->type.param.charset = MSG_CHARSET_UTF8;
		}
		pMultipart->type.encoding = MSG_ENCODING_8BIT;
	} else {
		pMultipart->type.encoding = MSG_ENCODING_BINARY;
	}

	if (szOrgFilePath) {
		strncpy(pMultipart->pBody->szOrgFilePath, szOrgFilePath, MSG_FILEPATH_LEN_MAX - 1);
		pMultipart->pBody->offset = 0;
		pMultipart->pBody->size = MsgGetFileSize(szOrgFilePath);
	}
	return pMultipart;
}

bool MmsGetTypeByFileName(int *type, char *szFileName)
{
	char *pExt   = NULL;
	AvCodecType AvType = AV_CODEC_NONE;

	/* AVMS unknown or text/image file format identify type from file extention  */

	pExt = strrchr(szFileName, '.');
	if (pExt == NULL || pExt[0] == '\0')
		goto __CATCH;

	pExt++;

	if (strcasecmp(pExt, "mp4") == 0 || strcasecmp(pExt, "mpeg4") == 0 ||
		strcasecmp(pExt, "3gp") == 0 || strcasecmp(pExt, "3gpp") == 0) {
		/* Audio / Video format. If file exists already, AvGetFileCodecType() can identify the format  */
		if (szFileName[0] != '/')
			goto __CATCH;

		AvType = AvGetFileCodecType(szFileName);

		switch (AvType) {
		case AV_DEC_AUDIO_MPEG4:
			*type = MIME_AUDIO_MP4;//*type = MIME_AUDIO_3GPP;
			break;

		case AV_DEC_VIDEO_MPEG4:
			*type = MIME_VIDEO_MP4;
			break;

		default:
			*type = MIME_VIDEO_3GPP;
			break;
		}
		return true;
	}

	if (strcasecmp(pExt, "amr") == 0) {
		*type = MIME_AUDIO_AMR;
		return true;
	} else if ((strcasecmp(pExt, "mid") == 0) || (strcasecmp(pExt, "midi") == 0)) {
		*type = MIME_AUDIO_MIDI;
		return true;
	} else if (strcasecmp(pExt, "imy") == 0) {
		*type = MIME_TEXT_X_IMELODY;
		return true;
	}

	*type = MimeGetMimeFromExtInt((const char *)pExt);

	return true;

__CATCH:

	*type = MIME_UNKNOWN;
	return false;

}

bool MmsComposeMessage(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData, char *pFileData)
{
	MSG_BEGIN();

	char *pRawData = NULL;
	AutoPtr<char> buf(&pRawData);

	struct tm *timeInfo = NULL;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	msg_error_t	err = MSG_SUCCESS;

	// Initialize mmsMsg structure
	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	// setting mmsMsg structure
	pMmsMsg->mailbox = pMsgInfo->folderId;
	pMmsMsg->msgID = pMsgInfo->msgId;

	memset(pMmsMsg->szTrID, 0, MMS_TR_ID_LEN + 1);
	memset(pMmsMsg->szContentLocation, 0, MMS_LOCATION_LEN + 1);
	memset(pMmsMsg->szMsgID, 0, MMS_MSG_ID_LEN + 1);
	memset(pMmsMsg->szForwardMsgID, 0, MMS_MSG_ID_LEN + 1);

	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_DRAFT;

	MSG_DEBUG("## delivery = %d ##", pSendOptInfo->bDeliverReq);
	MSG_DEBUG("## read = %d ##", pSendOptInfo->option.mmsSendOptInfo.bReadReq);
	MSG_DEBUG("## priority = %d ##", pSendOptInfo->option.mmsSendOptInfo.priority);
	MSG_DEBUG("## expiryTime = %d ##", pSendOptInfo->option.mmsSendOptInfo.expiryTime.time);

	if (pSendOptInfo->bSetting == false) {
		unsigned int expiryTime;
		MSG_MMS_DELIVERY_TIME_T deliveryTime;

		pMmsMsg->mmsAttrib.priority = (MmsPriority)MsgSettingGetInt(MMS_SEND_PRIORITY);

		MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &pMmsMsg->mmsAttrib.bAskDeliveryReport);
		MsgSettingGetBool(MMS_SEND_READ_REPLY, &pMmsMsg->mmsAttrib.bAskReadReply);
		MsgSettingGetBool(MSG_KEEP_COPY, &pMmsMsg->mmsAttrib.bLeaveCopy);

		expiryTime = (unsigned int)MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);

		if (expiryTime == 0)
			pMmsMsg->mmsAttrib.expiryTime.type = MMS_TIMETYPE_NONE;
		else {
			pMmsMsg->mmsAttrib.expiryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.expiryTime.time = expiryTime;
		}

		deliveryTime = (MSG_MMS_DELIVERY_TIME_T)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);

		if (deliveryTime == MSG_DELIVERY_TIME_CUSTOM) {
			pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = true;

			pMmsMsg->mmsAttrib.deliveryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY);
		} else {
			pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = false;

			pMmsMsg->mmsAttrib.deliveryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)deliveryTime;
		}
	} else {
		pMmsMsg->mmsAttrib.priority = (MmsPriority)pSendOptInfo->option.mmsSendOptInfo.priority;
		pMmsMsg->mmsAttrib.bAskDeliveryReport = pSendOptInfo->bDeliverReq;
		pMmsMsg->mmsAttrib.bAskReadReply = pSendOptInfo->option.mmsSendOptInfo.bReadReq;
		pMmsMsg->mmsAttrib.expiryTime.type = pSendOptInfo->option.mmsSendOptInfo.expiryTime.type;
		pMmsMsg->mmsAttrib.bLeaveCopy = pSendOptInfo->bKeepCopy;

		if (pMmsMsg->mmsAttrib.expiryTime.type != MMS_TIMETYPE_NONE)
			pMmsMsg->mmsAttrib.expiryTime.time = pSendOptInfo->option.mmsSendOptInfo.expiryTime.time;

		pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime;
		pMmsMsg->mmsAttrib.deliveryTime.type = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.type;
		pMmsMsg->mmsAttrib.deliveryTime.time = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time;
	}

	MSG_DEBUG("pSendOptInfo->bSetting = %d", pSendOptInfo->bSetting);
	MSG_DEBUG("pMmsMsg->mmsAttrib.bLeaveCopy = %d", pMmsMsg->mmsAttrib.bLeaveCopy);
	MSG_DEBUG("pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = %d", pMmsMsg->mmsAttrib.bUseDeliveryCustomTime);
	MSG_DEBUG("pMmsMsg->mmsAttrib.deliveryTime.type = %d", pMmsMsg->mmsAttrib.deliveryTime.type);
	MSG_DEBUG("pMmsMsg->mmsAttrib.deliveryTime.time = %d", pMmsMsg->mmsAttrib.deliveryTime.time);

	/* MMS-1.3-con-739 */
	pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)MsgSettingGetInt(MMS_SEND_MSG_CLASS);
	/* MMS-1.3-con-739 */
#ifdef MMS_13_CON_742_ENABLED
	/* MMS-1.3-con-742 */
	pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);
	/* MMS-1.3-con-742 */
#endif

	MSG_DEBUG("@@@ pMmsMsg->mmsAttrib.bAskDeliveryReport = %d @@@", pMmsMsg->mmsAttrib.bAskDeliveryReport);
	MSG_DEBUG("@@@ pMmsMsg->mmsAttrib.bAskReadReply = %d @@@", pMmsMsg->mmsAttrib.bAskReadReply);
	MSG_DEBUG("@@@ pMmsMsg->mmsAttrib.priority = %d @@@", pMmsMsg->mmsAttrib.priority);

	// setting date
	time(&RawTime);
	timeInfo = localtime(&RawTime);
	nTimeInSecs = mktime(timeInfo);
	pMmsMsg->mmsAttrib.date = nTimeInSecs;	// todo: need to subtract timeline value to make GMT+0 time

	//setting subject
	strcpy(pMmsMsg->mmsAttrib.szSubject, pMsgInfo->subject);

	//setting adddress
	MmsSetMsgAddressList(&pMmsMsg->mmsAttrib, pMsgInfo);
	MmsGetMsgBodyfromMsgInfo(pMsgInfo, pMsgData, pFileData);

	int pageCnt = _MsgMmsGetPageCount(pMsgData);

	if (pageCnt == 0) {	// Multipart mixed
		pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
		pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
		MmsMakePreviewInfo(pMsgInfo->msgId, pMsgData);
	} else {	// Multipart related

		int RawDataSize = 0;

		time_t RawTime = 0;
		time(&RawTime);
		snprintf(pMsgData->szSmilFilePath, MSG_FILEPATH_LEN_MAX, "%lu", RawTime);

		MsgMMSCreateSMIL(pMsgData);

		RawDataSize = MmsGetSmilRawData(pMsgData, &pRawData);
		if (RawDataSize < 0) {
			MSG_DEBUG("Smil file size is less than 0");
			return false;
		}
		MSG_DEBUG("%s", pRawData);
		if (pRawData)
			MmsInsertPresentation(pMmsMsg, MIME_APPLICATION_SMIL, NULL, pRawData, strlen(pRawData));

		pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;
		pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;

		for (int i = 0; i < pageCnt; ++i) {
			MMS_PAGE_S *pPage = _MsgMmsGetPage(pMsgData, i);
			int mediaCnt = pPage->mediaCnt;
			MSG_DEBUG("PAGE %d's media Cnt: %d", i+1, mediaCnt);

			for (int j = 0; j < mediaCnt; ++j) {
				MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, j);

				switch (pMedia->mediatype) {
				case MMS_SMIL_MEDIA_IMG:
				case MMS_SMIL_MEDIA_VIDEO:
				case MMS_SMIL_MEDIA_AUDIO:
				case MMS_SMIL_MEDIA_TEXT:
					if (pMedia->szFilePath[0] != 0) {
						if (!MmsInsertPartFromFile(pMmsMsg, pMedia->szFileName, pMedia->szFilePath, pMedia->szContentID))
							return false;
					}
					break;

				default:
					break;
				}
			}
		}

		char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };;
		snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%d.mms", pMsgInfo->msgId);

		MmsPluginStorage *pStorage = MmsPluginStorage::instance();
		err = pStorage->getMsgText(pMsgData, pMsgInfo->msgText);
		MmsMakePreviewInfo(pMsgInfo->msgId, pMsgData);
	}

#ifdef FEATURE_JAVA_MMS
	MSG_DEBUG("msgAppId: valid:%d appId:%s replyToAppId:%s", pMsgData->msgAppId.valid, pMsgData->msgAppId.appId, pMsgData->msgAppId.replyToAppId);
	if (pMsgData->msgAppId.valid) {	// check if msgAppId.valid is true, both appId and replytoappId must have a meaning data
		if (pMsgData->msgAppId.appId[0] != 0) {
			pMmsMsg->msgType.param.szApplicationID = (char *)malloc(strlen(pMsgData->msgAppId.appId) + 1);
			if (pMmsMsg->msgType.param.szApplicationID == NULL) {
				MSG_DEBUG("Error: out of Memory");
				return false;
			}
			memset(pMmsMsg->msgType.param.szApplicationID, 0, strlen(pMsgData->msgAppId.appId) + 1);

			strcpy(pMmsMsg->msgType.param.szApplicationID, pMsgData->msgAppId.appId);
		}

		if (pMsgData->msgAppId.replyToAppId[0] != 0) {
			pMmsMsg->msgType.param.szReplyToApplicationID = (char *)malloc(strlen(pMsgData->msgAppId.replyToAppId) + 1);
			if (pMmsMsg->msgType.param.szReplyToApplicationID == NULL) {
				MSG_DEBUG("Error: out of Memory");
				return false;
			}
			memset(pMmsMsg->msgType.param.szReplyToApplicationID, 0, strlen(pMsgData->msgAppId.replyToAppId) + 1);

			strcpy(pMmsMsg->msgType.param.szReplyToApplicationID, pMsgData->msgAppId.replyToAppId);
		}
	}
#endif

	//Processing Attachment List
	for (int i = 0; i < _MsgMmsGetAttachCount(pMsgData); ++i) {
		MMS_ATTACH_S *pMedia = _MsgMmsGetAttachment(pMsgData, i);
		if (pMedia->szFilePath[0] != 0) {
			if (!MmsInsertPartFromFile(pMmsMsg, pMedia->szFileName, pMedia->szFilePath, NULL)) {
				free(pMedia);
				return false;
			}
		}
	}

	return true;
}

void MmsComposeNotiMessage(MmsMsg *pMmsMsg, msg_message_id_t msgID)
{
	MSG_BEGIN();

	struct tm *timeInfo = NULL;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	pMmsMsg->msgID = msgID;

	pMmsMsg->mmsAttrib.version = mmsHeader.version;

	// setting date
	time(&RawTime);
	timeInfo = localtime(&RawTime);
	nTimeInSecs = mktime(timeInfo);
	pMmsMsg->mmsAttrib.date = nTimeInSecs;

	pMmsMsg->mmsAttrib.bReportAllowed = mmsHeader.reportAllowed;
	pMmsMsg->mmsAttrib.bAskDeliveryReport = mmsHeader.deliveryReport;

	MSG_DEBUG("######## Version = %d ########", pMmsMsg->mmsAttrib.version);

	strncpy(pMmsMsg->szTrID, mmsHeader.szTrID, MMS_TR_ID_LEN);
	strncpy(pMmsMsg->szMsgID, mmsHeader.szMsgID, MMS_MSG_ID_LEN);
	pMmsMsg->szForwardMsgID[0] = '\0';

	if (mmsHeader.pFrom) {
		MmsAddrUtilRemovePlmnString(mmsHeader.pFrom->szAddr );
		strncpy(pMmsMsg->mmsAttrib.szFrom, mmsHeader.pFrom->szAddr, MSG_LOCALE_ADDR_LEN + 9);
	}

	strncpy(pMmsMsg->mmsAttrib.szSubject, mmsHeader.szSubject, MSG_LOCALE_SUBJ_LEN);
	strncpy(pMmsMsg->szContentLocation, mmsHeader.szContentLocation, MMS_LOCATION_LEN);

	pMmsMsg->mmsAttrib.msgClass = mmsHeader.msgClass;
	pMmsMsg->mmsAttrib.msgSize = mmsHeader.msgSize;
	pMmsMsg->mmsAttrib.expiryTime.type = mmsHeader.expiryTime.type;
	pMmsMsg->mmsAttrib.expiryTime.time = mmsHeader.expiryTime.time;
	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_NOTIFY;
	pMmsMsg->mmsAttrib.bRead = false;
	pMmsMsg->mailbox = MSG_INBOX_ID;

	pMmsMsg->mmsAttrib.replyCharge.chargeType = mmsHeader.replyCharge.chargeType;
	pMmsMsg->mmsAttrib.replyCharge.deadLine.type = mmsHeader.replyCharge.deadLine.type;
	pMmsMsg->mmsAttrib.replyCharge.deadLine.time = mmsHeader.replyCharge.deadLine.time;
	pMmsMsg->mmsAttrib.replyCharge.chargeSize = mmsHeader.replyCharge.chargeSize;

	strncpy(pMmsMsg->mmsAttrib.replyCharge.szChargeID, mmsHeader.replyCharge.szChargeID, MMS_MSG_ID_LEN);

	MSG_END();
}

void MmsComposeReadReportMessage(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, msg_message_id_t selectedMsgId)
{
	struct tm *timeInfo = NULL;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	// setting mmsMsg structure
	pMmsMsg->mailbox = pMsgInfo->folderId;
	pMmsMsg->msgID = pMsgInfo->msgId;

	memset(pMmsMsg->szTrID, 0, MMS_TR_ID_LEN + 1);
	memset(pMmsMsg->szContentLocation, 0, MMS_LOCATION_LEN + 1);
	memset(pMmsMsg->szForwardMsgID, 0, MMS_MSG_ID_LEN + 1);

	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_DRAFT;

	// setting date
	time(&RawTime);
	timeInfo = localtime(&RawTime);
	nTimeInSecs = mktime(timeInfo);
	pMmsMsg->mmsAttrib.date = nTimeInSecs;

	// setting szMsgId
	MmsPluginStorage::instance()->getMmsMessageId(selectedMsgId, pMmsMsg);

	//setting subject
	strcpy(pMmsMsg->mmsAttrib.szSubject, pMsgInfo->subject);

	//setting adddress
	MmsSetMsgAddressList(&pMmsMsg->mmsAttrib, pMsgInfo);

	if (pMmsMsg->mmsAttrib.szTo)
		strncpy(pMmsMsg->mmsAttrib.szFrom, pMmsMsg->mmsAttrib.szTo, strlen(pMmsMsg->mmsAttrib.szTo));
}

bool MmsFindMatchedMedia(MMS_MESSAGE_DATA_S *pMsgData, char *pszFilePath)
{
	if (pMsgData == NULL || pszFilePath == NULL)
		return false;

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pMsgData->pageCnt; pageIdx++) {
			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			if (page && page->medialist) {
				for (int mediaIdx = 0; mediaIdx < page->mediaCnt; mediaIdx++) {
					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, mediaIdx);
					if (media) {
						if (strcmp(pszFilePath, media->szFilePath) == 0)
							return true;
					}
				}
			}
		}
	}

	return false;
}

bool MmsCheckAdditionalMedia(MMS_MESSAGE_DATA_S *pMsgData, MsgType *partHeader)
{
	if (MmsFindMatchedMedia(pMsgData, partHeader->param.szFileName))
		return false;
	else
		return true;
}

/*PIM objects SHALL be supported as attachments to an MM*/
bool MmsRemovePims(MMS_MESSAGE_DATA_S *pMsgData)
{
	GList *cur_page = NULL;
	GList *cur_media = NULL;

	if (pMsgData == NULL)
		return false;

	cur_page = pMsgData->pagelist;

	while(cur_page) {
		MMS_PAGE_S *page = (MMS_PAGE_S *)cur_page->data;
		if (page) {
			cur_media = page->medialist;

			while(cur_media) {
				MMS_MEDIA_S *pMedia = (MMS_MEDIA_S *)cur_media->data;
				if (pMedia) {
					int tempType;
					MsgGetTypeByFileName(&tempType, pMedia->szFilePath);
					if (tempType == MIME_TEXT_X_VCALENDAR || tempType == MIME_TEXT_X_VCARD) {
						page->medialist = g_list_remove_all(page->medialist, pMedia);
						page->mediaCnt = g_list_length(page->medialist);
						cur_media = page->medialist;
						free(pMedia);
					} else {
						cur_media =  g_list_next(cur_media);
					}
				} else {
					cur_media =  g_list_next(cur_media);
				}
			} //cur_media while for remove pims file in list

			if (page->medialist == NULL) {//remove empty page
				pMsgData->pagelist = g_list_remove_all(pMsgData->pagelist, page);
				pMsgData->pageCnt = g_list_length(pMsgData->pagelist);
				cur_page = pMsgData->pagelist;
				free(page);
			} else {
				cur_page =  g_list_next(cur_page);
			}
		}
	}//cur_page while


	return true;
}

msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	if (pMmsMsg == NULL)
		return MSG_ERR_NULL_POINTER;

	MmsPluginStorage::instance()->removePreviewInfo(msgId); //remove exist previnfo

	if (pMmsMsg->pageCnt > 0) {

		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_PAGE, (char *)"pagecount", pMmsMsg->pageCnt);

		pPage = _MsgMmsGetPage(pMmsMsg, 0);
		for (int j = 0; j < pPage->mediaCnt; j++) {

			pMedia = _MsgMmsGetMedia(pPage, j);
			MSG_DEBUG("pMedia's Name: %s", pMedia->szFilePath);

			if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG || pMedia->mediatype == MMS_SMIL_MEDIA_VIDEO) {
				char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };
				char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };
				char *pszExt = NULL;

				memset(szFileName, 0x00, MSG_FILENAME_LEN_MAX+1);
				memset(thumbPath, 0x00, MSG_FILEPATH_LEN_MAX);

				snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%d.mms",msgId);

				if ((pszExt = strrchr(pMedia->szFilePath, '.')) != NULL && !strcasecmp(pszExt, ".png")) {
					snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"/%s.png", szFileName);
				} else {
					snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"/%s.jpg", szFileName);
				}

				if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG) {
					if (makeImageThumbnail(pMedia->szFilePath, thumbPath) == true) {
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_IMG, thumbPath);
					} else {
						MSG_DEBUG("Fail of generating thumbnail: %s to %s", pMedia->szFilePath, thumbPath);
					}
				} else {
					if (makeImageThumbnail(pMedia->szFilePath, thumbPath) == true) {
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_VIDEO, thumbPath);
					} else {
						MSG_DEBUG("Fail of generating thumbnail: %s to %s", pMedia->szFilePath, thumbPath);
					}
				}

			} else if (pMedia->mediatype == MMS_SMIL_MEDIA_AUDIO) {
				MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_AUDIO, pMedia->szFileName);
			}
		}
	} else {
		MSG_DEBUG("There is no page");
	}

	int attachCnt = _MsgMmsGetAttachCount(pMmsMsg);
	if (attachCnt > 0) {
		MMS_ATTACH_S *pAttach = _MsgMmsGetAttachment(pMmsMsg, 0);
		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_ATTACH, pAttach->szFileName, attachCnt);
	} else {
		MSG_DEBUG("There is no attachment");
	}

	return MSG_SUCCESS;
}

void MmsPrintFileInfoForVLD(MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	if (pMmsMsg == NULL)
		return;

	if (pMmsMsg->pageCnt > 0) {
		for (int i = 0; i < pMmsMsg->pageCnt; i++) {

			pPage = _MsgMmsGetPage(pMmsMsg, i);

			if (pPage == NULL)
				continue;

			for (int j = 0; j < pPage->mediaCnt; j++) {

				pMedia = _MsgMmsGetMedia(pPage, j);
				if (pMedia == NULL)
					continue;

				MSG_MMS_VLD_FILE("[%s], %d", pMedia->szFilePath, MsgGetFileSize(pMedia->szFilePath));
			}
		}
	}

	int attachCnt = _MsgMmsGetAttachCount(pMmsMsg);
	if (attachCnt > 0) {
		for (int i = 0; i < pMmsMsg->attachCnt; i++) {
		MMS_ATTACH_S *pAttach = _MsgMmsGetAttachment(pMmsMsg, i);
		MSG_MMS_VLD_FILE("[%s], %d", pAttach->szFilePath, MsgGetFileSize(pAttach->szFilePath));
		}
	}
}

MMS_MEDIA_S *MmsFindMediaWithCID(MMS_MESSAGE_DATA_S *pMsgData, const char *szContentID)
{

	if (pMsgData == NULL || szContentID == NULL || strlen(szContentID) == 0) {
		MSG_DEBUG("Invalid Parameter pMsgData = %p, szContentID = %p", pMsgData, szContentID);
		return NULL;
	}

	if (pMsgData->pagelist) {
		for (int pageIdx = 0; pageIdx < pMsgData->pageCnt; pageIdx++) {

			MMS_PAGE_S *page = (MMS_PAGE_S *)g_list_nth_data(pMsgData->pagelist, pageIdx);

			if (page && page->medialist) {

				for (int mediaIdx = 0; mediaIdx < page->mediaCnt; mediaIdx++) {

					MMS_MEDIA_S *media = (MMS_MEDIA_S *)g_list_nth_data(page->medialist, mediaIdx);

					if (media) {
						if (strcmp(media->szContentID,  szContentID) == 0) {
							MSG_DEBUG("Find media with Content ID [%s] from pMsgData", szContentID);
							return media;
						}

					} else {
						MSG_DEBUG("Error media NULL");
						return NULL;
					}
				} //end for media list
			}
		} //end for page list
	}

	MSG_DEBUG("Not exist Matched media with [%s]", szContentID);
	return NULL;
}

MsgMultipart *MmsGetNthMultipart(MmsMsg *pMsg, int index)
{
	MsgMultipart *pPart = NULL;

	if (MsgIsMultipart(pMsg->msgType.type) == true) {

		pPart = pMsg->msgBody.body.pMultipart;

		while (pPart && index--) {
			pPart = pPart->pNext;
		}

		if (pPart == NULL) {
			MSG_DEBUG("There is no such Multipart [index = %d].", index);
			return NULL;
		} else {
			return pPart;
		}
	} else {
		MSG_DEBUG("This Msg is not Multipart");
	}

	return NULL;
}

bool MmsInsertPartToMmsData(MMS_MESSAGE_DATA_S *pMsgData, MMS_MULTIPART_DATA_S *pMultipart)
{
	MMS_MEDIA_S * match_media = NULL;
	bool isPimsFile = false;
	int tempType;//MimeType

	if (pMsgData == NULL || pMultipart == NULL) {
		return false;
	}

	//for pims file add to attach
	MsgGetTypeByFileName(&tempType, pMultipart->szFilePath);
	if (tempType == MIME_TEXT_X_VCALENDAR || tempType == MIME_TEXT_X_VCARD) {
		MSG_DEBUG("Pims File");
		isPimsFile = true;
	} else {

		if (strlen(pMultipart->szContentID) > 0) {
			char szTempContentID[MSG_MSG_ID_LEN + 1] = {0,};
			removeLessGreaterMark(pMultipart->szContentID, szTempContentID, sizeof(szTempContentID));
			match_media = MmsFindMediaWithCID(pMsgData, szTempContentID);

			if (match_media == NULL && strlen(pMultipart->szContentLocation) > 0 ) {
				match_media = MmsFindMediaWithCID(pMsgData, pMultipart->szContentLocation);
			}

		} else {
			MSG_DEBUG("ContentID is NULL");
			match_media = NULL;
		}

	}

	if (match_media && isPimsFile == false) { // set file path
		snprintf(match_media->szFilePath, sizeof(match_media->szFilePath), "%s", pMultipart->szFilePath);
		snprintf(match_media->szFileName, sizeof(match_media->szFileName), "%s", pMultipart->szFileName);
		snprintf(match_media->szContentID, sizeof(match_media->szContentID), "%s", pMultipart->szContentID);
		snprintf(match_media->szContentLocation, sizeof(match_media->szContentLocation), "%s", pMultipart->szContentLocation);
		snprintf(match_media->szContentType, sizeof(match_media->szContentType), "%s", pMultipart->szContentType);
	} else { // add attach

		MMS_ATTACH_S *attachment = NULL;
		attachment = (MMS_ATTACH_S *)calloc(sizeof(MMS_ATTACH_S), 1);

		attachment->mediatype = (MimeType)tempType;
		snprintf(attachment->szContentType, sizeof(attachment->szContentType), "%s", pMultipart->szContentType);
		snprintf(attachment->szFilePath, sizeof(attachment->szFilePath), "%s", pMultipart->szFilePath);
		snprintf(attachment->szFileName, sizeof(attachment->szFileName), "%s", pMultipart->szFileName);
		attachment->fileSize = MsgGetFileSize(attachment->szFilePath);

		if (_MsgMmsAddAttachment(pMsgData, attachment) != MSG_SUCCESS) {
			g_free(attachment);
			return false;
		}
	}

	return true;
}

bool MmsEncodeMmsMessage(MmsMsg *pMmsMsg, const char *raw_filepath)
{
	bool encode_ret = false;
	mode_t file_mode = (S_IRUSR | S_IWUSR);

	if (pMmsMsg == NULL || raw_filepath == NULL) {
		MSG_DEBUG("Invalid Parameter pMmsMsg = %p , raw_filepath = %p", pMmsMsg, raw_filepath);
		return false;
	}

	FILE *pFile = MsgOpenFile(raw_filepath, "wb+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", strerror(errno));
		goto __CATCH;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MSG_DEBUG("File Fseek Error: %s", strerror(errno));
		goto __CATCH;
	}

	if (fchmod(fileno(pFile), file_mode) < 0) {
		MSG_DEBUG("File chmod Error: %s", strerror(errno));
		goto __CATCH;
	}

	switch(pMmsMsg->mmsAttrib.msgType)
	{
		case MMS_MSGTYPE_SEND_REQ:
		case MMS_MSGTYPE_SEND_CONF:
			encode_ret = MmsEncodeSendReq(pFile, pMmsMsg);
			if (encode_ret == false) {
				MSG_DEBUG("Fail to MmsEncodeSendReq");
				goto __CATCH;
			}
		break;
		default:
			MSG_DEBUG("Not Support msg type : %d", pMmsMsg->mmsAttrib.msgType);
			goto __CATCH;
		break;
	}

	MsgFsync(pFile);	//file is written to device immediately, it prevents missing file data from unexpected power off
	MsgCloseFile(pFile);

	return true;

__CATCH:
	if (pFile) {
		MsgCloseFile(pFile);
	}

	return false;
}

bool MmsInsertPartFromMultipart(MmsMsg *pMsg, MMS_MULTIPART_DATA_S *pNewMultipart)
{
	MsgMultipart *pMultipart = NULL;
	MsgMultipart *pLastPart = NULL;

	MimeType mimeType = MIME_UNKNOWN;
	char *pExt = NULL;

	pExt = strrchr(pNewMultipart->szFilePath, '.');

	if (pExt == NULL || pExt[0] == '\0' || strrchr(pExt, '/')) {
		//mimeType = MIME_UNKNOWN;
		mimeType = MimeGetMimeIntFromMimeString(pNewMultipart->szContentType);
	} else {
		if (strcasecmp(pExt, ".dcf") == 0)
			mimeType = MIME_APPLICATION_VND_OMA_DRM_CONTENT;
		else {
			if (MmsGetTypeByFileName((int *)&mimeType, pNewMultipart->szFilePath) == false)
				goto __CATCH;
		}
	}

	MSG_DEBUG("type = %d, name = %s, filepath = %s, cid = %s, cl = %s", mimeType, pNewMultipart->szFileName, pNewMultipart->szFilePath, pNewMultipart->szContentID, pNewMultipart->szContentLocation);

	if (mimeType == MIME_UNKNOWN)
		mimeType = MIME_APPLICATION_OCTET_STREAM;

	if (MmsIsMultipart(pMsg->msgType.type) == true) {
		/* Insert as a multipart */
		pMultipart = MmsMakeMultipart(mimeType, pNewMultipart->szFileName, pNewMultipart->szFilePath, pNewMultipart->szContentID, pNewMultipart->szContentLocation);

		if (pMultipart == NULL)
			goto __CATCH;

		if (pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED ||
			pMsg->mmsAttrib.contentType == MIME_MULTIPART_MIXED)
			pMultipart->type.disposition = MSG_DISPOSITION_ATTACHMENT;

		if (pMsg->msgBody.body.pMultipart == NULL) {
			pMsg->msgBody.body.pMultipart = pMultipart;
		} else {
			pLastPart = pMsg->msgBody.body.pMultipart;
			while (pLastPart->pNext) {
				pLastPart = pLastPart->pNext;
			}

			pLastPart->pNext = pMultipart;
		}

		pMsg->msgBody.size += pMultipart->pBody->size;
		pMsg->msgType.contentSize += pMultipart->pBody->size;
	} else {
		/* Single part - Insert as a message body */
		if (pMsg->mmsAttrib.contentType != mimeType || pMsg->msgType.type != mimeType)
			goto __CATCH;

		strncpy(pMsg->msgType.param.szName, pNewMultipart->szFileName, MSG_LOCALE_FILENAME_LEN_MAX);

		if (MmsIsText(pMsg->msgType.type) == true) {
			pMsg->msgType.param.charset = MSG_CHARSET_UTF8;
		}

		strncpy(pMsg->msgBody.szOrgFilePath, pNewMultipart->szFilePath, MSG_FILEPATH_LEN_MAX - 1);
		pMsg->msgBody.offset = 0;
		pMsg->msgBody.size = MsgGetFileSize(pNewMultipart->szFilePath);
		pMsg->msgType.contentSize = MsgGetFileSize(pNewMultipart->szFilePath);
	}

	pMsg->nPartCount++;

	return true;

__CATCH:
	return false;

}

bool MmsMakeMmsData(MmsMsg *pMsg, MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MSG_BEGIN();

	bzero(pMmsMsg, sizeof(MMS_MESSAGE_DATA_S));
	pMmsMsg->regionCnt = 0;
	pMmsMsg->pageCnt = 0;
	pMmsMsg->attachCnt = 0;
	pMmsMsg->transitionCnt = 0;
	pMmsMsg->metaCnt = 0;
	memset(pMmsMsg->szSmilFilePath, 0, MSG_FILEPATH_LEN_MAX);

	if (pMsg->mmsAttrib.contentType == MIME_MULTIPART_RELATED || pMsg->mmsAttrib.contentType == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {
		char *pSmilDoc = NULL;

		if (pMsg->msgBody.pPresentationBody) {
			if (pMsg->msgBody.pPresentationBody->body.pText) {
				pSmilDoc = pMsg->msgBody.pPresentationBody->body.pText;
				if (pSmilDoc) {
					MmsSmilParseSmilDocOnlyLayout(pMmsMsg, pSmilDoc);
				}

				pMmsMsg->smil.type = MIME_APPLICATION_SMIL;
				snprintf(pMmsMsg->smil.szContentType, MSG_MSG_ID_LEN, "%s", MimeGetMimeStringFromMimeInt(pMsg->msgBody.presentationType.type));
				snprintf(pMmsMsg->smil.szContentID, MSG_MSG_ID_LEN, "%s", pMsg->msgBody.presentationType.szContentID);
				snprintf(pMmsMsg->smil.szContentLocation, MSG_MSG_ID_LEN, "%s", pMsg->msgBody.presentationType.szContentLocation);
				snprintf(pMmsMsg->smil.szFileName, MSG_FILENAME_LEN_MAX, "%s", pMsg->msgBody.presentationType.param.szName);
				snprintf(pMmsMsg->smil.szFilePath, MSG_FILEPATH_LEN_MAX, MSG_DATA_PATH"%s", pMsg->msgBody.presentationType.param.szFileName);
			}
		} else {
			MSG_DEBUG("Not Exist pPresentationBody");
		}
	}

	int partCnt = pMsg->nPartCount;

	for (int i = 0; i < partCnt; ++i) {

		MsgMultipart *multipart = MmsGetNthMultipart(pMsg, i);
		MMS_MULTIPART_DATA_S pMultipart;

		if (multipart == NULL) {
			MSG_DEBUG("multipart is NULL [%d]", i);
			goto FREE_CATCH;
		}

		bzero(&pMultipart, sizeof(MMS_MULTIPART_DATA_S));
		snprintf(pMultipart.szContentID, sizeof(pMultipart.szContentID), "%s", multipart->type.szContentID);
		snprintf(pMultipart.szContentLocation, sizeof(pMultipart.szContentLocation), "%s", multipart->type.szContentLocation);
		snprintf(pMultipart.szFileName, sizeof(pMultipart.szFileName), "%s", multipart->type.param.szName);
		snprintf(pMultipart.szFilePath, sizeof(pMultipart.szFilePath), "%s", multipart->pBody->szOrgFilePath);
		snprintf(pMultipart.szContentType, sizeof(pMultipart.szContentType), "%s",MimeGetMimeStringFromMimeInt(multipart->type.type));

		if (MmsInsertPartToMmsData(pMmsMsg, &pMultipart) == false) {
			MSG_DEBUG("Fail to MmsSetMultipartToMmsData");
			goto FREE_CATCH;
		}
	}
	MSG_END();
	return true;
FREE_CATCH:
	return false;
}

void MmsPrintMmsMsg(const MmsMsg *pMmsMsg)
{
	MmsMsgType msgType = pMmsMsg->mmsAttrib.msgType;
	if (msgType == MMS_MSGTYPE_SEND_REQ) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_SEND_REQ");
	} else if (msgType == MMS_MSGTYPE_SEND_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_SEND_CONF");
	} else if (msgType == MMS_MSGTYPE_NOTIFICATION_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_NOTIFICATION_IND");
	} else if (msgType == MMS_MSGTYPE_NOTIFYRESP_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_NOTIFYRESP_IND");
	} else if (msgType == MMS_MSGTYPE_RETRIEVE_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_RETRIEVE_CONF");
	} else if (msgType == MMS_MSGTYPE_ACKNOWLEDGE_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_ACKNOWLEDGE_IND");
	} else if (msgType == MMS_MSGTYPE_DELIVERY_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_DELIVERY_IND");
	} else if (msgType == MMS_MSGTYPE_READREC_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READREC_IND");
	} else if (msgType == MMS_MSGTYPE_READORG_IND) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READORG_IND");
	} else if (msgType == MMS_MSGTYPE_FORWARD_REQ) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_FORWARD_REQ");
	} else if (msgType == MMS_MSGTYPE_FORWARD_CONF) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_FORWARD_CONF");
	} else if (msgType == MMS_MSGTYPE_READ_REPLY) {
		PRINT_KEY_VAL_STR("mms type", "MMS_MSGTYPE_READ_REPLY");
	} else {
		PRINT_KEY_VAL_STR("mms type", "Unknown");
	}

	PRINT_KEY_VAL_STR("mms ver", pMmsMsg->mmsAttrib.version);

}

bool MmsComposeSendReq(MmsMsg *pMmsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMsgData)
{
	MSG_BEGIN();

	char *pRawData = NULL;
	AutoPtr<char> buf(&pRawData);
	struct tm *timeInfo = NULL;
	time_t RawTime = 0;
	time_t nTimeInSecs = 0;

	// Initialize mmsMsg structure
	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	// setting mmsMsg structure
	pMmsMsg->mailbox = pMsgInfo->folderId;
	pMmsMsg->msgID = pMsgInfo->msgId;

	memset(pMmsMsg->szTrID, 0, MMS_TR_ID_LEN + 1);
	memset(pMmsMsg->szContentLocation, 0, MMS_LOCATION_LEN + 1);
	memset(pMmsMsg->szMsgID, 0, MMS_MSG_ID_LEN + 1);
	memset(pMmsMsg->szForwardMsgID, 0, MMS_MSG_ID_LEN + 1);

	pMmsMsg->mmsAttrib.dataType = MMS_DATATYPE_DRAFT;
	pMmsMsg->mmsAttrib.msgType = MMS_MSGTYPE_SEND_REQ;

	if (pSendOptInfo->bSetting == false) {
		unsigned int expiryTime;
		MSG_MMS_DELIVERY_TIME_T deliveryTime;

		pMmsMsg->mmsAttrib.priority = (MmsPriority)MsgSettingGetInt(MMS_SEND_PRIORITY);

		MsgSettingGetBool(MMS_SEND_DELIVERY_REPORT, &pMmsMsg->mmsAttrib.bAskDeliveryReport);
		MsgSettingGetBool(MMS_SEND_READ_REPLY, &pMmsMsg->mmsAttrib.bAskReadReply);
		MsgSettingGetBool(MSG_KEEP_COPY, &pMmsMsg->mmsAttrib.bLeaveCopy);

		expiryTime = (unsigned int)MsgSettingGetInt(MMS_SEND_EXPIRY_TIME);

		if (expiryTime == 0)
			pMmsMsg->mmsAttrib.expiryTime.type = MMS_TIMETYPE_NONE;
		else {
			pMmsMsg->mmsAttrib.expiryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.expiryTime.time = expiryTime;
		}

		deliveryTime = (MSG_MMS_DELIVERY_TIME_T)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);

		if (deliveryTime == MSG_DELIVERY_TIME_CUSTOM) {
			pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = true;

			pMmsMsg->mmsAttrib.deliveryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_CUSTOM_DELIVERY);
		} else {
			pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = false;

			pMmsMsg->mmsAttrib.deliveryTime.type = MMS_TIMETYPE_RELATIVE;
			pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)deliveryTime;
		}
	} else {
		pMmsMsg->mmsAttrib.priority = (MmsPriority)pSendOptInfo->option.mmsSendOptInfo.priority;
		pMmsMsg->mmsAttrib.bAskDeliveryReport = pSendOptInfo->bDeliverReq;
		pMmsMsg->mmsAttrib.bAskReadReply = pSendOptInfo->option.mmsSendOptInfo.bReadReq;
		pMmsMsg->mmsAttrib.expiryTime.type = pSendOptInfo->option.mmsSendOptInfo.expiryTime.type;
		pMmsMsg->mmsAttrib.bLeaveCopy = pSendOptInfo->bKeepCopy;

		if (pMmsMsg->mmsAttrib.expiryTime.type != MMS_TIMETYPE_NONE)
			pMmsMsg->mmsAttrib.expiryTime.time = pSendOptInfo->option.mmsSendOptInfo.expiryTime.time;

		pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = pSendOptInfo->option.mmsSendOptInfo.bUseDeliveryCustomTime;
		pMmsMsg->mmsAttrib.deliveryTime.type = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.type;
		pMmsMsg->mmsAttrib.deliveryTime.time = pSendOptInfo->option.mmsSendOptInfo.deliveryTime.time;
	}

	MSG_DEBUG("pSendOptInfo->bSetting = %d", pSendOptInfo->bSetting);
	MSG_DEBUG("pMmsMsg->mmsAttrib.bLeaveCopy = %d", pMmsMsg->mmsAttrib.bLeaveCopy);

	MSG_DEBUG("pMmsMsg->mmsAttrib.bUseDeliveryCustomTime = %d", pMmsMsg->mmsAttrib.bUseDeliveryCustomTime);
	MSG_DEBUG("pMmsMsg->mmsAttrib.deliveryTime.type = %d", pMmsMsg->mmsAttrib.deliveryTime.type);
	MSG_DEBUG("pMmsMsg->mmsAttrib.deliveryTime.time = %d", pMmsMsg->mmsAttrib.deliveryTime.time);

	MSG_DEBUG("pMmsMsg->mmsAttrib.priority = %d", pMmsMsg->mmsAttrib.priority);

	MSG_DEBUG("pMmsMsg->mmsAttrib.bAskDeliveryReport = %d", pMmsMsg->mmsAttrib.bAskDeliveryReport);
	MSG_DEBUG("pMmsMsg->mmsAttrib.bAskReadReply = %d", pMmsMsg->mmsAttrib.bAskReadReply);

	MSG_DEBUG("pMmsMsg->mmsAttrib.expiryTime.type = %d", pMmsMsg->mmsAttrib.expiryTime.type);
	MSG_DEBUG("pMmsMsg->mmsAttrib.expiryTime.time = %d", pMmsMsg->mmsAttrib.expiryTime.time);

	/* MMS-1.3-con-739 */
	pMmsMsg->mmsAttrib.msgClass = (MmsMsgClass)MsgSettingGetInt(MMS_SEND_MSG_CLASS);
	/* MMS-1.3-con-739 */
#ifdef MMS_13_CON_742_ENABLED
	/* MMS-1.3-con-742 */
	pMmsMsg->mmsAttrib.deliveryTime.time = (unsigned int)MsgSettingGetInt(MMS_SEND_DELIVERY_TIME);
	/* MMS-1.3-con-742 */
#endif

	// setting date
	time(&RawTime);
	timeInfo = localtime(&RawTime);
	nTimeInSecs = mktime(timeInfo);
	pMmsMsg->mmsAttrib.date = nTimeInSecs;	// todo: need to subtract timeline value to make GMT+0 time

	//setting subject
	strcpy(pMmsMsg->mmsAttrib.szSubject, pMsgInfo->subject);

	//setting adddress
	MmsSetMsgAddressList(&pMmsMsg->mmsAttrib, pMsgInfo);

	//default type mixed
	pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
	pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;

	int pageCnt = _MsgMmsGetPageCount(pMsgData);

	if (pageCnt >  0) {	// Multipart related

		int RawDataSize = 0;
		time_t RawTime = 0;
		time(&RawTime);

		snprintf(pMsgData->szSmilFilePath, MSG_FILEPATH_LEN_MAX, "%lu", RawTime);

		MsgMMSCreateSMIL(pMsgData);

		RawDataSize = MmsGetSmilRawData(pMsgData, &pRawData);
		if (RawDataSize < 0) {
			MSG_DEBUG("Smil file size is less than 0");
			return false;
		}
		MSG_DEBUG("%s", pRawData);
		if (pRawData)
			MmsInsertPresentation(pMmsMsg, MIME_APPLICATION_SMIL, NULL, pRawData, strlen(pRawData));

		pMmsMsg->mmsAttrib.contentType = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;
		pMmsMsg->msgType.type = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;

	}

	if (pMmsMsg->msgType.type == MIME_APPLICATION_VND_WAP_MULTIPART_RELATED) {

		int pageCnt = _MsgMmsGetPageCount(pMsgData);

		for (int i = 0; i < pageCnt; ++i) {
			MMS_PAGE_S *pPage = _MsgMmsGetPage(pMsgData, i);
			int mediaCnt = pPage->mediaCnt;
			MSG_DEBUG("PAGE %d's media Cnt: %d", i+1, mediaCnt);

			for (int j = 0; j < mediaCnt; ++j) {
				MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, j);
				if (pMedia->szFilePath[0] != 0) {
					MMS_MULTIPART_DATA_S pMultipart;
					bzero(&pMultipart, sizeof(MMS_MULTIPART_DATA_S));
					snprintf(pMultipart.szContentID, sizeof(pMultipart.szContentID), "%s", pMedia->szContentID);
					snprintf(pMultipart.szContentLocation, sizeof(pMultipart.szContentLocation), "%s", pMedia->szContentLocation);
					snprintf(pMultipart.szFileName, sizeof(pMultipart.szFileName), "%s", pMedia->szFileName);
					snprintf(pMultipart.szFilePath, sizeof(pMultipart.szFilePath), "%s", pMedia->szFilePath);
					snprintf(pMultipart.szContentType, sizeof(pMultipart.szContentType), "%s", pMedia->szContentType);

					if (!MmsInsertPartFromMultipart(pMmsMsg, &pMultipart))
						return false;
				}
			}
		}
	}

	//Processing Attachment List
	for (int i = 0; i < _MsgMmsGetAttachCount(pMsgData); ++i) {
		MMS_ATTACH_S *pMedia = _MsgMmsGetAttachment(pMsgData, i);
		if (pMedia->szFilePath[0] != 0) {
			MMS_MULTIPART_DATA_S pMultipart;
			bzero(&pMultipart, sizeof(MMS_MULTIPART_DATA_S));
			snprintf(pMultipart.szContentID, sizeof(pMultipart.szContentID), "%s", pMedia->szFileName);
			snprintf(pMultipart.szContentLocation, sizeof(pMultipart.szContentLocation), "%s", pMedia->szFileName);
			snprintf(pMultipart.szFileName, sizeof(pMultipart.szFileName), "%s", pMedia->szFileName);
			snprintf(pMultipart.szFilePath, sizeof(pMultipart.szFilePath), "%s", pMedia->szFilePath);
			snprintf(pMultipart.szContentType, sizeof(pMultipart.szContentType), "%s", pMedia->szContentType);

			if (!MmsInsertPartFromMultipart(pMmsMsg, &pMultipart))
				return false;
		}
	}

	return true;
}

int MmsUpdatePreviewData(MSG_MESSAGE_INFO_S *pMsgInfo)
{
	MSG_BEGIN();
	MmsMsg *pMmsMsg;
	MMS_MESSAGE_DATA_S msgData = {0,};

	MmsPluginStorage::instance()->getMmsMessage(&pMmsMsg);

	MmsReleaseMmsMsg(pMmsMsg);

	if (MmsReadMsgBody(pMsgInfo->msgId, true, false, NULL) == false) {
		MSG_DEBUG("Fail to MmsReadMsgBody");
		goto __CATCH;
	}

	if (MmsMakeMmsData(pMmsMsg, &msgData) == false) {
		MSG_DEBUG("Fail to makeMmsMessageData");
		goto __CATCH;
	}

	MmsMakePreviewInfo(pMsgInfo->msgId, &msgData);

	MmsPluginStorage::instance()->getMsgText(&msgData, pMsgInfo->msgText);

	MmsReleaseMmsMsg(pMmsMsg);
	MsgMmsReleaseMmsLists(&msgData);

	MSG_END();
	return 0;

__CATCH:
	MmsReleaseMmsMsg(pMmsMsg);
	MsgMmsReleaseMmsLists(&msgData);
	return -1;
}
