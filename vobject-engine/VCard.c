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

#include "VTypes.h"
#include "VCard.h"
#include "vobject.h"

#define MAX_TYPE_NAME_LEN 50
#define MAX_PARAM_NAME_LEN 50
#define CHECK_START 1
#define CHECK_END 2

#define VFREE(obj) if (obj != NULL) { /*VDATA_TRACE("%p",obj);*/ free(obj); \
	obj = NULL; \
}

#define TRIM(obj) if (obj != NULL) {\
	_VRLSpace(obj);\
	_VRTSpace(obj);\
}

#define UPPER(obj, start, end) if (obj != NULL) {\
	for (start = 0; start < end; start++)\
		obj[start] = toupper(obj[start]);\
}

#define GO_NEXT_CHAR(current, rowData, addedCnt) {\
	current = *rowData;\
	rowData++;\
	(*addedCnt)++;\
}

#define SET_PARAM_VALUE(PARAM, SZVALUE, LIST, LISTCNT, PARAMOBJ, PTYPE, ENC) {\
	PARAM = 0;\
	PARAM |= __VCardGetValue(SZVALUE, LIST, LISTCNT);\
	if (PARAM != UNKNOWN_NAME) {\
		PARAMOBJ->parameter = PTYPE;\
		if (PTYPE == VCARD_PARAM_ENCODING)\
			ENC = PARAM;\
		break;\
	} \
}

#define LENGTH_TYPE_LIST(obj, len) for (len = 0; obj[len] != NULL; len++);

extern char* _VUnfoldingNoSpecNew(char *string);

/** GLOBAL VARIABLE DECLARATION AND INITIALIZATION */
/** vCard Types. */
char* pszCardTypeList[] = {
	"ADR",			/* Delivery Address -> address*/
	"AGENT",			/* Agent -> assistant name, assistant number*/
	"BDAY",			/* Birthday -> birthday */
	"BEGIN",			/* BEGIN VCARD DELIMITER*/
	"CATEGORIES",	/* Categories is a multivalued attribute */
	"CLASS",			/* */
	"EMAIL",			/* Email -> email */
	"END",			/* END VCARD DELIMITER*/
	"FN",			/* Formatted Name -> display_name */
	"GEO",			/* Geographic Positioning*/
	"KEY",			/* Public Key*/
	"LABEL",			/* Label Address -> address*/
	"LOGO",			/* Logo*/
	"MAILER",		/* Email Program (Optional)*/
	"N",				/* Name -> name */
	"NAME",			/* Name -> name */
	"NICKNAME",		/* Nickname -> nickname */
	"NOTE",			/* Note -> note */
	"ORG",			/* Organization Name or Organizational unit -> department*/
	"PHOTO",			/* Photograph -> caller id*/
	"PRODID",		/* */
	"PROFILE",		/* */
	"REV",			/* Last Revision(combination of calendar date & time)*/
	"ROLE",			/* Role or occupation */
	"SORT-STRING",	/* */
	"SOUND",			/* Sound*/
	"SOURCE",		/* */
	"TEL",			/* Telephone -> phone number */
	"TITLE",			/* Job Title -> job title */
	"TZ",			/* Time Zone*/
	"UID",			/* Unique Identifier*/
	"URL",			/* URL -> web address */
	"VERSION",		/* Version*/
	"X-IRMC-LUID",	/* */
	NULL
};

/** Parameter */
char* pszCardParamList[] = {
	"CHARSET",
	"CONTEXT",
	"ENCODING",
	"LANGUAGE",
	"TYPE",
	"VALUE"
};

/** Encoding value */
ValueObj pEncList[] = {
	{"B",			0x00000001},
	{"BASE64",	0x00000002},
	{"QUOTED-PRINTABLE", 0x00000004},
	{"7BIT",		0x00000008},
	{"8BIT",		0x00000010}
};

/** Character set value */
ValueObj pCharsetList[] = {
	{"UTF-8",			0x00000001},
	{"UTF-16",		0x00000002},
	{"ISO-8859-1",	0x00000004}
};

/** Value value */
ValueObj pValueList[] = {
	{"BINARY",			0x00000001},
	{"BOOLEAN",			0x00000002},
	{"DATE",				0x00000004},
	{"DATE-TIME",		0x00000008},
	{"FLOAT",				0x00000010},
	{"INTEGER",			0x00000020},
	{"PHONE-NUMBER",	0x00000040},
	{"TEXT",				0x00000080},
	{"TIME",				0x00000100},
	{"URI",					0x00000200},
	{"URL",					0x00000400},
	{"UTC-OFFSET",		0x00000800},
	{"VCARD",				0x00001000}
};

/** Type value */
ValueObj pTypeList[] = {
	{"AIFF",		0x00000001},
	{"BBS",			0x00000002},
	{"CAR",			0x00000004},
	{"CELL",		0x00000008},
	{"DOM",			0x00000010},
	{"WORK",		0x00000020},
	{"FAX",			0x00000040},
	{"GIF",			0x00000080},
	{"HOME",		0x00000100},
	{"INTL",		0x00000200},
	{"INTERNET",	0x00000400},
	{"ISDN",		0x00000800},
	{"JPEG",		0x00001000},
	{"MOBILE",	0x00002000},
	{"MODEM",		0x00004000},
	{"MSG",			0x00008000},
	{"PAGER",		0x00010000},
	{"PARCEL",	0x00020000},
	{"PCM",			0x00040000},
	{"PCS",			0x00080000},
	{"PNG",			0x00100000},
	{"POSTAL",	0x00200000},
	{"PREF",		0x00400000},
	{"VIDEO",		0x00800000},
	{"VOICE",		0x01000000},
	{"WAVE",		0x02000000},
	{"WBMP",		0x04000000},
	{"ETC",			0x08000000},
	{"X400",		0x10000000}
};

