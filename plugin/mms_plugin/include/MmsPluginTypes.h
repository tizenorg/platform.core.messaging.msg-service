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

#ifndef MMS_PLUGIN_TYPES_H
#define MMS_PLUGIN_TYPES_H

#include "MsgTypes.h"
#include "MsgInternalTypes.h"

#define IN        /*! Pfrefix*/
#define OUT       /*! Pfrefix*/
#define INOUT     /*! Pfrefix*/

#define RETRY_MAX 1
#define HTTP_REQUEST_LEN   1024

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

typedef struct _mmsTranQEntity {
	bool isCompleted;
	msg_request_id_t reqID;
	int msgId;
	unsigned int sessionId;
	char transactionId[MMS_TR_ID_LEN + 1];
	unsigned int simId;

	MMS_PDU_TYPE_T eMmsPduType;
	MMS_HTTP_CMD_TYPE_T eHttpCmdType;

	MMS_PDU_TYPE_T eMmsTransactionStatus;

	int postDataLen;
	char *pPostData;

	int getDataLen;
	char *pGetData;
} mmsTranQEntity;

#endif //MMS_PLUGIN_TYPES_H

