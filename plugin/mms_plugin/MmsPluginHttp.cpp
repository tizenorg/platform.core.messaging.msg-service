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

#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>
#include "MmsPluginDebug.h"
#include "MmsPluginHttp.h"
#include "MmsPluginUserAgent.h"
#include "MmsPluginConnManWrapper.h"
#include "MsgGconfWrapper.h"
#include "MmsPluginUtil.h"

static void __http_print_profile(CURL *curl);
static int __http_debug_cb(CURL *input_curl, curl_infotype input_info_type, char *input_data, size_t input_size, void *input_void);

static void __httpAllocHeaderInfo(curl_slist **responseHeaders, char *szUrl, int ulContentLen);
static bool __httpGetHeaderField(MMS_HTTP_HEADER_FIELD_T httpHeaderItem, char *szHeaderBuffer);
static void __httpGetHost(char *szUrl, char *szHost, int nBufferLen);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

static int __http_debug_cb(CURL *input_curl, curl_infotype input_info_type, char *input_data, size_t input_size, void *input_void)
{
	MSG_SEC_INFO("curl_infotype [%d] : %s", input_info_type, input_data);
	return 0;
}

static size_t __http_write_response_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
	MSG_BEGIN();
	FILE *writehere = (FILE *)data;
	return fwrite(ptr, size, nmemb, writehere);
}

static void __http_print_profile(CURL *curl)
{
	double speed_upload, speed_download, total_time;
	double size_up, size_down;
	double content_length;

	char *content_type = NULL;
	char *ip = NULL;
	char *url = NULL;

	long port;
	long size;

	MSG_DEBUG("**************************************************************************************************");

	/* time */
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
	MSG_SEC_INFO("profile http Time: total %.3f seconds", total_time);

	/* url */
	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
	MSG_SEC_INFO("profile http Url: %s", url);

	/* size */
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &size_up);
	MSG_SEC_INFO("profile http Size: upload %.3f bytes", size_up);

	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size_down);
	MSG_SEC_INFO("profile http Size: download %.3f bytes", size_down);

	curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &size);
	MSG_SEC_INFO("profile http Size: header %ld bytes", size);

	curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &size);
	MSG_SEC_INFO("profile http Size: request %ld bytes", size);

	/* speed */
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
	MSG_SEC_INFO("profile http Speed: upload %.3f bytes/sec", speed_upload);

	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed_download);
	MSG_SEC_INFO("profile http Speed: download %.3f bytes/sec", speed_download);

	/* content */
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
	MSG_SEC_INFO("profile http Content: type %s", content_type);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
	MSG_SEC_INFO("profile http Content: length download %.3f", content_length);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &content_length);
	MSG_SEC_INFO("profile http Content: length upload %.3f", content_length);

	/* ip & port */
	curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);
	MSG_SEC_INFO("profile http primary: ip %s", ip);

	curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port);
	MSG_SEC_INFO("profile http primary: port %ld", port);

	curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &ip);
	MSG_SEC_INFO("profile http local: ip %s", ip);

	curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &port);
	MSG_SEC_INFO("profile http local: port %ld", port);

	MSG_DEBUG("**************************************************************************************************");
}

static void __httpAllocHeaderInfo(curl_slist **responseHeaders, char *szUrl, int ulContentLen)
{
	char szBuffer[1025] = {0, };
	char pcheader[HTTP_REQUEST_LEN] = {0, };

	bool nResult = __httpGetHeaderField(MMS_HH_CONTENT_TYPE, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Content-Type: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	if (ulContentLen > 0) {
		memset(szBuffer, 0, 1025);
		memset(pcheader, 0, HTTP_REQUEST_LEN);
		snprintf(szBuffer, 1024, "%d", ulContentLen);
		if (nResult) {
			snprintf(pcheader, HTTP_REQUEST_LEN, "Content-Length: %s", szBuffer);
			MSG_INFO("%s", pcheader);
			*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
		}
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);

	__httpGetHost(szUrl, szBuffer, 1024);
	if (strlen(szBuffer)) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Host: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Accept: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_CHARSET, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Accept-Charset: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_LANGUAGE, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Accept-Language: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}
/* NOW not support gzip, deflate encoding in MMS Plugin
	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_ENCODING, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "Accept-Encoding: %s", szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}
*/
	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_USER_AGENT, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "User-Agent: %s", szBuffer);
		MSG_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_UA_PROFILE, szBuffer);
	if (nResult) {
		snprintf(pcheader, HTTP_REQUEST_LEN, "X-wap-profile: %s", szBuffer);
		MSG_SEC_INFO("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

#if defined(FEATURE_SMS_CDMA)
	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_MDN, szBuffer);

	/* TODO : if it needs to check http header mdn value, add code to below. */
#endif

	if (ulContentLen > 0) /* if post transaction then Disable 'Expect: 100-contine' option */
		*responseHeaders = curl_slist_append(*responseHeaders, "Expect:");
}

