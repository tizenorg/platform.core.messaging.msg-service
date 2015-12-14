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

 /**
 *	@file 		MsgPluginInterface.h
 *	@brief 		Defines plug-ins API of messaging framework
 *	@version 	1.0
 */

#ifndef MSG_PLUGIN_INTERFACE_H
#define MSG_PLUGIN_INTERFACE_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Plug-in API
 *	@section		Program
 *	- Program : Messaging Plug-in API Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTransportTypes.h"
#include "MsgStorageTypes.h"
#include "MsgSettingTypes.h"
#include "MsgInternalTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**
 *	@ingroup	MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_PLUGIN_API	Messaging Plug-in API
 *	@{
 */

/*==================================================================================================
                                         TYPE DEFINES
==================================================================================================*/
typedef struct _MSG_PLUGIN_HANDLER_S MSG_PLUGIN_HANDLER_S;

typedef struct _MSG_PLUGIN_LISTENER_S MSG_PLUGIN_LISTENER_S;


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

 /**

 * \par Description:
 * A Prototype of the function which will be called when the plug-in is loaded at Message Framework.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Create plug-in handle for Message Framework.
 * Message Framework will able to reach plugin functions by handle.
 *
 * \par Typical use case:
 * Create plug-in handle.
 *
 * \par Method of function operation:
 * Set handle information by plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param input - handle is Message handle.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_NULL_POINTER		Plug-in handle is invalid.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
msg_error_t MsgPlgCreateHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle);


 /**

 * \par Description:
 * A Prototype of the function which will be called when the plug-in is finalized at Message Framework.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Destroy plug-in handle for Message Framework.
 *
 * \par Typical use case:
 * Destroy plug-in handle.
 *
 * \par Method of function operation:
 * Remove handle information by plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param input - handle is Message handle.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_NULL_POINTER		Plug-in handle is invalid.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
msg_error_t MsgPlgDestroyHandle(MSG_PLUGIN_HANDLER_S *pPluginHandle);




/* Control API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when the plug-in is loaded at Message Framework.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Initializing plug-in.
 * Precede jobs must done in this function.
 *
 * \par Typical use case:
 * Initializing plug-in.
 *
 * \par Method of function operation:
 * Make up pre-works for plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_REGEVENT	Plug-in's error during operations.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgInitialize)();


 /**

 * \par Description:
 * A Prototype of the function which will be called when the plug-in is loaded at Message Framework.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Create plug-in handle for Message Framework.
 * Message Framework will able to reach plugin functions by handle.
 *
 * \par Typical use case:
 * Create plug-in handle.
 *
 * \par Method of function operation:
 * Set handle information by plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param input - handle is Message handle.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_NULL_POINTER		Plug-in handle is invalid.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgFinalize)();


 /**

 * \par Description:
 * A Prototype of the function which will be called when the plug-in is loaded at Message Framework.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Regist listeners to handle incomming messages..
 * .
 *
 * \par Typical use case:
 * Regist listeners.
 *
 * \par Method of function operation:
 * Set listener informations in plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param input - listener is callback listeners.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgRegisterListener)(MSG_PLUGIN_LISTENER_S *pListener);


 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to check sim card status.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Checking sim card status  for Message Framework.
 *
 * \par Typical use case:
 * To check sim card status.
 *
 * \par Method of function operation:
 * Check SIM card status and return result of sim card is changed or not.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param output - bChanged is a boolean type value which shows sim card status.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgCheckSimStatus)(MSG_SIM_STATUS_T *pStatus);


 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to check device status.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Create plug-in handle for Message Framework.
 * Message Framework will able to reach plugin functions by handle.
 *
 * \par Typical use case:
 * Create plug-in handle.
 *
 * \par Method of function operation:
 * Set handle information by plug-in.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins..
 *
 * \param input - handle is Message handle.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_NULL_POINTER		Plug-in handle is invalid.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgCheckDeviceStatus)();


/* Transport API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to submit a message to send.
 * ALL Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Send message..
 *
 * \par Typical use case:
 * To send messages.
 *
 * \par Method of function operation:
 * Convert message data to raw message data.
 * Send raw message data to selected network.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pReqInfo is message data to send.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_TRANSPORT	Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgSubmitRequest)(MSG_REQUEST_INFO_S *pReqInfo);


/* Storage API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to save a message to SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Save a message to SIM card.
 *
 * \par Typical use case:
 * To save a message to SIM card..
 *
 * \par Method of function operation:
 * Convert message data to raw message data.
 * Save the raw message data to SIM card.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pMsgInfo is the information of message.
 * \param output - pSimIdList is the list of messages saved in SIM card.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS						Success in operation.
 * - MSG_ERR_PLUGIN_SIM_MSG_FULL	SIM card's capacity for SMS message is full.
 * - MSG_ERR_PLUGIN_STORAGE		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgInitSimMessage)();


/* Storage API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to save a message to SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Save a message to SIM card.
 *
 * \par Typical use case:
 * To save a message to SIM card..
 *
 * \par Method of function operation:
 * Convert message data to raw message data.
 * Save the raw message data to SIM card.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pMsgInfo is the information of message.
 * \param output - pSimIdList is the list of messages saved in SIM card.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS						Success in operation.
 * - MSG_ERR_PLUGIN_SIM_MSG_FULL	SIM card's capacity for SMS message is full.
 * - MSG_ERR_PLUGIN_STORAGE		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgSaveSimMessage)(const MSG_MESSAGE_INFO_S *pMsgInfo, SMS_SIM_ID_LIST_S *pSimIdList);


 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to delete a message in SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * delete a message in SIM card.
 *
 * \par Typical use case:
 * To save a message to SIM card..
 *
 * \par Method of function operation:
 * delete a message data in SIM card which is indexed by ID.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - SimMsgId is the index of the message to delete.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS						Success in operation.
 * - MSG_ERR_PLUGIN_STORAGE		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
#ifdef FEATURE_SMS_CDMA
typedef msg_error_t (*MsgPlgDeleteSimMessage)(msg_sim_id_t SimMsgId);
#else
typedef msg_error_t (*MsgPlgDeleteSimMessage)(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId);
#endif


 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to set read status of  message in SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Set read status of SIM cad message.
 *
 * \par Typical use case:
 * To set read status of SIM card message.
 *
 * \par Method of function operation:
 * Save the given read status to SIM card message.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - SimMsgId is the index of the message to set read status.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS						Success in operation.
 * - MSG_ERR_PLUGIN_STORAGE		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
#ifdef FEATURE_SMS_CDMA
typedef msg_error_t (*MsgPlgSetReadStatus)(msg_sim_id_t SimMsgId);
#else
typedef msg_error_t (*MsgPlgSetReadStatus)(msg_sim_slot_id_t sim_idx, msg_sim_id_t SimMsgId);
#endif

 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to set read status of  message in SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Set read status of SIM cad message.
 *
 * \par Typical use case:
 * To set read status of SIM card message.
 *
 * \par Method of function operation:
 * Save the given read status to SIM card message.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - SimMsgId is the index of the message to set read status.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS						Success in operation.
 * - MSG_ERR_PLUGIN_STORAGE		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
#ifdef FEATURE_SMS_CDMA
typedef msg_error_t (*MsgPlgSetMemoryStatus)(msg_error_t Error);
#else
typedef msg_error_t (*MsgPlgSetMemoryStatus)(msg_sim_slot_id_t sim_idx, msg_error_t Error);
#endif

/* Setting API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to save configuration of network to SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Save configuration information to SIM card.
 *
 * \par Typical use case:
 * To save configuration information to SIM card.
 *
 * \par Method of function operation:
 * Convert information data to raw data.
 * Save raw data to SIM card.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pSetting is information of configuration.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_SETTING		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgInitConfigData)(MSG_SIM_STATUS_T SimStatus);


/* Setting API */
 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to save configuration of network to SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Save configuration information to SIM card.
 *
 * \par Typical use case:
 * To save configuration information to SIM card.
 *
 * \par Method of function operation:
 * Convert information data to raw data.
 * Save raw data to SIM card.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pSetting is information of configuration.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_SETTING		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgSetConfigData)(const MSG_SETTING_S *pSetting);


 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to get configuration of network from SIM card.
 * SMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Get configuration information from SIM card.
 *
 * \par Typical use case:
 * To get configuration information from SIM card.
 *
 * \par Method of function operation:
 * Convert raw data to information data.
 * Get configuration data from SIM card.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pSetting is information of configuration.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_SETTING		Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgGetConfigData)(MSG_SETTING_S *pSetting);

