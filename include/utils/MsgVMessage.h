/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef MSG_VMESSAGE_H
#define MSG_VMESSAGE_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"
#include "MsgUtilStorage.h"

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
char* MsgVMessageAddRecord(MsgDbHandler *pDbHandle, MSG_MESSAGE_INFO_S* pMsg);
char* MsgVMessageEncode(MSG_MESSAGE_INFO_S *pMsg);
char* _convert_tm_to_vdata_str(const struct tm * tm);
bool _convert_vdata_str_to_tm(const char* szText, struct tm * tm);

#endif //MSG_VMESSAGE_H

