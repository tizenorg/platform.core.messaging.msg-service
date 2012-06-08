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

// for sl message browser launch
#include <app_service.h>

#include "MsgDebug.h"
#include "MsgUtilFile.h"
#include "MsgContact.h"
#include "MsgUtilStorage.h"
#include "MsgGconfWrapper.h"
#include "MsgSoundPlayer.h"
#include "MsgPluginManager.h"
#include "MsgStorageHandler.h"
#include "MsgDeliverHandler.h"
#include "MsgNotificationWrapper.h"

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/
extern MsgDbHandler dbHandle;


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
MSG_ERROR_T MsgHandleMmsConfIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_ID_T reqID)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_DEBUG(" msgtype subtype is [%d]", pMsgInfo->msgType.subType);

	if (pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgInfo->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		// keep origianl subType
		MSG_SUB_TYPE_T subType = pMsgInfo->msgType.subType;

		/* If Retrieve Failed, Msg SubType is remained as Notification Ind */
		if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_FAIL)
			pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS;

		err = MsgStoUpdateMMSMessage(pMsgInfo);

		if (err == MSG_SUCCESS)
			MSG_DEBUG("Command Handle Success : MsgStoUpdateMessage()");

		if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
			MSG_DEBUG("### MSG_NETWORK_RETRIEVE_SUCCESS ###");
			//Update Mms Message to MMS Plugin DB

			// MMS Received Ind Process Func
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsgInfo->msgType.mainType);

			//Contents of msg Data was removed and replaced to retrievedFilePath
			// NOTICE:: now it was moved to handleEvent in MsgListnerThread
			err = plg->updateMessage(pMsgInfo, NULL, NULL);
			if (err != MSG_SUCCESS)
				return MSG_ERR_STORAGE_ERROR;
		}

		MSG_DEBUG(" ######################### MESSAGE UPDATE COMPLETED!!! ######################");

		bool readStatus = false;

		MsgStoGetReadStatus(pMsgInfo->msgId, &readStatus);
		MSG_DEBUG("### readStatus = %d ###", readStatus);

		//Update Read Status to Unread beacaus Noti is Read
		if (subType == MSG_RETRIEVE_MANUALCONF_MMS && pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
			MSG_DEBUG("###2. subType = %d ###", pMsgInfo->msgType.subType);

			if (readStatus == true)
				MsgStoSetReadStatus(&dbHandle, pMsgInfo->msgId, false);
		}

		// update badge
		if (subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
			int smsCnt = 0;
			int mmsCnt = 0;

			smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
			mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

			MsgSettingHandleNewMsg(smsCnt, mmsCnt);
		}

		if (subType == MSG_RETRIEVE_AUTOCONF_MMS) {
			// play message-tone when MMS retrieved
			MsgSoundPlayStart();
			MsgInsertNoti(&dbHandle, pMsgInfo);
		}  else if (subType == MSG_RETRIEVE_MANUALCONF_MMS) {
			if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
				MSG_DEBUG("Manual success");
				MsgInsertTicker("Message Retrieved", MESSAGE_RETRIEVED);
			} else if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_FAIL) {
				MSG_DEBUG("Manual failed");
				MsgInsertTicker("Retrieving message failed", RETRIEVING_MESSAGE_FAILED);
			}
		}
	}
	else if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS || pMsgInfo->msgType.subType == MSG_SENDCONF_MMS)
	{
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsgInfo->msgType.mainType);
		// change subType for storage update
		pMsgInfo->msgType.subType = MSG_SENDCONF_MMS;

		err = MsgStoUpdateMMSMessage(pMsgInfo);
		if (err == MSG_SUCCESS)
			MSG_DEBUG("Command Handle Success : MsgStoUpdateMMSMessage()");

		err = plg->updateMessage(pMsgInfo, NULL, NULL);
		if (err != MSG_SUCCESS)
			return MSG_ERR_STORAGE_ERROR;

		MSG_DEBUG("pMsg->networkStatus : %d", pMsgInfo->networkStatus);
		err = MsgStoUpdateNetworkStatus(pMsgInfo, pMsgInfo->networkStatus);
		if (err != MSG_SUCCESS)
			return MSG_ERR_STORAGE_ERROR;

		if (pMsgInfo->networkStatus == MSG_NETWORK_SEND_SUCCESS)
		{
			err = MsgStoMoveMessageToFolder(pMsgInfo->msgId, MSG_SENTBOX_ID);
			if (err != MSG_SUCCESS)
				return MSG_ERR_STORAGE_ERROR;
		}

		// Get subject and text
		MsgStoGetText(pMsgInfo->msgId, pMsgInfo->subject, pMsgInfo->msgText);

		MSG_DEBUG(" ######################### MESSAGE UPDATE COMPLETED!!! ######################");
	}
	return err;
}

MSG_ERROR_T MsgHandleIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti)
{
	MSG_BEGIN();

	MSG_ERROR_T err = MSG_SUCCESS;

	if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE)
	{
		if (pMsgInfo->msgPort.valid == true)
			return MSG_SUCCESS;

		err = MsgHandleSMS(pMsgInfo, pSendNoti);

		if (err == MSG_SUCCESS && *pSendNoti == true)
		{
			MsgSoundPlayStart();

//			if (pMsgInfo->msgType.subType != MSG_STATUS_REPORT_SMS)
			{
				int smsCnt = 0, mmsCnt = 0;

				smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
				mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

				MsgSettingHandleNewMsg(smsCnt, mmsCnt);
				MsgInsertNoti(&dbHandle, pMsgInfo);
			}
		}
	}
	else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE)
	{
		err = MsgHandleMMS(pMsgInfo, pSendNoti);
	}

	MSG_END();

	return err;
}