static bool __httpGetHeaderField(MMS_HTTP_HEADER_FIELD_T httpHeaderItem, char *szHeaderBuffer)
{
	bool result;

	result = false;
	if (szHeaderBuffer != NULL) {
		switch (httpHeaderItem) {
		case MMS_HH_CONTENT_TYPE:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_CONTENT_TYPE);
			result = true;
			break;

		case MMS_HH_ACCEPT:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_ACCEPT);
			result = true;
			break;

		case MMS_HH_ACCEPT_CHARSET:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_CHARSET);
#if defined(FEATURE_SMS_CDMA)
			result = false;
#else
			result = true;
#endif
			break;

		case MMS_HH_ACCEPT_LANGUAGE:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_LANGUAGE);
			result = true;
			break;

		case MMS_HH_ACCEPT_ENCODING:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_ENCODING);
			result = true;
			break;

		case MMS_HH_USER_AGENT: {
#if 0
				char szUserAgent[1024 + 1];
				char *uagent = NULL;

				if (MsgSettingGetString(VCONFKEY_BROWSER_USER_AGENT, &uagent) != MSG_SUCCESS) {
					MSG_INFO("MsgSettingGetString() is failed");
				}

				if (uagent && strlen(uagent) > 0) {
					MSG_SEC_INFO("Get UserAgent : %s = %s", VCONFKEY_BROWSER_USER_AGENT, uagent);
					memset(szUserAgent, 0x00, sizeof(szUserAgent));
					MmsRemoveMarkup(uagent, szUserAgent, sizeof(szUserAgent));
				} else {
					memset(szUserAgent, 0x00, sizeof(szUserAgent));
					MSG_SEC_INFO("Get Default UserAgent : %s", MSG_MMS_HH_USER_AGENT);
					snprintf(szUserAgent, 1024, "%s", MSG_MMS_HH_USER_AGENT);
				}

				snprintf((char *)szHeaderBuffer, 1024, "%s", szUserAgent);
				if (uagent) {
					free(uagent);
					uagent = NULL;
				}
#else
				snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_USER_AGENT);
#endif
				result = true;
			}
			break;

		case MMS_HH_UA_PROFILE: {
				char *szUAProfile = NULL;
				if (MsgSettingGetString(MSG_MMS_UA_PROFILE, &szUAProfile) != MSG_SUCCESS) {
					MSG_INFO("MsgSettingGetString() is failed");
				}

				snprintf((char *)szHeaderBuffer, 1024, "%s", szUAProfile);
				if (szUAProfile) {
					free(szUAProfile);
					szUAProfile = NULL;
				}
				result = true;
			}
			break;

#if defined(FEATURE_SMS_CDMA)
		case MMS_HH_MDN: {
/*
			char *mdn = NULL;
			if (MsgSettingGetString(MSG_SIM_MSISDN, &mdn) != MSG_SUCCESS) {
				MSG_INFO("MsgSettingGetString() is failed");
			}

			if (mdn != NULL && strlen(mdn) > 0) {
				result = true;
				snprintf((char *)szHeaderBuffer, 1024, "%s", mdn);
			} else {
				result = false;
			}
			if (mdn) {
				free(mdn);
				mdn = NULL;
			}
			break;
*/
		}
#endif
		default:
			MSG_WARN("invalid param [%d]", httpHeaderItem);
			break;
		}
	}

	return result;
}

