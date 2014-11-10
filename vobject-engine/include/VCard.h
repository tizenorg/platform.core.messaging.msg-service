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

#ifndef _VCARD_H
#define _VCARD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "VTypes.h"

/****************************************************************************************************/
/*											ENUMERATION DECLARATION									*/
/****************************************************************************************************/

/**
* @enum vCardType
* This enumeration defines the property of vcard.
*/
typedef enum
{
	VCARD_TYPE_ADR,			/**<This is address of vcard. */
	VCARD_TYPE_AGENT,			/**<This is agent of vcard. */
	VCARD_TYPE_BDAY,			/**<This is bday of vcard. */
	VCARD_TYPE_BEGIN,			/**<This is begin of vcard. */
	VCARD_TYPE_CATEGORIES,	/**<This is categories of vcard. */
	VCARD_TYPE_CLASS,			/**<This is class of vcard. */
	VCARD_TYPE_EMAIL,			/**<This is email of vcard. */
	VCARD_TYPE_END,			/**<This is end of vcard. */
	VCARD_TYPE_FN,				/**<This is FN  of vcard. */
	VCARD_TYPE_GEO,			/**<This is GEO of vcard. */
	VCARD_TYPE_KEY,			/**<This is key of vcard. */
	VCARD_TYPE_LABEL,			/**<This is label of vcard. */
	VCARD_TYPE_LOGO,			/**<This is logo of vcard. */
	VCARD_TYPE_MAILER,		/**<This is mailer of vcar. */
	VCARD_TYPE_N,				/**<This is N of vcard. */
	VCARD_TYPE_NAME,			/**<This is name of vcard. */
	VCARD_TYPE_NICKNAME,		/**<This is nick name of vcard. */
	VCARD_TYPE_NOTE,			/**<This is note of vcard. */
	VCARD_TYPE_ORG,			/**<This is ORG of vcard. */
	VCARD_TYPE_PHOTO,			/**<This is photo of vcard. */
	VCARD_TYPE_PRODID,		/**<This is PRODID of vcard. */
	VCARD_TYPE_PROFILE,		/**<This is profile of vcard. */
	VCARD_TYPE_REV,			/**<This is REV of vcard. */
	VCARD_TYPE_ROLE,			/**<This is ROLE of vcard. */
	VCARD_TYPE_SORT_STRING,	/**<This is sort string of vcard. */
	VCARD_TYPE_SOUND,			/**<This is sound of vcard. */
	VCARD_TYPE_SOURCE,		/**<This is source of vcard. */
	VCARD_TYPE_TEL,			/**<This is tel of vcard. */
	VCARD_TYPE_TITLE,			/**<This is title of vcard. */
	VCARD_TYPE_TZ,				/**<This is TZ of vcard. */
	VCARD_TYPE_UID,			/**<This is uid of vcard. */
	VCARD_TYPE_URL,			/**<This is url of vcard. */
	VCARD_TYPE_VERSION,		/**<This is version of vcard. */
	VCARD_TYPE_XIRMCLUID		/**<This is xirmcl uid of vcard. */
}vCardType;

#define VCARD_TYPE_NUM	34		/**< number of vcard type */

/**
* @enum vCardParamName
* This enumeration defines the name of vcard parameter.
*/
typedef enum
{
	VCARD_PARAM_CHARSET,		/**<This is charset parameter. */
	VCARD_PARAM_CONTEXT,		/**<This is context parameter. */
	VCARD_PARAM_ENCODING,	/**<This is encoding parameter. */
	VCARD_PARAM_LANGUAGE,	/**<This is language parameter. */
	VCARD_PARAM_TYPE,			/**<This is type parameter. */
	VCARD_PARAM_VALUE		/**<This is value parameter. */
}vCardParamName;

#define VCARD_PARAM_NUM		6	/**< number of vcard parameter */

/**
* @enum vCardParamName
* This enumeration defines the value of encoding parameter.
*/
typedef enum
{
	VCARD_ENC_PARAM_B,					/**<This is b encoding parameter. */
	VCARD_ENC_PARAM_BASE64,				/**<This isbase64 encoding parameter. */
	VCARD_ENC_PARAM_QUOTED_PRINTABLE,	/**<This is quoted printable encoding parameter. */
	VCARD_ENC_PARAM_7BIT,				/**<This is 7 bit encoding parameter. */
	VCARD_ENC_PARAM_8BIT					/**<This is 8 bit encoding parameter. */
}vCardEncVal;

#define VCARD_ENCODE_PARAM_NUM	5	/**< number of vcard encoding parameter */

/**
* @enum vCardCharsetVal
* This enumeration defines the value of charset parameter.
*/
typedef enum
{
	VCARD_CHARSET_PARAM_UTF_8,		/**<This is utf-8 charset parameter. */
	VCARD_CHARSET_PARAM_UTF_16,		/**<This is utf-16 charset parameter. */
	VCARD_CHARSET_PARAM_SHIFT_JIS,	/**<This is shift-jis charset parameter. */
	VCARD_CHARSET_PARAM_ISO_8859_1	/**<This is iso-8859-1 charset parameter. */
}vCardCharsetVal;

#define VCARD_CHARSET_PARAM_NUM  4	/**< number of vcard charset parameter */

