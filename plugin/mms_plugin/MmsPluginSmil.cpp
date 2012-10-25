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

#include<stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "MmsPluginSmil.h"
#include "MmsPluginMessage.h"
#include "MmsPluginStorage.h"
#include "MsgDebug.h"
#include "MmsPluginCodec.h"
#include "MsgMmsMessage.h"

#include "MsgTypes.h"
#include "MmsPluginSetup.h"
#include "MsgUtilFile.h"

/* static variables */
static char gszEmptyRawDoc[] = "<smil><head><layout></layout></head><body></body></smil>";
static MmsSmilDoc *__gpaMmsSmilDoc[MMS_SMIL_MAX_DOC]={NULL, };
static char gszColor[MMS_SMIL_COLOR_SIZE] = {0, };


char *MmsSmilGetPresentationData(msg_message_id_t msgId)
{
	MmsMsg *pMsg;

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);

	if (pMsg == NULL) {
		MSG_DEBUG("pMsg is NULL");
		goto _LCATCH;
	}

	if (msgId != pMsg->msgID) {
		MSG_DEBUG("Invalid Message Id");
		return NULL;
	}

	if (!pMsg->msgBody.pPresentationBody)
		goto _LCATCH;

	if (!pMsg->msgBody.pPresentationBody->body.pText)
		goto _LCATCH;
	else
		return pMsg->msgBody.pPresentationBody->body.pText;

_LCATCH:
	return NULL;
}

bool MmsSmilParseSmilDoc(MMS_MESSAGE_DATA_S *pMmsMsg, char *pSmilDoc)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	MSG_DEBUG("%s", pSmilDoc);
	doc = xmlParseMemory(pSmilDoc, strlen(pSmilDoc));

	if (doc == NULL) {
		MSG_DEBUG("Document not parsed successfully. \n");
		return false;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		MSG_DEBUG("empty document\n");
		xmlFreeDoc(doc);
		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "smil")) {
		MSG_DEBUG("document of the wrong type, root node != smil");
		xmlFreeDoc(doc);
		return false;
	}

	MmsSmilGetElement(pMmsMsg, cur);

	xmlFreeDoc(doc);

	return true;
}

