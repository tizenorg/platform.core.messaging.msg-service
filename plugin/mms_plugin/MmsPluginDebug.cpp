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
#include "MmsPluginMIME.h"
#include "MmsPluginMessage.h"

static char gszDebugStringBuf[50];

static char *MmsDebugPrintUnknownValue(int value)
{
	printf(gszDebugStringBuf, "unknown value(%d)", value);
	return gszDebugStringBuf;
}


const char *MmsDebugGetMimeType(MimeType mimeType)
{
	switch (mimeType) {
	case MIME_APPLICATION_XML:
		return "MIME_APPLICATION_XML";
	case MIME_APPLICATION_WML_XML:
		return "MIME_APPLICATION_WML_XML";
	case MIME_APPLICATION_XHTML_XML:
		return "MIME_APPLICATION_XHTML_XML";
	case MIME_APPLICATION_JAVA_VM:
		return "MIME_APPLICATION_JAVA_VM";
	case MIME_APPLICATION_SMIL:
		return "MIME_APPLICATION_SMIL";
	case MIME_APPLICATION_JAVA_ARCHIVE:
		return "MIME_APPLICATION_JAVA_ARCHIVE";
	case MIME_APPLICATION_JAVA:
		return "MIME_APPLICATION_JAVA";
	case MIME_APPLICATION_OCTET_STREAM:
		return "MIME_APPLICATION_OCTET_STREAM";
	case MIME_APPLICATION_STUDIOM:
		return "MIME_APPLICATION_STUDIOM";
	case MIME_APPLICATION_FUNMEDIA:
		return "MIME_APPLICATION_FUNMEDIA";
	case MIME_APPLICATION_MSWORD:
		return "MIME_APPLICATION_MSWORD";
	case MIME_APPLICATION_PDF:
		return "MIME_APPLICATION_PDF";
	case MIME_APPLICATION_ASTERIC:
		return "MIME_APPLICATION_ASTERIC";
	case MIME_APPLICATION_VND_WAP_XHTMLXML:
		return "MIME_APPLICATION_VND_WAP_XHTMLXML";
	case MIME_APPLICATION_VND_WAP_WMLC:
		return "MIME_APPLICATION_VND_WAP_WMLC";
	case MIME_APPLICATION_VND_WAP_WMLSCRIPTC:
		return "MIME_APPLICATION_VND_WAP_WMLSCRIPTC";
	case MIME_APPLICATION_VND_WAP_WTA_EVENTC:
		return "MIME_APPLICATION_VND_WAP_WTA_EVENTC";
	case MIME_APPLICATION_VND_WAP_UAPROF:
		return "MIME_APPLICATION_VND_WAP_UAPROF";
	case MIME_APPLICATION_VND_WAP_SIC:
		return "MIME_APPLICATION_VND_WAP_SIC";
	case MIME_APPLICATION_VND_WAP_SLC:
		return "MIME_APPLICATION_VND_WAP_SLC";
	case MIME_APPLICATION_VND_WAP_COC:
		return "MIME_APPLICATION_VND_WAP_COC";
	case MIME_APPLICATION_VND_WAP_SIA:
		return "MIME_APPLICATION_VND_WAP_SIA";
	case MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML:
		return "MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML";
	case MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA";
	case MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES";
	case MIME_APPLICATION_VND_WAP_MULTIPART_MIXED:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_MIXED";
	case MIME_APPLICATION_VND_WAP_MULTIPART_RELATED:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_RELATED";
	case MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE";
	case MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC:
		return "MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC";
	case MIME_APPLICATION_VND_OMA_DD_XML:
		return "MIME_APPLICATION_VND_OMA_DD_XML";
	case MIME_APPLICATION_VND_OMA_DRM_MESSAGE:
		return "MIME_APPLICATION_VND_OMA_DRM_MESSAGE";
	case MIME_APPLICATION_VND_OMA_DRM_CONTENT:
		return "MIME_APPLICATION_VND_OMA_DRM_CONTENT";
	case MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML:
		return "MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML";
	case MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML:
		return "MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML";
	case MIME_APPLICATION_VND_SMAF:
		return "MIME_APPLICATION_VND_SMAF";
	case MIME_APPLICATION_VND_RN_REALMEDIA:
		return "MIME_APPLICATION_VND_RN_REALMEDIA";
	case MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE:
		return "MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE";
	case MIME_APPLICATION_VND_EXCEL:
		return "MIME_APPLICATION_VND_EXCEL";
	case MIME_APPLICATION_X_HDMLC:
		return "MIME_APPLICATION_X_HDMLC";
	case MIME_APPLICATION_X_X968_USERCERT:
		return "MIME_APPLICATION_X_X968_USERCERT";
	case MIME_APPLICATION_X_WWW_FORM_URLENCODED:
		return "MIME_APPLICATION_X_WWW_FORM_URLENCODED";
	case MIME_APPLICATION_X_SMAF:
		return "MIME_APPLICATION_X_SMAF";
	case MIME_APPLICATION_X_FLASH:
		return "MIME_APPLICATION_X_FLASH";
	case MIME_APPLICATION_X_EXCEL:
		return "MIME_APPLICATION_X_EXCEL";
	case MIME_APPLICATION_X_POWERPOINT:
		return "MIME_APPLICATION_X_POWERPOINT";

	case MIME_AUDIO_BASIC:
		return "MIME_AUDIO_BASIC";
	case MIME_AUDIO_MPEG:
		return "MIME_AUDIO_MPEG";
	case MIME_AUDIO_MP3:
		return "MIME_AUDIO_MP3";
	case MIME_AUDIO_MPG3:
		return "MIME_AUDIO_MPG3";
	case MIME_AUDIO_MPEG3:
		return "MIME_AUDIO_MPEG3";
	case MIME_AUDIO_MPG:
		return "MIME_AUDIO_MPG";
	case MIME_AUDIO_AAC:
		return "MIME_AUDIO_AAC";
	case MIME_AUDIO_G72:
		return "MIME_AUDIO_G72";
	case MIME_AUDIO_AMR:
		return "MIME_AUDIO_AMR";
	case MIME_AUDIO_AMR_WB:
		return "MIME_AUDIO_AMR_WB";
	case MIME_AUDIO_MMF:
		return "MIME_AUDIO_MMF";
	case MIME_AUDIO_SMAF:
		return "MIME_AUDIO_SMAF";
	case MIME_AUDIO_IMELODY:
		return "MIME_AUDIO_IMELODY";
	case MIME_AUDIO_MELODY:
		return "MIME_AUDIO_MELODY";
	case MIME_AUDIO_MID:
		return "MIME_AUDIO_MID";
	case MIME_AUDIO_MIDI:
		return "MIME_AUDIO_MIDI";
	case MIME_AUDIO_X_MID:
		return "MIME_AUDIO_X_MID";
	case MIME_AUDIO_SP_MIDI:
		return "MIME_AUDIO_SP_MIDI";
	case MIME_AUDIO_WAVE:
		return "MIME_AUDIO_WAVE";
	case MIME_AUDIO_3GPP:
		return "MIME_AUDIO_3GPP";
	case MIME_AUDIO_MP4:
		return "MIME_AUDIO_MP4";
	case MIME_AUDIO_MP4A_LATM:
		return "MIME_AUDIO_MP4A_LATM";
	case MIME_AUDIO_VND_RN_REALAUDIO:
		return "MIME_AUDIO_VND_RN_REALAUDIO";
	case MIME_AUDIO_X_MPEG:
		return "MIME_AUDIO_X_MPEG";
	case MIME_AUDIO_X_MP3:
		return "MIME_AUDIO_X_MP3";
	case MIME_AUDIO_X_MPEG3:
		return "MIME_AUDIO_X_MPEG3";
	case MIME_AUDIO_X_MPG:
		return "MIME_AUDIO_X_MPG";
	case MIME_AUDIO_X_AMR:
		return "MIME_AUDIO_X_AMR";
	case MIME_AUDIO_X_MMF:
		return "MIME_AUDIO_X_MMF";
	case MIME_AUDIO_X_SMAF:
		return "MIME_AUDIO_X_SMAF";
	case MIME_AUDIO_X_IMELODY:
		return "MIME_AUDIO_X_IMELODY";
	case MIME_AUDIO_X_MIDI:
		return "MIME_AUDIO_X_MIDI";
	case MIME_AUDIO_X_MPEGAUDIO:
		return "MIME_AUDIO_X_MPEGAUDIO";
	case MIME_AUDIO_X_PN_REALAUDIO:
		return "MIME_AUDIO_X_PN_REALAUDIO";
	case MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO:
		return "MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO";
	case MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE:
		return "MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE";
	case MIME_AUDIO_X_WAV:
		return "MIME_AUDIO_X_WAV";

	case MIME_IMAGE_GIF:
		return "MIME_IMAGE_GIF";
	case MIME_IMAGE_JPEG:
		return "MIME_IMAGE_JPEG";
	case MIME_IMAGE_JPG:
		return "MIME_IMAGE_JPG";
	case MIME_IMAGE_TIFF:
		return "MIME_IMAGE_TIFF";
	case MIME_IMAGE_TIF:
		return "MIME_IMAGE_TIF";
	case MIME_IMAGE_PNG:
		return "MIME_IMAGE_PNG";
	case MIME_IMAGE_WBMP:
		return "MIME_IMAGE_WBMP";
	case MIME_IMAGE_PJPEG:
		return "MIME_IMAGE_PJPEG";
	case MIME_IMAGE_BMP:
		return "MIME_IMAGE_BMP";
	case MIME_IMAGE_SVG:
		return "MIME_IMAGE_SVG";
	case MIME_IMAGE_SVG1:
		return "MIME_IMAGE_SVG1";
	case MIME_IMAGE_VND_WAP_WBMP:
		return "MIME_IMAGE_VND_WAP_WBMP";

	case MIME_IMAGE_X_BMP:
		return "MIME_IMAGE_X_BMP";

	case MIME_MESSAGE_RFC822:
		return "MIME_MESSAGE_RFC822";

	case MIME_MULTIPART_MIXED:
		return "MIME_MULTIPART_MIXED";
	case MIME_MULTIPART_RELATED:
		return "MIME_MULTIPART_RELATED";
	case MIME_MULTIPART_ALTERNATIVE:
		return "MIME_MULTIPART_ALTERNATIVE";
	case MIME_MULTIPART_FORM_DATA:
		return "MIME_MULTIPART_FORM_DATA";
	case MIME_MULTIPART_BYTERANGE:
		return "MIME_MULTIPART_BYTERANGE";
	case MIME_MULTIPART_REPORT:
		return "MIME_MULTIPART_REPORT";

	case MIME_TEXT_TXT:
		return "MIME_TEXT_TXT";
	case MIME_TEXT_HTML:
		return "MIME_TEXT_HTML";
	case MIME_TEXT_PLAIN:
		return "MIME_TEXT_PLAIN";
	case MIME_TEXT_CSS:
		return "MIME_TEXT_CSS";
	case MIME_TEXT_XML:
		return "MIME_TEXT_XML";
	case MIME_TEXT_IMELODY:
		return "MIME_TEXT_IMELODY";
	case MIME_TEXT_VND_WAP_WMLSCRIPT:
		return "MIME_TEXT_VND_WAP_WMLSCRIPT";
	case MIME_TEXT_VND_WAP_WML:
		return "MIME_TEXT_VND_WAP_WML";
	case MIME_TEXT_VND_WAP_WTA_EVENT:
		return "MIME_TEXT_VND_WAP_WTA_EVENT";
	case MIME_TEXT_VND_WAP_CONNECTIVITY_XML:
		return "MIME_TEXT_VND_WAP_CONNECTIVITY_XML";
	case MIME_TEXT_VND_WAP_SI:
		return "MIME_TEXT_VND_WAP_SI";
	case MIME_TEXT_VND_WAP_SL:
		return "MIME_TEXT_VND_WAP_SL";
	case MIME_TEXT_VND_WAP_CO:
		return "MIME_TEXT_VND_WAP_CO";
	case MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR:
		return "MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR";
	case MIME_TEXT_X_HDML:
		return "MIME_TEXT_X_HDML";
	case MIME_TEXT_X_VCALENDAR:
		return "MIME_TEXT_X_VCALENDAR";
	case MIME_TEXT_X_VCARD:
		return "MIME_TEXT_X_VCARD";
	case MIME_TEXT_X_IMELODY:
		return "MIME_TEXT_X_IMELODY";

	case MIME_VIDEO_MPEG4:
		return "MIME_VIDEO_MPEG4";
	case MIME_VIDEO_MP4:
		return "MIME_VIDEO_MP4";
	case MIME_VIDEO_H263:
		return "MIME_VIDEO_H263";
	case MIME_VIDEO_3GPP:
		return "MIME_VIDEO_3GPP";
	case MIME_VIDEO_3GP:
		return "MIME_VIDEO_3GP";
	case MIME_VIDEO_AVI:
		return "MIME_VIDEO_AVI";
	case MIME_VIDEO_SDP:
		return "MIME_VIDEO_SDP";
	case MIME_VIDEO_VND_RN_REALVIDEO:
		return "MIME_VIDEO_VND_RN_REALVIDEO";
	case MIME_VIDEO_X_MP4:
		return "MIME_VIDEO_X_MP4";
	case MIME_VIDEO_X_PV_MP4:
		return "MIME_VIDEO_X_PV_MP4";
	case MIME_VIDEO_X_PN_REALVIDEO:
		return "MIME_VIDEO_X_PN_REALVIDEO";
	case MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO:
		return "MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO";
	default:
		return MmsDebugPrintUnknownValue(mimeType);
	}
}


