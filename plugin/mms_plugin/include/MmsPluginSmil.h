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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "MsgTypes.h"
#include "MsgMmsTypes.h"
#include "MmsPluginCodec.h"

#ifndef MMS_PLUGIN_SMIL_H
#define MMS_PLUGIN_SMIL_H

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

typedef	int	HMmsSmil;			// SmilDoc Handle

#define MMS_SMIL_MAX_DOC	1
#define MMS_SMIL_COLOR_SIZE	10
#define INVALID_HOBJ	-1
#define SP_NO_COLOR_SET	-1
#define SP_BLACK	0

/* Structures */
/**
 *	@brief	Represents Smil Doc Information. \n
 */
typedef struct _MmsSmilDoc {
	xmlDocPtr pSmilDoc;
	xmlNodePtr pstRootNode;
} MmsSmilDoc;

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
#ifdef MMS_SMIL_ANIMATE
	ATTRIBUTE_ATTRIBUTE_NAME,
	ATTRIBUTE_ATTRIBUTE_TYPE,
	ATTRIBUTE_TARGET_ELEMENT,
	ATTRIBUTE_FROM,
	ATTRIBUTE_TO,
	ATTRIBUTE_BY,
	ATTRIBUTE_VALUES,
	ATTRIBUTE_CALCMODE,
#endif
} SMIL_ATTRIBUTE_T;

char *MmsSmilGetPresentationData(MSG_MESSAGE_ID_T msgId);
bool MmsSmilParseSmilDoc(MMS_MESSAGE_DATA_S *pMmsMsg, char *pSmilDoc);
void MmsSmilGetElement(MMS_MESSAGE_DATA_S *pMmsMsg, xmlNode *a_node);
int MmsSmilGetColorValue(xmlChar *content);
int MmsSmilGetTime(char *pValue);
int MmsSmilAtoIHexa(char *pInput);
#ifndef __SUPPORT_DRM__
int MmsSmilGetMediaSrcForNormalMsg(char *szOutbuf, char *szInBuf, MsgMultipart *pPart);
#else
int MmsSmilGetMediaSrcForNormalMsg(char *szOutbuf, char *szInBuf, MsgMultipart *pPart, MMS_MEDIA_S *pMedia);
#endif
int MmsSmilGetElementID(char *pString);
int MmsSmilGetAttrID(char *pString);
bool MmsSmilGetFontAttrib(char *pString);
MmsTextDirection MmsSmilGetFontDirection(char *pString);
int MmsSmilGetFontSizeValue(char *pString);
MmsSmilFontType MmsSmilGetFontTypeValue(char *pString);
bool MmsSmilGetMediaFilePath(MMS_MEDIA_S *pMedia, char *pszTemp, int msgID);
bool __MmsGetRealFileName(MmsSmilMediaType mediaType, char *pszSrc, char *pszName, int msgID);
/**	@fn		bool MMSGenerateSmilBuffer(MMS_MESSAGE_DATA_S *pstMsgBody)
 *	@brief	Forms Smil Buffer using pstMsgBody. \n
 *	@param[in/out]	pstMsgBody is Message handle. \n
 *	@retval	TRUE				In case of Success. \n
 *	@retval	FALSE				In case of failure. \n
 */
bool MMSGenerateSmilBuffer(MMS_MESSAGE_DATA_S *pstMsgBody);

/**	@fn		HMmsSmil MmsSmilCreateEmptySmilDoc(void)
 *	@brief	Creates default Smil Doc based on input gszEmptyRawDoc. \n
 *	@retval	Returns Smil Document number. \n
 */
HMmsSmil MmsSmilCreateEmptySmilDoc(void);

 /**	@fn			HMmsSmil MmsSmilCreateSmilDoc(char *pszRawData)
 *	@brief		Creates Smil Doc based on input pszRawData. \n
 *	@param[in]	pszRawData is smil buffer. \n
 *	@retval		Returns Smil Document number. \n
 */
HMmsSmil MmsSmilCreateSmilDoc(char *pszRawData);

 /**	@fn			BOOL MmsSmilDestroyDoc(HMmsSmil hSmilDoc)
 *	@brief		Destroys Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval	TRUE				In case of Success. \n
 *	@retval	FALSE				In case of failure. \n
 */
