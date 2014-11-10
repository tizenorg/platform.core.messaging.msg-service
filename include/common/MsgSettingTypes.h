/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef MSG_SETTING_TYPES_H
#define MSG_SETTING_TYPES_H

/**
 *	@file 		MsgSettingTypes.h
 *	@brief 		Defines setting types of messaging framework
 *	@version 	1.0
 */

/**
 *	@section		Introduction
 *	- Introduction : Overview on Messaging Setting Types
 *	@section		Program
 *	- Program : Messaging Setting Types Reference
 */

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgTypes.h"

/**
 *	@ingroup		MESSAGING_FRAMEWORK
 *	@defgroup	MESSAGING_SETTING_TYPES	Messaging Setting Types
 *	@{
 */

/*==================================================================================================
                                         TYPES
==================================================================================================*/

/**
 *	@brief	Represents an option type. \n
 *	The values for this type SHOULD be in _MSG_OPTION_TYPE_E.
 */
typedef unsigned int MSG_OPTION_TYPE_T;


/**
 *	@brief	Represents an SMS network mode. \n
 *	The values for this type SHOULD be in _MSG_SMS_NETWORK_MODE_E.
 */
typedef unsigned char MSG_SMS_NETWORK_MODE_T;


/**
 *	@brief	Represents the period of playing alert tone. \n
 *	The values for this type SHOULD be in _MSG_ALERT_TONE_E.
 */
typedef unsigned char MSG_ALERT_TONE_T;


/**
 *	@brief	Represents the default storage to save SMS. \n
 *	The values for this type SHOULD be in _MSG_SMS_SAVE_STORAGE_E.
 */
typedef unsigned char MSG_SMS_SAVE_STORAGE_T;


/**
 *	@brief	Represents the type of number for SMS center address. \n
 *	The values for this type SHOULD be in _MSG_SMS_TON_E.
 */
typedef unsigned char MSG_SMS_TON_T;


/**
 *	@brief	Represents the numbering plan ID for SMS center address. \n
 *	The values for this type SHOULD be in _MSG_SMS_NPI_E.
 */
typedef unsigned char MSG_SMS_NPI_T;


/**
 *	@brief	Represents the protocol ID for SMS center address. \n
 *	The values for this type SHOULD be in _MSG_SMS_PID_E.
 */
typedef unsigned char MSG_SMS_PID_T;


/**
 *	@brief	Represents the validity period of SMS. \n
 *	The values for this type SHOULD be in _MSG_VAL_PERIOD_E.
 */
typedef unsigned char MSG_VAL_PERIOD_T;


/**
 *	@brief	Represents the class type of MMS. \n
 *	The values for this type SHOULD be in _MSG_MMS_MSG_CLASS_TYPE_E.
 */
typedef unsigned char MSG_MMS_MSG_CLASS_TYPE_T;


/**
 *	@brief	Represents the expiry time of MMS. \n
 *	The values for this type SHOULD be in _MSG_MMS_EXPIRY_TIME_E.
 */
typedef unsigned int MSG_MMS_EXPIRY_TIME_T;


/**
 *	@brief	Represents the delivery time of MMS. \n
 *	The values for this type SHOULD be in _MSG_MMS_DELIVERY_TIME_E.
 */
typedef unsigned int MSG_MMS_DELIVERY_TIME_T;


/**
 *	@brief	Represents the reply charging type of MMS. \n
 *	The values for this type SHOULD be in _MSG_MMS_REPLY_CHARGING_TYPE_E.
 */
typedef unsigned char MSG_MMS_REPLY_CHARGING_TYPE_T;


/**
 *	@brief	Represents the creation mode of MMS. \n
 *	The values for this type SHOULD be in _MSG_MMS_CREATION_MODE_E.
 */
typedef unsigned char MSG_MMS_CREATION_MODE_T;

/**
 *	@brief	Represents the retrieve type for MMS home network. \n
 *	The values for this type SHOULD be in _MSG_MMS_HOME_RETRIEVE_TYPE_E.
 */
typedef unsigned char MSG_MMS_HOME_RETRIEVE_TYPE_T;


/**
 *	@brief	Represents the retrieve type for MMS abroad network. \n
 *	The values for this type SHOULD be in _MSG_MMS_ABROAD_RETRIEVE_TYPE_E.
 */
typedef unsigned char MSG_MMS_ABROAD_RETRIEVE_TYPE_T;


/**
 *	@brief	Represents the frequency of sending MMS read report. \n
 *	The values for this type SHOULD be in _MSG_MMS_SEND_READ_REPORT_E.
 */
typedef unsigned char MSG_MMS_SEND_READ_REPORT_T;


/**
 *	@brief	Represents whether a push message is received or not. \n
 *	The values for this type SHOULD be in _MSG_PUSH_RECEIVE_TYPE_E.
 */
typedef unsigned char MSG_PUSH_RECEIVE_TYPE_T;


/**
 *	@brief	Represents the service type for a push message. \n
 *	The values for this type SHOULD be in _MSG_PUSH_SERVICE_TYPE_E.
 */