/* MsgMmsMsg.h */
const char *MmsDebugGetMmsReport(MmsReport report)
{
	switch (report) {
	case MMS_REPORT_ERROR:
		return "MMS_REPORT_ERROR";
	case MMS_REPORT_YES:
		return "MMS_REPORT_YES";
	case MMS_REPORT_NO:
		return "MMS_REPORT_NO";
	}

	return MmsDebugPrintUnknownValue(report);
}


const char *MmsDebugGetMmsReportAllowed(MmsReportAllowed reportAllowed)
{
	switch (reportAllowed) {
	case MMS_REPORTALLOWED_ERROR:
		return "MMS_REPORTALLOWED_ERROR";
	case MMS_REPORTALLOWED_YES:
		return "MMS_REPORTALLOWED_YES";
	case MMS_REPORTALLOWED_NO:
		return "MMS_REPORTALLOWED_NO";
	}

	return MmsDebugPrintUnknownValue(reportAllowed);
}


const char *MmsDebugGetMmsReadStatus(msg_read_report_status_t readStatus)
{
	_MSG_READ_REPORT_STATUS_E readReport = (_MSG_READ_REPORT_STATUS_E)readStatus;

	switch (readReport) {
	case MSG_READ_REPORT_NONE:
		return "MMS_READSTATUS_NONE";
	case MSG_READ_REPORT_IS_READ:
		return "MMS_IS_READ";
	case MSG_READ_REPORT_IS_DELETED:
		return "MMS_IS_DELETED";
	}

	return MmsDebugPrintUnknownValue(readStatus);
}

