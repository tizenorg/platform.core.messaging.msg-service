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

#ifndef SMS_PLUGIN_CONCAT_HANDLER_H
#define SMS_PLUGIN_CONCAT_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <string>
#include <map>
#include <vector>

using namespace std;

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
	MSG_SIM_ID_T		simId;
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

private:
	SmsPluginConcatHandler();
	virtual ~SmsPluginConcatHandler();

	static SmsPluginConcatHandler* pInstance;

	unsigned char checkConcatMsg(SMS_CONCAT_MSG_S *pConcatMsg, SMS_USERDATA_S *pUserData);
	int makeConcatUserData(unsigned short MsgRef, char **ppTotalData);

	void convertConcatToMsginfo(const SMS_DELIVER_S *pTpdu, const char *pUserData, int DataSize, MSG_MESSAGE_INFO_S *pMsgInfo);

	void removeFromConcatList(unsigned short MsgRef);


	vector<SMS_CONCAT_INFO_S> concatList;
};

#endif //SMS_PLUGIN_CONCAT_HANDLER_H