/* MMS API */
/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to store MMS message.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Save MMS message to plug-in defined DB.
*
* \par Typical use case:
* To save MMS message to plug-in defined DB.
*
* \par Method of function operation:
* Convert MMS Message data to cetain format for plug-in.
* Save the certain format MMS message information to DB.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param input - pMsgInfo is information of MMS message.
* \param input - pSendOptInfo is information of sending option.
* \param output - pFileData is the file path of saved MMS message.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgAddMessage)(MSG_MESSAGE_INFO_S *pMsgInfo,  MSG_SENDINGOPT_INFO_S* pSendOptInfo, char* pFileData);


/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to process received MMS message indicator.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Process MMS message indicator.
*
* \par Typical use case:
* To process MMS message indicator.
*
* \par Method of function operation:
* Procces MMS Message indicator in options.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param input - pMsgInfo is information of MMS message.
* \param input - pRequest is information of request options.
* \param output - bRejects shows the reject status.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgProcessReceivedInd)(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_REQUEST_INFO_S* pRequest, bool* bReject);


/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to update MMS message.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Update MMS message to plug-in defined DB.
*
* \par Typical use case:
* To update MMS message to plug-in defined DB.
*
* \par Method of function operation:
* Convert MMS Message data to cetain format for plug-in.
* Update the certain format MMS message information to DB.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param input - pMsgInfo is information of MMS message.
* \param input - pSendOptInfo is information of sending option.
* \param output - pFileData is the file path of saved MMS message.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgUpdateMessage)(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S* pSendOptInfo, char* pFileData);


