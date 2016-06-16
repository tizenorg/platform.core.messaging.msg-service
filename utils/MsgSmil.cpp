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

#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "MsgMmsMessage.h"
#include "MsgInternalTypes.h"
#include "MsgSmil.h"
#include "MsgUtilFunction.h"
#include "MsgDebug.h"

#define INVALID_HOBJ	-1
#define MSG_SMIL_MAX_DOC	1
#define MSG_SMIL_COLOR_SIZE	10
#define MSG_STDSTR_SHORT	0x7F

typedef int HMsgSmil; /* SmilDoc Handle */

typedef enum _SMIL_ELEMENT_T {
	ELEMENT_SMIL,
	ELEMENT_HEAD,
	ELEMENT_LAYOUT,
	ELEMENT_ROOTLAYOUT,
	ELEMENT_REGION,
	ELEMENT_TRANSITION,
	ELEMENT_META,
	ELEMENT_BODY,
	ELEMENT_PAR,
	ELEMENT_PARAM,
	ELEMENT_TEXT,
	ELEMENT_IMG,
	ELEMENT_AUDIO,
	ELEMENT_VIDEO,
	ELEMENT_REF,
	ELEMENT_ANIMATE,
	ELEMENT_MAX,
} SMIL_ELEMENT_T;

typedef enum _SMIL_ATTRIBUTE_T {
	ATTRIBUTE_UNKNOWN = -1,
	ATTRIBUTE_ID,
	ATTRIBUTE_TOP,
	ATTRIBUTE_LEFT,
	ATTRIBUTE_WIDTH,
	ATTRIBUTE_HEIGHT,
	ATTRIBUTE_FIT,
	ATTRIBUTE_BGCOLOR,
	ATTRIBUTE_DUR,
	ATTRIBUTE_SRC,
	ATTRIBUTE_COLOR,
	ATTRIBUTE_BOLD,
	ATTRIBUTE_UNDERLINE,
	ATTRIBUTE_ITALIC,
	ATTRIBUTE_REVERSE,
	ATTRIBUTE_DIRECTION,
	ATTRIBUTE_SIZE,
	ATTRIBUTE_FONT,
	ATTRIBUTE_REGION,
	ATTRIBUTE_NAME,
	ATTRIBUTE_VALUE,
	ATTRIBUTE_ALT,
	ATTRIBUTE_TYPE,
	ATTRIBUTE_SUBTYPE,
	ATTRIBUTE_CONTENT,
	ATTRIBUTE_FGCOLOR,
	ATTRIBUTE_TEXTFORMAT,
	ATTRIBUTE_TRANSIN,
	ATTRIBUTE_TRANSOUT,
	ATTRIBUTE_BEGIN,
	ATTRIBUTE_END,
	ATTRIBUTE_REPEAT_COUNT,
} SMIL_ATTRIBUTE_T;

typedef struct _MsgSmilDoc {
	xmlDocPtr pSmilDoc;
	xmlNodePtr pstRootNode;
} MsgSmilDoc;

/* static variables */
static char gszEmptyRawDoc[] = "<smil><head><layout></layout></head><body></body></smil>";

__thread MsgSmilDoc *__gpaMsgSmilDoc[MSG_SMIL_MAX_DOC]={NULL, };
__thread char gszColor[MSG_SMIL_COLOR_SIZE + 1] = {0, };

__thread bool gCmd[ELEMENT_MAX] = {false, };
__thread MMS_SMIL_ROOTLAYOUT gRootlayout = {};
__thread MMS_SMIL_REGION *gRegion = NULL;
__thread MMS_PAGE_S *gPage = NULL;
__thread MMS_MEDIA_S *gMedia = NULL;
__thread MMS_SMIL_TRANSITION *gTransition = NULL;
__thread MMS_SMIL_META *gMeta = NULL;

/* For Parse Smil */
static int MsgSmilGetColorValue(xmlChar *content);
static int MsgSmilGetTime(char *pValue);
static int MsgSmilAtoIHexa(char *pInput);
static int MsgSmilGetElementID(char *pString);
static int MsgSmilGetAttrID(char *pString);
static int MsgSmilGetFontSizeValue(char *pString);
static bool MsgSmilGetFontAttrib(char *pString);
static MmsTextDirection MsgSmilGetFontDirection(char *pString);
static MmsSmilFontType MsgSmilGetFontTypeValue(char *pString);

static xmlNodePtr MsgSmilGetNodeByElementName(xmlNodePtr pstNode, char *pszName);

/* For Generate Smil */
static HMsgSmil MsgSmilCreateEmptySmilDoc(void);
static HMsgSmil MsgSmilCreateSmilDoc(char *pszRawData);
static bool MsgSmilDestroyDoc(HMsgSmil hSmilDoc);
static bool IsValidSmilDocNo(int nSmilDocNo);
static char *MsgSmilGetRawData(HMsgSmil hSmilDoc);

static const char *MsgSmilColorValueToString(int nValue);

static bool MsgSmilAddPage(HMsgSmil hSmilDoc, MMS_PAGE_S *pstSmilPage);
static bool MsgSmilAddRootLayout(HMsgSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout);
static bool MsgSmilAddRegion(HMsgSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion);
static bool MsgSmilAddMedia(HMsgSmil hSmilDoc, int nPageNo, int nMediaIdx, MMS_MEDIA_S *pstSmilMedia, char *pszContentID);
static xmlNode *MsgSmilCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID);
static xmlNode *MsgSmilCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID);
static bool MsgSmilInsertFirstChild(xmlNode *pParent, xmlNode *pNode);
static bool MsgSmilInsertNode(xmlNode *pParent, xmlNode *pLeftSibling, xmlNode *pNode);
static void MsgSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue);


int MsgSmilGetColorValue(xmlChar *content)
{
	int color;
	char color_inp[9] = {0, };

	if (content[0] == '#') {	/* RGB value */
		snprintf(color_inp, sizeof(color_inp), "FF%s", (char *)&content[1]);
		color = MsgSmilAtoIHexa(color_inp);
	} else if (content[0] == '0' && (content[1] == 'x' || content[1] == 'X')) {
		snprintf(color_inp, sizeof(color_inp), "%s", (char *)&content[2]);
		color = MsgSmilAtoIHexa(color_inp);
	} else {
		MSG_DEBUG("Invalid Color Value");
		color = 0x00000000;
	}

	return color;
}


const char *MsgSmilColorValueToString(int nValue)
{
	unsigned char alpha = (nValue & 0xFF000000) >> 24;
	unsigned char red = (nValue & 0xFF0000) >> 16;
	unsigned char green = (nValue & 0x00FF00) >> 8;
	unsigned char blue = nValue & 0x0000FF;

	if (alpha == 0xFF) {
		snprintf(gszColor, sizeof(gszColor), "#%02x%02x%02x", red, green, blue);
	} else {
		snprintf(gszColor, sizeof(gszColor), "0x%02x%02x%02x%02x", alpha, red, green, blue);
	}

	MSG_DEBUG("color value : [%s]", gszColor);

	return gszColor;
}


int MsgSmilAtoIHexa(char *pInput)
{
	int res = 0;
	int len	= 0;
	int temp = 1;
	int i  = 0;
	int j = 0;
	char *pOutput = NULL;

	len = strlen(pInput);
	pOutput = (char *)malloc(len + 1);

	if (pOutput == NULL) {
		MSG_DEBUG("Memory full");
		goto __CATCH;
	}

	memset(pOutput, 0, len + 1);

	for (i = len - 1; i >= 0; i--) {
		for (j = 0; j < (len - 1 - i); j++) {
			temp *= 16;
		}

		switch (pInput[i]) {
		case '0':
			pOutput[i] = 0;
			break;

		case '1':
			pOutput[i] = 1;
			break;

		case '2':
			pOutput[i] = 2;
			break;

		case '3':
			pOutput[i] = 3;
			break;

		case '4':
			pOutput[i] = 4;
			break;

		case '5':
			pOutput[i] = 5;
			break;

		case '6':
			pOutput[i] = 6;
			break;

		case '7':
			pOutput[i] = 7;
			break;

		case '8':
			pOutput[i] = 8;
			break;

		case '9':
			pOutput[i] = 9;
			break;

		case 'a':
		case 'A':
			pOutput[i] = 10;
			break;

		case 'b':
		case 'B':
			pOutput[i] = 11;
			break;

		case 'c':
		case 'C':
			pOutput[i] = 12;
			break;

		case 'd':
		case 'D':
			pOutput[i] = 13;
			break;

		case 'e':
		case 'E':
			pOutput[i] = 14;
			break;

		case 'f':
		case 'F':
			pOutput[i] = 15;
			break;
		}

		res += (pOutput[i] * temp);
		temp = 1;
	}

__CATCH:

	if (pOutput) {
		free(pOutput);
		pOutput = NULL;
	}

	return res;
}