/** FUNCTION DECLARATION	*/
int __VCardGetName(char*, char**, int);
int __VCardGetValue(char*, const ValueObj*, int);
int __VCardGetTypeName(char*, int*, int*);
int __VCardGetParamName(char*, int*, int*);
int __VIsVcardFile(char*, int);
char* __VCardGetParamVal(char*, int*, int*);
char* __VCardGetTypeVal(char*, int*, int*, int, VObject*);
char* __VCardTypeEncode(VObject*, char*);
char* __VCardParamEncode(VObject*, int*);

#ifdef VDATA_GROUPNAME_SUPPORTED
char*	gszGroupName;
#endif // VDATA_GROUPNAME_SUPPORTED


/**
 * __VCardGetName() compares the string and vCard type, parameter name.
 *
 * @param   	szString  	Name which will compare
 * @param		pszList[]		Name list of vCard type and param
 * @param		size				Number of total element of list
 *
 * @return 	index      	The index in the list
 */
int
__VCardGetName(char* szString, char* pszList[], int size)
{
	VDATA_TRACE_BEGINE
	int high, low, i, diff;

	low = 0;
	high = size - 1;

	for (; high >= low; diff < 0 ? (low = i+1) : (high = i-1)) {
		i = (low + high) / 2;
		if ((diff = strcmp(pszList[i], szString)) == 0) /* success: found it */
			return i;
	}
	VDATA_TRACE_END
	return UNKNOWN_NAME;
}

/**
 * __VCardGetValue() compares the string and vCard type, parameter value.
 *
 * @param   	szString	Value which will compare
 * @param		list[]		Value list of vCard param
 * @param		size			Number of total element of list
 *
 * @return  	flag      The value's flag.
 */
int
__VCardGetValue(char* szString, const ValueObj list[], int size)
{
	VDATA_TRACE_BEGINE
	int i = 0, diff = -1;
	char* szTemp = szString;

	SysRequireEx(szString, UNKNOWN_NAME);
	SysRequireEx(size > 0, UNKNOWN_NAME);

	UPPER(szTemp, i, strlen(szTemp));

	for (i = 0; i < size-1; i++) {
		VDATA_TRACE(" i : %d", i);
		VDATA_TRACE(" for loop %d < %d, list[%d] : %p, list[%d].szName : %p", i, size, i, list[i], i, list[i].szName);
		VDATA_TRACE(" i : %d", i);
		if (list[i].szName != NULL) {
			VDATA_TRACE(" list[%d].szName != NULL", i);
			VDATA_TRACE(" before strcmp %s %s", list[i].szName, szTemp);
			VDATA_TRACE(" before strcmp %d", strcmp(list[i].szName, szTemp));
			if ((diff = strcmp(list[i].szName, szTemp)) == 0) { /* success: found it */
				VDATA_TRACE(" return %d", list[i].flag);
				VDATA_TRACE_END
				return list[i].flag;
			}
			VDATA_TRACE(" after strcmp %s %s", list[i].szName, szTemp);
		}
	}
	VDATA_TRACE(" return UNKNOWN_NAME");
	VDATA_TRACE_END
	return UNKNOWN_NAME;
}

/**
 * __VCardGetTypeName() fine the type name and returns the index number
 *
 * @param		pVCardRaw	The raw data
 * @param		pStatus		Decoder status
 * @param		pDLen		retrived length
 *
 * @return  	res			The index in type list
 */
int
__VCardGetTypeName(char* pVCardRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int	i, index, res;
	char	c;
	char	name[MAX_TYPE_NAME_LEN+1] = {0,};

#ifdef VDATA_GROUPNAME_SUPPORTED
	char*	szGroupName = NULL;
#endif // VDATA_GROUPNAME_SUPPORTED

	SysRequireEx(pVCardRaw, UNKNOWN_NAME);

	i = index = 0;
	res = UNKNOWN_NAME;

	while (true) {

		GO_NEXT_CHAR(c, pVCardRaw, pDLen);

		/**
		 * TYPE NAME's length is must be less than MAX_TYPE_NAME_LEN.
		 * If TYPE NAME's value is over MAX_TYPE_NAME_LEN, return UNKNOWN_NAME.
		 * And then Decoding Step shoud not be changed.
		 */
		if (index >= MAX_TYPE_NAME_LEN) {
			*pStatus = VCARD_TYPE_NAME_STATUS;
			res = UNKNOWN_NAME;
			break;
		}

		/**
		 * There is a delimeter between TYPE NAME and next element(=Param, or Type Value).
		 * If VTYPE_TOKEN_SEMICOLON or VTYPE_TOKEN_COLON is faced with,
		 * find TYPE NAME's value in pszCardTypeList, and then return searched result.
		 */
		if ((c == VTYPE_TOKEN_SEMICOLON) || (c == VTYPE_TOKEN_COLON)) {
			name[index] = 0x00;
			TRIM(name);
			UPPER(name, i, index);
			res = __VCardGetName(name, (char**)pszCardTypeList, VCARD_TYPE_NUM);
			break;
		}
		/** current version not support grouping vcard type */
		else if (c == VTYPE_TOKEN_DOT) {
#ifdef VDATA_GROUPNAME_SUPPORTED
			name[index] = '\0';
			szGroupName = (char*) calloc(1,  index+1);
			if (szGroupName != NULL) {
				strncpy(szGroupName, name, index);
				gszGroupName = szGroupName;
			}
			index = 0;
#endif
		}
		/**
		 * There is no new line in TYPE NAME.
		 * If new line character is faced with, return UNKNOWN_NAME;
		 */
		else if ((c == '\r') || (c == '\n')) {
			(*pDLen)++;
			*pStatus = VCARD_TYPE_NAME_STATUS;
			res = UNKNOWN_NAME;
			return res;
		} else if (_VIsSpace(c)) {
		} else
			name[index++] = c;
	}

	/**
	 *	Set Next Step.
	 *
	 */
	if (c == VTYPE_TOKEN_SEMICOLON)
		/**
		 * This case next token is parameter. So set VCARD_PARAM_NAME_STATUS step.
		 */
		*pStatus = VCARD_PARAM_NAME_STATUS;
	else {
		if (res != UNKNOWN_NAME)
			/**
			 * This case next string is value. So set VCARD_TYPE_VALUE_STATUS step.
			 */
			*pStatus = VCARD_TYPE_VALUE_STATUS;
		else
			/**
			 * In current step, TYPE NAME is invalid. So Try to get TYPE NAME again from next position.
			 */
			*pStatus = VCARD_TYPE_NAME_STATUS;
	}
	VDATA_TRACE_END
	return res;
}

