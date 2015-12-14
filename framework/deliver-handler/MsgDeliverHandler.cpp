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

#ifndef MSG_WEARABLE_PROFILE
#include <app_control.h>
#endif // MSG_WEARABLE_PROFILE

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
#include "MsgDevicedWrapper.h"

/*==================================================================================================
                                     VARIABLES
==================================================================================================*/

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

void MsgPlayTTSMode(MSG_SUB_TYPE_T msgSubType, msg_message_id_t msgId, bool isFavorites)
{
	MSG_BEGIN();
#if 0
	bool bNotification = true;

	if (MsgSettingGetBool(MSG_SETTING_NOTIFICATION, &bNotification) != MSG_SUCCESS) {
		MSG_DEBUG("MsgSettingGetBool is failed.");
	}

	if (bNotification == false) {
		MSG_DEBUG("Msg Alert notification is off.");
		return;
	}

	bool isTTSOn = false;
	MsgSettingGetBool(VCONFKEY_SETAPPL_DRIVINGMODE_DRIVINGMODE, &isTTSOn);
	MSG_DEBUG("VCONFKEY_SETAPPL_DRIVINGMODE_DRIVINGMODE [%d]", isTTSOn);

	if(isTTSOn) {
		bool isVoiceMail = false;

		if (msgSubType == MSG_MWI_VOICE_SMS) {
			bool isVoiceMailOn = false;
			MsgSettingGetBool(VCONFKEY_SETAPPL_DRIVINGMODE_NEWVOICEMAILS, &isVoiceMailOn);
			MSG_DEBUG("VCONFKEY_SETAPPL_DRIVINGMODE_NEWVOICEMAILS [%d]", isVoiceMailOn);
			if (isVoiceMailOn) {
				isVoiceMail = true;
			} else {
				return;
			}
		} else {
			bool isTTSMsgOn = false;
			MsgSettingGetBool(VCONFKEY_SETAPPL_DRIVINGMODE_MESSAGE, &isTTSMsgOn);
			MSG_DEBUG("VCONFKEY_SETAPPL_DRIVINGMODE_MESSAGE [%d]", isTTSMsgOn);
			if (!isTTSMsgOn || !isFavorites) {
				return;
			}
		}

#ifndef MSG_WEARABLE_PROFILE
		app_control_h svc_h;

		int ret = APP_CONTROL_ERROR_NONE;

		ret = app_control_create(&svc_h);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_DEBUG("app_control_create() is failed : %d", ret);
			app_control_destroy(svc_h);
			return;
		}

		ret = app_control_set_app_id(svc_h, "org.tizen.msg-ui-tts-play");
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_DEBUG("app_control_set_app_id() is failed : %d", ret);
			app_control_destroy(svc_h);
			return;
		}

		if (isVoiceMail) {
			ret = app_control_add_extra_data(svc_h, "type", "voicemail");
		} else {
			char tmpStrMsgId[10];
			memset(&tmpStrMsgId, 0x00, sizeof(tmpStrMsgId));
			snprintf(tmpStrMsgId, sizeof(tmpStrMsgId), "%d", msgId);
			MSG_DEBUG("tmpStrMsgId [%s]", tmpStrMsgId);
			ret = app_control_add_extra_data(svc_h, "msgId", tmpStrMsgId);
		}

		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
			app_control_destroy(svc_h);
			return;
		}

		ret = app_control_send_launch_request(svc_h, NULL, NULL);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_DEBUG("app_control_send_launch_request() is failed : %d", ret);
			app_control_destroy(svc_h);
			return;
		}

		app_control_destroy(svc_h);
#endif // MSG_WEARABLE_PROFILE
	}

	MsgChangePmState();
#endif
	MSG_END();
}


