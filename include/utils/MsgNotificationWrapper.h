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