/**
 * __VCardGetParamName() fine the param name and returns the index number
 *
 * @param		pVCardRaw	The raw data
 * @param		pStatus		Decoder status
 * @param		pDLen		retrived length
 *
 * @return  	res			The index in type list
 */
int
__VCardGetParamName(char* pVCardRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int	i, index, res;

	char	c;
	char	name[MAX_PARAM_NAME_LEN+1] = {0,};
	char* pTemp = pVCardRaw;

	SysRequireEx(pVCardRaw, UNKNOWN_NAME);

	i = index = 0;
	res = UNKNOWN_NAME;

	while (true) {
		GO_NEXT_CHAR(c, pVCardRaw, pDLen);

		/**
		 * PARAM NAME's length is must be less than MAX_PARAM_NAME_LEN.
		 * If PARAM NAME's value is over MAX_PARAM_NAME_LEN, return UNKNOWN_NAME.
		 * And then Decoding Step shoud not be changed.
		 */
		if (index >= MAX_PARAM_NAME_LEN) {
			*pStatus = VCARD_TYPE_NAME_STATUS;
			res = UNKNOWN_NAME;
			break;
		}

		/**
		 * There is a delimeter between PARAM NAME and next element(=Param, or Param Value).
		 * If VTYPE_TOKEN_EQUAL is faced with,
		 * find PARAM NAME's value in pszCardParamList, and then return searched result.
		 */
		if (c == VTYPE_TOKEN_EQUAL) {
			name[index] = '\0';
			TRIM(name);
			UPPER(name, i, index);
			res = __VCardGetName(name, (char**)pszCardParamList, VCARD_PARAM_NUM);
			if (res == UNKNOWN_NAME) {
				(*pDLen) = 0;
			}
			*pStatus = VCARD_PARAM_VALUE_STATUS;
			break;
		}
		/**
		 * This case, There is no parameter type. Only Parameter Value.
		 * In VCARD_PARAM_NAME_STATUS status, VTYPE_TOKEN_COLON means that anything parameter is no more.
		 * so set next step to VCARD_PARAM_VALUE_STATUS.
		 *
		 * Ex) TEL;WORK:+12341234
		 *        ------ ":" next is TEL TYPE's value.
		 *
		 * VCARD_PARAM_NAME_STATUS(current) -> VCARD_PARAM_VALUE_STATUS
		 * -> VCARD_TYPE_VALUE_STATUS -> MOVE TO NEXT TYPE
		 */
		else if (c == VTYPE_TOKEN_COLON) {
			*pStatus = VCARD_PARAM_VALUE_STATUS;
			pVCardRaw = pTemp;
			(*pDLen) = 0;
			res = UNKNOWN_NAME;
			break;
		}
		/**
		 * This case, There is no parameter type. Only Parameter Value.
		 * In VCARD_PARAM_NAME_STATUS status, VTYPE_TOKEN_SEMICOLON means that there is a next parameter.
		 * so set next step to VCARD_PARAM_NAME_STATUS.
		 *
		 * Ex) TEL;WORK;PREF:+12341234
		 *        ------ ":" next is TEL TYPE's value.
		 *
		 * VCARD_PARAM_NAME_STATUS(current) -> VCARD_PARAM_NAME_STATUS
		 * -> VCARD_PARAM_VALUE_STATUS -> VCARD_TYPE_VALUE_STATUS -> MOVE TO NEXT TYPE
		 */
		else if (c == VTYPE_TOKEN_SEMICOLON) {
			*pStatus = VCARD_PARAM_NAME_STATUS;
			pVCardRaw = pTemp;
			(*pDLen) = 0;
			res = UNKNOWN_NAME;
			break;
		} else if ((c == '\r') || (c == '\n') || (_VIsSpace(c))) {

		} else
			name[index++] = c;
	}
	VDATA_TRACE_END
	return res;
}

bool
__VCardFreeVTreeMemory(VTree * pTree)
{
	VDATA_TRACE_BEGINE
	VObject*		pCurObj = NULL;
	VObject*		pNextObj = NULL;

	VTree*			pCurTree = NULL;
	VTree*			pNextTree = NULL;

	VParam* pCurParam = NULL;
	VParam* pNextParam = NULL;

	int count = 0;
	int i = 0;

	SysRequireEx(pTree->treeType == VCARD, false);
	SysRequireEx(pTree != NULL, false);
	VDATA_TRACE("vcard_free_vtree_memory() entered.");

	if (pTree->treeType != VCARD) {
		VDATA_TRACE_END
		return true;
	}

	pCurTree = pTree;

	while (pCurTree) {
		pNextTree = pCurTree->pNext;
		pCurObj = pCurTree->pTop;

		while (pCurObj) {

			pNextObj = pCurObj->pSibling;
			count = pCurObj->valueCount;

			for (i = 0; i < count; i++) {
				VFREE(pCurObj->pszValue[i]);
			}

#ifdef VDATA_GROUPNAME_SUPPORTED
			if (pCurObj->pszGroupName)
				VFREE(pCurObj->pszGroupName);
#endif

			if (pCurObj->pParam) {

				pCurParam = pCurObj->pParam;

				while (pCurParam != NULL) {
					pNextParam = pCurParam->pNext;
					VDATA_TRACE("pNEXT ==> %p", pCurParam->pNext);
					VDATA_TRACE("pPARAM ==> %p", pCurParam->parameter);
					VDATA_TRACE("pVALUE ==> %p", pCurParam->paramValue);
					VDATA_TRACE("pCurParam : %p", pCurParam);
					VDATA_TRACE("pCurParam->parameter : %d", pCurParam->parameter);
					VDATA_TRACE("pCurParam->paramValue : %d", pCurParam->paramValue);
					if (pNextParam != NULL) {
						VDATA_TRACE("pNextParam : %p", pNextParam);
						VDATA_TRACE("pNextParam->parameter : %d", pNextParam->parameter);
						VDATA_TRACE("pNextParam->paramValue : %d", pNextParam->paramValue);
					}
					VFREE(pCurParam);
					pCurParam = pNextParam;
				}
			}

			VFREE(pCurObj);
			pCurObj = pNextObj;
		}

		VFREE(pCurTree);
		pCurTree = pNextTree;
	}

	VDATA_TRACE("exit vcard_free_vtree_memory");
	VDATA_TRACE_END
	return true;
}

