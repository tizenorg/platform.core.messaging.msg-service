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


/**
 *	@file 		MsgTestSetting.h
 *	@brief 		Defines setting test function of messaging framework
 *	@version		1.0
 */

#ifndef MSG_TEST_SETTING_H
#define MSG_TEST_SETTING_H

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Setting Test Function
 *	@section		Program
 *	- Program : Messaging Setting Test Function Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/

#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_SETTING_TEST_FUNCTION	Messaging Setting Test Function
 *	@{
 */

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

/**	@fn		void MsgTestSettingMain(MSG_HANDLE_T hMsgHandle)
 *	@brief	Shows a setting menu.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestSettingMain(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgSelectMenu(MSG_HANDLE_T hMsgHandle, char *pMenu)
 *	@brief	Selects a setting menu.
 *	@param[in]	hMsgHandle is Message handle. \n
 *	@param[in]	pMenu is a pointer that indicates which menu is selected. \n
 */
void MsgSelectMenu(MSG_HANDLE_T hMsgHandle, char *pMenu);


/**	@fn		void MsgTestGeneralOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestGeneralOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestSMSSendOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestSMSSendOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestSMSCList(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestSMSCList(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestMMSSendOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestMMSSendOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestMMSRecvOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestMMSRecvOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestMMSStyleOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestMMSStyleOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestPushMsgOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestPushMsgOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestCBMsgOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestCBMsgOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestVoiceMailOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestVoiceMailOpt(MSG_HANDLE_T hMsgHandle);


/**	@fn		void MsgTestMsgSizeOpt(MSG_HANDLE_T hMsgHandle)
 *	@brief	Tests MsgGetConfig and MsgSetConfig.
 *	@param[in]	hMsgHandle is Message handle.
 */
void MsgTestMsgSizeOpt(MSG_HANDLE_T hMsgHandle);

/**
 *	@}
 */

#endif // MSG_TEST_SETTING_H

