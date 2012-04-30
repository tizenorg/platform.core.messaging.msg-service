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

