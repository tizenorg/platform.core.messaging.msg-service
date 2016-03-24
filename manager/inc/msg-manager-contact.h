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
	msg_message_id_t		msgId;
	int						msgType;
	int						folderId;
	int						simIndex;
	char					msgText[MAX_CONTACT_TEXT_LEN + 1];
	char					subject[MAX_CONTACT_TEXT_LEN + 1];
	msg_list_handle_t		addrList;
} contactInfo;

typedef struct
{
	msg_contact_id_t	contactId;							/**< Indicates the unique contact ID. */
	int						addrbookId;							/**< Indicates the address book ID. */
	char 					firstName[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the first name of contact. */
	char 					lastName[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the last name of contact. */
	char 					middleName[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the middle name of contact. */
	char 					prefix[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the prefix of contact. */
	char 					suffix[MAX_DISPLAY_NAME_LEN+1];		/**< Indicates the suffix of contact. */
	char 					imagePath[MSG_FILEPATH_LEN_MAX+1];		/**< Indicates the image path of contact. */
	char						alerttonePath[MSG_FILEPATH_LEN_MAX+1];		/**< Indicates the message alert tone path of contact. */
	char						vibrationPath[MSG_FILEPATH_LEN_MAX+1];		/**< Indicates the vibration path of contact. */
} MSG_MGR_CONTACT_INFO_S;

typedef struct
{
	msg_address_type_t		addressType;													/**< The type of an address in case of an Email or a mobile phone */
	msg_recipient_type_t		recipientType;													/**< The type of recipient address in case of To, Cc, and Bcc */
	msg_contact_id_t			contactId;															/**< The contact ID of address */
	char										addressVal[MAX_ADDRESS_VAL_LEN+1];		/**< The actual value of an address */
	char										displayName[MAX_DISPLAY_NAME_LEN+1];	/**< The display name of an address */
} MSG_MGR_ADDRESS_INFO_S;

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

int MsgMgrOpenContactSvc();
int MsgMgrCloseContactSvc();
void MsgMgrAddPhoneLog(contactInfo *contact_info);
int MsgMgrGetContactInfo(const MSG_MGR_ADDRESS_INFO_S *pAddrInfo, MSG_MGR_CONTACT_INFO_S *pContactInfo);

#endif /*__MSG_MGR_CONTACT_H__ */