typedef unsigned char MSG_PUSH_SERVICE_TYPE_T;


/**
 *	@brief	Represents the language type for a cell broadcasting message. \n
 *	The values for this type SHOULD be in _MSG_CB_LANGUAGE_TYPE_E.
 */
typedef unsigned char MSG_CB_LANGUAGE_TYPE_T;


/**
 *	@brief	Represents the SIM status from telephony. \n
 *	The values for this type SHOULD be in \ref _MSG_SIM_STATUS_E.
 */
typedef unsigned char MSG_SIM_STATUS_T;


/*==================================================================================================
                                         STRUCTURES
==================================================================================================*/

/**
 *	@brief	Represents SMSC address information.
 */
typedef struct
{
	MSG_SMS_TON_T			ton;								/**< Type of number */
	MSG_SMS_NPI_T			npi;								/**< Numbering plan ID */
	char						address[SMSC_ADDR_MAX+1];		/**< SMSC address */
} MSG_SMSC_ADDRESS_S;


/**
 *	@brief	Represents SMSC data information.
 */
typedef struct
{
	MSG_SMS_PID_T			pid;							/**< Protocol idendifier */
	MSG_VAL_PERIOD_T		valPeriod;					/**< Validity period value */
	char						name[SMSC_NAME_MAX+1];		/**< SMSC name */
	MSG_SMSC_ADDRESS_S		smscAddr;					/**< SMSC address structure */
} MSG_SMSC_DATA_S;

/**
 *	@brief	Represents an SMSC list.
 */
typedef struct
{
	int						selected;							/**< Selected SMSC index */
	int						totalCnt;							/**< The count of total SMSC addresses */
	MSG_SMSC_DATA_S			smscData[SMSC_LIST_MAX];			/**< SMSC data information list*/
}MSG_SMSC_LIST_S;

/**
 *	@brief	Represents an SMSC list.
 */
typedef struct
{
	int						selected;				/**< Selected SMSC index */
	msg_struct_list_s		*smsc_list;				/**< SMSC data information list*/
}MSG_SMSC_LIST_HIDDEN_S;

/**
 *	@brief	Represents the information of a cell broadcasting channel.
 */
typedef struct
{
	bool			bActivate;							/**< Indicates whether the CB channel is activate or passive. */
	unsigned int 	from;								/**< Indicates the start ID of a CB channel range. */
	unsigned int 	to;									/**< Indicates the end ID of a CB channel range. */
	char			name[CB_CHANNEL_NAME_MAX+1];		/**< Indicates the name of a CB channel. */
} MSG_CB_CHANNEL_INFO_S;


/**
 *	@brief	Represents cell broadcasting channel information.
 */
typedef struct
{
	int						channelCnt;					/**< The count of CB channels */
	MSG_CB_CHANNEL_INFO_S	channelInfo[CB_CHANNEL_MAX];	/**< The structure of CB channel information */
} MSG_CB_CHANNEL_S;

/**
 *	@brief	Represents an general option.
 */
typedef struct
{
	bool							bKeepCopy;			/**< Indicates whether the SMS message copy is kept or not. */
	MSG_ALERT_TONE_T			alertTone;			/**< Indicates the period of playing alert tone. */
	bool							bAutoErase;			/**< Indicates whether the auto-erase option is enabled or not. */
}MSG_GENERAL_OPT_S;


/**
 *	@brief	Represents an SMS send option.
 */
typedef struct
{
	msg_encode_type_t			dcs;					/**< Indicates the string encoding type. */
	MSG_SMS_NETWORK_MODE_T	netMode;				/**< Indicates the network mode (CS/PS) to send SMS. */
	bool							bReplyPath;			/**< Indicates whether the SMS reply path is set or not. */
	bool							bDeliveryReport;		/**< Indicates whether the SMS delivery report will be sent or not. */
	MSG_SMS_SAVE_STORAGE_T		saveStorage;			/**< Indicates the default storage to save SMS. */
}MSG_SMS_SENDOPT_S;


/**
 *	@brief	Represents an MMS send option.
 */
typedef struct
{
	MSG_MMS_MSG_CLASS_TYPE_T 	msgClass;
	msg_priority_type_t			priority;				/**< Indicates the priority of the message. */
	MSG_MMS_EXPIRY_TIME_T		expiryTime;			/**< Indicates the time when the message is to be removed from the MMSC. */
	MSG_MMS_DELIVERY_TIME_T		deliveryTime;			/**< Indicates the message transmission time which is set in the MMSC. */
	unsigned int					customDeliveryTime;	/**< Indicates the message transmission time which is set in the MMSC. */
	bool							bSenderVisibility;		/**< Indicates whether the address is hidden or not. */
	bool							bDeliveryReport;		/**< Indicates whether the delivery report will be sent or not. */
	bool							bReadReply;			/**< Indicates whether the read report will be sent or not. */
	bool							bKeepCopy;			/**< Indicates whether the message copy is kept or not. */
	bool							bBodyReplying;		/**< Indicates whether the body is included when replying or not. */
	bool							bHideRecipients;		/**< Indicates whether the recipients are hidden or not. */

	MSG_MMS_REPLY_CHARGING_TYPE_T 	replyCharging;
	unsigned int						replyChargingDeadline;
	unsigned int /*long*/				replyChargingSize;

	MSG_MMS_CREATION_MODE_T	creationMode;
} MSG_MMS_SENDOPT_S;


