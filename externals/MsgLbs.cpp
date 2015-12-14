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


#include "MsgDebug.h"

extern "C"
{
	#include <lbs_agps.h>
}

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/


void MsgLbsSms(const char *msgBody, int msgSize) {
	int ret = LBS_AGPS_ERROR_NONE;

	ret = lbs_agps_sms(msgBody, msgSize);

	if (ret != LBS_AGPS_ERROR_NONE)
		MSG_DEBUG("lbs_agps_sms failed. [%d]", ret);
}


void MsgLbsWapPush(const char *pushHeader, const char *pushBody, int pushBodySize) {
	int ret = LBS_AGPS_ERROR_NONE;

	ret = lbs_agps_wap_push(pushHeader, pushBody, pushBodySize);

	if (ret != LBS_AGPS_ERROR_NONE)
		MSG_DEBUG("lbs_agps_wap_push failed. [%d]", ret);
}
