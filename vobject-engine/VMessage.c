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
#include "VMessage.h"
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
	PARAM |= __VMsgGetValue(SZVALUE, LIST, LISTCNT);\
	if (PARAM != UNKNOWN_NAME) {\
		PARAMOBJ->parameter = PTYPE;\
		if (PTYPE == VMSG_PARAM_ENCODING)\
			ENC = PARAM;\
		break;\
	} \
}

#define LENGTH_TYPE_LIST(obj, len) for (len = 0; obj[len] != NULL; len++);

extern char* _VUnfoldingNoSpecNew(char *string);

/** GLOBAL VARIABLE DECLARATION AND INITIALIZATION */
/** vMsg Types. */
char* pszMsgTypeList[] = {
	"BEGIN",
	"END",
	"TEL",
	"VBODY",
	"VCARD",
	"VMSG",
	"X-BODY-CONTENTS",
	"X-BODY-SUBJECT",
	"X-IRMC-BOX",
	"X-IRMC-STATUS",
	"X-MESSAGE-TYPE",
	"X-SS-DT",
};

/** Parameter */
char* pszMsgParamList[] = {
	"CHARSET",
	"CONTEXT",
	"ENCODING",
	"LANGUAGE",
	"TYPE",
	"VALUE"
};

/** Encoding value */
ValueObj pMsgEncList[] = {
	{"B",			0x00000001},
	{"BASE64",	0x00000002},
	{"QUOTED-PRINTABLE", 0x00000004},
	{"7BIT",		0x00000008},
	{"8BIT",		0x00000010}
};

/** Character set value */
ValueObj pMsgCharsetList[] = {
	{"UTF-8",			0x00000001},
	{"UTF-16",		0x00000002},
	{"ISO-8859-1",	0x00000004}
};

/** Value value */
ValueObj pMsgValueList[] = {
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
	{"VMSG",				0x00001000}
};