void MsgLaunchClass0(msg_message_id_t msgId)
{
	MSG_BEGIN();
#ifndef MSG_WEARABLE_PROFILE
	app_control_h svc_h;

	int ret = APP_CONTROL_ERROR_NONE;

	ret = app_control_create(&svc_h);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_DEBUG("app_control_create() is failed : %d", ret);
		app_control_destroy(svc_h);
		return;
	}

	ret = app_control_set_app_id(svc_h, "org.tizen.msg-ui-class0");
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_DEBUG("app_control_set_app_id() is failed : %d", ret);
		app_control_destroy(svc_h);
		return;
	}

	ret = app_control_add_extra_data(svc_h, "type", "msg_id");
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
		app_control_destroy(svc_h);
		return;
	}

	char tmpStrMsgId[10];
	memset(&tmpStrMsgId, 0x00, sizeof(tmpStrMsgId));
	snprintf(tmpStrMsgId, sizeof(tmpStrMsgId), "%d", msgId);
	MSG_DEBUG("tmpStrMsgId [%s]", tmpStrMsgId);
	ret = app_control_add_extra_data(svc_h, "msgId", tmpStrMsgId);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
		app_control_destroy(svc_h);
		return;
	}

	ret = app_control_send_launch_request(svc_h, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_DEBUG("app_control_send_launch_request() is failed : %d", ret);
		app_control_destroy(svc_h);
		return;
	}

	app_control_destroy(svc_h);
#endif // MSG_WEARABLE_PROFILE
	MSG_END();
}


