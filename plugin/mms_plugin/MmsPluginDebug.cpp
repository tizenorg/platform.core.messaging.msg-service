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
	case MIME_APPLICATION_VND_SAMSUNG_THEME:
		return "MIME_APPLICATION_VND_SAMSUNG_THEME";
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
