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


#include <device/power.h>
#include <device/display.h>

#include "MsgCallStatusManager.h"
#include "MsgDebug.h"
#include "MsgMutex.h"
#include "MsgGconfWrapper.h"
#include "MsgDevicedWrapper.h"

int g_lock_cnt = 0;
Mutex mx;

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
	int callStatus = 0;

	callStatus = MsgGetCallStatus();
	MSG_DEBUG("Call Status = %d", callStatus);

	if (callStatus > 0 && callStatus < 3) {
		MSG_DEBUG("Call is activated. Do not turn on the lcd.");
	} else {
		MSG_DEBUG("Call is not activated. Turn on the lcd.");
		device_display_change_state(DISPLAY_STATE_NORMAL);
	}

	MSG_END();
}
