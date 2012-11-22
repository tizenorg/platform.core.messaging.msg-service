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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "MsgTypes.h"
#include "MsgCppTypes.h"
#include "MsgException.h"
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MmsPluginMessage.h"
#include "MmsPluginMIME.h"
#include "MmsPluginAvCodec.h"
#include "MsgUtilFile.h"
#include "MsgDebug.h"
#include "MmsPluginCodec.h"
#include "MmsPluginStorage.h"
#include "MsgMmsMessage.h"
#include "MsgGconfWrapper.h"
#include "MmsPluginUtil.h"
#include "MmsPluginSMILValidate.h"


bool MmsInitMsgAttrib(MmsAttrib *pAttrib)
{
	MSG_DEBUG("MmsInitMsgAttrib");

	pAttrib->bAskDeliveryReport = false;

	pAttrib->bAskReadReply = false;
	pAttrib->bRead = false;
	pAttrib->bReportAllowed = false;
	pAttrib->readReportAllowedType = MMS_RECEIVE_READ_REPORT_ALLOWED;
	pAttrib->readReportSendStatus = MMS_RECEIVE_READ_REPORT_NO_SEND;
	pAttrib->bReadReportSent = false;

	pAttrib->bHideAddress = false;
	pAttrib->date = 0;
	pAttrib->bUseDeliveryCustomTime = false;
	pAttrib->deliveryTime.type = MMS_TIMETYPE_RELATIVE;
	pAttrib->deliveryTime.time = 0;
	pAttrib->bUseExpiryCustomTime = false;
	pAttrib->expiryTime.type = MMS_TIMETYPE_RELATIVE;
	pAttrib->expiryTime.time = 0;
	memset(&pAttrib->expiryTime, 0, sizeof(MmsTimeStruct));
	pAttrib->msgClass = MMS_MSGCLASS_PERSONAL;
	pAttrib->msgStatus = MSG_DELIVERY_REPORT_NONE;
	pAttrib->priority = MMS_PRIORITY_NORMAL;
	pAttrib->responseStatus = MMS_RESPSTATUS_ERROR;
	pAttrib->retrieveStatus = MMS_RETRSTATUS_ERROR;
	pAttrib->contentType = MIME_UNKNOWN;
	pAttrib->msgSize = 0;
	pAttrib->bLeaveCopy = true;

	pAttrib->specialMsgType = MMS_SPECIAL_MSG_TYPE_NONE;
#ifdef __SUPPORT_DRM__
	pAttrib->drmType = MSG_DRM_TYPE_NONE;
	pAttrib->roWaitingTimerMax = 0;
	pAttrib->pszDrmData = NULL;
#endif
	pAttrib->version = MMS_VERSION;

	memset(pAttrib->szFrom, 0, MSG_LOCALE_ADDR_LEN + 10);
	memset(pAttrib->szResponseText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);
	memset(pAttrib->szRetrieveText, 0, MMS_LOCALE_RESP_TEXT_LEN + 1);
	memset(pAttrib->szSubject, 0, MSG_LOCALE_SUBJ_LEN + 1);
	pAttrib->szTo = NULL;
	pAttrib->szCc = NULL;
	pAttrib->szBcc = NULL;

	pAttrib->pMultiStatus = NULL;

	pAttrib->replyCharge.chargeType = MMS_REPLY_NONE;
	memset(&pAttrib->replyCharge.deadLine , 0, sizeof(MmsTimeStruct));
	pAttrib->replyCharge.chargeSize = 0;
	memset(pAttrib->replyCharge.szChargeID, 0, MMS_MSG_ID_LEN);

	return true;
}

bool MmsInitMsgType(MsgType *pMsgType)
{
	MSG_DEBUG("MmsInitMsgType");
	pMsgType->offset = 0;
	pMsgType->size = 0;
	pMsgType->contentSize = 0;
	pMsgType->disposition = -1;
	pMsgType->encoding = -1;
	pMsgType->type = MIME_UNKNOWN;
	pMsgType->section = -1;

	pMsgType->szOrgFilePath[0] = '\0';
	pMsgType->szContentID[0] = '\0';
	pMsgType->szContentLocation[0] = '\0';

	pMsgType->szContentRepPos[0] = '\0';
	pMsgType->szContentRepSize[0] = '\0';
	pMsgType->szContentRepIndex[0] = '\0';

	MmsInitMsgContentParam(&pMsgType->param);
#ifdef __SUPPORT_DRM__
	__MsgInitMsgDRMInfo(&pMsgType->drmInfo);
#endif

	return true;
}