int MsgSmilGetTime(char *pValue)
{
	char *pTemp = NULL;
	bool bMSec = false;
	int retVal = 0;
	int i = 0;

	if (pValue == NULL || pValue[0] == '\0')
		return 0;

	/* Default time unit -> millisecond */
	if (strstr(pValue, "msec"))
		bMSec = true;

	if (strstr(pValue, "ms"))
		bMSec = true;

	pTemp = (char *)calloc(1, strlen(pValue) + 1);

	if (NULL == pTemp) {
		MSG_DEBUG("calloc for <time> attribute is failed");
		return 0;
	}

	while (isdigit(pValue[i])) {
		pTemp[i] = pValue[i];
		i++;
	}
	pTemp[i] = '\0';

	/* Detect 's' and 'ms' here */
	retVal = atoi(pTemp);

	if (bMSec == false)
		retVal *= 1000;

	if (pTemp) {
		free(pTemp);
		pTemp = NULL;
	}

	return retVal;
}

int MsgSmilGetElementID(char *pString)
{
	if (!strcmp(pString, "smil"))
		return ELEMENT_SMIL;
	else if (!strcmp(pString, "head"))
		return ELEMENT_HEAD;
	else if (!strcmp(pString, "layout"))
		return ELEMENT_LAYOUT;
	else if (!strcmp(pString, "root-layout"))
		return ELEMENT_ROOTLAYOUT;
	else if (!strcmp(pString, "region"))
		return ELEMENT_REGION;
	else if (!strcmp(pString, "body"))
		return ELEMENT_BODY;
	else if (!strcmp(pString, "par"))
		return ELEMENT_PAR;
	else if (!strcmp(pString, "param"))
		return ELEMENT_PARAM;
	else if (!strcmp(pString, "text"))
		return ELEMENT_TEXT;
	else if (!strcmp(pString, "img"))
		return ELEMENT_IMG;
	else if (!strcmp(pString, "audio"))
		return ELEMENT_AUDIO;
	else if (!strcmp(pString, "video"))
		return ELEMENT_VIDEO;
	else if (!strcmp(pString, "ref"))
		return ELEMENT_REF;
	else if (!strcmp(pString, "animate"))
		return ELEMENT_ANIMATE;
	else if (!strcmp(pString, "root-layout"))
		return ELEMENT_HEAD;
	else if (!strcmp(pString, "transition"))
		return ELEMENT_TRANSITION;
	else if (!strcmp(pString, "meta"))
		return ELEMENT_META;
	else
		return -1;
}

int MsgSmilGetAttrID(char *pString)
{
	if (!strcmp(pString, "id"))
		return ATTRIBUTE_ID;
	else if (!strcmp(pString, "top"))
		return ATTRIBUTE_TOP;
	else if (!strcmp(pString, "left"))
		return ATTRIBUTE_LEFT;
	else if (!strcmp(pString, "width"))
		return ATTRIBUTE_WIDTH;
	else if (!strcmp(pString, "height"))
		return ATTRIBUTE_HEIGHT;
	else if (!strcmp(pString, "fit"))
		return ATTRIBUTE_FIT;
	else if (!strcmp(pString, "backgroundColor") || !strcmp(pString, "background-color"))
		return ATTRIBUTE_BGCOLOR;
	else if (!strcmp(pString, "dur"))
		return ATTRIBUTE_DUR;
	else if (!strcmp(pString, "src"))
		return ATTRIBUTE_SRC;
	else if (!strcmp(pString, "color"))
		return ATTRIBUTE_COLOR;
	else if (!strcmp(pString, "bold"))
		return ATTRIBUTE_BOLD;
	else if (!strcmp(pString, "underline"))
		return ATTRIBUTE_UNDERLINE;
	else if (!strcmp(pString, "italic"))
		return ATTRIBUTE_ITALIC;
	else if (!strcmp(pString, "reverse"))
		return ATTRIBUTE_REVERSE;
	else if (!strcmp(pString, "direction"))
		return ATTRIBUTE_DIRECTION;
	else if (!strcmp(pString, "size"))
		return ATTRIBUTE_SIZE;
	else if (!strcmp(pString, "font"))
		return ATTRIBUTE_FONT;
	else if (!strcmp(pString, "region"))
		return ATTRIBUTE_REGION;
	else if (!strcmp(pString, "name"))
		return ATTRIBUTE_NAME;
	else if (!strcmp(pString, "value"))
		return ATTRIBUTE_VALUE;
	else if (!strcmp(pString, "alt"))
		return ATTRIBUTE_ALT;
	else if (!strcmp(pString, "type"))
		return ATTRIBUTE_TYPE;
	else if (!strcmp(pString, "subtype"))
		return ATTRIBUTE_SUBTYPE;
	else if (!strcmp(pString, "content"))
		return ATTRIBUTE_CONTENT;
	else if (!strcmp(pString, "transIn"))
		return ATTRIBUTE_TRANSIN;
	else if (!strcmp(pString, "transOut"))
		return ATTRIBUTE_TRANSOUT;
	else if (!strcmp(pString, "begin"))
		return ATTRIBUTE_BEGIN;
	else if (!strcmp(pString, "end"))
		return ATTRIBUTE_END;
	else if (!strcmp(pString, "repeatCount"))
		return ATTRIBUTE_REPEAT_COUNT;

	return -1;
}

bool MsgSmilGetFontAttrib(char *pString)
{
	if (!strcmp(pString, "true"))
		return true;
	else
		return false;
}

MmsTextDirection MsgSmilGetFontDirection(char *pString)
{
	MmsTextDirection direction = MMS_TEXT_DIRECTION_INVALID;

	if (!strcmp(pString, "right"))
		direction = MMS_TEXT_DIRECTION_RIGHT;
	else if (!strcmp(pString, "down"))
		direction = MMS_TEXT_DIRECTION_DOWN;

	return direction;
}

int MsgSmilGetFontSizeValue(char *pString)
{
	if (!strcmp(pString, "small"))
		return MMS_SMIL_FONT_SIZE_SMALL;
	else if (!strcmp(pString, "normal"))
		return MMS_SMIL_FONT_SIZE_NORMAL;
	else if (!strcmp(pString, "large"))
		return MMS_SMIL_FONT_SIZE_LARGE;
	else
		return atoi(pString);
}

MmsSmilFontType MsgSmilGetFontTypeValue(char *pString)
{
	if (!strcmp(pString, "normal"))
		return MMS_SMIL_FONT_TYPE_NORMAL;
	else if (!strcmp(pString, "italic"))
		return MMS_SMIL_FONT_TYPE_ITALIC;
	else if (!strcmp(pString, "bold"))
		return MMS_SMIL_FONT_TYPE_BOLD;
	else if (!strcmp(pString, "underline"))
		return MMS_SMIL_FONT_TYPE_UNDERLINE;
	else
		return MMS_SMIL_FONT_TYPE_NORMAL;
}

HMsgSmil MsgSmilCreateEmptySmilDoc(void)
{
	HMsgSmil hMmsSmil;

	MSG_BEGIN();

	hMmsSmil = MsgSmilCreateSmilDoc(gszEmptyRawDoc);

	MSG_DEBUG("Create an empty smilDoc.(Handle = %d)", hMmsSmil);

	MSG_END();

	return hMmsSmil;
}

