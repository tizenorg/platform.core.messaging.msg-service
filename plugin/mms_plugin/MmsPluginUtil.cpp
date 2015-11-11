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
#include <errno.h>
#include <sys/stat.h>
#include <glib.h>
#include "MsgUtilFile.h"
#include "MmsPluginDebug.h"
#include "MmsPluginUtil.h"
#include <string>

using namespace std;


FILE *MmsFileOpen(char *pFileName)
{
	mode_t file_mode = (S_IRUSR | S_IWUSR);

	if (!pFileName) {
		MSG_DEBUG("pFileName NULL: %s", g_strerror(errno));
		return NULL;
	}

	MSG_DEBUG("pFileName = %s", pFileName);

	FILE *pFile = MsgOpenFile(pFileName, "wb+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", g_strerror(errno));
		return NULL;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", g_strerror(errno));
		return NULL;
	}

	if (fchmod(fileno(pFile), file_mode) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File chmod Error: %s", g_strerror(errno));
		return NULL;
	}

	return pFile;
}


bool MmsIsAsciiString(const char *pszText)
{
	if (pszText == NULL) {
		MSG_DEBUG(MMS_DEBUG_STR_INVALID_PARAM" pszText = %p", pszText);
		return false;
	}

	int length = strlen(pszText);
	if (length <= 0) {
		MSG_DEBUG("Input parameter is NULL string");
		return false;
	}

	for (int i = 0; i < length; ++i) {
		if (!isascii(pszText[i])) {
			MSG_DEBUG("It is not Ascii code");
			return false;
		}
	}

	MSG_DEBUG("It is Ascii code");
	return true;
}

void MmsRemoveLessGreaterChar(const char *szSrcID, char *szDest, int destSize)
{
	char szBuf[MSG_MSG_ID_LEN + 1] = {0, };
	int cLen = strlen(szSrcID);

	if (cLen > 1 && szSrcID[0] == '<' && szSrcID[cLen - 1] == '>') {
		strncpy(szBuf, &szSrcID[1], cLen - 2);
		szBuf[cLen - 2] = '\0';
	} else if (cLen > 1 && szSrcID[0] == '"' && szSrcID[cLen-1] == '"') {
		strncpy(szBuf, &szSrcID[1], cLen - 2);
		szBuf[cLen - 2] = '\0';
	} else {
		strncpy(szBuf, szSrcID, cLen);
		szBuf[cLen] = '\0';
	}

	snprintf(szDest, destSize, "%s", szBuf);
}

bool MmsRemoveMarkup(const char *src, char *dst, int dstsize)
{
	const int M_IN = 1;
	const int M_OUT = 0;

	int state = M_OUT;
	const char *srcptr = NULL;
	char *dstptr = NULL;

	if (src == NULL || dst == NULL || dstsize == 0)
		return false;

	dstptr = dst;
	srcptr = src;

	while (*srcptr != '\0' && ((dstptr-dst) < dstsize)) {

		if (*srcptr == '<') state = M_IN;
		if (*srcptr == '>') state = M_OUT;

		if (state == M_OUT && *srcptr != '<' && *srcptr != '>') {
			*dstptr = *srcptr;
			dstptr++;
		}

		srcptr++;
	}

	if (((dstptr-dst) < dstsize)) {
		*dstptr = '\0';
	} else {
		*(dst + dstsize -1) = '\0';
	}

	return true;
}

/* change character ' ' to '_' */
bool MmsReplaceSpaceChar(char *pszText)
{
	if (!pszText) {
		return false;
	}

	char *spaceCharPtr = strchr(pszText, ' ');

	while (spaceCharPtr) {

		*spaceCharPtr = '_';

		spaceCharPtr = strchr(pszText, ' ');
	}

	return true;
}

char *MmsReplaceNonAsciiUtf8(const char *szSrc, char replaceChar)
{
	int offset = 0;
	int srcLen = 0;
	const unsigned char *ptr = NULL;
	unsigned char b1, b2, b3, b4;
	string str;

	if (szSrc == NULL) {
		return NULL;
	}

	srcLen = strlen(szSrc);

	if (srcLen == 0) {
		return NULL;
	}

	ptr = (const unsigned char*)szSrc;

	while (offset < srcLen && *(ptr) != '\0') {

		b1 = *(ptr);

		if ((b1 & 0x80) == 0) { /* 1byte : 0xxx xxxx */
			offset += 1;
			ptr += 1;

			str += b1;
		} else if ((b1 & 0xE0) == 0xC0) { /* 2byte : 110x xxxx */
			offset += 2;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);

			if (b2 >= 0x80) { /* 10xx xxxx */
				ptr += 2;
				str += replaceChar;
			} else {
				return NULL;
			}

		} else if ((b1 & 0xF0) == 0xE0) { /* 3byte : 1110 xxxx */

			offset += 3;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);

			if (b2 >= 0x80 && b3 >= 0x80) { /* 10xx xxxx */
				ptr += 3;
				str += replaceChar;
			} else {
				return NULL;
			}

		} else if ((b1 & 0xF8) == 0xF0) { /* 4byte : 1111 xxxx */

			offset += 4;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);
			b4 = *(ptr + 3);

			if (b2 >= 0x80 && b3 >= 0x80 && b4 >= 0x80) { /* 10xx xxxx */
				ptr += 4;
				str += replaceChar;

			} else {
				return NULL;
			}
		} else {
			return NULL;
		}
	}


	if (str.empty() == true) {
		return NULL;
	}

	MSG_DEBUG("str is UTF-8 [%s]", str.c_str());

	return g_strdup(str.c_str());
}

bool MmsIsUtf8String(const unsigned char *szSrc, int nChar)
{
	int offset = 0;
	int srcLen = 0;
	const unsigned char *ptr;
	unsigned char b1, b2, b3, b4;

	if (szSrc == NULL) {
		return false;
	}

	srcLen = nChar;

	if (srcLen == 0) {
		return true;
	}

	ptr = (const unsigned char*)szSrc;

	while (offset < srcLen && *(ptr) != '\0') {

		b1 = *(ptr);

		if ((b1 & 0x80) == 0) { /* 1byte : 0xxx xxxx */
			offset += 1;
			ptr += 1;
		} else if ((b1 & 0xE0) == 0xC0) { /* 2byte : 110x xxxx */
			offset += 2;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);

			if (b2 >= 0x80) { /* 10xx xxxx */
				ptr += 2;
			} else {
				return false;
			}

		} else if ((b1 & 0xF0) == 0xE0) { /* 3byte : 1110 xxxx */

			offset += 3;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);

			if (b2 >= 0x80 && b3 >= 0x80) { /* 10xx xxxx */
				ptr += 3;
			} else {
				return false;
			}

		} else if ((b1 & 0xF8) == 0xF0) { /* 4byte : 1111 xxxx */

			offset += 4;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);
			b4 = *(ptr + 3);

			if (b2 >= 0x80 && b3 >= 0x80 && b4 >= 0x80) { /* 10xx xxxx */
				ptr += 4;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} /* while */

	return true;
}