bool MmsSmilDestroyDoc(HMmsSmil hSmilDoc);

/**	@fn			static bool IsValidSmilDocNo(int nSmilDocNo)
 *	@brief		Form Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval		Returns Smil Buffer	In case of success. \n
 *	@retval		Returns NULL			In case of failure. \n
 */
bool IsValidSmilDocNo(int nSmilDocNo);

/**	@fn			static char * MmsSmilGetRawData( HMmsSmil hSmilDoc )
 *	@brief		Form Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval		Returns Smil Buffer		In case of success. \n
 *	@retval		Returns NULL			In case of failure. \n
 */
char *MmsSmilGetRawData(HMmsSmil hSmilDoc);

/**	@fn			static bool MmsSmilAddPage(HMmsSmil hSmilDoc, MMS_PAGE_S *pstSmilPage)
 *	@brief		Add Page to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilPage specifies page information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddPage(HMmsSmil hSmilDoc, MMS_PAGE_S *pstSmilPage);

/**	@fn			static bool MmsSmilAddRootLayout(HMmsSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout)
 *	@brief		Add Rootlayout to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilRootLayout specifies RootLayout information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddRootLayout(HMmsSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout);

/**	@fn			static bool MmsSmilAddRegion(HMmsSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion)
 *	@brief		Add Region to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilRegion specifies Region information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddRegion(HMmsSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion);

/**	@fn			static bool MmsSmilAddMedia( HMmsSmil hSmilDoc, int nPageNo, MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Add Media to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	nPageNo specifies page number to which media belongs. \n
 *	@param[in]	nMediaIdx specifies medi ID. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddMedia(HMmsSmil hSmilDoc, int nPageNo, int nMediaIdx, MMS_MEDIA_S *pstSmilMedia, char *pszContentID);

/**	@fn			static xmlNode *__MmsCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Create Text Element. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		Text Element node		In case of Success. \n
 *	@retval		NULL				In case of failure. \n
 */
xmlNode *__MmsCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID);

/**	@fn			static xmlNode *__MmsCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Create Image/Audio/Video Element. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		Image/Audio/Video Element node	In case of Success. \n
 *	@retval		NULL							In case of failure. \n
 */
xmlNode *__MmsCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID);

/**	@fn			static bool __MmsInsertFirstChild(xmlNode *pstParent, xmlNode *pNode)
 *	@brief		Inserts first child to parent node. \n
 *	@param[in]	pstParent specifies Parent node. \n
 *	@param[in]	pNode specifies Child node. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool __MmsInsertFirstChild(xmlNode *pParent, xmlNode *pNode);

/**	@fn			static bool __MmsSmilInsertNode(xmlNode *pstParent, xmlNode *pstLeftSibling, xmlNode *pNode)
 *	@brief		Inserts node. \n
 *	@param[in]	pstParent specifies Parent node. \n
 *	@param[in]	pstLeftSibling specifies Left Sibling node. \n
 *	@param[in]	pNode specifies Child node. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool __MmsSmilInsertNode(xmlNode *pParent, xmlNode *pLeftSibling, xmlNode *pNode);

/**	@fn			static void _MmsSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue)
 *	@brief		Sets Attribute. \n
 *	@param[in]	pNode specifies node. \n
 *	@param[in]	szField specifies attribute field. \n
 *	@param[in]	szValue specifies value of field \n
 */
void _MmsSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue);

/**	@fn			static char *__MmsSmilFindColorValue(int nValue)
 *	@brief		Converts color to RGB. \n
 *	@param[in]	nValue specifies color value. \n
 *	@retval		RGB value. \n
 */
char *__MmsSmilFindColorValue(int nValue);

/**	@fn			static xmlNodePtr UtilxmlStringGetNodeList(xmlNodePtr pstNode, char *pszName)
 *	@brief		Get node based on pszValue. \n
 *	@param[in]	pNode specifies node. \n
 *	@param[in]	pszName specifies name field. \n
 *	@retval		RGB value. \n
 */
xmlNodePtr UtilxmlStringGetNodeList(xmlNodePtr pstNode, char *pszName);

#endif//MMS_PLUGIN_SMIL_H