/**
 * __VCardGetParamVal() fine the param value and returns value.
 *
 * @param		pVCardRaw	The raw data
 * @param		pStatus		Decoder status
 * @param		pDLen		retrived length
 *
 * @return  	buffer  	The result value
 */
char*
__VCardGetParamVal(char* pVCardRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int len = 0;
	char	c;
	char* pBuf = NULL;
	char* pTemp = pVCardRaw;

	SysRequireEx(pVCardRaw, NULL);

	while (true) {
		GO_NEXT_CHAR(c, pVCardRaw, pDLen);
		len++;
		switch (c) {
			case VTYPE_TOKEN_SEMICOLON:
				*pStatus = VCARD_PARAM_NAME_STATUS;
				break;
			case VTYPE_TOKEN_COLON:
				*pStatus = VCARD_TYPE_VALUE_STATUS;
				break;
			case VTYPE_TOKEN_COMMA:
				*pStatus = VCARD_PARAM_VALUE_STATUS;
				break;
		}
		if (c == VTYPE_TOKEN_SEMICOLON
				|| c == VTYPE_TOKEN_COLON
				|| c == VTYPE_TOKEN_COMMA
				|| c == 0x00)
			break;
	}

	/* if (len < 1)
		return NULL;
	*/

	pBuf = (char *)calloc(1, len);
	if (pBuf  == NULL)
		return NULL;

	memset(pBuf, 0x00, len);
	memcpy(pBuf, pTemp, len-1);
	TRIM(pBuf);
	VDATA_TRACE_END
	return pBuf;
}


/**
 * __VCardGetTypeVal() fine the type value and returns value.
 *
 * @param   	pVCardRaw  	The raw data
 * @param		status			Decoder status
 * @return 	buffer     	The result value
 */
char*
__VCardGetTypeVal(char* pVCardRaw, int* pStatus, int* pDLen, int enc, VObject* pType)
{
	VDATA_TRACE_BEGINE
	int num = 0;
	int len = 0;
	int bufferCount = 0;

	bool bEscape = false;

	char	c, c1, c2;
	char* pBuf = NULL;
	char* pTemp = pVCardRaw;
	char* pTmpBuf = NULL;
	int Status = 0;
	int Len = 0;

	SysRequireEx(pVCardRaw, NULL);

	while (true) {
		GO_NEXT_CHAR(c, pVCardRaw, pDLen);

		if (c == 0x00) break;

		len++;

		/** This case means that there are more type's value. */
		if (c == VTYPE_TOKEN_SEMICOLON && bEscape == false) {

			if ((pBuf = (char *)calloc(1, len)) == NULL) return NULL;

			memset(pBuf, 0x00, len);
			memcpy(pBuf, pTemp, len-1);

			TRIM(pBuf);
			_VUnescape(pBuf);

			*pStatus = VCARD_TYPE_VALUE_STATUS;

			/** Base 64 Decoding */
			if ((enc & pEncList[1].flag) || (enc & pEncList[0].flag)) {

				bufferCount = (len * 6 / 8) + 2;

				if ((pTmpBuf = (char *)calloc(1, bufferCount)) == NULL) {
					VFREE(pBuf);
					return NULL;
				}

				memset(pTmpBuf, 0x00, bufferCount);
				num = _VB64Decode(pTmpBuf, pBuf);

				if (pType != NULL) pType->numOfBiData = num;

				VFREE(pBuf);
				pBuf = pTmpBuf;
				pTmpBuf = NULL;
				break;
			}

			/** Quoted Printable Decoding */
			if (enc & pEncList[2].flag) {

				int i = 0, j = 0;

				while (pBuf[i]) {
					if (pBuf[i] == '\n' || pBuf[i] == '\r') {
						i++;
						if (pBuf[i] == '\n' || pBuf[i] == '\r')
							i++;

						if (pBuf[j-1] == '=') j--;
					} else
						pBuf[j++] = pBuf[i++];
				}
				pBuf[j] = '\0';

				_VQPDecode(pBuf);
				TRIM(pBuf);
				break;
			}
			break;
		}

		if (c == '\\')
			bEscape = true;
		else if (bEscape == true && c != VTYPE_TOKEN_SEMICOLON)
			bEscape = false;
		else if ((c == '\r') || (c == '\n')) {
			c2 = *(pVCardRaw-2);

			if (c2 == '=' && (enc & pEncList[2].flag)) {
				c1 = *pVCardRaw;
				if ((c1 == '\r') || (c1 == '\n')) {
					pVCardRaw += 1;
					(*pDLen) += 1;
					len++;
				}
			} else if (__VCardGetTypeName(pVCardRaw, &Status, &Len) != UNKNOWN_NAME) {
                		--len;
				if ((pBuf = (char *)calloc(1, len)) == NULL) return NULL;

				memset(pBuf, 0x00, len);
				memcpy(pBuf, pTemp, len-1);

				TRIM(pBuf);
				_VUnescape(pBuf);

				*pStatus = VCARD_TYPE_NAME_STATUS;

				c1 = *pVCardRaw;

				if ((c1 == '\r') || (c1 == '\n')) {
					pVCardRaw += 1;
					(*pDLen) += 1;
				}

				if ((enc & pEncList[1].flag) || (enc & pEncList[0].flag)) {

					bufferCount = (len * 6 / 8) + 5;

					if ((pTmpBuf = (char *)calloc(1, bufferCount)) == NULL) {
						VFREE(pBuf);
						return NULL;
					}

					memset(pTmpBuf, 0x00, bufferCount);
					num = _VB64Decode(pTmpBuf, pBuf);

					if (pType != NULL)
						pType->numOfBiData = num;

					VFREE(pBuf);
					pBuf = pTmpBuf;
					pTmpBuf = NULL;
					break;
				}

				if (enc & pEncList[2].flag) {

					int i = 0, j = 0;

					while (pBuf[i]) {
						if (pBuf[i] == '\n' || pBuf[i] == '\r') {
							i++;
							if (pBuf[i] == '\n' || pBuf[i] == '\r')
								i++;

							if (pBuf[j-1] == '=') j--;
						} else
							pBuf[j++] = pBuf[i++];
					}
					pBuf[j] = '\0';

					_VQPDecode(pBuf);
					TRIM(pBuf);
					break;
				}
				break;
			}
		}
	}
	VDATA_TRACE_END
	return pBuf;
}