/**
* @enum vCardValVal
* This enumeration defines the value of value parameter.
*/
typedef enum
{
	VCARD_VALUE_PARAM_BINARY,		/**<This is binary value parameter. */
	VCARD_VALUE_PARAM_BOOLEAN,		/**<This is boolean value parameter. */
	VCARD_VALUE_PARAM_DATE,			/**<This is date value parameter. */
	VCARD_VALUE_PARAM_DATE_TIME,	/**<This is date time value parameter. */
	VCARD_VALUE_PARAM_FLOAT,		/**<This is float value parameter. */
	VCARD_VALUE_PARAM_INTEGER,		/**<This is integer value parameter. */
	VCARD_VALUE_PARAM_PHONE_NUMBER,	/**<This is phone number value parameter. */
	VCARD_VALUE_PARAM_TEXT,				/**<This is text value parameter. */
	VCARD_VALUE_PARAM_TIME,				/**<This is time value parameter. */
	VCARD_VALUE_PARAM_URI,				/**<This is uri value parameter. */
	VCARD_VALUE_PARAM_URL,				/**<This is url value parameter. */
	VCARD_VALUE_PARAM_UTC_OFFSET,		/**<This is utc offset value parameter. */
	VCARD_VALUE_PARAM_VCARD				/**<This is vcard value parameter. */
}vCardValVal;

#define VCARD_VALUE_PARAM_NUM		13		/**< number of vcard value parameter */

/**
* @enum vCardValVal
* This enumeration defines the value of type parameter.
*/
typedef enum
{
	VCARD_TYPE_PARAM_AIFF,		/**<This is aiff type parameter. */
	VCARD_TYPE_PARAM_BBS,		/**<This is bbs type parameter. */
	VCARD_TYPE_PARAM_CAR,		/**<This is car type parameter. */
	VCARD_TYPE_PARAM_CELL,		/**<This is cell type parameter. */
	VCARD_TYPE_PARAM_DOM,		/**<This is dom type parameter. */
	VCARD_TYPE_PARAM_WORK,		/**<This is work type parameter. */
	VCARD_TYPE_PARAM_FAX,		/**<This is fax type parameter. */
	VCARD_TYPE_PARAM_GIF,		/**<This is gif type parameter. */
	VCARD_TYPE_PARAM_HOME,		/**<This is home type parameter. */
	VCARD_TYPE_PARAM_INTL,		/**<This is intl type parameter. */
	VCARD_TYPE_PARAM_INTERNET,	/**<This is internet type parameter. */
	VCARD_TYPE_PARAM_ISDN,		/**<This is ISDN type parameter. */
	VCARD_TYPE_PARAM_JPEG,		/**<This is jpeg type parameter. */
	VCARD_TYPE_PARAM_MOBILE,		/**<This is mobile type parameter. */
	VCARD_TYPE_PARAM_MODEM,		/**<This is mpdem type parameter. */
	VCARD_TYPE_PARAM_MSG,		/**<This is msg type parameter. */
	VCARD_TYPE_PARAM_PAGER,		/**<This is pager type parameter. */
	VCARD_TYPE_PARAM_PARCEL,		/**<This is parcel type parameter. */
	VCARD_TYPE_PARAM_PCM,		/**<This is PCM type parameter. */
	VCARD_TYPE_PARAM_PCS,		/**<This is PCS type parameter. */
	VCARD_TYPE_PARAM_PNG,		/**<This is png type parameter. */
	VCARD_TYPE_PARAM_POSTAL,		/**<This is potsal type parameter. */
	VCARD_TYPE_PARAM_PREF,		/**<This is pref type parameter. */
	VCARD_TYPE_PARAM_VIDEO,		/**<This is video type parameter. */
	VCARD_TYPE_PARAM_VOICE,		/**<This is voice type parameter. */
	VCARD_TYPE_PARAM_WAVE,		/**<This is wave type parameter. */
	VCARD_TYPE_PARAM_WBMP,		/**<This is wbmp type parameter. */
	VCARD_TYPE_PARAM_ETC,		/**<This is etc type parameter. */
	VCARD_TYPE_PARAM_X400,		/**<This is X400 type parameter. */
	VCARD_TYPE_PARAM_X_IRMC_N	/**<This is X-IRMC-N type parameter. */
}vCardTypeVal;

#define VCARD_TYPE_PARAM_NUM		30		/**< number of vcard type parameter */

/* VCard Encoder/Decoder status. */
#define VCARD_TYPE_NAME_STATUS	1	/**< vcard type name status */
#define VCARD_PARAM_NAME_STATUS	2	/**< vcard parameter name status */
#define VCARD_TYPE_VALUE_STATUS	3	/**< vcard type value status */
#define VCARD_PARAM_VALUE_STATUS 4	/**< vcard parameter value status */


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
SLPAPI VTree*	vcard_decode(char* pVCardRaw);

/**
* @fn char* vcard_encode(VTree* pVTree);
* This function encodes a vTree to a string.
*
* @return	This function returns a pointer to a vcard string.
* @param[in] pVTree  Points to a VTree.
* @see vcard_decode
*/
SLPAPI char*	vcard_encode(VTree* pVTree);

/**
* @fn char* vcard_free_vtree_memory(VTree* pTree);
* This function free a pTree allocated memory
*
* @return	This function returns value of success or fail
* @param[in] pVTree  Points to a VTree.
*/
SLPAPI bool vcard_free_vtree_memory(VTree * pTree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VCARD_H_ */

/**
* @}
*/