static void __httpGetHost(char *szUrl, char *szHost, int nBufferLen)
{
	if (szUrl == NULL || szHost == NULL)
		return;

	const char *prefixString = "http://";
	const char *delim = "/\\=@";

	int prefixLength = strlen(prefixString);

	char *startPtr = szUrl;
	char *movePtr = NULL;

	MSG_SEC_DEBUG("startPtr(%s)", startPtr);

	if (strncasecmp(startPtr, prefixString, prefixLength) == 0) {
		MSG_SEC_DEBUG("(%s) exist", prefixString);
		startPtr += prefixLength;
		movePtr = startPtr;
		movePtr = strpbrk(movePtr, delim);
		MSG_SEC_DEBUG("strpbrk --> movePtr(%s)", movePtr);
		if (movePtr == NULL) {
			strncpy(szHost, startPtr, nBufferLen);
			MSG_SEC_DEBUG("szHost(%s)", szHost);
		} else {
			int nCopyLen = movePtr - startPtr;
			strncpy(szHost, startPtr, nCopyLen);
			MSG_SEC_DEBUG("szHost(%s)", szHost);
		}
	} else {
		MSG_SEC_DEBUG("(%s) not exist", prefixString);
		movePtr = startPtr;
		movePtr = strpbrk(movePtr, delim);
		MSG_SEC_DEBUG("strpbrk --> movePtr(%s)", movePtr);
		if (movePtr == NULL) {
			strncpy(szHost, startPtr, nBufferLen);
			MSG_SEC_DEBUG("szHost(%s)", szHost);
		} else {
			int nCopyLen = movePtr - startPtr;
			strncpy(szHost, startPtr, nCopyLen);
			MSG_SEC_DEBUG("szHost(%s)", szHost);
		}
	}
}

static int __http_multi_perform(void *session)
{
	MSG_BEGIN();

	CURLM *multi_handle;
	CURLMcode rcm;

	int still_running;
	int ret = 0;
	bool connection_open_flag = false;

	CURLMsg *msg;
	int msgs_left;

	multi_handle = curl_multi_init();

	if (curl_multi_add_handle(multi_handle, session) != 0) {
		MSG_ERR("curl_multi_add_handle is failed");
		curl_multi_cleanup(multi_handle);
		return -1;
	}

	/* we start some action by calling perform right away */
	rcm = curl_multi_perform(multi_handle, &still_running);
	MSG_INFO("curl_multi_perform first end : rcm = %d, still_running = %d", rcm, still_running);

	do {
		int retval;
		struct timeval timeout;

		int max_fd = -1;
		fd_set fd_r;
		fd_set fd_w;
		fd_set fd_excp;

		FD_ZERO(&fd_r);
		FD_ZERO(&fd_w);
		FD_ZERO(&fd_excp);

		timeout.tv_sec = 120;
		timeout.tv_usec = 0;

		curl_multi_fdset(multi_handle, &fd_r, &fd_w, &fd_excp, &max_fd);

		retval = select(max_fd+1, &fd_r, &fd_w, &fd_excp, &timeout);

		if (retval == -1) { /* select error */
			MSG_ERR("select error");
			ret = -1;
			break;
		} else if (retval == 0) {	/* timeout */
			MSG_ERR("time out");
			ret = -1;
			break;
		} else { /* action */
			MSG_DEBUG("retval = %d", retval);
			rcm = curl_multi_perform(multi_handle, &still_running);
		}

		connection_open_flag = MmsPluginCmAgent::instance()->getCmStatus();
		if (connection_open_flag == false) {
			MSG_DEBUG("Connection Closed");
			ret = -1;
		}

		MSG_INFO("curl_multi_perform end : rcm = %d, still_running = %d, cm_open = %d", rcm, still_running, connection_open_flag);
	} while (still_running && (connection_open_flag == true));

	while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			if (msg->easy_handle == session) {
				MSG_INFO("HTTP transfer completed with status %d", msg->data.result);
				if (msg->data.result != 0) {
					ret = msg->data.result;
				}

				curl_multi_remove_handle(multi_handle, session);
			} else {
				MSG_WARN("Unknown handle HTTP transfer completed with status %d", msg->data.result);
			}
		}
	}

	curl_multi_cleanup(multi_handle);

	MSG_END();
	return ret;
}