msg_error_t MsgHandleMmsConfIncomingMsg(MSG_MESSAGE_INFO_S *pMsgInfo, msg_request_id_t reqID)
{
	MSG_BEGIN();

	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	MSG_DEBUG(" msgtype subtype is [%d]", pMsgInfo->msgType.subType);

	if (pMsgInfo->msgType.subType == MSG_RETRIEVE_AUTOCONF_MMS || pMsgInfo->msgType.subType == MSG_RETRIEVE_MANUALCONF_MMS) {
		// keep original subType
		MSG_SUB_TYPE_T subType = pMsgInfo->msgType.subType;

		/* If Retrieve Failed, Msg SubType is remained as Notification Ind */
		if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_FAIL || pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_PENDING) {
			pMsgInfo->folderId = MSG_INBOX_ID;
			pMsgInfo->msgType.subType = MSG_NOTIFICATIONIND_MMS;
		}

//		err = MsgStoUpdateMMSMessage(pMsgInfo);
//
//		if (err == MSG_SUCCESS)
//			MSG_DEBUG("Command Handle Success : MsgStoUpdateMessage()");

		if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
			MSG_DEBUG("### MSG_NETWORK_RETRIEVE_SUCCESS ###");
			//Update Mms Message to MMS Plugin DB

			// MMS Received Ind Process Func
			MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsgInfo->msgType.mainType);
			if (plg == NULL)
				return MSG_ERR_NULL_POINTER;

			//Contents of msg Data was removed and replaced to retrievedFilePath
			// NOTICE:: now it was moved to handleEvent in MsgListnerThread
			err = plg->updateMessage(pMsgInfo, NULL, NULL);
			if (err != MSG_SUCCESS)
				return MSG_ERR_STORAGE_ERROR;
		}

		err = MsgStoUpdateMMSMessage(pMsgInfo);//For Text Update

		if (err == MSG_SUCCESS)
			MSG_DEBUG("Command Handle Success : MsgStoUpdateMessage()");


		MSG_DEBUG(" ######################### MESSAGE UPDATE COMPLETED!!! ######################");

		bool readStatus = false;

		MsgStoGetReadStatus(pMsgInfo->msgId, &readStatus);
		MSG_DEBUG("### readStatus = %d ###", readStatus);

		//Update Read Status to Unread because Noti is Read
		if (subType == MSG_RETRIEVE_MANUALCONF_MMS && pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
			MSG_DEBUG("###2. subType = %d ###", pMsgInfo->msgType.subType);

			if (readStatus == true) {
				MsgStoSetReadStatus(dbHandle, pMsgInfo->msgId, false);
				MsgRefreshNotification(MSG_NOTI_TYPE_NORMAL, false, MSG_ACTIVE_NOTI_TYPE_NONE);
			}
		}

		if (subType == MSG_RETRIEVE_AUTOCONF_MMS && pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			bool isFavorites = false;
			if (!checkBlockingMode(pMsgInfo->addressList[0].addressVal, &isFavorites)) {
				MsgPlayTTSMode(subType, pMsgInfo->msgId, isFavorites);
			}

			// add phone log
			MSG_DEBUG("Enter MsgAddPhoneLog() for mms message.");
			MsgAddPhoneLog(pMsgInfo);
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED

			if (pMsgInfo->folderId == MSG_INBOX_ID)
				MsgInsertNotification(pMsgInfo);

		} else if (subType == MSG_RETRIEVE_MANUALCONF_MMS) {
			if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_SUCCESS) {
				MSG_DEBUG("Manual success");
#if 0 //disable as per UX request to not show success notification : 2015.9.18
				if (pMsgInfo->folderId == MSG_INBOX_ID)
					MsgInsertTicker("Message Retrieved", MESSAGE_RETRIEVED, true, 0);
#endif
			} else if (pMsgInfo->networkStatus == MSG_NETWORK_RETRIEVE_FAIL) {
				MSG_DEBUG("Manual failed");
				MsgInsertTicker("Retrieving message failed", RETRIEVING_MESSAGE_FAILED, true, pMsgInfo->msgId);
			}
		}
	} else if (pMsgInfo->msgType.subType == MSG_SENDREQ_MMS || pMsgInfo->msgType.subType == MSG_SENDCONF_MMS) {
		MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(pMsgInfo->msgType.mainType);
		if (plg == NULL)
			return MSG_ERR_NULL_POINTER;

		if (pMsgInfo->networkStatus == MSG_NETWORK_SEND_PENDING)
			pMsgInfo->msgType.subType = MSG_SENDREQ_MMS;
		else
			pMsgInfo->msgType.subType = MSG_SENDCONF_MMS; // change subType for storage update

		err = MsgStoUpdateMMSMessage(pMsgInfo);
		if (err == MSG_SUCCESS)
			MSG_DEBUG("Command Handle Success : MsgStoUpdateMMSMessage()");

		err = plg->updateMessage(pMsgInfo, NULL, NULL);
		if (err != MSG_SUCCESS)
			return MSG_ERR_STORAGE_ERROR;

		MSG_DEBUG("pMsg->networkStatus : %d", pMsgInfo->networkStatus);
		err = MsgStoUpdateNetworkStatus(dbHandle, pMsgInfo, pMsgInfo->networkStatus);
		if (err != MSG_SUCCESS)
			return MSG_ERR_STORAGE_ERROR;

		if (pMsgInfo->networkStatus == MSG_NETWORK_SEND_SUCCESS) {
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

	MsgDisplayLock();

	if (pMsgInfo->msgType.mainType == MSG_SMS_TYPE) {
		bool bOnlyNoti = false;

		err = MsgHandleSMS(pMsgInfo, pSendNoti, &bOnlyNoti);

		if (err == MSG_SUCCESS && ((*pSendNoti) || bOnlyNoti)) {
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			bool isFavorites = false;
			if (!checkBlockingMode(pMsgInfo->addressList[0].addressVal, &isFavorites)) {
				MsgPlayTTSMode(pMsgInfo->msgType.subType, pMsgInfo->msgId, isFavorites);
			}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
			MsgInsertNotification(pMsgInfo);
		}
	} else if (pMsgInfo->msgType.mainType == MSG_MMS_TYPE) {
		err = MsgHandleMMS(pMsgInfo, pSendNoti);
	}

	MsgDisplayUnlock();

	//Add Phone Log Data
	if ((err == MSG_SUCCESS) &&
		(pMsgInfo->folderId == MSG_INBOX_ID || pMsgInfo->folderId == MSG_SPAMBOX_ID) &&
		(pMsgInfo->msgType.mainType == MSG_SMS_TYPE) &&
		(pMsgInfo->msgType.subType != MSG_WAP_SL_SMS)) {
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
		MSG_DEBUG("Enter MsgAddPhoneLog() : pMsg->folderId [%d]", pMsgInfo->folderId);
		MsgAddPhoneLog(pMsgInfo);
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED
	}

	// Auto delete
	//MsgStoAutoDeleteConversation(pMsgInfo->threadId);

	MSG_END();

	return err;
}


msg_error_t MsgHandleSMS(MSG_MESSAGE_INFO_S *pMsgInfo, bool *pSendNoti, bool *bOnlyNoti)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();

	if (pMsgInfo->msgPort.valid == true) {
		*pSendNoti = false;
		*bOnlyNoti = false;
		return MSG_SUCCESS;
	}

	// Add SMS message
	MSG_SENDINGOPT_INFO_S send_opt;
	memset(&send_opt, 0x00, sizeof(MSG_SENDINGOPT_INFO_S));

	if (pMsgInfo->msgType.classType == MSG_CLASS_2) {
#ifdef MSG_NOTI_INTEGRATION
		MSG_DEBUG("Copy Class2 message to phone storage.");
		pMsgInfo->msgType.classType = MSG_CLASS_NONE;
		pMsgInfo->storageId = MSG_STORAGE_PHONE;
		pMsgInfo->msgId = 0;
#else
		MSG_DEBUG("Class2 message is already added.");
		return MSG_SUCCESS;
#endif
	}

	if (pMsgInfo->msgType.subType == MSG_NORMAL_SMS) {
		MSG_DEBUG("Add Normal Message");
		err = MsgStoAddMessage(pMsgInfo, &send_opt);
	} else if ((pMsgInfo->msgType.subType >= MSG_REPLACE_TYPE1_SMS) && (pMsgInfo->msgType.subType <= MSG_REPLACE_TYPE7_SMS)) {
		MSG_DEBUG("Add Replace Message");

		pMsgInfo->msgId = 0;
		MsgStoGetReplaceMsgId(pMsgInfo);

		if (pMsgInfo->msgId > 0) {
			err = MsgStoUpdateMessage(pMsgInfo, &send_opt);
		} else {
			err = MsgStoAddMessage(pMsgInfo, &send_opt);
			pMsgInfo->msgType.subType = MSG_NORMAL_SMS; // To make 'insert' type for storage change callback.
		}
	} else if ((pMsgInfo->msgType.subType == MSG_WAP_SI_SMS) || (pMsgInfo->msgType.subType == MSG_WAP_CO_SMS) || (pMsgInfo->msgType.subType == MSG_WAP_SL_SMS)) {
		MSG_DEBUG("Add WAP Push Message");
		switch (pMsgInfo->msgType.subType) {
			case MSG_WAP_SI_SMS:
			case MSG_WAP_SL_SMS:
				err = MsgStoAddWAPMsg(pMsgInfo);
				break;
			case MSG_WAP_CO_SMS:
				err = MsgStoAddCOWAPMsg(pMsgInfo);
				break;
			default :
				break;
		}
	} else if ((pMsgInfo->msgType.subType == MSG_CB_SMS) || (pMsgInfo->msgType.subType == MSG_JAVACB_SMS)) {
		/** check add message option */
	} else if (pMsgInfo->msgType.subType == MSG_STATUS_REPORT_SMS) {
		MSG_DEBUG("Add Report Message");
	} else if (pMsgInfo->msgType.subType >= MSG_MWI_VOICE_SMS && pMsgInfo->msgType.subType <= MSG_MWI_OTHER_SMS) {
		char keyName[MAX_VCONFKEY_NAME_LEN];
		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_NUMBER, pMsgInfo->sim_idx);
		char *voiceNumber = MsgSettingGetString(keyName);

		if (voiceNumber) {
			MSG_SEC_DEBUG("Voice Mail Number [%s]", voiceNumber);

			memset(pMsgInfo->addressList[0].addressVal, 0x00, sizeof(pMsgInfo->addressList[0].addressVal));
			snprintf(pMsgInfo->addressList[0].addressVal, sizeof(pMsgInfo->addressList[0].addressVal), "%s", voiceNumber);
			memset(keyName, 0x00, sizeof(keyName));
			snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_ALPHA_ID, pMsgInfo->sim_idx);
			char *alphaId = MsgSettingGetString(keyName);
			if (alphaId) {
				memset(pMsgInfo->addressList->displayName, 0x00, sizeof(pMsgInfo->addressList->displayName));
				memcpy(pMsgInfo->addressList->displayName, alphaId, sizeof(pMsgInfo->addressList->displayName)-1);
				g_free(alphaId);
				alphaId = NULL;
			}

			free(voiceNumber);
			voiceNumber = NULL;
		}

		memset(keyName, 0x00, sizeof(keyName));
		snprintf(keyName, sizeof(keyName), "%s/%d", VOICEMAIL_COUNT, pMsgInfo->sim_idx);
		int voicecnt = MsgSettingGetInt(keyName);
		memset(pMsgInfo->msgText, 0x00, sizeof(pMsgInfo->msgText));
		snprintf(pMsgInfo->msgText, sizeof(pMsgInfo->msgText), "%d", voicecnt);
		pMsgInfo->dataSize = strlen(pMsgInfo->msgText);
		MSG_DEBUG("Add Voice or other Message");
	}
	else {
		MSG_DEBUG("No matching type [%d]", pMsgInfo->msgType.subType);
		return err;
	}

	if (err != MSG_SUCCESS)
		MSG_DEBUG("Add message - Error : [%d]", err);

	if (pMsgInfo->msgType.subType == MSG_NORMAL_SMS) {
		if (MsgCheckFilter(dbHandle, pMsgInfo) == true) {
			// Move to SpamBox
			err = MsgStoMoveMessageToFolder(pMsgInfo->msgId, MSG_SPAMBOX_ID);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoMoveMessageToFolder() Error : [%d]", err);
			} else {
				pMsgInfo->folderId = MSG_SPAMBOX_ID;
			}

			*pSendNoti = false;
			*bOnlyNoti = false;
		} else if (pMsgInfo->msgType.classType == MSG_CLASS_0) {
			MsgLaunchClass0(pMsgInfo->msgId);
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			bool isFavorites = false;
			if (!checkBlockingMode(pMsgInfo->addressList[0].addressVal, &isFavorites)) {
				MsgPlayTTSMode(pMsgInfo->msgType.subType, pMsgInfo->msgId, isFavorites);
				MsgSoundPlayer::instance()->MsgSoundPlayStart(&(pMsgInfo->addressList[0]), MSG_SOUND_PLAY_USER);
			}
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			*pSendNoti = false;
			*bOnlyNoti = false;

			MsgInsertOnlyActiveNotification(MSG_NOTI_TYPE_CLASS0, pMsgInfo);
		}
	} else if ((pMsgInfo->msgType.subType >= MSG_WAP_SI_SMS) && (pMsgInfo->msgType.subType <= MSG_WAP_CO_SMS)) {
		MSG_DEBUG("Starting WAP Message Incoming.");

#ifndef MSG_WEARABLE_PROFILE
		MSG_PUSH_SERVICE_TYPE_T serviceType = (MSG_PUSH_SERVICE_TYPE_T)MsgSettingGetInt(PUSH_SERVICE_TYPE);
		app_control_h svc_handle = NULL;

		switch (pMsgInfo->msgType.subType) {
			case MSG_WAP_SL_SMS: {
				*pSendNoti = true;

				if (serviceType == MSG_PUSH_SERVICE_ALWAYS) {
					if (app_control_create(&svc_handle) < 0) {
						MSG_DEBUG("Fail to create service handle");
						break;
					}
					if (!svc_handle) {
						MSG_DEBUG("Service handle is NULL");
						break;
					}
					if (app_control_set_operation(svc_handle, APP_CONTROL_OPERATION_VIEW) < 0) {
						MSG_DEBUG("Fail to create service handle");
						app_control_destroy(svc_handle);
						break;
					}
					if (app_control_set_uri(svc_handle, pMsgInfo->msgText) < 0) {
						MSG_DEBUG("Fail to set uri");
						app_control_destroy(svc_handle);
						break;
					}
					if (app_control_set_app_id(svc_handle, MSG_SVC_PKG_NAME_BROWSER) < 0) {
						MSG_DEBUG("Fail to set package");
						app_control_destroy(svc_handle);
						break;
					}
					if (app_control_send_launch_request(svc_handle, NULL, NULL) < 0) {
						MSG_DEBUG("Fail to launch browser");
						app_control_destroy(svc_handle);
						break;
					}

					app_control_destroy(svc_handle);

				} else if (serviceType == MSG_PUSH_SERVICE_PROMPT) {
					MSG_DEBUG("WAP Message SL(Always Ask) start.");

					app_control_h svc_h;
					int ret = APP_CONTROL_ERROR_NONE;

					ret = app_control_create(&svc_h);
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_create() is failed : %d", ret);
						break;
					}

					ret = app_control_set_app_id(svc_h, "org.tizen.message-dialog");
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_set_app_id() is failed : %d", ret);
						app_control_destroy(svc_h);
						break;
					}

					ret = app_control_add_extra_data(svc_h, "mode", "WAP_PUSH_SL");
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
						app_control_destroy(svc_h);
						break;
					}

					ret = app_control_add_extra_data(svc_h, "url", pMsgInfo->msgText);
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
						app_control_destroy(svc_h);
						break;
					}

					ret = app_control_add_extra_data(svc_h, "address", pMsgInfo->addressList[0].addressVal);
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_add_extra_data() is failed : %d", ret);
						app_control_destroy(svc_h);
						break;
					}

					ret = app_control_send_launch_request(svc_h, NULL, NULL);
					if (ret != APP_CONTROL_ERROR_NONE) {
						MSG_DEBUG("app_control_send_launch_request() is failed : %d", ret);
					}

					app_control_destroy(svc_h);
					MSG_DEBUG("app_control_destroy() returns : %d", ret);
				}
			}
			break;

			case MSG_WAP_SI_SMS:
				*pSendNoti = true;
				break;
			case MSG_WAP_CO_SMS:
				*pSendNoti = false;
				MsgInsertOnlyActiveNotification(MSG_NOTI_TYPE_NORMAL, pMsgInfo);
				break;
		}
