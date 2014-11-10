/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <errno.h>
#include "MmsPluginConnManWrapper.h"
#include "MmsPluginDebug.h"
#include "MmsPluginHttp.h"
#include <glib.h>
#include "net_connection.h"

#define MMS_CONTEXT_INVOKE_WAIT_TIME	30
#define MMS_CONNECTION_API_WAIT_TIME	50

#define MMS_FREE(obj)\
	if (obj){\
		free(obj);\
		obj = NULL;\
	}


static Mutex g_mx;
static CndVar g_cv;
static connection_h connection = NULL;

void __connection_profile_print(connection_profile_h profile)
{
	int ret;
	char *profile_id = NULL;
	char *profile_name = NULL;
	char *interface_name = NULL;
	char *ip_address = NULL;
	char *subnet_mask = NULL;
	char *gateway_address = NULL;
	char *dns_address = NULL;
	char *proxy_address = NULL;
	char *apn = NULL;
	char *user_name = NULL;
	char *password = NULL;
	char *home_url = NULL;
	bool is_roaming;

	connection_profile_type_e profile_type;
	connection_profile_state_e profile_state;
	connection_ip_config_type_e ip_type = CONNECTION_IP_CONFIG_TYPE_NONE;
	connection_proxy_type_e proxy_type;
	connection_cellular_service_type_e service_type = CONNECTION_CELLULAR_SERVICE_TYPE_UNKNOWN;
	connection_cellular_auth_type_e auth_type = CONNECTION_CELLULAR_AUTH_TYPE_NONE;

	MSG_DEBUG("**************************************************************************************************");
	ret = connection_profile_get_id(profile, &profile_id);
	MSG_DEBUG("Profile Id = [%s]", profile_id);

	ret = connection_profile_get_name(profile, &profile_name);
	MSG_DEBUG("Profile Name = [%s]", profile_name);

	ret = connection_profile_get_type(profile, &profile_type);

	if (profile_type == CONNECTION_PROFILE_TYPE_CELLULAR) {
		MSG_DEBUG("Profile Type = [CELLULAR]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_WIFI) {
		MSG_DEBUG("Profile Type = [WIFI]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_ETHERNET) {
		MSG_DEBUG("Profile Type = [ETHERNET]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_BT) {
		MSG_DEBUG("Profile Type = [BT]");
	} else {
		MSG_DEBUG("Profile Type = Unknown [%d]", profile_type);
	}

	ret = connection_profile_get_network_interface_name(profile, &interface_name);
	MSG_DEBUG("Profile Interface Name = [%s]", interface_name);

	ret = connection_profile_get_state(profile, &profile_state);
	if (profile_state == CONNECTION_PROFILE_STATE_DISCONNECTED) {
		MSG_DEBUG("Profile State = [DISCONNECTED]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_ASSOCIATION) {
		MSG_DEBUG("Profile State = [ASSOCIATION]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_CONFIGURATION) {
		MSG_DEBUG("Profile State = [CONFIGURATION]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_CONNECTED) {
		MSG_DEBUG("Profile State = [CONNECTED]");
	} else {
		MSG_DEBUG("Profile State = Unknown [%d]", profile_state);
	}

	ret = connection_profile_get_ip_config_type(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &ip_type);
	MSG_DEBUG("Profile Ip Config Type = [%d]", ip_type);

	ret = connection_profile_get_ip_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &ip_address);
	MSG_DEBUG("Profile Ip Address = [%s]", ip_address);

	ret = connection_profile_get_subnet_mask(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &subnet_mask);
	MSG_DEBUG("Profile Subnet Mask = [%s]", subnet_mask);

	ret = connection_profile_get_gateway_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &gateway_address);
	MSG_DEBUG("Profile Gateway Address = [%s]", gateway_address);

	ret = connection_profile_get_dns_address(profile, 1, CONNECTION_ADDRESS_FAMILY_IPV4, &dns_address);
	MSG_DEBUG("Profile Dns Address = [%s]", dns_address);

	ret = connection_profile_get_proxy_type(profile, &proxy_type);
	MSG_DEBUG("Profile Proxy Type = [%d]", proxy_type);

	ret = connection_profile_get_proxy_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);
	MSG_DEBUG("Profile Proxy Address = [%s]", proxy_address);

	ret = connection_profile_get_cellular_service_type(profile, &service_type);
	if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET) {
		MSG_DEBUG("Profile Service Type = [INTERNET]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_MMS) {
		MSG_DEBUG("Profile Service Type = [MMS]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_PREPAID_INTERNET) {
		MSG_DEBUG("Profile Service Type = [PREPAID_INTERNET]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_PREPAID_MMS) {
		MSG_DEBUG("Profile Service Type = [PREPAID_MMS]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_TETHERING) {
		MSG_DEBUG("Profile Service Type = [TETHERING]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_APPLICATION) {
		MSG_DEBUG("Profile Service Type = [APPLICATION]");
	} else {
		MSG_DEBUG("Profile Service Type = [Unknown][%d]", service_type);
	}

	ret = connection_profile_get_cellular_apn(profile, &apn);
	MSG_DEBUG("Profile Apn = [%s]", apn);

	ret = connection_profile_get_cellular_auth_info(profile, &auth_type, &user_name, &password);
	MSG_DEBUG("Profile Auth Type = [%d]", &auth_type);
	MSG_DEBUG("Profile Auth Name = [%s]", &user_name);
	MSG_DEBUG("Profile Auth Passward = [%s]", &password);

	ret = connection_profile_get_cellular_home_url(profile, &home_url);
	MSG_DEBUG("Profile Home Url = [%s]", home_url);

	ret = connection_profile_is_cellular_roaming(profile, &is_roaming);
	MSG_DEBUG("Profile Roaming = [%d]", is_roaming);
	MSG_DEBUG("**************************************************************************************************");

	MMS_FREE(profile_id);
	MMS_FREE(profile_name);
	MMS_FREE(interface_name);
	MMS_FREE(ip_address);
	MMS_FREE(subnet_mask);
	MMS_FREE(gateway_address);
	MMS_FREE(dns_address);
	MMS_FREE(proxy_address);
	MMS_FREE(apn);
	MMS_FREE(user_name);
	MMS_FREE(password);
	MMS_FREE(home_url);
}

static void __connection_type_changed_cb(connection_type_e type, void* user_data)
{
	MSG_DEBUG("Type changed callback, connection type : %d", type);
}

static void __connection_ip_changed_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	MSG_DEBUG("IP changed callback, IPv4 address : %s, IPv6 address : %s",
			ipv4_address, (ipv6_address ? ipv6_address : "NULL"));
}

static void __connection_proxy_changed_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	MSG_DEBUG("Proxy changed callback, IPv4 address : %s, IPv6 address : %s",
			ipv4_address, (ipv6_address ? ipv6_address : "NULL"));
}

static void __connection_profile_opened_cb(connection_error_e result, void* user_data)
{
	if (result == CONNECTION_ERROR_NONE || result == CONNECTION_ERROR_ALREADY_EXISTS)
		MSG_DEBUG("Connection open Succeeded [%d]", result);
	else
		MSG_DEBUG("Connection open Failed, err : %d", result);

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	cmAgent->open_callback(result, user_data);
}

static void __connection_profile_closed_cb(connection_error_e result, void* user_data)
{
	if (result ==  CONNECTION_ERROR_NONE)
		MSG_DEBUG("Connection close Succeeded");
	else
		MSG_DEBUG("Connection close Failed, err : %d", result);

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	cmAgent->close_callback(result, user_data);
}

static gboolean __connection_create(void *pVoid)
{
	MSG_BEGIN();

	bool ret = false;
	bool *ret_val = (bool *)pVoid;

	if (connection) {
		MSG_DEBUG("connection already exist");
		ret = true;
	} else {
		int err = connection_create(&connection);

		if (CONNECTION_ERROR_NONE == err) {
			connection_set_type_changed_cb(connection, __connection_type_changed_cb, NULL);
			connection_set_ip_address_changed_cb(connection, __connection_ip_changed_cb, NULL);
			connection_set_proxy_address_changed_cb(connection, __connection_proxy_changed_cb, NULL);
			ret = true;
			MSG_DEBUG("Client registration success [%p] ", connection);
		} else {
			MSG_DEBUG("Client registration failed %d", err);
		}
	}

	if (ret_val) {
		*ret_val = ret;
	}

	MSG_END();
	return FALSE;
}

static gboolean __connection_destroy(void *pVoid)
{
	MSG_BEGIN();

	int rv;
	int netOpenResult = NET_ERR_NONE;

	if (connection != NULL) {
		rv = connection_destroy(connection);
		connection = NULL;
		MSG_DEBUG("connection destory !!");
	} else {
		MSG_DEBUG("Cannot connection destroy : Handle is NULL");
		rv = CONNECTION_ERROR_INVALID_OPERATION;
	}

	MSG_END();
	return FALSE;
}

static gboolean __connection_profile_open(void *pVoid)
{
	MSG_BEGIN();

	int netOpenResult = NET_ERR_NONE;
	int *ret_val = (int *)pVoid;

	connection_profile_h profile;
	int err;

	err = connection_get_default_cellular_service_profile(connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &profile);

	if (err != CONNECTION_ERROR_NONE) {
		MSG_DEBUG("connection_get_default_cellular_service_profile Failed!! [%d]", err);
		netOpenResult = NET_ERR_UNKNOWN;
	} else {

		if (connection_open_profile(connection, profile, __connection_profile_opened_cb, NULL) != CONNECTION_ERROR_NONE) {
			MSG_DEBUG("Connection open Failed!!");
			netOpenResult = NET_ERR_UNKNOWN;
		}
	}

	connection_profile_destroy(profile);

	if (ret_val) {
		*ret_val = netOpenResult;
	}

	MSG_END();

	return FALSE;
}

static gboolean __connection_profile_close(void *pVoid)
{
	MSG_BEGIN();

	int netOpenResult = NET_ERR_NONE;

	int *ret_val = (int *)pVoid;

	connection_profile_h profile;
	int err;

	err = connection_get_default_cellular_service_profile(connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &profile);

	if (err  != CONNECTION_ERROR_NONE) {
		MSG_DEBUG("connection_get_default_cellular_service_profile Failed!! [%d]", err);
		netOpenResult = NET_ERR_UNKNOWN;
	} else {

		if (connection_close_profile(connection, profile, __connection_profile_closed_cb, NULL) != CONNECTION_ERROR_NONE) {
			MSG_DEBUG("Connection close Failed!!");
			netOpenResult = NET_ERR_UNKNOWN;
		}

	}

	connection_profile_destroy(profile);

	if (ret_val) {
		*ret_val = netOpenResult;
	}

	MSG_END();

	return FALSE;
}


void context_invoke_end_cb(gpointer data)
{
	g_mx.lock();

	MSG_DEBUG("@@ SIGNAL @@");

	g_cv.signal();

	g_mx.unlock();
}

/*
 * Network api should run at g_main_loop to receive callback
 * */
void context_invoke(GSourceFunc func, void *ret)
{
	MSG_BEGIN();

	int time_ret = 0;

	g_mx.lock();

	g_main_context_invoke_full(NULL, G_PRIORITY_HIGH, func, ret,  context_invoke_end_cb);

	MSG_DEBUG("@@ WAIT @@");

	time_ret = g_cv.timedwait(g_mx.pMutex(), MMS_CONTEXT_INVOKE_WAIT_TIME);
	g_mx.unlock();

	if (time_ret == ETIMEDOUT) {
		MSG_DEBUG("@@ WAKE by timeout@@");
	} else {
		MSG_DEBUG("@@ WAKE by signal@@");
	}

	MSG_END();
}

MmsPluginCmAgent *MmsPluginCmAgent::pInstance = NULL;

MmsPluginCmAgent *MmsPluginCmAgent::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginCmAgent();

	return pInstance;
}

MmsPluginCmAgent::MmsPluginCmAgent()
{
	MSG_BEGIN();

	isCmOpened = false;

	isCmRegistered = false;

	home_url = NULL;
	interface_name = NULL;
	proxy_address = NULL;
	MSG_END();
}

MmsPluginCmAgent::~MmsPluginCmAgent()
{
	MMS_FREE(home_url);
	MMS_FREE(interface_name);
	MMS_FREE(proxy_address);
}

bool MmsPluginCmAgent::open()
{
	MSG_BEGIN();

	int netOpenResult = NET_ERR_NONE;

	lock();

	if (isCmOpened == false) {

		isCmRegistered = false;

		context_invoke(__connection_create, &isCmRegistered);

		if (isCmRegistered == true) {

			MSG_DEBUG("net_open_connection for MMS");

			context_invoke(__connection_profile_open, &netOpenResult);

			if (netOpenResult == NET_ERR_NONE) {

				MSG_DEBUG("## WAITING UNTIL Network Connection Open. ##");

				int time_ret = 0;

				time_ret = cv.timedwait(mx.pMutex(), MMS_CONNECTION_API_WAIT_TIME); // isCmOpened will changed by processCBdatas

				MSG_DEBUG("## WAKE ##");

				if (time_ret == ETIMEDOUT) {
					MSG_DEBUG("Network Connection Open Time Out.");
				}

				if(!isCmOpened) {
					MSG_DEBUG("Network Connection Open Failed");
				}

			} else { //error
				MSG_FATAL("Error!! net_open_connection_with_profile() failed. [%d]", netOpenResult);
			}

			if (isCmOpened == false) {
				context_invoke( __connection_destroy, NULL);
			}

		} else {
			MSG_FATAL("## Failed network callback registration ##");
		}
	} else {
		MSG_DEBUG("Network is already opened.");
	}

	unlock();
	MSG_END();
	return isCmOpened;
}


void MmsPluginCmAgent::close()
{
	MSG_BEGIN();

	lock();

	if (isCmOpened) {
		int netOpenResult = NET_ERR_NONE;

		context_invoke(__connection_profile_close, &netOpenResult);

		if (netOpenResult == NET_ERR_NONE) {

			MSG_DEBUG("## WAITING UNTIL Network Connection Close. ##");

			int time_ret = 0;

			time_ret = cv.timedwait(mx.pMutex(), MMS_CONNECTION_API_WAIT_TIME);

			MSG_DEBUG("## WAKE ##");

			if (time_ret == ETIMEDOUT) {
				MSG_DEBUG("Network Connection Close Timed Out.");
			}

		} else {
			MSG_DEBUG("Error!! net_close_connection() failed");
		}

		isCmOpened = false;
	} else {
		MSG_DEBUG ("Network Connection is not opened.");
	}

	if (isCmRegistered == true) {
		context_invoke(__connection_destroy, NULL);
		isCmRegistered = false;
	}

	unlock();

	MSG_END();
}

void MmsPluginCmAgent::open_callback(connection_error_e result, void* user_data)
{
	lock();

	connection_profile_h profile;
	connection_cellular_state_e state;
	int err;

	if (result == CONNECTION_ERROR_NONE || result == CONNECTION_ERROR_ALREADY_EXISTS) {

		err = connection_get_cellular_state(connection, &state);

		MSG_DEBUG("connection_get_cellular_state ret [%d], state [%d]", err, state);

		err = connection_get_default_cellular_service_profile(connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &profile);
		if (err != CONNECTION_ERROR_NONE) {
			MSG_DEBUG("connection_get_default_cellular_service_profile Failed!! [%d]", err);
		}

		if (profile) {
			isCmOpened = true;

			MMS_FREE(this->home_url);
			MMS_FREE(this->interface_name);
			MMS_FREE(this->proxy_address);

			err = connection_profile_get_cellular_home_url(profile, &this->home_url);
			err = connection_profile_get_network_interface_name(profile, &this->interface_name);
			err = connection_profile_get_proxy_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &this->proxy_address);

			__connection_profile_print(profile);

			connection_profile_destroy(profile);
		}

	} else {
		MSG_DEBUG("connection open profile Failed!! [%d]", result);
	}

	MSG_DEBUG("## SIGNAL ##");
	signal();

	unlock();
}

void MmsPluginCmAgent::close_callback(connection_error_e result, void* user_data)
{
	lock();

	MMS_FREE(this->home_url);
	MMS_FREE(this->interface_name);
	MMS_FREE(this->proxy_address);

	isCmOpened = false;
	MSG_DEBUG("## SIGNAL ##");
	signal();

	unlock();
}

bool MmsPluginCmAgent::getInterfaceName(const char **deviceName)
{
	if (!isCmOpened)
		return false;

	if (deviceName == NULL)
		return false;

	*deviceName = interface_name;

	return true;
}

bool MmsPluginCmAgent::getHomeUrl(const char **homeURL)
{
	if (!isCmOpened)
		return false;

	if (homeURL == NULL)
		return false;

	*homeURL = home_url;

	return true;
}


bool MmsPluginCmAgent::getProxyAddr(const char **proxyAddr)
{
	if (!isCmOpened)
		return false;

	if (proxyAddr == NULL)
		return false;

	*proxyAddr = proxy_address;

	return true;
}
