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

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgCallStatusManager.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgContact.h"
#include "MsgMemory.h"
#include "MsgGconfWrapper.h"
#include "MsgPluginManager.h"
#include "MsgSettingHandler.h"
#include "MsgStorageHandler.h"
#include "MsgSubmitHandler.h"
#include "MsgDeliverHandler.h"
#include "MsgTransManager.h"
#include "MsgStorageTypes.h"
#include "MsgSoundPlayer.h"
#include "MsgCmdHandler.h"
#include "MsgUtilFile.h"
#include "MsgUtilStorage.h"
#include "MsgNotificationWrapper.h"

#include <errno.h>
#include <glib.h>
#include <sys/stat.h>
#include <wait.h>

static GMainLoop* mainloop = NULL;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/


void* InitMsgServer(void*)
{
	msg_error_t err = MSG_SUCCESS;
	MSG_DEBUG("Start InitMsgServer.");

#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	/* Init contact digit number */
	MsgInitContactSvc();
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

	MsgInitCallStatusManager();

	try {
		/* ipc data folder set acl for priv_read and priv_write */
		if (!MsgAclInit()) {
			MSG_ERR("FAIL TO INITIALIZE ACL [%d]", err);
		}

		/* storage handler initialize */
		err = MsgStoInitDB(false);
		if (err != MSG_SUCCESS) {
			MSG_ERR("FAIL TO INITIALIZE STORAGE HANDLER [%d]", err);
		}

		MsgInitNoti();

		/* plugin manager initialize */
		MsgPluginManager::instance()->initialize();
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
	}

/*	MsgSoundPlayer::instance()->MsgSoundInitRepeatAlarm(); */

	MsgStoDisconnectDB();

	MsgReleaseMemory();
	MSG_DEBUG("End InitMsgServer.");

	return (void*)0;
}


void* StartMsgServer(void*)
{
	try {
		if (MsgTransactionManager::instance()->initCynara() == false) {
			MSG_ERR("Cynara initialize failed. It will try again when API is called.");
		}

		MsgTransactionManager::instance()->run();
	} catch (MsgException& e) {
		MSG_FATAL("%s", e.what());
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
	}

	MsgTransactionManager::instance()->finishCynara();

	if (g_main_loop_is_running(mainloop))
		g_main_loop_quit(mainloop);

	return (void*)0;
}


int main(void)
{
#if !GLIB_CHECK_VERSION(2, 31, 0)
	g_thread_init(NULL);
#endif
	/* set to ignore child process terminated signal */
	signal(SIGCHLD, SIG_IGN);

	MSG_INFO("===========START MESSAGING FRAMEWORK==========");

#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init();
#endif
	/* Reset message server ready flag */
	if(MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, false) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetBool FAIL: VCONFKEY_MSG_SERVER_READY");

	/* init server */
	InitMsgServer(NULL);

	pthread_t startThreadId;

	/* start transaction manager */
	if (pthread_create(&startThreadId, NULL, StartMsgServer, NULL) != 0) {
		MSG_DEBUG("StartMsgServer not invoked: %s", g_strerror(errno));
		return -1;
	}


	mainloop = g_main_loop_new(NULL, FALSE);

	if (mainloop != NULL) {
		MSG_DEBUG("Start Messaging Framework!!!");

		/* Run GMainLoop */
		g_main_loop_run(mainloop);
	} else {
		MSG_DEBUG("Fail to start Messaging Framework!!!");
	}

	/* Disconnect to DB */
	MsgStoDisconnectDB();

	MsgDeInitCallStatusManager();

	return 0;
}

