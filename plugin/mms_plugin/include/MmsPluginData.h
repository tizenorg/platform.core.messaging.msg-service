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

#define	MMS_INVALID_MSG_ID	-1

/* Mailbox type */
#define MSG_UI_MAILBOX_MY_FOLDER_MAX	10

typedef enum {
	MSG_MAILBOX_ROOT		= -1,
	MSG_MAILBOX_WRITE		= 0,
	MSG_MAILBOX_INBOX		= 1,
	MSG_MAILBOX_DRAFT		= 2,
	MSG_MAILBOX_SENT		= 3,
	MSG_MAILBOX_MAILBOX		= 4,
#ifdef _MSG_DB_INCLUDE_OUTBOX
	MSG_MAILBOX_OUTBOX		= 5,
#endif
	MSG_MAILBOX_TEMPLATE		= 6,		//NEW_TEMPLATE
	MSG_MAILBOX_MYFOLDER		= 7,
	MSG_MAILBOX_MYFOLDER_LIST	= 8,


	MSG_MAILBOX_PRESET			= (MSG_MAILBOX_MYFOLDER_LIST  + MSG_UI_MAILBOX_MY_FOLDER_MAX),

	MSG_MAILBOX_NONE

} MsgMailboxType;

