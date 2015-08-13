/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef SMS_CDMA_PLUGIN_EVENT_HANDLER_H
#define SMS_CDMA_PLUGIN_EVENT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <string>
#include <map>
#include <vector>
#include <list>

using namespace std;

#include "MsgMutex.h"
#include "MsgTextConvert.h"
#include "MsgPluginInterface.h"
#include "SmsCdmaPluginTypes.h"


/*==================================================================================================
                                     VARIABLES AND DEFINES
==================================================================================================*/
struct wap_data_s
{
	int	length;
	char data[SMS_MAX_USER_DATA_LEN+1];
};

typedef map<unsigned char, wap_data_s> wapDataMap;

typedef struct _sms_wap_msg_s
{
	unsigned short	msgId;
	unsigned char		totalSeg;
	unsigned char		segNum;
} sms_wap_msg_s;

typedef struct _sms_wap_info_s
{
	unsigned short	msgId;
	unsigned char		totalSeg;
	unsigned char		segNum;

	unsigned int		totalSize;
	wapDataMap			data;
} sms_wap_info_s;



/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginEventHandler
{
public:
	static SmsPluginEventHandler* instance();

	void registerListener(MSG_PLUGIN_LISTENER_S *pListener);
	void handleSentStatus(msg_network_status_t NetStatus);
	void handleMsgIncoming(sms_trans_p2p_msg_s *p_p2p_msg);
	void handleCbMsgIncoming(sms_trans_broadcast_msg_s *p_cb_msg);
	void handleWapMsgIncoming(sms_trans_p2p_msg_s *p_p2p_msg);

	void handleResendMessage(void);

	msg_error_t callbackMsgIncoming(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t callbackStorageChange(msg_storage_change_type_t storageChangeType, MSG_MESSAGE_INFO_S *pMsgInfo);

	void convertTpduToMsginfo(sms_trans_p2p_msg_s *p_p2p_msg, MSG_MESSAGE_INFO_S *p_msg_info);
	void convertTpduToMsginfo(sms_trans_broadcast_msg_s *p_cb_msg, MSG_MESSAGE_INFO_S *p_msg_info);

	void SetSentInfo(sms_sent_info_s *pSentInfo);

	void setDeviceStatus();
	bool getDeviceStatus();
	void setNeedInitConfig(bool bNeeded);
	bool getNeedInitConfig();

	void handleSyncMLMsgIncoming(msg_syncml_message_type_t msgType, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen);
	void handleLBSMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen);
	void handlePushMsgIncoming(char* pPushHeader, char* pPushBody, int pushBodyLen, char *app_id, char *content_type);

private:
	SmsPluginEventHandler();
	virtual ~SmsPluginEventHandler();

	static SmsPluginEventHandler* pInstance;

	MSG_PLUGIN_LISTENER_S listener;

	sms_sent_info_s sentInfo;

	bool devStatus;
	bool bNeedInitConfig;

	Mutex mx;
	CndVar cv;
	vector<sms_wap_info_s> wapList;

	void convertDeliverMsgToMsgInfo(sms_telesvc_deliver_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info);
	void convertCMASMsgToMsgInfo(sms_telesvc_deliver_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info);
	void convertAckMsgToMsgInfo(sms_telesvc_deliver_ack_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info);
	void convertReportMsgToMsgInfo(sms_telesvc_report_s *p_deliver, MSG_MESSAGE_INFO_S *p_msg_info);

	msg_encode_type_t getEncodeType(sms_encoding_type_t encode_type);

	unsigned short checkWapMsg(sms_wap_msg_s *pMsg, sms_telesvc_userdata_s *pUserdata);
	int MakeWapUserData(unsigned short msgId, char **ppTotalData);

	bool checkCbOpt(sms_trans_svc_ctg_t svc_ctg);

};

#endif //SMS_CDMA_PLUGIN_EVENT_HANDLER_H

