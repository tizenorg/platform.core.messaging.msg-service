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

#include <glib.h>
#include <sys/socket.h>

#include "MsgDebug.h"
#include "MsgZoneManager.h"

#ifdef FEATURE_CONTAINER_ENABLE
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define MSG_ZONE_KNOX "knox"
#define MSG_ZONE_PERSONAL "personal"

#include <vasum.h>

vsm_context_h vsm_ctx = NULL;
vsm_zone_h real_zone = NULL;
GIOChannel *gio_channel = NULL;

int zone_ref_count = 0;


gboolean _zone_mainloop_cb(GIOChannel *channel, GIOCondition condition, void *data)
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE
	struct vsm_context *ctx = (struct vsm_context *)data;
	vsm_enter_eventloop(ctx, 0, 0);
	MSG_END();
#endif
	return true;
}


int _get_peer_pid(int fd)
{
	struct ucred cred;
	socklen_t cr_len = sizeof(cred);
	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &cr_len) < 0) {
		return -1;
	}
	return cred.pid;
}

#endif

void MsgZoneInit()
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE

	MsgZoneClean();

	vsm_ctx = vsm_create_context();
	if (vsm_ctx == NULL) {
		MSG_FATAL("vsm_create_context failed");
		return;
	}

	int fd = vsm_get_poll_fd(vsm_ctx);
	gio_channel = g_io_channel_unix_new(fd);
	if (gio_channel == NULL) {
		MSG_FATAL("g_io_channel_unix_new failed");
		return;
	}

	g_io_add_watch(gio_channel, G_IO_IN, _zone_mainloop_cb, vsm_ctx);

#endif
	MSG_END();
}


void MsgZoneClean()
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE
	if (vsm_ctx != NULL) {
		vsm_cleanup_context(vsm_ctx);
		vsm_ctx = NULL;
	}
	if (gio_channel != NULL) {
		g_io_channel_unref(gio_channel);
		gio_channel = NULL;
	}
#endif
	MSG_END();
}


char* MsgZoneGetName(int fd)
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE
	if (vsm_ctx == NULL) {
		MsgZoneInit();
	}
	int pid = _get_peer_pid(fd);
	vsm_zone_h zone = vsm_lookup_zone_by_pid(vsm_ctx, pid);
	if (zone == NULL) {
		MSG_FATAL("vsm_lookup_zone_by_pid failed, PID = %d", pid);
		return NULL;
	}
	MSG_END();
	return strdup(vsm_get_zone_name(zone));
#endif
	return NULL;
}


bool MsgZoneIsAllowed(int fd)
{
#ifdef FEATURE_CONTAINER_ENABLE
	char *zone_name = MsgZoneGetName(fd);
	if (zone_name == NULL) {
		return false;
	}

	MSG_DEBUG("zone_name=[%s]", zone_name);
	bool result = (!g_strcmp0(zone_name, MSG_ZONE_PERSONAL) || (!g_strcmp0(zone_name, "")));
	g_free(zone_name);
	zone_name = NULL;

	return result;
#endif
	return true;
}


void MsgZoneChange()
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE
	if (vsm_ctx == NULL) {
		MSG_DEBUG("vsm_ctx == NULL");
		return;
	}
	if (real_zone != NULL && zone_ref_count > 0) {
		zone_ref_count++;
		MSG_DEBUG("zone_ref_count++");
		return;
	}

	vsm_zone_h current_zone = vsm_get_foreground(vsm_ctx);
	vsm_zone_h effective_zone = vsm_lookup_zone_by_name(vsm_ctx, MSG_ZONE_PERSONAL);
	if (!g_strcmp0(vsm_get_zone_name(current_zone), MSG_ZONE_KNOX) && real_zone == NULL) {
		real_zone = vsm_join_zone(effective_zone);
		MSG_DEBUG("Change Zone");
		zone_ref_count++;
	}
#endif
	MSG_END();
}


void MsgZoneRevert()
{
	MSG_BEGIN();
#ifdef FEATURE_CONTAINER_ENABLE
	if (real_zone == NULL || zone_ref_count == 0) {
		MSG_DEBUG("real_zone == NULL || zone_ref_count == 0");
		return;
	}
	zone_ref_count--;
	if (zone_ref_count == 0) {
		vsm_join_zone(real_zone);
		real_zone = NULL;
		MSG_DEBUG("Revert Zone");
	}
#endif
	MSG_END();
}