void MmsSmilGetElement(MMS_MESSAGE_DATA_S *pMmsMsg, xmlNode *a_node)
{
	MSG_BEGIN();

	int elementType;
	int attrType;
	MMS_SMIL_ROOTLAYOUT rootlayout = {};
	static bool cmd[ELEMENT_MAX] = {false, };
	static MMS_SMIL_REGION *pRegion;
	static MMS_PAGE_S *pPage;
	static MMS_MEDIA_S *pMedia;
	static MMS_SMIL_TRANSITION *pTransition;
	static MMS_SMIL_META *pMeta;

	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		MSG_DEBUG("******* node, name: %s ***\n", cur_node->name);

		if (cur_node->type == XML_ELEMENT_NODE) {
			// Get Smil Element =====================================================
			MSG_DEBUG("*** node type: Element, name: %s ***\n", cur_node->name);

			switch (elementType = MmsSmilGetElementID((char *)cur_node->name)) {
			case ELEMENT_ROOTLAYOUT:
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_ROOTLAYOUT] = true;
				break;

			case ELEMENT_REGION:
				pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_REGION] = true;
				break;

			case ELEMENT_TRANSITION:
				pTransition = (MMS_SMIL_TRANSITION *)calloc(sizeof(MMS_SMIL_TRANSITION), 1);
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_TRANSITION] = true;
				break;

			case ELEMENT_META:
				pMeta = (MMS_SMIL_META *)calloc(sizeof(MMS_SMIL_META), 1);
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_META] = true;
				break;

			case ELEMENT_PAR:
				pPage = (MMS_PAGE_S *)calloc(sizeof(MMS_PAGE_S), 1);
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_PAR] = true;
				break;

			case ELEMENT_PARAM: // Need to check the original element type
				break;

			case ELEMENT_TEXT:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_TEXT;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_TEXT] = true;
				break;

			case ELEMENT_IMG:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_IMG;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_IMG] = true;
				break;

			case ELEMENT_AUDIO:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_AUDIO;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_AUDIO] = true;
				break;

			case ELEMENT_VIDEO:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_VIDEO;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_VIDEO] = true;
				break;

			case ELEMENT_REF:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_IMG_OR_VIDEO;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_REF] = true;
				break;

			case ELEMENT_ANIMATE:
				pMedia = (MMS_MEDIA_S *)calloc(sizeof(MMS_MEDIA_S), 1);
				pMedia->mediatype = MMS_SMIL_MEDIA_ANIMATE;
				memset(cmd, 0, ELEMENT_MAX);
				cmd[ELEMENT_ANIMATE] = true;
				break;

			default:
				memset(cmd, 0, ELEMENT_MAX);
				break;
			}

			//Get Smil Attribute =====================================================
			xmlAttr *pAttr = cur_node->properties;
			SMIL_ATTRIBUTE_T paramType = ATTRIBUTE_UNKNOWN;

			for ( ; pAttr; pAttr = pAttr->next) {
				MSG_DEBUG("AttributeType: (%s, %s) ", pAttr->name, pAttr->children->content);
				switch (attrType = MmsSmilGetAttrID((char *)pAttr->name)) {
				case ATTRIBUTE_ID:
					{
						if (cmd[ELEMENT_REGION]) {
							strncpy(pRegion->szID, (char *)pAttr->children->content, MAX_SMIL_REGION_ID - 1);
						} else if (cmd[ELEMENT_TRANSITION]) {
							strncpy(pTransition->szID, (char *)pAttr->children->content, MAX_SMIL_TRANSITION_ID - 1);
						} else if (cmd[ELEMENT_META]) {
							strncpy(pMeta->szID, (char *)pAttr->children->content, MAX_SMIL_META_ID - 1);
						}
					}
					break;

				case ATTRIBUTE_TOP:
					{
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (cmd[ELEMENT_REGION]) {
							pRegion->nTop.bUnitPercent = bUnitPercent;
							pRegion->nTop.value = value;
						}
					}
					break;

				case ATTRIBUTE_LEFT:
					{
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (cmd[ELEMENT_REGION]) {
							pRegion->nLeft.bUnitPercent = bUnitPercent;
							pRegion->nLeft.value = value;
						}
					}
					break;


				case ATTRIBUTE_WIDTH:
					{
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (cmd[ELEMENT_ROOTLAYOUT]) {
							rootlayout.width.bUnitPercent = bUnitPercent;
							rootlayout.width.value = value;
						} else if (cmd[ELEMENT_REGION]) {
							pRegion->width.bUnitPercent = bUnitPercent;
							pRegion->width.value = value;
						}
					}
					break;

				case ATTRIBUTE_HEIGHT:
					{
						int bUnitPercent;
						int value;

						if (strchr((char *)pAttr->children->content, '%'))
							bUnitPercent = true;
						else
							bUnitPercent = false;

						value = atoi((char *)pAttr->children->content);

						if (cmd[ELEMENT_ROOTLAYOUT]) {
							rootlayout.height.bUnitPercent = bUnitPercent;
							rootlayout.height.value = value;
						} else if (cmd[ELEMENT_REGION]) {
							pRegion->height.bUnitPercent = bUnitPercent;
							pRegion->height.value = value;
						}
					}
					break;

				case ATTRIBUTE_FIT:
					if (cmd[ELEMENT_REGION]) {
						if (!strcmp((char *)pAttr->children->content, "meet")) {
							pRegion->fit = MMSUI_IMAGE_REGION_FIT_MEET;
						} else {
							pRegion->fit = MMSUI_IMAGE_REGION_FIT_HIDDEN;
						}
					}
					break;

				case ATTRIBUTE_BGCOLOR:
					if (cmd[ELEMENT_ROOTLAYOUT])
						rootlayout.bgColor = MmsSmilGetColorValue(pAttr->children->content);
					else if (cmd[ELEMENT_REGION])
						pRegion->bgColor = MmsSmilGetColorValue(pAttr->children->content);
					else if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nBgColor = MmsSmilGetColorValue(pAttr->children->content);
					else
						pMedia->sMedia.sAVI.nBgColor = MmsSmilGetColorValue(pAttr->children->content);

					break;

				case ATTRIBUTE_DUR:
					if (cmd[ELEMENT_PAR])
						pPage->nDur =  MmsSmilGetTime((char *)pAttr->children->content);
					else if (cmd[ELEMENT_TRANSITION])
						pTransition->nDur =  MmsSmilGetTime((char *)pAttr->children->content);
					else if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nDurTime =  MmsSmilGetTime((char *)pAttr->children->content);
					else
						pMedia->sMedia.sAVI.nDurTime =  MmsSmilGetTime((char *)pAttr->children->content);

					break;

				case ATTRIBUTE_SRC:
					{
						char *szSrc;
						char szTmpSrc[MSG_FILEPATH_LEN_MAX] = {0,};
						char szOutBuf[MSG_FILEPATH_LEN_MAX] = {0, };
						int cLen;
						MsgMultipart *pPart = NULL;
						MmsMsg *pMsg;

						szSrc = MsgChangeHexString((char *)pAttr->children->content);

						memcpy(pMedia->szSrc, szSrc, strlen(szSrc) + 1);
						free(szSrc);

						cLen = strlen(pMedia->szSrc);
						if (!strncasecmp(pMedia->szSrc, "cid:", 4)) {
							strncpy(szTmpSrc, pMedia->szSrc + 4, cLen - 4);
							szTmpSrc[cLen - 4] = '\0';
						} else {
							strncpy(szTmpSrc, pMedia->szSrc, cLen);
							szTmpSrc[cLen] = '\0';
						}

						MmsPluginStorage::instance()->getMmsMessage(&pMsg);
						pPart = pMsg->msgBody.body.pMultipart;
#ifndef __SUPPORT_DRM__
						MmsSmilGetMediaSrcForNormalMsg(szOutBuf, szTmpSrc, pPart);
#else
						MmsSmilGetMediaSrcForNormalMsg(szOutBuf, szTmpSrc, pPart, pMedia);
#endif

						strcpy(pMedia->szSrc, szOutBuf);
						MmsSmilGetMediaFilePath(pMedia, szTmpSrc, pMsg->msgID);
					}
					break;

				case ATTRIBUTE_COLOR:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nColor = MmsSmilGetColorValue(pAttr->children->content);
					break;

				case ATTRIBUTE_SIZE:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nSize = atoi((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_BOLD:
					if (cmd[ELEMENT_TEXT]) {
						pMedia->sMedia.sText.bBold = MmsSmilGetFontAttrib((char *)pAttr->children->content);
					}
					break;

				case ATTRIBUTE_UNDERLINE:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.bUnderLine = MmsSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_ITALIC:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.bItalic = MmsSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_REVERSE:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.bReverse = MmsSmilGetFontAttrib((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_DIRECTION:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nDirection = MmsSmilGetFontDirection((char *)pAttr->children->content);
					break;
				case ATTRIBUTE_REGION:
					strncpy(pMedia->regionId, (char *)pAttr->children->content, MAX_SMIL_REGION_ID - 1);
					break;

				case ATTRIBUTE_TRANSIN:
					if (cmd[ELEMENT_TEXT])
						strncpy(pMedia->sMedia.sText.szTransInId, (char *)pAttr->children->content, MAX_SMIL_TRANSIN_ID - 1);
					else
						strncpy(pMedia->sMedia.sAVI.szTransInId, (char *)pAttr->children->content, MAX_SMIL_TRANSIN_ID - 1);
					break;

				case ATTRIBUTE_TRANSOUT:
					if (cmd[ELEMENT_TEXT])
						strncpy(pMedia->sMedia.sText.szTransOutId, (char *)pAttr->children->content, MAX_SMIL_TRANSOUT_ID - 1);
					else
						strncpy(pMedia->sMedia.sAVI.szTransOutId, (char *)pAttr->children->content, MAX_SMIL_TRANSOUT_ID - 1);
					break;

				case ATTRIBUTE_BEGIN:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nBegin = MmsSmilGetTime((char *)pAttr->children->content);
					else
						pMedia->sMedia.sAVI.nBegin = MmsSmilGetTime((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_END:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nEnd = MmsSmilGetTime((char *)pAttr->children->content);
					else
						pMedia->sMedia.sAVI.nEnd = MmsSmilGetTime((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_REPEAT_COUNT:
					if (cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nRepeat = atoi((char *)pAttr->children->content);
					else
						pMedia->sMedia.sAVI.nRepeat = atoi((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_NAME:
					if (!strcmp((char *)pAttr->children->content, "foreground-color") || !strcmp((char *)pAttr->children->content, "foregroundcolor"))
						paramType = ATTRIBUTE_FGCOLOR;
					else if (!strcmp((char *)pAttr->children->content, "background-color") || !strcmp((char *)pAttr->children->content, "backgroundcolor"))
						paramType = ATTRIBUTE_BGCOLOR;
					else if (!strcmp((char *)pAttr->children->content, "textsize"))
						paramType = ATTRIBUTE_SIZE;
					else if (!strcmp((char *)pAttr->children->content, "textattribute"))
						paramType = ATTRIBUTE_TEXTFORMAT;

					if (cmd[ELEMENT_META])
						strncpy(pMeta->szName, (char *)pAttr->children->content, MAX_SMIL_META_NAME - 1);
					break;

				case ATTRIBUTE_VALUE:
					if (paramType == ATTRIBUTE_SIZE && cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nSize = MmsSmilGetFontSizeValue((char *)pAttr->children->content);
					else if (paramType == ATTRIBUTE_FGCOLOR && cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nColor =  MmsSmilGetColorValue(pAttr->children->content);
					else if (paramType == ATTRIBUTE_BGCOLOR && cmd[ELEMENT_TEXT])
						pMedia->sMedia.sText.nBgColor =  MmsSmilGetColorValue(pAttr->children->content);
					else if (paramType == ATTRIBUTE_TEXTFORMAT && cmd[ELEMENT_TEXT]) {
						MmsSmilFontType fontType;

						fontType = MmsSmilGetFontTypeValue((char *)pAttr->children->content);

						if (fontType == MMS_SMIL_FONT_TYPE_BOLD)
							pMedia->sMedia.sText.bBold = true;
						else
							pMedia->sMedia.sText.bBold = false;

						if (fontType == MMS_SMIL_FONT_TYPE_ITALIC)
							pMedia->sMedia.sText.bItalic = true;
						else
							pMedia->sMedia.sText.bItalic = false;

						if (fontType == MMS_SMIL_FONT_TYPE_UNDERLINE)
							pMedia->sMedia.sText.bUnderLine = true;
						else
							pMedia->sMedia.sText.bUnderLine = false;
					}
					break;

				case ATTRIBUTE_ALT:
					strncpy(pMedia->szAlt, (char *)pAttr->children->content, MAX_SMIL_ALT_LEN - 1);
					break;

				case ATTRIBUTE_TYPE:
					pTransition->nType = (MmsSmilTransType)atoi((char *)pAttr->children->content);

					switch (pTransition->nType) {
					case MMS_SMIL_TRANS_SLIDEWIPE:
						pTransition->nSubType = MMS_SMIL_TRANS_SUB_FROM_LEFT;
						break;
					case MMS_SMIL_TRANS_BARWIPE:
						pTransition->nSubType = MMS_SMIL_TRANS_SUB_TOP_TO_BOTTOM;
						break;
					case MMS_SMIL_TRANS_BARNDOORWIPE:
						pTransition->nSubType = MMS_SMIL_TRANS_SUB_HORIZONTAL;
						break;
					default:
						pTransition->nSubType = MMS_SMIL_TRANS_SUB_NONE;
						break;
					}

					break;

				case ATTRIBUTE_SUBTYPE:
					pTransition->nSubType = (MmsSmilTransSubType)atoi((char *)pAttr->children->content);
					break;

				case ATTRIBUTE_CONTENT:
					strncpy(pMeta->szContent, (char *)pAttr->children->content, MAX_SMIL_META_CONTENT - 1);
					break;
				default:
					MSG_DEBUG("Undefined Attribute was found!!!!!");
				}
			}

			if (paramType == ATTRIBUTE_UNKNOWN && cmd[ELEMENT_REGION]) {
				// Insert a region to region list
				_MsgMmsAddRegion(pMmsMsg, pRegion);
			} else if (paramType == ATTRIBUTE_UNKNOWN && cmd[ELEMENT_PAR]) {
				//Insert a page to page list
				_MsgMmsAddPage(pMmsMsg, pPage);
			} else if (paramType == ATTRIBUTE_UNKNOWN && (cmd[ELEMENT_TEXT] ||cmd[ELEMENT_IMG] ||cmd[ELEMENT_AUDIO] ||cmd[ELEMENT_VIDEO] ||cmd[ELEMENT_ANIMATE])) {
				//Insert a media to media list
				_MsgMmsAddMedia(pPage, pMedia);
			} else if (paramType == ATTRIBUTE_UNKNOWN && cmd[ELEMENT_ROOTLAYOUT]) {
				_MsgMmsSetRootLayout(pMmsMsg, &rootlayout);
			} else if (paramType == ATTRIBUTE_UNKNOWN && cmd[ELEMENT_TRANSITION]) {
				//Insert a transition to transition list
				_MsgMmsAddTransition(pMmsMsg, pTransition);
			} else if (paramType == ATTRIBUTE_UNKNOWN && cmd[ELEMENT_META]) {
				//Insert a meta to meta list
				_MsgMmsAddMeta(pMmsMsg, pMeta);
			}

			paramType = ATTRIBUTE_UNKNOWN;
		}

		MmsSmilGetElement(pMmsMsg, cur_node->children);
	}

	MSG_END();
}


int MmsSmilGetColorValue(xmlChar *content)
{
	int color;

	if (content[0] == '#')	// RGB value
		color = MmsSmilAtoIHexa((char *)&content[1]);
	else if (content[0] == '0' && (content[1] == 'x' || content[1] == 'X'))
		color = MmsSmilAtoIHexa((char *)&content[2]);
	else {
		MSG_DEBUG("Invalid Color Value");
		color = -1;
	}

	return color;
}

int MmsSmilAtoIHexa(char *pInput)
{
	int res = 0;
	int len	= 0;
	int temp = 1;
	int i  = 0;
	int j = 0;
	char *pOutput = NULL;

	MSG_DEBUG("__MmsSmilAtoIHexa() enter..\n");

	len = strlen(pInput);
	pOutput = (char *)malloc(len + 1);

	if (pOutput == NULL) {
		MSG_DEBUG("__MmsSmilAtoIHexa: Memory full \n");
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

int MmsSmilGetTime(char *pValue)
{
	char *pTemp = NULL;
	bool bMSec = false;
	int retVal = 0;
	int i = 0;
	int len = 0;

	if (pValue == NULL || pValue[0] == '\0')
		return 0;

	len = strlen(pValue);

	/* Default time unit -> millisecond */
	if (strstr(pValue, "msec"))
		bMSec = true;

	if (strstr(pValue, "ms"))
		bMSec = true;

	pTemp = (char *)malloc(strlen(pValue) + 1);

	if (NULL == pTemp) {
		MSG_DEBUG("__MmsSmilGetTime : malloc for <time> attribute is failed \n");
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
#ifndef __SUPPORT_DRM__
int MmsSmilGetMediaSrcForNormalMsg(char *szOutbuf, char *szInBuf, MsgMultipart *pPart)
#else
int MmsSmilGetMediaSrcForNormalMsg(char *szOutbuf, char *szInBuf, MsgMultipart *pPart, MMS_MEDIA_S *pMedia)
#endif
{
	char szContentID[MSG_MSG_ID_LEN + 1] = {0, };
	char szContentLI[MSG_MSG_ID_LEN + 1] = {0, };
	int cLen  = 0;
	int nPart = 0;

	MSG_DEBUG("szInBuf: %s", szInBuf);
	while (pPart && pPart->pBody) {
		if (pPart->type.szContentID[0]) {
			cLen = strlen(pPart->type.szContentID);

			if (pPart->type.szContentID[0] == '<' && pPart->type.szContentID[cLen - 1] == '>') {
				strncpy(szContentID, &pPart->type.szContentID[1], cLen - 2);
				szContentID[cLen - 2] = '\0';
			} else if (pPart->type.szContentID[0] == MSG_CH_QUOT && pPart->type.szContentID[cLen-1] == MSG_CH_QUOT) {
				strncpy(szContentID, &pPart->type.szContentID[1], cLen - 2);
				szContentID[cLen - 2] = '\0';
			} else {
				strncpy(szContentID, pPart->type.szContentID, cLen);
				szContentID[cLen] = '\0';
			}
		} else {
			szContentID[0] = '\0';
		}

		MSG_DEBUG("szContentID: %s", szContentID);

		if (pPart->type.szContentLocation[0]) {
			cLen = strlen(pPart->type.szContentLocation);

			if (pPart->type.szContentLocation[0] == MSG_CH_QUOT &&
				pPart->type.szContentLocation[cLen-1] == MSG_CH_QUOT) {
				strncpy(szContentLI, &pPart->type.szContentLocation[1], cLen-2);
				szContentLI[cLen-2] = '\0';
			} else {
				strncpy(szContentLI, pPart->type.szContentLocation, cLen);
				szContentLI[cLen] = '\0';
			}
		} else {
			szContentLI[0] = '\0';
		}

		MSG_DEBUG("szContentLocation: %s", szContentLI);

		if (strcasecmp(szContentID, szInBuf) == 0) {
			strcpy(szOutbuf, pPart->type.param.szFileName);
			MSG_DEBUG("match with szContentID.");
			goto RETURN;
		} else {
			char *szInFileName = strrchr(szInBuf, '/');

			if (szInFileName == NULL) {
				szInFileName = szInBuf;
			} else
				szInFileName++;

			if (strcasecmp(szContentLI, szInFileName) == 0) {
				strcpy(szOutbuf, pPart->type.param.szFileName);
				MSG_DEBUG("match with szContentLI.");
				goto RETURN;
			} else if (strcasecmp(pPart->type.param.szName, szInBuf) == 0) {
				strcpy(szOutbuf, pPart->type.param.szFileName);
				MSG_DEBUG("match with pPart->type.param.szName.");
				goto RETURN;
			} else if (strlen(szContentID) > 4) {
				if (strcasecmp(strtok(szContentID, "."), strtok(szInBuf, ".")) == 0) {
					strcpy(szOutbuf, pPart->type.param.szFileName);
					MSG_DEBUG("only name is match with szContentID.");
					goto RETURN;
				}
			} else if (strlen(szContentLI) > 4) {
				if (strcasecmp(strtok(szContentLI, "."), strtok(szInBuf, ".")) == 0) {
					strcpy(szOutbuf, pPart->type.param.szFileName);
					MSG_DEBUG("only name is match with szContentLI.");
					goto RETURN;
				}
			}
		}

		nPart++;
		pPart = pPart->pNext;
	}

	return -1;

RETURN:
#ifdef __SUPPORT_DRM__
	pMedia->drmType = pPart->type.drmInfo.drmType;

	if (pPart->type.drmInfo.szDrm2FullPath != NULL) {
		MSG_DEBUG("szDrm2FullPath: %s", pPart->type.drmInfo.szDrm2FullPath);
		strncpy(pMedia->szDrm2FullPath, pPart->type.drmInfo.szDrm2FullPath, MSG_FILEPATH_LEN_MAX - 1);
	}
#endif
	return nPart;

}

int	MmsSmilGetElementID(char *pString)
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

int	MmsSmilGetAttrID(char *pString)
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
	else if (!strcmp(pString, "backgroundColor"))
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
	else
		return -1;
}

bool MmsSmilGetFontAttrib(char *pString)
{
	if (!strcmp(pString, "true"))
		return true;
	else
		return false;
}

MmsTextDirection MmsSmilGetFontDirection(char *pString)
{
	MmsTextDirection direction = MMS_TEXT_DIRECTION_INVALID;

	if (!strcmp(pString, "right"))
		direction = MMS_TEXT_DIRECTION_RIGHT;
	else if (!strcmp(pString, "down"))
		direction = MMS_TEXT_DIRECTION_DOWN;

	return direction;
}

int MmsSmilGetFontSizeValue(char *pString)
{
	MSG_DEBUG(" #### content = %s #### ", pString);
	if (!strcmp(pString, "small"))
		return MMS_SMIL_FONT_SIZE_SMALL;
	else if (!strcmp(pString, "normal"))
		return MMS_SMIL_FONT_SIZE_NORMAL;
	else if (!strcmp(pString, "large"))
		return MMS_SMIL_FONT_SIZE_LARGE;
	else
		return atoi(pString);
}

MmsSmilFontType MmsSmilGetFontTypeValue(char *pString)
{
	MSG_DEBUG(" #### content = %s #### ", pString);

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

bool MmsSmilGetMediaFilePath(MMS_MEDIA_S *pMedia, char *pszTemp, int msgID)
{
	if (!pMedia) {
		MSG_DEBUG("pMedia is NULL");
		return false;
	}

	snprintf(pMedia->szFilePath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_DATA_PATH, pMedia->szSrc);
	MSG_DEBUG("pMedia's filePath: %s", pMedia->szFilePath);

	__MmsGetRealFileName(pMedia->mediatype, pMedia->szSrc, pMedia->szFileName, msgID);
	MSG_DEBUG("pMedia's fileName: %s", pMedia->szFileName);

	snprintf(pMedia->szContentID, MSG_MSG_ID_LEN+1, "%s", pszTemp);
	MSG_DEBUG("pMedia's ContentID: %s", pMedia->szContentID);

	return true;
}


/**	@fn		bool MMSGenerateSmilBuffer(MMS_MESSAGE_DATA_S *pstMsgBody)
 *	@brief	Forms Smil Buffer using pstMsgBody. \n
 *	@param[in/out]	pstMsgBody is Message handle. \n
 *	@retval	TRUE				In case of Success. \n
 *	@retval	FALSE				In case of failure. \n
 */
bool MMSGenerateSmilBuffer(MMS_MESSAGE_DATA_S *pstMsgBody)
{
	HMmsSmil hSmilDoc = INVALID_HOBJ;
	int nIndex;
	int nMediaIndex;
	int nTotalPageNum;
	int nTotalMediaNum;
	int nRegionCount;
	MMS_PAGE_S *pstPage;
	MMS_MEDIA_S *pstMedia;
	MMS_SMIL_REGION *pstRegion;
	char *pszRawData;

	MSG_BEGIN();

	hSmilDoc = MmsSmilCreateEmptySmilDoc();
	MSG_DEBUG("Smil Doc =%d",hSmilDoc);
	if (INVALID_HOBJ == hSmilDoc) {
		MSG_DEBUG("Invalid SmilDoc[%d]",hSmilDoc);
		return false;
	}
	// Add Root Layout to Smil Document
	if (false == MmsSmilAddRootLayout(hSmilDoc, &(pstMsgBody->rootlayout))) {
		MSG_DEBUG("MmsSmilAddRootLayout Failed");
		MmsSmilDestroyDoc(hSmilDoc);
	}
	//Add Region list to Smil Document
	nRegionCount = pstMsgBody->regionCnt;
	MSG_DEBUG(" Region Count =%d",nRegionCount);
	for (nIndex = 0; nIndex < nRegionCount; nIndex++) {
		MSG_DEBUG("Calling _MsgMmsGetSmilRegion");
		pstRegion = _MsgMmsGetSmilRegion(pstMsgBody, nIndex);
		if (NULL == pstRegion) {
			MSG_DEBUG("pstRegion is NULL");
			MmsSmilDestroyDoc(hSmilDoc);
			return false;
		}
		MSG_DEBUG("Calling MmsSmilAddRegion");
		if (false == MmsSmilAddRegion(hSmilDoc, pstRegion)) {
			MSG_DEBUG("Adding Region to smil doc failed");
			MmsSmilDestroyDoc(hSmilDoc);
			return false;
		}
	}
	// Add page list to Smil Document
	 nTotalPageNum = pstMsgBody->pageCnt ;
	MSG_DEBUG(" Page Count =%d",nTotalPageNum);
	for (nIndex = 0; nIndex < nTotalPageNum; nIndex++) {
		MSG_DEBUG("Calling _MsgMmsGetPage");
		pstPage = _MsgMmsGetPage(pstMsgBody, nIndex);
		if (NULL == pstPage) {
			MSG_DEBUG("pstPage is NULL");
			MmsSmilDestroyDoc(hSmilDoc);
			return false;
		}
		// Add page to smil doc
		MSG_DEBUG("Calling MmsSmilAddPage");
		if (false == MmsSmilAddPage(hSmilDoc, pstPage)) {
			MSG_DEBUG("Adding page to smil doc failed");
			MmsSmilDestroyDoc(hSmilDoc);
			return false;
		}
		nTotalMediaNum = pstPage->mediaCnt;
		MSG_DEBUG(" Media Count =%d",nTotalMediaNum);
		for (nMediaIndex = 0; nMediaIndex < nTotalMediaNum; nMediaIndex++) {
			MSG_DEBUG("Calling _MsgMmsGetMedia");
			pstMedia = _MsgMmsGetMedia(pstPage, nMediaIndex);
			if (NULL == pstMedia) {
				MSG_DEBUG("pMedia is NULL");
				MmsSmilDestroyDoc(hSmilDoc);
				return false;
			}
			MSG_DEBUG("Calling MmsSmilAddMedia");
			if (false == MmsSmilAddMedia(hSmilDoc, nIndex, nMediaIndex, pstMedia, pstMedia->szContentID)) {
				MSG_DEBUG("MmsSmilAddMedia failed");
				MmsSmilDestroyDoc(hSmilDoc);
				return false;
			}
		}
	}
	MSG_DEBUG("MMSGenerateSmilBuffer: Start update template");
	pszRawData = MmsSmilGetRawData(hSmilDoc);
	if (NULL == pszRawData) {
		MSG_DEBUG("MMSGenerateSmilBuffer: MmsSmilGetRawData failed");
		MmsSmilDestroyDoc(hSmilDoc);
		return false;
	}

	char fullpath[MSG_FILEPATH_LEN_MAX] = {0,};
	snprintf(fullpath, MSG_FILEPATH_LEN_MAX, MSG_SMIL_FILE_PATH"%s", pstMsgBody->szSmilFilePath);

	if (MsgWriteSmilFile(fullpath, pszRawData, strlen(pszRawData) + 1) == false) {
		MSG_DEBUG("MMSGenerateSmilBuffer: MsgWriteSmilFile failed");
		xmlFree((xmlChar*)pszRawData);
		MmsSmilDestroyDoc(hSmilDoc);
		return false;
	}

	MSG_DEBUG("MMSGenerateSmilBuffer: complete update template\n");
	xmlFree((xmlChar*)pszRawData);
	MmsSmilDestroyDoc(hSmilDoc);
	MSG_END();
	return true;
}

/**	@fn		static HMmsSmil MmsSmilCreateEmptySmilDoc(void)
 *	@brief	Creates default Smil Doc based on input gszEmptyRawDoc. \n
 *	@retval	Returns Smil Document number. \n
 */
HMmsSmil MmsSmilCreateEmptySmilDoc(void)
{
	HMmsSmil hMmsSmil;

	MSG_BEGIN();

	hMmsSmil = MmsSmilCreateSmilDoc(gszEmptyRawDoc);

	MSG_DEBUG("Create an empty smilDoc.(Handle = %d)", hMmsSmil);

	MSG_END();

	return hMmsSmil;
}

/**	@fn			static HMmsSmil MmsSmilCreateSmilDoc(char *pszRawData)
 *	@brief		Creates Smil Doc based on input pszRawData. \n
 *	@param[in]	pszRawData is smil buffer. \n
 *	@retval		Returns Smil Document number. \n
 */
HMmsSmil MmsSmilCreateSmilDoc(char *pszRawData)
{
	int nSmilDocNo = 0;
	xmlNodePtr stRootNode;

	MSG_BEGIN();

	// Destroy smil doc if present
	if (NULL != __gpaMmsSmilDoc[nSmilDocNo]) {
		MSG_DEBUG("Calling MmsSmilDestroyDoc");
		if (false == MmsSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MmsSmilDestroyDoc: Failed!");
		}
	}

	for (nSmilDocNo = 0; nSmilDocNo < MMS_SMIL_MAX_DOC; nSmilDocNo++) {
		if (NULL == __gpaMmsSmilDoc[nSmilDocNo])
			break;
	}

	if (MMS_SMIL_MAX_DOC == nSmilDocNo) {
		MSG_DEBUG("SmilDoc table is full. Can't create.");
		return INVALID_HOBJ;
	}
	__gpaMmsSmilDoc[nSmilDocNo] = (MmsSmilDoc*)malloc(sizeof(MmsSmilDoc));
	if (NULL ==  __gpaMmsSmilDoc[nSmilDocNo]) {
		MSG_DEBUG("Memory Allocation Failed.");
		return INVALID_HOBJ;
	}
	memset(__gpaMmsSmilDoc[nSmilDocNo], 0, sizeof(MmsSmilDoc));

	__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc = xmlParseMemory(pszRawData, strlen(pszRawData));
	if (NULL == __gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc) {
		MSG_DEBUG("Document not parsed successfully.");
		if (false == MmsSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MmsSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}
	stRootNode = xmlDocGetRootElement(__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc);
	if (NULL == stRootNode) {
		MSG_DEBUG("Empty document\n");
		if (false == MmsSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MmsSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}
	if (xmlStrcmp(stRootNode->name, (const xmlChar *) "smil")) {
		MSG_DEBUG("Document of the wrong type, root node != smil");
		if (false == MmsSmilDestroyDoc(nSmilDocNo)) {
			MSG_DEBUG("MmsSmilDestroyDoc: Failed!");
		}
		return INVALID_HOBJ;
	}

	__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode = stRootNode;

	MSG_END();
	return ((HMmsSmil)nSmilDocNo);
}

/**	@fn			static bool MmsSmilDestroyDoc(HMmsSmil hSmilDoc)
 *	@brief		Destroys Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval	TRUE				In case of Success. \n
 *	@retval	FALSE				In case of failure. \n
 */
bool MmsSmilDestroyDoc(HMmsSmil hSmilDoc)
{
	int nSmilDocNo = (int)hSmilDoc;
	bool bFlag = true;
	MSG_BEGIN();

	if (0 <= nSmilDocNo &&
		nSmilDocNo < MMS_SMIL_MAX_DOC &&
		__gpaMmsSmilDoc[nSmilDocNo]) {
		if (__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc) {
			xmlFreeDoc(__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc);
		}

		if (__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode) {
			//Need to Check
		}
		free(__gpaMmsSmilDoc[nSmilDocNo]);
		__gpaMmsSmilDoc[nSmilDocNo] = NULL;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		bFlag =  false;
	}

	MSG_END();
	return bFlag;
}

/**	@fn			static bool IsValidSmilDocNo(int nSmilDocNo)
 *	@brief		Form Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval		Returns Smil Buffer 	In case of success. \n
 *	@retval		Returns NULL			In case of failure. \n
 */
bool IsValidSmilDocNo(int nSmilDocNo)
{
	bool bIsValidSmil = false;

	MSG_BEGIN();

	if (0 <= nSmilDocNo &&
		nSmilDocNo < MMS_SMIL_MAX_DOC &&
		__gpaMmsSmilDoc[nSmilDocNo] &&
		__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc) {
		bIsValidSmil = true;
	}

	MSG_END();
	return bIsValidSmil;
}

/**	@fn			static char  MmsSmilGetRawData(HMmsSmil hSmilDoc)
 *	@brief		Form Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@retval		Returns Smil Buffer 	In case of success. \n
 *	@retval		Returns NULL			In case of failure. \n
 */
char *MmsSmilGetRawData(HMmsSmil hSmilDoc)
{
	int nSmilDocNo = (int)hSmilDoc;
	char *pszRawData = NULL;

	MSG_BEGIN();

	if (IsValidSmilDocNo(nSmilDocNo)) {
		xmlSaveFormatFileEnc("-", __gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc, "UTF-8", 1);
	 	xmlDocDumpMemory(__gpaMmsSmilDoc[nSmilDocNo]->pSmilDoc, (xmlChar **)(&pszRawData) , NULL);
		if (NULL == pszRawData) {
			MSG_DEBUG("Invalid pSmilDoc (now wellformed document)");
		}
		MSG_END();
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
	}
	return pszRawData;
}

/**	@fn			static bool MmsSmilAddPage(HMmsSmil hSmilDoc, MMS_PAGE_S *pstSmilPage)
 *	@brief		Add Page to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilPage specifies page information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddPage(HMmsSmil hSmilDoc, MMS_PAGE_S *pstSmilPage)
{
	int nSmilDocNo = hSmilDoc;

	MSG_BEGIN();

	bool ret = true;

	if (IsValidSmilDocNo(nSmilDocNo)) {
		xmlNodePtr pstParElement;
		xmlNodePtr pstBodyElement;
		xmlNodePtr pstParList;
		char szBuf[MSG_STDSTR_SHORT] = {0, };

		pstBodyElement = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"body");

		if (NULL == pstBodyElement) {
			MSG_DEBUG("There is no <body> node. Can't create <par> node.");
			return false;
		}
		MSG_DEBUG("Body Element Name = %s", (char *)pstBodyElement->name);
		/* Create "par"  node and insert it */
		pstParElement = xmlNewNode(NULL, (xmlChar *)"par");

		if (NULL == pstParElement) {
			MSG_DEBUG("Can't create <par> node. (from XmlParser) \n");
			return false;
		}
		MSG_DEBUG("Par Element Name = %s", (char *)pstParElement->name);

		/* Add attributes to "par" */
		if (pstSmilPage->nDur > 0) {
			snprintf(szBuf, MSG_STDSTR_SHORT, "%dms", pstSmilPage->nDur);
			xmlSetProp(pstParElement, (const xmlChar *)"dur", (const xmlChar *)szBuf);
		}
		/* Find the insertion point : right sibling of the last <par> node or first child of <body> */

		pstParList = xmlGetLastChild(pstBodyElement);

		if (pstParList) {
			ret = __MmsSmilInsertNode(pstBodyElement, pstParList, pstParElement);
		} else {
			ret = __MmsInsertFirstChild(pstBodyElement, pstParElement);
		}
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}

	return ret;
}

/**	@fn			static bool MmsSmilAddRootLayout(HMmsSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout)
 *	@brief		Add Rootlayout to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilRootLayout specifies RootLayout information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddRootLayout(HMmsSmil hSmilDoc, MMS_SMIL_ROOTLAYOUT *pstSmilRootLayout)
{
	int nSmilDocNo = hSmilDoc;
	xmlNodePtr pstRootLayout = NULL;
	xmlNodePtr pstLayoutList = NULL;
	xmlNodePtr pstRootLayoutList = NULL;
	char szBuf[MSG_STDSTR_SHORT] = {0, };

	MSG_BEGIN();

	if (IsValidSmilDocNo(nSmilDocNo)) {
		pstLayoutList = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"layout");

		if (NULL == pstLayoutList) {
			MSG_DEBUG("There is no <layout> node. Can't create <root-layout> node.");
			return false;
		}
		MSG_DEBUG("Layout Element Name = %s ", (char *)pstLayoutList->name);

		pstRootLayoutList = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"root-layout");

		if (NULL != pstRootLayoutList) {
			MSG_DEBUG("MmsSmilAddRootLayout: There is <root-layout> node already");
			MSG_DEBUG("Root Layout Element Name = %s  type=%d\n", (char *)pstRootLayoutList->name);
			return false;
		}
		/* Create "root-layout" node and insert it */
		pstRootLayout = xmlNewNode(NULL, (xmlChar *)"root-layout");
		if (NULL == pstRootLayout) {
			MSG_DEBUG("Can't create <root-layout> node. (from XmlParser)");
			return false;
		}
		MSG_DEBUG("Root Layout Element Name = %s", (char *)pstRootLayout->name);

		if (pstSmilRootLayout->bgColor != SP_NO_COLOR_SET) {	// Replace value later
			xmlSetProp(pstRootLayout, (const xmlChar *)"backgroundColor", (const xmlChar *)__MmsSmilFindColorValue(pstSmilRootLayout->bgColor));
		}
		MSG_DEBUG(" Set Width");
		if (true == pstSmilRootLayout->width.bUnitPercent) {
			snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRootLayout->width.value);
			xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)szBuf);
		} else {
			if (pstSmilRootLayout->width.value > 0) {
				snprintf(szBuf, MSG_STDSTR_SHORT, "%d", pstSmilRootLayout->width.value);
				xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)szBuf);
			} else {
				xmlSetProp(pstRootLayout, (const xmlChar *)"width", (const xmlChar *)"100%");
			}
		}
		MSG_DEBUG(" Set Height");
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
		__MmsInsertFirstChild(pstLayoutList, pstRootLayout);

		return true;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)", nSmilDocNo);
		return false;
	}

}


/**	@fn			static bool MmsSmilAddRegion(HMmsSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion)
 *	@brief		Add Region to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	pstSmilRegion specifies Region information. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddRegion(HMmsSmil hSmilDoc, MMS_SMIL_REGION *pstSmilRegion)
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

		pstLayoutList = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"layout");
		if (NULL == pstLayoutList) {
			MSG_DEBUG(" There is no <layout> node. Can't create <region> node");
			return false;
		}
		MSG_DEBUG("Layout Element Name = %s ", (char *)pstLayoutList->name);

		/* Find the insertion point : right sibling of the last root-layout node or first child of pLayout */
		pstRootLayoutList = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"root-layout");

		if (NULL == pstRootLayoutList) {
			MSG_DEBUG("There is no <root-layout> node. Can't create <root-layout> node.");
			return false;
		} else {
			MSG_DEBUG("Root Layout Element Name = %s ", (char *)pstRootLayoutList->name);
			pAttribute =  pstRootLayoutList->properties;
		}

		if (NULL == pAttribute) {
			MSG_DEBUG("There is no Attribute in <root-layout> node.");
			return false;
		}

		xmlAttrPtr pstAttr = pAttribute;
		for ( ; pstAttr; pstAttr = pstAttr->next) {
			int	attrType;
			MSG_DEBUG("AttributeType: (%s, %s) ", pstAttr->name, pstAttr->children->content);
			switch (attrType = MmsSmilGetAttrID((char *)pstAttr->name)) {
			case ATTRIBUTE_WIDTH:
				{
					int bUnitPercent;

					if (strchr((char *)pstAttr->children->content, '%'))
						bUnitPercent = true;
					else
						bUnitPercent = false;

					nRootWidth = atoi((char *)pstAttr->children->content);
				}
				break;

			case ATTRIBUTE_HEIGHT:
				{
					int bUnitPercent;

					if (strchr((char *)pstAttr->children->content, '%'))
						bUnitPercent = true;
					else
						bUnitPercent = false;

					nRootHeight = atoi((char *)pstAttr->children->content);
				}
				break;
			}
		}

		/* Create "region" node and insert it */
		MSG_DEBUG("Create Region Node");
		pstRegion = xmlNewNode(NULL, (xmlChar *)"region");
		if (NULL == pstRegion) {
			MSG_DEBUG("Can't create <region> node. (from XmlParser)");
			return false;
		}
		/* Add attributes */
		if (pstSmilRegion) {
			MSG_DEBUG(" [Set Attribute] : Region Id");
			if (strlen(pstSmilRegion->szID) > 0) {
				xmlSetProp(pstRegion, (const xmlChar *)"id", (const xmlChar *)pstSmilRegion->szID);
			}
			MSG_DEBUG(" [Set Attribute] : BkGrd Color");
			// Default Color WHITE, always send
			if (pstSmilRegion->bgColor != SP_NO_COLOR_SET) {
				xmlSetProp(pstRegion, (const xmlChar *)"backgroundColor", (const xmlChar *)__MmsSmilFindColorValue(pstSmilRegion->bgColor));
			}
			MSG_DEBUG(" [Set Attribute] : Width");

			if (true == pstSmilRegion->width.bUnitPercent) {
				if (pstSmilRegion->width.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->width.value);
					xmlSetProp(pstRegion, (const xmlChar *)"width", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->width = %d   \n", pstSmilRegion->width.value);
				}
			} else {
				// Note: nRootWidth should be in terms of value(pixel) not unitpercent(%)
				// Validation should be done before this.
				if (pstSmilRegion->width.value >= 0 &&
					pstSmilRegion->width.value <= nRootWidth) {
					int iWidth = (pstSmilRegion->width.value * 100) / nRootWidth;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iWidth);
					xmlSetProp(pstRegion, (const xmlChar *)"width", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->width= %d  iWidth = %d \n", pstSmilRegion->width.value, iWidth);
				}
			}
			MSG_DEBUG(" [Set Attribute] : Height");
			if (true == pstSmilRegion->height.bUnitPercent) {
				if (pstSmilRegion->height.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->height.value);
					xmlSetProp(pstRegion, (const xmlChar *)"height", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->height = %d   \n", pstSmilRegion->height.value);
				}
			} else {
				// Note: nRootHeight should be in terms of value(pixel) not unitpercent(%)
				// Validation should be done before this.
				if (pstSmilRegion->height.value >= 0 &&
					pstSmilRegion->height.value <= nRootHeight) {
					int iHeight = (pstSmilRegion->height.value * 100) / nRootHeight;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iHeight);
					xmlSetProp(pstRegion, (const xmlChar *)"height", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->height = %d  iHeight = %d \n", pstSmilRegion->height.value, iHeight);
				}
			}
			MSG_DEBUG(" [Set Attribute] : Left");
			if (true == pstSmilRegion->nLeft.bUnitPercent) {
				if (pstSmilRegion->nLeft.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->nLeft.value);
					xmlSetProp(pstRegion, (const xmlChar *)"left", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->left = %d   \n", pstSmilRegion->nLeft.value);
				}
			} else {
				// Note: nRootWidth should be in terms of value(pixel) not unitpercent(%)
				// Validation should be done before this.
				if (pstSmilRegion->nLeft.value >= 0) {
					int iLeft = (pstSmilRegion->nLeft.value * 100) / nRootWidth;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iLeft);
					xmlSetProp(pstRegion, (const xmlChar *)"left", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: SmilRegion->iLeft = %d       iLeft = %d \n", pstSmilRegion->nLeft.value, iLeft);
				}
			}
			MSG_DEBUG(" [Set Attribute] : Top");
			if (true == pstSmilRegion->nTop.bUnitPercent) {
				if (pstSmilRegion->nTop.value > 0) {
					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", pstSmilRegion->nTop.value);
					xmlSetProp(pstRegion, (const xmlChar *)"top", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->nTop= %d   \n", pstSmilRegion->nTop.value);
				}
			} else {
				// Note: nRootHeight should be in terms of value(pixel) not unitpercent(%)
				// Validation should be done before this.
				if (pstSmilRegion->nTop.value >= 0) {
					int iTop = (pstSmilRegion->nTop.value * 100) / nRootHeight;

					snprintf(szBuf, MSG_STDSTR_SHORT, "%d%%", iTop);
					xmlSetProp(pstRegion, (const xmlChar *)"top", (const xmlChar *)szBuf);
					MSG_DEBUG("MmsSmilAddRegion: pstSmilRegion->nTop= %d  iTop = %d \n", pstSmilRegion->nTop.value, iTop);
				}
			}
			MSG_DEBUG(" [Set Attribute] : Fit");
			//Fit Attribute
			if (MMSUI_IMAGE_REGION_FIT_MEET == pstSmilRegion->fit) {
				xmlSetProp(pstRegion, (const xmlChar *)"fit", (const xmlChar *)"meet");
			} else if (MMSUI_IMAGE_REGION_FIT_HIDDEN == pstSmilRegion->fit) {
				xmlSetProp(pstRegion, (const xmlChar *)"fit", (const xmlChar *)"hidden");
			}

			__MmsSmilInsertNode(pstLayoutList, pstRootLayoutList, pstRegion);

		} else
			MSG_DEBUG("There is no attribute in <region> node\n");

		MSG_END();
		return true;
	} else {
		MSG_DEBUG("Invalid SmilDoc(hSmilDoc:%d)\n", nSmilDocNo);
		return false;
	}
}

/**	@fn			static bool MmsSmilAddMedia( HMmsSmil hSmilDoc, int nPageNo, MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Add Media to Smil Doc. \n
 *	@param[in]	hSmilDoc is smil doc number. \n
 *	@param[in]	nPageNo specifies page number to which media belongs. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool MmsSmilAddMedia( HMmsSmil hSmilDoc, int nPageNo, int nMediaIdx, MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
{
	int nSmilDocNo = hSmilDoc;

	MSG_BEGIN();

	if (NULL == pszContentID) {
		MSG_DEBUG(" Content Id is NULL");
		return false;
	}
	memset(pszContentID, 0, MMS_CONTENT_ID_LEN + 1);
	if (IsValidSmilDocNo(nSmilDocNo)) {
		int nIndex = 0;
		xmlNode *pstMedia;
		xmlNode *pstLastChild;
		xmlNodePtr pstParList;
		char *pszExt ;

		pstParList = UtilxmlStringGetNodeList(__gpaMmsSmilDoc[nSmilDocNo]->pstRootNode, (char *)"par");
		if (NULL == pstParList) {
			MSG_DEBUG("There is no <par> node. Can't create <media> node.");
			return false;
		}
		MSG_DEBUG("Par Element Name = %s ", (char *)pstParList->name);
		for (nIndex = 0; (pstParList &&  nIndex < nPageNo); nIndex++) {
			pstParList = pstParList->next;
		}
		if (NULL == pstParList) {
			MSG_DEBUG("There is no such page node. Can't insert <media> node.");
			return false;
		}
		MSG_DEBUG("Par Element Name = %s ", (char *)pstParList->name);
		/* Find insertion point and make a contentID */

		pstLastChild = xmlGetLastChild(pstParList);

		pszExt = strrchr(pstSmilMedia->szFileName, '.');
		if (pszExt && !strrchr(pszExt, '/'))
			snprintf(pszContentID, MSG_MSG_ID_LEN+1, "%lu_%lu%s", (ULONG)nPageNo, (ULONG)nMediaIdx, pszExt);
		else
			snprintf(pszContentID, MSG_MSG_ID_LEN+1, "%lu_%lu", (ULONG)nPageNo, (ULONG)nMediaIdx);

		/* Create <media> node and insert set attribute */
		MSG_DEBUG(" Create Media Node");
		switch (pstSmilMedia->mediatype) {
		case MMS_SMIL_MEDIA_TEXT:
			pstMedia = __MmsCreateTextNode(pstSmilMedia, pszContentID);
			break;
		case MMS_SMIL_MEDIA_AUDIO:
		case MMS_SMIL_MEDIA_VIDEO:
		case MMS_SMIL_MEDIA_IMG:
			pstMedia = __MmsCreateMMNode(pstSmilMedia, pszContentID);
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
			__MmsSmilInsertNode(pstParList, pstLastChild, pstMedia);
		else
			__MmsInsertFirstChild(pstParList, pstMedia);

		MSG_END();
		return true;
	} else {
		MSG_DEBUG("MmsSmilAddMedia: Invalid SmilDoc(hSmilDoc:%d)\n", nSmilDocNo);
		return false;
	}
}

/**	@fn			static xmlNode *__MmsCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Create Text Element. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		Text Element node		In case of Success. \n
 *	@retval		NULL				In case of failure. \n
 */
xmlNode *__MmsCreateTextNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
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
	MSG_DEBUG("Text Element Name = %s ", (char *)pstMedia->name);

	/* Add attributes */
	if (pstSmilMedia) {
		MSG_DEBUG("[Set Attribute] Region Id ");
		if (strlen(pstSmilMedia->regionId) > 0) {
			xmlSetProp(pstMedia, (const xmlChar *)"region", (const xmlChar *)pstSmilMedia->regionId);
		}
		MSG_DEBUG("[Set Attribute] Begin ");
		if (pstSmilMedia->sMedia.sText.nBegin > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sText.nBegin);
			xmlSetProp(pstMedia, (const xmlChar *)"begin", (const xmlChar *) szBuf);
		}
		MSG_DEBUG("[Set Attribute] Duration");
		if (pstSmilMedia->sMedia.sText.nDurTime > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sText.nDurTime);
			xmlSetProp(pstMedia, (const xmlChar *)"dur", (const xmlChar *)szBuf);
		}
		MSG_DEBUG("[Set Attribute] Alternate");
		if (strlen(pstSmilMedia->szAlt) > 0) {
			snprintf (szBuf, sizeof(szBuf), "%s", pstSmilMedia->szAlt);
			xmlSetProp(pstMedia, (const xmlChar *)"alt", (const xmlChar *)szBuf);
		}
		MSG_DEBUG("[Set Attribute] Src");

		char szFilePathWithCid[MMS_CONTENT_ID_LEN + 5];	// for "cid:"

		snprintf (szFilePathWithCid, sizeof(szFilePathWithCid), "cid:%s", pszContentID);
		_MmsSmilSetAttribute(pstMedia, (char *)"src", szFilePathWithCid);

		MSG_DEBUG("[Set Attribute] Font Foreground Color");

		if (pstSmilMedia->sMedia.sText.nColor!= SP_BLACK) {	// Chnage after getting exact values
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");

			if (NULL == pstParam) {
				MSG_DEBUG("Cannot create <param> node");
				return false;
			}
			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"foreground-color");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)__MmsSmilFindColorValue(pstSmilMedia->sMedia.sText.nColor));
			__MmsInsertFirstChild(pstMedia, pstParam);
		}

		MSG_DEBUG("[Set Attribute] Font Background Color");

		if (pstSmilMedia->sMedia.sText.nBgColor != SP_BLACK) {	// Chnage after getting exact values
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");

			if (NULL == pstParam) {
				MSG_DEBUG("Cannot create <param> node");
				return false;
			}
			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"background-color");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)__MmsSmilFindColorValue(pstSmilMedia->sMedia.sText.nBgColor));
			__MmsInsertFirstChild(pstMedia, pstParam);
		}

		MSG_DEBUG("[Set Attribute] Size");
		if (pstSmilMedia->sMedia.sText.nSize > 0) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node \n");
				return false;
			}

			if (pstSmilMedia->sMedia.sText.nSize  <= MMS_SMIL_FONT_SIZE_SMALL)
				strcpy(szSizeBuf, "small");
			else if ((pstSmilMedia->sMedia.sText.nSize  > MMS_SMIL_FONT_SIZE_SMALL) && (pstSmilMedia->sMedia.sText.nSize  < MMS_SMIL_FONT_SIZE_LARGE))
				strcpy(szSizeBuf, "normal");
			else
				strcpy(szSizeBuf, "large");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textsize");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);
			__MmsInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bBold == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node \n");
				return false;
			}

			strcpy(szSizeBuf, "bold");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);
			__MmsInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bItalic == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node \n");
				return false;
			}

			strcpy(szSizeBuf, "italic");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);
			__MmsInsertFirstChild(pstMedia, pstParam);
		}

		if (pstSmilMedia->sMedia.sText.bUnderLine == true) {
			pstParam = xmlNewNode(NULL, (xmlChar *)"param");
			if (NULL == pstParam) {
				MSG_DEBUG(" __MmsCreateTextNode: cannot create <param> node \n");
				return false;
			}

			strcpy(szSizeBuf, "underline");

			xmlSetProp(pstParam, (const xmlChar *)"name", (const xmlChar *)"textattribute");
			xmlSetProp(pstParam, (const xmlChar *)"value", (const xmlChar *)szSizeBuf);
			__MmsInsertFirstChild(pstMedia, pstParam);
		}
	}

	MSG_END();
	return pstMedia;
}