/**
 *	@brief	Represents an MMS receive option.
 */
typedef struct
{
	MSG_MMS_HOME_RETRIEVE_TYPE_T		homeNetwork;	/**< Indicates the retrieve type for MMS home network. */
	MSG_MMS_ABROAD_RETRIEVE_TYPE_T	abroadNetwok;	/**< Indicates the retrieve type for MMS abroad network. */
//	MSG_MMS_SEND_READ_REPORT_T		readReceipt;	/**< Indicates whether the read report will be sent or not. */
	bool				readReceipt;					/**< Indicates whether the read report will be sent or not. */
	bool				bDeliveryReceipt;			/**< Indicates whether the delivery report will be sent or not. */
	bool				bRejectUnknown;			/**< Indicates whether unknown addresses are rejected or not. */
	bool				bRejectAdvertisement;	/**< Indicates whether advertisement is rejected or not. */
} MSG_MMS_RECVOPT_S;


/**
 *	@brief	Represents an MMS style option.
 */
typedef struct
{
	unsigned int	fontSize;
	bool			bFontStyleBold;
	bool			bFontStyleItalic;
	bool			bFontStyleUnderline;
	unsigned int	fontColorRed;
	unsigned int	fontColorGreen;
	unsigned int	fontColorBlue;
	unsigned int	fontColorHue;
	unsigned int	bgColorRed;
	unsigned int	bgColorGreen;
	unsigned int	bgColorBlue;
	unsigned int	bgColorHue;
	unsigned int	pageDur;
	unsigned int	pageCustomDur;
	unsigned int	pageDurManual;
} MSG_MMS_STYLEOPT_S;


/**
 *	@brief	Represents a push message option.
 */
typedef struct
{
	bool						bReceive;		/**< Indicates whether the push message is received or not. */
	MSG_PUSH_SERVICE_TYPE_T	serviceType;		/**< Indicates the service type of a push message. */
} MSG_PUSHMSG_OPT_S;


/**
 *	@brief	Represents a cell broadcasting message option.
 */
typedef struct
{
	bool				bReceive;		/**< Indicates whether the CB message is received or not. */
	int					maxSimCnt;		/**< Indicates the number of channels which can be stored in SIM. */
	MSG_CB_CHANNEL_S	channelData;		/**< Indicates the cell broadcasting channel information. */
	bool				bLanguage[CB_LANG_TYPE_MAX];		/**< Indicates whether the language name of a cell broadcasting message is set or not. */
} MSG_CBMSG_OPT_S;

typedef struct
{
	bool					bReceive;		/**< Indicates whether the CB message is received or not. */
	int					maxSimCnt;		/**< Indicates the number of channels which can be stored in SIM. */
	msg_struct_list_s	*channelData;		/**< Indicates the cell broadcasting channel information. */
	bool					bLanguage[CB_LANG_TYPE_MAX];		/**< Indicates whether the language name of a cell broadcasting message is set or not. */
} MSG_CBMSG_OPT_HIDDEN_S;

/**
 *	@brief	Represents a voice mail number option.
 */
typedef struct
{
	char mailNumber[MAX_PHONE_NUMBER_LEN+1];
} MSG_VOICEMAIL_OPT_S;


/**
 *	@brief	Represents a MMS size option.
 */
typedef struct
{
	int nMsgSize;
} MSG_MSGSIZE_OPT_S;


/**
 *	@brief	Represents setting information.
 */
typedef struct
{
	MSG_OPTION_TYPE_T	type;		/**< The option type in a setting */

	union
	{
		MSG_GENERAL_OPT_S		generalOpt;		/**< General option */
		MSG_SMS_SENDOPT_S		smsSendOpt;		/**< SMS send option */
		MSG_SMSC_LIST_S			smscList;			/**< SMSC list option */
		MSG_MMS_SENDOPT_S		mmsSendOpt;		/**< MMS send option */
		MSG_MMS_RECVOPT_S		mmsRecvOpt;		/**< MMS receive option */
		MSG_MMS_STYLEOPT_S		mmsStyleOpt;		/**< MMS style option */
		MSG_PUSHMSG_OPT_S		pushMsgOpt;		/**< Push message option */
		MSG_CBMSG_OPT_S		cbMsgOpt;		/**< Cell broadcasting message option */
		MSG_VOICEMAIL_OPT_S		voiceMailOpt;		/**< voice mail option */
		MSG_MSGSIZE_OPT_S		msgSizeOpt;		/**< MMS size option */
	} option;
}MSG_SETTING_S;


/**
 *	@}
 */

#endif // MSG_SETTING_TYPES_H

