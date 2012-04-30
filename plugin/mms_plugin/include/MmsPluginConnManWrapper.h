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

#ifndef MMSPLUGINCONNMANWRAPPER_H
#define MMSPLUGINCONNMANWRAPPER_H

#include <network-cm-intf.h>
#include <network-pm-intf.h>

#include "MsgDebug.h"

#include "MmsPluginTypes.h"
#include "MsgMutex.h"


class MmsPluginCmAgent
{
public:
	static MmsPluginCmAgent *instance();

	/* open and close are occasionally called when transaction begins and ends */
	bool open();
	void close();

	bool getCmStatus() { return isCmOpened; }

	void processCBdatas(net_event_info_t *event_cb, void *user_data);

	bool getDeviceName(char *deviceName);
	bool getHomeURL(char *homeURL);
	bool getProxyAddr(char *proxyAddr);
	int getProxyPort();

private:
	MmsPluginCmAgent();
	~MmsPluginCmAgent();

	static MmsPluginCmAgent *pInstance;

	/* register/deregister is called once at initialization/finalization of MMS plugin */
	bool registration();
	void deregistration();


	void lock() { mx.lock(); }
	void unlock() { mx.unlock(); }
	void signal() { cv.signal(); }
	void setCmStatus() { isCmOpened = true; }
	void resetCmStatus() { isCmOpened = false; }

	// shared variable between CmAgent and Dnet callback
	bool isCmOpened;
	Mutex mx;
	CndVar cv;

	net_profile_info_t mmsProfile;
};

#endif