int
VCardGetTypeValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VCardGetTypeValue() enter..\n");
	VDATA_TRACE_END
	return pTypeList[index].flag;
}

int
VCardGetValValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VCardGetValValue() enter..\n");
	VDATA_TRACE_END
	return pValueList[index].flag;
}

int
VCardGetEncValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VCardGetEncValue() enter..\n");
	VDATA_TRACE_END
	return pEncList[index].flag;
}

int
VCardGetCharsetValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VCardGetCharsetValue() enter..\n");
	VDATA_TRACE_END
	return pCharsetList[index].flag;
}

/*
 * vcard_decode() decode the vCard data and returns vObject struct
 *
 * @param       pVCardRaw            The raw data
 * @return      vObject             The result value
 */
SLPAPI VTree*
vcard_decode(char *pCardRaw)
{
	VDATA_TRACE_BEGINE;
	char* szValue = NULL;
	char* szCardBegin = NULL;
	char* pCardRawTmp = NULL;
	VTree* pVCard = NULL;
	VParam* pTmpParam = NULL;
	VObject* pTemp = NULL;

	char	c;

	int type, param;
	int status = VCARD_TYPE_NAME_STATUS;
	int done = false;
	int valueCount = 0;
	int len;
	int dLen = 0;
	int param_status = false;
	int numberedParam = 0;
	int enc = 0;
	//int start_status = 0;
	char* temp = NULL;

	bool vcard_ended = false;

	SysRequireEx(pCardRaw != NULL, NULL);
	len = strlen(pCardRaw);
	VDATA_TRACE("length of pCardRaw = %d", len);

	pCardRaw = _VUnfoldingNoSpecNew(pCardRaw);
	if (pCardRaw == NULL) {
		return NULL;
	}
	pCardRawTmp = pCardRaw;
	len = _VManySpace2Space(pCardRaw);

	VDATA_TRACE("ret value of _VManySpace2Space = %d", len);

	if (!__VIsVcardFile(pCardRaw, CHECK_START)) {
		VFREE(pCardRawTmp);
		VDATA_TRACE_END
		return NULL;
	}


	while (true && !done) {
		c = *pCardRaw;

		if ((c == '\0') || done)
			break;

		switch (status) {
			case VCARD_TYPE_NAME_STATUS:
				dLen = 0;
				type = __VCardGetTypeName(pCardRaw, &status, &dLen);
				pCardRaw += dLen;

				if (type == -1)
					break;

				switch (type) {
					case VCARD_TYPE_BEGIN:
						if (pVCard) {
							free(pVCard);
							pVCard = NULL;
						}

						if ((pVCard = (VTree*)calloc(1,  sizeof(VTree))) == NULL) {
							//start_status = 1;
							goto CATCH;
						}

						memset(pVCard, 0x00, sizeof(VTree));

						dLen = 0;
						szCardBegin = __VCardGetTypeVal(pCardRaw, &status, &dLen, enc, NULL);
						pCardRaw += dLen;
						VFREE(szCardBegin);

						pVCard->treeType = VCARD;
						pVCard->pTop = NULL;
						pVCard->pCur = NULL;
						pVCard->pNext = NULL;
						break;

					case VCARD_TYPE_END:
						enc = 0;
						if (strstr(pCardRaw, "VCARD") != NULL) {
							pCardRaw += dLen;
							done = true;
							vcard_ended = true;
						} else	{
							status = VCARD_TYPE_NAME_STATUS;
							pCardRaw += dLen;
							/*VFREE(etemp); */
						}
						break;

					case UNKNOWN_NAME:
						break;

					default:
						if (UNKNOWN_NAME == type || type < 0) {
							status = VCARD_TYPE_NAME_STATUS;
							break;
						}

						if ((pTemp = (VObject*)calloc(1,  sizeof(VObject))) == NULL) {
							goto CATCH;
						}

						memset(pTemp, 0, sizeof(VObject));
						pTemp->property = type;

						if (pVCard->pTop == NULL) {
							pVCard->pTop = pTemp;
							pVCard->pCur = pTemp;
						} else {
							pVCard->pCur->pSibling = pTemp;
							pVCard->pCur = pTemp;
						}

						break;
				}

				numberedParam = 0;
				param_status = false;
				valueCount = 0;

#ifdef VDATA_GROUPNAME_SUPPORTED
				if (gszGroupName != NULL)
					pVCard->pCur->pszGroupName = gszGroupName;
#endif
				break;

			case VCARD_PARAM_NAME_STATUS:
			{
				dLen = 0;
				param = __VCardGetParamName(pCardRaw, &status, &dLen);
				pCardRaw += dLen;

				if (param_status != true) {

					if ((pTmpParam = (VParam*)calloc(1,  sizeof(VParam))) == NULL)
							goto CATCH;

					param_status = true;
					pVCard->pCur->pParam = pTmpParam;
					memset(pTmpParam, 0x00, sizeof(VParam));
					VDATA_TRACE("pTmpParam : %p", pTmpParam);
				} else {
					if ((pTmpParam->pNext = (VParam*)calloc(1,  sizeof(VParam))) == NULL)
							goto CATCH;

					pTmpParam = pTmpParam->pNext;
					memset(pTmpParam, 0x00, sizeof(VParam));
					VDATA_TRACE("pTmpParam : %p", pTmpParam);
				}

				pTmpParam->parameter = param;
				break;
			}
			case VCARD_PARAM_VALUE_STATUS:
				dLen = 0;
				numberedParam = 0;
				switch (pTmpParam->parameter) {
					case VCARD_PARAM_TYPE:
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);
						numberedParam |= __VCardGetValue(szValue, pTypeList, VCARD_TYPE_PARAM_NUM);
						break;
					case VCARD_PARAM_VALUE:
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);
						numberedParam |= __VCardGetValue(szValue, pValueList, VCARD_VALUE_PARAM_NUM);
						break;
					case VCARD_PARAM_ENCODING:
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);
						numberedParam |= __VCardGetValue(szValue, pEncList, VCARD_ENCODE_PARAM_NUM);
						enc = numberedParam;
						break;
					case VCARD_PARAM_CHARSET:
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);
						numberedParam |= __VCardGetValue(szValue, pCharsetList, VCARD_CHARSET_PARAM_NUM);
						break;
					case VCARD_PARAM_CONTEXT:
					case VCARD_PARAM_LANGUAGE:
						// prevent 7605 08.03.13
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);
						numberedParam = 0;
						break;
					default:
						szValue = __VCardGetParamVal(pCardRaw, &status, &dLen);

						SET_PARAM_VALUE(numberedParam, szValue, pTypeList, VCARD_TYPE_PARAM_NUM, pTmpParam, VCARD_PARAM_TYPE, enc);
						SET_PARAM_VALUE(numberedParam, szValue, pValueList, VCARD_VALUE_PARAM_NUM, pTmpParam, VCARD_PARAM_VALUE, enc);
						SET_PARAM_VALUE(numberedParam, szValue, pEncList, VCARD_ENCODE_PARAM_NUM, pTmpParam, VCARD_PARAM_ENCODING, enc);
						SET_PARAM_VALUE(numberedParam, szValue, pCharsetList, VCARD_CHARSET_PARAM_NUM, pTmpParam, VCARD_PARAM_CHARSET, enc);

						numberedParam = 0;
						pCardRaw += dLen;
						dLen = 0;

						break;
				}

				VDATA_TRACE("%d, %s, %p", numberedParam, szValue, pTmpParam);
				pTmpParam->paramValue = numberedParam;
				pTmpParam->pNext = NULL;
				VFREE(szValue);
				pCardRaw += dLen;
				break;
			case VCARD_TYPE_VALUE_STATUS:
				dLen = 0;
				temp = __VCardGetTypeVal(pCardRaw, &status, &dLen, enc, pVCard->pCur);

				if (valueCount <= VDATA_VALUE_COUNT_MAX) {
					pVCard->pCur->pszValue[valueCount] = temp;
					valueCount++;
					pVCard->pCur->valueCount = valueCount;
				} else
					VFREE(temp);

				pCardRaw += dLen;
				break;
		}
	}

	VFREE(pCardRawTmp);

	if (pVCard->pTop == NULL)
		goto CATCH;

	if (!vcard_ended) {
		goto CATCH1;
	}
	VDATA_TRACE_END
	return pVCard;