HMsgSmil MsgSmilCreateSmilDoc(char *pszRawData)
{
	int nSmilDocNo = 0;
	xmlNodePtr stRootNode;

	MSG_BEGIN();

	/* Destroy smil doc if present */
	if (NULL != __gpaMsgSmilDoc[nSmilDocNo]) {
		if (false == MsgSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MsgSmilDestroyDoc: Failed!");
		}
	}

	for (nSmilDocNo = 0; nSmilDocNo < MSG_SMIL_MAX_DOC; nSmilDocNo++) {
		if (NULL == __gpaMsgSmilDoc[nSmilDocNo])
			break;
	}

	if (MSG_SMIL_MAX_DOC == nSmilDocNo) {
		MSG_DEBUG("SmilDoc table is full. Can't create.");
		return INVALID_HOBJ;
	}
	__gpaMsgSmilDoc[nSmilDocNo] = (MsgSmilDoc*)malloc(sizeof(MsgSmilDoc));
	if (NULL ==  __gpaMsgSmilDoc[nSmilDocNo]) {
		MSG_DEBUG("Memory Allocation Failed.");
		return INVALID_HOBJ;
	}
	memset(__gpaMsgSmilDoc[nSmilDocNo], 0, sizeof(MsgSmilDoc));

	__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc = xmlParseMemory(pszRawData, strlen(pszRawData));
	if (NULL == __gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc) {
		if (false == MsgSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MsgSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}

	stRootNode = xmlDocGetRootElement(__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc);
	if (NULL == stRootNode) {
		MSG_DEBUG("Empty document");
		if (false == MsgSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MsgSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}

	if (xmlStrcmp(stRootNode->name, (const xmlChar *) "smil")) {
		MSG_DEBUG("Document of the wrong type, root node != smil");
		if (false == MsgSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MsgSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}

	__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode = stRootNode;

	MSG_END();
	return ((HMsgSmil)nSmilDocNo);
}

bool MsgSmilDestroyDoc(HMsgSmil hSmilDoc)
{
	int nSmilDocNo = (int)hSmilDoc;
	bool bFlag = true;
	MSG_BEGIN();

	if (0 <= nSmilDocNo &&
		nSmilDocNo < MSG_SMIL_MAX_DOC &&
		__gpaMsgSmilDoc[nSmilDocNo]) {
		if (__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc) {
			xmlFreeDoc(__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc);
		}

		if (__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode) {
			/* Need to Check */
		}
		free(__gpaMsgSmilDoc[nSmilDocNo]);
		__gpaMsgSmilDoc[nSmilDocNo] = NULL;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		bFlag =  false;
	}

	MSG_END();
	return bFlag;
}

bool IsValidSmilDocNo(int nSmilDocNo)
{
	bool bIsValidSmil = false;

	MSG_BEGIN();

	if (0 <= nSmilDocNo &&
		nSmilDocNo < MSG_SMIL_MAX_DOC &&
		__gpaMsgSmilDoc[nSmilDocNo] &&
		__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc) {
		bIsValidSmil = true;
	}

	MSG_END();
	return bIsValidSmil;
}

char *MsgSmilGetRawData(HMsgSmil hSmilDoc)
{
	int nSmilDocNo = (int)hSmilDoc;
	char *pszRawData = NULL;

	MSG_BEGIN();

	if (IsValidSmilDocNo(nSmilDocNo)) {
		/* xmlSaveFormatFileEnc("-", __gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc, "UTF-8", 1); */
		xmlDocDumpMemory(__gpaMsgSmilDoc[nSmilDocNo]->pSmilDoc, (xmlChar **)(&pszRawData) , NULL);
		if (NULL == pszRawData) {
			MSG_DEBUG("Invalid pSmilDoc (now wellformed document)");
		}
		MSG_END();
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
	}
	return pszRawData;
}

bool MsgSmilAddPage(HMsgSmil hSmilDoc, MMS_PAGE_S *pstSmilPage)
{
	int nSmilDocNo = hSmilDoc;

	MSG_BEGIN();

	bool ret = true;

	if (IsValidSmilDocNo(nSmilDocNo)) {
		xmlNodePtr pstParElement;
		xmlNodePtr pstBodyElement;
		xmlNodePtr pstParList;
		char szBuf[MSG_STDSTR_SHORT] = {0, };

		pstBodyElement = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"body");

		if (NULL == pstBodyElement) {
			MSG_DEBUG("There is no <body> node. Can't create <par> node.");
			return false;
		}

		/* Create "par"  node and insert it */
		pstParElement = xmlNewNode(NULL, (xmlChar *)"par");

		if (NULL == pstParElement) {
			MSG_DEBUG("Can't create <par> node. (from XmlParser)");
			return false;
		}

		MSG_SEC_DEBUG("Par Element Name = %s", (char *)pstParElement->name);

		/* Add attributes to "par" */
		if (pstSmilPage->nDur > 0) {
			snprintf(szBuf, MSG_STDSTR_SHORT, "%dms", pstSmilPage->nDur);
			xmlSetProp(pstParElement, (const xmlChar *)"dur", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] : dur : [%s]", szBuf);
		}

		/* Find the insertion point : right sibling of the last <par> node or first child of <body> */
		pstParList = xmlGetLastChild(pstBodyElement);

		if (pstParList) {
			ret = MsgSmilInsertNode(pstBodyElement, pstParList, pstParElement);
			if (ret == false)
				xmlFreeNode(pstParElement);
		} else {
			ret = MsgSmilInsertFirstChild(pstBodyElement, pstParElement);
			if (ret == false)
				xmlFreeNode(pstParElement);
		}
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}

	return ret;
}

bool MsgSmilAddRootLayout(HMsgSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout)
{
	int nSmilDocNo = hSmilDoc;
	xmlNodePtr pstRootLayout = NULL;
	xmlNodePtr pstLayoutList = NULL;
	xmlNodePtr pstRootLayoutList = NULL;
	char szBuf[MSG_STDSTR_SHORT] = {0, };

	MSG_BEGIN();

	if (IsValidSmilDocNo(nSmilDocNo)) {
		pstLayoutList = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"layout");

		if (NULL == pstLayoutList) {
			MSG_DEBUG("There is no <layout> node. Can't create <root-layout> node.");
			return false;
		}
		MSG_SEC_DEBUG("Layout Element Name = %s ", (char *)pstLayoutList->name);

		pstRootLayoutList = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"root-layout");

		if (NULL != pstRootLayoutList) {
			MSG_DEBUG("There is <root-layout> node already");
			MSG_SEC_DEBUG("Root Layout Element Name = %s", (char *)pstRootLayoutList->name);
			return false;
		}
		/* Create "root-layout" node and insert it */
		pstRootLayout = xmlNewNode(NULL, (xmlChar *)"root-layout");
		if (NULL == pstRootLayout) {
			MSG_DEBUG("Can't create <root-layout> node. (from XmlParser)");
			return false;
		}
		MSG_SEC_DEBUG("Root Layout Element Name = %s", (char *)pstRootLayout->name);

		if (pstSmilRootLayout->bBgColor == true) {	/* Replace value later */
			if (((pstSmilRootLayout->bgColor & 0xFF000000) >> 24) > 0) { /* check alpha value */
				xmlSetProp(pstRootLayout, (const xmlChar *)"backgroundColor", (const xmlChar *)MsgSmilColorValueToString(pstSmilRootLayout->bgColor));
			}
		}

		/* width */
		if (true == pstSmilRootLayout->width.bUnitPercent) {
			snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRootLayout->width.value);
			xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)szBuf);
		} else {
			if (pstSmilRootLayout->width.value > 0) {
				snprintf(szBuf, MSG_STDSTR_SHORT, "%d", pstSmilRootLayout->width.value);
				xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)szBuf);
			} else {
				MSG_DEBUG("Set Width : default value [100%%]");
				xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)"100%");
			}
		}

		/* Height */
		if (true == pstSmilRootLayout->height.bUnitPercent) {
			snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRootLayout->height.value);
			xmlSetProp(pstRootLayout, (const xmlChar *)"height", (const xmlChar *)szBuf);
		} else {
			if (pstSmilRootLayout->height.value > 0) {
				snprintf(szBuf, MSG_STDSTR_SHORT, "%d", pstSmilRootLayout->height.value);
				xmlSetProp(pstRootLayout, (const xmlChar *)"height", (const xmlChar *)szBuf);
			} else {
				xmlSetProp(pstRootLayout, (const xmlChar *)"height", (const xmlChar *)"100%");
			}
		}

		xmlAddChild(pstLayoutList, pstRootLayout);

		return true;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}
}

