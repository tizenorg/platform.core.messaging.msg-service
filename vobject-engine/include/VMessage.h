/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef _VMSG_H
#define _VMSG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "VTypes.h"

/****************************************************************************************************/
/*											ENUMERATION DECLARATION									*/
/****************************************************************************************************/

/**
* @enum vMsgType
* This enumeration defines the property of vcard.
*/
typedef enum
{
	VMSG_TYPE_BEGIN,
	VMSG_TYPE_END,
	VMSG_TYPE_TEL,
	VMSG_TYPE_VBODY,
	VMSG_TYPE_VCARD,
	VMSG_TYPE_VMSG,
	VMSG_TYPE_BODY,
	VMSG_TYPE_SUBJECT,
	VMSG_TYPE_MSGBOX,
	VMSG_TYPE_STATUS,
	VMSG_TYPE_MSGTYPE,
	VMSG_TYPE_DATE,
}vMsgType;

#define VMSG_TYPE_NUM	12		/**< number of vcard type */

/**
* @enum vMsgParamName
* This enumeration defines the name of vcard parameter.
*/
typedef enum
{
	VMSG_PARAM_CHARSET,		/**<This is charset parameter. */
	VMSG_PARAM_CONTEXT,		/**<This is context parameter. */
	VMSG_PARAM_ENCODING,	/**<This is encoding parameter. */
	VMSG_PARAM_LANGUAGE,	/**<This is language parameter. */
	VMSG_PARAM_TYPE,			/**<This is type parameter. */
	VMSG_PARAM_VALUE		/**<This is value parameter. */
}vMsgParamName;

#define VMSG_PARAM_NUM		6	/**< number of vcard parameter */

/**
* @enum vMsgParamName
* This enumeration defines the value of encoding parameter.
*/
typedef enum
{
	VMSG_ENC_PARAM_B,					/**<This is b encoding parameter. */
	VMSG_ENC_PARAM_BASE64,				/**<This isbase64 encoding parameter. */
	VMSG_ENC_PARAM_QUOTED_PRINTABLE,	/**<This is quoted printable encoding parameter. */
	VMSG_ENC_PARAM_7BIT,				/**<This is 7 bit encoding parameter. */
	VMSG_ENC_PARAM_8BIT					/**<This is 8 bit encoding parameter. */
}vMsgEncVal;

#define VMSG_ENCODE_PARAM_NUM	5	/**< number of vcard encoding parameter */

/**
* @enum vMsgCharsetVal
* This enumeration defines the value of charset parameter.
*/
typedef enum
{
	VMSG_CHARSET_PARAM_UTF_8,		/**<This is utf-8 charset parameter. */
	VMSG_CHARSET_PARAM_UTF_16,		/**<This is utf-16 charset parameter. */
	VMSG_CHARSET_PARAM_SHIFT_JIS,	/**<This is shift-jis charset parameter. */
	VMSG_CHARSET_PARAM_ISO_8859_1	/**<This is iso-8859-1 charset parameter. */
}vMsgCharsetVal;

#define VMSG_CHARSET_PARAM_NUM  4	/**< number of vcard charset parameter */

/**
* @enum vMsgValVal
* This enumeration defines the value of value parameter.
*/
typedef enum
{
	VMSG_VALUE_PARAM_BINARY,		/**<This is binary value parameter. */
	VMSG_VALUE_PARAM_BOOLEAN,		/**<This is boolean value parameter. */
	VMSG_VALUE_PARAM_DATE,			/**<This is date value parameter. */
	VMSG_VALUE_PARAM_DATE_TIME,	/**<This is date time value parameter. */
	VMSG_VALUE_PARAM_FLOAT,		/**<This is float value parameter. */
	VMSG_VALUE_PARAM_INTEGER,		/**<This is integer value parameter. */
	VMSG_VALUE_PARAM_PHONE_NUMBER,	/**<This is phone number value parameter. */
	VMSG_VALUE_PARAM_TEXT,				/**<This is text value parameter. */
	VMSG_VALUE_PARAM_TIME,				/**<This is time value parameter. */
	VMSG_VALUE_PARAM_URI,				/**<This is uri value parameter. */
	VMSG_VALUE_PARAM_URL,				/**<This is url value parameter. */
	VMSG_VALUE_PARAM_UTC_OFFSET,		/**<This is utc offset value parameter. */
	VMSG_VALUE_PARAM_VMSG				/**<This is vcard value parameter. */
}vMsgValVal;