CATCH:
	VFREE(pTemp);
CATCH1:
	VFREE(pCardRawTmp);
	__VCardFreeVTreeMemory(pVCard);
	VDATA_TRACE_END
	return NULL;
}

/*
 * vcard_encode() compares the string and vCard type, parameter value.
 *
 * @param       pVCardRaw            Data which will be encoded
 * @return      char *              Encoded result
 */
SLPAPI char*
vcard_encode(VTree *pVCardRaw)
{
	VDATA_TRACE_BEGINE
	char*		pVCardRes = NULL;
	VObject *	pTmpObj =  NULL;
	char*		pTemp = NULL;
	int			len;
	int			total = 0;
	int 		cnt = 0;
	int 		lenTypeList = 0;

	LENGTH_TYPE_LIST(pszCardTypeList, lenTypeList);

	SysRequireEx(pVCardRaw != NULL, NULL);
	SysRequireEx(pVCardRaw->pTop != NULL, NULL);
	SysRequireEx(pVCardRaw->pTop->property >= 0, NULL);
	SysRequireEx(pVCardRaw->pTop->property < lenTypeList, NULL);
	SysRequireEx(pVCardRaw->treeType == VCARD, NULL);
	SysRequireEx(pVCardRaw->pTop->valueCount > 0, NULL);

	//VDATA_TRACE("START %d %d", pVCardRaw->pTop->property, lenTypeList);

	for (; cnt < pVCardRaw->pTop->valueCount; cnt++) {

		if (pVCardRaw->pTop->pszValue[cnt] == NULL)  {
			VDATA_TRACE("pVCardRaw->pTop->valueCount : %d", pVCardRaw->pTop->valueCount);
			VDATA_TRACE("pVCardRaw->pTop->pszValue[%d] : %s", cnt, pVCardRaw->pTop->pszValue[cnt]);
			VDATA_TRACE_END
			return NULL;
		}
	}
	total += sizeof(char) * (14 + 14);
	if ((pVCardRes = (char *)calloc(1, total)) == NULL) {
		VDATA_TRACE("vcard_encode:calloc failed\n");
		VDATA_TRACE_END
		return NULL;
	}

	memcpy(pVCardRes, "BEGIN:VCARD\r\n", 14);
	g_strlcat(pVCardRes, "VERSION:2.1\r\n", total - strlen(pVCardRes));

	pTmpObj = pVCardRaw->pTop;

	while (true) {
		if (pTmpObj == NULL)
			break;

		if ((pTemp = __VCardTypeEncode(pTmpObj, pszCardTypeList[pTmpObj->property])) != NULL) {
			len = strlen(pTemp);
			total += len + sizeof(char) * 10;
			if ((pVCardRes = (char*)realloc(pVCardRes, total)) == NULL) {
				VDATA_TRACE("vcard_encode():realloc failed\n");
				VFREE(pTemp);
				pTemp = NULL;
				VDATA_TRACE_END
				return NULL;
			}

			if (strncmp(pTemp, "VERSION", strlen("VERSION")) != 0)
				g_strlcat(pVCardRes, pTemp, total - strlen(pVCardRes));

			VDATA_TRACE("pTemp : %s", pTemp);

			VFREE(pTemp);
			pTemp = NULL;
		}

		if (pTmpObj->pSibling != NULL)
			pTmpObj = pTmpObj->pSibling;
		else
			break;
	}

	total += sizeof(char) * 12;
	if ((pVCardRes = (char *)realloc(pVCardRes, total)) == NULL) {
		VDATA_TRACE("vcard_encode:realloc failed\n");
		VDATA_TRACE_END
		return NULL;
	}
	g_strlcat(pVCardRes, "END:VCARD\r\n", total - strlen(pVCardRes));
	VDATA_TRACE_END
	return pVCardRes;
}