bool MsgSmilAddRegion(HMsgSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion)
{
	int nSmilDocNo = hSmilDoc;

	MSG_BEGIN();

	if (IsValidSmilDocNo(nSmilDocNo)) {
		int nRootWidth = 0;
		int nRootHeight = 0;
		xmlNodePtr pstRegion;
		xmlNodePtr pstLayoutList;
		xmlNodePtr pstRootLayoutList;
		xmlAttrPtr pAttribute;
		char szBuf[MSG_STDSTR_SHORT] = {0, };

		pstLayoutList = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"layout");
		if (NULL == pstLayoutList) {
			MSG_DEBUG(" There is no <layout> node. Can't create <region> node");
			return false;
		}

		/* Find the insertion point : right sibling of the last root-layout node or first child of pLayout */
		pstRootLayoutList = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"root-layout");

		if (NULL == pstRootLayoutList) {
			MSG_DEBUG("There is no <root-layout> node. Can't create <root-layout> node.");
			return false;
		} else {
			pAttribute =  pstRootLayoutList->properties;
		}

		if (NULL == pAttribute) {
			MSG_DEBUG("There is no Attribute in <root-layout> node.");
			return false;
		}

		xmlAttrPtr pstAttr = pAttribute;
		for ( ; pstAttr; pstAttr = pstAttr->next) {
			int	attrType;
			MSG_SEC_DEBUG("AttributeType: (%s, %s) ", pstAttr->name, pstAttr->children->content);
			switch (attrType = MsgSmilGetAttrID((char *)pstAttr->name)) {
			case ATTRIBUTE_WIDTH: {
#if 0
				int bUnitPercent;

				if (strchr((char *)pstAttr->children->content, '%'))
					bUnitPercent = true;
				else
					bUnitPercent = false;
#endif
					nRootWidth = atoi((char *)pstAttr->children->content);
				}
				break;

			case ATTRIBUTE_HEIGHT: {
#if 0
				int bUnitPercent;

				if (strchr((char *)pstAttr->children->content, '%'))
					bUnitPercent = true;
				else
					bUnitPercent = false;
#endif
					nRootHeight = atoi((char *)pstAttr->children->content);
				}
				break;
			}
		}


		/* Add attributes */
		if (pstSmilRegion) {
			/* Create "region" node and insert it */
			pstRegion = xmlNewNode(NULL, (xmlChar *)"region");
			if (NULL == pstRegion) {
				MSG_DEBUG("Can't create <region> node. (from XmlParser)");
				return false;
			}

			MSG_SEC_DEBUG("Region Element Name = %s", (char *)pstRegion->name);

			if (strlen(pstSmilRegion->szID) > 0) {
				xmlSetProp(pstRegion, (const xmlChar *)"id", (const xmlChar *)pstSmilRegion->szID);
				MSG_DEBUG("[Set Attribute] : Region Id : [%s]", pstSmilRegion->szID);
			}

			if (pstSmilRegion->bBgColor == true) {
				MSG_DEBUG(" [Set Attribute] : BkGrd Color");
				if (((pstSmilRegion->bgColor & 0xFF000000) >> 24) > 0) {
					xmlSetProp(pstRegion, (const xmlChar *)"backgroundColor", (const xmlChar *)MsgSmilColorValueToString(pstSmilRegion->bgColor));
					MSG_DEBUG("[Set Attribute] : backgroundColor [%s]", MsgSmilColorValueToString(pstSmilRegion->bgColor));
				}
			}


			if (true == pstSmilRegion->width.bUnitPercent) {
				if (pstSmilRegion->width.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->width.value);
					xmlSetProp(pstRegion, (const xmlChar *)"width", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Width : [%s]", szBuf);
				}
			} else {
				/* Note: nRootWidth should be in terms of value(pixel) not unitpercent(%) */
				/* Validation should be done before this. */
				if (pstSmilRegion->width.value >= 0 &&
					pstSmilRegion->width.value <= nRootWidth &&
					nRootWidth != 0) {
					int iWidth = (pstSmilRegion->width.value * 100) / nRootWidth;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iWidth);
					xmlSetProp(pstRegion, (const xmlChar *)"width", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Width : [%s]", szBuf);
				}
			}

			if (true == pstSmilRegion->height.bUnitPercent) {
				if (pstSmilRegion->height.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->height.value);
					xmlSetProp(pstRegion, (const xmlChar *)"height", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Height : [%s]", szBuf);
				}
			} else {
				/* Note: nRootHeight should be in terms of value(pixel) not unitpercent(%) */
				/* Validation should be done before this. */
				if (pstSmilRegion->height.value >= 0 &&
					pstSmilRegion->height.value <= nRootHeight &&
					nRootHeight != 0) {
					int iHeight = (pstSmilRegion->height.value * 100) / nRootHeight;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iHeight);
					xmlSetProp(pstRegion, (const xmlChar *)"height", (const xmlChar *)szBuf);

					MSG_DEBUG("[Set Attribute] : Height : [%s]", szBuf);
				}
			}

			if (true == pstSmilRegion->nLeft.bUnitPercent) {
				if (pstSmilRegion->nLeft.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->nLeft.value);
					xmlSetProp(pstRegion, (const xmlChar *)"left", (const xmlChar *)szBuf);

					MSG_DEBUG("[Set Attribute] : Left : [%s]", szBuf);
				}
			} else {
				/* Note: nRootWidth should be in terms of value(pixel) not unitpercent(%) */
				/* Validation should be done before this. */
				if (pstSmilRegion->nLeft.value >= 0 && nRootWidth != 0) {
					int iLeft = (pstSmilRegion->nLeft.value * 100) / nRootWidth;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iLeft);
					xmlSetProp(pstRegion, (const xmlChar *)"left", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Left : [%s]", szBuf);
				}
			}

			if (true == pstSmilRegion->nTop.bUnitPercent) {
				if (pstSmilRegion->nTop.value > 0 &&
						nRootHeight != 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->nTop.value);
					xmlSetProp(pstRegion, (const xmlChar *)"top", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Top : [%s]", szBuf);
				}
			} else {
				/* Note: nRootHeight should be in terms of value(pixel) not unitpercent(%) */
				/* Validation should be done before this. */
				if (pstSmilRegion->nTop.value >= 0 && nRootHeight != 0) {
					int iTop = (pstSmilRegion->nTop.value * 100) / nRootHeight;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iTop);
					xmlSetProp(pstRegion, (const xmlChar *)"top", (const xmlChar *)szBuf);
					MSG_DEBUG("[Set Attribute] : Top : [%s]", szBuf);
				}
			}

			/* Fit Attribute */
			if (MMSUI_IMAGE_REGION_FIT_MEET == pstSmilRegion->fit) {
				xmlSetProp(pstRegion, (const xmlChar *)"fit", (const xmlChar *)"meet");
				MSG_DEBUG("[Set Attribute] : fit : [meet]");
			} else if (MMSUI_IMAGE_REGION_FIT_HIDDEN == pstSmilRegion->fit) {
				xmlSetProp(pstRegion, (const xmlChar *)"fit", (const xmlChar *)"hidden");
				MSG_DEBUG("[Set Attribute] : fit : [hidden]");
			}

			MsgSmilInsertNode(pstLayoutList, pstRootLayoutList, pstRegion);
		} else {
			MSG_DEBUG("There is no attribute in <region> node");
		}

		MSG_END();
		return true;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}
}