#define VMSG_VALUE_PARAM_NUM		13		/**< number of vcard value parameter */

/**
* @enum vMsgValVal
* This enumeration defines the value of type parameter.
*/
typedef enum
{
	VMSG_TYPE_PARAM_AIFF,		/**<This is aiff type parameter. */
	VMSG_TYPE_PARAM_BBS,		/**<This is bbs type parameter. */
	VMSG_TYPE_PARAM_CAR,		/**<This is car type parameter. */
	VMSG_TYPE_PARAM_CELL,		/**<This is cell type parameter. */
	VMSG_TYPE_PARAM_DOM,		/**<This is dom type parameter. */
	VMSG_TYPE_PARAM_WORK,		/**<This is work type parameter. */
	VMSG_TYPE_PARAM_FAX,		/**<This is fax type parameter. */
	VMSG_TYPE_PARAM_GIF,		/**<This is gif type parameter. */
	VMSG_TYPE_PARAM_HOME,		/**<This is home type parameter. */
	VMSG_TYPE_PARAM_INTL,		/**<This is intl type parameter. */
	VMSG_TYPE_PARAM_INTERNET,	/**<This is internet type parameter. */
	VMSG_TYPE_PARAM_ISDN,		/**<This is ISDN type parameter. */
	VMSG_TYPE_PARAM_JPEG,		/**<This is jpeg type parameter. */
	VMSG_TYPE_PARAM_MOBILE,		/**<This is mobile type parameter. */
	VMSG_TYPE_PARAM_MODEM,		/**<This is mpdem type parameter. */
	VMSG_TYPE_PARAM_MSG,		/**<This is msg type parameter. */
	VMSG_TYPE_PARAM_PAGER,		/**<This is pager type parameter. */
	VMSG_TYPE_PARAM_PARCEL,		/**<This is parcel type parameter. */
	VMSG_TYPE_PARAM_PCM,		/**<This is PCM type parameter. */
	VMSG_TYPE_PARAM_PCS,		/**<This is PCS type parameter. */
	VMSG_TYPE_PARAM_PNG,		/**<This is png type parameter. */
	VMSG_TYPE_PARAM_POSTAL,		/**<This is potsal type parameter. */
	VMSG_TYPE_PARAM_PREF,		/**<This is pref type parameter. */
	VMSG_TYPE_PARAM_VIDEO,		/**<This is video type parameter. */
	VMSG_TYPE_PARAM_VOICE,		/**<This is voice type parameter. */
	VMSG_TYPE_PARAM_WAVE,		/**<This is wave type parameter. */
	VMSG_TYPE_PARAM_WBMP,		/**<This is wbmp type parameter. */
	VMSG_TYPE_PARAM_ETC,		/**<This is etc type parameter. */
	VMSG_TYPE_PARAM_X400,		/**<This is X400 type parameter. */
	VMSG_TYPE_PARAM_X_IRMC_N	/**<This is X-IRMC-N type parameter. */
}vMsgTypeVal;

#define VMSG_TYPE_PARAM_NUM		30		/**< number of vcard type parameter */

/* VCard Encoder/Decoder status. */
#define VMSG_TYPE_NAME_STATUS	1	/**< vcard type name status */
#define VMSG_PARAM_NAME_STATUS	2	/**< vcard parameter name status */
#define VMSG_TYPE_VALUE_STATUS	3	/**< vcard type value status */
#define VMSG_PARAM_VALUE_STATUS 4	/**< vcard parameter value status */


/*
 * Public Function Prototypes
 */


/**
* @fn VTree* vcard_decode(char* pVCardRaw);
* This function decodes a vcard string to a vTree.
*
* @return	This function returns a pointer to VTree.
* @param[in] pVCardRaw  Points to the vcard string.
* @see vcard_encode
*/
SLPAPI VTree*	vmsg_decode(char* pVMsgRaw);

/**
* @fn char* vcard_encode(VTree* pVTree);
* This function encodes a vTree to a string.
*
* @return	This function returns a pointer to a vcard string.
* @param[in] pVTree  Points to a VTree.
* @see vcard_decode
*/
SLPAPI char*	vmsg_encode(VTree* pVTree);

/**
* @fn char* vcard_free_vtree_memory(VTree* pTree);
* This function free a pTree allocated memory
*
* @return	This function returns value of success or fail
* @param[in] pVTree  Points to a VTree.
*/
SLPAPI bool vmsg_free_vtree_memory(VTree * pTree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VMSG_H_ */

/**
* @}
*/