/*
 * VIsVcardFile() verify VCard file.
 *
 * @param       pVCardRaw           Data which will be encoded
 * @return      int                 result (true or false)
 */
int
__VIsVcardFile(char *pCardRaw, int mode)
{
	int i = 0;
	bool rtnValue = true;
	char *pszVcardBegin = "BEGIN:VCARD";

	switch (mode) {
		case CHECK_START:
			for (i = 0; i < 11; i++)
				if (*pszVcardBegin++ != *pCardRaw++)
					rtnValue = false;
			break;

		default:
			rtnValue = false;
	}
	VDATA_TRACE_END
	return rtnValue;
}


/*
 * vCardTypeEncoder() compares the string and vCard type, parameter value.
 *
 * @param		typeObj				Data which will be encoded
 * @param		type				Name of the type
 * @return      char *              Encoded result
 */
char*
__VCardTypeEncode(VObject *pTypeObj, char *pType)
{
	VDATA_TRACE_BEGINE
	int			len;
	char*		pTemp = NULL;
	char*		szTypeValue = NULL;
	int			i;
	int			enc = 0;
	char*		pEncode = NULL;
	char*		pRes = NULL;
	int			total = 0;
	int			biLen = 0;

	len = strlen(pType);
	biLen = pTypeObj->numOfBiData;

#ifdef VDATA_GROUPNAME_SUPPORTED
	if (pTypeObj->pszGroupName != NULL) {
		len += strlen(pTypeObj->pszGroupName) + 1;
	}
#endif // VDATA_GROUPNAME_SUPPORTED
	if ((szTypeValue = (char *)calloc(1,  total += (len+1))) == NULL) {
		VDATA_TRACE("__VCardTypeEncode():calloc failed\n");
		VDATA_TRACE_END
		return NULL;
	}
	memset(szTypeValue, '\0', (len+1));
#ifdef VDATA_GROUPNAME_SUPPORTED
	if (pTypeObj->pszGroupName != NULL) {
		g_strlcat(szTypeValue, pTypeObj->pszGroupName, total - strlen(szTypeValue));
		g_strlcat(szTypeValue, ".", total - strlen(szTypeValue));
	}
#endif // VDATA_GROUPNAME_SUPPORTED
	g_strlcat(szTypeValue, pType, total - strlen(szTypeValue));

	pTemp = __VCardParamEncode(pTypeObj, &enc);
	if (pTemp != NULL) {
		len = strlen(pTemp);
		if ((szTypeValue = (char *)realloc(szTypeValue, (total += len))) == NULL) {
			VDATA_TRACE("__VCardTypeEncode():realloc failed\n");
			VFREE(pTemp);
			pTemp = NULL
			VDATA_TRACE_END;
			return NULL;
		}
		g_strlcat(szTypeValue, pTemp, total - strlen(szTypeValue));
		VFREE(pTemp);
		pTemp = NULL;
	}

	if ((szTypeValue = (char *)realloc(szTypeValue, (total += 2))) == NULL) {
		VDATA_TRACE_END
		return NULL;
	}

	g_strlcat(szTypeValue, ":", total - strlen(szTypeValue));

	len = 0;

	if (strcmp(pType, pszCardTypeList[19]) != 0)	{
		for (i = 0; i < pTypeObj->valueCount; i++) {

			if (pTypeObj->pszValue[i] != NULL)
				len += strlen(pTypeObj->pszValue[i]);
         }
	} else {
		len += biLen;
	}

	for (i = 0; i < pTypeObj->valueCount; i++) {

		if (i == 0) {
			if ((pEncode = (char *)calloc(1,  len+20)) == NULL) {
				VFREE(szTypeValue);
				VDATA_TRACE_END
				return NULL;
			}

			memset(pEncode, '\0', len+20);

			if (strcmp(pType, pszCardTypeList[19]) != 0)	{
				g_strlcat(pEncode, pTypeObj->pszValue[i], len+20 - strlen(pEncode));
				_VEscape(pEncode);
			} else {
				memcpy(pEncode, pTypeObj->pszValue[i], biLen);
			}
		} else {
			char	buf[1000];
			strncpy(buf, pTypeObj->pszValue[i], 999);
			_VEscape(buf);
			g_strlcat(pEncode, ";", len+20 - strlen(pEncode));
			g_strlcat(pEncode, buf, len+20 - strlen(pEncode));
		}
	}

	if (strcmp(pType, pszCardTypeList[19]) != 0)	{
		if (pEncode) {
			g_strlcat(pEncode, "\0\0", len+20 - strlen(pEncode));
			len = strlen(pEncode);
		}
	} else {
		len = biLen;
	}

	if (enc & pEncList[2].flag) {
		if ((pRes = (char *)calloc(1, len * 6 + 10)) == NULL) {
			VFREE(pEncode);
			VFREE(szTypeValue);
			VDATA_TRACE_END
			return NULL;
		}
		if (pEncode)
			_VQPEncode(pRes, pEncode);
		VFREE(pEncode);
	} else if (enc & pEncList[1].flag) {
		if ((pRes = (char *)calloc(1, (len * 8 / 6) + 4)) == NULL) {
			VFREE(pEncode);
			VFREE(szTypeValue);
			VDATA_TRACE_END
			return NULL;
		}

		memset(pRes, '\0', ((len * 8 / 6) + 4));
		_VB64Encode(pRes, pEncode, biLen);
		VFREE(pEncode);
	} else {
		if ((pRes = (char *)calloc(1, len+30)) == NULL) {
			VFREE(pEncode);
			VFREE(szTypeValue);
			VDATA_TRACE_END
			return NULL;
		}
		memset(pRes, '\0', (len + 30));
		if (pEncode) {
			memcpy(pRes, pEncode, len);
			VFREE(pEncode);
		}
	}

	if ((pRes = (char *)realloc(pRes, strlen(pRes) + 3)) == NULL) {
		VFREE(pEncode);
		VFREE(szTypeValue);
		VDATA_TRACE_END
		return NULL;
	}
	g_strlcat(pRes, "\r\n", 2);

	len = strlen(pRes);

	if ((szTypeValue = (char *)realloc(szTypeValue, (total += (len+3)))) == NULL) {
		VFREE(pEncode);
		VFREE(pRes);
		VDATA_TRACE_END
		return NULL;
	}

	g_strlcat(szTypeValue, pRes, total - strlen(szTypeValue));

	if (strcmp(pType, pszCardTypeList[19]) != 0) {
		_VRLSpace(szTypeValue);
		_VRTSpace(szTypeValue);
	}

	VFREE(pRes);
	VDATA_TRACE_END
	return szTypeValue;
}