/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to get MMS message.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Get MMS message.
*
* \par Typical use case:
* To get MMS message.
*
* \par Method of function operation:
* Get MMS Message from certian plug-in defined DB.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param output - pMsg is information of MMS message.
* \param output - pSendOptInfo is information of sending options.
* \param output - pDestMsg is file path of MMS message.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgGetMmsMessage)(MSG_MESSAGE_INFO_S* pMsg,	MSG_SENDINGOPT_INFO_S* pSendOptInfo, char** pDestMsg);


/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to update reject status of MMS message.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Update reject status of MMS message.
*
* \par Typical use case:
* To update reject status of MMS message.
*
* \par Method of function operation:
* Update reject status of MMS Message from certian plug-in defined DB.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param input - pMsgInfo is information of MMS message.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgUpdateRejectStatus)(MSG_MESSAGE_INFO_S *pMsgInfo);


/**

* \par Description:
* A Prototype of the function which will be called when Message Framework has to compose read report for MMS message.
* MMS Plug-in SHOULD implement this function.
*
* \par Purpose:
* Compose read report for MMS message.
*
* \par Typical use case:
* To compose read report for MMS message.
*
* \par Method of function operation:
* Compose read report for MMS message.
* Send read report.
*
* \par Sync (or) Async:
* This is a Synchronous API.
*
* \par Important notes:
*  This function MUST be implement by plug-ins.
*
* \param input - pMsgInfo is information of MMS message.
*
* \return Return Type (int(msg_error_t)) \n
* - MSG_SUCCESS					  Success in operation.
* - MSG_ERR_PLUGIN_TRANSPORT   Exception is occured in plug-in.
*
* \par Prospective clients:
* Internal/Plug-ins.
*
* \par Related functions:
* None
*
* \par Known issues/bugs:
* None
*
*/
/*================================================================================================*/
typedef msg_error_t (*MsgPlgComposeReadReport)(MSG_MESSAGE_INFO_S *pMsgInfo);

 /**

 * \par Description:
 * A Prototype of the function which will be called when Message Framework has to compose read report for MMS message.
 * MMS Plug-in SHOULD implement this function.
 *
 * \par Purpose:
 * Compose read report for MMS message.
 *
 * \par Typical use case:
 * To compose read report for MMS message.
 *
 * \par Method of function operation:
 * Compose read report for MMS message.
 * Send read report.
 *
 * \par Sync (or) Async:
 * This is a Synchronous API.
 *
 * \par Important notes:
 *	This function MUST be implement by plug-ins.
 *
 * \param input - pMsgInfo is information of MMS message.
 *
 * \return Return Type (int(msg_error_t)) \n
 * - MSG_SUCCESS					Success in operation.
 * - MSG_ERR_PLUGIN_TRANSPORT	Exception is occured in plug-in.
 *
 * \par Prospective clients:
 * Internal/Plug-ins.
 *
 * \par Related functions:
 * None
 *
 * \par Known issues/bugs:
 * None
 *
 */
/*================================================================================================*/
typedef msg_error_t (*MsgPlgRestoreMsg)(MSG_MESSAGE_INFO_S *pMsg, char* pRcvdBody, int rcvdBodyLen, char* filePath);

typedef msg_error_t (*MsgPlgGetMeImei) (char *pImei);

typedef msg_error_t (*MsgPlgGetDefaultNetworkSimId) (int *simId);


