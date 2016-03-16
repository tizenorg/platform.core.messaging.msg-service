/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved
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

#ifndef __MSG_MGR_CONTACT_H__
#define __MSG_MGR_CONTACT_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <msg.h>
#include <contacts.h>

/*==================================================================================================
                                    DEFINES
==================================================================================================*/
#define MAX_CONTACT_TEXT_LEN 100


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/
typedef struct _contactInfo {
	int msgId;
	int msgType;
	int folderId;
	int simIndex;
	char msgText[MAX_CONTACT_TEXT_LEN + 1];
	char subject[MAX_CONTACT_TEXT_LEN + 1];
	msg_list_handle_t addrList;
} contactInfo;


/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

int MsgMgrOpenContactSvc();
int MsgMgrCloseContactSvc();
void MsgMgrAddPhoneLog(contactInfo *contact_info);

#endif /*__MSG_MGR_CONTACT_H__ */

