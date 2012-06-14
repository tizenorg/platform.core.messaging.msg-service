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