MmsPluginHttpAgent *MmsPluginHttpAgent::pInstance = NULL;
MmsPluginHttpAgent *MmsPluginHttpAgent::instance()
{
	if (!pInstance) {
		pInstance = new MmsPluginHttpAgent();
	}

	return pInstance;
}


MmsPluginHttpAgent::MmsPluginHttpAgent()
{
	MSG_BEGIN();

	abort = false;
	respfile = NULL;
	session_header = NULL;
	session_option = NULL;

	transaction_type = MMS_HTTP_TRANSACTION_TYPE_UNKNOWN;
	MSG_END();
}

MmsPluginHttpAgent::~MmsPluginHttpAgent()
{
	MSG_BEGIN();

	if (session_header) {
		MSG_DEBUG("session header is exist : free session header");
		curl_slist_free_all((curl_slist *)session_header);
		session_header = NULL;
	}

	if (session_option) {
		MSG_DEBUG("session is exist : cleanup session");
		curl_easy_cleanup(session_option);
		session_option = NULL;
	}

	MSG_END();
}

void MmsPluginHttpAgent::initSession()
{
	MSG_BEGIN();

	this->transaction_type = MMS_HTTP_TRANSACTION_TYPE_UNKNOWN;

	if (session_header) {
		MSG_DEBUG("session header is exist : free session header");
		curl_slist_free_all((curl_slist *)session_header);
		session_header = NULL;
	}

	if (session_option) {
		MSG_DEBUG("session is exist : cleanup session");
		curl_easy_cleanup(session_option);
		session_option = NULL;
	}

	if (respfile) {
		fclose(respfile);
		respfile = NULL;
	}

	initAbortFlag();

	MSG_END();
}

void MmsPluginHttpAgent::clearSession()
{
	MSG_BEGIN();

	this->transaction_type = MMS_HTTP_TRANSACTION_TYPE_UNKNOWN;

	if (session_header) {
		MSG_DEBUG("session header is exist : free session header");
		curl_slist_free_all((curl_slist *)session_header);
		session_header = NULL;
	}

	if (session_option) {
		MSG_DEBUG("session is exist : cleanup session");
		curl_easy_cleanup(session_option);
		session_option = NULL;
	}

	if (respfile) {
		fclose(respfile);
		respfile = NULL;
	}

	initAbortFlag();
	MSG_END();
}

