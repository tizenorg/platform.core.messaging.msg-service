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

#ifndef MSG_CONTACT_H
#define MSG_CONTACT_H

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgInternalTypes.h"


typedef void (*MsgContactChangeCB)();

//contacts-service is not used for gear
#ifndef MSG_CONTACTS_SERVICE_NOT_SUPPORTED
/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/
msg_error_t MsgOpenContactSvc();
msg_error_t MsgCloseContactSvc();

msg_error_t MsgInitContactSvc(MsgContactChangeCB cb);

msg_error_t MsgGetContactInfo(const MSG_ADDRESS_INFO_S *pAddrInfo, MSG_CONTACT_INFO_S *pContactInfo);
msg_error_t MsgGetContactSearchList(const char *pSearchVal, MSG_ADDRESS_INFO_S **pAddrInfo, int *count);

void MsgSyncAddressbook();
void MsgSyncContact();

bool MsgInsertContact(MSG_CONTACT_INFO_S *pContactInfo, const char *pNumber);
bool MsgUpdateContact(int index, int type);
bool MsgDeleteContact(int index);

int MsgGetContactNameOrder();
msg_error_t MsgGetContactStyleDisplayName(const char *first, const char *last, const char *middle, const char *prefix, const char *suffix, int contactNameOrder, char *displayName, unsigned int size);

void MsgAddPhoneLog(const MSG_MESSAGE_INFO_S *pMsgInfo);
void MsgDeletePhoneLog(msg_message_id_t msgId);

int MsgContactSVCBeginTrans();
int MsgContactSVCEndTrans(bool bSuccess);

bool checkBlockingMode(char *address, bool *pisFavorites);
#endif //MSG_CONTACTS_SERVICE_NOT_SUPPORTED

int MsgContactGetMinMatchDigit();
void MsgConvertNumber(const char* pSrcNum, char* pDestNum, int destSize);
bool MsgIsNumber(const char* pSrc);
#endif //MSG_CONTACT_H

