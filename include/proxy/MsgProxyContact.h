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

#ifndef MSG_PROXY_CONTACT_H
#define MSG_PROXY_CONTACT_H


/*==================================================================================================
											INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"

/*==================================================================================================
											FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgOpenContactSvc();
msg_error_t MsgCloseContactSvc();

msg_error_t MsgGetContactSearchList(const char *pSearchVal, MSG_ADDRESS_INFO_S **pAddrInfo, int *count);


#endif /* MSG_PROXY_CONTACT_H */

