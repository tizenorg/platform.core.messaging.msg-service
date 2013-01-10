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

#ifndef MMS_PLUGIN_HTTP_H
#define MMS_PLUGIN_HTTP_H

#include <map>
#include "MmsPluginTypes.h"

#define MSG_MMS_HH_CONTENT_TYPE     "application/vnd.wap.mms-message"
#define MSG_MMS_HH_ACCEPT           "application/vnd.wap.mms-message, */*"
#define MSG_MMS_HH_CHARSET          "utf-8"
#define MSG_MMS_HH_LANGUAGE         "zh-cn, en"
#define MSG_MMS_HH_ENCODING         "deflate,gzip"

#define MSG_MMS_HH_USER_AGENT		"Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML, like Gecko) Mobile Tizen Browser/1.0"
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

class MmsPluginHttpAgent
{
	public:
		static MmsPluginHttpAgent *instance();

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

		MMS_PLUGIN_HTTP_CONTEXT_S mmsPlgCd;

		std::map<MMS_HTTP_CMD_TYPE_T,int(*)(MMS_PLUGIN_HTTP_DATA_S *)> httpCmdHandler;
};

#endif //MMS_PLUGIN_HTTP_H
