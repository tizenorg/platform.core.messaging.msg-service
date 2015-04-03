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

#ifndef SMS_PLUGIN_CBMSG_HANDLER_H
#define SMS_PLUGIN_CBMSG_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <map>
#include <vector>

using namespace std;

#include "MsgTextConvert.h"
#include "SmsPluginTypes.h"

extern "C"
{
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <ITapiNetText.h>
}


/*==================================================================================================
                                         VARIABLES
==================================================================================================*/
typedef map<unsigned char, SMS_CBMSG_PAGE_S> cbPageMap;


typedef struct _CB_PAGE_INFO_S
{
	unsigned char		geoScope;
	unsigned char		updateNum;
	unsigned short		msgCode;
	unsigned short		msgId;
	unsigned char		totalPages;

	unsigned char		pageCnt;
	unsigned int		totalSize;
	cbPageMap		data;
} CB_PAGE_INFO_S;


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginCbMsgHandler
{
public:
	static SmsPluginCbMsgHandler* instance();

	void handleCbMsg(TapiHandle *handle, TelSmsCbMsg_t *pCbMsg);
	void handleEtwsMsg(TapiHandle *handle, TelSmsEtwsMsg_t *pEtwsMsg);

private:
	SmsPluginCbMsgHandler();
	virtual ~SmsPluginCbMsgHandler();

	static SmsPluginCbMsgHandler* pInstance;

	void Decode2gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage);
	void Decode3gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage);
	void DecodeEtwsMsg(TelSmsEtwsMsg_t *pEtwsMsg, SMS_ETWS_PRIMARY_S *pEtwsPn);
	unsigned short encodeCbSerialNum ( SMS_CBMSG_SERIAL_NUM_S snFields );
	int CMAS_class(unsigned short message_id);

	bool checkCbOpt(SMS_CBMSG_PAGE_S CbPage, bool *pJavaMsg, msg_sim_slot_id_t simIndex);
	unsigned char checkCbPage(SMS_CBMSG_PAGE_S CbPage);
	void MakeCbMsg(SMS_CBMSG_PAGE_S CbPage, SMS_CBMSG_S *pCbMsg);
	void convertCbMsgToMsginfo(SMS_CBMSG_S *pCbMsg, MSG_MESSAGE_INFO_S *pMsgInfo, msg_sim_slot_id_t simIndex);
	void convertEtwsMsgToMsginfo(SMS_CBMSG_PAGE_S EtwsMsg, MSG_MESSAGE_INFO_S *pMsgInfo, msg_sim_slot_id_t simIndex);
	int convertTextToUtf8 (unsigned char* outBuf, int outBufSize, SMS_CBMSG_S* pCbMsg);
	void addToPageList(SMS_CBMSG_PAGE_S CbPage);
	void removeFromPageList(SMS_CBMSG_PAGE_S CbPage);

	void decodeCbMsgDCS(unsigned char dcsData, const unsigned char *pMsgData, SMS_CBMSG_DCS_S* pDcs);
	void convertLangType(SMS_CBMSG_LANG_TYPE_T InType , MSG_CB_LANGUAGE_TYPE_T *pOutType);
	unsigned long getRecvTime();
	void getDisplayName(unsigned short	MsgId, char *pDisplayName, msg_sim_slot_id_t simIndex);

	vector<CB_PAGE_INFO_S> pageList;

};

#endif //SMS_PLUGIN_CBMSG_HANDLER_H

