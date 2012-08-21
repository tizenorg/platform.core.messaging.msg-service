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

#ifndef MMSTPDUTYPES_H
#define MMSTPDUTYPES_H

#define	__SUPPORT_LIBCURL__

#include "MmsPluginMessage.h"
#include "MsgStorageTypes.h"


typedef enum _E_UA_STATE {
	eUA_IDLE = 0,
	eUA_READY,
	eUA_WAITING
} E_UA_STATE;


typedef unsigned int MMS_NET_ERROR_T;

typedef unsigned int MMS_PDU_TYPE_T;

typedef unsigned char MMS_HTTP_CMD_TYPE_T;


enum _MMS_NET_ERROR_E {
	eMMS_SUCCESS = 0,
	eMMS_CM_OPEN_SUCCESS,
	eMMS_CM_OPEN_FAILED,
	eMMS_CM_CLOSE_IND,
	eMMS_CM_CLOSE_RSP,
	eMMS_CM_CLOSE_FAILED,	//5
	eMMS_CM_KILL_RSP,
	eMMS_CM_KILL_FAILED,

	eMMS_HTTP_SESSION_INIT,
	eMMS_HTTP_SESSION_CLOSED,
	eMMS_HTTP_SESSION_OPEN_FAILED, //10
	eMMS_HTTP_SENT_SUCCESS,
	eMMS_HTTP_CONF_SUCCESS,
	eMMS_HTTP_ERROR_NETWORK,
	eMMS_HTTP_CONF_RECEIVED_TIMEOUT,
	eMMS_HTTP_RECV_DATA,	//15
	eMMS_HTTP_EVENT_RECV_DATA_PROGRESS,
	eMMS_HTTP_EVENT_RECV_DATA_ERROR,
	eMMS_HTTP_EVENT_SENT_ACK_COMPLETED,
	eMMS_HTTP_ERROR_UNKNOWN,
	eMMS_EXCEPTIONAL_ERROR,	//20

	eMMS_UNKNOWN
};

typedef struct _MMS_COND_S {
	bool valid;
	MMS_NET_ERROR_T reason;

} MMS_COND_S;

enum _MMS_PDU_TYPE_E {
	eMMS_SEND_REQ = 0,
	eMMS_SEND_CONF,
	eMMS_NOTIFICATION_IND,
	eMMS_NOTIFYRESP_IND,
	eMMS_RETRIEVE_AUTO_CONF,
	eMMS_ACKNOWLEDGE_IND,
	eMMS_DELIVERY_IND,
	eMMS_READREC_IND,
	eMMS_READORIG_IND,
	eMMS_READREPORT_REQ,
	eMMS_READREPORT_CONF,	//10
	eMMS_FORWARD_REQ,
	eMMS_FORWARD_CONF,
	eMMS_RETRIEVE_AUTO,
	eMMS_RETRIEVE_MANUAL,
	eMMS_RETRIEVE_MANUAL_CONF,
	eMMS_CANCEL_REQ,
	eMMS_CANCEL_CONF,
	eMMS_DELETE_REQ,
	eMMS_DELETE_CONF,
	eMMS_MBOX_STORE_REQ,	// 20
	eMMS_MBOX_STORE_CONF,
	eMMS_MBOX_VIEW_REQ,
	eMMS_MBOX_VIEW_CONF,
	eMMS_MBOX_UPLOAD_REQ,
	eMMS_MBOX_UPLOAD_CONF,
	eMMS_MBOX_DELETE_REQ,
	eMMS_MBOX_DELETE_CONF,
};

enum _MMS_HTTP_CMD_TYPE_E {
	eHTTP_CMD_REGISTER = 0,
	eHTTP_CMD_DEREGISTER,
	eHTTP_CMD_INIT_SESSION,
	eHTTP_CMD_CANCEL_SESSION,
	eHTTP_CMD_CLOSE_SESSION,
	eHTTP_CMD_DELETE_SESSION,
	eHTTP_CMD_POST_TRANSACTION,
	eHTTP_CMD_GET_TRANSACTION,
	eHTTP_CMD_DELETE_TRANSACTION
};

typedef enum {
	MSG_CHECK_ADDR_TYPE_PHONE = 0x01,
	MSG_CHECK_ADDR_TYPE_EMAIL = 0x02,
	MSG_CHECK_ADDR_TYPE_IPV4  = 0x04,
	MSG_CHECK_ADDR_TYPE_IPV6  = 0x08,
	MSG_CHECK_ADDR_TYPE_NUM   = 0x10,
	MSG_CHECK_ADDR_TYPE_ALIAS = 0x20
} MsgAddrCheckType;

typedef struct _mmsTranQEntity {
	bool isCompleted;
	msg_request_id_t reqID;
	int msgId;
	unsigned int sessionId;
	char transactionId[MMS_TR_ID_LEN + 1];

	MMS_PDU_TYPE_T eMmsPduType;
	MMS_HTTP_CMD_TYPE_T eHttpCmdType;

	MMS_PDU_TYPE_T eMmsTransactionStatus;

	int postDataLen;
	char *pPostData;

	int getDataLen;
	char *pGetData;
} mmsTranQEntity;

typedef enum {
	MMS_RM_PDU_TYPE,

	MMS_RM_SEND_REQ,
	MMS_RM_GET_REQ_AUTO,
	MMS_RM_GET_REQ_MANUAL,
	MMS_RM_NOTIFY_RESP_IND,
	MMS_RM_ACK_IND,

	MMS_RM_NOTI_IND,
	MMS_RM_RETRIEVE_CONF,

	MMS_RM_READ_REPORT_V10,
	MMS_RM_READ_REPORT_V11
} MmsRmPduType;

