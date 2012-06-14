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

#ifndef SMS_PLUGIN_SIMMSG_H
#define SMS_PLUGIN_SIMMSG_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgMutex.h"

#include "MsgStorageTypes.h"
#include "SmsPluginTypes.h"
#include "MsgInternalTypes.h"

extern "C"
{
#ifndef _TAPI_NETTEXT_H_
	#include "ITapiNetText.h"
	#include "ITapiSim.h"
#endif
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSimMsg
{
public:
	static SmsPluginSimMsg* instance();

	void initSimMessage();
	MSG_ERROR_T saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);
	MSG_ERROR_T saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo);
	void deleteSimMessage(MSG_SIM_ID_T SimMsgId);
	void getSimMessageList(MSG_SIM_MSG_INFO_LIST_S **ppSimMsgList);
	void setReadStatus(MSG_SIM_ID_T SimMsgId);
	bool checkSimMsgFull(unsigned int SegCnt);

	void setSimMsgCntEvent(const MSG_SIM_COUNT_S *pSimMsgCnt);
	void setSimMsgEvent(const MSG_MESSAGE_INFO_S *pMsgInfo, bool bSuccess);
	void setSimEvent(MSG_SIM_ID_T SimId, bool bResult);

private:
	SmsPluginSimMsg();
	~SmsPluginSimMsg();

	void getSimMsgCount(MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsg(MSG_SIM_ID_T SimMsgId, MSG_MESSAGE_INFO_S* pMsgInfo);

	void setSmsOptions(SMS_DELIVER_S *pDeliver);
	void convertTimeStamp(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver);

	bool getSimMsgCntEvent(MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsgEvent(MSG_MESSAGE_INFO_S *pMsgInfo);
	bool getSimEvent(MSG_SIM_ID_T *pSimId);

	static SmsPluginSimMsg* pInstance;

	MSG_SIM_ID_T 			simMsgId;

	MSG_SIM_COUNT_S			simMsgCnt;

	MSG_MESSAGE_INFO_S		simMsgInfo;

	unsigned int 				usedCnt;
	unsigned int 				totalCnt;

	bool						bTapiResult;
	bool						bClass2Msg;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_SIMMSG_H

