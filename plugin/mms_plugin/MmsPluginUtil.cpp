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

#include <mm_file.h>
#include <mm_util_jpeg.h>
#include <mm_util_imgp.h>

#include <media-thumbnail.h>

//----str
#include <ctype.h>

#include "MsgDebug.h"
#include "MmsPluginUtil.h"

bool makeImageThumbnail(char *srcPath, char *dstPath)
{
	if (srcPath == NULL || dstPath == NULL) {
		MSG_DEBUG("Make thumbnail: failed [Invalid Parameter]");
		return false;
	}

	int err = -1;
	err = thumbnail_request_save_to_file(srcPath, MEDIA_THUMB_LARGE, dstPath);
	if (err < 0) {
		MSG_DEBUG("Make thumbnail: failed");
		return false;
	}

	MSG_DEBUG("Make thumbnail: success [%s]", dstPath);
	return true;
}

bool makeVideoThumbnail(char *srcPath, char *dstPath)
{
	MMHandleType content_attrs = (MMHandleType)NULL;
	char *err_attr_name = NULL;
	int fileRet = 0;
	int trackCount = 0;

	if (srcPath == NULL || dstPath == NULL) {
		MSG_DEBUG("Make thumbnail: failed [Invalid Parameter]");
		return false;
	}

	mm_file_create_content_attrs(&content_attrs, srcPath);
	fileRet = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_VIDEO_TRACK_COUNT, &trackCount, NULL);

	if (fileRet != 0) {
		MSG_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);

		if (err_attr_name) {
			free(err_attr_name);
			err_attr_name = NULL;
		}

		mm_file_destroy_content_attrs(content_attrs);

		return false;
	}

	MSG_DEBUG("video track num: %d", trackCount);

	if (trackCount <= 0) {
		mm_file_destroy_content_attrs(content_attrs);
		return false;
	}


	int thumbnailWidth = 0;
	int thumbnailHeight = 0;
	int thumbnailSize = 0;
	void *thumbnail = NULL;

	fileRet = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_VIDEO_WIDTH, &thumbnailWidth,
																MM_FILE_CONTENT_VIDEO_HEIGHT, &thumbnailHeight,
																MM_FILE_CONTENT_VIDEO_THUMBNAIL, &thumbnail, &thumbnailSize,
																NULL);

	if (fileRet != 0) {
		MSG_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);
		if (err_attr_name) {
			free(err_attr_name);
			err_attr_name = NULL;
		}

		mm_file_destroy_content_attrs(content_attrs);
		return false;
	}

	MSG_DEBUG("video width: %d", thumbnailWidth);
	MSG_DEBUG("video height: %d", thumbnailHeight);
	MSG_DEBUG("video thumbnail: %p", thumbnail);

	if (thumbnail == NULL) {
		mm_file_destroy_content_attrs(content_attrs);
		return false;
	}

	fileRet = mm_util_jpeg_encode_to_file (dstPath, thumbnail, thumbnailWidth, thumbnailHeight, MM_UTIL_JPEG_FMT_RGB888, 70);
	if (fileRet != 0) {
		MSG_DEBUG("mm_util_jpeg_encode_to_file fails [%d]", fileRet);
		mm_file_destroy_content_attrs(content_attrs);
		return false;
	}

	MSG_DEBUG("Make thumbnail: success [%s]", dstPath);
	mm_file_destroy_content_attrs(content_attrs);
	return true;
}

//----------------------------------------------------------------------
bool MsgIsASCII(char *pszText)
{
	int length = strlen(pszText);

	for (int i = 0; i < length; ++i) {
		if (!isascii(pszText[i])) {
			MSG_DEBUG("_MsgIsASCII false.");
			return false;
		}
	}

	MSG_DEBUG("_MsgIsASCII true.");
	return true;
}


bool MsgReplaceNonAscii(char *szInText, char **szOutText, char replaceChar)
{
	MSG_DEBUG("_MsgReplaceNonAscii");
	int nCount = 0;
	int index = 0;
	int cLen = 0;
	char *pNew = NULL;

	cLen = strlen(szInText);

	pNew = (char *)malloc(cLen + 1);

	if (pNew == NULL)
		return false;

	memset(pNew, 0, cLen + 1);

	while (*(szInText+nCount) != '\0') {
		if (0x0001 <= *(szInText+nCount) && *(szInText+nCount) <= 0x007F) {
			MSG_DEBUG("_MsgReplaceNonAscii: non ascii characters (1bytes). \n");
			pNew[index] = szInText[nCount];
			nCount += 1;
			index += 1;
		} else {
			MSG_DEBUG("_MsgReplaceNonAscii: UTF-8 characters (2bytes). \n");
			pNew[index] = replaceChar;
			nCount += 1;
			index +=1;
		}
	}

	*szOutText = pNew;
	return true;
}

