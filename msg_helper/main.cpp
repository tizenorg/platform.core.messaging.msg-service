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

#include <stdio.h>
#include <glib.h>
#include <sys/time.h>
#include <stdlib.h>

#include "MsgHelper.h"
#include "MsgUtilFile.h"
#include "MsgCppTypes.h"
#include "MsgDebug.h"

static GMainLoop *loop;

static gboolean _worker_done(void* data)
{
	if (g_main_loop_is_running(loop))
		g_main_loop_quit(loop);

	return 0;
}

// may called by threads
void worker_done()
{
	g_idle_add(_worker_done,NULL);
}

int main(int argc, char** argv)
{

	MSG_DEBUG("############### Start msg_helper ###############");

#if !GLIB_CHECK_VERSION(2, 31, 0)
	g_thread_init(NULL);
#endif

	bool notEnd = false;

	if(argc < 1)
	{
		MSG_DEBUG("No arguments to run msg_helper.");
		return 0;
	}

	MSG_DEBUG("argv[0] [%s] ", argv[0]);

	if(g_strcmp0(argv[0], MSG_NORMAL_SOUND_START) == 0)
	{
		MsgSoundPlayStart(false);
		notEnd = true;
	}
	if(g_strcmp0(argv[0], MSG_EMERGENCY_SOUND_START) == 0)
	{
		MsgSoundPlayStart(true);
		notEnd = true;
	}
	else if(g_strcmp0(argv[0],MSG_SOUND_STOP) == 0)
	{
		MsgSoundPlayStop();
	}

	if(notEnd)
	{
		loop = g_main_loop_new(NULL, FALSE);

		if (MsgSensorConnect() == MSG_SUCCESS)
			if (MsgRegSensorCB(&worker_done) != MSG_SUCCESS)
				MsgSensorDisconnect();

		if (loop != NULL)
		{
			MSG_DEBUG("Waiting for working jobs to be finished!!!");

			// Run GMainLoop
			g_main_loop_run(loop);
		}
		else
		{
			MSG_DEBUG("Fail to create g_main_loop!!!");
			MsgSensorDisconnect();
		}
	}

	return 0;
}