/**	@fn			static xmlNode *__MmsCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
 *	@brief		Create Image/Audio/Video Element. \n
 *	@param[in]	pstSmilMedia specifies Media information. \n
 *	@param[in]	pszContentID specifies Content ID of media. \n
 *	@retval		Image/Audio/Video Element node	In case of Success. \n
 *	@retval		NULL							In case of failure. \n
 */
xmlNode *__MmsCreateMMNode(MMS_MEDIA_S *pstSmilMedia, char *pszContentID)
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

	if (pstSmilMedia) {
		char szFilePathWithCid[MMS_CONTENT_ID_LEN + 5];  	// for "cid:"

		MSG_DEBUG("[Set Attribute] Region Id ");
		if (strlen(pstSmilMedia->regionId) > 0) {
			xmlSetProp(pstMedia, (const xmlChar *)"region", (const xmlChar *)pstSmilMedia->regionId);
		}
		MSG_DEBUG("[Set Attribute] Src ");
		snprintf (szFilePathWithCid, sizeof(szFilePathWithCid), "cid:%s", pszContentID);
		_MmsSmilSetAttribute(pstMedia, (char *)"src", szFilePathWithCid);

		MSG_DEBUG("[Set Attribute] Begin ");
		if (pstSmilMedia->sMedia.sAVI.nBegin > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sAVI.nBegin);
			xmlSetProp(pstMedia, (const xmlChar *)"begin", (const xmlChar *)szBuf);
		}
		MSG_DEBUG("[Set Attribute] Duration ");
		if (pstSmilMedia->sMedia.sAVI.nDurTime > 0) {
			snprintf (szBuf, sizeof(szBuf), "%dms", pstSmilMedia->sMedia.sAVI.nDurTime);
			xmlSetProp(pstMedia, (const xmlChar *)"dur", (const xmlChar *)szBuf);
		}
		MSG_DEBUG("[Set Attribute] Alt ");
		if (strlen(pstSmilMedia->szAlt) > 0) {
			snprintf (szBuf, sizeof(szBuf), "%s", pstSmilMedia->szAlt);
			xmlSetProp(pstMedia, (const xmlChar *)"alt", (const xmlChar *)szBuf);
		}
	} else {
		MSG_DEBUG("There is no attribute in such-<media> node");
	}

	MSG_END();
	return pstMedia;
}

