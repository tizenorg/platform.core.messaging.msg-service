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

#ifndef SMS_PLUGIN_CBMSG_HANDLER_H
#define SMS_PLUGIN_CBMSG_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <map>
#include <vector>

using namespace std;

#include "SmsPluginTypes.h"

extern "C"
{
#ifndef _TAPI_NETTEXT_H_
	#include "ITapiNetText.h"
//	#include "ITapi_common.h"
#endif
}


/*==================================================================================================
                                         VARIABLES
==================================================================================================*/
typedef map<unsigned char, string> cbPageMap;


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

	void handleCbMsg(TelSmsCbMsg_t *pCbMsg);

private:
	SmsPluginCbMsgHandler();
	virtual ~SmsPluginCbMsgHandler();

	static SmsPluginCbMsgHandler* pInstance;

	void Decode2gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage);
	void Decode3gCbMsg(TelSmsCbMsg_t *pCbMsg, SMS_CBMSG_PAGE_S *pCbPage);

	bool checkCbOpt(SMS_CBMSG_PAGE_S CbPage, bool *pJavaMsg);
	unsigned char checkCbPage(SMS_CBMSG_PAGE_S CbPage);
	void MakeCbMsg(SMS_CBMSG_PAGE_S CbPage, SMS_CBMSG_S *pCbMsg);
	void convertCbMsgToMsginfo(SMS_CBMSG_S cbMsg, MSG_MESSAGE_INFO_S *pMsgInfo);
	void addToPageLiat(SMS_CBMSG_PAGE_S CbPage);
	void removeFromPageList(SMS_CBMSG_PAGE_S CbPage);

	void decodeCbMsgDCS(unsigned char dcsData, const unsigned char *pMsgData, SMS_CBMSG_DCS_S* pDcs);
	void convertLangType(SMS_CBMSG_LANG_TYPE_T InType , MSG_CB_LANGUAGE_TYPE_T *pOutType);
	unsigned long getRecvTime();
	void getDisplayName(unsigned short	MsgId, char *pDisplayName);

	vector<CB_PAGE_INFO_S> pageList;
};

#endif //SMS_PLUGIN_CBMSG_HANDLER_H

