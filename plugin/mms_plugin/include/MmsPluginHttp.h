/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
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
#include "MsgMutex.h"

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

typedef enum _MMS_HTTP_TRANSACTION_TYPE_E {
	MMS_HTTP_TRANSACTION_TYPE_UNKNOWN = 0,
	MMS_HTTP_TRANSACTION_TYPE_GET,
	MMS_HTTP_TRANSACTION_TYPE_POST,
} MMS_HTTP_TRANSACTION_TYPE_E;

typedef enum _MMS_HTTP_ERROR_E {
	MMS_HTTP_ERROR_NONE = 0,
	MMS_HTTP_ERROR_ABORT,
	MMS_HTTP_ERROR_TRANSACTION_TYPE,
	MMS_HTTP_ERROR_TRANSACTION,
} MMS_HTTP_ERROR_E;

typedef struct _http_session_info_s {
	MMS_HTTP_TRANSACTION_TYPE_E transaction_type;
	const char *url;
	const char *proxy;
	const char *interface;
	const char *post_data;
	unsigned int post_data_len;
	char *response_data;
	unsigned int response_data_len;
} http_request_info_s;

class MmsPluginHttpAgent
{
	public:
		static MmsPluginHttpAgent *instance();

		MMS_HTTP_ERROR_E httpRequest(http_request_info_s &request_info);

		void setAbortFlag(){
			MutexLocker locker(mx);
			abort = true;
		};

		bool getAbortFlag(){
			MutexLocker locker(mx);
			return abort;
		};

	private:
		static MmsPluginHttpAgent *pInstance;

		MmsPluginHttpAgent();
		~MmsPluginHttpAgent();

		void initSession();
		MMS_HTTP_ERROR_E setSession(http_request_info_s &request_info);
		MMS_HTTP_ERROR_E startTransaction();
		void clearSession();

		void initAbortFlag(){
			MutexLocker locker(mx);
			abort = false;
		};

		MMS_HTTP_TRANSACTION_TYPE_E transaction_type;

		void *session_header;
		void *session_option;

		FILE *respfile;
		bool abort;
		Mutex mx;
};

#endif //MMS_PLUGIN_HTTP_H
