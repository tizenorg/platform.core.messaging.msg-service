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

#include "MmsPluginHttp.h"
#include "MmsPluginUserAgent.h"
#include "stdlib.h"
#include <time.h>
#include "MmsPluginConnManWrapper.h"
#include <curl/curl.h>

static bool __httpGetHeaderField(MMS_HTTP_HEADER_FIELD_T httpHeaderItem, char *szHeaderBuffer);
static void __httpGetHost(char *szHost, int nBufferLen);
static void __httpAllocHeaderInfo(curl_slist **responseHeaders, char *szUrl, int ulContentLen);

static MMS_NET_ERROR_T __httpReceiveData(void *ptr, size_t size, size_t nmemb, void *userdata);
static size_t  __httpGetTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata);
static size_t  __httpPostTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata);

static int __httpCmdInitSession(MMS_PLUGIN_HTTP_DATA_S *httpConfig);
static int __httpCmdPostTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig);
static int __httpCmdGetTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig);

static void __http_print_profile(CURL *curl);

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
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

	//time
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
	MSG_DEBUG("profile http Time: total %.3f seconds", total_time);

	//url
	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);
	MSG_DEBUG("profile http Url: %s", url);

	//size
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &size_up);
	MSG_DEBUG("profile http Size: upload %.3f bytes", size_up);

	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size_down);
	MSG_DEBUG("profile http Size: download %.3f bytes", size_down);

	curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &size);
	MSG_DEBUG("profile http Size: header %ld bytes", size);

	curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &size);
	MSG_DEBUG("profile http Size: request %ld bytes", size);

	//speed
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
	MSG_DEBUG("profile http Speed: upload %.3f bytes/sec", speed_upload);

	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed_download);
	MSG_DEBUG("profile http Speed: download %.3f bytes/sec", speed_download);

	//content
	curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
	MSG_DEBUG("profile http Content: type %s", content_type);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
	MSG_DEBUG("profile http Content: length download %.3f", content_length);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &content_length);
	MSG_DEBUG("profile http Content: length upload %.3f", content_length);

	//ip & port
	curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);
	MSG_DEBUG("profile http primary: ip %s", ip);

	curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port);
	MSG_DEBUG("profile http primary: port %ld", port);

	curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &ip);
	MSG_DEBUG("profile http local: ip %s", ip);

	curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &port);
	MSG_DEBUG("profile http local: port %ld", port);
	MSG_DEBUG("**************************************************************************************************");
}