const char *MmsDebugGetMsgType(MmsMsgType msgType)
{
	switch (msgType) {
	case MMS_MSGTYPE_ERROR:
		return "error";
	case MMS_MSGTYPE_SEND_REQ:
		return "send.req";
	case MMS_MSGTYPE_SEND_CONF:
		return "send.conf";
	case MMS_MSGTYPE_NOTIFICATION_IND:
		return "notification.ind";
	case MMS_MSGTYPE_NOTIFYRESP_IND:
		return "notifyResp.ind";
	case MMS_MSGTYPE_RETRIEVE_CONF:
		return "retrieve conf";
	case MMS_MSGTYPE_ACKNOWLEDGE_IND:
		return "acknowledge ind";
	case MMS_MSGTYPE_DELIVERY_IND:
		return "delivery ind";
	case MMS_MSGTYPE_READREC_IND:
		return "read rec ind";
	case MMS_MSGTYPE_READORG_IND:
		return "read org ind";
	case MMS_MSGTYPE_FORWARD_REQ:
		return "forward req";
	case MMS_MSGTYPE_FORWARD_CONF:
		return "forward conf";
	case MMS_MSGTYPE_READ_REPLY:
		return "read reply";
	default:
		return MmsDebugPrintUnknownValue(msgType);
	}
}

const char *MmsDebugGetResponseStatus(MmsResponseStatus responseStatus)
{
	switch (responseStatus) {
	case MMS_RESPSTATUS_ERROR:
		return "error";
	case MMS_RESPSTATUS_OK:
		return "ok";
	case MMS_RESPSTAUTS_ERROR_UNSPECIFIED:
		return "unspecified";
	case MMS_RESPSTAUTS_ERROR_SERVICEDENIED:
		return "service denied";
	case MMS_RESPSTAUTS_ERROR_MESSAGEFORMATCORRUPT:
		return "message format corrupt";
	case MMS_RESPSTAUTS_ERROR_SENDINGADDRESSUNRESOLVED:
		return "sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_MESSAGENOTFOUND:
		return "message not found";
	case MMS_RESPSTAUTS_ERROR_NETWORKPROBLEM:
		return "network problem";
	case MMS_RESPSTAUTS_ERROR_CONTENTNOTACCEPTED:
		return "content not accepted";
	case MMS_RESPSTAUTS_ERROR_UNSUPPORTEDMESSAGE:
		return "unsupported message";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_FAILURE:
		return "transient failure";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_SENDING_ADDRESS_UNRESOLVED:
		return "transient sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_MESSAGE_NOT_FOUND:
		return "transient message not found";
	case MMS_RESPSTAUTS_ERROR_TRANSIENT_NETWORK_PROBLEM:
		return "transient network problem";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_FAILURE:
		return "permanent failure";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_SERVICE_DENIED:
		return "permanent service denied";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_FORMAT_CORRUPT:
		return "permanent message format corrupt";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_SENDING_ADDRESS_UNRESOLVED:
		return "permanent sending address unresolved";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_MESSAGE_NOT_FOUND:
		return "permanent message not found";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_CONTENT_NOT_ACCEPTED:
		return "permanent content not accepted";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_LIMITATIONS_NOT_MET:
		return "permanent reply charging limitations not met";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_REQUEST_NOT_ACCEPTED:
		return "permanent reply charging request not accepted";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_FORWARDING_DENIED:
		return "permanent reply charging forwarding denied";
	case MMS_RESPSTAUTS_ERROR_PERMANENT_REPLY_CHARGING_NOT_SUPPORTED:
		return "permanent reply charging not supported";
	}

	return MmsDebugPrintUnknownValue(responseStatus);
}


const char *MmsDebugGetRetrieveStatus(MmsRetrieveStatus retrieveStatus)
{
	switch (retrieveStatus) {
	case MMS_RETRSTATUS_ERROR:
		return "error";
	case MMS_RETRSTATUS_OK:
		return "ok";
	case MMS_RETRSTATUS_TRANSIENT_FAILURE:
		return "transient failure";
	case MMS_RETRSTATUS_TRANSIENT_MESSAGE_NOT_FOUND:
		return "transient message not found";
	case MMS_RETRSTATUS_TRANSIENT_NETWORK_PROBLEM:
		return "transient network problem";
	case MMS_RETRSTATUS_PERMANENT_FAILURE:
		return "permanent failure";
	case MMS_RETRSTATUS_PERMANENT_SERVICE_DENIED:
		return "permanent service denied";
	case MMS_RETRSTATUS_PERMANENT_MESSAGE_NOT_FOUND:
		return "permanent message not found";
	case MMS_RETRSTATUS_PERMANENT_CONTENT_UNSUPPORT:
		return "permanent content unsupport";
	}

	return MmsDebugPrintUnknownValue(retrieveStatus);
}


const char *MmsDebugGetMsgStatus(msg_delivery_report_status_t msgStatus)
{
	switch (msgStatus) {
	case MSG_DELIVERY_REPORT_ERROR:
		return "error";
	case MSG_DELIVERY_REPORT_EXPIRED:
		return "expired";
	case MSG_DELIVERY_REPORT_SUCCESS:
		return "retrieved";
	case MSG_DELIVERY_REPORT_REJECTED:
		return "rejected";
	case MSG_DELIVERY_REPORT_DEFERRED:
		return "deferred";
	case MSG_DELIVERY_REPORT_UNRECOGNISED:
		return "unrecognised";
	case MSG_DELIVERY_REPORT_INDETERMINATE:
		return "indeterminate";
	case MSG_DELIVERY_REPORT_FORWARDED:
		return "forwarded";
	case MSG_DELIVERY_REPORT_UNREACHABLE:
		return "unreachable";
	}

	return MmsDebugPrintUnknownValue(msgStatus);
}


const char *MmsDebugGetMsgClass(MmsMsgClass msgClass)
{
	switch (msgClass) {
	case MMS_MSGCLASS_ERROR:
		return "error";
	case MMS_MSGCLASS_PERSONAL:
		return "personal";
	case MMS_MSGCLASS_ADVERTISEMENT:
		return "advertisement";
	case MMS_MSGCLASS_INFORMATIONAL:
		return "information";
	case MMS_MSGCLASS_AUTO:
		return "auto";
	}

	return MmsDebugPrintUnknownValue(msgClass);
}


const char *MmsDebugGetDataType(MmsDataType dataType)
{
	switch (dataType) {
	case MMS_DATATYPE_NONE:
		return "MMS_DATATYPE_NONE";
	case MMS_DATATYPE_READ:
		return "MMS_DATATYPE_READ";
	case MMS_DATATYPE_SENT:
		return "MMS_DATATYPE_SENT";
	case MMS_DATATYPE_NOTIFY:
		return "MMS_DATATYPE_NOTIFY";
	case MMS_DATATYPE_UNSENT:
		return "MMS_DATATYPE_UNSENT";
	case MMS_DATATYPE_DRAFT:
		return "MMS_DATATYPE_DRAFT";
	case MMS_DATATYPE_SENDING:
		return "MMS_DATATYPE_SENDING";
	case MMS_DATATYPE_DRM_RO_WAITING:
		return "MMS_DATATYPE_DRM_RO_WAITING";
	case MMS_DATATYPE_RETRIEVING:
		return "MMS_DATATYPE_RETRIEVING";
	case MMS_DATATYPE_UNRETV:
		return "MMS_DATATYPE_UNRETV";
	default:
		return MmsDebugPrintUnknownValue(dataType);
	}
}

