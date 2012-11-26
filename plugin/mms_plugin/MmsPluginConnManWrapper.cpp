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

#include "MmsPluginConnManWrapper.h"

void network_print_profile(net_profile_info_t *ProfInfo)
{
	int di = 0;

	unsigned char *ipaddr;
	unsigned char *netmaskaddr;
	unsigned char *gwaddr;
	net_dev_info_t *net_info;

	if (ProfInfo->profile_type == NET_DEVICE_CELLULAR) {
		ipaddr = (unsigned char *)&ProfInfo->ProfileInfo.Pdp.net_info.IpAddr.Data.Ipv4.s_addr;
		netmaskaddr = (unsigned char *)&ProfInfo->ProfileInfo.Pdp.net_info.SubnetMask.Data.Ipv4.s_addr;
		gwaddr = (unsigned char *)&ProfInfo->ProfileInfo.Pdp.net_info.GatewayAddr.Data.Ipv4.s_addr;
		net_info = &(ProfInfo->ProfileInfo.Pdp.net_info);
	} else {
		MSG_DEBUG("Error!!! Invalid profile type\n");
		return;
	}

	MSG_DEBUG("**************************************************************************************************");

	MSG_DEBUG("Profile Name = [%s]\n", ProfInfo->ProfileName);

	if (ProfInfo->ProfileState == NET_STATE_TYPE_IDLE)
		MSG_DEBUG("Profile State = [idle]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_FAILURE)
		MSG_DEBUG("Profile State = [failure]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_ASSOCIATION)
		MSG_DEBUG("Profile State = [association]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_CONFIGURATION)
		MSG_DEBUG("Profile State = [configuration]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_READY)
		MSG_DEBUG("Profile State = [ready]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_ONLINE)
		MSG_DEBUG("Profile State = [online]\n");
	else if (ProfInfo->ProfileState == NET_STATE_TYPE_DISCONNECT)
		MSG_DEBUG("Profile State = [disconnect]\n");
	else
		MSG_DEBUG("Profile State = [unknown]\n");

		MSG_DEBUG("Profile Type = [pdp]\n");

	if (ProfInfo->ProfileInfo.Pdp.ProtocolType == NET_PDP_TYPE_GPRS)
		MSG_DEBUG("Profile Protocol Type = [GPRS]\n");
	else if (ProfInfo->ProfileInfo.Pdp.ProtocolType == NET_PDP_TYPE_EDGE)
		MSG_DEBUG("Profile Protocol Type = [EDGE]\n");
	else if (ProfInfo->ProfileInfo.Pdp.ProtocolType == NET_PDP_TYPE_UMTS)
		MSG_DEBUG("Profile Protocol Type = [UMTS]\n");
	else
		MSG_DEBUG("Profile Protocol Type = [NONE]\n");

	MSG_DEBUG("Profile APN = [%s]\n", ProfInfo->ProfileInfo.Pdp.Apn);

	if (ProfInfo->ProfileInfo.Pdp.AuthInfo.AuthType == NET_PDP_AUTH_PAP)
		MSG_DEBUG("Profile Auth Type = [PAP]\n");
	else if (ProfInfo->ProfileInfo.Pdp.AuthInfo.AuthType == NET_PDP_AUTH_CHAP)
		MSG_DEBUG("Profile Auth Type = [CHAP]\n");
	else
		MSG_DEBUG("Profile Auth Type = [NONE]\n");

	MSG_DEBUG("Profile Auth UserName = [%s]\n", ProfInfo->ProfileInfo.Pdp.AuthInfo.UserName);
	MSG_DEBUG("Profile Auth Password = [%s]\n", ProfInfo->ProfileInfo.Pdp.AuthInfo.Password);
	MSG_DEBUG("Profile Home URL = [%s]\n", ProfInfo->ProfileInfo.Pdp.HomeURL);
	MSG_DEBUG("Profile MCC = [%s]\n", ProfInfo->ProfileInfo.Pdp.Mcc);
	MSG_DEBUG("Profile MNC = [%s]\n", ProfInfo->ProfileInfo.Pdp.Mnc);
	MSG_DEBUG("Profile Roaming = [%d]\n", (int)ProfInfo->ProfileInfo.Pdp.Roaming);
	MSG_DEBUG("Profile Setup Required = [%d]\n", (int)ProfInfo->ProfileInfo.Pdp.SetupRequired);
	MSG_DEBUG("Profile Favourite = [%d]\n", (int)ProfInfo->Favourite);
	MSG_DEBUG("Profile Device Name = [%s]\n", net_info->DevName);
	MSG_DEBUG("Profile DNS Count = [%d]\n", net_info->DnsCount);
	for (di = 0; di < net_info->DnsCount; di++) {
		unsigned char *dns = (unsigned char *)&net_info->DnsAddr[di].Data.Ipv4.s_addr;
		MSG_DEBUG("Profile DNS Address %d = [%d.%d.%d.%d]\n", di+1, dns[0], dns[1], dns[2], dns[3]);
	}

	if (net_info->IpConfigType == NET_IP_CONFIG_TYPE_DYNAMIC)
		MSG_DEBUG("Profile IPv4 Method = [NET_IP_CONFIG_TYPE_DYNAMIC]\n");
	else if (net_info->IpConfigType == NET_IP_CONFIG_TYPE_STATIC)
		MSG_DEBUG("Profile IPv4 Method = [NET_IP_CONFIG_TYPE_STATIC]\n");
	else if (net_info->IpConfigType == NET_IP_CONFIG_TYPE_FIXED)
		MSG_DEBUG("Profile IPv4 Method = [NET_IP_CONFIG_TYPE_FIXED]\n");
	else if (net_info->IpConfigType == NET_IP_CONFIG_TYPE_OFF)
		MSG_DEBUG("Profile IPv4 Method = [NET_IP_CONFIG_TYPE_OFF]\n");
	else
		MSG_DEBUG("Profile IPv4 Method = [UNKNOWN]\n");

	MSG_DEBUG("Profile IP Address = [%d.%d.%d.%d]\n", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
	MSG_DEBUG("Profile Netmask = [%d.%d.%d.%d]\n", netmaskaddr[0], netmaskaddr[1], netmaskaddr[2], netmaskaddr[3]);
	MSG_DEBUG("Profile Gateway = [%d.%d.%d.%d]\n", gwaddr[0], gwaddr[1], gwaddr[2], gwaddr[3]);

	if (net_info->ProxyMethod == NET_PROXY_TYPE_DIRECT)
		MSG_DEBUG("Proxy Method = [direct]\n");
	else if (net_info->ProxyMethod == NET_PROXY_TYPE_AUTO)
		MSG_DEBUG("Proxy Method = [auto]\n");
	else if (net_info->ProxyMethod == NET_PROXY_TYPE_MANUAL)
		MSG_DEBUG("Proxy Method = [manual]\n");
	else
		MSG_DEBUG("Proxy Method = [unknown]\n");

	MSG_DEBUG("Profile Proxy = [%s]\n", net_info->ProxyAddr);

	MSG_DEBUG("**************************************************************************************************");
}



void network_evt_cb (net_event_info_t *event_cb, void *user_data)
{
	MSG_BEGIN();

	MmsPluginCmAgent::instance()->processCBdatas(event_cb, user_data);

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
	bzero(&mmsProfile, sizeof (net_profile_info_t));

	MSG_END();
}

MmsPluginCmAgent::~MmsPluginCmAgent()
{

}

bool MmsPluginCmAgent::registration()
{
	MSG_BEGIN();

	bool registResult = true;

	if (net_register_client((net_event_cb_t) network_evt_cb, NULL) != NET_ERR_NONE) {
		MSG_DEBUG("Error!! net_register_client() failed.\n");
		registResult = false;
	}

	MSG_END();

	return registResult;
}

bool MmsPluginCmAgent::open()
{
	MSG_BEGIN();

	if (!isCmOpened) {
		if (!registration())
			return false;

		int netOpenResult = NET_ERR_NONE;
		net_service_type_t service_type = NET_SERVICE_MMS;

		netOpenResult = net_open_connection_with_preference(service_type);
		if (netOpenResult != NET_ERR_NONE) {
			MSG_DEBUG("Error!! net_open_connection_with_profile() failed. [%d]", netOpenResult);
			deregistration();
			return false;
		}

		MSG_DEBUG("WAITING UNTIL Network Connection Open.");

		lock();

		int time_ret = 0;
		time_ret = cv.timedwait(mx.pMutex(), 50);

		unlock();

		if (time_ret == ETIMEDOUT) {
			MSG_DEBUG("Network Connection Open Time Out.");
			deregistration();
			return false;
		} else {
			if(!isCmOpened) {
				MSG_DEBUG("Network connection open failed");
				deregistration();
				return false;
			}
		}
	} else {
		MSG_DEBUG("Network is already opened.");
		return false;
	}

	MSG_END();

	return isCmOpened;
}


void MmsPluginCmAgent::close()
{
	MSG_BEGIN();

	if (isCmOpened) {
		int netOpenResult = NET_ERR_NONE;

		netOpenResult = net_close_connection(mmsProfile.ProfileName);
		if (netOpenResult != NET_ERR_NONE) {
			MSG_DEBUG("Error!! net_close_connection() failed.\n");
			deregistration();
			return;
		}

		MSG_DEBUG("WAITING UNTIL Network Connection Close.");

		lock();

		int time_ret = 0;
		time_ret = cv.timedwait(mx.pMutex(), 50);

		unlock();

		if (time_ret == ETIMEDOUT) {
			MSG_DEBUG("Network Connection Close Timed Out.");
		}

		deregistration();
		isCmOpened = false;
	} else {
		MSG_DEBUG ("Network Connection is not opened.");
		return;
	}
}


void MmsPluginCmAgent::deregistration()
{
	MSG_BEGIN();

	int netOpenResult = NET_ERR_NONE;

	netOpenResult = net_deregister_client();
	if (netOpenResult != NET_ERR_NONE)
		MSG_DEBUG("Error!! net_deregister_client() failed. [%d]", netOpenResult);
	else
		MSG_DEBUG ("net_deregister_client() Success.");

	MSG_END();
}


void MmsPluginCmAgent::processCBdatas(net_event_info_t *event_cb, void *user_data)
{
	MSG_BEGIN();

	net_dev_info_t *dev_info = NULL;
	net_profile_info_t *prof_info = NULL;

	switch (event_cb->Event) {
	case NET_EVENT_NET_CONFIGURE_RSP:
		MSG_DEBUG("Received Network Configuration response: %d \n", event_cb->Error);
		dev_info = (net_dev_info_t *)event_cb->Data;

		/*Successful PDP Activation*/
		if (event_cb->Error == NET_ERR_NONE) {
			char *ip = (char *)&dev_info->IpAddr.Data.Ipv4;
			char *netmask = (char *)&dev_info->SubnetMask.Data.Ipv4;
			char *gateway = (char *)&dev_info->GatewayAddr.Data.Ipv4;
			char *dns1 = (char *)&dev_info->DnsAddr[0].Data.Ipv4.s_addr;
			char *dns2 = (char *)&dev_info->DnsAddr[1].Data.Ipv4.s_addr;

			MSG_DEBUG("= IP address [%d.%d.%d.%d]\n",
					(int)ip[0], ip[1], ip[2], ip[3]);
			MSG_DEBUG("= Netmask [%d.%d.%d.%d]\n",
					(int)netmask[0], netmask[1], netmask[2], netmask[3]);
			MSG_DEBUG("= Gateway [%d.%d.%d.%d]\n",
					(int)gateway[0], gateway[1], gateway[2], gateway[3]);
			MSG_DEBUG("= DNS address [%d.%d.%d.%d]\n",
					(int)dns1[0], dns1[1], dns1[2], dns1[3]);
			MSG_DEBUG("= DNS2 address [%d.%d.%d.%d]\n",
					(int)dns2[0], dns2[1], dns2[2], dns2[3]);
			MSG_DEBUG("= Device name [%s]\n", dev_info->DevName);
			MSG_DEBUG("= Profile name [%s]\n", dev_info->ProfileName);
		} else {
			MSG_DEBUG("Error!!! Network Configuration Failed %d \n", event_cb->Error);
		}
		break;

		/*Response from Datacom for PDP Activation Request*/
	case NET_EVENT_OPEN_IND:
		MSG_DEBUG("Got Open Indication\n");
		MSG_DEBUG("Received ACTIVATION response: %d \n", event_cb->Error);
		break;
	case NET_EVENT_OPEN_RSP:
		MSG_DEBUG("Got Open RSP\n");
		MSG_DEBUG("Received ACTIVATION response: %d \n", event_cb->Error);
		prof_info = (net_profile_info_t *)event_cb->Data;

		/*Successful PDP Activation*/
		if (event_cb->Error == NET_ERR_NONE || event_cb->Error == NET_ERR_ACTIVE_CONNECTION_EXISTS) {
			network_print_profile(prof_info);

			lock();

			memcpy(&mmsProfile, prof_info, sizeof(net_profile_info_t));
			isCmOpened = true;
			signal();

			unlock();
		} else {
			MSG_DEBUG("Activation Failed %d \n", event_cb->Error); // open failed
			lock();
			isCmOpened = false;
			signal();
			unlock();
		}
		break;

	case NET_EVENT_CLOSE_RSP:
		MSG_DEBUG("Got Close RSP\n");

		lock();

		bzero(&mmsProfile, sizeof(net_profile_info_t));
		isCmOpened = false;
		signal();

		unlock();
		break;

	case NET_EVENT_CLOSE_IND:
		MSG_DEBUG("Got Close IND\n");
		break;
	case NET_EVENT_PROFILE_MODIFY_IND:
		MSG_DEBUG("Received Profile modified Indication\n");
		MSG_DEBUG("No. of profile [%d]\n", event_cb->Datalength);
		break;
	case NET_EVENT_NET_STATE_IND:
		MSG_DEBUG("Received NET_EVENT_NET_STATE_IND\n");
		break;
	default :
		MSG_DEBUG("Error! Unknown Event = %d\n\n", event_cb->Event);
		break;
	}

	MSG_END();
}


bool MmsPluginCmAgent::getDeviceName(char *deviceName)
{
	if (!isCmOpened)
		return false;

	int deviceNameLength = strlen(mmsProfile.ProfileInfo.Pdp.net_info.DevName);

	if (deviceNameLength > 0) {
		strncpy(deviceName, mmsProfile.ProfileInfo.Pdp.net_info.DevName, deviceNameLength);
		deviceName[deviceNameLength] = '\0';
		return true;
	}

	return false;
}


bool MmsPluginCmAgent::getHomeURL(char *homeURL)
{
	if (!isCmOpened)
		return false;

	int homeURLLength = strlen(mmsProfile.ProfileInfo.Pdp.HomeURL);

	if (homeURLLength > 0) {
		strncpy(homeURL, mmsProfile.ProfileInfo.Pdp.HomeURL, homeURLLength);
		homeURL[homeURLLength] = '\0';
		return true;
	}

	return false;
}


bool MmsPluginCmAgent::getProxyAddr(char *proxyAddr)
{
	if (!isCmOpened)
		return false;

	int proxyAddrLength = strlen(mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr);

	if (proxyAddrLength > 0) {
		if (strchr(mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr, ':') == NULL)
			return false;

		proxyAddrLength = proxyAddrLength - strlen(strchr(mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr, ':'));
		strncpy(proxyAddr, mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr, proxyAddrLength);
		proxyAddr[proxyAddrLength] = '\0';

		return true;
	}

	return false;
}


int MmsPluginCmAgent::getProxyPort()
{
	if (!isCmOpened)
		return -1;

	int proxyAddrLength = strlen(mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr);

	if (proxyAddrLength > 0) {
		char *pPort = strchr(mmsProfile.ProfileInfo.Pdp.net_info.ProxyAddr, ':') + 1;
		if (pPort)
			return atoi(pPort);
		else
			return -1;
	}

	return -1;
}