bool MsgSmilAddMedia(HMsgSmil hSmilDoc, int nPageNo, int nMediaIdx, MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
{
	int nSmilDocNo = hSmilDoc;

	MSG_BEGIN();

	if (NULL == pszContentID) {
		MSG_DEBUG(" Content Id is NULL");
		return false;
	}
	memset(pszContentID, 0, sizeof(pstSmilMedia->szContentID));
	if (IsValidSmilDocNo(nSmilDocNo)) {
		int nIndex = 0;
		xmlNode *pstMedia;
		xmlNode *pstLastChild;
		xmlNodePtr pstParList;
		char *pszExt ;

		pstParList = MsgSmilGetNodeByElementName(__gpaMsgSmilDoc[nSmilDocNo]->pstRootNode, (char *)"par");

		if (NULL == pstParList) {
			MSG_DEBUG("There is no <par> node. Can't create <media> node.");
			return false;
		}

		for (nIndex = 0; (pstParList &&  nIndex < nPageNo); nIndex++) {
			pstParList = pstParList->next;
		}

		if (NULL == pstParList) {
			MSG_DEBUG("There is no such page node. Can't insert <media> node.");
			return false;
		}

		/* Find insertion point and make a contentID */
		pstLastChild = xmlGetLastChild(pstParList);

		pszExt = strrchr(pstSmilMedia->szFileName, '.');
		if (pszExt && !strrchr(pszExt, '/'))
			snprintf(pszContentID, MSG_MSG_ID_LEN+1, "%lu_%lu%s", (unsigned long)nPageNo, (unsigned long)nMediaIdx, pszExt);
		else
			snprintf(pszContentID, MSG_MSG_ID_LEN+1, "%lu_%lu", (unsigned long)nPageNo, (unsigned long)nMediaIdx);

		/* remove space character in content location */
		msg_replace_space_char(pstSmilMedia->szFileName);

		memset(pstSmilMedia->szContentLocation, 0, sizeof(pstSmilMedia->szContentLocation));
		gchar *tmpContentLoc = msg_replace_non_ascii_char(pstSmilMedia->szFileName, '_');
		if (tmpContentLoc) {
			MSG_SEC_DEBUG("tmpContentLoc = [%s]", tmpContentLoc);
			snprintf(pstSmilMedia->szContentLocation, sizeof(pstSmilMedia->szContentLocation), "%s", tmpContentLoc);
			g_free(tmpContentLoc);
			tmpContentLoc = NULL;
		} else {
			MSG_WARN("tmpContentLoc is NULL.");
			snprintf(pstSmilMedia->szContentLocation, sizeof(pstSmilMedia->szContentLocation), "%s", pstSmilMedia->szFileName);
		}

		/* Create <media> node and insert set attribute */
		switch (pstSmilMedia->mediatype) {
		case MMS_SMIL_MEDIA_TEXT:
			pstMedia = MsgSmilCreateTextNode(pstSmilMedia, pszContentID);
			break;
		case MMS_SMIL_MEDIA_AUDIO:
		case MMS_SMIL_MEDIA_VIDEO:
		case MMS_SMIL_MEDIA_IMG:
			pstMedia = MsgSmilCreateMMNode(pstSmilMedia, pszContentID);
			break;
		default:
			MSG_DEBUG("Invalid media type. Can't insert such-<media> node.");
			return false;
		}

		if (NULL == pstMedia) {
			MSG_DEBUG("Can't create <media> node. (from XmlParser) (media-type:%d)", pstSmilMedia->mediatype);
			return false;
		}

		/* Find the insertion point : the last child of <par> node */
		if (pstLastChild)
			MsgSmilInsertNode(pstParList, pstLastChild, pstMedia);
		else
			MsgSmilInsertFirstChild(pstParList, pstMedia);

		MSG_END();
		return true;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}
}

xmlNode *MsgSmilCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
{
	xmlNode *pstMedia = NULL;
	xmlNode *pstParam = NULL;
	char szBuf[MSG_STDSTR_SHORT] = {0, };
	char szSizeBuf[MSG_STDSTR_SHORT] = {0, };

	MSG_BEGIN();

	pstMedia = xmlNewNode(NULL, (xmlChar *)"text");
	if (NULL == pstMedia) {
		MSG_DEBUG("Can't create <Text> node.");
		return NULL;
	}

	/* Add attributes */
	if (pstSmilMedia) {
		if (strlen(pstSmilMedia->regionId) > 0) {
			xmlSetProp(pstMedia, (const xmlChar *)"region", (const xmlChar *)pstSmilMedia->regionId);
			MSG_DEBUG("[Set Attribute] Region Id : [%s]", pstSmilMedia->regionId);
		}

		if (pstSmilMedia->sMedia.sText.nBegin > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sText.nBegin);
			xmlSetProp(pstMedia, (const xmlChar *)"begin", (const xmlChar *) szBuf);
			MSG_DEBUG("[Set Attribute] Begin : [%s]", szBuf);
		}

		if (pstSmilMedia->sMedia.sText.nDurTime > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sText.nDurTime);
			xmlSetProp(pstMedia, (const xmlChar *)"dur", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] Duration : [%s]", szBuf);
		}

		if (strlen(pstSmilMedia->szAlt) > 0) {
			snprintf (szBuf, sizeof(szBuf), "%s", pstSmilMedia->szAlt);
			xmlSetProp(pstMedia, (const xmlChar *)"alt", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] Alternate : [%s]", szBuf);
		}

#if 0
		char szFilePathWithCid[MSG_MSG_ID_LEN + 5];	/* for "cid:" */

		snprintf (szFilePathWithCid, sizeof(szFilePathWithCid), "cid:%s", pszContentID);
		MsgSmilSetAttribute(pstMedia, (char *)"src", szFilePathWithCid);
#endif
		if (strlen(pstSmilMedia->szContentLocation) > 0) {
			MsgSmilSetAttribute(pstMedia, (char *)"src", pstSmilMedia->szContentLocation); /* using content Location in Smil */
			MSG_DEBUG("[Set Attribute] Src : [%s]", pstSmilMedia->szContentLocation);
		}

		if (((pstSmilMedia->sMedia.sText.nColor & 0xFF000000) >> 24) > 0) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");

			if (NULL == pstParam) {
				MSG_DEBUG("Cannot create <param> node");
				return NULL;
			}

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"foreground-color");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)MsgSmilColorValueToString(pstSmilMedia->sMedia.sText.nColor));

			MSG_DEBUG("[Set param] Font Foreground Color : [0x%08x]", pstSmilMedia->sMedia.sText.nColor);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}

		if (((pstSmilMedia->sMedia.sText.nBgColor & 0xFF000000) >> 24) > 0) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");

			if (NULL == pstParam) {
				MSG_DEBUG("Cannot create <param> node");
				return NULL;
			}

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"background-color");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)MsgSmilColorValueToString(pstSmilMedia->sMedia.sText.nBgColor));

			MSG_DEBUG("[Set param] Font Background Color : [0x%08x]", pstSmilMedia->sMedia.sText.nBgColor);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.nSize > 0) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node");
				return NULL;
			}

			if (pstSmilMedia->sMedia.sText.nSize  <= MMS_SMIL_FONT_SIZE_SMALL) {
				snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "small");
			} else if ((pstSmilMedia->sMedia.sText.nSize  > MMS_SMIL_FONT_SIZE_SMALL) && (pstSmilMedia->sMedia.sText.nSize  < MMS_SMIL_FONT_SIZE_LARGE)) {
				snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "normal");
			} else {
				snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "large");
			}

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textsize");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);

			MSG_DEBUG("[Set param] textsize : [%s]", szSizeBuf);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bBold == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node");
				return NULL;
			}

			snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "bold");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);

			MSG_DEBUG("[Set param] textattribute : [%s]", szSizeBuf);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bItalic == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node");
				return NULL;
			}

			snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "italic");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);

			MSG_DEBUG("[Set param] textattribute : [%s]", szSizeBuf);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bUnderLine == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node");
				return NULL;
			}

			snprintf(szSizeBuf, sizeof(szSizeBuf), "%s", "underline");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);

			MSG_DEBUG("[Set param] textattribute : [%s]", szSizeBuf);

			MsgSmilInsertFirstChild(pstMedia, pstParam);
		}
	}

	MSG_END();
	return pstMedia;
}