#ifdef MMS_PLUGIN_DEBUG_ENABLE
bool
MmsDebugPrintMsgAttributes(char *pszFunc, MmsAttrib *pAttrib, bool bAll)
{
	SysRequireEx(pAttrib != NULL, false);

	if (pszFunc)
		SysDebug((MID_MMS|DBG_MMS_COMMON,"%s ========= \n", pszFunc));

	SysDebug((MID_MMS|DBG_MMS_COMMON, "szSubject = %s \n", pAttrib->szSubject));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "contentType=%s \n", MmsDebugGetMimeType(pAttrib->contentType)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "dataType=%s \n", MmsDebugGetDataType(pAttrib->dataType)));

	if (pAttrib->specialMsgType != MMS_SPECIAL_MSG_TYPE_NONE)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "spcialMsgType = %lu \n", pAttrib->specialMsgType));

	SysDebug((MID_MMS|DBG_MMS_COMMON, "date=%lu, msgSize=%lu, bRead=%lu\n", pAttrib->date, pAttrib->msgSize, pAttrib->bRead));

	if (!bAll)
		return true;

	SysDebug((MID_MMS|DBG_MMS_COMMON, "msgClass=%s, priority=%lu \n", MmsDebugGetMsgClass(pAttrib->msgClass), pAttrib->priority ));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "deliveryTime.type = %lu, time = %lu\n", pAttrib->deliveryTime.type, pAttrib->deliveryTime.time));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "expiryTime.type = %lu, time = %lu\n", pAttrib->expiryTime.type, pAttrib->expiryTime.time));

	if (pAttrib->szTo && pAttrib->szTo[0] != 0)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "szTo = %s \n", pAttrib->szTo));
	if (pAttrib->szCc && pAttrib->szCc[0] != 0)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "szCc = %s \n", pAttrib->szCc));
	if (pAttrib->szBcc && pAttrib->szBcc[0] != 0)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "szBcc = %s \n", pAttrib->szBcc));

#if defined(_MMS_SUPPORT_RECEIVING_OPTION_PROMPT)
	SysDebug((MID_MMS|DBG_MMS_COMMON, "readReportAllowedType = %lu \n", pAttrib->readReportAllowedType));
#endif
	SysDebug((MID_MMS|DBG_MMS_COMMON, "bAskDeliveryReport=%lu, bReportAllowed=%lu, bAskReadReply=%lu, bLeaveCopy=%lu \n",
			pAttrib->bAskDeliveryReport, pAttrib->bReportAllowed, pAttrib->bAskReadReply, pAttrib->bLeaveCopy));

	if (pAttrib->bHideAddress)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "bHideAddress=true\n"));

	SysDebug((MID_MMS|DBG_MMS_COMMON, "msgStatus = %s \n", MmsDebugGetMsgStatus(pAttrib->msgStatus)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "responseStatus = %s \n", MmsDebugGetResponseStatus(pAttrib->responseStatus)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "retrieveStatus = %s \n", MmsDebugGetRetrieveStatus(pAttrib->retrieveStatus)));

	if (pAttrib->szResponseText && pAttrib->szResponseText[0] != 0)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "szResponseText = %s \n", pAttrib->szResponseText));

	if (pAttrib->szRetrieveText && pAttrib->szRetrieveText[0] != 0)
		SysDebug((MID_MMS|DBG_MMS_COMMON, "szRetrieveText = %s \n", pAttrib->szRetrieveText));

	return true;
}

char *MmsDebugGetMsgDrmType(MsgDrmType drmType)
{
	switch (drmType) {
	case MSG_DRM_TYPE_NONE:
		return "MSG_DRM_TYPE_NONE";
	case MSG_DRM_TYPE_FL:
		return "MSG_DRM_TYPE_FL";
	case MSG_DRM_TYPE_CD:
		return "MSG_DRM_TYPE_CD";
	case MSG_DRM_TYPE_SD:
		return "MSG_DRM_TYPE_SD";
	case MSG_DRM_TYPE_SSD:
		return "MSG_DRM_TYPE_SSD";
	}

	return MmsDebugPrintUnknownValue(drmType);
}

char *MmsDebugGetDrmDeliveryMode(DrmDeliveryMode deliveryMode)
{
	switch (deliveryMode) {
	case DRM_DELIVERYMODE_FORWARD_LOCK:
		return "DRM_DELIVERYMODE_FORWARD_LOCK";
	case DRM_DELIVERYMODE_COMBINED_DELIVERY:
		return "DRM_DELIVERYMODE_COMBINED_DELIVERY";
	case DRM_DELIVERYMODE_SEPARATE_DELIVERY:
		return "DRM_DELIVERYMODE_SEPARATE_DELIVERY";
	case DRM_DELIVERYMODE_SPECIAL_SEPARATE:
		return "DRM_DELIVERYMODE_SPECIAL_SEPARATE";
	}

	return MmsDebugPrintUnknownValue(deliveryMode);
}

char *MmsDebugGetDrmRightState(DrmRightState rightState)
{
	switch (rightState) {
	case DRMRIGHT_STATE_NORIGHTS:
		return "DRMRIGHT_STATE_NORIGHTS";
	case DRMRIGHT_STATE_INVALID_RIGHTS:
		return "DRMRIGHT_STATE_INVALID_RIGHTS";
	case DRMRIGHT_STATE_VALID_RIGHTS:
		return "DRMRIGHT_STATE_VALID_RIGHTS";
	case DRMRIGHT_STATE_EXPIRED_RIGHTS:
		return "DRMRIGHT_STATE_EXPIRED_RIGHTS";
	}

	return MmsDebugPrintUnknownValue(rightState);
}


bool MmsDebugPrintDrmRight(DrmRight *pDrmRight)
{
	if (!pDrmRight)
		return true;

	if (pDrmRight->rightStatus == DRMRIGHT_STATE_NORIGHTS) {
		SysDebug((MID_MMS|DBG_MMS_COMMON, "rightStatus = %s \n", MmsDebugGetDrmRightState(pDrmRight->rightStatus)));
		return true;
	}

	SysDebug((MID_MMS|DBG_MMS_COMMON, "iDeliveryType = %s \n", MmsDebugGetDrmDeliveryMode(pDrmRight->iDeliveryType)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "bestConstraint = %s \n", MmsDebugGetDrmConsumeMode(pDrmRight->bestConstraint)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "bFirstIntervalRender = %d \n", pDrmRight->bFirstIntervalRender));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "rightStatus = %s \n", MmsDebugGetDrmRightState(pDrmRight->rightStatus)));

	SysDebug((MID_MMS|DBG_MMS_COMMON, "bValidIssuer = %d \n", pDrmRight->bValidIssuer));

	return true;
}

char *MmsDebugPrintMsgDRMStatus(MsgDRMStatus status)
{
	switch (status) {
	case MSG_DRM_STATUS_INVALID:
		return "MSG_DRM_STATUS_INVALID";
	case MSG_DRM_STATUS_VALID:
		return "MSG_DRM_STATUS_VALID";
	case MSG_DRM_STATUS_EXPIRED:
		return "MSG_DRM_STATUS_EXPIRED";
	}

	return MmsDebugPrintUnknownValue(status);
}