MMS_HTTP_ERROR_E MmsPluginHttpAgent::setSession(http_request_info_s &request_info)
{
	MSG_BEGIN();
	MMS_HTTP_ERROR_E http_error = MMS_HTTP_ERROR_NONE;
	int content_len = 0;
	char *url = NULL;

	/* Verify request info */
	if (request_info.transaction_type != MMS_HTTP_TRANSACTION_TYPE_GET
			&& request_info.transaction_type != MMS_HTTP_TRANSACTION_TYPE_POST) {
		MSG_ERR("transaction_type of request_info is Invaild [%d]", request_info.transaction_type);
		http_error = MMS_HTTP_ERROR_TRANSACTION_TYPE;
		goto __CATCH;
	}

	if (request_info.transaction_type == MMS_HTTP_TRANSACTION_TYPE_POST) {
		if (request_info.post_data == NULL || request_info.post_data_len == 0) {
			MSG_ERR("post data info is Invaild");
			http_error = MMS_HTTP_ERROR_SESSION;
			goto __CATCH;
		}
	}

	if (request_info.url == NULL || request_info.proxy == NULL || request_info.interface == NULL) {
		MSG_ERR("request_info parameter invalid url [%s], proxy [%s] interface [%s]", request_info.url, request_info.proxy, request_info.interface);
		http_error = MMS_HTTP_ERROR_SESSION;
		goto __CATCH;
	}

	/* Set type */
	this->transaction_type = request_info.transaction_type;
	MSG_DEBUG("set transaction type [%d]", this->transaction_type);

	/* Set http Headers */
	if (this->transaction_type == MMS_HTTP_TRANSACTION_TYPE_POST) {
		content_len = request_info.post_data_len;
	} else { /* MMS_HTTP_TRANSACTION_TYPE_GET */
		content_len = 0;
	}

	url = g_strdup(request_info.url);

	if (url) {
		__httpAllocHeaderInfo((curl_slist**)&session_header, url, content_len);
		if (session_header == NULL) {
			MSG_ERR("Failed to httpAllocHeaderInfo");
			http_error = MMS_HTTP_ERROR_SESSION;
			goto __CATCH;
		}

		free(url);
		url = NULL;
	} else {
		MSG_ERR("Failed to strdup");
		goto __CATCH;
	}


	/* Set curl option */
	session_option = curl_easy_init();
	if (session_option == NULL) {
		MSG_ERR("curl_easy_init() failed");
		http_error = MMS_HTTP_ERROR_SESSION;
		goto __CATCH;
	}

	curl_easy_setopt(session_option, CURLOPT_PROXY, request_info.proxy);
	curl_easy_setopt(session_option, CURLOPT_VERBOSE, true);
	curl_easy_setopt(session_option, CURLOPT_URL, request_info.url);
	curl_easy_setopt(session_option, CURLOPT_NOPROGRESS, true);
	curl_easy_setopt(session_option, CURLOPT_HTTPHEADER, session_header);
	curl_easy_setopt(session_option, CURLOPT_DEBUGFUNCTION , __http_debug_cb);
	curl_easy_setopt(session_option, CURLOPT_INTERFACE, request_info.interface);

	if (respfile) {
		curl_easy_setopt(session_option, CURLOPT_WRITEFUNCTION, __http_write_response_cb);
		curl_easy_setopt(session_option, CURLOPT_WRITEDATA, respfile);
	}

	if (transaction_type == MMS_HTTP_TRANSACTION_TYPE_POST) {
		curl_easy_setopt(session_option, CURLOPT_POST, true);
	 	curl_easy_setopt(session_option, CURLOPT_POSTFIELDS, request_info.post_data);
		curl_easy_setopt(session_option, CURLOPT_POSTFIELDSIZE, request_info.post_data_len);
/*		curl_easy_setopt(session_option, CURLOPT_TCP_NODELAY, 1); */
	}

	MSG_END();
	return http_error;

__CATCH:
	/* CID 338211: freeing url (with check) in case of error */
	if (url) {
		free(url);
		url = NULL;
	}
	clearSession();

	MSG_END();
	return http_error;
}

MMS_HTTP_ERROR_E MmsPluginHttpAgent::startTransaction()
{
	MSG_BEGIN();
	MMS_HTTP_ERROR_E http_error = MMS_HTTP_ERROR_NONE;

	int rc = __http_multi_perform(session_option);

	__http_print_profile(session_option);

	if (rc != 0) {
		MSG_ERR("__http_multi_perform return error rc [%d]", rc);
		http_error = MMS_HTTP_ERROR_TRANSACTION;
	}

	MSG_END();
	return http_error;
}

MMS_HTTP_ERROR_E MmsPluginHttpAgent::httpRequest(http_request_info_s &request_info)
{
	MSG_BEGIN();

	const char *conf_filename = MSG_DATA_PATH"mms.conf";
	MMS_HTTP_ERROR_E http_error = MMS_HTTP_ERROR_NONE;

	this->initSession();

	respfile = fopen(conf_filename, "wb");

	/* set session */
	http_error = this->setSession(request_info);
	if (http_error != MMS_HTTP_ERROR_NONE) {
		MSG_ERR("Fail to setSession");
		goto __CATCH;
	}

	/* transaction */
	http_error = this->startTransaction();

	if (http_error != MMS_HTTP_ERROR_NONE) {
		MSG_ERR("Fail to startTransaction");
		goto __CATCH;
	}

	/* close conf file & load response data */
	if (respfile) {
		fclose(respfile);
		respfile = NULL;

		if (g_file_get_contents((gchar*)conf_filename, (gchar**)&request_info.response_data, (gsize*)&request_info.response_data_len, NULL) == false) {
			MSG_WARN("Fail to g_file_get_contents");
		}
	}

	this->clearSession();

	MSG_END();
	return http_error;

__CATCH:

	if (respfile) {
		fclose(respfile);
		respfile = NULL;
	}

	this->clearSession();

	MSG_END();
	return http_error;
}
