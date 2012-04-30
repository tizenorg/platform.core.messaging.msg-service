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

