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
	#include <tapi_common.h>
	#include <TelSms.h>
	#include <TapiUtility.h>
	#include <ITapiNetText.h>
}

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginSimMsg
{
public:
	static SmsPluginSimMsg* instance();

	void initSimMessage();
	msg_error_t saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);
	msg_error_t saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo);
	void deleteSimMessage(msg_sim_id_t SimMsgId);
	void getSimMessageList(MSG_SIM_MSG_INFO_LIST_S **ppSimMsgList);
	void setReadStatus(msg_sim_id_t SimMsgId);
	bool checkSimMsgFull(unsigned int SegCnt);

	void setSimMsgCntEvent(const MSG_SIM_COUNT_S *pSimMsgCnt);
	void setSimMsgEvent(const MSG_MESSAGE_INFO_S *pMsgInfo, bool bSuccess);
	void setSaveSimMsgEvent(int simMsgId, int result);
	void setSaveClass2MsgEvent(int simMsgId, int result);
	void setSimEvent(msg_sim_id_t SimId, bool bResult);

	void setSmsData(const char *sca, const char *szData, int msgLength);

private:
	SmsPluginSimMsg();
	~SmsPluginSimMsg();

	void getSimMsgCount(MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsg(msg_sim_id_t SimMsgId, MSG_MESSAGE_INFO_S* pMsgInfo);

	void setSmsOptions(SMS_DELIVER_S *pDeliver);
	void convertTimeStamp(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver);

	bool getSimMsgCntEvent(MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsgEvent(MSG_MESSAGE_INFO_S *pMsgInfo);
	bool getSimEvent(msg_sim_id_t *pSimId);

	static SmsPluginSimMsg* pInstance;

	msg_sim_id_t				simMsgId;

	MSG_SIM_COUNT_S			simMsgCnt;

	MSG_MESSAGE_INFO_S		simMsgInfo;

	unsigned int 				usedCnt;
	unsigned int 				totalCnt;

	bool						bTapiResult;

	SMS_DATA_INFO_S			simMsgDataInfo;

	Mutex mx;
	CndVar cv;
};

#endif //SMS_PLUGIN_SIMMSG_H

