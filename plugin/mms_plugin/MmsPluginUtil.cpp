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

#include <mm_file.h>
#include <mm_util_jpeg.h>
#include <mm_util_imgp.h>
#include <thumbnail_util.h>
#include <image_util.h>

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include "MsgUtilFile.h"
#include "MsgMutex.h"
#include "MmsPluginDebug.h"
#include "MmsPluginUtil.h"
#include <string>

using namespace std;


Mutex g_mx;
CndVar g_cv;

void thumbnail_completed_cb(thumbnail_util_error_e error, const char *request_id,
									int thumb_width, int thumb_height,
									unsigned char *thumb_data, int thumb_size, void *user_data)
{
	if (!user_data) {
		MSG_DEBUG("dstPath is NULL");
		return;
	}

	MSG_BEGIN();

	g_mx.lock();
	MSG_DEBUG("=================[RESULT]");
	MSG_DEBUG("error_code [%d]", error);
	MSG_DEBUG("request id [%s]", request_id);
	MSG_DEBUG("width [%d], height [%d]", thumb_width, thumb_height);
	MSG_DEBUG("raw_data [0x%x], size [%d]", *thumb_data, thumb_size);

	int ret = 0;
	ret = image_util_encode_jpeg(thumb_data, thumb_width, thumb_height, IMAGE_UTIL_COLORSPACE_BGRA8888, 90, (char *)user_data);;
	if (ret != IMAGE_UTIL_ERROR_NONE)
		MSG_DEBUG("image_util_encode_jpeg() is failed");

	g_cv.signal();
	g_mx.unlock();

	MSG_END();
}

bool MmsMakeImageThumbnail(char *srcPath, char *dstPath)
{
	if (srcPath == NULL || dstPath == NULL) {
		MSG_DEBUG(MMS_DEBUG_STR_INVALID_PARAM" src = %p, dst = %p", srcPath, dstPath);
		return false;
	}

	if (MsgAccessFile(srcPath, R_OK) == false) {
		MSG_DEBUG("not exist source file [%s]", srcPath);
		return false;
	}

	g_mx.lock();

	int time_ret = 0;

	int ret = THUMBNAIL_UTIL_ERROR_NONE;
	char *req_id = NULL;
	thumbnail_h thumb_h;
	thumbnail_util_create(&thumb_h);
//	thumbnail_util_set_size(thumb_h, 240, 240);
	thumbnail_util_set_path(thumb_h, srcPath);
	MSG_DEBUG("thumbnail_util_extract");

	ret = thumbnail_util_extract(thumb_h, thumbnail_completed_cb, dstPath, &req_id);
	thumbnail_util_destroy(thumb_h);
	if (req_id) {
		g_free(req_id);
		req_id = NULL;
	}

	if (ret != THUMBNAIL_UTIL_ERROR_NONE) {
		MSG_DEBUG("thumbnail_util_extract is failed");
		g_mx.unlock();
		return false;
	}

	time_ret = g_cv.timedwait(g_mx.pMutex(), 5);

	g_mx.unlock();

	if (time_ret == ETIMEDOUT) {
		MSG_INFO("@@ WAKE by timeout@@");
		return false;
	}

	if (MsgAccessFile(dstPath, F_OK) == false) {
		MSG_DEBUG("not exist result file [%s]", dstPath);
		return false;
	}

	MSG_DEBUG("Make thumbnail: success [%s]", dstPath);
	return true;
}

