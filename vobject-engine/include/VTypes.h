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

#ifndef _VTYPES_H
#define _VTYPES_H

#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#ifndef SLPAPI
#define SLPAPI __attribute__ ((visibility("default")))
#endif

#define CR				0x0d
#define LF				0x0a
#define TAB				0x09
#define WSP				0x20
#define UNKNOWN_NAME 	0x80000000

#define VCARD			0x01
#define VCALENDAR		0x02
#define VNOTE 0x0b

#define VEVENT			0x03
#define VTODO			0x04
#define VJOURNAL		0x05
#define VFREEBUSY		0x06
#define VTIMEZONE		0x07
#define VALARM			0x08
#define VMESSAGE		0x0c
#define VBODY			0X0d
#define STANDARD		0x09
#define DAYLIGHT		0x0a

#define VTYPE_TOKEN_SEMICOLON		';'
#define VTYPE_TOKEN_COLON			':'
#define VTYPE_TOKEN_EQUAL			'='
#define VTYPE_TOKEN_COMMA			','
#define VTYPE_TOKEN_DOT				'.'
#define VTYPE_TOKEN_QUOTE			'\''
#define VTYPE_TOKEN_DBLQUOTE		'"'

#define VDATA_VALUE_COUNT_MAX	2000

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/****************************************************************************************************/
/*									 GLOBAL STRUCTURE DECLARATION									*/
/****************************************************************************************************/

typedef struct _VParam VParam;
typedef struct _VObject VObject;
typedef struct _ValueObj ValueObj;
typedef struct _VTree VTree;

struct _VTree
{
	int			treeType;
	VObject*	pTop;
	VObject*	pCur;
	VTree*		pNext;
};

struct _VParam
{
	int			parameter;
	int			paramValue;
	VParam*		pNext;
};

struct _VObject
{
	int			property;
	VParam*		pParam;
	int			valueCount;
	int			numOfBiData;
	char*		pszValue[VDATA_VALUE_COUNT_MAX];
	VObject*	pSibling;
	VObject*	pParent;
	VObject*	pChild;

	char*		pszGroupName; //VDATA_GROUPNAME_SUPPORTED
};

struct _ValueObj
{
	char*		szName;
	int			flag;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // _VTYPES_H