/**	@fn			static bool __MmsInsertFirstChild(xmlNode *pstParent, xmlNode *pstCurr)
 *	@brief		Inserts first child to parent node. \n
 *	@param[in]	pstParent specifies Parent node. \n
 *	@param[in]	pstCurr specifies Child node. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool __MmsInsertFirstChild(xmlNode *pstParent, xmlNode *pstCurr)
{
	bool bFlag = true;

	MSG_BEGIN();

	 if (NULL == xmlAddChild(pstParent, pstCurr)) {
 		MSG_DEBUG("%s Node not added", pstCurr->name);
	 	bFlag = false;
	 }

	 MSG_END();
	 return bFlag;
}


/**	@fn			static bool __MmsSmilInsertNode(xmlNode *pstParent, xmlNode *pstLeftSibling, xmlNode *pstCurr)
 *	@brief		Inserts node. \n
 *	@param[in]	pstParent specifies Parent node. \n
 *	@param[in]	pstLeftSibling specifies Left Sibling node. \n
 *	@param[in]	pstCurr specifies Child node. \n
 *	@retval		TRUE				In case of Success. \n
 *	@retval		FALSE				In case of failure. \n
 */
bool __MmsSmilInsertNode(xmlNode *pstParent, xmlNode *pstLeftSibling, xmlNode *pstCurr)
{
	MSG_BEGIN();
	bool bFlag = true;

	if (pstLeftSibling) {
		/* Parent Node is Unused */

		while (pstLeftSibling->next !=NULL)
			pstLeftSibling = pstLeftSibling->next;

		 if (NULL == xmlAddNextSibling(pstLeftSibling, pstCurr)) {
	  		MSG_DEBUG("%s Node not added", pstCurr->name);
		 	bFlag = false;
		 }
	} else {
		 if (NULL == xmlAddChild(pstParent, pstCurr)) {
		 	 MSG_DEBUG("%s Node not added", pstCurr->name);
			bFlag = false;
		 }
	}
	MSG_END();
	return bFlag;
}


