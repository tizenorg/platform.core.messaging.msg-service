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


#ifndef MSG_WEARABLE_PROFILE
#include <bundle.h>
#endif /* MSG_WEARABLE_PROFILE */
#include <device/power.h>

#include "MsgCallStatusManager.h"
#include "MsgDebug.h"
#include "MsgMutex.h"
#include "MsgGconfWrapper.h"
#include "MsgDevicedWrapper.h"
#include "MsgUtilFunction.h"

int g_lock_cnt = 0;
MsgMutex mx;

void MsgDisplayLock()
{
	MSG_BEGIN();

	int ret = 0;

	mx.lock();

	if (g_lock_cnt <= 0) {
		ret = device_power_request_lock(POWER_LOCK_CPU, 0);
		if (ret < 0) {
			MSG_DEBUG("device_power_request_lock() is failed [%d]", ret);
		}
	}

	g_lock_cnt++;

	mx.unlock();

	MSG_DEBUG("Display lock count = [%d]", g_lock_cnt);

	MSG_END();
}


void MsgDisplayUnlock()
{
	MSG_BEGIN();

	int ret = 0;

	mx.lock();

	g_lock_cnt--;

	MSG_DEBUG("Display lock count = [%d]", g_lock_cnt);

	if (g_lock_cnt <= 0) {
		ret = device_power_release_lock(POWER_LOCK_CPU);
		if (ret < 0) {
			MSG_DEBUG("device_power_release_lock() is failed [%d]", ret);
		}
	}

	mx.unlock();

	MSG_END();
}


void MsgChangePmState()
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE
	bundle *bundle_data = bundle_create();

	bundle_add_str(bundle_data, "cmd", "change_pm_state");

	msg_launch_app(MSG_MGR_APP_ID, bundle_data);

	bundle_free(bundle_data);
#endif /* MSG_WEARABLE_PROFILE */
	MSG_END();
}