static void __httpAllocHeaderInfo(curl_slist **responseHeaders, char *szUrl, int ulContentLen)
{
	char szBuffer[1025] = {0, };
	char pcheader[HTTP_REQUEST_LEN] = {0, };


	bool nResult = __httpGetHeaderField(MMS_HH_CONTENT_TYPE, szBuffer);
	if (nResult) {
		strcat(pcheader,"Content-Type: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	snprintf(szBuffer, 1024, "%d", ulContentLen);
	if (nResult) {
		strcat(pcheader, "Content-Length: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_HOST, szBuffer);
	if (nResult) {
		strcat(pcheader, "HOST: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_CHARSET, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Charset: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_LANGUAGE, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Language: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_ACCEPT_ENCODING, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Encoding: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_USER_AGENT, szBuffer);
	if (nResult) {
		strcat(pcheader, "User-Agent: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = __httpGetHeaderField(MMS_HH_WAP_PROFILE, szBuffer);
	if (nResult) {
		strcat(pcheader, "X-wap-profile: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}
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

		case MMS_HH_HOST:
			__httpGetHost(szHeaderBuffer, 1024);
			if (strlen(szHeaderBuffer) > 0)
				result = true;
			else
				result = false;
			break;

		case MMS_HH_ACCEPT:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_ACCEPT);
			result = true;
			break;

		case MMS_HH_ACCEPT_CHARSET:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_CHARSET);
			result = true;
			break;

		case MMS_HH_ACCEPT_LANGUAGE:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_LANGUAGE);
			result = true;
			break;

		case MMS_HH_ACCEPT_ENCODING:
			snprintf((char *)szHeaderBuffer, 1024, "%s", MSG_MMS_HH_ENCODING);
			result = true;
			break;

		case MMS_HH_USER_AGENT:
			{
				char szUserAgent[1024 + 1] = {0,};

				memset(szUserAgent, 0x00, (sizeof(char) * (1024 + 1) ));
				snprintf(szUserAgent, 1024, "%s", MSG_MMS_HH_USER_AGENT);

				snprintf((char *)szHeaderBuffer, 1024, "%s", szUserAgent);
				result = true;
			}
			break;

		case MMS_HH_WAP_PROFILE:
			{
				char szUAProfile[1024 + 1] = {0,};

				memset(szUAProfile, 0x00, (sizeof(char)*(1024+1) ));
				snprintf(szUAProfile, 1024, "%s", MSG_MMS_WAP_PROFILE);

				snprintf((char *)szHeaderBuffer, 1024, "%s", szUAProfile);
				result = true;
			}
			break;

		default:
			MSG_DEBUG("invalid param");
			break;
		}
	}

	return result;
}

static void __httpGetHost(char *szHost, int nBufferLen)
{
	MmsPluginHttpAgent *pHttpAgent = MmsPluginHttpAgent::instance();
	MMS_PLUGIN_HTTP_DATA_S *httpConfigData = pHttpAgent->getHttpConfigData();

	const char *prefixString = "http://";
	const char *delim = ":/\\=@";

	int prefixLength = strlen(prefixString);

	char *startPtr = &httpConfigData->mmscConfig.mmscUrl[0];
	char *movePtr = NULL;

	MSG_DEBUG("startPtr(%s)", startPtr);

	if (strncasecmp(startPtr, prefixString, prefixLength) == 0) {
		MSG_DEBUG("(%s) exist", prefixString);
		startPtr += prefixLength;
		movePtr = startPtr;
		movePtr = strpbrk(movePtr, delim);
		MSG_DEBUG("strpbrk --> movePtr(%s)", movePtr);
		if (movePtr == NULL) {
			strncpy(szHost, startPtr, nBufferLen);
			MSG_DEBUG("szHost(%s)", szHost);
		} else {
			int nCopyLen = movePtr - startPtr;
			strncpy(szHost, startPtr, nCopyLen);
			MSG_DEBUG("szHost(%s)", szHost);
		}
	} else {
		MSG_DEBUG("(%s) not exist", prefixString);
		movePtr = startPtr;
		movePtr = strpbrk(movePtr, delim);
		MSG_DEBUG("strpbrk --> movePtr(%s)", movePtr);
		if (movePtr == NULL) {
			strncpy(szHost, startPtr, nBufferLen);
			MSG_DEBUG("szHost(%s)", szHost);
		} else {
			int nCopyLen = movePtr - startPtr;
			strncpy(szHost, startPtr, nCopyLen);
			MSG_DEBUG("szHost(%s)", szHost);
		}
	}
}


static MMS_NET_ERROR_T __httpReceiveData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	MMS_NET_ERROR_T httpRet = eMMS_UNKNOWN;

	MmsPluginHttpAgent *pHttpAgent = MmsPluginHttpAgent::instance();
	MMS_PLUGIN_HTTP_CONTEXT_S *pMmsPlgCd = pHttpAgent->getMmsPldCd();
	long length_received = size * nmemb;

	if (length_received) {
		//Save the received buffer in a safe place.
		if (pMmsPlgCd->final_content_buf == NULL) {
			MSG_DEBUG("Body Lenghth Read = %d", length_received);
			pMmsPlgCd->final_content_buf = (unsigned char *)malloc((length_received + 1) * sizeof(unsigned char));
			memset(pMmsPlgCd->final_content_buf,0x0,((length_received + 1) * sizeof(unsigned char)));
			MSG_DEBUG(" Global g_final_content_buf=%0x", pMmsPlgCd->final_content_buf);
		} else {
			//realloc pHttpEvent->bodyLen extra and memset
			pMmsPlgCd->final_content_buf = (unsigned char *)realloc(pMmsPlgCd->final_content_buf,
					(pMmsPlgCd->bufOffset + length_received + 1) * sizeof(unsigned char));
			MSG_DEBUG("Body Lenghth Read = %d Content Length = %d", length_received, pMmsPlgCd->bufOffset);
			memset((pMmsPlgCd->final_content_buf +pMmsPlgCd->bufOffset), 0x0,
					((length_received + 1) * sizeof(unsigned char)));
			MSG_DEBUG(" Global g_final_content_buf=%0x", pMmsPlgCd->final_content_buf);
		}

		//copy body
		if (pMmsPlgCd->final_content_buf != NULL) {
			memcpy( pMmsPlgCd->final_content_buf + pMmsPlgCd->bufOffset, ptr, length_received);
			MSG_DEBUG("Current g_bufOffset =%d", pMmsPlgCd->bufOffset);
			/*  Content  Received */
			MSG_DEBUG("Total Content received PTR =%0X, Content Size =%d", pMmsPlgCd->final_content_buf,
						pMmsPlgCd->bufOffset);
			pMmsPlgCd->bufOffset = pMmsPlgCd->bufOffset + length_received;
			httpRet = eMMS_UNKNOWN;
		}
	} else {
		MSG_DEBUG("End of Data transfer");
		MSG_DEBUG("MmsHttpReadData Buffer Size = %d", pMmsPlgCd->bufOffset);
		MSG_DEBUG("MmsHttpReadData Buffer = %s", pMmsPlgCd->final_content_buf);

		if (pMmsPlgCd->bufOffset == 0) {
			/*  This is applicable when - M-ReadRec.inf,M-Ack.ind, M-NotifyResp.ind posted  */
			MSG_DEBUG(" Content Size is Zero");

			if (pMmsPlgCd->final_content_buf != NULL) {
				free(pMmsPlgCd->final_content_buf );
 				pMmsPlgCd->final_content_buf = NULL;
			}

			httpRet = eMMS_HTTP_EVENT_SENT_ACK_COMPLETED;
		} else if (pMmsPlgCd->final_content_buf != NULL && pMmsPlgCd->bufOffset != 0) {
			// Process Http Data
			MSG_DEBUG(" Send Received Data to UA");
			httpRet = eMMS_HTTP_RECV_DATA; // eMMS_HTTP_RECV_DATA;
		} else {
			httpRet = eMMS_UNKNOWN; // check later
		}

		return httpRet;
	}

	return httpRet;
}


static size_t  __httpPostTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	MSG_DEBUG(" ======  HTTP_EVENT_SENT ========");
	long length_received = size * nmemb;
	__httpReceiveData(ptr, size, nmemb, userdata);

	return length_received;
}

static size_t  __httpGetTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	MSG_DEBUG(" ======  HTTP_EVENT_RECEIVED ========");
	long length_received = size * nmemb;
	__httpReceiveData(ptr, size, nmemb, userdata);

	return length_received;
}

static int __httpCmdInitSession(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
{
	MSG_DEBUG("HttpCmd Init Session");

	char proxyAddr[MAX_IPV4_LENGTH + 1] = {0};

	snprintf(proxyAddr, MAX_IPV4_LENGTH + 1, "%s:%d", httpConfig->mmscConfig.httpProxyIpAddr, httpConfig->mmscConfig.proxyPortNo);
	MSG_DEBUG("profileId [%d], proxyAddr [%s]", httpConfig->currentProfileId, proxyAddr);

	CURL *curl_session = curl_easy_init();
	if (NULL == curl_session) {
		MSG_DEBUG("curl_easy_init() failed");
		return eMMS_HTTP_SESSION_OPEN_FAILED;
	}

	int curl_status = curl_easy_setopt(curl_session, CURLOPT_PROXY, proxyAddr);
	if (curl_status != CURLM_OK) {
		MSG_DEBUG("curl_easy_setopt(): CURLOPT_PROXY failed");
		curl_easy_cleanup(curl_session);
		return eMMS_HTTP_SESSION_OPEN_FAILED;
	}

	httpConfig->session = curl_session;

	return eMMS_HTTP_SESSION_INIT;
}


static int __httpCmdPostTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
{
	int trId;

	MSG_DEBUG("HttpCmd Post Transaction");
	MSG_DEBUG(" === HTTP Agent Thread : Signal ==> eMMS_HTTP_SIGNAL_POST_TRANSACTION");

	char deviceName[1024] = {0,};

	MmsPluginCmAgent::instance()->getDeviceName(deviceName);

	MSG_DEBUG("deviceName: [%s]", deviceName);
	int curl_status = curl_easy_setopt(httpConfig->session, CURLOPT_INTERFACE, deviceName);

	if (curl_status != CURLM_OK) {
		MSG_DEBUG("curl_easy_setopt(): CURLOPT_INTERFACE failed");

		return eMMS_EXCEPTIONAL_ERROR;
	}

	CURLcode rc = curl_easy_perform(httpConfig->session);

	__http_print_profile(httpConfig->session);

	MmsPluginHttpAgent*	httpAgent = MmsPluginHttpAgent::instance();
	MMS_PLUGIN_HTTP_DATA_S *httpConfigData = httpAgent->getHttpConfigData();
	if (httpConfigData->sessionHeader) {
		curl_slist_free_all((curl_slist *)httpConfigData->sessionHeader);
		httpConfigData->sessionHeader = NULL;
	}

	if (CURLE_OK != rc) {
		MSG_DEBUG("curl_easy_perform return error rc[%d]", rc);

		return eMMS_HTTP_ERROR_NETWORK;
	}

	MSG_DEBUG("## End Transaction ##");
	MSG_DEBUG("############ trID = %d ###########", trId);

	srandom((unsigned int) time(NULL));
	trId = random() % 1000000000 + 1;

	httpConfig->transactionId = trId;

	return eMMS_HTTP_SENT_SUCCESS;
}

static int __httpCmdGetTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
{
	int trId;

	MSG_DEBUG("HttpCmd Get Transaction");
	MSG_DEBUG(" === HTTP Agent Thread : Signal ==> eMMS_HTTP_SIGNAL_GET_TRANSACTION");

	char deviceName[1024] = {0,};
	MmsPluginCmAgent::instance()->getDeviceName(deviceName);
	MSG_DEBUG("deviceName: [%s]", deviceName);

	int curl_status = curl_easy_setopt(httpConfig->session, CURLOPT_INTERFACE, deviceName);
	if (curl_status != CURLM_OK) {
		MSG_DEBUG("curl_easy_setopt(): CURLOPT_INTERFACE failed");

		return eMMS_EXCEPTIONAL_ERROR;
	}

	CURLcode rc = curl_easy_perform(httpConfig->session);

	__http_print_profile(httpConfig->session);

	MmsPluginHttpAgent*	httpAgent = MmsPluginHttpAgent::instance();
	MMS_PLUGIN_HTTP_DATA_S *httpConfigData = httpAgent->getHttpConfigData();
	if (httpConfigData->sessionHeader) {
		curl_slist_free_all((curl_slist *)httpConfigData->sessionHeader);
		httpConfigData->sessionHeader = NULL;
	}

	if (CURLE_OK != rc) {
		MSG_DEBUG("curl_easy_perform return error = %d", rc);

		return eMMS_HTTP_ERROR_NETWORK;
	}

 	MSG_DEBUG("## End Transaction ##");
	MSG_DEBUG("############ trID = %d ###########", trId);
	srandom((unsigned int) time(NULL));
	trId = random() % 1000000000 + 1;
	httpConfig->transactionId = trId;

	return eMMS_HTTP_SENT_SUCCESS;
}

MmsPluginHttpAgent *MmsPluginHttpAgent::pInstance = NULL;
MmsPluginHttpAgent *MmsPluginHttpAgent::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginHttpAgent();

	return pInstance;
}


MmsPluginHttpAgent::MmsPluginHttpAgent()
{
	MSG_DEBUG("MmsPluginHttpAgent()");

	bzero(&httpConfigData, sizeof(httpConfigData));
	bzero(&mmsPlgCd, sizeof(mmsPlgCd));

	httpCmdHandler.clear();

	httpCmdHandler[eHTTP_CMD_INIT_SESSION] = &__httpCmdInitSession;
	httpCmdHandler[eHTTP_CMD_POST_TRANSACTION] = &__httpCmdPostTransaction;
	httpCmdHandler[eHTTP_CMD_GET_TRANSACTION] = &__httpCmdGetTransaction;
}

MmsPluginHttpAgent::~MmsPluginHttpAgent()
{

}

void MmsPluginHttpAgent::SetMMSProfile()
{
	MSG_BEGIN();

	MMSC_CONFIG_DATA_S *mmscConfig = &(httpConfigData.mmscConfig);

	MmsPluginCmAgent::instance()->getHomeURL(mmscConfig->mmscUrl);
	if (strlen(mmscConfig->mmscUrl) < 1) {
		strcpy(mmscConfig->mmscUrl, DEFAULT_MMSC_URL);
	}

	MmsPluginCmAgent::instance()->getProxyAddr(mmscConfig->httpProxyIpAddr);
	mmscConfig->proxyPortNo = MmsPluginCmAgent::instance()->getProxyPort();

	MSG_END();
}

int MmsPluginHttpAgent::cmdRequest(MMS_HTTP_CMD_TYPE_T cmdType)
{
	MSG_DEBUG("cmdRequest:%x", cmdType);

	int ret = 0;

	ret = httpCmdHandler[cmdType](&httpConfigData);

	return ret;
}

MMS_PLUGIN_HTTP_CONTEXT_S* MmsPluginHttpAgent::getMmsPldCd()
{
	return &mmsPlgCd;
}

MMS_PLUGIN_HTTP_DATA_S *MmsPluginHttpAgent::getHttpConfigData()
{
	return &httpConfigData;
}

int MmsPluginHttpAgent::setSession(mmsTranQEntity *qEntity)
{
	MSG_DEBUG("%s %d", qEntity->pPostData, qEntity->postDataLen);

	if (qEntity->eHttpCmdType == eHTTP_CMD_POST_TRANSACTION) {
		MSG_DEBUG("HttpCmd Post Transaction");
		MSG_DEBUG(" === HTTP Agent Thread : Signal ==> eMMS_HTTP_SIGNAL_POST_TRANSACTION");

		curl_slist *responseHeaders = NULL;

		__httpAllocHeaderInfo(&responseHeaders, NULL, qEntity->postDataLen);

		//Disable 'Expect: 100-contine' option
		responseHeaders = curl_slist_append(responseHeaders, "Expect:");

		MSG_DEBUG(" === MMSCURI = %s === ", httpConfigData.mmscConfig.mmscUrl);

		httpConfigData.sessionHeader = (void *)responseHeaders;

		MSG_DEBUG("## Start Transaction : Post ##");
		curl_easy_setopt(httpConfigData.session, CURLOPT_VERBOSE, true);
		curl_easy_setopt(httpConfigData.session, CURLOPT_POST, true);
	 	curl_easy_setopt(httpConfigData.session, CURLOPT_URL, httpConfigData.mmscConfig.mmscUrl);
		curl_easy_setopt(httpConfigData.session, CURLOPT_NOPROGRESS, true);
		curl_easy_setopt(httpConfigData.session, CURLOPT_HTTPHEADER, responseHeaders);
		curl_easy_setopt(httpConfigData.session, CURLOPT_POSTFIELDS, qEntity->pPostData);
		curl_easy_setopt(httpConfigData.session, CURLOPT_POSTFIELDSIZE, qEntity->postDataLen);
		curl_easy_setopt(httpConfigData.session, CURLOPT_WRITEFUNCTION, __httpPostTransactionCB);

		curl_easy_setopt(httpConfigData.session, CURLOPT_TCP_NODELAY, 1);
	} else if (qEntity->eHttpCmdType == eHTTP_CMD_GET_TRANSACTION) {
		MSG_DEBUG("MmsHttpInitTransactionGet  %d pGetData (%s)", qEntity->getDataLen, qEntity->pGetData);
	   	MSG_DEBUG("MmsHttpInitTransactionGet  mmscURL (%s) ", httpConfigData.mmscConfig.mmscUrl);

		char szUrl[MAX_MMSC_URL_LEN] = {0, };

		memcpy(szUrl, qEntity->pGetData, qEntity->getDataLen);

	 	MSG_DEBUG("MmsHttpInitTransactionGet  szURL (%s)", szUrl);

	 	curl_slist *responseHeaders = NULL;

		__httpAllocHeaderInfo(&responseHeaders, NULL, 0);

		httpConfigData.sessionHeader = (void *)responseHeaders;

		MSG_DEBUG("## Start Transaction : Get ##");
		curl_easy_setopt(httpConfigData.session, CURLOPT_VERBOSE, true);
	 	curl_easy_setopt(httpConfigData.session, CURLOPT_URL, szUrl);
		curl_easy_setopt(httpConfigData.session, CURLOPT_NOPROGRESS, true);
		curl_easy_setopt(httpConfigData.session, CURLOPT_HTTPHEADER, responseHeaders);
		curl_easy_setopt(httpConfigData.session, CURLOPT_WRITEFUNCTION, __httpGetTransactionCB);
	} else {
		MSG_DEBUG("Unknown eHttpCmdType [%d]", qEntity->eHttpCmdType);
		return -1;
	}

	return 0;
}


void MmsPluginHttpAgent::clearSession()
{
	MSG_BEGIN();

	if (httpConfigData.sessionHeader) {
		curl_slist_free_all((curl_slist *)httpConfigData.sessionHeader);
		httpConfigData.sessionHeader = NULL;
	}

	if (httpConfigData.session == NULL) {
		MSG_DEBUG("[Error]httpConfigData.session is NULL");
		return;
	}

	curl_easy_cleanup(httpConfigData.session);

	httpConfigData.session = NULL;

	MSG_END();
}
