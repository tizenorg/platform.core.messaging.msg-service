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
#include "MsgSpamFilter.h"
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
msg_error_t MsgHandleMmsConfIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_request_id_t reqID)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

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
			MsgSoundPlayStart(false);

			// add phone log
			MSG_DEBUG("Enter MsgAddPhoneLog() for mms message.");
			MsgAddPhoneLog(pMsgInfo);

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
		err = MsgStoUpdateNetworkStatus(&dbHandle, pMsgInfo, pMsgInfo->networkStatus);
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

msg_error_t MsgHandleIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;

	if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE) {

		bool bOnlyNoti = false;

		err = MsgHandleSMS(pMsgInfo, pSendNoti, &bOnlyNoti);

		if (err == MSG_SUCCESS) {
			MsgSoundPlayStart(false);
			if (*pSendNoti == true) {
				int smsCnt = 0, mmsCnt = 0;

				smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
				mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

				MsgSettingHandleNewMsg(smsCnt, mmsCnt);
				MsgInsertNoti(&dbHandle, pMsgInfo);
			} else if (bOnlyNoti == true) {
				MsgInsertNoti(pMsgInfo);
			}
		}
	}
	else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE)
	{
		err = MsgHandleMMS(pMsgInfo, pSendNoti);
	}

	//Add Phone Log Data
	if ((err == MSG_SUCCESS) &&
		(pMsgInfo->folderId == MSG_INBOX_ID || pMsgInfo->folderId == MSG_SPAMBOX_ID) &&
		(pMsgInfo->msgType.mainType == MSG_SMS_TYPE) &&
		(pMsgInfo->msgType.subType != MSG_WAP_SL_SMS)) {

		MSG_DEBUG("Enter MsgAddPhoneLog() : pMsg->folderId [%d]", pMsgInfo->folderId);

		MsgAddPhoneLog(pMsgInfo);
	}

	MSG_END();

	return err;
}


msg_error_t MsgHandleSMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti, bool *bOnlyNoti)
{
	msg_error_t err = MSG_SUCCESS;

	if (pMsgInfo->msgPort.valid == true) {
			*pSendNoti = false;
			*bOnlyNoti = false;
			return MSG_SUCCESS;
	}
	if (pMsgInfo->msgType.subType == MSG_NORMAL_SMS) {
		if (MsgCheckFilter(&dbHandle, pMsgInfo) == true) {
			// Move to SpamBox
			err = MsgStoMoveMessageToFolder(pMsgInfo->msgId, MSG_SPAMBOX_ID);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoMoveMessageToFolder() Error : [%d]", err);
			} else {
				pMsgInfo->folderId = MSG_SPAMBOX_ID;
			}

			// Update Conversation table
			err = MsgStoUpdateConversation(&dbHandle, pMsgInfo->threadId);

			if (err != MSG_SUCCESS)
				MSG_DEBUG("MsgStoUpdateConversation() Error : [%d]", err);

			*pSendNoti = false;
			*bOnlyNoti = false;
		}
	} else if ((pMsgInfo->msgType.subType >= MSG_WAP_SI_SMS) && (pMsgInfo->msgType.subType <= MSG_WAP_CO_SMS)) {
		MSG_DEBUG("Starting WAP Message Incoming.");

		MSG_PUSH_SERVICE_TYPE_T serviceType = (MSG_PUSH_SERVICE_TYPE_T)MsgSettingGetInt(PUSH_SERVICE_TYPE);
		service_h svc_handle = NULL;

		switch (pMsgInfo->msgType.subType) {
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

					snprintf(urlString, MAX_COMMAND_LEN, "/usr/apps/org.tizen.message/bin/message-dialog -m PUSH_MSG_ALWAYS_ASK -u %s &", pMsgInfo->msgText);

					system(urlString);
				}

			}
			break;

			case MSG_WAP_SI_SMS:
				*pSendNoti = true;
				break;
			case MSG_WAP_CO_SMS:
				*pSendNoti = false;
				break;
		}
	} else if (pMsgInfo->msgType.subType == MSG_STATUS_REPORT_SMS) {
		*pSendNoti = false;
		*bOnlyNoti = true;
	} else if (pMsgInfo->msgType.subType >= MSG_MWI_VOICE_SMS && pMsgInfo->msgType.subType <= MSG_MWI_OTHER_SMS) {
		if (pMsgInfo->bStore == false) {
			*pSendNoti = false;
			*bOnlyNoti = true;
		}
	} 


	return err;
}


msg_error_t MsgHandleMMS(MSG_MESSAGE_INFO_S *pMsgInfo,  bool *pSendNoti)
{
	msg_error_t err = MSG_SUCCESS;

	MSG_REQUEST_INFO_S request = {0};
	bool bReject = false;
	bool bFiltered = false;

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
		bFiltered = MsgCheckFilter(&dbHandle, pMsgInfo);

		if (bFiltered == true) {
			pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
			*pSendNoti = false;
		}

		err = MsgStoAddMessage(pMsgInfo, NULL);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddMessage() Error: [%d]", err);
			return err;
		}
	} else if (pMsgInfo->msgType.subType == MSG_READORGIND_MMS || pMsgInfo->msgType.subType == MSG_DELIVERYIND_MMS) {
		if (MsgInsertMmsReportToNoti(&dbHandle, pMsgInfo) == MSG_SUCCESS) {
			MsgSoundPlayStart(false);
			*pSendNoti = false;
		}
	}

	//In the case of m-notification-ind, we should decide whether to send m-notify-response-ind or http 'Get'
	//submit request
	if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS && bFiltered == false) {
		if (request.msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS && bReject == false) {
			MsgSoundPlayStart(false);

			int smsCnt = 0;
			int mmsCnt = 0;

			smsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_SMS_TYPE);
			mmsCnt = MsgStoGetUnreadCnt(&dbHandle, MSG_MMS_TYPE);

			MsgSettingHandleNewMsg(smsCnt, mmsCnt);
			MsgInsertNoti(&dbHandle, pMsgInfo);
		}

		request.msgInfo.msgId = pMsgInfo->msgId;

		MSG_DEBUG("-=====================[[[ %s ]]]] =========================", pMsgInfo->msgData);
		err = plg->submitReq(&request);

		if (err == MSG_SUCCESS) {
			MSG_DEBUG("Process Message Success : processReceivedInd()");
		} else {
			MSG_DEBUG("Process Message Fail : processReceivedInd()");
		}
	}

	return err;
}