bool MmsMakeVideoThumbnail(char *srcPath, char *dstPath)
{
	MMHandleType content_attrs = (MMHandleType)NULL;
	char *err_attr_name = NULL;
	int fileRet = 0;
	int trackCount = 0;

	if (srcPath == NULL || dstPath == NULL) {
		MSG_DEBUG(MMS_DEBUG_STR_INVALID_PARAM" src = %p, dst = %p", srcPath, dstPath);
		return false;
	}

	if (MsgAccessFile(srcPath, R_OK) == false) {
		MSG_DEBUG("not exist source file [%s]", srcPath);
		return false;
	}

	fileRet = mm_file_create_content_attrs(&content_attrs, srcPath);
	if (fileRet != 0) {
		mm_file_destroy_content_attrs(content_attrs);
		MSG_DEBUG("mm_file_create_content_attrs fail [%d]", fileRet);
		return false;
	}

	fileRet = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_VIDEO_TRACK_COUNT, &trackCount, NULL);
	if (fileRet != 0) {
		MSG_SEC_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);

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
		MSG_SEC_DEBUG("mm_file_get_attrs fails [%s]", err_attr_name);
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

	if (MsgAccessFile(dstPath, F_OK) == false) {
		MSG_DEBUG("not exist result file [%s]", dstPath);
		mm_file_destroy_content_attrs(content_attrs);
		return false;
	}

	MSG_DEBUG("Make thumbnail: success [%s]", dstPath);
	mm_file_destroy_content_attrs(content_attrs);
	return true;
}

FILE *MmsFileOpen(char *pFileName)
{
	mode_t file_mode = (S_IRUSR | S_IWUSR);

	if (!pFileName) {
		MSG_DEBUG("pFileName NULL: %s", strerror(errno));
		return NULL;
	}

	MSG_DEBUG("pFileName = %s", pFileName);

	FILE *pFile = MsgOpenFile(pFileName, "wb+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", strerror(errno));
		return NULL;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", strerror(errno));
		return NULL;
	}

	if (fchmod(fileno(pFile), file_mode) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File chmod Error: %s", strerror(errno));
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

//change character ' ' to '_'
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

	while(offset < srcLen && *(ptr) != '\0') {

		b1 = *(ptr);

		if ((b1 & 0x80) == 0) { //1byte : 0xxx xxxx
			offset += 1;
			ptr += 1;

			str += b1;
		} else  if ((b1 & 0xE0) == 0xC0) { //2byte : 110x xxxx
			offset += 2;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);

			if (b2 >= 0x80) { //10xx xxxx
				ptr += 2;
				str += replaceChar;
			} else {
				return NULL;
			}

		} else  if ((b1 & 0xF0) == 0xE0) { //3byte : 1110 xxxx

			offset += 3;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);

			if (b2 >= 0x80 && b3 >= 0x80) { //10xx xxxx
				ptr += 3;
				str += replaceChar;
			} else {
				return NULL;
			}

		} else  if ((b1 & 0xF8) == 0xF0) { //4byte : 1111 xxxx

			offset += 4;

			if (offset > srcLen)
				return NULL;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);
			b4 = *(ptr + 3);

			if (b2 >= 0x80 && b3 >= 0x80 && b4 >= 0x80) { //10xx xxxx
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

	while(offset < srcLen && *(ptr) != '\0') {

		b1 = *(ptr);

		if ((b1 & 0x80) == 0) { //1byte : 0xxx xxxx
			offset += 1;
			ptr += 1;
		} else  if ((b1 & 0xE0) == 0xC0) { //2byte : 110x xxxx
			offset += 2;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);

			if (b2 >= 0x80) { //10xx xxxx
				ptr += 2;
			} else {
				return false;
			}

		} else  if ((b1 & 0xF0) == 0xE0) { //3byte : 1110 xxxx

			offset += 3;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);

			if (b2 >= 0x80 && b3 >= 0x80) { //10xx xxxx
				ptr += 3;
			} else {
				return false;
			}

		} else  if ((b1 & 0xF8) == 0xF0) { //4byte : 1111 xxxx

			offset += 4;

			if (offset > srcLen)
				return false;

			b2 = *(ptr + 1);
			b3 = *(ptr + 2);
			b4 = *(ptr + 3);

			if (b2 >= 0x80 && b3 >= 0x80 && b4 >= 0x80) { //10xx xxxx
				ptr += 4;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}//while

	return true;
}
