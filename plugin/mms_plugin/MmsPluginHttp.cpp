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

#include "MmsPluginHttp.h"
#include "MmsPluginUserAgent.h"
#include "stdlib.h"
#include <time.h>
#include "MmsPluginConnManWrapper.h"



void httpGetHostFromUrl(char *pUrl, char *pHost)
{
	int hostLen = 0;
	int tailLen = 0;
	char *tail = NULL;

	if (strstr(pUrl, "HTTP://") == NULL) {
		tail = strstr(pUrl, "/");
		if (NULL == tail)
			tailLen = 0;
		else
			tailLen = strlen(tail);

		hostLen = strlen(pUrl) - tailLen;
		memcpy(pHost, pUrl, hostLen);
		pHost[hostLen] = '\0';
	} else {
		tail = strstr(&pUrl[7], "/");
		if (NULL == tail)
			tailLen = 0;
		else
			tailLen = strlen(tail);

		hostLen = strlen(pUrl) - tailLen - 7;
		memcpy(pHost, &pUrl[7], hostLen);
		pHost[hostLen] = '\0';
	}
}

void  HttpHeaderInfo(curl_slist **responseHeaders, char *szUrl, int ulContentLen)
{
	char szBuffer[1025] = {0, };
	char pcheader[HTTP_REQUEST_LEN] = {0, };

	bool nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_CONTENT_TYPE, szBuffer);
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
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_HOST, szBuffer);
	if (nResult) {
		strcat(pcheader, "HOST: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_ACCEPT, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_ACCEPT_CHARSET, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Charset: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_ACCEPT_LANGUAGE, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Language: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_ACCEPT_ENCODING, szBuffer);
	if (nResult) {
		strcat(pcheader, "Accept-Encoding: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_USER_AGENT, szBuffer);
	if (nResult) {
		strcat(pcheader, "User-Agent: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}

	memset(szBuffer, 0, 1025);
	memset(pcheader, 0, HTTP_REQUEST_LEN);
	nResult = MsgMmsGetCustomHTTPHeader(MMS_HH_WAP_PROFILE, szBuffer);
	if (nResult) {
		strcat(pcheader, "X-wap-profile: ");
		strcat(pcheader, szBuffer);
		MSG_DEBUG("%s", pcheader);
		*responseHeaders = curl_slist_append(*responseHeaders, pcheader);
	}
}

bool MsgMmsGetCustomHTTPHeader(MMS_HTTP_HEADER_FIELD_T httpHeaderItem, char *szHeaderBuffer)
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
			MsgMmsGetHost(szHeaderBuffer, 1024);
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

void MsgMmsGetHost(char *szHost, int nBufferLen)
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


MMS_NET_ERROR_T MmsHttpReadData(void *ptr, size_t size, size_t nmemb, void *userdata)
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


size_t  MmsHttpPostTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	MSG_DEBUG(" ======  HTTP_EVENT_SENT ========");
	long length_received = size * nmemb;
	MmsHttpReadData(ptr, size, nmemb, userdata);

	return length_received;
}

size_t  MmsHttpGetTransactionCB(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	MSG_DEBUG(" ======  HTTP_EVENT_RECEIVED ========");
	long length_received = size * nmemb;
	MmsHttpReadData(ptr, size, nmemb, userdata);

	return length_received;
}

int httpCmdInitSession(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
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


int httpCmdPostTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
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

	MSG_PROFILE_BEGIN(libcurl);

	CURLcode rc = curl_easy_perform(httpConfig->session);

	MSG_PROFILE_END(libcurl);

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

int httpCmdGetTransaction(MMS_PLUGIN_HTTP_DATA_S *httpConfig)
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

	waiting = false;

	httpCmdHandler[eHTTP_CMD_INIT_SESSION] = &httpCmdInitSession;
	httpCmdHandler[eHTTP_CMD_POST_TRANSACTION] = &httpCmdPostTransaction;
	httpCmdHandler[eHTTP_CMD_GET_TRANSACTION] = &httpCmdGetTransaction;
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

void MmsPluginHttpAgent::setHttpWaitingFlag(bool val)
{
	waiting = val;
}

bool MmsPluginHttpAgent::getHttpWaitingFlag()
{
	return waiting;
}


int MmsPluginHttpAgent::setSession(mmsTranQEntity *qEntity)
{
	MSG_DEBUG("%s %d", qEntity->pPostData, qEntity->postDataLen);

	if (qEntity->eHttpCmdType == eHTTP_CMD_POST_TRANSACTION) {
		MSG_DEBUG("HttpCmd Post Transaction");
		MSG_DEBUG(" === HTTP Agent Thread : Signal ==> eMMS_HTTP_SIGNAL_POST_TRANSACTION");

		curl_slist *responseHeaders = NULL;

		HttpHeaderInfo(&responseHeaders, NULL, qEntity->postDataLen);
		responseHeaders = curl_slist_append(responseHeaders, "Pragma: ");
		responseHeaders = curl_slist_append(responseHeaders, "Proxy-Connection: ");
		responseHeaders = curl_slist_append(responseHeaders, "Expect: ");
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
		curl_easy_setopt(httpConfigData.session, CURLOPT_WRITEFUNCTION, MmsHttpPostTransactionCB);

		curl_easy_setopt(httpConfigData.session, CURLOPT_TCP_NODELAY, 1);
	} else if (qEntity->eHttpCmdType == eHTTP_CMD_GET_TRANSACTION) {
		MSG_DEBUG("MmsHttpInitTransactionGet  %d pGetData (%s)", qEntity->getDataLen, qEntity->pGetData);
	   	MSG_DEBUG("MmsHttpInitTransactionGet  mmscURL (%s) ", httpConfigData.mmscConfig.mmscUrl);

		char szUrl[MAX_MMSC_URL_LEN] = {0, };

		memcpy(szUrl, qEntity->pGetData, qEntity->getDataLen);

	 	MSG_DEBUG("MmsHttpInitTransactionGet  szURL (%s)", szUrl);

		curl_slist *responseHeaders = NULL;

		HttpHeaderInfo(&responseHeaders, szUrl, 0);
		responseHeaders = curl_slist_append(responseHeaders, "Pragma: ");
		responseHeaders = curl_slist_append(responseHeaders, "Proxy-Connection: ");

		httpConfigData.sessionHeader = (void *)responseHeaders;

		MSG_DEBUG("## Start Transaction : Get ##");
		curl_easy_setopt(httpConfigData.session, CURLOPT_VERBOSE, true);
	 	curl_easy_setopt(httpConfigData.session, CURLOPT_URL, szUrl);
		curl_easy_setopt(httpConfigData.session, CURLOPT_NOPROGRESS, true);
		curl_easy_setopt(httpConfigData.session, CURLOPT_HTTPHEADER, responseHeaders);
		curl_easy_setopt(httpConfigData.session, CURLOPT_WRITEFUNCTION, MmsHttpGetTransactionCB);
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
