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

	g_thread_init(NULL);

	bool notEnd = false;

	if(argc < 1)
	{
		MSG_DEBUG("No arguments to run msg_helper.");
		return 0;
	}

	MSG_DEBUG("argv[0] [%s] ", argv[0]);

	if(strcmp(argv[0],MSG_SOUND_START)==0)
	{
		MsgSoundPlayStart();
		notEnd = true;
	}
	else if(strcmp(argv[0],MSG_SOUND_STOP)==0)
	{
		MsgSoundPlayStop();
	}

	if(notEnd)
	{
		loop = g_main_loop_new(NULL, FALSE);


		if (loop != NULL)
		{
			MSG_DEBUG("Waiting for working jobs to be finished!!!");

			// Run GMainLoop
			g_main_loop_run(loop);
		}
		else
		{
			MSG_DEBUG("Fail to create g_main_loop!!!");
		}
	}

	return 0;
}
