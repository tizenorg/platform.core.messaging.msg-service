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

#ifndef MMSPLUGINHTTP_H
#define MMSPLUGINHTTP_H

#include <map>

#include "MsgDebug.h"
#include "MmsPluginTypes.h"
#include "MsgMutex.h"

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

class MmsPluginHttpAgent
{
	public:
		static MmsPluginHttpAgent *instance();


		void setHttpWaitingFlag(bool val);
		bool getHttpWaitingFlag();

		int cmdRequest(MMS_HTTP_CMD_TYPE_T cmdType);

		int setSession(mmsTranQEntity *qEntity);
		void clearSession();

		void SetMMSProfile();

		MMS_PLUGIN_HTTP_DATA_S *getHttpConfigData();
		MMS_PLUGIN_HTTP_CONTEXT_S *getMmsPldCd();

		MMS_PLUGIN_HTTP_DATA_S httpConfigData;

	private:
		static MmsPluginHttpAgent *pInstance;


		MmsPluginHttpAgent();
		~MmsPluginHttpAgent();

		bool waiting;

		MMS_PLUGIN_HTTP_CONTEXT_S mmsPlgCd;

		std::map<MMS_HTTP_CMD_TYPE_T,int(*)(MMS_PLUGIN_HTTP_DATA_S *)> httpCmdHandler;
};


#define MSG_MMS_HH_CONTENT_TYPE     "application/vnd.wap.mms-message"
#define MSG_MMS_HH_ACCEPT           "application/vnd.wap.mms-message, */*"
#define MSG_MMS_HH_CHARSET          "utf-8"
#define MSG_MMS_HH_LANGUAGE         "zh-cn, en"
#define MSG_MMS_HH_ENCODING         "deflate,gzip"

#define MSG_MMS_HH_USER_AGENT		"Tizen", "Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML, like Gecko) Mobile Tizen Browser/1.0"
#define MSG_MMS_WAP_PROFILE         ""

typedef unsigned int MMS_HTTP_HEADER_FIELD_T;

enum _MMS_HTTP_HEADER_FIELD_E {
	MMS_HH_CONTENT_TYPE,
	MMS_HH_HOST,
	MMS_HH_ACCEPT,
	MMS_HH_ACCEPT_CHARSET,
	MMS_HH_ACCEPT_LANGUAGE,
	MMS_HH_ACCEPT_ENCODING,
	MMS_HH_USER_AGENT,
	MMS_HH_WAP_PROFILE
};
void HttpHeaderInfo(curl_slist *responseHeaders, char *szUrl, int ulContentLen);
bool MsgMmsGetCustomHTTPHeader(MMS_HTTP_HEADER_FIELD_T httpHeaderItem, char *szHeaderBuffer);
void MsgMmsGetHost(char *szHost, int nBufferLen);

#endif