#endif // MSG_WEARABLE_PROFILE
	} else if (pMsgInfo->msgType.subType == MSG_STATUS_REPORT_SMS) {
		*pSendNoti = false;
		*bOnlyNoti = true;
	} else if (pMsgInfo->msgType.subType >= MSG_MWI_VOICE_SMS && pMsgInfo->msgType.subType <= MSG_MWI_OTHER_SMS) {
		if (pMsgInfo->bStore == false) {
			*pSendNoti = false;
			*bOnlyNoti = true;
		}
	}

	// Update Conversation table
	err = MsgStoUpdateConversation(dbHandle, pMsgInfo->threadId);

	if (err != MSG_SUCCESS)
		MSG_DEBUG("MsgStoUpdateConversation() Error : [%d]", err);

	return err;
}


msg_error_t MsgHandleMMS(MSG_MESSAGE_INFO_S *pMsgInfo,  bool *pSendNoti)
{
	msg_error_t err = MSG_SUCCESS;
	MsgDbHandler *dbHandle = getDbHandle();
	MSG_REQUEST_INFO_S request = {0};
	bool bReject = false;
	bool bFiltered = false;

	// MMS Received Ind Process Func
	MSG_MAIN_TYPE_T msgMainType = pMsgInfo->msgType.mainType;
	MsgPlugin *plg = MsgPluginManager::instance()->getPlugin(msgMainType);

	if (plg == NULL)
		return MSG_ERR_NULL_POINTER;

	// Read the default network SIM
	MsgPlugin *sms_plg = MsgPluginManager::instance()->getPlugin(MSG_SMS_TYPE);

	if (sms_plg == NULL)
		return MSG_ERR_NULL_POINTER;

	int defaultNetworkSimId = 0;

	err = sms_plg->getDefaultNetworkSimId(&defaultNetworkSimId);

	MSG_DEBUG("######defaultNetworkSimId = %d, err = %d", defaultNetworkSimId, err);

	if (err != MSG_SUCCESS) {
		MSG_ERR("getDefaultNetworkSimId is failed=[%d]", err);
		return err;
	}

	err = MsgSettingSetInt(MSG_NETWORK_SIM, (int)defaultNetworkSimId);
	if (err != MSG_SUCCESS) {
		MSG_ERR("Error to set config data [%s], err = %d", MSG_NETWORK_SIM, err);
		return err;
	}

	// Need to process m-delivery-ind, m-notification-ind, m-read-orig-ind
	err = plg->processReceivedInd(pMsgInfo, &request, &bReject);

	if (err == MSG_SUCCESS) {
		MSG_DEBUG("Process Message Success : processReceivedInd(), btextsms %d", pMsgInfo->bTextSms);
	} else {
		MSG_DEBUG("Process Message Fail : processReceivedInd()");
		*pSendNoti = false;
		return err;
	}

	// Add into DB
	if ((pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) && bReject == false) {
		/* It should send noti response even if MMS is blocked by sender address. */
		//bFiltered = MsgCheckFilter(dbHandle, pMsgInfo);
		if (pMsgInfo->folderId == MSG_SPAMBOX_ID) {
			bFiltered = true;
		}

		if (bFiltered == true) {
			err = MsgStoMoveMessageToFolder(pMsgInfo->msgId, MSG_SPAMBOX_ID);

			if (err != MSG_SUCCESS) {
				MSG_DEBUG("MsgStoMoveMessageToFolder() Error : [%d]", err);
			} else {
				pMsgInfo->folderId = MSG_SPAMBOX_ID;
			}

			pMsgInfo->networkStatus = MSG_NETWORK_RETRIEVE_FAIL;
			*pSendNoti = false;
		} else {/* If it is not SPAM message, it should be check whether filter option is enable or not. */
			bool filterFlag = false;
			MsgGetFilterOperation(&filterFlag);
			if (filterFlag == true && request.msgInfo.msgType.subType == MSG_GET_MMS) {
				 /* Not to show MMS notification msg until it retrieves. */
				pMsgInfo->folderId = MSG_IOSBOX_ID;
				*pSendNoti = false;
			}
		}

		err = MsgStoAddMessage(pMsgInfo, NULL);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("MsgStoAddMessage() Error: [%d]", err);
			return err;
		}
	} else if (pMsgInfo->msgType.subType == MSG_READORGIND_MMS || pMsgInfo->msgType.subType == MSG_DELIVERYIND_MMS) {
		if (MsgInsertNotification(pMsgInfo) == MSG_SUCCESS) {
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			bool isFavorites = false;
			if (!checkBlockingMode(pMsgInfo->addressList[0].addressVal, &isFavorites)) {
				MsgPlayTTSMode(pMsgInfo->msgType.subType, pMsgInfo->msgId, isFavorites);
			}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */
			*pSendNoti = false;
		}
	}

