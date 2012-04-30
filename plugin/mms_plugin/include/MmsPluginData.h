/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
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

