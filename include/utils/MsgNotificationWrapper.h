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

#ifndef MSG_QUICKPANEL_WRAPPER_H
#define MSG_QUICKPANEL_WRAPPER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgInternalTypes.h"
#include "MsgSqliteWrapper.h"


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define NORMAL_MSG_ICON_PATH	"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.png"
#define VOICE_MSG_ICON_PATH	"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.voice.png"
#define CB_MSG_ICON_PATH		"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.cb.png"
#define NOTI_MSG_ICON_PATH		"/opt/apps/org.tizen.message/res/icons/default/small/org.tizen.message.noti.png"

#define MSG_APP_PACKAGE_NAME "message"
#define MSG_APP_LOCALEDIR 		"/opt/apps/org.tizen.message/res/locale"

#define SENDING_MULTIMEDIA_MESSAGE_FAILED "IDS_MSGF_POP_SENDING_MULTIMEDIA_MESSAGE_FAILED"
#define MULTIMEDIA_MESSAGE_SENT                   "IDS_MSGF_POP_MULTIMEDIA_MESSAGE_SENT"
#define RETRIEVING_MESSAGE_FAILED                 "IDS_MSGF_POP_RETRIEVING_MESSAGE_FAILED"
#define MESSAGE_RETRIEVED                               "IDS_MSGF_BODY_MESSAGE_RETRIEVED"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

MSG_ERROR_T MsgInsertNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);

MSG_ERROR_T MsgInsertSmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_ID_T MsgId, MSG_DELIVERY_REPORT_STATUS_T Status);

MSG_ERROR_T MsgInsertMmsReportToNoti(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);

MSG_ERROR_T MsgDeleteNotiByMsgId(MSG_MESSAGE_ID_T msgId);

MSG_ERROR_T MsgDeleteNotiByThreadId(MSG_THREAD_ID_T ThreadId);

MSG_ERROR_T MsgInsertTicker(const char* pTickerMsg, const char* pLocaleTickerMsg);

#endif // MSG_QUICKPANEL_WRAPPER_H