xmlNode *MsgSmilCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
{
	xmlNode *pstMedia = NULL;
	char szBuf[MSG_STDSTR_SHORT] = {0, };

	MSG_BEGIN();

	if (!pstSmilMedia)
		return NULL;

	switch (pstSmilMedia->mediatype) {
	case MMS_SMIL_MEDIA_AUDIO:
		pstMedia = xmlNewNode(NULL, (xmlChar *)"audio");
		break;

	case MMS_SMIL_MEDIA_VIDEO:
		pstMedia = xmlNewNode(NULL, (xmlChar *)"video");
		break;

	case MMS_SMIL_MEDIA_IMG:
		pstMedia = xmlNewNode(NULL, (xmlChar *)"img");
		break;
	default:
		MSG_DEBUG("Invalid media type. Can't insert such-<media> node.");
		return NULL;
	}

	if (pstMedia) {
		if (strlen(pstSmilMedia->regionId) > 0) {
			xmlSetProp(pstMedia, (const xmlChar *)"region", (const xmlChar *)pstSmilMedia->regionId);
			MSG_DEBUG("[Set Attribute] Region Id : [%s]", pstSmilMedia->regionId);
		}

#if 0 /* set src attribute cid */
		char szFilePathWithCid[MSG_MSG_ID_LEN + 5];		/* for "cid:" */
		snprintf (szFilePathWithCid, sizeof(szFilePathWithCid), "cid:%s", pszContentID);
		MsgSmilSetAttribute(pstMedia, (char *)"src", szFilePathWithCid);
#endif
		if (strlen(pstSmilMedia->szContentLocation) > 0) {
			MsgSmilSetAttribute(pstMedia, (char *)"src", pstSmilMedia->szContentLocation); /* using content Location in Smil */
			MSG_SEC_DEBUG("[Set Attribute] src : [%s]", pstSmilMedia->szContentLocation);
		}

		if (pstSmilMedia->sMedia.sAVI.nBegin > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sAVI.nBegin);
			xmlSetProp(pstMedia, (const xmlChar *)"begin", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] begin : [%s]", szBuf);
		}

		if (pstSmilMedia->sMedia.sAVI.nDurTime > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sAVI.nDurTime);
			xmlSetProp(pstMedia, (const xmlChar *)"dur", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] dur : [%s]", szBuf);
		}

		if (strlen(pstSmilMedia->szAlt) > 0) {
			snprintf (szBuf, sizeof(szBuf), "%s", pstSmilMedia->szAlt);
			xmlSetProp(pstMedia, (const xmlChar *)"alt", (const xmlChar *)szBuf);
			MSG_DEBUG("[Set Attribute] alt : [%s]", szBuf);
		}
	} else {
		MSG_DEBUG("There is no attribute in such-<media> node");
	}

	MSG_END();
	return pstMedia;
}

bool MsgSmilInsertFirstChild(xmlNode *pstParent, xmlNode *pstCurr)
{
	bool bFlag = true;

	MSG_BEGIN();

	 if (NULL == xmlAddChild(pstParent, pstCurr)) {
		MSG_SEC_DEBUG("%s Node not added", pstCurr->name);
		bFlag = false;
	 }

	 MSG_END();
	 return bFlag;
}

bool MsgSmilInsertNode(xmlNode *pstParent, xmlNode *pstLeftSibling, xmlNode *pstCurr)
{
	MSG_BEGIN();
	bool bFlag = true;

	if (pstLeftSibling) {
		/* Parent Node is Unused */
		while (pstLeftSibling->next != NULL)
			pstLeftSibling = pstLeftSibling->next;

		 if (NULL == xmlAddNextSibling(pstLeftSibling, pstCurr)) {
			MSG_SEC_DEBUG("%s Node not added", pstCurr->name);
			bFlag = false;
		 }
	} else {
		 if (NULL == xmlAddChild(pstParent, pstCurr)) {
			MSG_SEC_DEBUG("%s Node not added", pstCurr->name);
			bFlag = false;
		 }
	}
	MSG_END();
	return bFlag;
}

void MsgSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue)
{
	MSG_BEGIN();

	if (pNode && szField && strlen(szField)) {
		if (szValue && strlen(szValue)) {
			xmlSetProp(pNode, (const xmlChar *)szField, (const xmlChar *)szValue);
		} else {
			xmlSetProp(pNode, (const xmlChar *)szField, (const xmlChar *)"");
		}
	}

	MSG_END();
}

xmlNodePtr MsgSmilGetNodeByElementName(xmlNodePtr pstNode, char *pszName)
{
	if ((NULL != pstNode) && (NULL != pszName)) {
		xmlNodePtr pstTempNode;
		xmlNodePtr pstReturnNode;

		pstTempNode = pstNode;

		for ( ; pstTempNode; pstTempNode = pstTempNode->next) {
			if (0 == strcasecmp((char *)pstTempNode->name, pszName)) {
				MSG_SEC_DEBUG("Find Node : name [%s][%p]", (char *)pstTempNode->name, pstTempNode);
				return pstTempNode;
			}

			if (pstTempNode->children) {
				pstReturnNode = MsgSmilGetNodeByElementName(pstTempNode->children, pszName);
				if (NULL != pstReturnNode) {
					return pstReturnNode;
				}
			}
		}
	}
	return NULL;
}

