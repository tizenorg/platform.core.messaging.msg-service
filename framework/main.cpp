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
#include "MsgUtilStorage.h"
#include "MsgNotificationWrapper.h"

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

	if(chmod( MSG_IPC_DATA_PATH, S_IRWXU | S_IRWXG ) !=0) { //public shared file: pass data by file
		MSG_DEBUG("Fail to chmod [%s].", MSG_IPC_DATA_PATH);
	}
	chown( MSG_IPC_DATA_PATH, 0, 6502 );

	return MSG_SUCCESS;
}


void SendMobileTrackerMsg()
{
	MSG_BEGIN();

	bool bEnabled = false;

	if (MsgSettingGetBool(VCONFKEY_SETAPPL_FIND_MY_MOBILE_SIM_CHANGE_ALERT_BOOL, &bEnabled) < 0)
	{
		MSG_DEBUG("Can't read VCONFKEY_SETAPPL_FIND_MY_MOBILE_SIM_CHANGE_ALERT_BOOL");
		return;
	}

	if (bEnabled == false)
	{
		MSG_DEBUG("Mobile Tracker Option [%d]", bEnabled);
		return;
	}
	// to wait modem init
	// temporary code.
	else
	{
		MSG_DEBUG("Waiting for modem ready, 22 sec.");
		sleep(22);
	}

	MSG_REQUEST_INFO_S req = {0};

	req.sendOptInfo.bSetting = false;

	req.msgInfo.msgId = 0;
	req.msgInfo.threadId = 0;
	req.msgInfo.folderId = MSG_DRAFT_ID;
	req.msgInfo.msgType.mainType = MSG_SMS_TYPE;
	req.msgInfo.msgType.subType = MSG_NORMAL_SMS;
	req.msgInfo.msgType.classType = MSG_CLASS_NONE;
	req.msgInfo.storageId = MSG_STORAGE_PHONE;
	req.msgInfo.displayTime = 0;
	req.msgInfo.networkStatus = MSG_NETWORK_NOT_SEND;
	req.msgInfo.encodeType = MSG_ENCODE_AUTO;
	req.msgInfo.bRead = false;
	req.msgInfo.bProtected = false;
	req.msgInfo.priority = MSG_MESSAGE_PRIORITY_NORMAL;
	req.msgInfo.direction = MSG_DIRECTION_TYPE_MO;
	req.msgInfo.msgPort.valid = false;
	req.msgInfo.bTextSms = true;

	// Get Message Text
	char* msgText = NULL;

	msgText = MsgSettingGetString(VCONFKEY_SETAPPL_FIND_MY_MOBILE_MESSAGE_STR);

	if (msgText == NULL)
	{
		MSG_DEBUG("Assign Default Msg");
		msgText = strdup(MSG_MOBILE_TRACKER_MSG);
		if (msgText == NULL) {
			MSG_DEBUG("msgText is NULL.");
			return;
		}
	}

	MSG_DEBUG("mobile tracker msg : [%s]", msgText);

	req.msgInfo.dataSize = strlen(msgText);
	strncpy(req.msgInfo.msgText, msgText, req.msgInfo.dataSize);

	// Get Address List
	char *orgRecipientList = NULL;

	orgRecipientList = MsgSettingGetString(VCONFKEY_SETAPPL_FIND_MY_MOBILE_RECIPIENTS_STR);

	if (orgRecipientList == NULL)
	{
		MSG_DEBUG("recipient list is NULL");
		free(msgText);
		msgText = NULL;
		return;
	}

	int len = strlen(orgRecipientList);
	char recipientList[len + 1];

	memset(recipientList, 0, len + 1);
	memcpy(recipientList, orgRecipientList, len);

	MSG_DEBUG("recipient list : [%s]", recipientList);

	req.msgInfo.nAddressCnt = 1;

	char *token;

	token = strtok(recipientList, "|");

	MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

	msg_error_t err = MSG_SUCCESS;

	while (token)
	{
		req.msgInfo.addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
		req.msgInfo.addressList[0].recipientType = MSG_RECIPIENTS_TYPE_TO;

		memset(req.msgInfo.addressList[0].addressVal, 0x00, MAX_ADDRESS_VAL_LEN);
		strncpy(req.msgInfo.addressList[0].addressVal, token, MAX_ADDRESS_VAL_LEN);

		MSG_DEBUG("address : [%s]", req.msgInfo.addressList[0].addressVal);

		if (plg != NULL)
		{
			MSG_DEBUG("mobile tracker msg : [%s]", msgText);

			err = plg->submitReq(&req);

			if (err != MSG_SUCCESS)
			{
				MSG_DEBUG("fail to send mobile tracker msg : [%d]", err);
				break;
			}
		}

		token = strtok(NULL, "|");
	}

	if (msgText) {
		free(msgText);
		msgText = NULL;
	}

	if (orgRecipientList) {
		free(orgRecipientList);
		orgRecipientList = NULL;
	}


	MSG_END();

	return;
}


void* StartMsgServer(void*)
{

	MsgOpenContactSvc();

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

	MsgCloseContactSvc();

	return (void*)0;
}


void* InitMsgServer(void*)
{
	msg_error_t err = MSG_SUCCESS;

	MsgOpenContactSvc();

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

	MSG_MAIN_TYPE_T mainType = MSG_SMS_TYPE;
	MsgPlugin* plg = MsgPluginManager::instance()->getPlugin(mainType);

	// storage handler initialize
	err = MsgStoInitDB(false);

	if (err != MSG_SUCCESS) {
		MSG_DEBUG("FAIL TO INITIALIZE STORAGE HANDLER [%d]", err);
	}

	// Set Msg FW Ready Flag
	MsgSettingSetBool(VCONFKEY_MSG_SERVER_READY, true);
	MSG_DEBUG("### VCONFKEY_MSG_SERVER_READY ###");

	if (plg == NULL) {
		MSG_DEBUG("No plugin for %d type", mainType);

		MsgReleaseMemory();
		return (void*)0;
	}

	// Clear and reset notification
	MsgCleanAndResetNoti();

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

		// Add SendMobileTrackerMsg() to GMainLoop
		if (simStatus == MSG_SIM_STATUS_CHANGED) {
			MSG_DEBUG("Send Mobile Tracker Message");

			SendMobileTrackerMsg();
		}
	} else {
		MSG_DEBUG("checkSimStatus() error");
	}

	MsgReleaseMemory();

	// Register Callback to get the change of contact
	MsgInitContactSvc(&MsgContactChangedCallback);

	MsgSoundInitRepeatAlarm();

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
#if !GLIB_CHECK_VERSION(2, 31, 0)
	g_thread_init(NULL);
#endif
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

	// Regist vconf CB.
	MsgSettingRegVconfCB();

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

	// Remove vconf CB
	MsgSettingRemoveVconfCB();

	// Close Contact Sevice
	MsgCloseContactSvc();

	// Disconnect to DB
	MsgStoDisconnectDB();

	return 0;
}