/** Type value */
ValueObj pMsgTypeList[] = {
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
int __VMsgGetName(char*, char**, int);
int __VMsgGetValue(char*, const ValueObj*, int);
int __VMsgGetTypeName(char*, int*, int*);
int __VMsgGetParamName(char*, int*, int*);
int __VIsVmsgFile(char*, int);
char* __VMsgGetParamVal(char*, int*, int*);
char* __VMsgGetTypeVal(char*, int*, int*, int, VObject*);
char* __VMsgTypeEncode(VObject*, char*);
char* __VMsgParamEncode(VObject*, int*);


/**
 * __VMsgGetName() compares the string and vMsg type, parameter name.
 *
 * @param		szString		Name which will compare
 * @param		pszList[]		Name list of vMsg type and param
 * @param		size			Number of total element of list
 *
 * @return		index			The index in the list
 */
int
__VMsgGetName(char* szString, char* pszList[], int size)
{
	VDATA_TRACE_BEGINE
	int high, low, i, diff = 0;

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
 * __VMsgGetValue() compares the string and vMsg type, parameter value.
 *
 * @param		szString	Value which will compare
 * @param		list[]		Value list of vMsg param
 * @param		size		Number of total element of list
 *
 * @return		flag		The value's flag.
 */
int
__VMsgGetValue(char* szString, const ValueObj list[], int size)
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
 * __VMsgGetTypeName() fine the type name and returns the index number
 *
 * @param		pVMsgRaw	The raw data
 * @param		pStatus	Decoder status
 * @param		pDLen		retrived length
 *
 * @return		res			The index in type list
 */
int
__VMsgGetTypeName(char* pVMsgRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int	i, index, res;
	char	c;
	char	name[MAX_TYPE_NAME_LEN+1] = {0,};

	SysRequireEx(pVMsgRaw, UNKNOWN_NAME);

	i = index = 0;
	res = UNKNOWN_NAME;

	while (true) {

		GO_NEXT_CHAR(c, pVMsgRaw, pDLen);

		/**
		 * TYPE NAME's length is must be less than MAX_TYPE_NAME_LEN.
		 * If TYPE NAME's value is over MAX_TYPE_NAME_LEN, return UNKNOWN_NAME.
		 * And then Decoding Step shoud not be changed.
		 */
		if (index >= MAX_TYPE_NAME_LEN) {
			*pStatus = VMSG_TYPE_NAME_STATUS;
			res = UNKNOWN_NAME;
			break;
		}

		/**
		 * There is a delimeter between TYPE NAME and next element(=Param, or Type Value).
		 * If VTYPE_TOKEN_SEMICOLON or VTYPE_TOKEN_COLON is faced with,
		 * find TYPE NAME's value in pszMsgTypeList, and then return searched result.
		 */
		if ((c == VTYPE_TOKEN_SEMICOLON) || (c == VTYPE_TOKEN_COLON)) {
			name[index] = 0x00;
			TRIM(name);
			UPPER(name, i, index);
			res = __VMsgGetName(name, (char**)pszMsgTypeList, VMSG_TYPE_NUM);
			break;
		}
		/**
		 * There is no new line in TYPE NAME.
		 * If new line character is faced with, return UNKNOWN_NAME;
		 */
		else if ((c == '\r') || (c == '\n')) {
			(*pDLen)++;
			*pStatus = VMSG_TYPE_NAME_STATUS;
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
		 * This case next token is parameter. So set VMSG_PARAM_NAME_STATUS step.
		 */
		*pStatus = VMSG_PARAM_NAME_STATUS;
	else {
		if (res != UNKNOWN_NAME)
			/**
			 * This case next string is value. So set VMSG_TYPE_VALUE_STATUS step.
			 */
			*pStatus = VMSG_TYPE_VALUE_STATUS;
		else
			/**
			 * In current step, TYPE NAME is invalid. So Try to get TYPE NAME again from next position.
			 */
			*pStatus = VMSG_TYPE_NAME_STATUS;
	}
	VDATA_TRACE_END
	return res;
}

/**
 * __VMsgGetParamName() fine the param name and returns the index number
 *
 * @param		pVMsgRaw	The raw data
 * @param		pStatus	Decoder status
 * @param		pDLen		retrived length
 *
 * @return		res			The index in type list
 */
int
__VMsgGetParamName(char* pVMsgRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int	i, index, res;

	char	c;
	char	name[MAX_PARAM_NAME_LEN+1] = {0,};
	char* pTemp = pVMsgRaw;

	SysRequireEx(pVMsgRaw, UNKNOWN_NAME);

	i = index = 0;
	res = UNKNOWN_NAME;

	while (true) {
		GO_NEXT_CHAR(c, pVMsgRaw, pDLen);

		/**
		 * PARAM NAME's length is must be less than MAX_PARAM_NAME_LEN.
		 * If PARAM NAME's value is over MAX_PARAM_NAME_LEN, return UNKNOWN_NAME.
		 * And then Decoding Step shoud not be changed.
		 */
		if (index >= MAX_PARAM_NAME_LEN) {
			*pStatus = VMSG_TYPE_NAME_STATUS;
			res = UNKNOWN_NAME;
			break;
		}

		/**
		 * There is a delimeter between PARAM NAME and next element(=Param, or Param Value).
		 * If VTYPE_TOKEN_EQUAL is faced with,
		 * find PARAM NAME's value in pszMsgParamList, and then return searched result.
		 */
		if (c == VTYPE_TOKEN_EQUAL) {
			name[index] = '\0';
			TRIM(name);
			UPPER(name, i, index);
			res = __VMsgGetName(name, (char**)pszMsgParamList, VMSG_PARAM_NUM);
			if (res == UNKNOWN_NAME)
				(*pDLen) = 0;
			*pStatus = VMSG_PARAM_VALUE_STATUS;
			break;
		}
		/**
		 * This case, There is no parameter type. Only Parameter Value.
		 * In VMSG_PARAM_NAME_STATUS status, VTYPE_TOKEN_COLON means that anything parameter is no more.
		 * so set next step to VMSG_PARAM_VALUE_STATUS.
		 *
		 * Ex) TEL;WORK:+12341234
		 *        ------ ":" next is TEL TYPE's value.
		 *
		 * VMSG_PARAM_NAME_STATUS(current) -> VMSG_PARAM_VALUE_STATUS
		 * -> VMSG_TYPE_VALUE_STATUS -> MOVE TO NEXT TYPE
		 */
		else if (c == VTYPE_TOKEN_COLON) {
			*pStatus = VMSG_PARAM_VALUE_STATUS;
			pVMsgRaw = pTemp;
			(*pDLen) = 0;
			res = UNKNOWN_NAME;
			break;
		}
		/**
		 * This case, There is no parameter type. Only Parameter Value.
		 * In VMSG_PARAM_NAME_STATUS status, VTYPE_TOKEN_SEMICOLON means that there is a next parameter.
		 * so set next step to VMSG_PARAM_NAME_STATUS.
		 *
		 * Ex) TEL;WORK;PREF:+12341234
		 *        ------ ":" next is TEL TYPE's value.
		 *
		 * VMSG_PARAM_NAME_STATUS(current) -> VMSG_PARAM_NAME_STATUS
		 * -> VMSG_PARAM_VALUE_STATUS -> VMSG_TYPE_VALUE_STATUS -> MOVE TO NEXT TYPE
		 */
		else if (c == VTYPE_TOKEN_SEMICOLON) {
			*pStatus = VMSG_PARAM_NAME_STATUS;
			pVMsgRaw = pTemp;
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
__VMsgFreeVTreeMemory(VTree * pTree)
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

	SysRequireEx(pTree->treeType == VMESSAGE, false);
	SysRequireEx(pTree != NULL, false);
	VDATA_TRACE("vmsg_free_vtree_memory() entered.");

	if (pTree->treeType != VMESSAGE) {
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

			for (i = 0; i < count; i++)
				VFREE(pCurObj->pszValue[i]);

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

	VDATA_TRACE("exit vmsg_free_vtree_memory");
	VDATA_TRACE_END
	return true;
}

/**
 * __VMsgGetParamVal() fine the param value and returns value.
 *
 * @param		pVMsgRaw	The raw data
 * @param		pStatus	Decoder status
 * @param		pDLen		retrived length
 *
 * @return		buffer		The result value
 */
char*
__VMsgGetParamVal(char* pVMsgRaw, int* pStatus, int* pDLen)
{
	VDATA_TRACE_BEGINE
	int len = 0;
	char	c;
	char* pBuf = NULL;
	char* pTemp = pVMsgRaw;

	SysRequireEx(pVMsgRaw, NULL);

	while (true) {
		GO_NEXT_CHAR(c, pVMsgRaw, pDLen);
		len++;
		switch (c) {
		case VTYPE_TOKEN_SEMICOLON:
			*pStatus = VMSG_PARAM_NAME_STATUS;
			break;
		case VTYPE_TOKEN_COLON:
			*pStatus = VMSG_TYPE_VALUE_STATUS;
			break;
		case VTYPE_TOKEN_COMMA:
			*pStatus = VMSG_PARAM_VALUE_STATUS;
			break;
		}
		if (c == VTYPE_TOKEN_SEMICOLON
				|| c == VTYPE_TOKEN_COLON
				|| c == VTYPE_TOKEN_COMMA
				|| c == 0x00)
			break;
	}

	if (len < 1 || (pBuf = (char *)calloc(1, len)) == NULL)
		return NULL;
	memset(pBuf, 0x00, len);
	memcpy(pBuf, pTemp, len-1);
	TRIM(pBuf);
	VDATA_TRACE_END
	return pBuf;
}


/**
 * __VMsgGetTypeVal() fine the type value and returns value.
 *
 * @param		pVMsgRaw		The raw data
 * @param		status			Decoder status
 * @return		buffer			The result value
 */
char*
__VMsgGetTypeVal(char* pVMsgRaw, int* pStatus, int* pDLen, int enc, VObject* pType)
{
	VDATA_TRACE_BEGINE
	int num = 0;
	int len = 0;
	int bufferCount = 0;

	bool bEscape = false;

	char	c, c1, c2;
	char* pBuf = NULL;
	char* pTemp = pVMsgRaw;
	char* pTmpBuf = NULL;
	int Status = 0;
	int Len = 0;

	SysRequireEx(pVMsgRaw, NULL);
	VDATA_TRACE("pVMsgRaw: %s", pVMsgRaw);
	while (true) {
		GO_NEXT_CHAR(c, pVMsgRaw, pDLen);

		if (c == 0x00)
			break;

		len++;

		/** This case means that there are more type's value. */
		if (c == VTYPE_TOKEN_SEMICOLON && bEscape == false) {
			if ((pBuf = (char *)calloc(1, len)) == NULL)
				return NULL;

			memset(pBuf, 0x00, len);
			memcpy(pBuf, pTemp, len-1);

			TRIM(pBuf);
			_VUnescape(pBuf);

			*pStatus = VMSG_TYPE_VALUE_STATUS;

			/** Base 64 Decoding */
			if ((enc & pMsgEncList[1].flag) || (enc & pMsgEncList[0].flag)) {

				bufferCount = (len * 6 / 8) + 2;

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

			/** Quoted Printable Decoding */
			if (enc & pMsgEncList[2].flag) {

				int i = 0, j = 0;

				while (pBuf[i]) {
					if (pBuf[i] == '\n' || pBuf[i] == '\r') {
						i++;
						if (pBuf[i] == '\n' || pBuf[i] == '\r')
							i++;

						if (pBuf[j-1] == '=')
							j--;
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
			c2 = *(pVMsgRaw-2);

			if (c2 == '=' && (enc & pMsgEncList[2].flag)) {
				c1 = *pVMsgRaw;
				if ((c1 == '\r') || (c1 == '\n')) {
					pVMsgRaw += 1;
					(*pDLen) += 1;
					len++;
				}
			} else if (__VMsgGetTypeName(pVMsgRaw, &Status, &Len) != UNKNOWN_NAME) {
				if ((pBuf = (char *)calloc(1, len)) == NULL)
					return NULL;

				memset(pBuf, 0x00, len);
				memcpy(pBuf, pTemp, len-1);

				TRIM(pBuf);
				_VUnescape(pBuf);
				VDATA_TRACE("pVMsgRaw: %s", pVMsgRaw);
				*pStatus = VMSG_TYPE_NAME_STATUS;

				c1 = *pVMsgRaw;

				if ((c1 == '\r') || (c1 == '\n')) {
					pVMsgRaw += 1;
					(*pDLen) += 1;
				}

				if ((enc & pMsgEncList[1].flag) || (enc & pMsgEncList[0].flag)) {

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

				if (enc & pMsgEncList[2].flag) {

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
VMsgGetTypeValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VMsgGetTypeValue() enter..\n");
	VDATA_TRACE_END
	return pMsgTypeList[index].flag;
}

int
VMsgGetValValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VMsgGetValValue() enter..\n");
	VDATA_TRACE_END
	return pMsgValueList[index].flag;
}

int
VMsgGetEncValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VMsgGetEncValue() enter..\n");
	VDATA_TRACE_END
	return pMsgEncList[index].flag;
}

int
VMsgGetCharsetValue(int index)
{
	VDATA_TRACE_BEGINE
	VDATA_TRACE("VMsgGetCharsetValue() enter..\n");
	VDATA_TRACE_END
	return pMsgCharsetList[index].flag;
}

/*
 * vmsg_decode() decode the vMsg data and returns vObject struct
 *
 * @param       pVMsgRaw            The raw data
 * @return      vObject             The result value
 */
VTree* vmsg_decode(char *pMsgRaw)
{
	VDATA_TRACE_BEGINE;

	char* szValue = NULL;
	char* szMsgBegin = NULL;
	char* pMsgRawTmp = NULL;
	VTree* pVMsg = NULL;
	VTree* pVBody = NULL;
	VTree* pCurrent = NULL;
	VTree* pVCard = NULL;
	VParam* pTmpParam = NULL;
	VObject* pTemp = NULL;

	char	c;

	int type, param;
	int status = VMSG_TYPE_NAME_STATUS;
	int done = false;
	int valueCount = 0;
	int len;
	int dLen = 0;
	int param_status = false;
	int numberedParam = 0;
	int enc = 0;
	int start_status = 0;
	char* temp = NULL;
	char* pVTree = NULL;
	bool vmsg_ended = false;

	SysRequireEx(pMsgRaw != NULL, NULL);
	len = strlen(pMsgRaw);
	VDATA_TRACE("length of pCardRaw = %d", len);

	len = _VUnfoldingNoSpec(pMsgRaw, VMESSAGE);
	pMsgRawTmp = pMsgRaw;
	len = _VManySpace2Space(pMsgRaw);

	VDATA_TRACE("ret value of _VManySpace2Space = %d", len);

	if (!__VIsVmsgFile(pMsgRaw, CHECK_START)) {
		VFREE(pMsgRawTmp);
		VDATA_TRACE_END
		return NULL;
	}

	while (true && !done) {
		c = *pMsgRaw;

		if ((c == '\0') || done)
			break;

		switch (status) {
		case VMSG_TYPE_NAME_STATUS:
			dLen = 0;
			type = __VMsgGetTypeName(pMsgRaw, &status, &dLen);

			if (type == VMSG_TYPE_BEGIN)
				pVTree = pMsgRaw;

			pMsgRaw += dLen;

			if (type == -1)
				break;

			switch (type) {
			case VMSG_TYPE_BEGIN:
				dLen = 0;
				enc = 0;
				szMsgBegin = __VMsgGetTypeVal(pMsgRaw, &status, &dLen, enc, NULL);
				pMsgRaw += dLen;

				if (szMsgBegin == NULL)
					goto CATCH;

				if (!strncmp(szMsgBegin, "VCARD", strlen("VCARD"))) {
					VDATA_TRACE("pVTree: %s", pVTree);
					pVCard = vcard_decode(pVTree);
					pCurrent->pNext = pVCard;
					pCurrent = pVCard;

					dLen = ((strstr(pMsgRaw, "END:VCARD") + 9) - pMsgRaw);
					pMsgRaw += dLen;
				} else {

					if (start_status == 1)
						goto CATCH;

					if (!strncmp(szMsgBegin, "VMSG", strlen("VMSG"))) {
						if ((pVMsg = (VTree*)calloc(1, sizeof(VTree))) == NULL) {
							start_status = 1;
							goto CATCH;
						}
						memset(pVMsg, 0x00, sizeof(VTree));

						pVMsg->treeType = VMESSAGE;
						pVMsg->pTop = NULL;
						pVMsg->pCur = NULL;
						pVMsg->pNext = NULL;
						pCurrent = pVMsg;
					} else if (!strncmp(szMsgBegin, "VBODY", strlen("VBODY"))) {
						if ((pVBody = (VTree*)calloc(1, sizeof(VTree))) == NULL) {
							start_status = 1;
							goto CATCH;
						}

						memset(pVBody, 0x00, sizeof(VTree));
						pVBody->treeType = VBODY;
						pVBody->pTop = NULL;
						pVBody->pCur = NULL;
						pVBody->pNext = NULL;
						pCurrent->pNext = pVBody;
						pCurrent = pVBody;
					}
				}

				VFREE(szMsgBegin);
				break;

			case VMSG_TYPE_END:
				dLen = 0;
				enc = 0;
				/* szMsgBegin = __VMsgGetTypeVal(pMsgRaw, &status, &dLen, enc, NULL); */

				if (!strncmp(pMsgRaw, "VMSG", strlen("VMSG"))) {
					done = true;
					vmsg_ended = true;
				} else {
					status = VMSG_TYPE_NAME_STATUS;
					/* pMsgRaw += dLen; */
				}
				VDATA_TRACE("pMsgRaw:%s", pMsgRaw);
				pMsgRaw += dLen;
				break;

			case UNKNOWN_NAME:
				break;

			default:
				if (UNKNOWN_NAME == type || type < 0) {
					status = VMSG_TYPE_NAME_STATUS;
					break;
				}

				if ((pTemp = (VObject*)calloc(1, sizeof(VObject))) == NULL)
					goto CATCH;

				memset(pTemp, 0, sizeof(VObject));
				pTemp->property = type;

				if (pCurrent->pTop == NULL) {
					pCurrent->pTop = pTemp;
					pCurrent->pCur = pTemp;
				} else {
					pCurrent->pCur->pSibling = pTemp;
					pCurrent->pCur = pTemp;
				}

				break;
			}

			numberedParam = 0;
			param_status = false;
			valueCount = 0;
			break;

		case VMSG_PARAM_NAME_STATUS:
		{
			dLen = 0;
			param = __VMsgGetParamName(pMsgRaw, &status, &dLen);
			pMsgRaw += dLen;

			if (param_status != true) {

				if ((pTmpParam = (VParam*)calloc(1, sizeof(VParam))) == NULL)
					goto CATCH;

				param_status = true;
				pCurrent->pCur->pParam = pTmpParam;
				memset(pTmpParam, 0x00, sizeof(VParam));
				VDATA_TRACE("pTmpParam : %p", pTmpParam);
			} else {
				if ((pTmpParam->pNext = (VParam*)calloc(1, sizeof(VParam))) == NULL)
					goto CATCH;

				pTmpParam = pTmpParam->pNext;
				memset(pTmpParam, 0x00, sizeof(VParam));
				VDATA_TRACE("pTmpParam : %p", pTmpParam);
			}

			pTmpParam->parameter = param;
			break;
		}
		case VMSG_PARAM_VALUE_STATUS:
			dLen = 0;
			numberedParam = 0;
			switch (pTmpParam->parameter) {
			case VMSG_PARAM_TYPE:
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);
				numberedParam |= __VMsgGetValue(szValue, pMsgTypeList, VMSG_TYPE_PARAM_NUM);
				break;
			case VMSG_PARAM_VALUE:
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);
				numberedParam |= __VMsgGetValue(szValue, pMsgValueList, VMSG_VALUE_PARAM_NUM);
				break;
			case VMSG_PARAM_ENCODING:
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);
				numberedParam |= __VMsgGetValue(szValue, pMsgEncList, VMSG_ENCODE_PARAM_NUM);
				enc = numberedParam;
				break;
			case VMSG_PARAM_CHARSET:
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);
				numberedParam |= __VMsgGetValue(szValue, pMsgCharsetList, VMSG_CHARSET_PARAM_NUM);
				break;
			case VMSG_PARAM_CONTEXT:
			case VMSG_PARAM_LANGUAGE:
				/* prevent 7605 08.03.13 */
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);
				numberedParam = 0;
				break;
			default:
				szValue = __VMsgGetParamVal(pMsgRaw, &status, &dLen);

				SET_PARAM_VALUE(numberedParam, szValue, pMsgTypeList, VMSG_TYPE_PARAM_NUM, pTmpParam, VMSG_PARAM_TYPE, enc);
				SET_PARAM_VALUE(numberedParam, szValue, pMsgValueList, VMSG_VALUE_PARAM_NUM, pTmpParam, VMSG_PARAM_VALUE, enc);
				SET_PARAM_VALUE(numberedParam, szValue, pMsgEncList, VMSG_ENCODE_PARAM_NUM, pTmpParam, VMSG_PARAM_ENCODING, enc);
				SET_PARAM_VALUE(numberedParam, szValue, pMsgCharsetList, VMSG_CHARSET_PARAM_NUM, pTmpParam, VMSG_PARAM_CHARSET, enc);

				numberedParam = 0;
				pMsgRaw += dLen;
				dLen = 0;

				break;
			}

			VDATA_TRACE("%d, %s, %p", numberedParam, szValue, pTmpParam);
			pTmpParam->paramValue = numberedParam;
			pTmpParam->pNext = NULL;
			VFREE(szValue);
			pMsgRaw += dLen;
			break;
		case VMSG_TYPE_VALUE_STATUS:
			dLen = 0;
			temp = __VMsgGetTypeVal(pMsgRaw, &status, &dLen, enc, pCurrent->pCur);

			if (valueCount < VDATA_VALUE_COUNT_MAX) {
				pCurrent->pCur->pszValue[valueCount] = temp;
				valueCount++;
				pCurrent->pCur->valueCount = valueCount;
				VDATA_TRACE("object property: %d, value: %s", pCurrent->pCur->property, pCurrent->pCur->pszValue[valueCount - 1]);
			} else
				VFREE(temp);

			pMsgRaw += dLen;
			break;
		}
	}
	VDATA_TRACE("pMsgRawTmp: %s", pMsgRawTmp);
	/* VFREE(pMsgRawTmp); */

	if (pVMsg->pTop == NULL) {
		VDATA_TRACE("pVMsg->Top: NULL");
		goto CATCH;
	}

	if (!vmsg_ended) {
		VDATA_TRACE("vmsg_ended: false");
		goto CATCH1;
	}
	VDATA_TRACE_END
	return pVMsg;

CATCH:
	VFREE(pTemp);
CATCH1:
	VFREE(szMsgBegin);
	VFREE(pMsgRawTmp);
	vmsg_free_vtree_memory(pVMsg);
	VDATA_TRACE_END
	return NULL;
}

/*
 * vmsg_encode() compares the string and vMsg type, parameter value.
 *
 * @param       pVMsgRaw            Data which will be encoded
 * @return      char *              Encoded result
 */
char* vmsg_encode(VTree *pVMsgRaw)
{
	VDATA_TRACE_BEGINE
	char*		pVMsgRes = NULL;
	char*		pTmpVMsgRes = NULL;
	VTree*		pTmpTree = NULL;
	VObject *	pTmpObj =  NULL;
	char*		pTemp = NULL;
	int			len;
	int			total = 0;
	int			cnt = 0;

	for (; cnt < pVMsgRaw->pTop->valueCount; cnt++) {

		if (pVMsgRaw->pTop->pszValue[cnt] == NULL)  {
			VDATA_TRACE("pVMsgRaw->pTop->valueCount : %d", pVMsgRaw->pTop->valueCount);
			VDATA_TRACE("pVMsgRaw->pTop->pszValue[%d] : %s", cnt, pVMsgRaw->pTop->pszValue[cnt]);
			VDATA_TRACE_END
			return NULL;
		}
	}

	pTmpTree = pVMsgRaw;
	pTmpObj = pVMsgRaw->pTop;

	while (true) {
		switch (pTmpTree->treeType) {
		case VMESSAGE:
			if (pVMsgRes) {
				free(pVMsgRes);
				pVMsgRes = NULL;
			}

			if ((pVMsgRes = (char *)calloc(1, sizeof(char) * (total += 13))) == NULL) {
				VDATA_TRACE("vmsg_encode:calloc failed\n");
				VDATA_TRACE_END
				return NULL;
			}
			memcpy(pVMsgRes, "BEGIN:VMSG\r\n", 13);
			break;

		case VBODY:
			if ((pTmpVMsgRes = (char *)realloc(pVMsgRes,  sizeof(char) * (total += 14))) == NULL) {
				VDATA_TRACE("vmsg_encode:realloc failed\n");
				VFREE(pVMsgRes);
				VDATA_TRACE_END
				return NULL;
			}

			pVMsgRes = pTmpVMsgRes;
			g_strlcat(pVMsgRes, "BEGIN:VBODY\r\n", 13);
			break;

		case VCARD:
			break;
		}

		while (true) {
			if (pTmpObj == NULL)
				break;

			if ((pTemp = __VMsgTypeEncode(pTmpObj, pTmpObj->property == VCARD_TYPE_TEL ? "TEL" : pszMsgTypeList[pTmpObj->property])) != NULL) {
				if (pTmpTree->treeType == VCARD) {
					char* encoded  = NULL;

					encoded = vcard_encode(pTmpTree);
					if (encoded == NULL) {
						VDATA_TRACE("vcard_encode() failed\n");
						VFREE(pTemp);
						VFREE(pVMsgRes);
						VDATA_TRACE_END
						return NULL;
					}

					len = strlen(encoded);

					if ((pTmpVMsgRes = (char*)realloc(pVMsgRes, (total += len+10))) == NULL) {
						VDATA_TRACE("vmsg_encode():realloc failed\n");
						VFREE(pTemp);
						VFREE(encoded);
						VFREE(pVMsgRes);
						VDATA_TRACE_END
						return NULL;
					}

					pVMsgRes = pTmpVMsgRes;
					g_strlcat(pVMsgRes, encoded, len+10);
					VDATA_TRACE("pTemp : %s", encoded);
					VFREE(pTemp);
					VFREE(encoded);
					break;
				}  else {
					len = strlen(pTemp);

					if ((pTmpVMsgRes = (char*)realloc(pVMsgRes, (total += len+10))) == NULL) {
						VDATA_TRACE("vmsg_encode():realloc failed\n");
						VFREE(pTemp);
						VFREE(pVMsgRes);
						VDATA_TRACE_END
						return NULL;
					}
					pVMsgRes = pTmpVMsgRes;
					g_strlcat(pVMsgRes, pTemp, len+10);
					VDATA_TRACE("pTemp : %s", pTemp);
					VFREE(pTemp);

					if (pTmpObj->pSibling != NULL)
						pTmpObj = pTmpObj->pSibling;
					else
						break;
				}
			}


		}

		switch (pTmpTree->treeType) {
		case VBODY:
			if ((pTmpVMsgRes = (char *)realloc(pVMsgRes, (total += 12))) == NULL) {
				VFREE(pVMsgRes);
				VDATA_TRACE("vcal_encode():realloc failed\n");
				return NULL;
			}

			pVMsgRes = pTmpVMsgRes;
			g_strlcat(pVMsgRes, "END:VBODY\r\n", 12);
			break;

		case VCARD:
			break;

		case VMESSAGE:
			break;

		}

		if (pTmpTree->pNext != NULL)
			pTmpTree = pTmpTree->pNext;
		else
			break;
		pTmpObj = pTmpTree->pTop;
	}

	if ((pTmpVMsgRes = (char *)realloc(pVMsgRes, (total += 11))) == NULL) {
		VDATA_TRACE("vmsg_encode:realloc failed\n");
		VFREE(pVMsgRes);
		VDATA_TRACE_END
		return NULL;
	}

	pVMsgRes = pTmpVMsgRes;
	g_strlcat(pVMsgRes, "END:VMSG\r\n", 11);
	VDATA_TRACE_END
	return pVMsgRes;
}


/*
 * VIsVmsgFile() verify VMsg file.
 *
 * @param       pVMsgRaw           Data which will be encoded
 * @return      int                 result (true or false)
 */
int
__VIsVmsgFile(char *pMsgRaw, int mode)
{
	int i = 0;
	bool rtnValue = true;
	char *pszVmsgBegin = "BEGIN:VMSG";

	switch (mode) {
	case CHECK_START:
		for (i = 0; i < 10; i++)
			if (*pszVmsgBegin++ != *pMsgRaw++)
				rtnValue = false;
		break;

	default:
		rtnValue = false;
	}
	VDATA_TRACE_END
	return rtnValue;
}


/*
 * vMsgTypeEncoder() compares the string and vMsg type, parameter value.
 *
 * @param		typeObj				Data which will be encoded
 * @param		type				Name of the type
 * @return      char *              Encoded result
 */
char*
__VMsgTypeEncode(VObject *pTypeObj, char *pType)
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

	if ((szTypeValue = (char *)calloc(1, total += (len+1))) == NULL) {
		VDATA_TRACE("__VMsgTypeEncode():calloc failed\n");
		VDATA_TRACE_END
		return NULL;
	}
	memset(szTypeValue, '\0', (len+1));
	g_strlcat(szTypeValue, pType, len+1);

	pTemp = __VMsgParamEncode(pTypeObj, &enc);
	if (pTemp != NULL) {
		len = strlen(pTemp);
		if ((szTypeValue = (char *)realloc(szTypeValue, (total += len))) == NULL) {
			VDATA_TRACE("__VMsgTypeEncode():realloc failed\n");
			VFREE(pTemp);
			pTemp = NULL
			VDATA_TRACE_END;
			return NULL;
		}
		g_strlcat(szTypeValue, pTemp, len);
		VFREE(pTemp);
		pTemp = NULL;
	}

	if ((szTypeValue = (char *)realloc(szTypeValue, (total += 2))) == NULL) {
		VDATA_TRACE_END
		return NULL;
	}

	g_strlcat(szTypeValue, ":", 2);

	len = 0;

	if (strcmp(pType, pszMsgTypeList[6]) != 0) {
		for (i = 0; i < pTypeObj->valueCount; i++) {

			if (pTypeObj->pszValue[i] != NULL)
				len += strlen(pTypeObj->pszValue[i]);
		}
	} else
		len += biLen;


	for (i = 0; i < pTypeObj->valueCount; i++) {

		if (i == 0) {
			if ((pEncode = (char *)calloc(1, len+20)) == NULL) {
				VFREE(szTypeValue);
				VDATA_TRACE_END
				return NULL;
			}

			memset(pEncode, '\0', len+20);

			if (strcmp(pType, pszMsgTypeList[6]) != 0) {
				g_strlcat(pEncode, pTypeObj->pszValue[i], 20);
				_VEscape(pEncode);
			} else
				memcpy(pEncode, pTypeObj->pszValue[i], biLen);
		} else {
			char	buf[1000];
			strncpy(buf, pTypeObj->pszValue[i], 999);
			_VEscape(buf);
			g_strlcat(pEncode, ";" , len+20);
			g_strlcat(pEncode, buf , len+20);
		}
	}

	if (strcmp(pType, pszMsgTypeList[6]) != 0)	{
		if (pEncode) {
			g_strlcat(pEncode, "\0\0", 2);
			len = strlen(pEncode);
		}
	} else {
		len = biLen;
	}

	if (enc & pMsgEncList[2].flag) {
		if ((pRes = (char *)calloc(1, len * 6 + 10)) == NULL) {
			VFREE(pEncode);
			VFREE(szTypeValue);
			VDATA_TRACE_END
			return NULL;
		}
		if (pEncode)
			_VQPEncode(pRes, pEncode);
		VFREE(pEncode);
	} else if (enc & pMsgEncList[1].flag) {
		if ((pRes = (char *)calloc(1, (len * 8 / 6) + 48)) == NULL) {
			VFREE(pEncode);
			VFREE(szTypeValue);
			VDATA_TRACE_END
			return NULL;
		}

		memset(pRes, '\0', ((len * 8 / 6) + 48));
		_VB64Encode(pRes, pEncode, biLen);
		VDATA_TRACE("Origin Size: %d, Allocated Size %d, Coverted Size: %d\n", biLen, (len * 8 / 6) + 48, strlen(pRes));
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
	g_strlcat(pRes, "\r\n", strlen(pRes) + 2);

	len = strlen(pRes);

	if ((szTypeValue = (char *)realloc(szTypeValue, (total += (len+3)))) == NULL) {
		VFREE(pEncode);
		VFREE(pRes);
		VDATA_TRACE_END
		return NULL;
	}

	g_strlcat(szTypeValue, pRes, total - 1);

	if (strcmp(pType, pszMsgTypeList[6]) != 0) {
		_VRLSpace(szTypeValue);
		_VRTSpace(szTypeValue);
	}

	VFREE(pRes);

	VDATA_TRACE_END
	return szTypeValue;
}

/**
 * __VMsgParamEncode() Parameter Encoding.
 *
 * @param		pTypeObj		Data which will be encoded
 * @param		pEnc				Name of the type
 */
char *
__VMsgParamEncode(VObject* pTypeObj, int* pEnc)
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
		g_strlcat(szParam, ";" , len);
		if (pTemp->parameter != VMSG_PARAM_TYPE) {
			g_strlcat(szParam, pszMsgParamList[pTemp->parameter], len);
			g_strlcat(szParam, "=", len);
		}

		/** Set Parameter Value name. */
		switch (pTemp->parameter) {
		case VMSG_PARAM_ENCODING:
			*pEnc = pMsgEncList[pTemp->paramValue].flag;
			shift = VMSG_ENCODE_PARAM_NUM;
			pList = pMsgEncList; bSupported = true;
			break;
		case VMSG_PARAM_TYPE:
			shift = VMSG_TYPE_PARAM_NUM;
			pList = pMsgTypeList; bSupported = true;
			break;
		case VMSG_PARAM_VALUE:
			shift = VMSG_VALUE_PARAM_NUM;
			pList = pMsgValueList; bSupported = true;
			break;
		case VMSG_PARAM_CHARSET:
			shift = VMSG_CHARSET_PARAM_NUM;
			pList = pMsgCharsetList; bSupported = true;
			break;
		default:
			if ((szParam = (char*)realloc(szParam, 5)) == NULL) {
				VDATA_TRACE_END
				return NULL;
			}
			g_strlcat(szParam, "NONE", strlen("NONE"));
		}

		/** exchage parameter value's to string.*/
		if (bSupported == true) {

			for (i = 0, sNum = 0x00000001; i < shift; i++) {

				if (pList[pTemp->paramValue].flag & sNum) {
					if ((szParam = (char *)realloc(szParam, (len += (strlen(pList[i].szName) + 2)))) == NULL) {
						VDATA_TRACE_END
						return NULL;
					}

					g_strlcat(szParam, pList[i].szName, len);
					g_strlcat(szParam, "; ", len);
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

bool
vmsg_free_vtree_memory(VTree * pTree)
{
	VDATA_TRACE_BEGINE
	if (pTree == NULL) {
		VDATA_TRACE_END
		return false;
	}
	VDATA_TRACE_END
	return __VMsgFreeVTreeMemory(pTree);
}