bool MsgIsSpace(char *pszText)
{
	MSG_DEBUG("_MsgIsSpace");
	if (!pszText) {
		MSG_DEBUG("_MsgIsSpace: pszText == NULL!\n");
		return false;
	}

	if (strchr(pszText, ' ') != NULL)
		return true;
	else
		return false;
}

bool MsgReplaceSpecialChar(char *szInText, char **szOutText, char specialChar)
{
	MSG_DEBUG("_MsgReplaceSpecialChar");
	char *pszOutText = NULL;
	char szBuf[10] = {0, };
	char temp[5] = {0, };
	int cLen = 0;
	int i = 0;

	if (!szInText) {
		MSG_DEBUG("_MsgReplaceSpecialChar: szInText == NULL! \n");
		return false;
	}

	if (!szOutText)
		return false;

	cLen = strlen(szInText);

	if (specialChar == ' ') {
		if ((pszOutText = (char *)malloc(cLen + 1)) == NULL) {
			MSG_DEBUG("_MsgReplaceSpecialChar : %d line. MemAlloc failed.\n", __LINE__);
			return false;
		}
		memset(pszOutText, 0, cLen + 1);

		*szOutText = pszOutText;
	}

	for (i = 0; i<cLen; i++) {
		switch (specialChar) {
		// changed space to '_'
		case ' ':
			pszOutText[i] = (szInText[i] == specialChar) ? '_' : szInText[i];
			break;

		default:
			if (szInText[i] != specialChar) {
				temp[0] = szInText[i];
				*szOutText = MsgStrAppend(*szOutText, temp);
				continue;
			} else {
				MsgConvertCharToHex(specialChar, szBuf);
				*szOutText = MsgStrAppend(*szOutText, (char *)"%");
				*szOutText = MsgStrAppend(*szOutText, szBuf);
			}
			break;
		}
	}

	MSG_DEBUG("_MsgReplaceSpecialChar : output text : [%s]\n", pszOutText);

	return true;
}

char *MsgStrAppend(char *szInputStr1, char *szInputStr2)
{
	MSG_DEBUG("MsgStrAppend");
	char *szOutputStr = NULL;

	if (szInputStr1 == NULL) {
		szOutputStr = MsgStrCopy(szInputStr2);
	} else {
		int length1 = 0;
		int length2 = 0;
		length1 = MsgStrlen(szInputStr1);
		length2 = MsgStrlen(szInputStr2);

		szOutputStr = (char *)malloc(length1 + length2 + 1);

		if (szOutputStr == NULL)
			goto __CATCH;

		memset(szOutputStr, 0, length1 + length2 + 1);

		strncpy(szOutputStr, szInputStr1, length1);

		if (length2 > 0)
			strcat(szOutputStr, szInputStr2);

		free(szInputStr1);
		szInputStr1 = NULL;
	}

	return szOutputStr;

__CATCH:
	return NULL;
}

char *MsgStrCopy(const char *string)
{
	char *pDst = NULL;

	if (string) {
		pDst = (char *)malloc(1 + strlen(string));
		if (pDst == NULL) {
			MSG_DEBUG("MsgStrCopy: pDst MemAlloc Fail \n");
			return NULL;
		}

		memset(pDst, 0, strlen(string) + 1);

		strcpy(pDst,string);

		return pDst;
	}

	return NULL;
}

char *MsgStrNCopy(const char *string, int length)
{
	char *pDst = NULL;

	if (string) {
		pDst = (char *)malloc(1 + length);
		if (pDst == NULL) {
			MSG_DEBUG("MsgStrNCopy: pDst MemAlloc Fail \n");
			return NULL;
		}

		memset(pDst, 0, length + 1);
		strncpy(pDst,string, length);

		return pDst;
	}

	return NULL;
}

int MsgStrlen(char * pStr)
{
	if (pStr == NULL)
		return 0;

	return strlen(pStr);
}

bool MsgConvertCharToHex(char pSrc, char *pDest)
{
	static unsigned char saucHex[] = "0123456789ABCDEF";

	pDest[0] = saucHex[pSrc >> 4];
	pDest[1] = saucHex[pSrc & 0xF];
	pDest[2] = 0;

	return true;
}