/* framework defined callbacks. */
typedef void (*MsgPlgOnSentStatus)(MSG_SENT_STATUS_S *pSentStatus);
typedef void (*MsgPlgOnStorageChange)(msg_storage_change_type_t storageChangeType, msg_id_list_s *pMsgIdList);
typedef msg_error_t (*MsgPlgOnMsgIncoming)(MSG_MESSAGE_INFO_S *pMsgInfo);
typedef msg_error_t (*MsgPlgOnInitSimBySat)(void);
typedef msg_error_t (*MsgPlgOnSyncMLMsgIncoming)(MSG_SYNCML_MESSAGE_DATA_S *pSyncMLData);
typedef msg_error_t (*MsgPlgOnLBSMsgIncoming)(MSG_LBS_MESSAGE_DATA_S *pLBSData);
typedef msg_error_t (*MsgPlgOnPushMsgIncoming)(MSG_PUSH_MESSAGE_DATA_S *pPushData);
typedef msg_error_t (*MsgPlgOnCBMsgIncoming)(MSG_CB_MSG_S *pCbMsg, MSG_MESSAGE_INFO_S *pMsgInfo);
typedef msg_error_t (*MsgPlgOnMmsConfIncoming)(MSG_MESSAGE_INFO_S *pMsgInfo, msg_request_id_t *pRequest);
typedef msg_error_t (*MsgPlgOnSimMessageIncoming)(MSG_MESSAGE_INFO_S *pMsgInfo, int *simIdList, msg_message_id_t *retMsgId, int listSize);
typedef msg_error_t (*MsgPlgOnResendMessage)(void);
#ifdef FEATURE_SMS_CDMA
typedef bool (*MsgPlgCheckUniqueness)(MSG_UNIQUE_INDEX_S *p_msg, msg_message_id_t msgId, bool ischecked);
#endif
typedef msg_error_t (*MsgPlgOnInitImsi)(int sim_idx);

/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
struct _MSG_PLUGIN_LISTENER_S
{
	MsgPlgOnSentStatus					pfSentStatusCb;					/** The function pointer of sent status callback. */
	MsgPlgOnStorageChange			pfStorageChangeCb;			/** The function pointer of storage change callback. */
	MsgPlgOnMsgIncoming				pfMsgIncomingCb;				/** The function pointer of receive message callback. */
	MsgPlgOnInitSimBySat				pfInitSimBySatCb;				/** The function pointer of init SIM callback. */
	MsgPlgOnSyncMLMsgIncoming	pfSyncMLMsgIncomingCb;	/** The function pointer of receive syncML message callback. */
	MsgPlgOnLBSMsgIncoming			pfLBSMsgIncomingCb;			/** The function pointer of receive LBS message callback. */
	MsgPlgOnPushMsgIncoming		pfPushMsgIncomingCb;		/** The function pointer of receive Push message callback. */
	MsgPlgOnCBMsgIncoming			pfCBMsgIncomingCb;			/** The function pointer of receive cb message callback. */
	MsgPlgOnMmsConfIncoming 		pfMmsConfIncomingCb;		/** The function pointer of receive MMS conf */
	MsgPlgOnSimMessageIncoming	pfSimMsgIncomingCb;			/** The function pointer of sim message callback */
	MsgPlgOnResendMessage		pfResendMessageCb;
#ifdef FEATURE_SMS_CDMA
	MsgPlgCheckUniqueness		pfCheckUniquenessCb;
#endif
	MsgPlgOnInitImsi			pfSimInitImsiCb;
};


struct _MSG_PLUGIN_HANDLER_S
{
	MsgPlgInitialize    				pfInitialize;               		/**< The function pointer of initialize. */
	MsgPlgFinalize      				pfFinalize;                 		/**< The function pointer of finalize. */
	MsgPlgRegisterListener 			pfRegisterListener;      		/**< The function pointer of register listener. */
	MsgPlgSubmitRequest 			pfSubmitRequest;           	/**< The function pointer of submit request. */
	MsgPlgSaveSimMessage			pfSaveSimMessage;		/**< The function pointer of save SIM msg. */
	MsgPlgDeleteSimMessage		pfDeleteSimMessage;		/**< The function pointer of delete SIM msg. */
	MsgPlgSetReadStatus			pfSetReadStatus;			/**< The function pointer of set read status. */
	MsgPlgSetMemoryStatus		pfSetMemoryStatus;		/**< The function pointer of set memory status. */
	MsgPlgSetConfigData			pfSetConfigData;			/**< The function pointer of save setting. */
	MsgPlgGetConfigData			pfGetConfigData;			/**< The function pointer of get setting. */
	MsgPlgRestoreMsg				pfRestoreMsg;
	MsgPlgAddMessage				pfAddMessage;			/**< The function pointer of  add Message. */
	MsgPlgProcessReceivedInd		pfProcessReceivedInd;	/**< The function pointer of  Process Notification Ind. */
	MsgPlgUpdateMessage 			pfUpdateMessage;		/**< The function pointer of Update MMS Message */
	MsgPlgGetMmsMessage 			pfGetMmsMessage;
	MsgPlgUpdateRejectStatus		pfUpdateRejectStatus;
	MsgPlgComposeReadReport 		pfComposeReadReport;
	MsgPlgGetDefaultNetworkSimId	pfGetDefaultNetworkSimId;
};

#ifdef __cplusplus
}
#endif

#endif
