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

#ifndef MMS_PLUGIN_CONNMAN_H
#define MMS_PLUGIN_CONNMAN_H

#include "MsgMutex.h"
#include "net_connection.h"

typedef enum {
	MSG_CM_ERR_NONE = 0,
	MSG_CM_ERR_UNKNOWN,
 } cm_error_e;

class MmsPluginCmAgent {
public:
	static MmsPluginCmAgent *instance();

	bool open();
	void close();

	bool getCmStatus() { return isCmOpened; }

	void connection_profile_open_callback(connection_error_e result, void* user_data);
	void connection_profile_close_callback(connection_error_e result, void* user_data);
	void connection_profile_state_changed_cb(connection_profile_state_e state, void* user_data);
	bool getInterfaceName(const char **deviceName);
	bool getProxyAddr(const char **proxyAddr);
	bool getHomeUrl(const char **homeURL);
	bool getDnsAddrList(const char **dnsAddrList);

private:
	MmsPluginCmAgent();
	~MmsPluginCmAgent();

	static MmsPluginCmAgent *pInstance;

	void lock() { mx.lock(); }
	void unlock() { mx.unlock(); }
	void signal() { cv.signal(); }

	void setCmStatus() { isCmOpened = true; }
	void resetCmStatus() { isCmOpened = false; }

	bool isCmOpened; /* connection & profile connect */
	bool waitProfileOpen;
	char *home_url;
	char *interface_name;
	char *proxy_address;
	char *dns_address_list;
	MsgMutex mx;
	MsgCndVar cv;
};

#endif /* MMS_PLUGIN_CONNMAN_H */
