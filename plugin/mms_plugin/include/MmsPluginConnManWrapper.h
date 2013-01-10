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

#ifndef MMS_PLUGIN_CONNMAN_H
#define MMS_PLUGIN_CONNMAN_H

#include <network-cm-intf.h>
#include <network-pm-intf.h>
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

#endif //MMS_PLUGIN_CONNMAN_H