bool MmsDebugPrintMulitpartEntry(MsgMultipart *pMultipart, int index)
{
	SysDebug((MID_MMS|DBG_MMS_COMMON, "------------------------------\n"));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "%dth multipart info\n", index));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "header size=%d\n", pMultipart->type.size));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "body size=%d\n", pMultipart->type.contentSize));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "content type=%s\n", MmsDebugGetMimeType(pMultipart->type.type)));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "content ID=%s\n", pMultipart->type.szContentID));
	SysDebug((MID_MMS|DBG_MMS_COMMON, "content location=%s\n", pMultipart->type.szContentLocation));

	if (pMultipart->type.type == MIME_TEXT_PLAIN) {
		SysDebug((MID_MMS|DBG_MMS_COMMON, "text info\n"));
		SysDebug((MID_MMS|DBG_MMS_COMMON, "charset=%d\n", pMultipart->type.param.charset));
		SysDebug((MID_MMS|DBG_MMS_COMMON, "text file name=%s\n", pMultipart->type.param.szName));
	}

	if (pMultipart->type.drmInfo.drmType != MSG_DRM_TYPE_NONE) {
		SysDebug((MID_MMS|DBG_MMS_COMMON, "drm info\n"));
		SysDebug((MID_MMS|DBG_MMS_COMMON, "drm type=%s\n", MmsDebugGetMsgDrmType(pMultipart->type.drmInfo.drmType)));
		SysDebug((MID_MMS|DBG_MMS_COMMON, "drm content type=%s\n", MmsDebugGetMimeType(pMultipart->type.drmInfo.contentType)));
		SysDebug((MID_MMS|DBG_MMS_COMMON, "drm content URI=%s\n", pMultipart->type.drmInfo.szContentURI));
	}
	SysDebug((MID_MMS|DBG_MMS_COMMON, "------------------------------\n"));
	return true;
}


bool MmsDebugPrintCurrentEventHandler(char *pszFunc, MmsDebugEvType evType)
{
	HEventHandler hMmsEH;
	HEventHandler hSmsEH;
	HEventHandler hEmailEH;
	HEventHandler hCommonEH;
	HEventHandler hCurrentEH;

	hMmsEH		= WmGetEventHandlerByName(EHN_MMS);
	hSmsEH		= WmGetEventHandlerByName(EHN_SMS);
	hEmailEH	= WmGetEventHandlerByName(EHN_EMAIL);
	hCommonEH	= WmGetEventHandlerByName(EHN_MESSENGER);
	hCurrentEH	= WmGetCurrentEventHandler();

	switch (evType) {
	case MMS_DEBUG_EV_MMS:
		SysRequireExf(hCurrentEH == hMmsEH, false, ("%s, hMmsEH=%d, hSmsEH=%d, hEmailEH=%d, hCommonEH=%d, hCurrentEH=%d\n",	\
							pszFunc, hMmsEH, hSmsEH, hEmailEH, hCommonEH, hCurrentEH));
		break;

	case MMS_DEBUG_EV_SMS:
		SysRequireExf(hCurrentEH == hSmsEH, false, ("%s, hMmsEH=%d, hSmsEH=%d, hEmailEH=%d, hCommonEH=%d, hCurrentEH=%d\n",	\
							pszFunc, hMmsEH, hSmsEH, hEmailEH, hCommonEH, hCurrentEH));
		break;

	case MMS_DEBUG_EV_COMMON:
		SysRequireExf(hCurrentEH == hCommonEH, false, ("%s, hMmsEH=%d, hSmsEH=%d, hEmailEH=%d, hCommonEH=%d, hCurrentEH=%d\n",	\
							pszFunc, hMmsEH, hSmsEH, hEmailEH, hCommonEH, hCurrentEH));
		break;

	case MMS_DEBUG_EV_EMAIL:
		SysRequireExf(hCurrentEH == hEmailEH, false, ("%s, hMmsEH=%d, hSmsEH=%d, hEmailEH=%d, hCommonEH=%d, hCurrentEH=%d\n",	\
							pszFunc, hMmsEH, hSmsEH, hEmailEH, hCommonEH, hCurrentEH));
		break;

	case MMS_DEBUG_EV_NONE:
	default:
		SysDebug((MID_MMS,"%s, hMmsEH=%d, hSmsEH=%d, hEmailEH=%d, hCommonEH=%d, hCurrentEH=%d\n",	\
							pszFunc, hMmsEH, hSmsEH, hEmailEH, hCommonEH, hCurrentEH));
		break;
	}
	return true;
}


char *DebugPrintGetRmResultInd(MmsRmResultInd indType)
{
	switch (indType) {
	case RM_RESULTIND_NONE:
		return "RM_RESULTIND_NONE";

	case RM_RESULTIND_MANUAL_SEND:
		return "RM_RESULTIND_MANUAL_SEND";

	case RM_RESULTIND_AUTO_SEND:
		return "RM_RESULTIND_AUTO_SEND";

	case RM_RESULTIND_MANUAL_GET:
		return "RM_RESULTIND_MANUAL_GET";

	case RM_RESULTIND_AUTO_GET:
		return "RM_RESULTIND_AUTO_GET";

	case RM_RESULTIND_MANUAL_FORWARD:
		return "RM_RESULTIND_MANUAL_FORWARD";
	}

	return MmsDebugPrintUnknownValue(indType);
}


char *DebugPrintHttpStatusCode(int status)
{
	switch (status) {
	case HTTP_STATUS_100:
		return "HTTP_STATUS_100";
	case HTTP_STATUS_101:
		return "HTTP_STATUS_101";

	case HTTP_STATUS_200:
		return "HTTP_STATUS_200";
	case HTTP_STATUS_201:
		return "HTTP_STATUS_201";
	case HTTP_STATUS_202:
		return "HTTP_STATUS_202";
	case HTTP_STATUS_203:
		return "HTTP_STATUS_203";
	case HTTP_STATUS_204:
		return "HTTP_STATUS_204";
	case HTTP_STATUS_205:
		return "HTTP_STATUS_205";
	case HTTP_STATUS_206:
		return "HTTP_STATUS_206";

	case HTTP_STATUS_300:
		return "HTTP_STATUS_300";
	case HTTP_STATUS_301:
		return "HTTP_STATUS_301";
	case HTTP_STATUS_302:
		return "HTTP_STATUS_302";
	case HTTP_STATUS_303:
		return "HTTP_STATUS_303";
	case HTTP_STATUS_304:
		return "HTTP_STATUS_304";
	case HTTP_STATUS_305:
		return "HTTP_STATUS_305";

	case HTTP_STATUS_400:
		return "HTTP_STATUS_400";
	case HTTP_STATUS_401:
		return "HTTP_STATUS_401";
	case HTTP_STATUS_402:
		return "HTTP_STATUS_402";
	case HTTP_STATUS_403:
		return "HTTP_STATUS_403";
	case HTTP_STATUS_404:
		return "HTTP_STATUS_404";
	case HTTP_STATUS_405:
		return "HTTP_STATUS_405";
	case HTTP_STATUS_406:
		return "HTTP_STATUS_406";
	case HTTP_STATUS_407:
		return "HTTP_STATUS_407";
	case HTTP_STATUS_408:
		return "HTTP_STATUS_408";
	case HTTP_STATUS_409:
		return "HTTP_STATUS_409";
	case HTTP_STATUS_410:
		return "HTTP_STATUS_410";
	case HTTP_STATUS_411:
		return "HTTP_STATUS_411";
	case HTTP_STATUS_412:
		return "HTTP_STATUS_412";
	case HTTP_STATUS_413:
		return "HTTP_STATUS_413";
	case HTTP_STATUS_414:
		return "HTTP_STATUS_414";
	case HTTP_STATUS_415:
		return "HTTP_STATUS_415";

	case HTTP_STATUS_500:
		return "HTTP_STATUS_500";
	case HTTP_STATUS_501:
		return "HTTP_STATUS_501";
	case HTTP_STATUS_502:
		return "HTTP_STATUS_502";
	case HTTP_STATUS_503:
		return "HTTP_STATUS_503";
	case HTTP_STATUS_504:
		return "HTTP_STATUS_504";
	case HTTP_STATUS_505:
		return "HTTP_STATUS_505";

	case REASON_WTP_UNKNOWN:
		return "REASON_WTP_UNKNOWN";
	case REASON_WTP_PROTO_ERR:
		return "REASON_WTP_PROTO_ERR";
	case REASON_WTP_INVALID_TID:
		return "REASON_WTP_INVALID_TID";
	case REASON_WTP_NOT_IMPLEMENTED_CL2:
		return "REASON_WTP_NOT_IMPLEMENTED_CL2";
	case REASON_WTP_NOT_IMPLEMENTED_SAR:
		return "REASON_WTP_NOT_IMPLEMENTED_SAR";
	case REASON_WTP_NOT_IMPLEMENTEDU_ACK:
		return "REASON_WTP_NOT_IMPLEMENTEDU_ACK";
	case REASON_WTP_VERSIONONE:
		return "REASON_WTP_VERSIONONE";
	case REASON_WTP_CAPTEMP_EXCEED:
		return "REASON_WTP_CAPTEMP_EXCEED";
	case REASON_WTP_NO_RESPONSE:
		return "REASON_WTP_NO_RESPONSE";
	case REASON_WTP_MESSAGE_TOO_LARGE:
		return "REASON_WTP_MESSAGE_TOO_LARGE";

	case REASON_PROTOERR:
		return "REASON_PROTOERR";	//  = 0xE0
	case REASON_DISCONNECT:
		return "REASON_DISCONNECT";
	case REASON_SUSPEND:
		return "REASON_SUSPEND";
	case REASON_RESUME:
		return "REASON_RESUME";
	case REASON_CONGESTION:
		return "REASON_CONGESTION";
	case REASON_CONNECTERR:
		return "REASON_CONNECTERR";
	case REASON_MRUEXCEEDED:
		return "REASON_MRUEXCEEDED";
	case REASON_MOREXCEEDED:
		return "REASON_MOREXCEEDED";
	case REASON_PEERREQ:
		return "REASON_PEERREQ";
	case REASON_NETERR:
		return "REASON_NETERR";
	case REASON_USERREQ:
		return "REASON_USERREQ";
	/* added for ver 1.2 */
	case REASON_USERRFS:
		return "REASON_USERRFS";
	case REASON_PND:
		return "REASON_PND";
	case REASON_USERDCR:
		return "REASON_USERDCR";
	case REASON_USERDCU:
		return "REASON_USERDCU";
	}

	return MmsDebugPrintUnknownValue(status);
}


