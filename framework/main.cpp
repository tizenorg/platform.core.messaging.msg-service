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

#include <errno.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <sys/stat.h>
#include <wait.h>

static GMainLoop* mainloop = NULL;


/*==================================================================================================
                                     DEFINES
==================================================================================================*/
#define MSG_MOBILE_TRACKER_MSG "Mobile Tracker Alert"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T InitMmsDir()
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
			MSG_DEBUG("The %s already exists", MSG_SMIL_FILE_PATH);
		}
		else
		{
			MSG_DEBUG("Error while mkdir %s", MSG_SMIL_FILE_PATH);
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
			char exeStr[1024];
			snprintf(exeStr, 1024, "rm %s*.DATA", MSG_IPC_DATA_PATH);
			system(exeStr);
		}
		else
		{
			MSG_DEBUG("Error while mkdir %s", MSG_IPC_DATA_PATH);
			return MSG_ERR_DB_MAKE_DIR;
		}
	}

	chmod( MSG_IPC_DATA_PATH, S_IRWXU | S_IRWXG ); //public shared file: pass data by file
	chown( MSG_IPC_DATA_PATH, 0, 6502 );

	return MSG_SUCCESS;
}


void* StartMsgServer(void*)
{
	try
	{
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

	if (g_main_loop_is_running(mainloop))
		g_main_loop_quit(mainloop);

	return (void*)0;
}


void* InitMsgServer(void*)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

	// storage handler initialize
	err = MsgStoInitDB(false);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("FAIL TO INITIALIZE STORAGE HANDLER [%d]", err);
	}

	if (plg == NULL) {
		MSG_DEBUG("No plugin for %d type", mainType);

		MsgReleaseMemory();

		// Set Msg FW Ready Flag
		MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, true);

		return (void*)0;
	}

	MSG_SIM_STATUS_T simStatus = MSG_SIM_STATUS_NORMAL;

	// Check Sim Status
	if (plg->checkSimStatus(&simStatus) == MSG_SUCCESS) {

		// Add the change of SIM to vconf
		if (MsgSettingSetInt(MSG_SIM_CHANGED, (int)simStatus) != MSG_SUCCESS) {
			MSG_DEBUG("Error to set config data [%s]", MSG_SIM_CHANGED);
		}

		if (simStatus != MSG_SIM_STATUS_NOT_FOUND) {
			// Check Device Status
			if (plg->checkDeviceStatus() != MSG_SUCCESS) {
				MSG_DEBUG("checkDeviceStatus() error");

				MsgReleaseMemory();

				// Set Msg FW Ready Flag
				MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, true);

				return (void*)0;
			}
		}

		// Init SIM Message
		if (MsgInitSimMessage(simStatus) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to Initialize SIM Message");
		}

		// Init SIM Configuration
		if (MsgInitSimConfig(simStatus) != MSG_SUCCESS) {
			MSG_DEBUG("Fail to Initialize SIM Configuration");
		}
	} else {
		MSG_DEBUG("checkSimStatus() error");
	}

	MsgReleaseMemory();

	// Try to connect contact server  if it is not opened.
	MsgOpenContactSvc();

	// Register Callback to get the change of contact
	MsgInitContactSvc(&MsgContactChangedCallback);

	// Set Msg FW Ready Flag
	MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, true);

	return (void*)0;
}


static gboolean InitThreadFunc(void* pData)
{
	MSG_BEGIN();

	pthread_t initThreadId;

	// initialize msg fw
	if (pthread_create(&initThreadId, NULL, InitMsgServer, NULL) != 0)
	{
		MSG_DEBUG("InitMsgFw not invoked: %s", strerror(errno));
		return -1;
	}

	pthread_detach(initThreadId);

	MSG_END();

	return FALSE;
}


int main(void)
{
	g_thread_init(NULL);
	dbus_g_thread_init();

////////////////////////////////////

/// set to ignore child process terminated signal.
signal( SIGCHLD, SIG_IGN );

////////////////////////////////////


	MSG_DEBUG("===========START MESSAGING FRAMEWORK==========");

	// Reset message server ready flag
	MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, false);

	// Connect to DB
	//	MsgStoConnectDB();

	// Open Contact Service
	MsgOpenContactSvc();

	// Clean up mms dir
	InitMmsDir();

	try
	{
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

	pthread_t startThreadId;

	// start transaction manager
	if (pthread_create(&startThreadId, NULL, StartMsgServer, NULL) != 0)
	{
		MSG_DEBUG("StartMsgServer not invoked: %s", strerror(errno));
		return -1;
	}

	MsgTransactionManager::instance()->getTMStatus();

	mainloop = g_main_loop_new(NULL, FALSE);

	g_type_init();

	g_idle_add(InitThreadFunc, NULL);

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

	// Close Contact Sevice
	MsgCloseContactSvc();

	// Disconnect to DB
	MsgStoDisconnectDB();

	return 0;
}

