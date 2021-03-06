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

#ifndef MMS_PLUGIN_SMIL_VALIDATE_H
#define MMS_PLUGIN_SMIL_VALIDATE_H


/*==================================================================================================
 *                                     HEADER
 *==================================================================================================*/
#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MmsPluginMessage.h"

/*==================================================================================================
 *                                     DEFINE
 *==================================================================================================*/
/**
 *	@brief	Represents the default values of SMIL root layout. \n
 */
#define MSG_SMIL_ROOT_LAYOUT_WIDTH	(100)
#define MSG_SMIL_ROOT_LAYOUT_HEIGHT	(100)
#define MSG_SMIL_ROOT_LAYOUT_IN_PERCENT	(true)
#define MSG_SMIL_ROOT_LAYOUT_BG_COLOR	"#FFFFFF"

/**
 *	@brief	Represents the default values of SMIL Region \n
 */
#define MSG_SMIL_DEFAULT_TXT_REG	"text"
#define MSG_SMIL_DEFAULT_IMG_REG	"image"
#define MSG_SMIL_DEFAULT_FULL_REG	"full"
#define MSG_SMIL_REG_FIT_TYPE		"meet"
#define MSG_SMIL_REG_BG_COLOR		"#FFFFFF"

#define MSG_SMIL_FULL_REG_LEFT				(0)
#define MSG_SMIL_FULL_REG_TOP				(0)
#define MSG_SMIL_FULL_REG_WIDTH				(100)
#define MSG_SMIL_FULL_REG_HEIGHT			(100)

#define MSG_SMIL_FIRST_REG_LEFT				(0)
#define MSG_SMIL_FIRST_REG_TOP				(0)
#define MSG_SMIL_FIRST_REG_WIDTH			(100)
#define MSG_SMIL_FIRST_REG_HEIGHT			(50)

#define MSG_SMIL_SECOND_REG_LEFT			(0)
#define MSG_SMIL_SECOND_REG_TOP				(50)
#define MSG_SMIL_SECOND_REG_WIDTH			(100)
#define MSG_SMIL_SECOND_REG_HEIGHT			(50)

/**
 *	@brief	Represents the default values of SMIL Media \n
 */
#define MSG_SMIL_REG_POSITION_IS_DEFAULT	(true)
#define MSG_SMIL_TEXT_ON_TOP				(true)


/*==================================================================================================
 *                                     ENUMS
 *==================================================================================================*/

/**
 *	@brief	Represents the values of an smil error code. \n
 *	Success code is zero, but all error codes SHOULD be negative and smaller than MSG_SMIL_ERR_BEGIN. \n
 */
enum MSG_SMIL_ERR_E {
	MSG_SMIL_SUCCESS = 0,
	MSG_SMIL_ERR_UNKNOWN = -255,
	MSG_SMIL_ERR_INVALID_ROOTLAYOUT,
	MSG_SMIL_ERR_INVALID_REGION_INFO,
	MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS,
	MSG_SMIL_ERR_INVALID_SMIL_FILE_PATH,
	MSG_SMIL_ERR_INVALID_PAGE_INFO,
	MSG_SMIL_ERR_INVALID_PAGE_DUR,
	MSG_SMIL_ERR_INVALID_PARAM,
};

/*==================================================================================================
 *                                     FUNCTIONS
 *==================================================================================================*/

/**	@fn		MSG_SMIL_ERR_E MsgMMSCreateSMIL(MMS_MESSAGE_DATA_S *pMsgData)
 *	@brief	Creates a SMIL buffer based on the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@param	pError is a pointer to a SMIL error type. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS		Success in operation. \n
 *	@retval	MSG_SMIL_ERR_UNKNOWN				Encoding failed \n
 *	@retval	MSG_SMIL_ERR_INVALID_ROOTLAYOUT		Invalid Root Layout \n
  *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO	Invalid Region \n
 *	@retval	MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS	Duplicate media \n
 *	@retval	MSG_SMIL_ERR_INVALID_SMIL_FILE_PATH	Invalid Smil Path \n
 *	@retval	MSG_SMIL_ERR_INVALID_PAGE_INFO		Invalid Page Info \n
 *	@retval	MSG_SMIL_ERR_INVALID_PAGE_DUR		Invalid Page Duration \n
 *	@retval	MSG_SMIL_ERR_INVALID_PARAM			Invalid Parameter \n
 */
MSG_SMIL_ERR_E MsgMMSCreateSMIL(MMS_MESSAGE_DATA_S *pMsgData);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSValidateSMILRootLayout(MMS_MESSAGE_DATA_S *pMsgData)
 *	@brief	Validates the Root layout information present in the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_ROOTLAYOUT			Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSValidateSMILRootLayout(MMS_MESSAGE_DATA_S *pMsgData);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSAddDefaultSMILRegions(MMS_MESSAGE_DATA_S *pMsgData)
 *	@brief	Adds the default Region information based on the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_ROOTLAYOUT			Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSAddDefaultSMILRootLayout(MMS_MESSAGE_DATA_S *pMsgData);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSValidateSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool *pbRegAdded)
 *	@brief	Validates the Region information present in the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO		Failure in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_PAGE_INFO			Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSValidateSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool *pbRegAdded);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSAddDefaultFirstSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg)
 *	@brief	Adds the default first Region information when two regions are present in a page based on the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO		Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSAddDefaultFirstSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSAddDefaultSecondSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg)
 *	@brief	Adds the default second Region information when two regions are present in a page based on the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO		Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSAddDefaultSecondSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSAddDefaultFullSMILRegion(MMS_MESSAGE_DATA_S *pMsgData)
 *	@brief	Adds the default full Region information when only region is present in a page based on the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO		Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSAddDefaultFullSMILRegion(MMS_MESSAGE_DATA_S *pMsgData);

/**	@fn		MSG_SMIL_ERR_E _MsgMMSValidateSMILPage(MMS_MESSAGE_DATA_S *pMsgData, bool bRegAdded)
 *	@brief	Validates the Page information present in the Message handle.
 *	@param	pMsgData is Message handle. \n
 *	@return	MSG_SMIL_ERR_E \n
 *	@retval	MSG_SMIL_SUCCESS						Success in operation. \n
 *	@retval	MSG_SMIL_ERR_INVALID_PAGE_INFO			Failure in operation. \n
 *	@retval	MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS		Same Kind of Media Exists in SMIL page. \n
 *	@retval	MSG_SMIL_ERR_INVALID_REGION_INFO		Failure in operation. \n
 */
MSG_SMIL_ERR_E _MsgMMSValidateSMILPage(MMS_MESSAGE_DATA_S *pMsgData, bool bRegAdded);

#endif // MMS_PLUGIN_SMIL_ENCODE_H
