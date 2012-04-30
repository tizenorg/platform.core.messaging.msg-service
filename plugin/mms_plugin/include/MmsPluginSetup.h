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

#include "MmsPluginMessage.h"

#define MSG_STDSTR_SHORT			0x7F

/*
 * structures for setup
 */

typedef enum {
	MMS_RECEIVE_AUTO,
	MMS_RECEIVE_MANUAL,
	MMS_RECEIVE_RESTRICT,
	MMS_RECEIVE_REJECT,
	MMS_RECEIVE_CNT,
} MmsRecvType;

typedef enum {
	MMS_RESTRICTED_MODE,
	MMS_WARNING_MODE,
	MMS_FREE_MODE
} MmsUiMsgCreationMode;

typedef	enum {
	MMS_OPCO_DEFAULT	= 0,
	MMS_OPCO_FRANCE		= 1,
	MMS_OPCO_GERMAN		= 2,
	MMS_OPCO_GREECE		= 3,
	MMS_OPCO_IRELANDS	= 4,
	MMS_OPCO_ITALY		= 5,
	MMS_OPCO_NETHERLANDS = 6,
	MMS_OPCO_PORTUGAL	= 7,
	MMS_OPCO_SPAIN		= 8,
	MMS_OPCO_SWEDEN		= 9,
	MMS_OPCO_SWISS		= 10,
	MMS_OPCO_UK			= 11,
	MMS_OPCO_AUSTRIA	= 12,
	MMS_OPCO_BELGIUM	= 13,
	MMS_OPCO_HUNGARY	= 14,
	MMS_OPCO_NUM
} MmsOpCo;

typedef	struct {
	bool bHideAddress;
	bool bAskDeliveyReport;
	bool bAskReadReply;
	MmsPriority priority;
	char szFrom[MSG_LOCALE_ADDR_LEN + 10];
	MmsMsgClass msgClass;
	MmsTimeStruct expiryTime;
	MmsTimeStruct deliveryTime;
	bool bLeaveCopy;
	bool bAttachVcard;
	bool bAttachSign;
	bool bExpiryUseCustomTime;

	bool bDeliveryUseCustomTime;

	MmsTimeStruct expiryCustomTime;
	MmsTimeStruct deliveryCustomTime;

	char szSign[MSG_LOCALE_SIGN_LEN + 1];

	//MMS_V1_1
	MmsReplyCharge replyCharge;

	int creationMode; // Creation Mode
} MmsSendSetup;

typedef struct {
	bool bSendDeliveryReport;
	bool bSendReadReply;
	bool bAnonymousReject;
	bool bRejectAdvertisement;
	MmsRecvType recvHomeNetwork;
	MmsRecvType recvRoamingNetwork;
} MmsRecvSetup;

typedef	struct {
	bool bUserSettingMsgMaxSize;
	int msgMmsMaxSize;
} MmsMsgSizeSetup;


typedef struct {
	MmsUiMsgCreationMode creationMode;
} MmsCreationModeSetup;


typedef struct {
	bool bCustomTime;
	int slideIntervalTime;
	int customTime;
} MmsSlideSetup;

typedef struct {
	int	postcardName;
	int	postcardAdditionalInfo;
	int	postcardStreet;
	int	postcardZip;
	int	postcardCity;
	int	postcardStateNProvince;
	int	postcardCountry;
} MmsPostcardSetup;

typedef struct {
	MmsSendSetup sendSetup;
	MmsRecvSetup recvSetup;
	MmsMsgSizeSetup msgSizeSetup;
	MmsPostcardSetup postcardSetup[5];									// ref) POSTCARD_EDIT_TYPE_EXTENDED4

	MmsCreationModeSetup creationModeSetup;

	int autoResizeSize;

	/* UI not-visible fields ------------------------------------ */

	MmsOpCo nOpCo;
	int maxSendSize;
	char szPostcardOffice[MSG_STDSTR_SHORT + 1];				// Only Privosioning can change this field
	char szPostcardOfficeExtend01[MSG_STDSTR_SHORT + 1];
	char szPostcardOfficeExtend02[MSG_STDSTR_SHORT + 1];
	char szPostcardOfficeExtend03[MSG_STDSTR_SHORT + 1];
	char szPostcardOfficeExtend04[MSG_STDSTR_SHORT + 1];
	bool bPostcardAvailable;									// Only Privosioning can change this field
	char szMmlUrl[MSG_STDSTR_SHORT + 1];					//MML Home Url
	char szMmlPhoneNumber[MSG_STDSTR_SHORT + 1];			//MML Upload phonenumber

	char szMmsTmplDownloadURL[MSG_STDSTR_SHORT + 1];	//  for MMS More Templates

} MmsSetup;

/* global Setting */
extern MmsSetup gMmsSetup;

