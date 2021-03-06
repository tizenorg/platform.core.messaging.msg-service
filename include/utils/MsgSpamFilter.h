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

#ifndef MSG_SPAM_FILTER_H
#define MSG_SPAM_FILTER_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgFilterTypes.h"
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"
#include "MsgSqliteWrapper.h"


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgSetFilterOperation(bool bSetFlag);
msg_error_t MsgGetFilterOperation(bool *pSetFlag);

bool MsgCheckFilter(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S *pMsgInfo);

#endif // MSG_SPAM_FILTER_H
