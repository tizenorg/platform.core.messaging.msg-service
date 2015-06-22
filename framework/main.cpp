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
#include "MsgZoneManager.h"

#include <errno.h>
#include <glib.h>
#include <sys/stat.h>
#include <wait.h>

static GMainLoop* mainloop = NULL;

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

msg_error_t InitMmsDir()
{
	if (mkdir(MSG_DATA_ROOT_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
	{
		if (errno == EEXIST)
		{
			MSG_DEBUG("The %s already exists", MSG_DATA_ROOT_PATH);
		}
		else
		{
			MSG_DEBUG("Error while mkdir %s", MSG_DATA_ROOT_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	if (mkdir(MSG_SMIL_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
	{
		if (errno == EEXIST)
		{
			MSG_SEC_DEBUG("The %s already exists", MSG_SMIL_FILE_PATH);
		}
		else
		{
			MSG_SEC_DEBUG("Error while mkdir %s", MSG_SMIL_FILE_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	if (mkdir(MSG_DATA_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
	{
		if (errno == EEXIST)
		{
			MSG_DEBUG("The %s already exists", MSG_DATA_PATH);
		}
		else
		{
			MSG_DEBUG("Error while mkdir %s", MSG_DATA_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	if (mkdir(MSG_THUMBNAIL_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
		if (errno == EEXIST) {
			MSG_DEBUG("The %s already exists.", MSG_THUMBNAIL_PATH);
		} else {
			MSG_DEBUG(" Error while mkdir %s", MSG_THUMBNAIL_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	if (mkdir(MSG_IPC_DATA_PATH, S_IRWXU ) < 0)
	{
		if (errno == EEXIST)
		{
			MSG_DEBUG("The %s already exists", MSG_IPC_DATA_PATH);
			// if IPC data path is already exist, clear all files in folder.
			MsgRmRf((char *)MSG_IPC_DATA_PATH);
		}
		else
		{
			MSG_DEBUG("Error while mkdir %s", MSG_IPC_DATA_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	if (MsgChmod( MSG_IPC_DATA_PATH, S_IRWXU | S_IRWXG) == 0) { //public shared file: pass data by file
		MSG_DEBUG("Fail to chmod [%s].", MSG_IPC_DATA_PATH);
	}
	if (MsgChown(MSG_DATA_ROOT_PATH, 200, 5000) == 0) {
		MSG_DEBUG("Fail to chown [%s].", MSG_DATA_ROOT_PATH);
	}
	if (MsgChown(MSG_DATA_PATH, 200, 5000) == 0) {
		MSG_DEBUG("Fail to chown [%s].", MSG_DATA_PATH);
	}
	if (MsgChown(MSG_SMIL_FILE_PATH, 200, 5000) == 0) {
		MSG_DEBUG("Fail to chown [%s].", MSG_SMIL_FILE_PATH);
	}
	if (MsgChown(MSG_IPC_DATA_PATH, 200, 5000) == 0) {
		MSG_DEBUG("Fail to chown [%s].", MSG_IPC_DATA_PATH);
	}
	if (MsgChown(MSG_THUMBNAIL_PATH, 200, 5000) == 0) {
		MSG_DEBUG("Fail to chown [%s].", MSG_THUMBNAIL_PATH);
	}

	return MSG_SUCCESS;
}


void* InitMsgServer(void*)
{
	msg_error_t err = MSG_SUCCESS;
	MSG_DEBUG("Start InitMsgServer.");

	//CID 356902: Moving try block up to include MsgStoInitDB which also throws MsgException
	try
	{
		// storage handler initialize
		err = MsgStoInitDB(false);
		if (err != MSG_SUCCESS) {
			MSG_ERR("FAIL TO INITIALIZE STORAGE HANDLER [%d]", err);
		}

		MsgInitNoti();

		// plugin manager initialize
		MsgPluginManager::instance()->initialize();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
	}
	catch (exception& e)
	{
		MSG_FATAL("%s", e.what());
	}


//	MsgSoundPlayer::instance()->MsgSoundInitRepeatAlarm();

	MsgStoDisconnectDB();

	MsgReleaseMemory();
	MSG_DEBUG("End InitMsgServer.");

	return (void*)0;
}


void* StartMsgServer(void*)
{
	try
	{
		if (MsgTransactionManager::instance()->initCynara() == false) {
			MSG_ERR("Cynara initialize failed. It will try again when API is called.");
		}

		MsgTransactionManager::instance()->run();
	}
	catch (MsgException& e)
	{
		MSG_FATAL("%s", e.what());
	}
	catch (exception& e)
	{
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
	// Reset message server ready flag
	if(MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, false) != MSG_SUCCESS)
		MSG_DEBUG("MsgSettingSetBool FAIL: VCONFKEY_MSG_SERVER_READY");

	// Connect to DB
	//	MsgStoConnectDB();

	// Clean up mms dir
	InitMmsDir();

	// init server
	InitMsgServer(NULL);

	pthread_t startThreadId;

	// start transaction manager
	if (pthread_create(&startThreadId, NULL, StartMsgServer, NULL) != 0)
	{
		MSG_DEBUG("StartMsgServer not invoked: %s", strerror(errno));
		return -1;
	}

	// Regist vconf CB.
	MsgSettingRegVconfCB();

	mainloop = g_main_loop_new(NULL, FALSE);

	if (mainloop != NULL)
	{
		MSG_DEBUG("Start Messaging Framework!!!");

		// Run GMainLoop
		g_main_loop_run(mainloop);
	}
	else
	{
		MSG_DEBUG("Fail to start Messaging Framework!!!");
	}

	// Remove vconf CB
	MsgSettingRemoveVconfCB();
	//contacts-service is not used for gear
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	// Close Contact Sevice
	MsgCloseContactSvc();
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED

	// Disconnect to DB
	MsgStoDisconnectDB();

	return 0;
}