/**
 * __VCardParamEncode() Parameter Encoding.
 *
 * @param		pTypeObj		Data which will be encoded
 * @param		pEnc				Name of the type
 */
char *
__VCardParamEncode(VObject* pTypeObj, int* pEnc)
{
	VDATA_TRACE_BEGINE
	int i = 0;
	int len = 0;
	int sNum = 0;
	int shift = 0;
	bool bSupported;
	char* szParam = NULL;
	VParam* pTemp = NULL;
	ValueObj*	pList = NULL;

	/** Paramter initialize. */
	pTemp = pTypeObj->pParam;

	/** Momory Allocation for parameter string. */
	if (pTemp != NULL) {
		if ((szParam = (char*)calloc(1, len += 2)) == NULL) {
			VDATA_TRACE_END
			return NULL;
		}
		memset(szParam, 0x00, 2);
	}

	/** appending pamaters. */
	while (true) {

		if (pTemp == NULL) break;

		bSupported = false;

		/** Expand szParam string. For appending.*/
		if ((szParam = (char *)realloc(szParam, len += 15)) == NULL) {
			VDATA_TRACE_END
			return NULL;
		}

		/** appending paramter name. */
		g_strlcat(szParam, ";", len - strlen(szParam));
		if (pTemp->parameter != VCARD_PARAM_TYPE) {
			g_strlcat(szParam, pszCardParamList[pTemp->parameter], len - strlen(szParam));
			g_strlcat(szParam, "=", len - strlen(szParam));
		}

		/** Set Parameter Value name. */
		switch (pTemp->parameter) {
			case VCARD_PARAM_ENCODING:
				*pEnc = pTemp->paramValue;
				shift = VCARD_ENCODE_PARAM_NUM;
				pList = pEncList; bSupported = true;
				break;
			case VCARD_PARAM_TYPE:
				shift = VCARD_TYPE_PARAM_NUM;
				pList = pTypeList; bSupported = true;
				break;
			case VCARD_PARAM_VALUE:
				shift = VCARD_VALUE_PARAM_NUM;
				pList = pValueList; bSupported = true;
				break;
			case VCARD_PARAM_CHARSET:
				shift = VCARD_CHARSET_PARAM_NUM;
				pList = pCharsetList; bSupported = true;
				break;
			default:
				if ((szParam = (char*)realloc(szParam, 5)) == NULL) {
					VDATA_TRACE_END
					return NULL;
				}
				g_strlcat(szParam, "NONE", 5 - strlen(szParam));
		}

		/** exchage parameter value's to string.*/
		if (bSupported == true) {

			for (i = 0, sNum = 0x00000001; i < shift; i++) {

				if (pTemp->paramValue & sNum) {
					if ((szParam = (char *)realloc(szParam, (len += (strlen(pList[i].szName) + 2)))) == NULL) {
						VDATA_TRACE_END
						return NULL;
					}

					g_strlcat(szParam, pList[i].szName, len - strlen(szParam));
					g_strlcat(szParam, "; ", len - strlen(szParam));
				}

				sNum <<= 1;
			}
		}

		/** remove semicolon from tail. */
		for (i = strlen(szParam); i > 0 ; i--) {

			if (szParam[i] == ' ' && szParam[i-1] == ';') {
				szParam[i-1] = '\0';
				break;
			}
		}

		if (pTemp->pNext != NULL)
			pTemp = pTemp->pNext;
		else
			break;
	}
	VDATA_TRACE_END
	return szParam;
}

SLPAPI bool
vcard_free_vtree_memory(VTree * pTree)
{
	VDATA_TRACE_BEGINE
	if (pTree == NULL) {
		VDATA_TRACE_END
		return false;
	}
	VDATA_TRACE_END
	return __VCardFreeVTreeMemory(pTree);
}

