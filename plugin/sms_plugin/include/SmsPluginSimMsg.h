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

	void initSimMessage(struct tapi_handle *handle);
	msg_error_t saveSimMessage(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);
	msg_error_t saveClass2Message(const MSG_MESSAGE_INFO_S *pMsgInfo);
	void deleteSimMessage(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId);
	void setReadStatus(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId);
	bool checkSimMsgFull(msg_sim_slot_id_t sim_idx, unsigned int SegCnt);

	void setSimMsgCntEvent(struct tapi_handle *handle, const MSG_SIM_COUNT_S *pSimMsgCnt);
	void setSimMsgEvent(struct tapi_handle *handle, const MSG_MESSAGE_INFO_S *pMsgInfo, bool bSuccess);
	void setSaveSimMsgEvent(struct tapi_handle *handle, int simId, int result);
	void setSaveClass2MsgEvent(struct tapi_handle *handle, int simId, int result, MSG_MESSAGE_INFO_S *data);
	void setSimEvent(msg_sim_id_t SimId, bool bResult);
	void setUpdateSimEvent(int SimId, bool bResult);
	void setDelSimEvent(int SimId, bool bResult);

	void setSmsData(const char *sca, const char *szData, int msgLength);
	void setSmsTpduTotalSegCount(int totalSeg);

private:
	SmsPluginSimMsg();
	~SmsPluginSimMsg();

	void getSimMsgCount(struct tapi_handle *handle, MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsg(struct tapi_handle *handle, msg_sim_id_t SimMsgId, MSG_MESSAGE_INFO_S* pMsgInfo, int *simIdList);

	void setSmsOptions(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver);
	void convertTimeStamp(const MSG_MESSAGE_INFO_S* pMsgInfo, SMS_DELIVER_S *pDeliver);

	bool getSimMsgCntEvent(struct tapi_handle *handle, MSG_SIM_COUNT_S *pSimMsgCnt);
	bool getSimMsgEvent(struct tapi_handle *handle, MSG_MESSAGE_INFO_S *pMsgInfo);
	bool getSimEvent(msg_sim_id_t *pSimId);
	bool getUpdateSimEvent();
	bool getDelSimEvent(int *pSimId);


	static SmsPluginSimMsg* pInstance;

	msg_sim_id_t				simMsgId;
	int				delSimMsgId;
	int		simIdList[MAX_SIM_SMS_NUM];		/** send total simIds to handleSimMsg **/

	MSG_SIM_COUNT_S			simMsgCnt;

	MSG_MESSAGE_INFO_S		simMsgInfo;
	MSG_ADDRESS_INFO_S		simAddrInfo;

	int					usedCnt;
	int					totalCnt;

	bool						bTapiResult;

	SMS_DATA_INFO_S			simMsgDataInfo;

	MsgMutex mx;
	MsgCndVar cv;
};

#endif /* SMS_PLUGIN_SIMMSG_H */