void MsgSmilParseNode(MMS_MESSAGE_DATA_S *pMmsMsg, xmlNode *a_node, int depth)
{
	MSG_BEGIN();

	int elementType;
	int attrType;

	xmlNode *cur_node = NULL;

	if (depth == 0) { /* init */
		memset(gCmd, 0x00, ELEMENT_MAX);
		memset(&gRootlayout, 0x00, sizeof(MMS_SMIL_ROOTLAYOUT));
		gRegion = NULL;
		gPage = NULL;
		gMedia = NULL;
		gTransition = NULL;
		gMeta = NULL;
	}

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		MSG_SEC_DEBUG("## [%d] node type : [Element], name: [%s] ##", depth, cur_node->name);

		if (cur_node->type == XML_ELEMENT_NODE) {
			/* Get Smil Element */
			switch (elementType = MsgSmilGetElementID((char *)cur_node->name)) {
			case ELEMENT_ROOTLAYOUT:
				gCmd[ELEMENT_ROOTLAYOUT] = true;
				break;

			case ELEMENT_REGION:
				if (gRegion) {
					MSG_DEBUG("region exist");
					return;
				}

				gRegion = (MMS_SMIL_REGION *)calloc(1, sizeof(MMS_SMIL_REGION));
				gCmd[ELEMENT_REGION] = true;
				break;

			case ELEMENT_TRANSITION:
				if (gTransition) {
					MSG_DEBUG("Transition exist");
					return;
				}

				gTransition = (MMS_SMIL_TRANSITION *)calloc(1, sizeof(MMS_SMIL_TRANSITION));
				gCmd[ELEMENT_TRANSITION] = true;
				break;

			case ELEMENT_META:
				if (gMeta) {
					MSG_DEBUG("Meta exist");
					return;
				}

				gMeta = (MMS_SMIL_META *)calloc(1, sizeof(MMS_SMIL_META));
				gCmd[ELEMENT_META] = true;
				break;

			case ELEMENT_PAR:
				if (gPage) {
					MSG_DEBUG("Page exist");
					return;
				}

				gPage = (MMS_PAGE_S *)calloc(1, sizeof(MMS_PAGE_S));
				gCmd[ELEMENT_PAR] = true;
				break;

			case ELEMENT_PARAM: /* Need to check the original element type */
				gCmd[ELEMENT_PARAM] = true;
				break;

			case ELEMENT_TEXT:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_TEXT;
				gCmd[ELEMENT_TEXT] = true;
				break;

			case ELEMENT_IMG:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_IMG;
				gCmd[ELEMENT_IMG] = true;
				break;

			case ELEMENT_AUDIO:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_AUDIO;
				gCmd[ELEMENT_AUDIO] = true;
				break;

			case ELEMENT_VIDEO:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_VIDEO;
				gCmd[ELEMENT_VIDEO] = true;
				break;

			case ELEMENT_REF:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_IMG_OR_VIDEO;
				gCmd[ELEMENT_REF] = true;
				break;

			case ELEMENT_ANIMATE:
				if (gMedia) {
					MSG_DEBUG("Media exist");
					return;
				}

				gMedia = (MMS_MEDIA_S *)calloc(1, sizeof(MMS_MEDIA_S));
				gMedia->mediatype = MMS_SMIL_MEDIA_ANIMATE;
				gCmd[ELEMENT_ANIMATE] = true;
				break;

			default:
				MSG_DEBUG("Unsupported element type [%d]", elementType);
				break;
			}

			/* Get Smil Attribute */
			xmlAttr *pAttr = cur_node->properties;

			SMIL_ATTRIBUTE_T paramType = ATTRIBUTE_UNKNOWN;

			for ( ; pAttr; pAttr = pAttr->next) {
				MSG_SEC_DEBUG("## attribute type : name [%s] value [%s] ##", pAttr->name, pAttr->children->content);

				switch (attrType = MsgSmilGetAttrID((char *)pAttr->name)) {
				case ATTRIBUTE_ID: {
						if (gCmd[ELEMENT_REGION] && gRegion) {
							strncpy(gRegion->szID, (char *)pAttr->children->content, MAX_SMIL_REGION_ID - 1);
						} else if (gCmd[ELEMENT_TRANSITION] && gTransition) {
							strncpy(gTransition->szID, (char *)pAttr->children->content, MAX_SMIL_TRANSITION_ID - 1);
						} else if (gCmd[ELEMENT_META] && gMeta) {
							strncpy(gMeta->szID, (char *)pAttr->children->content, MAX_SMIL_META_ID - 1);
						}
					}
					break;

				case ATTRIBUTE_TOP: {
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (gCmd[ELEMENT_REGION] && gRegion) {
							gRegion->nTop.bUnitPercent = bUnitPercent;
							gRegion->nTop.value = value;
						}
					}
					break;

				case ATTRIBUTE_LEFT: {
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (gCmd[ELEMENT_REGION] && gRegion) {
							gRegion->nLeft.bUnitPercent = bUnitPercent;
							gRegion->nLeft.value = value;
						}
					}
					break;


				case ATTRIBUTE_WIDTH: {
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (gCmd[ELEMENT_ROOTLAYOUT]) {
							gRootlayout.width.bUnitPercent = bUnitPercent;
							gRootlayout.width.value = value;
						} else if (gCmd[ELEMENT_REGION] && gRegion) {
							gRegion->width.bUnitPercent = bUnitPercent;
							gRegion->width.value = value;
						}
					}
					break;

				case ATTRIBUTE_HEIGHT: {
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (gCmd[ELEMENT_ROOTLAYOUT]) {
							gRootlayout.height.bUnitPercent = bUnitPercent;
							gRootlayout.height.value = value;
						} else if (gCmd[ELEMENT_REGION] && gRegion) {
							gRegion->height.bUnitPercent = bUnitPercent;
							gRegion->height.value = value;
						}
					}
					break;

				case ATTRIBUTE_FIT:
					if (gCmd[ELEMENT_REGION] && gRegion) {
						if (!strcmp((char *)pAttr->children->content, "meet")) {
							gRegion->fit = MMSUI_IMAGE_REGION_FIT_MEET;
						} else {
							gRegion->fit = MMSUI_IMAGE_REGION_FIT_HIDDEN;
						}
					}
					break;

				case ATTRIBUTE_BGCOLOR:
					if (gCmd[ELEMENT_ROOTLAYOUT]) {
						gRootlayout.bBgColor = true;
						gRootlayout.bgColor = MsgSmilGetColorValue(pAttr->children->content);
					} else if (gCmd[ELEMENT_REGION] && gRegion) {
						gRegion->bBgColor = true;
						gRegion->bgColor = MsgSmilGetColorValue(pAttr->children->content);
					} else if (gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.nBgColor = MsgSmilGetColorValue(pAttr->children->content);
					} else if (gMedia) {
						gMedia->sMedia.sAVI.nBgColor = MsgSmilGetColorValue(pAttr->children->content);
					}

					break;

				case ATTRIBUTE_DUR:
					if (gCmd[ELEMENT_PAR] && gPage) {
						if (elementType == ELEMENT_PAR)
							gPage->nDur =  MsgSmilGetTime((char *)pAttr->children->content);
					} else if (gCmd[ELEMENT_TRANSITION] && gTransition) {
						gTransition->nDur =  MsgSmilGetTime((char *)pAttr->children->content);
					} else if (gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.nDurTime =  MsgSmilGetTime((char *)pAttr->children->content);
					} else if (gMedia) {
						gMedia->sMedia.sAVI.nDurTime =  MsgSmilGetTime((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_SRC: {
					if (gMedia) {
						char szContentID[MSG_MSG_ID_LEN + 1] = {0, };
						int cLen;

						snprintf(szContentID, sizeof(szContentID), "%s", (char *)pAttr->children->content);

						cLen = strlen(szContentID);
						if (!strncasecmp(szContentID, "cid:", 4)) {
							strncpy(gMedia->szContentID, szContentID + 4, cLen - 4);
							gMedia->szContentID[cLen - 4] = '\0';
						} else {
							strncpy(gMedia->szContentID, szContentID, cLen);
							gMedia->szContentID[cLen] = '\0';
						}
					}
					break;
				}
				case ATTRIBUTE_COLOR:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.nColor = MsgSmilGetColorValue(pAttr->children->content);
					break;

				case ATTRIBUTE_SIZE:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.nSize = atoi((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_BOLD:
					if (gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.bBold = MsgSmilGetFontAttrib((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_UNDERLINE:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.bUnderLine = MsgSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_ITALIC:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.bItalic = MsgSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_REVERSE:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.bReverse = MsgSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_DIRECTION:
					if (gCmd[ELEMENT_TEXT] && gMedia)
						gMedia->sMedia.sText.nDirection = MsgSmilGetFontDirection((char *)pAttr->children->content);
					break;
				case ATTRIBUTE_REGION:
					if (gMedia) {
						strncpy(gMedia->regionId, (char *)pAttr->children->content, MAX_SMIL_REGION_ID - 1);
					}
					break;

				case ATTRIBUTE_TRANSIN:
					if (gMedia) {
						if (gCmd[ELEMENT_TEXT] )
							strncpy(gMedia->sMedia.sText.szTransInId, (char *)pAttr->children->content, MAX_SMIL_TRANSIN_ID - 1);
						else
							strncpy(gMedia->sMedia.sAVI.szTransInId, (char *)pAttr->children->content, MAX_SMIL_TRANSIN_ID - 1);
					}
					break;

				case ATTRIBUTE_TRANSOUT:
					if (gMedia) {
						if (gCmd[ELEMENT_TEXT])
							strncpy(gMedia->sMedia.sText.szTransOutId, (char *)pAttr->children->content, MAX_SMIL_TRANSOUT_ID - 1);
						else
							strncpy(gMedia->sMedia.sAVI.szTransOutId, (char *)pAttr->children->content, MAX_SMIL_TRANSOUT_ID - 1);
					}
					break;

				case ATTRIBUTE_BEGIN:
					if (gMedia) {
						if (gCmd[ELEMENT_TEXT])
							gMedia->sMedia.sText.nBegin = MsgSmilGetTime((char *)pAttr->children->content);
						else
							gMedia->sMedia.sAVI.nBegin = MsgSmilGetTime((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_END:
					if (gMedia) {
						if (gCmd[ELEMENT_TEXT])
							gMedia->sMedia.sText.nEnd = MsgSmilGetTime((char *)pAttr->children->content);
						else
							gMedia->sMedia.sAVI.nEnd = MsgSmilGetTime((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_REPEAT_COUNT:
					if (gMedia) {
						if (gCmd[ELEMENT_TEXT])
							gMedia->sMedia.sText.nRepeat = atoi((char *)pAttr->children->content);
						else
							gMedia->sMedia.sAVI.nRepeat = atoi((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_NAME:
					if (gCmd[ELEMENT_PARAM]) {
						if (!strcmp((char *)pAttr->children->content, "foreground-color") || !strcmp((char *)pAttr->children->content, "foregroundcolor"))
							paramType = ATTRIBUTE_FGCOLOR;
						else if (!strcmp((char *)pAttr->children->content, "background-color") || !strcmp((char *)pAttr->children->content, "backgroundcolor"))
							paramType = ATTRIBUTE_BGCOLOR;
						else if (!strcmp((char *)pAttr->children->content, "textsize"))
							paramType = ATTRIBUTE_SIZE;
						else if (!strcmp((char *)pAttr->children->content, "textattribute"))
							paramType = ATTRIBUTE_TEXTFORMAT;

					} else if (gCmd[ELEMENT_META] && gMeta) {
						strncpy(gMeta->szName, (char *)pAttr->children->content, MAX_SMIL_META_NAME - 1);
					}
					break;

				case ATTRIBUTE_VALUE:

					if (paramType == ATTRIBUTE_SIZE && gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.nSize = MsgSmilGetFontSizeValue((char *)pAttr->children->content);
					} else if (paramType == ATTRIBUTE_FGCOLOR && gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.nColor =  MsgSmilGetColorValue(pAttr->children->content);
					} else if (paramType == ATTRIBUTE_BGCOLOR && gCmd[ELEMENT_TEXT] && gMedia) {
						gMedia->sMedia.sText.nBgColor =  MsgSmilGetColorValue(pAttr->children->content);
					} else if (paramType == ATTRIBUTE_TEXTFORMAT && gCmd[ELEMENT_TEXT] && gMedia) {
						MmsSmilFontType fontType;

						fontType = MsgSmilGetFontTypeValue((char *)pAttr->children->content);

						if (fontType == MMS_SMIL_FONT_TYPE_BOLD)
							gMedia->sMedia.sText.bBold = true;
						else
							gMedia->sMedia.sText.bBold = false;

						if (fontType == MMS_SMIL_FONT_TYPE_ITALIC)
							gMedia->sMedia.sText.bItalic = true;
						else
							gMedia->sMedia.sText.bItalic = false;

						if (fontType == MMS_SMIL_FONT_TYPE_UNDERLINE)
							gMedia->sMedia.sText.bUnderLine = true;
						else
							gMedia->sMedia.sText.bUnderLine = false;
					}
					paramType = ATTRIBUTE_UNKNOWN;
					break;

				case ATTRIBUTE_ALT:
					if (gMedia) {
						strncpy(gMedia->szAlt, (char *)pAttr->children->content, MAX_SMIL_ALT_LEN - 1);
					}
					break;

				case ATTRIBUTE_TYPE:
					if (gTransition) {
						gTransition->nType = (MmsSmilTransType)atoi((char *)pAttr->children->content);

						switch (gTransition->nType) {
						case MMS_SMIL_TRANS_SLIDEWIPE:
							gTransition->nSubType = MMS_SMIL_TRANS_SUB_FROM_LEFT;
							break;
						case MMS_SMIL_TRANS_BARWIPE:
							gTransition->nSubType = MMS_SMIL_TRANS_SUB_TOP_TO_BOTTOM;
							break;
						case MMS_SMIL_TRANS_BARNDOORWIPE:
							gTransition->nSubType = MMS_SMIL_TRANS_SUB_HORIZONTAL;
							break;
						default:
							gTransition->nSubType = MMS_SMIL_TRANS_SUB_NONE;
							break;
						}
					}
					break;

				case ATTRIBUTE_SUBTYPE:
					if (gTransition)
						gTransition->nSubType = (MmsSmilTransSubType)atoi((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_CONTENT:
					if (gMeta)
						strncpy(gMeta->szContent, (char *)pAttr->children->content, MAX_SMIL_META_CONTENT - 1);
					break;

				default:
					MSG_DEBUG("Undefined Attribute was found!!!!!");
					break;
				}
			}

			if (cur_node->children) { /* child first */
				MsgSmilParseNode(pMmsMsg, cur_node->children, depth + 1);
			}

			if (elementType == ELEMENT_REGION && gRegion) {
				_MsgMmsAddRegion(pMmsMsg, gRegion);
				gRegion = NULL;
			} else if (elementType ==  ELEMENT_PAR && gPage) {
				_MsgMmsAddPage(pMmsMsg, gPage);
				gPage = NULL;
			} else if ((elementType == ELEMENT_TEXT || elementType == ELEMENT_IMG || elementType == ELEMENT_AUDIO || elementType == ELEMENT_VIDEO || elementType == ELEMENT_ANIMATE || elementType == ELEMENT_REF)
					&& gCmd[ELEMENT_PAR] && gPage && gMedia) {
				_MsgMmsAddMedia(gPage, gMedia);
				gMedia = NULL;
			} else if (elementType == ELEMENT_ROOTLAYOUT) {
				_MsgMmsSetRootLayout(pMmsMsg, &gRootlayout);
			} else if (elementType == ELEMENT_TRANSITION  && gTransition) {
				_MsgMmsAddTransition(pMmsMsg, gTransition);
				gTransition = NULL;
			} else if (elementType == ELEMENT_META && gMeta) {
				_MsgMmsAddMeta(pMmsMsg, gMeta);
				gMeta = NULL;
			}

			if (elementType >= ELEMENT_SMIL)
				gCmd[elementType] = false;

			paramType = ATTRIBUTE_UNKNOWN;
		}
	}

	MSG_END();
}

bool MsgSmilParseSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, const char *pSmilDoc)
{
	MSG_BEGIN();

	xmlDocPtr doc;
	xmlNodePtr cur;

	if (pSmilDoc == NULL || strlen(pSmilDoc) == 0) {
		MSG_DEBUG("Invalid Parameter : pSmilDoc [%p]", pSmilDoc);
		return false;
	}

	MSG_SEC_INFO("Parse Smil : [%s]", pSmilDoc);

	doc = xmlParseMemory(pSmilDoc, strlen(pSmilDoc));
	if (doc == NULL) {
		MSG_DEBUG("Failed xmlParseMemory");
		return false;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		MSG_DEBUG("Failed xmlDocGetRootElement");
		xmlFreeDoc(doc);
		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "smil")) {
		MSG_DEBUG("document of the wrong type, root node != smil");
		xmlFreeDoc(doc);
		return false;
	}

	MsgSmilParseNode(pstMsgMmsBody, cur, 0);

	xmlFreeDoc(doc);
	MSG_END();
	return true;
}

bool MsgSmilGenerateSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, char **ppSmilDoc)
{
	MSG_BEGIN();

	HMsgSmil hSmilDoc = INVALID_HOBJ;
	int nIndex;
	int nMediaIndex;
	int nTotalPageNum;
	int nTotalMediaNum;
	int nRegionCount;
	MMS_PAGE_S *pstPage;
	MMS_MEDIA_S *pstMedia;
	MMS_SMIL_REGION *pstRegion;
	char *pszRawData;

	hSmilDoc = MsgSmilCreateEmptySmilDoc();
	if (INVALID_HOBJ == hSmilDoc) {
		MSG_DEBUG("Invalid SmilDoc[%d]", hSmilDoc);
		return false;
	}
	/* Add Root Layout to Smil Document */
	if (false == MsgSmilAddRootLayout(hSmilDoc, &(pstMsgMmsBody->rootlayout))) {
		MSG_DEBUG("MsgSmilAddRootLayout Failed");
		MsgSmilDestroyDoc(hSmilDoc);
	}
	/* Add Region list to Smil Document */
	nRegionCount = pstMsgMmsBody->regionCnt;
	MSG_DEBUG("Region Count = [%d]", nRegionCount);
	for (nIndex = 0; nIndex < nRegionCount; nIndex++) {
		pstRegion = _MsgMmsGetSmilRegion(pstMsgMmsBody, nIndex);
		if (NULL == pstRegion) {
			MSG_DEBUG("pstRegion is NULL");
			MsgSmilDestroyDoc(hSmilDoc);
			return false;
		}

		if (false == MsgSmilAddRegion(hSmilDoc, pstRegion)) {
			MSG_DEBUG("Adding Region to smil doc failed");
			MsgSmilDestroyDoc(hSmilDoc);
			return false;
		}
	}

	/* Add page list to Smil Document */
	nTotalPageNum = pstMsgMmsBody->pageCnt ;
	MSG_DEBUG("Page Count = [%d]", nTotalPageNum);
	for (nIndex = 0; nIndex < nTotalPageNum; nIndex++) {
		pstPage = _MsgMmsGetPage(pstMsgMmsBody, nIndex);
		if (NULL == pstPage) {
			MSG_DEBUG("pstPage is NULL");
			MsgSmilDestroyDoc(hSmilDoc);
			return false;
		}

		/* Add page to smil doc */
		if (false == MsgSmilAddPage(hSmilDoc, pstPage)) {
			MSG_DEBUG("Adding page to smil doc failed");
			MsgSmilDestroyDoc(hSmilDoc);
			return false;
		}

		nTotalMediaNum = pstPage->mediaCnt;
		MSG_DEBUG("Media Count = [%d]", nTotalMediaNum);
		for (nMediaIndex = 0; nMediaIndex < nTotalMediaNum; nMediaIndex++) {
			pstMedia = _MsgMmsGetMedia(pstPage, nMediaIndex);
			if (NULL == pstMedia) {
				MSG_DEBUG("pMedia is NULL");
				MsgSmilDestroyDoc(hSmilDoc);
				return false;
			}

			if (false == MsgSmilAddMedia(hSmilDoc, nIndex, nMediaIndex, pstMedia, pstMedia->szContentID)) {
				MSG_DEBUG("MsgSmilAddMedia failed");
				MsgSmilDestroyDoc(hSmilDoc);
				return false;
			}
		}
	}

	pszRawData = MsgSmilGetRawData(hSmilDoc);
	if (NULL == pszRawData) {
		MSG_DEBUG("MsgSmilGetRawData failed");
		MsgSmilDestroyDoc(hSmilDoc);
		return false;
	}

	if (ppSmilDoc) {
		*ppSmilDoc = pszRawData;
		MSG_SEC_INFO("Generated Smil : [%s]", pszRawData);
	}

	MsgSmilDestroyDoc(hSmilDoc);
	MSG_END();
	return true;
}
