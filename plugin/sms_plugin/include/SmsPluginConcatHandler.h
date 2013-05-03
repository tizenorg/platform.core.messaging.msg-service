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

#ifndef SMS_PLUGIN_CONCAT_HANDLER_H
#define SMS_PLUGIN_CONCAT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <string>
#include <map>
#include <vector>

using namespace std;

#include "MsgTextConvert.h"
#include "SmsPluginTypes.h"


/*==================================================================================================
                                         VARIABLES AND DEFINES
==================================================================================================*/
#define MAX_CONCAT_LIST	10

struct CONCAT_DATA_S
{
	int	length;
	char data[MAX_USER_DATA_LEN+1];
};

typedef map<unsigned char, CONCAT_DATA_S> concatDataMap;

typedef struct _SMS_CONCAT_MSG_S
{
	unsigned short		msgRef;
	unsigned char		totalSeg;
	unsigned char		seqNum;

	SMS_TIMESTAMP_S	timeStamp;
	SMS_ADDRESS_S	originAddress;
	SMS_DCS_S		dcs;
	bool				bRead;
} SMS_CONCAT_MSG_S;

typedef struct _SMS_CONCAT_INFO_S
{
	unsigned short		msgRef;
	unsigned char		totalSeg;
	unsigned char		segCnt;

	SMS_TIMESTAMP_S	timeStamp;
	SMS_ADDRESS_S	originAddress;
	SMS_DCS_S		dcs;
	bool				bRead;

	unsigned int		totalSize;
	concatDataMap	data;
} SMS_CONCAT_INFO_S;

typedef struct _SMS_SIM_ID_S
{
	unsigned short		msgRef;
	msg_sim_id_t		simId;
} SMS_SIM_ID_S;


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginConcatHandler
{
public:
	static SmsPluginConcatHandler* instance();

	bool IsConcatMsg(SMS_USERDATA_S *pUserData);
	void handleConcatMsg(SMS_TPDU_S *pTpdu);
//	void handleConcatMsg(SMS_TPDU_S *pTpdu, msg_sim_id_t SimMsgId, bool bRead);

//	void handleBrokenMsg();

private:
	SmsPluginConcatHandler();
	virtual ~SmsPluginConcatHandler();

	static SmsPluginConcatHandler* pInstance;

	unsigned char checkConcatMsg(SMS_CONCAT_MSG_S *pConcatMsg, SMS_USERDATA_S *pUserData);
	int makeConcatUserData(unsigned short MsgRef, char **ppTotalData);

	void convertConcatToMsginfo(const SMS_DELIVER_S *pTpdu, const char *pUserData, int DataSize, MSG_MESSAGE_INFO_S *pMsgInfo);
//	void convertSimMsgToMsginfo(const SMS_CONCAT_MSG_S *pConcatMsg, const char *pUserData, int DataSize, MSG_MESSAGE_INFO_S *pMsgInfo);

	void removeFromConcatList(unsigned short MsgRef);

//	void addToSimIdList(unsigned short MsgRef, msg_sim_id_t SimMsgId);
//	void removeFromSimIdList(unsigned short MsgRef);

	vector<SMS_CONCAT_INFO_S> concatList;
//	vector<SMS_SIM_ID_S> simIdList;

	MsgTextConvert textCvt;
};

#endif //SMS_PLUGIN_CONCAT_HANDLER_H
