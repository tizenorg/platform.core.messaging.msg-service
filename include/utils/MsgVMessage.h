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
char* MsgVMessageEncode(MSG_MESSAGE_INFO_S *pMsg);
char *MsgVMessageEncodeSMS(MSG_MESSAGE_INFO_S *pMsg);
msg_error_t MsgVMessageDecodeSMS(const char *vmsg_stream, MSG_MESSAGE_INFO_S *pMsg);
bool _convert_vdata_str_to_tm(const char* szText, struct tm * tm);

#endif /* MSG_VMESSAGE_H */

