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

#include "MsgCallStatusManager.h"
#include "MsgDebug.h"
#include "MsgInternalTypes.h"
#include "MsgGconfWrapper.h"

GDBusConnection *call_status_gdbus_conn = NULL;
GDBusProxy *call_status_gdbus_proxy = NULL;
gint call_status_subs_id = 0;
gint call_status = 0;

static void call_status_change_received(GDBusConnection *connection, const gchar *sender_name,
		const gchar *object_path, const gchar *interface_name, const gchar *signal_name,
		GVariant *parameters, gpointer user_data)
{
	MSG_DEBUG("signal_name = [%s]", signal_name);

	if (g_strcmp0(signal_name, CALL_MGR_MEMBER_NAME) == 0) {
		g_variant_get(parameters, "(iis)", &call_status, NULL, NULL);
		MSG_INFO("callStatus = [%d]", call_status);
		if (call_status == 0)
			MsgSettingSetInt(MSG_MESSAGE_DURING_CALL, 0); //call not active
		else
			MsgSettingSetInt(MSG_MESSAGE_DURING_CALL, 1); //call active ( call_status: 2 [Ringing], call_status: 1 [In call])
	}
}

void MsgInitCallStatusManager()
{
	MSG_BEGIN();

	GError *error = NULL;

	if (call_status_gdbus_conn) {
		g_object_unref(call_status_gdbus_conn);
		call_status_gdbus_conn = NULL;
	}

	call_status_gdbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (error) {
		MSG_FATAL("g_bus_get_sync() failed : %s", error->message);
		g_error_free(error);
		error = NULL;
		goto _DBUS_ERROR;
	}

	if (call_status_gdbus_proxy) {
		g_object_unref(call_status_gdbus_proxy);
		call_status_gdbus_proxy = NULL;
	}

	call_status_gdbus_proxy = g_dbus_proxy_new_sync(call_status_gdbus_conn, G_DBUS_PROXY_FLAGS_NONE,
							NULL, CALL_MGR_BUS_NAME, CALL_MGR_PATH_NAME, CALL_MGR_INTERFACE_NAME, NULL, &error);
	if (error) {
		MSG_FATAL("g_dbus_proxy_new_sync() failed : %s", error->message);
		g_error_free(error);
		error = NULL;
		goto _DBUS_ERROR;
	}

	call_status_subs_id = g_dbus_connection_signal_subscribe(call_status_gdbus_conn, NULL,
							CALL_MGR_INTERFACE_NAME, CALL_MGR_MEMBER_NAME, CALL_MGR_PATH_NAME,
							NULL, G_DBUS_SIGNAL_FLAGS_NONE,
							call_status_change_received, NULL, NULL);
	MSG_END();
	return;

_DBUS_ERROR:
	if (call_status_gdbus_conn) {
		g_object_unref(call_status_gdbus_conn);
		call_status_gdbus_conn = NULL;
	}

	if (call_status_gdbus_proxy) {
		g_object_unref(call_status_gdbus_proxy);
		call_status_gdbus_proxy = NULL;
	}

	MSG_END();
	return;
}

void MsgDeInitCallStatusManager()
{
	if (call_status_subs_id) {
		g_dbus_connection_signal_unsubscribe(call_status_gdbus_conn, call_status_subs_id);
		call_status_subs_id = 0;
	}

	if (call_status_gdbus_conn) {
		g_object_unref(call_status_gdbus_conn);
		call_status_gdbus_conn = NULL;
	}

	if (call_status_gdbus_proxy) {
		g_object_unref(call_status_gdbus_proxy);
		call_status_gdbus_proxy = NULL;
	}
}

int MsgGetCallStatus()
{
	/* call_status is defined in <call-manager.h> in callmgr_client package.
	 * typedef enum {
	 *  CM_CALL_STATUS_IDLE,
	 *  CM_CALL_STATUS_RINGING,
	 *  CM_CALL_STATUS_OFFHOOK,
	 *  CM_CALL_STATUS_MAX
	 * } cm_call_status_e;
	 */
	return (int)call_status;
}