bool MmsInitMsgBody(MsgBody *pMsgBody)
{
	MSG_DEBUG("MmsInitMsgBody");
	pMsgBody->offset = 0;
	pMsgBody->size = 0;
	pMsgBody->body.pText = NULL;

	MmsInitMsgType(&pMsgBody->presentationType);
	pMsgBody->pPresentationBody = NULL;

	memset(pMsgBody->szOrgFilePath, 0, MSG_FILEPATH_LEN_MAX);

	return true;
}

bool MmsInitMsgContentParam(MsgContentParam *pMsgContentParam)
{
	MSG_DEBUG("MmsInitMsgContentParam");
	pMsgContentParam->charset = MSG_CHARSET_UNKNOWN;
	pMsgContentParam->type = MIME_UNKNOWN;
	pMsgContentParam->szBoundary[0] = '\0';
	pMsgContentParam->szFileName[0] = '\0';
	pMsgContentParam->szName[0] = '\0';
	pMsgContentParam->szStart[0] = '\0';
	pMsgContentParam->szStartInfo[0] = '\0';
	pMsgContentParam->pPresentation = NULL;
	pMsgContentParam->reportType = MSG_PARAM_REPORT_TYPE_UNKNOWN; // only used as parameter of Content-Type: multipart/report; report-type
#ifdef FEATURE_JAVA_MMS
	pMsgContentParam->szApplicationID = NULL;
	pMsgContentParam->szReplyToApplicationID = NULL;
#endif
	return true;
}

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

int MmsGetSmilRawData(MMS_MESSAGE_DATA_S *pMsgBody, char **pRawdata)
{
	MSG_BEGIN();

	if (MsgReadSmilFile(pMsgBody->szSmilFilePath, pRawdata) < 0)
		return false;

	MsgDeleteSmilFile(pMsgBody->szSmilFilePath);

	MSG_END();

	return true;
}


