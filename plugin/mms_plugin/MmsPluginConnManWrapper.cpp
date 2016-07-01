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

#include <errno.h>
#include <glib.h>
#include "MmsPluginConnManWrapper.h"
#include "MmsPluginDebug.h"
#include "MmsPluginHttp.h"
#include "net_connection.h"
#include "MmsPluginUtil.h"

#define MMS_CONTEXT_INVOKE_WAIT_TIME	30
#define MMS_CONNECTION_API_WAIT_TIME	420

static MsgMutex g_mx;
static MsgCndVar g_cv;
static connection_h g_connection = NULL;
static connection_profile_h g_profile = NULL;

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
	MSG_DEBUG("return value of connection_profile_get_id [%d]", ret);
	MSG_SEC_INFO("Profile Id = [%s]", profile_id);

	ret = connection_profile_get_name(profile, &profile_name);
	MSG_SEC_INFO("Profile Name = [%s]", profile_name);

	ret = connection_profile_get_type(profile, &profile_type);

	if (profile_type == CONNECTION_PROFILE_TYPE_CELLULAR) {
		MSG_SEC_INFO("Profile Type = [CELLULAR]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_WIFI) {
		MSG_SEC_INFO("Profile Type = [WIFI]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_ETHERNET) {
		MSG_SEC_INFO("Profile Type = [ETHERNET]");
	} else if (profile_type == CONNECTION_PROFILE_TYPE_BT) {
		MSG_SEC_INFO("Profile Type = [BT]");
	} else {
		MSG_SEC_INFO("Profile Type = Unknown [%d]", profile_type);
	}

	ret = connection_profile_get_network_interface_name(profile, &interface_name);
	MSG_SEC_INFO("Profile Interface Name = [%s]", interface_name);

	ret = connection_profile_get_state(profile, &profile_state);
	if (profile_state == CONNECTION_PROFILE_STATE_DISCONNECTED) {
		MSG_SEC_INFO("Profile State = [DISCONNECTED]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_ASSOCIATION) {
		MSG_SEC_INFO("Profile State = [ASSOCIATION]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_CONFIGURATION) {
		MSG_SEC_INFO("Profile State = [CONFIGURATION]");
	} else if (profile_state == CONNECTION_PROFILE_STATE_CONNECTED) {
		MSG_SEC_INFO("Profile State = [CONNECTED]");
	} else {
		MSG_SEC_INFO("Profile State = Unknown [%d]", profile_state);
	}

	ret = connection_profile_get_ip_config_type(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &ip_type);
	MSG_SEC_INFO("Profile Ip Config Type = [%d]", ip_type);

	ret = connection_profile_get_ip_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &ip_address);
	MSG_SEC_INFO("Profile Ip Address = [%s]", ip_address);

	ret = connection_profile_get_subnet_mask(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &subnet_mask);
	MSG_SEC_INFO("Profile Subnet Mask = [%s]", subnet_mask);

	ret = connection_profile_get_gateway_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &gateway_address);
	MSG_SEC_INFO("Profile Gateway Address = [%s]", gateway_address);

	ret = connection_profile_get_dns_address(profile, 1, CONNECTION_ADDRESS_FAMILY_IPV4, &dns_address);
	MSG_SEC_INFO("Profile Dns Address = [%s]", dns_address);

	ret = connection_profile_get_proxy_type(profile, &proxy_type);
	MSG_SEC_INFO("Profile Proxy Type = [%d]", proxy_type);

	ret = connection_profile_get_proxy_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);
	MSG_SEC_INFO("Profile Proxy Address = [%s]", proxy_address);

	ret = connection_profile_get_cellular_service_type(profile, &service_type);
	if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET) {
		MSG_SEC_INFO("Profile Service Type = [INTERNET]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_MMS) {
		MSG_SEC_INFO("Profile Service Type = [MMS]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_PREPAID_INTERNET) {
		MSG_SEC_INFO("Profile Service Type = [PREPAID_INTERNET]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_PREPAID_MMS) {
		MSG_SEC_INFO("Profile Service Type = [PREPAID_MMS]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_TETHERING) {
		MSG_SEC_INFO("Profile Service Type = [TETHERING]");
	} else if (service_type == CONNECTION_CELLULAR_SERVICE_TYPE_APPLICATION) {
		MSG_SEC_INFO("Profile Service Type = [APPLICATION]");
	} else {
		MSG_SEC_INFO("Profile Service Type = [Unknown][%d]", service_type);
	}

	ret = connection_profile_get_cellular_apn(profile, &apn);
	MSG_SEC_INFO("Profile Apn = [%s]", apn);

	ret = connection_profile_get_cellular_auth_info(profile, &auth_type, &user_name, &password);
	MSG_SEC_INFO("Profile Auth Type = [%d]", &auth_type);
	MSG_SEC_INFO("Profile Auth Name = [%s]", &user_name);
	MSG_SEC_INFO("Profile Auth Passward = [%s]", &password);

	ret = connection_profile_get_cellular_home_url(profile, &home_url);
	MSG_SEC_INFO("Profile Home Url = [%s]", home_url);

	ret = connection_profile_is_cellular_roaming(profile, &is_roaming);
	MSG_SEC_INFO("Profile Roaming = [%d]", is_roaming);
	MSG_DEBUG("**************************************************************************************************");

	MSG_FREE(profile_id);
	MSG_FREE(profile_name);
	MSG_FREE(interface_name);
	MSG_FREE(ip_address);
	MSG_FREE(subnet_mask);
	MSG_FREE(gateway_address);
	MSG_FREE(dns_address);
	MSG_FREE(proxy_address);
	MSG_FREE(apn);
	MSG_FREE(user_name);
	MSG_FREE(password);
	MSG_FREE(home_url);
}

static void __connection_type_changed_cb(connection_type_e type, void* user_data)
{
	MSG_INFO("Type changed callback, connection type : %d", type);
}

static void __connection_ip_changed_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	MSG_INFO("IP changed callback, IPv4 address : %s, IPv6 address : %s",
			ipv4_address, (ipv6_address ? ipv6_address : "NULL"));
}

static void __connection_proxy_changed_cb(const char* ipv4_address, const char* ipv6_address, void* user_data)
{
	MSG_INFO("Proxy changed callback, IPv4 address : %s, IPv6 address : %s",
			ipv4_address, (ipv6_address ? ipv6_address : "NULL"));
}

static void __connection_profile_opened_cb(connection_error_e result, void* user_data)
{
	if (result == CONNECTION_ERROR_NONE || result == CONNECTION_ERROR_ALREADY_EXISTS)
		MSG_INFO("Connection open Succeeded [%d]", result);
	else
		MSG_ERR("Connection open Failed, err : %d", result);

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	cmAgent->connection_profile_open_callback(result, user_data);
}

static void __connection_profile_closed_cb(connection_error_e result, void* user_data)
{
	if (result ==  CONNECTION_ERROR_NONE)
		MSG_INFO("Connection close Succeeded");
	else
		MSG_ERR("Connection close Failed, err : %d", result);

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	cmAgent->connection_profile_close_callback(result, user_data);
}


void __connection_profile_state_changed_cb(connection_profile_state_e state, void* user_data)
{
	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	cmAgent->connection_profile_state_changed_cb(state, user_data);
}

static gboolean __connection_create(void *pVoid)
{
	MSG_BEGIN();

	bool ret = false;
	bool *ret_val = (bool *)pVoid;

	if (g_connection) {
		MSG_INFO("connection already exist");
		ret = true;
	} else {
		int err = connection_create(&g_connection);

		if (CONNECTION_ERROR_NONE == err && g_connection) {
			connection_cellular_state_e cellular_state;
			connection_type_e net_state;

			err = connection_get_cellular_state(g_connection, &cellular_state);

			err = connection_get_type(g_connection, &net_state);

			if (cellular_state == CONNECTION_CELLULAR_STATE_AVAILABLE
				|| cellular_state == CONNECTION_CELLULAR_STATE_CONNECTED) {
				MSG_INFO("Client registration success [%p], cellular_state [%d], net_state [%d]", g_connection, cellular_state, net_state);

				err = connection_set_type_changed_cb(g_connection, __connection_type_changed_cb, NULL);

				err = connection_set_ip_address_changed_cb(g_connection, __connection_ip_changed_cb, NULL);

				err = connection_set_proxy_address_changed_cb(g_connection, __connection_proxy_changed_cb, NULL);

				ret = true;
			} else {
				MSG_INFO("Client registration Failed,  cellular state [%d], net_state [%d]", cellular_state, net_state);
				connection_destroy(g_connection);
				g_connection = NULL;
			}
		} else {
			MSG_WARN("Client registration failed %d", err);
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

	if (g_connection != NULL) {
		rv = connection_destroy(g_connection);
		g_connection = NULL;
		MSG_INFO("connection destory !!");
	} else {
		MSG_ERR("Cannot connection destroy : Handle is NULL");
		rv = CONNECTION_ERROR_INVALID_OPERATION;
	}

	MSG_DEBUG("return value of connection destroy [%d]", rv);

	MSG_END();
	return FALSE;
}

static gboolean __connection_profile_open(void *pVoid)
{
	MSG_BEGIN();

	int netOpenResult = MSG_CM_ERR_NONE;
	int *ret_val = (int *)pVoid;
	int err;

	if (g_profile) {
		MSG_WARN("connection profile Already exist!!, It will destroy");
		connection_profile_unset_state_changed_cb(g_profile);
		connection_profile_destroy(g_profile);
		g_profile = NULL;
	}

	err = connection_get_default_cellular_service_profile(g_connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &g_profile);

	if (err != CONNECTION_ERROR_NONE) {
		MSG_ERR("connection_get_default_cellular_service_profile Failed!! [%d]", err);
		netOpenResult = MSG_CM_ERR_UNKNOWN;
	} else {
		err = connection_profile_set_state_changed_cb(g_profile, __connection_profile_state_changed_cb, g_profile);

		if (connection_open_profile(g_connection, g_profile, __connection_profile_opened_cb, NULL) != CONNECTION_ERROR_NONE) {
			MSG_ERR("Connection open Failed!!");
			netOpenResult = MSG_CM_ERR_UNKNOWN;
		}
	}

	if (ret_val) {
		*ret_val = netOpenResult;
		MSG_DEBUG("[%d]", netOpenResult);
	}

	MSG_END();

	return FALSE;
}

static gboolean __connection_profile_close(void *pVoid)
{
	MSG_BEGIN();

	int netOpenResult = MSG_CM_ERR_NONE;

	int *ret_val = (int *)pVoid;

	if (g_profile) {
		connection_profile_unset_state_changed_cb(g_profile);

		if (connection_close_profile(g_connection, g_profile, __connection_profile_closed_cb, NULL) != CONNECTION_ERROR_NONE) {
			MSG_ERR("Connection close Failed!!");
			netOpenResult = MSG_CM_ERR_UNKNOWN;
		}

		connection_profile_destroy(g_profile);
		g_profile = NULL;
	}

	if (ret_val) {
		*ret_val = netOpenResult;
	}

	MSG_END();

	return FALSE;
}


void context_invoke_end_cb(gpointer data)
{
	g_mx.lock();

	MSG_INFO("@@ SIGNAL @@");

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

	g_main_context_invoke_full(NULL, G_PRIORITY_DEFAULT, func, ret,  context_invoke_end_cb);

	MSG_INFO("@@ WAIT @@");

	time_ret = g_cv.timedwait(g_mx.pMsgMutex(), MMS_CONTEXT_INVOKE_WAIT_TIME);

	g_mx.unlock();

	if (time_ret == ETIMEDOUT) {
		MSG_INFO("@@ WAKE by timeout@@");
	} else {
		MSG_INFO("@@ WAKE by signal@@");
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
	waitProfileOpen = false;

	home_url = NULL;
	interface_name = NULL;
	proxy_address = NULL;
	dns_address_list = NULL;
	MSG_END();
}

MmsPluginCmAgent::~MmsPluginCmAgent()
{
	MSG_FREE(home_url);
	MSG_FREE(interface_name);
	MSG_FREE(proxy_address);
	MSG_FREE(dns_address_list);
}

bool MmsPluginCmAgent::open()
{
	MSG_BEGIN();

	int netOpenResult = MSG_CM_ERR_NONE;
	int bConnection = false;
	int time_ret = 0;
	lock();

	/* create connection */
	context_invoke(__connection_create, &bConnection);

	if (bConnection == false || g_connection == NULL) {
		MSG_ERR("Failed __connection_create");
		goto __ERR_RETURN;
	}

	if (g_profile) {
		MSG_WARN("connection profile already exist");
		/* TODO:: get data; */
		/* goto __RETURN; */
	}

	waitProfileOpen = true;

	context_invoke(__connection_profile_open, &netOpenResult);

	if (netOpenResult != MSG_CM_ERR_NONE) {
		MSG_ERR("Failed __connection_profile_open. [%d]", netOpenResult);
		goto __ERR_RETURN;
	}

	MSG_INFO("## WAITING UNTIL __connection_profile_state CONNECT. ##");

	time_ret = cv.timedwait(mx.pMsgMutex(), MMS_CONNECTION_API_WAIT_TIME); /* isCmOpened will changed by processCBdatas */

	if (time_ret == ETIMEDOUT) {
		MSG_WARN("## WAKE by timeout ##");
	} else {
		MSG_INFO("## WAKE by SIGNAL ##");
	}

	if (isCmOpened == false) {
		MSG_WARN("");
		goto __ERR_RETURN;
	}

/* __RETURN: */
	unlock();
	MSG_END();
	return isCmOpened;

__ERR_RETURN:
	context_invoke(__connection_profile_close, NULL);
	context_invoke(__connection_destroy, NULL);
	unlock();
	MSG_END();
	return isCmOpened;
}


void MmsPluginCmAgent::close()
{
	MSG_BEGIN();

	lock();

	int netOpenResult = MSG_CM_ERR_NONE;
	int time_ret = 0;

	isCmOpened = false;

	if (g_profile == NULL) {
		MSG_INFO("connection profile is NULL");
		goto __RETURN;
	}

	context_invoke(__connection_profile_close, &netOpenResult);

	if (netOpenResult != MSG_CM_ERR_NONE) {
		MSG_ERR("Failed __connection_profile_close. [%d]", netOpenResult);
		goto __RETURN;
	}

	MSG_INFO("## WAITING UNTIL connection_profile_close_callback ##");

	time_ret = cv.timedwait(mx.pMsgMutex(), MMS_CONNECTION_API_WAIT_TIME);

	if (time_ret == ETIMEDOUT) {
		MSG_WARN("## WAKE by timeout ##");
	} else {
		MSG_INFO("## WAKE by SIGNAL ##");
	}


__RETURN:
	if (g_connection)
		context_invoke(__connection_destroy, NULL);

	g_profile = NULL;
	g_connection = NULL;

	MSG_FREE(this->home_url);
	MSG_FREE(this->interface_name);
	MSG_FREE(this->proxy_address);
	MSG_FREE(this->dns_address_list);

	unlock();

	MSG_END();
}

/* profile open callback */
void MmsPluginCmAgent::connection_profile_open_callback(connection_error_e result, void* user_data)
{
	lock();

	connection_cellular_state_e state;
	connection_profile_h profile = NULL;
	connection_profile_state_e  profile_state;
	int err = CONNECTION_ERROR_NONE;

	if (result == CONNECTION_ERROR_NONE || result == CONNECTION_ERROR_ALREADY_EXISTS) {
		err = connection_get_cellular_state(g_connection, &state);

		MSG_INFO("connection_get_cellular_state ret [%d], state [%d]", err, state);

		err = connection_get_default_cellular_service_profile(g_connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &profile);
		if (err != CONNECTION_ERROR_NONE || profile == NULL) {
			MSG_ERR("Failed connection_get_default_cellular_service_profile. err [%d], profile [%p]", err, profile);
			goto __SIGNAL_RETURN;
		}

		err = connection_profile_get_state(profile, &profile_state);

		MSG_DEBUG("profile state [%d]", profile_state);

		if (profile_state == CONNECTION_PROFILE_STATE_CONNECTED && waitProfileOpen == true) {
			__connection_profile_print(profile);

			MSG_FREE(this->home_url);
			MSG_FREE(this->interface_name);
			MSG_FREE(this->proxy_address);
			MSG_FREE(this->dns_address_list);

			err = connection_profile_get_cellular_home_url(profile, &this->home_url);
			if (err != CONNECTION_ERROR_NONE) {
				MSG_ERR("Failed connection_profile_get_cellular_home_url");
			}

			err = connection_profile_get_network_interface_name(profile, &this->interface_name);
			if (err != CONNECTION_ERROR_NONE) {
				MSG_ERR("Failed connection_profile_get_cellular_home_url");
			}

			err = connection_profile_get_proxy_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &this->proxy_address);
			if (err != CONNECTION_ERROR_NONE) {
				MSG_ERR("Failed connection_profile_get_cellular_home_url");
			}

			this->dns_address_list = (char *)calloc(1, 51);
			for (int i = 1; i <= 2; i++) {
				char *dns = NULL;
				err = connection_profile_get_dns_address(profile, i, CONNECTION_ADDRESS_FAMILY_IPV4, &dns);
				if (err != CONNECTION_ERROR_NONE) {
					MSG_ERR("Failed connection_profile_get_cellular_home_url");
				}

				if (dns != NULL) {
					MSG_INFO("dns [%s]", dns);
					if (g_strcmp0(dns, "0.0.0.0")) {
						if (!(strlen(this->dns_address_list) > 1)) {
							snprintf(this->dns_address_list, 50, "%s", dns);
						} else {
							snprintf(this->dns_address_list  + strlen(this->dns_address_list ), 50 - strlen(this->dns_address_list ), ",%s", dns);
						}
					}
					MSG_FREE(dns);
				}
				MSG_INFO("dns list[%s]", this->dns_address_list);
			}

			isCmOpened = true;

			goto __SIGNAL_RETURN; /* open success */

		} else {
			goto __NO_SIGNAL_RETURN; /* Just open success */
		}
	} else {
		MSG_ERR("connection open profile Failed!! [%d]", result);
		isCmOpened = false;
		goto __SIGNAL_RETURN;
	}

__NO_SIGNAL_RETURN: /* Just Open */
	if (profile)
		connection_profile_destroy(profile);
	unlock();
	return;

__SIGNAL_RETURN: /* Error or Already connected */
	if (profile)
		connection_profile_destroy(profile);

	if (waitProfileOpen == true) { /* open fail */
		waitProfileOpen = false;
		MSG_INFO("## SIGNAL ##");
		signal();
	}
	unlock();
	return;
}


void MmsPluginCmAgent::connection_profile_close_callback(connection_error_e result, void* user_data)
{
	lock();
	MSG_INFO("result [%d]", result);
	MSG_INFO("## SIGNAL ##");
	signal();

	unlock();
}

void MmsPluginCmAgent::connection_profile_state_changed_cb(connection_profile_state_e state, void* user_data)
{
	MSG_BEGIN();

	lock();

	int err;
	connection_profile_h profile = NULL;

	MSG_INFO("state [%d]", state);

	if (state != CONNECTION_PROFILE_STATE_CONNECTED) {
		isCmOpened = false;
		goto __NO_SIGNAL_RETURN;
	}

	if (isCmOpened == true) {
		MSG_INFO("already opened");
		goto __SIGNAL_RETURN;
	}

	/* Should get profile to get latest profile info*/
	err = connection_get_default_cellular_service_profile(g_connection, CONNECTION_CELLULAR_SERVICE_TYPE_MMS, &profile);
	if (err != CONNECTION_ERROR_NONE || profile == NULL) {
		MSG_ERR("Failed connection_get_default_cellular_service_profile. err [%d], profile [%p]", err, profile);
		goto __SIGNAL_RETURN;
	}

	if (state == CONNECTION_PROFILE_STATE_CONNECTED) {
		__connection_profile_print(profile);

		MSG_FREE(this->home_url);
		MSG_FREE(this->interface_name);
		MSG_FREE(this->proxy_address);
		MSG_FREE(this->dns_address_list);

		err = connection_profile_get_cellular_home_url(profile, &this->home_url);
		if (err != CONNECTION_ERROR_NONE) {
			MSG_ERR("Failed connection_profile_get_cellular_home_url");
		}

		err = connection_profile_get_network_interface_name(profile, &this->interface_name);
		if (err != CONNECTION_ERROR_NONE) {
			MSG_ERR("Failed connection_profile_get_cellular_home_url");
		}

		err = connection_profile_get_proxy_address(profile, CONNECTION_ADDRESS_FAMILY_IPV4, &this->proxy_address);
		if (err != CONNECTION_ERROR_NONE) {
			MSG_ERR("Failed connection_profile_get_cellular_home_url");
		}

		this->dns_address_list = (char *)calloc(1, 51);
		for (int i = 1; i <= 2; i++) {
			char *dns = NULL;
			err = connection_profile_get_dns_address(profile, i, CONNECTION_ADDRESS_FAMILY_IPV4, &dns);
			if (err != CONNECTION_ERROR_NONE) {
				MSG_ERR("Failed connection_profile_get_cellular_home_url");
			}

			if (dns != NULL) {
				MSG_INFO("dns [%s]", dns);
				if (g_strcmp0(dns, "0.0.0.0")) {
					if (!(strlen(this->dns_address_list) > 1)) {
						snprintf(this->dns_address_list, 50, "%s", dns);
					} else {
						snprintf(this->dns_address_list  + strlen(this->dns_address_list ), 50 - strlen(this->dns_address_list ), ",%s", dns);
					}
				}
				MSG_FREE(dns);
			}
			MSG_INFO("dns list[%s]", this->dns_address_list);
		}

		isCmOpened = true;
		goto __SIGNAL_RETURN;
	}

__NO_SIGNAL_RETURN: /* Default */
	unlock();
	return;

__SIGNAL_RETURN: /* Error or connected */
	if (profile)
		connection_profile_destroy(profile);

	if (waitProfileOpen == true) {
		waitProfileOpen = false;
		MSG_INFO("## SIGNAL ##");
		signal();
	}
	unlock();
	return;
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


bool MmsPluginCmAgent::getDnsAddrList(const char **dnsAddrList)
{
	if (!isCmOpened)
		return false;

	if (dnsAddrList == NULL)
		return false;

	*dnsAddrList = dns_address_list;

	return true;
}
