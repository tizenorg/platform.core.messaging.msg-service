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

#ifndef MSG_CONTACT_H
#define MSG_CONTACT_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"


typedef void (*MsgContactChangeCB)();

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
MSG_ERROR_T MsgOpenContactSvc();
MSG_ERROR_T MsgCloseContactSvc();

MSG_ERROR_T MsgInitContactSvc(MsgContactChangeCB cb);

MSG_ERROR_T MsgGetContactInfo(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_CONTACT_INFO_S *pContactInfo);

void MsgSyncContact();

bool MsgInsertContact(MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber);
bool MsgUpdateContact(int index, int type);
bool MsgDeleteContact(int index);

int MsgGetContactNameOrder();

int MsgContactSVCBeginTrans();
int MsgContactSVCEndTrans(bool bSuccess);


#endif //MSG_CONTACT_H