bool MmsInsertPresentation(MmsMsg *pMsg, MsgContentType mimeType, char *pData, int size)
{
	MSG_DEBUG("MmsInsertPresentation");

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
	snprintf(pMsg->msgBody.presentationType.szContentID, MSG_MSG_ID_LEN + 1, "<_S_>");

	snprintf(pMsg->msgType.param.szStart, MSG_MSG_ID_LEN + 1, "%s", pMsg->msgBody.presentationType.szContentID);
	pMsg->msgType.param.type = mimeType;

	memset(pMsg->msgBody.pPresentationBody->body.pText, 0, size + 1);
	strncpy(pMsg->msgBody.pPresentationBody->body.pText, pData, size);

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
	MsgContentType mimeType = MIME_UNKNOWN;
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

	if (mimeType == MIME_UNKNOWN)
		mimeType = MIME_APPLICATION_OCTET_STREAM;

	if (MmsIsMultipart(pMsg->msgType.type) == true) {
		/* Insert as a multipart */
		if (MsgGetFileSize(szOrgFilePath, &nFileSize) == false) {
			MSG_DEBUG("MsgGetFileSize: failed");
			goto __CATCH;
		}

		pMultipart = MmsMakeMultipart(mimeType, szTitleName, szOrgFilePath, NULL, 0, nFileSize, szContentID);
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
			if (pMultipart)
				pMultipart->type.encoding	= MSG_ENCODING_8BIT;
		} else {
			if (pMultipart)
				pMultipart->type.encoding	= MSG_ENCODING_BINARY;
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

	if (pMultipart == NULL)
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



MsgMultipart *MmsMakeMultipart(MsgContentType mimeType, char *szTitleName, char *szOrgFilePath, void *pData, int offset, int size, char *szContentID)
{
	MsgMultipart *pMultipart = NULL;

	if ((pMultipart = MmsAllocMultipart()) == NULL)
		goto __CATCH;

	pMultipart->type.type = mimeType;

	if (szTitleName && szTitleName[0]) {
		memset(pMultipart->type.param.szName, 0, MSG_LOCALE_FILENAME_LEN_MAX + 1);
		strncpy(pMultipart->type.param.szName, szTitleName, MSG_LOCALE_FILENAME_LEN_MAX);
	}

	if (szContentID && szContentID[0]) {
		memset(pMultipart->type.szContentID, 0, MSG_MSG_ID_LEN + 1);
		snprintf(pMultipart->type.szContentID, MSG_MSG_ID_LEN + 1, "<%s>", szContentID);

		memset(pMultipart->type.szContentLocation, 0, MSG_MSG_ID_LEN + 1);
		snprintf(pMultipart->type.szContentLocation, MSG_MSG_ID_LEN + 1, "%s", szContentID);
	}

	if (MmsIsText(mimeType) == true) {
		if (!MmsIsVitemContent (mimeType, pMultipart->type.param.szName)) {
			pMultipart->type.param.charset = MSG_CHARSET_UTF8;
		}
		pMultipart->type.encoding = MSG_ENCODING_8BIT;
	} else {
		pMultipart->type.encoding = MSG_ENCODING_BINARY;
	}

	if (szOrgFilePath == NULL) {
		if (pData != NULL) {
			if (MmsIsText(mimeType) == true) {
				pMultipart->pBody->body.pText = (char *)malloc(size + 1);
				if (pMultipart->pBody->body.pText == NULL)
					goto __CATCH;

				memset(pMultipart->pBody->body.pText, 0, size + 1);
			} else {
				pMultipart->pBody->body.pBinary = malloc(size);
				if (pMultipart->pBody->body.pBinary == NULL)
					goto __CATCH;

				memset(pMultipart->pBody->body.pBinary, 0, size);
			}

			memcpy(pMultipart->pBody->body.pBinary, pData, size);
			pMultipart->pBody->size = size;
		}
	} else {
		if (szOrgFilePath)
			strncpy(pMultipart->pBody->szOrgFilePath, szOrgFilePath, MSG_FILEPATH_LEN_MAX - 1);

		pMultipart->pBody->offset = offset;
		pMultipart->pBody->size = size;
	}

	return pMultipart;

__CATCH:

	if (pMultipart) {
		if (pMultipart->pBody) {
			if (pMultipart->pBody->body.pText) {
				free(pMultipart->pBody->body.pText);
				pMultipart->pBody->body.pText = NULL;
			}
			free(pMultipart->pBody);
			pMultipart->pBody = NULL;
		}
		free(pMultipart);
		pMultipart = NULL;
	}

	return NULL;
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
	pMmsMsg->mmsAttrib.date = nTimeInSecs;

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
			MmsInsertPresentation(pMmsMsg, MIME_APPLICATION_SMIL, pRawData, strlen(pRawData));

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
			if (!MmsInsertPartFromFile(pMmsMsg, pMedia->szFileName, pMedia->szFilePath, NULL))
				return false;
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

bool MmsGetMmsMessageBody(MmsMsg *pMmsMsg, char *retrievedFilePath)
{
	FILE *pFile	= NULL;
	MsgMultipart *pMultipart = NULL;
	int attachmax = MSG_ATTACH_MAX;
	int nSize = 0;

	/*	read from MMS raw file	*/
	if (retrievedFilePath)
		strncpy(pMmsMsg->szFileName, retrievedFilePath + strlen(MSG_DATA_PATH), strlen(retrievedFilePath + strlen(MSG_DATA_PATH)));
	else
		goto __CATCH;

	pFile = MsgOpenFile(retrievedFilePath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG( "_MmsReadMsgBody: invalid mailbox\n");
		goto __CATCH;
	}

	if (MsgGetFileSize(retrievedFilePath, &nSize) == false) {
		MSG_DEBUG("MsgGetFileSize: failed");
		goto __CATCH;
	}

	_MmsRegisterDecodeBuffer(gszMmsLoadBuf1,
							  gszMmsLoadBuf2,
							  MSG_MMS_DECODE_BUFFER_MAX);

	if (MmsBinaryDecodeMsgHeader(pFile, nSize) == false) {
		MSG_DEBUG( "_MmsReadMsgBody: MmsBinaryDecodeMsgHeader fail...\n");
		goto __CATCH;
	}

	if (MmsBinaryDecodeMsgBody(pFile, retrievedFilePath, nSize) == false) {
		MSG_DEBUG( "_MmsReadMsgBody: MmsBinaryDecodeMsgBody fail\n");
		goto __CATCH;
	}

	/* Set mmsHeader.msgType & msgBody to pMsg ----------- */

	pMmsMsg->mmsAttrib.contentType = (MsgContentType)mmsHeader.msgType.type;

	memcpy(&(pMmsMsg->msgType), &(mmsHeader.msgType), sizeof(MsgType));
	memcpy(&(pMmsMsg->msgBody), &(mmsHeader.msgBody), sizeof(MsgBody));

	MSG_DEBUG("#############################");
	MSG_DEBUG("## pMmsMsg->msgType.type = %d ##", pMmsMsg->msgType.type);
	MSG_DEBUG("## pMmsMsg->msgType.szContentID = %s ##", pMmsMsg->msgType.szContentID);
	MSG_DEBUG("## pMmsMsg->msgType.szContentLocation = %s ##", pMmsMsg->msgType.szContentLocation);
	MSG_DEBUG("#############################");

	if (pMmsMsg->msgBody.pPresentationBody) {
		if(MsgFseek(pFile, pMmsMsg->msgBody.pPresentationBody->offset, SEEK_SET) < 0)
			goto __CATCH;

		pMmsMsg->msgBody.pPresentationBody->body.pText = (char *)malloc(pMmsMsg->msgBody.pPresentationBody->size + 1);
		if (pMmsMsg->msgBody.pPresentationBody->body.pText == NULL)
			goto __CATCH;

		memset(pMmsMsg->msgBody.pPresentationBody->body.pText, 0, pMmsMsg->msgBody.pPresentationBody->size + 1);
		ULONG nRead = 0;
		nRead = MsgReadFile(pMmsMsg->msgBody.pPresentationBody->body.pText, sizeof(char), pMmsMsg->msgBody.pPresentationBody->size, pFile);

		if (nRead == 0)
			goto __CATCH;
	}

	MsgCloseFile(pFile);
	pFile = NULL;

	/* nPartCount */
	pMmsMsg->nPartCount = 0;

	if (MsgIsMultipart(mmsHeader.msgType.type) == true) {
		pMultipart = pMmsMsg->msgBody.body.pMultipart;
		while (pMultipart) {
			pMmsMsg->nPartCount++;

			if (pMultipart->type.type == MIME_TEXT_PLAIN)
				attachmax++;

			if ((mmsHeader.msgType.type == MIME_APPLICATION_VND_WAP_MULTIPART_MIXED) ||
				(mmsHeader.msgType.type == MIME_MULTIPART_MIXED)) {
				if ((pMmsMsg->nPartCount >= attachmax ) && (pMultipart->pNext != NULL)) {
					_MsgFreeBody(pMultipart->pNext->pBody, pMultipart->pNext->type.type);

					free(pMultipart->pNext->pBody);
					pMultipart->pNext->pBody = NULL;

					free(pMultipart->pNext);

					pMultipart->pNext = NULL;
					break;
				}
			}
			pMultipart = pMultipart->pNext;
		}
	} else {
		if (pMmsMsg->msgBody.size > 0)
			pMmsMsg->nPartCount++;
	}

	//call before processing urgent event.
	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();

	return true;

__CATCH:

	_MmsInitHeader();
	_MmsUnregisterDecodeBuffer();

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	_MsgFreeBody(&pMmsMsg->msgBody, pMmsMsg->msgType.type);
	MSG_DEBUG("_MmsReadMsgBody:    E  N  D    ( fail )    ******************** \n");

	return false;
}

#ifdef MMS_DELIEVERY_IND_ENABLED
MmsMsgMultiStatus *MmsComposeDeliveryIndMessage(MmsMsg *pMmsMsg, msg_message_id_t msgId)
{
	MmsMsgMultiStatus *pStatus = NULL;
	MmsMsgMultiStatus *pLastStatus = NULL;
	bool bFound = false;

	MmsInitMsgAttrib(&pMmsMsg->mmsAttrib);
	MmsInitMsgType(&pMmsMsg->msgType);
	MmsInitMsgBody(&pMmsMsg->msgBody);

	pMmsMsg->mmsAttrib.version = mmsHeader.version;

	pMmsMsg->mmsAttrib.pMultiStatus = MmsGetMultiStatus(msgId);

	pStatus = pMmsMsg->mmsAttrib.pMultiStatus;

	MSG_DEBUG("### pStatus->szTo = %s ###", pStatus->szTo);

	while (pStatus && !bFound) {
		MSG_DEBUG("### MmsAddrUtilCompareAddr ###");
		MSG_DEBUG("### mmsHeader.pTo->szAddr = %s ###", mmsHeader.pTo->szAddr);
		if (MmsAddrUtilCompareAddr(pStatus->szTo, mmsHeader.pTo->szAddr)) {
			bFound = true;
			break;
		}

		pStatus = pStatus->pNext;
	}

	if (bFound == false) {
		MSG_DEBUG("### bFound == false ###");
		/* Queue the delivery report  --------------------------- */

		pStatus = (MmsMsgMultiStatus *)malloc(sizeof(MmsMsgMultiStatus));
		memset(pStatus, 0, sizeof(MmsMsgMultiStatus));

		pStatus->readStatus = MMS_READSTATUS_NONE;

		memset(pStatus->szTo, 0, MSG_ADDR_LEN + 1);
		strncpy(pStatus->szTo, mmsHeader.pTo->szAddr, MSG_ADDR_LEN);

		if (pMmsMsg->mmsAttrib.pMultiStatus == NULL) {
			/* first delivery report */
			pMmsMsg->mmsAttrib.pMultiStatus = pStatus;
		} else {
			pLastStatus = pMmsMsg->mmsAttrib.pMultiStatus;
			while (pLastStatus->pNext) {
				pLastStatus = pLastStatus->pNext;
			}

			pLastStatus->pNext = pStatus;
			pLastStatus = pStatus;
		}
	}

	pStatus->handledTime = mmsHeader.date;
	pStatus->msgStatus = mmsHeader.msgStatus;
	pStatus->bDeliveryReportIsRead = false;

	_MmsDataUpdateLastStatus(pMmsMsg);

	pStatus->bDeliveyrReportIsLast = true;

	return pStatus;
}
#endif

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

int MmsSearchMsgId(char *toNumber, char *szMsgID)
{
	MSG_BEGIN();

	int msgId;

	msgId = MmsPluginStorage::instance()->searchMsgId(toNumber, szMsgID);

	if (msgId < 0)
		return 0;

	MSG_END();

	return msgId;
}

MmsMsgMultiStatus *MmsGetMultiStatus(msg_message_id_t msgId)
{
	MmsMsgMultiStatus *pMultiStatus;

	pMultiStatus = MmsPluginStorage::instance()->getMultiStatus(msgId);

	MSG_DEBUG("### szTo = %s ###", pMultiStatus->szTo);

	MSG_END();

	return pMultiStatus;
}

bool MmsCheckAdditionalMedia(MMS_MESSAGE_DATA_S *pMsgData, MsgType *partHeader)
{
	if (_MsgMmsFindMatchedMedia(pMsgData, partHeader->param.szFileName))
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


#ifdef __SUPPORT_DRM__
bool __MsgInitMsgDRMInfo(MsgDRMInfo *pMsgDrmInfo)
{
	pMsgDrmInfo->contentType = MIME_UNKNOWN;
	pMsgDrmInfo->drmType = MSG_DRM_TYPE_NONE;

	pMsgDrmInfo->szContentName = NULL;
	pMsgDrmInfo->szContentURI = NULL;
	pMsgDrmInfo->szContentDescription = NULL;
	pMsgDrmInfo->szContentVendor = NULL;
	pMsgDrmInfo->szRightIssuer = NULL;
	pMsgDrmInfo->szDrm2FullPath = NULL;
	pMsgDrmInfo->roWaitingTimerMax = 0;
	pMsgDrmInfo->bFwdLock = false;
	pMsgDrmInfo->bNoScreen = false;
	pMsgDrmInfo->bNoRingTone = false;
	pMsgDrmInfo->pszContentType = NULL;

	return true;
}
#endif

msg_error_t MmsMakePreviewInfo(int msgId, MMS_MESSAGE_DATA_S *pMmsMsg)
{
	MMS_PAGE_S *pPage = NULL;
	MMS_MEDIA_S *pMedia = NULL;

	if (pMmsMsg == NULL)
		return MSG_ERR_NULL_POINTER;

	if (pMmsMsg->pageCnt > 0) {

		MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_PAGE, (char *)"pagecount", pMmsMsg->pageCnt);

		pPage = _MsgMmsGetPage(pMmsMsg, 0);
		for (int j = 0; j < pPage->mediaCnt; j++) {

			pMedia = _MsgMmsGetMedia(pPage, j);
			MSG_DEBUG("pMedia's Name: %s", pMedia->szFilePath);

			if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG || pMedia->mediatype == MMS_SMIL_MEDIA_VIDEO) {
				char szFileName[MSG_FILENAME_LEN_MAX+1] = {0, };
				char thumbPath[MSG_FILEPATH_LEN_MAX+1] = {0, };

				memset(szFileName, 0x00, MSG_FILENAME_LEN_MAX+1);
				memset(thumbPath, 0x00, MSG_FILEPATH_LEN_MAX);

				snprintf(szFileName, MSG_FILENAME_LEN_MAX+1, "%d.mms",msgId);
				snprintf(thumbPath, MSG_FILEPATH_LEN_MAX, MSG_THUMBNAIL_PATH"/%s.jpg", szFileName);

				if (pMedia->mediatype == MMS_SMIL_MEDIA_IMG) {
					if (makeImageThumbnail(pMedia->szFilePath, thumbPath) == true) {
						MmsPluginStorage::instance()->insertPreviewInfo(msgId, MSG_MMS_ITEM_TYPE_IMG, thumbPath);
					} else {
						MSG_DEBUG("Fail of generating thumbnail: %s to %s", pMedia->szFilePath, thumbPath);
					}
				} else {
					if (makeVideoThumbnail(pMedia->szFilePath, thumbPath) == true) {;
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
