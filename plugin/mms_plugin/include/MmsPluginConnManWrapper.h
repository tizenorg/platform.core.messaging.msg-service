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

#ifndef MMS_PLUGIN_CONNMAN_H
#define MMS_PLUGIN_CONNMAN_H

#include <network-cm-intf.h>
#include <network-pm-intf.h>
#include "MsgMutex.h"
#include <glib.h>
#include "net_connection.h"

class MmsPluginCmAgent
{
public:
	static MmsPluginCmAgent *instance();

	/* open and close are occasionally called when transaction begins and ends */
	bool open();
	void close();

	bool getCmStatus() { return isCmOpened; }

	void open_callback(connection_error_e result, void* user_data);
	void close_callback(connection_error_e result, void* user_data);

	bool getInterfaceName(const char **deviceName);
	bool getProxyAddr(const char **proxyAddr);
	bool getHomeUrl(const char **homeURL);
private:
	MmsPluginCmAgent();
	~MmsPluginCmAgent();

	static MmsPluginCmAgent *pInstance;

	void lock() { mx.lock(); }
	void unlock() { mx.unlock(); }
	void signal() { cv.signal(); }
	void setCmStatus() { isCmOpened = true; }
	void resetCmStatus() { isCmOpened = false; }

	bool isCmOpened;
	bool isCmRegistered;

	char *home_url;
	char *interface_name;
	char *proxy_address;
	Mutex mx;
	CndVar cv;

};

#endif //MMS_PLUGIN_CONNMAN_H