#if 0
	if (pMsgInfo->bTextSms == false) {
		MsgDeleteFile(pMsgInfo->msgData); //ipc
		memset(pMsgInfo->msgData, 0x00, sizeof(pMsgInfo->msgData));
	}
#endif
	//In the case of m-notification-ind, we should decide whether to send m-notify-response-ind or http 'Get'
	//submit request
	if (pMsgInfo->msgType.subType == MSG_NOTIFICATIONIND_MMS) {
		if (request.msgInfo.msgType.subType == MSG_NOTIFYRESPIND_MMS && bReject == false && bFiltered == false) {
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
			bool isFavorites = false;
			if (!checkBlockingMode(pMsgInfo->addressList[0].addressVal, &isFavorites)) {
				MsgPlayTTSMode(pMsgInfo->msgType.subType, pMsgInfo->msgId, isFavorites);
			}
#endif /* MSG_CONTACTS_SERVICE_NOT_SUPPORTED */

			MsgInsertNotification(pMsgInfo);
		} else {
			*pSendNoti = false;
		}

		MSG_DEBUG("default_sim = %d, pMsgInfo->sim_idx = %d", defaultNetworkSimId, pMsgInfo->sim_idx);

		if (defaultNetworkSimId == pMsgInfo->sim_idx) {
			request.msgInfo.msgId = pMsgInfo->msgId;

			MSG_SEC_DEBUG("-=====================[[[ %s ]]]] =========================", request.msgInfo.msgData);
			err = plg->submitReq(&request);

			if (err == MSG_SUCCESS) {
				MSG_DEBUG("Process Message Success : processReceivedInd()");

				if (request.msgInfo.msgType.subType == MSG_GET_MMS) {
					MSG_DEBUG("Auto Retrieve Mode : update network status retrieving");
					MsgStoUpdateNetworkStatus(dbHandle, &(request.msgInfo), MSG_NETWORK_RETRIEVING);
				}

			} else {
				MSG_DEBUG("Process Message Fail : processReceivedInd()");
			}
		}
	}

	return err;
}