MSG_ERROR_T MsgHandleSMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	if ((pMsgInfo->msgType.subType >= MSG_WAP_SI_SMS) && (pMsgInfo->msgType.subType <= MSG_WAP_CO_SMS))
	{
		MSG_DEBUG("Starting WAP Message Incoming.");

		MSG_PUSH_SERVICE_TYPE_T serviceType = (MSG_PUSH_SERVICE_TYPE_T)MsgSettingGetInt(PUSH_SERVICE_TYPE);
		service_h svc_handle = NULL;

		switch (pMsgInfo->msgType.subType)
		{
			case MSG_WAP_SL_SMS:
			{
				*pSendNoti = false;

				if (serviceType == MSG_PUSH_SERVICE_ALWAYS) {
					if (service_create(&svc_handle) < 0) {
						MSG_DEBUG("Fail to create service handle");
						break;
					}
					if (!svc_handle) {
						MSG_DEBUG("Service handle is NULL");
						break;
					}
					if (service_set_operation(svc_handle, SERVICE_OPERATION_VIEW) < 0) {
						MSG_DEBUG("Fail to create service handle");
						service_destroy(svc_handle);
						break;
					}
					if (service_set_uri(svc_handle, pMsgInfo->msgText) < 0) {
						MSG_DEBUG("Fail to set uri");
						service_destroy(svc_handle);
						break;
					}
					if (service_set_package(svc_handle, MSG_SVC_PKG_NAME_BROWSER) < 0) {
						MSG_DEBUG("Fail to set package");
						service_destroy(svc_handle);
						break;
					}
					if (service_send_launch_request(svc_handle, NULL, NULL) < 0) {
						MSG_DEBUG("Fail to launch browser");
						service_destroy(svc_handle);
						break;
					}

					service_destroy(svc_handle);

				} else if (serviceType == MSG_PUSH_SERVICE_PROMPT) {
					char urlString[MAX_COMMAND_LEN+1];
					memset(urlString, 0x00, sizeof(urlString));

					snprintf(urlString, MAX_COMMAND_LEN, "message-dialog -m PUSH_MSG_ALWAYS_ASK -u %s &", pMsgInfo->msgText);

					system(urlString);
				}

			}
			break;

			case MSG_WAP_SI_SMS:
			case MSG_WAP_CO_SMS:
			{
				*pSendNoti = false;
			}
			break;
		}
	}
	else if (pMsgInfo->msgType.subType == MSG_STATUS_REPORT_SMS)
	{
		unsigned int addrId = 0;

		// Get Address ID
		MsgExistAddress(&dbHandle, pMsgInfo->addressList[0].addressVal, &addrId);

		MSG_DEBUG("address ID : [%d], Value : [%s]", addrId, pMsgInfo->addressList[0].addressVal);

		pMsgInfo->addressList[0].threadId = (MSG_THREAD_ID_T)addrId;
	}

	return err;
}


MSG_ERROR_T MsgHandleMMS(MSG_MESSAGE_INFO_S *pMsgInfo,  bool *pSendNoti)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_REQUEST_INFO_S request = {0};
	bool bReject = false;

	// MMS Received Ind Process Func
	MSG_MAIN_TYPE_T msgMainType = pMsgInfo->msgType.mainType;
	MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(msgMainType);

	// Need to process m-delivery-ind, m-notification-ind, m-read-orig-ind
	err = plg->processReceivedInd(pMsgInfo, &request, &bReject);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Process Message Success : processReceivedInd(), btextsms %d", pMsgInfo->bTextSms);
	} else {
		MSG_DEBUG("Process Message Fail : processReceivedInd()");
		return err;
	}

	// Add into DB
	if ((pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) && bReject == false) {

		err = MsgStoAddMessage(pMsgInfo, NULL);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddMessage() Error: [%d]", err);
			return err;
		}
	} else if (pMsgInfo->msgType.subType == MSG_READORGIND_MMS || pMsgInfo->msgType.subType == MSG_DELIVERYIND_MMS) {
		MsgSoundPlayStart();
		*pSendNoti = false;
	}

	//In the case of m-notification-ind, we should decide whether to send m-notify-response-ind or http 'Get'
	//submit request
	if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		if (request.msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS && bReject == false) {
			MsgSoundPlayStart();

			int smsCnt = 0;
			int mmsCnt = 0;

			smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
			mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

			MsgSettingHandleNewMsg(smsCnt, mmsCnt);
			MsgInsertNoti(&dbHandle, pMsgInfo);
		}

		request.msgInfo.msgId = pMsgInfo->msgId;

		MSG_DEBUG("-=====================[[[ %s ]]]] =========================", pMsgInfo->msgData);
		err = plg->submitReq(&request, false);

		if (err == MSG_SUCCESS) {
			MSG_DEBUG("Process Message Success : processReceivedInd()");
		} else {
			MSG_DEBUG("Process Message Fail : processReceivedInd()");
		}
	}

	return err;
}