char *DebugPrintRmMethodType(MmsRmMethodType method)
{
	switch (method) {
	case RM_METHOD_NONE:
		return "RM_METHOD_NONE";
	case RM_METHOD_GET:
		return "RM_METHOD_GET";
	case RM_METHOD_POST:
		return "RM_METHOD_POST";
	}

	return MmsDebugPrintUnknownValue(method);
}


char *DebugPrintGetMmsRmNetState(MmsRmNetState state)
{
	switch (state) {
	case RM_PROTO_IDLE:
		return "RM_PROTO_IDLE";
	case RM_PROTO_STARTING:
		return "RM_PROTO_STARTING";
	case RM_PROTO_STARTED:
		return "RM_PROTO_STARTED";
	case RM_PROTO_STOPPING:
		return "RM_PROTO_STOPPING";

	case RM_PROTO_WAITING:
		return "RM_PROTO_WAITING";
	case RM_PROTO_DISCONNECTING_OTHER_APP:
		return "RM_PROTO_DISCONNECTING_OTHER_APP";
	case RM_PROTO_DISCONNECTED_BY_OTHER_APP:
		return "RM_PROTO_DISCONNECTED_BY_OTHER_APP";

	case RM_PROTO_FAILED:
		return "RM_PROTO_FAILED";
	}

	return MmsDebugPrintUnknownValue(state);
}


char *DebugPrintGetMmsRmEntityState(MmsRmExEntityState stateEx)
{
	switch (stateEx) {
	case RM_ENTITY_IDLE:
		return "RM_ENTITY_IDLE";
	case RM_ENTITY_PROTO_STARTING:
		return "RM_ENTITY_PROTO_STARTING";

	case RM_ENTITY_CNXN_ESTABLISHING:
		return "RM_ENTITY_CNXN_ESTABLISHING";
	case RM_ENTITY_CNXN_ESTABLISHED:
		return "RM_ENTITY_CNXN_ESTABLISHED";

	case RM_ENTITY_SENDING_REQUEST:
		return "RM_ENTITY_SENDING_REQUEST";
	case RM_ENTITY_SENDING_REQ_N_NO_RESPONSE:
		return "RM_ENTITY_SENDING_REQ_N_NO_RESPONSE";

	case RM_ENTITY_SENDING_REQ_COMPLETED:
		return "RM_ENTITY_SENDING_REQ_COMPLETED";

	case RM_ENTITY_SENDING_REQ_FAILED:
		return "RM_ENTITY_SENDING_REQ_FAILED";
	case RM_ENTITY_SENDING_REQ_CANCELLED_BY_USER_CONFIRMED:
		return "RM_ENTITY_SENDING_REQ_CANCELLED_BY_USER_CONFIRMED";
	}

	return MmsDebugPrintUnknownValue(stateEx);
}


char *MmsDebugPrintMmsRmResult(MmsRmResult result)
{
	switch (result) {
	case MMS_RM_RESULT_SUCCESSED:
		return "MMS_RM_RESULT_SUCCESSED";
	case MMS_RM_RESULT_FAIL:
		return "MMS_RM_RESULT_FAIL";
	case MMS_RM_RESULT_FAIL_N_RETRY:
		return "MMS_RM_RESULT_FAIL_N_RETRY";

	case MMS_RM_RESULT_PROTO_STARTING_FAILED:
		return "MMS_RM_RESULT_PROTO_STARTING_FAILED";

	case MMS_RM_RESULT_DISCONNECT_MMS:
		return "MMS_RM_RESULT_DISCONNECT_MMS";
	case MMS_RM_RESULT_CLOSING_OTHER_APP_FAILED:
		return "MMS_RM_RESULT_CLOSING_OTHER_APP_FAILED";

	case MMS_RM_RESULT_SENDING_REQ_FAILED:
		return "MMS_RM_RESULT_SENDING_REQ_FAILED";
	case MMS_RM_RESULT_CANCELED_BY_USER:
		return "MMS_RM_RESULT_CANCELED_BY_USER";
	case MMS_RM_RESULT_CANCELED_FROM_RETRY_POPUP:
		return "MMS_RM_RESULT_CANCELED_FROM_RETRY_POPUP";

	case MMS_RM_RESULT_PROTO_FAILED:
		return "MMS_RM_RESULT_PROTO_FAILED";
	case MMS_RM_RESULT_DISCONNECTED_BY_OTHER:
		return "MMS_RM_RESULT_DISCONNECTED_BY_OTHER";

	case MMS_RM_RESULT_MEMORY_FULL:
		return "MMS_RM_RESULT_MEMORY_FULL";
	}

	return MmsDebugPrintUnknownValue(result);
}