bool __MmsGetRealFileName(MmsSmilMediaType mediaType, char *pszSrc, char *pszName, int msgID)
{
	MsgType partHeader;
	int partCnt;
	int i;

	MSG_DEBUG("__MmsUiGetRealFileName: mediaType[%d]", mediaType);
	MSG_DEBUG("__MmsUiGetRealFileName: pszSrc[%s]\n", pszSrc);

	if (!pszName) {
		MSG_DEBUG("__MmsUiGetRealFileName: pszName is null\n");
		return false;
	}

	if (mediaType == MMS_SMIL_MEDIA_TEXT || mediaType == MMS_SMIL_MEDIA_INVALID) {
		MSG_DEBUG("__MmsUiGetRealFileName: invalid mediaType(=%d)\n", mediaType);
		return false;
	}

	partCnt = MmsGetMediaPartCount(msgID);

	if (partCnt < 0) {
		MSG_DEBUG("__MmsUiGetRealFileName: partCnt < 0, (=%d)\n", partCnt);
		return false;
	}

	for (i = 0; i < partCnt; i++) {
		MmsGetMediaPartHeader(i, &partHeader);

		if (mediaType == MMS_SMIL_MEDIA_AUDIO ||
			mediaType == MMS_SMIL_MEDIA_VIDEO ||
			mediaType == MMS_SMIL_MEDIA_IMG) {
			if (!strcmp(partHeader.param.szFileName , pszSrc)) {
				// Found
				MSG_DEBUG("__MmsUiGetRealFileName: pszSrc[%s]\n", pszSrc);
				break;
			}
		}
	}

	if (i == partCnt)
		return false;

	if (partHeader.param.szName[0] != '\0') {
		int nameLen = strlen(partHeader.param.szName);

		if (nameLen > MSG_FILENAME_LEN_MAX - 1) {
			;//Need to check
		} else {
			strcpy(pszName, partHeader.param.szName);
		}

		return true;
	}
	return false;
}

