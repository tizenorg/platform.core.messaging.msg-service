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


/**
 *	@file 		Main.h
 *	@brief 		Defines function for test application of messaging framework
 *	@version		1.0
 */

#ifndef MSG_TEST_MAIN_H
#define MSG_TEST_MAIN_H

/**
 *	@mainpage
 *	@section		Introduction
 *	- This document includes as follows : \n
 *	What is Messaging API \n
 *	What are included in Messaging API, such as data types, functions, and so on \n
 *	@section		Contents
 *	- Modules
 *	- Structure
 *	- MessageHandle
 *	- Request
 *	- Callback
 *	- Filter
 *	- Storage
 *	- Extendability
 *	@section		Modules
 *	MAPI provides a service to send, receive, parse and assemble messages over arbitrary message delivery mechanisms such as SMS, MMS, and E-mail.
 *	Messaging Service consists of a collection of common functionality and sets of service specific functionality.
 *	Use of particular messaging service specific functionality shall be possible.
 *	MAPI provides a collection of data structures as well as functions to implement the functionality. \n\n
 *	The data structures and functions are grouped into modules, as listed below:
 *	- Message sending and reception
 *	- Storage management
 *	- Message storage
 *	- Folder storage
 *	@section		Structure
 *	Message \n\n
 *	A message represents a generic data collection of SMS, MMS, and E-mail.
 *	This is not specific to any service, but supports the extension of this structure.
 *	For service specific message, please refer to extendability of common structure.
 *	Each message has a unique message ID in the entire messaging framework.
 *	The message information can be retrieved from the storage by its message ID. \n\n
 *	This concept is defined in MsgTypes.h. \n\n
 *	Folder \n
 *	A folder represents a container of messages.
 *	Each folder has a unique folder ID in the entire messaging framework.
 *	The folder information can be retrieved from the storage by its folder ID.
 *	A folder can specify where it is stored.
 *	The storage where a folder is stored can be different from the storage where its containing messages are stored. \n\n
 *	This concept is defined in MsgStorageTypes.h. \n\n
 *	@section		MessageHandle
 *	MessageHandle is the connection between application and MAPI.
 *	When an application wants to send, receive, add, or get a message, it must open a MessageHandle first.
 *	To know how to use asynchronous callback for communication between application and framework, please refer to Callback. \n\n
 *	A MessageHandle can be assigned with following information: \n
 *	- Register MsgOnMessageIncomingCallback function.
 *	- Register MsgOnStatusChangedCallback function.
 *	- Set filter rule.
 *	@section		Request
 *	A request contains the information that is used to command framework for services, such as sending a message.
 *	A request is sent to Messaging Framework via a MessageHandle, and Messaging Framework chooses proper plugin to handle the request.
 *	A common usage for the request is to send a message. \n\n
 *	This concept is defined in MsgTypes.h. \n\n
 *	@section		Callback
 *	Callback is used by application to detect incoming message and underlayer status change.
 *	Two callback functions are implemented by applications and should be bound to a MessageHandle.
 *	Both prototypes are provided: \n
 *	- One is MsgOnMessageIncomingCallback, which is to listen incoming message.
 *	- The other is MsgOnStatusChangedCallback, which is to listen underlayer status change.
 *	@section		Filter
 *	When an application is only interested in some kinds of messages, filter list is used to filter these specific messages and return them to the application.
 *	Noted that filter is for filtering incoming messages, not a storage concept.
 *	Filter is the basic unit to set the rules, it tells the filter service how to filter the message.
 *	Filter list is composed of various filters. \n\n
 *	They are defined in MsgFilterTypes.h. \n\n
 *	Application can set a filter list for a MessageHandle to get the interesting message.
 *	Please refer to MSG_FILTER_S and MSG_FILTER_LIST_S for details.
 *	MAPI does NOT define any filters.
 *	Application developers should refer to the specific implementation of filter service to get the supported filters.
 *	@section		Storage
 *	Storage is a collection of messages and related information that are stored on one storage media.
 *	Storage media is the physical data storage device, such as SD card and Flash.
 *	One storage media can have more than one storages.
 *	Since storage media can be removable, the storages on the media can be detached and attached dynamically.
 *	For instance, user plugs out SD card from UE 1 and plugs in this card into UE 2, all the messages stored in this card will be hidden from UE 1 and be visible to UE 2.
 *	For details information, please refer to MSG_STORAGE_LIST_S.
 *	Application can query supported storages by MAPI method MsgGetStorageList.
 *	@section		Extendability
 *	There are several structures in LiMo Messageing designed to be extended. They are: \n
 *	- MSG_FOLDER_INFO_S
 *	- msg_message_t
 *	- MSG_REQUEST_S
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "MsgTypes.h"

#define TEST_APP_PIRNT_STR_LEN			128

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_TEST_APPLICATION_BASIC_FUNCTION	Messaging Test Application Basic Function
 *	@{
 */


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MSG_FATAL(fmt, ...) \
	do \
	{\
		printf("\n[%d] [MSGFW: %s: %s(): %d] *FAILED* << " fmt" >>\n", get_tid(), rindex(__FILE__, '/')+1,  __FUNCTION__, __LINE__,  ##__VA_ARGS__);\
	} while (0)

#define MSG_DEBUG(fmt, ...) \
	do\
	{\
		printf("\n[%d] [MSGFW: %s: %s(): %d] " fmt"\n", get_tid(), rindex(__FILE__, '/')+1, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
	} while (0)

#define MSG_BEGIN() \
	do\
    {\
        printf("\n[%d] BEGIN >>>> %s() at [MSGFW: %s: %d]\n", get_tid(),__FUNCTION__, rindex(__FILE__, '/')+1,  __LINE__ );\
    } while( 0 )

#define MSG_END() \
	do\
    {\
        printf("\n[%d] END   <<<< %s() at [MSGFW: %s: %d]\n", get_tid(), __FUNCTION__, rindex(__FILE__, '/')+1,  __LINE__); \
    } \
    while( 0 )


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

void print(const char* string);


void print(int i);


int get_tid();


/**	@fn		MSG_ERROR_T init_app()
 *	@brief	Initializes a test application.
 *	@return	MSG_ERROR_T
 *	@retval	MSG_SUCCESS				Success in operation. \n
 *	@retval	MSG_ERR_NULL_POINTER		pMsg is NULL. \n
 *	@retval	MSG_ERR_MEMORY_ERROR		Memory is error. \n
 *	@retval	MSG_ERR_INVALID_MSGHANDLE	Message handle is invalid. \n
 *	@retval	MSG_ERR_INVALID_PARAMETER	Parameter is invalid. \n
 */
MSG_ERROR_T init_app();


/**	@fn		void show_menu()
 *	@brief	Shows folder information and the test menu list.
 */
void* show_menu(void*);


/**	@fn		void run_app(char menu)
 *	@brief	Runs a selected menu in the test menu list.
 *	@param[in]	Menu indicates which menu is selected.
 */
void run_app(char *pMenu);

/**
 *	@}
 */

#endif //MSG_TEST_MAIN_H