/// CM //////////////////////////////////////////////////////

#define MAX_PROFILE_ID 1

#define MAX_URL_LENGTH 1024
#define MAX_IPV4_LENGTH 30

typedef struct _MMS_NETWORK_PROFILE_S {
	int profileId[MAX_PROFILE_ID];
	int profileCount;
} MMS_NETWORK_PROFILE_S;



/// HTTP ////////////////////////////////////////////////////

enum _E_MMS_CONNECTION_MODE {
	eTCP_WITH_PROXY = 0,  /** Means HTTP Proxy/Gateway are available */
	eTCP_WITHOUT_PROXY = 1   /**  Means HTTP Proxy/Gateway is not available */
};

typedef unsigned int E_MMS_CONNECTION_MODE; /**< Values from \ref  _E_MMS_CONNECTION_MODE   */

enum _E_MMS_NETWORK_ACCESS_POINT {
	eNETWORK_ACCESS_POINT_ACKTIVE = 1
};

typedef unsigned int E_MMS_NETWORK_ACCESS_POINT;

typedef struct  _MMSC_CONFIG_DATA_S {
	char mmscUrl[MAX_URL_LENGTH + 1];			/** if break, change it to NETPM_HOME_URL_LEN_MAX   */
	char httpProxyIpAddr[MAX_IPV4_LENGTH + 1];	/**   HTTP Proxy's URL or IP address */

	unsigned int proxyPortNo;	/** MMS HTTP proxy Port number  */
	E_MMS_CONNECTION_MODE connectionMode;	/**   Values must be from ENUM list -\ref E_MMS_CONNECTION_MODE  */
	E_MMS_NETWORK_ACCESS_POINT networkAccessPoint;	/**   Values must be from \ref E_MMS_NETWORK_ACCESS_POINT  */
	unsigned int bAutoRetrieveFlag;	/**  Value's shall be true or false */
	unsigned int profileId;			/** Profile is for CM Conn open*/
} MMSC_CONFIG_DATA_S;

typedef struct _MMS_PLUGIN_HTTP_DATA_S {
	int profileId;
	int profileCount;
	int currentProfileId;
	int sessionId;
	int transactionId;
	void *session;
	void *sessionHeader;
	MMSC_CONFIG_DATA_S mmscConfig;
} MMS_PLUGIN_HTTP_DATA_S;

typedef struct _MMS_PLUGIN_HTTP_CONTEXT_S {
	char *recv_header_buf ;
	int header_bufsize;
	unsigned char *final_content_buf; // Final Concatenated Buffer
	unsigned long ulContentLength;  // Content Leght Value received in HTTP OK Header
	unsigned long bufOffset; /** < Incremented once received Content_receiving or Content_received Event */
} MMS_PLUGIN_HTTP_CONTEXT_S;

typedef struct _MMS_PLUGIN_PROCESS_DATA_S {
	unsigned char *pData;
	unsigned long dataLen;
} MMS_PLUGIN_PROCESS_DATA_S;


///////////////////////////////////////////////////////////////

#define RETRY_MAX 1

typedef enum {
	MMS_HTTP_GET = 1,		/* GET	*/
	MMS_HTTP_POST = 2		/* POST	*/
} MMS_HTTP_METHOD;

#define MAX_MMSC_URL_LEN	100	/**   MAX URL Length includes NULL char */
#define MAX_HTTP_PROXY_IPADDR_LEN	100	/**   MAX  HTTP Proxy IP addres length  includes NULL char */

#define HTTP_REQUEST_LEN   1024
#define HTTP_VER     "HTTP/1.1"
#define CRLF             "\r\n"
#define CRLFCRLF     "\r\n\r\n"

/*  URL of  IIS Server - Where CGI script located */
#define IIS_PostURI   "/mms/post.exe"
#define OperatorPostUrl   "/mms/"
#define MAX_MMSC_IPADDR_LEN   512
#define	POST_URI   OperatorPostUrl



//////////////////////////////////////////////////////////
#define HTTP_RESP_SUCCESS    200
#define HTTP_PROB_RESP_SUCCESS    100


#define MMS_HTTP_HDR_CONNECTION "Keep-Alive"

/********* HTTP HEADER MACROS *********/
#define MMS_HTTP_HDR_CONTENT_TYPE "application/vnd.wap.mms-message"

#define MMS_HTTP_HDR_USER_AGENT "AX355"

#define MMS_HTTP_HDR_ACCEPT "application/vnd.wap.mms-message"

#define MMS_HTTP_HDR_ACCEPT_LANGUAGE "en"

#define MMS_HTTP_HDR_ACCEPT_CHARSET "US-ASCII, ISO-8859-1, UTF-8"

//  MMSC Address
#define NOW_MMSC_URL    ""
#define NOW_MMSC_IP      ""
#define NOW_MMSC_PROXY   ""
#define NOW_MMSC_PROXY_PORT

#define DEFAULT_MMSC_URL		NOW_MMSC_URL
#define DEFAULT_MMSC_IP		NOW_MMSC_IP
#define DEFAULT_MMSC_PORT
#define DEFAULT_HTTP_PROXY	NOW_MMSC_PROXY
#define DEFAULT_HTTP_PROXY_PORT	NOW_MMSC_PROXY_PORT

typedef struct _S_HTTP_UA_RECVING_DATA {
	int curr_len_recv;
	int total_data_len;
} S_HTTP_UA_RECVING_DATA;

/////////////////////////////////////////////////////

#define IN        /*! Pfrefix*/
#define OUT       /*! Pfrefix*/
#define INOUT     /*! Pfrefix*/

#endif