void MmsDebugPrintReqEntityInfo(MmsRmRequest *pEntity)
{
	SysDebug((MID_MMS,"     - proto state=%s\n", DebugPrintGetMmsRmNetState(_MmsRmNetGetState())));

	if (_MmsRmNetGetProtoType() == MMS_RM_WAP_CONNORIENTED || _MmsRmNetGetProtoType() == MMS_RM_WTLS) {
		SysDebug((MID_MMS,"     - wap cnxn state=%s\n", DebugPrintWspState(MmsRmWspGetCnxnState())));
	}

	if (!pEntity) {
		SysDebug((MID_EXCEPTION, "MmsDebugPrintReqEntityInfo : pEntity is NULL. \n"));
		return;
	}

	SysDebug((MID_MMS,"     - entity state=%s\n", DebugPrintGetMmsRmEntityState(pEntity->stateEx)));

	if (pEntity->cb.result.bSend) {
		if (pEntity->pduType == MMS_RM_READ_REPORT_V10) {
			SysDebug((MID_MMS,"     - sending read report v10\n"));
		} else if (pEntity->pduType == MMS_RM_READ_REPORT_V11) {
			SysDebug((MID_MMS,"     - sending read report v11\n"));
		} else {
			SysDebug((MID_MMS,"     - sending msg\n"));
		}
	} else {	// then receiving..
		if (pEntity->cb.result.bAutoRetrieving) {
			SysDebug((MID_MMS,"     - retrieving(auto)\n"));
		} else {
			SysDebug((MID_MMS,"     - retrieving(manual)\n"));
		}
	}

	SysDebug((MID_MMS,"     - msgID=%d, trID=0x%x, pduType=%s\n", pEntity->msgID, pEntity->trId, MmsDebugPrintRmPduType(pEntity->pduType)));

	if (pEntity->protoActvCount > 1) {
		SysDebug((MID_MMS,"     - protoActvCount=%d\n", pEntity->protoActvCount));
	}

	if (pEntity->reqCount > 1) {
		SysDebug((MID_MMS,"     - reqCount=%d\n", pEntity->reqCount));
	}

	if (pEntity->fullRetryCount > 1) {
		SysDebug((MID_MMS,"     - fullRetryCount=%d\n", pEntity->fullRetryCount));
	}
}

char *MmsDebugPrintHttpErrorCode(int errCode)
{
	switch (errCode) {
	case HTTPERR_INVALID_PARAM:
		return "HTTPERR_INVALID_PARAM";
	case HTTPERR_UNKNOWN:
		return "HTTPERR_UNKNOWN";
	case HTTPERR_INVALID_PROXY:
		return "HTTPERR_INVALID_PROXY";
	case HTTPERR_OUT_OF_MEMORY:
		return "HTTPERR_OUT_OF_MEMORY";
	case HTTPERR_NO_RESPONSE:
		return "HTTPERR_NO_RESPONSE";
	case HTTPERR_INIT:
		return "HTTPERR_INIT";
	case HTTPERR_NETDOWN:
		return "HTTPERR_NETDOWN";
	case HTTPERR_TIMEOUT:
		return "HTTPERR_TIMEOUT";
	case HTTPERR_HOST_UNREACH:
		return "HTTPERR_HOST_UNREACH";
	case HTTPERR_CONN_RESET:
		return "HTTPERR_CONN_RESET";
	case HTTPERR_INTERNAL:
		return "HTTPERR_INTERNAL";
	case HTTPERR_CHUNKEDTR:
		return "HTTPERR_CHUNKEDTR";
	case HTTPERR_USER:
		return "HTTPERR_USER";
	case HTTPERR_TOO_BIG:
		return "HTTPERR_TOO_BIG";
	case HTTPERR_NOT_SUPPORTED_SVC:
		return "HTTPERR_NOT_SUPPORTED_SVC";
	case HTTPERR_NO_CONTENT:
		return "HTTPERR_NO_CONTENT";
	case HTTPERR_WRITEFAIL:
		return "HTTPERR_WRITEFAIL";
	case HTTPERR_AUTHFAIL:
		return "HTTPERR_AUTHFAIL";

	case HTTPERR_DNSFAIL:
		return "HTTPERR_DNSFAIL";
	case HTTPERR_UAGENT_NOT_ALLOWED:
		return "HTTPERR_UAGENT_NOT_ALLOWED";

	}
	return MmsDebugPrintUnknownValue(errCode);
}

char *MmsDebugPrintProtoErrorCode(int errCode)
{
	switch (errCode) {
	case PROTO_ERROR_NONE:
		return "PROTO_ERROR_NONE";
	/* Wireless stack errors */
	case PROTO_ERROR_NO_SERVICE:
		return "PROTO_ERROR_NO_SERVICE";
	case PROTO_ERROR_INVALID_PARAM:
		return "PROTO_ERROR_INVALID_PARAM";
	case PROTO_ERROR_INVALID_APN:
		return "PROTO_ERROR_INVALID_APN";
	case PROTO_ERROR_INVALID_OP_MODE:
		return "PROTO_ERROR_INVALID_OP_MODE";
	case PROTO_ERROR_INVALID_PROTO_TYPE:
		return "PROTO_ERROR_INVALID_PROTO_TYPE";
	case PROTO_ERROR_INVALID_SERVICE_DOMAIN:
		return "PROTO_ERROR_INVALID_SERVICE_DOMAIN";
	case PROTO_ERROR_INVALID_PDP_DATA:
		return "PROTO_ERROR_INVALID_PDP_DATA";
	case PROTO_ERROR_INVALID_QOS:
		return "PROTO_ERROR_INVALID_QOS";
	case PROTO_ERROR_SESSION_DEACTIVATED:
		return "PROTO_ERROR_SESSION_DEACTIVATED";
	case PROTO_ERROR_AUTHENTICATION_FAILED:
		return "PROTO_ERROR_AUTHENTICATION_FAILED";
	case PROTO_ERROR_MOBILE_FAILURE:
		return "PROTO_ERROR_MOBILE_FAILURE";
	case PROTO_ERROR_NETWORK_FAILURE:
		return "PROTO_ERROR_NETWORK_FAILURE";
	case PROTO_ERROR_TIMEOUT:
		return "PROTO_ERROR_TIMEOUT";
	case PROTO_ERROR_NO_RESOURCE:
		return "PROTO_ERROR_NO_RESOURCE";
	case PROTO_ERROR_INVALID_CONTEXT_ID:
		return "PROTO_ERROR_INVALID_CONTEXT_ID";
	case PROTO_ERROR_MODEM_IN_USE:
		return "PROTO_ERROR_MODEM_IN_USE";
	case PROTO_ERROR_INVALID_SIM_STATE:
		return "PROTO_ERROR_INVALID_SIM_STATE:";
	case PROTO_ERROR_SERVICE_NOT_SUBSCRIBED:
		return "PROTO_ERROR_SERVICE_NOT_SUBSCRIBED";
	case PROTO_ERROR_SERVICE_NOT_IMPLEMENTED:
		return "PROTO_ERROR_SERVICE_NOT_IMPLEMENTED";
	case PROTO_ERROR_FDN_NOT_ALLOWED:
		return "PROTO_ERROR_FDN_NOT_ALLOWED";
	case PROTO_ERROR_CALL_INCOME:
		return "PROTO_ERROR_CALL_INCOME";
	case PROTO_ERROR_NOT_SUPPORT_3GCSD:
		return "PROTO_ERROR_NOT_SUPPORT_3GCSD";
	case PROTO_ERROR_UNKNOWN:
		return "PROTO_ERROR_UNKNOWN";
	case PROTO_ERROR_SI_OFFLINE:
		return "PROTO_ERROR_SI_OFFLINE";
	/* TCP/IP/PPP stack errors */
	case PROTO_ERROR_TCPIP_UP:
		return "PROTO_ERROR_TCPIP_UP";
	case PROTO_ERROR_TCPIP_DOWN:
		return "PROTO_ERROR_TCPIP_DOWN";
	case PROTO_ERROR_PPP_UP:
		return "PROTO_ERROR_PPP_UP";
	case PROTO_ERROR_PPP_DOWN:
		return "PROTO_ERROR_PPP_DOWN";
	case PROTO_ERROR_WDP_INIT:
		return "PROTO_ERROR_WDP_INIT";
	/* ProtoMgr API call errors */
	case PROTO_ERROR_ACTIVE_CONNECTIONS:
		return "PROTO_ERROR_ACTIVE_CONNECTIONS";
	case PROTO_ERROR_MAX_CONNECTIONS:
		return "PROTO_ERROR_MAX_CONNECTIONS";
	case PROTO_ERROR_INVALID_HANDLE:
		return "PROTO_ERROR_INVALID_HANDLE";
	case PROTO_ERROR_INVALID_CONTEXT:
		return "PROTO_ERROR_INVALID_CONTEXT";
	case PROTO_ERROR_INVALID_CALLBACK:
		return "PROTO_ERROR_INVALID_CALLBACK";
	case PROTO_ERROR_INVALID_EHN:
		return "PROTO_ERROR_INVALID_EHN";
	case PROTO_ERROR_INVALID_EVENTCLASS:
		return "PROTO_ERROR_INVALID_EVENTCLASS";
	case PROTO_ERROR_INVALID_REFTYPE:
		return "PROTO_ERROR_INVALID_REFTYPE";
	case PROTO_ERROR_INVALID_TIMEOUT:
		return "PROTO_ERROR_INVALID_TIMEOUT";
	case PROTO_ERROR_MAX_CLIENT:
		return "PROTO_ERROR_MAX_CLIENT";
	case PROTO_ERROR_MAX_ACCOUNT:
		return "PROTO_ERROR_MAX_ACCOUNT";
	case PROTO_ERROR_ACCESS_DENIED:
		return "PROTO_ERROR_ACCESS_DENIED";
	case PROTO_ERROR_REGISTRY_LOAD:
		return "PROTO_ERROR_REGISTRY_LOAD";

	case PROTO_ERROR_PCBROWSING_ON:
		return "PROTO_ERROR_PCBROWSING_ON";
	case PROTO_ERROR_REACTIVATION_REQ:
		return "PROTO_ERROR_REACTIVATION_REQ";
	case PROTO_ERROR_NO_PDP_ACTIVATED:
		return "PROTO_ERROR_NO_PDP_ACTIVATED";

	case PROTO_ERROR_MAX:
		return "PROTO_ERROR_MAX";

	}

	return MmsDebugPrintUnknownValue(errCode);
}