/**	@fn			static void _MmsSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue)
 *	@brief		Sets Attribute. \n
 *	@param[in]	pNode specifies node. \n
 *	@param[in]	szField specifies attribute field. \n
 *	@param[in]	szValue specifies value of field \n
 */
void _MmsSmilSetAttribute(xmlNode *pNode, char *szField, char *szValue)
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

/**	@fn			static char *__MmsSmilFindColorValue(int nValue)
 *	@brief		Converts color to RGB. \n
 *	@param[in]	nValue specifies color value. \n
 *	@retval		RGB value. \n
 */
char *__MmsSmilFindColorValue(int nValue)
{
	unsigned char red = (nValue & 0xFF0000) >> 16;
	unsigned char green = (nValue & 0x00FF00) >> 8;
	unsigned char blue = nValue & 0x0000FF;

	MSG_BEGIN();

	snprintf(gszColor,MMS_SMIL_COLOR_SIZE, "#%02x%02x%02x", red, green, blue);
	MSG_DEBUG("__MmsSmilFindColorValue: gszColor %s \n", gszColor);

	MSG_END();
	return gszColor;
}

/**	@fn			static xmlNodePtr UtilxmlStringGetNodeList(xmlNodePtr pstNode, char *pszValue)
 *	@brief		Get node based on pszValue. \n
 *	@param[in]	pNode specifies node. \n
 *	@param[in]	pszName specifies name field. \n
 *	@retval		RGB value. \n
 */
xmlNodePtr UtilxmlStringGetNodeList(xmlNodePtr pstNode, char *pszName)
{
	MSG_BEGIN();

	if ((NULL != pstNode) && (NULL != pszName)) {
		xmlNodePtr pstTempNode;
		xmlNodePtr pstReturnNode;

		pstTempNode = pstNode;

		for ( ; pstTempNode; pstTempNode = pstTempNode->next) {
			MSG_DEBUG("\n Node Name = %s[%p] children =%p \n", (char *)pstTempNode->name, pstTempNode, pstTempNode->children);
			MSG_DEBUG("\n Compare Parent Node = %s[%p] \n", (char *)pstTempNode->name, pstTempNode);
			if (0 == strcasecmp((char *)pstTempNode->name, pszName)) {
				return pstTempNode;
			}

			if (pstTempNode->children) {
				MSG_DEBUG("\n Enter Inside\n");
				pstReturnNode = UtilxmlStringGetNodeList(pstTempNode->children, pszName);
				if (NULL != pstReturnNode) {
					return pstReturnNode;
				}
			}

		}
	}
	return NULL;
}