char *DebugPrintWspResult(WspResult wspResult)
{
	switch (wspResult) {
	case WSP_RESULT_NO_ERROR:
		return "WSP_RESULT_NO_ERROR";
	case WSP_RESULT_NO_SESSION:
		return "WSP_RESULT_NO_SESSION";
	case WSP_RESULT_NOT_SUITABLE:
		return "WSP_RESULT_NOT_SUITABLE";

	case WSP_RESULT_NOT_RESPONDING:
		return "WSP_RESULT_NOT_RESPONDING";
	case WSP_RESULT_MOP_EXCEED:
		return "WSP_RESULT_MOP_EXCEED";
	case WSP_RESULT_MOM_EXCEED:
		return "WSP_RESULT_MOM_EXCEED";

	case WSP_RESULT_UNABLE:
		return "WSP_RESULT_UNABLE";
	case WSP_RESULT_UNKNOWN:
		return "WSP_RESULT_UNKNOWN";
	case WSP_RESULT_WRONG_PARAM:
		return "WSP_RESULT_WRONG_PARAM";

	case WSP_RESULT_NO_MEMORY:
		return "WSP_RESULT_NO_MEMORY";
	case WSP_RESULT_NO_HANDLE:
		return "WSP_RESULT_NO_HANDLE";

	case WSP_RESULT_CAPABILITY_UNABLE:
		return "WSP_RESULT_CAPABILITY_UNABLE";
	case WSP_RESULT_UAGENT_NOT_ALLOWED:
		return "WSP_RESULT_UAGENT_NOT_ALLOWED";
	}

	return MmsDebugPrintUnknownValue(wspResult);
}


char *DebugPrintWspState(MmsRmWapState wspState)
{
	switch (wspState) {	//gMmsRmWapState
	case RM_WAP_IDLE:
		return "RM_WAP_IDLE";
	case RM_WAP_CONNECTING:
		return "RM_WAP_CONNECTING";
	case RM_WAP_CONNECTED:
		return "RM_WAP_CONNECTED";
	case RM_WAP_DISCONNECTING:
		return "RM_WAP_DISCONNECTING";
	case RM_WAP_SECURE_REDIRECT:
		return "RM_WAP_SECURE_REDIRECT";
	case RM_WAP_FAILED:
		return "RM_WAP_FAILED";
	case RM_WAP_WTLS_HANDSHAKING:
		return "RM_WAP_WTLS_HANDSHAKING";
	case RM_WAP_WTLS_HANDSHAKE_COMPLETED:
		return "RM_WAP_WTLS_HANDSHAKE_COMPLETED";
	}

	return MmsDebugPrintUnknownValue(wspState);
}


char *MmsDebugPrintRmPduType(MmsRmPduType pduType)
{
	switch (pduType) {
	case MMS_RM_PDU_TYPE:
		return "MMS_RM_PDU_TYPE";

	case MMS_RM_SEND_REQ:
		return "MMS_RM_SEND_REQ";
	case MMS_RM_GET_REQ_AUTO:
		return "MMS_RM_GET_REQ_AUTO";
	case MMS_RM_GET_REQ_MANUAL:
		return "MMS_RM_GET_REQ_MANUAL";
	case MMS_RM_NOTIFY_RESP_IND:
		return "MMS_RM_NOTIFY_RESP_IND";
	case MMS_RM_ACK_IND:
		return "MMS_RM_ACK_IND";

	case MMS_RM_NOTI_IND:
		return "MMS_RM_NOTI_IND";
	case MMS_RM_RETRIEVE_CONF:
		return "MMS_RM_RETRIEVE_CONF";

	case MMS_RM_READ_REPORT_V10:
		return "MMS_RM_READ_REPORT_V10";
	case MMS_RM_READ_REPORT_V11:
		return "MMS_RM_READ_REPORT_V11";
	}

	return MmsDebugPrintUnknownValue(pduType);
}


char *MmsDebugPrintMailboxType(MsgMailboxType mailboxType)
{
	switch (mailboxType) {
	case MSG_MAILBOX_WRITE:
		return "MSG_MAILBOX_WRITE";
	case MSG_MAILBOX_INBOX:
		return "MSG_MAILBOX_INBOX";
	case MSG_MAILBOX_DRAFT:
		return "MSG_MAILBOX_DRAFT";
	case MSG_MAILBOX_SENT:
		return "MSG_MAILBOX_SENT";
	case MSG_MAILBOX_MAILBOX:
		return "MSG_MAILBOX_MAILBOX";
	case MSG_MAILBOX_OUTBOX:
		return "MSG_MAILBOX_OUTBOX";
	case MSG_MAILBOX_TEMPLATE:
		return "MSG_MAILBOX_TEMPLATE";
	case MSG_MAILBOX_MYFOLDER:
		return "MSG_MAILBOX_MYFOLDER";
	case MSG_MAILBOX_MYFOLDER_LIST:
		return "MSG_MAILBOX_MYFOLDER_LIST";
	case MSG_MAILBOX_PRESET:
		return "MSG_MAILBOX_PRESET";
	}

	return MmsDebugPrintUnknownValue(mailboxType);
}
#endif

