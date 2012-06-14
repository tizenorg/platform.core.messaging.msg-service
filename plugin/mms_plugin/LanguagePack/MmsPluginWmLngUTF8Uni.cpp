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

#include "MsgDebug.h"
#include "MmsPluginWmLngPack.h"


/* Local function */
int __WmLngUTF2Unicode (unsigned short *des,unsigned char *src, int nChar);


int __WmLngStrlenByByte(UCHAR *szInText, int byteCount)
{
	int nCount = 0;
	while (byteCount > 0 && (*szInText != '\0')) {
		if (*szInText < 0x80) {
			szInText++;
			byteCount--;
		} else if  (((0xC0 <= *szInText) && (*szInText < 0xE0)) && (*(szInText+1) >= 0x80)) {
			szInText += 2;
			byteCount -= 2;
		} else if  ((*szInText >= 0xE0) && (*(szInText+1) >= 0x80) && (*(szInText+2) >= 0x80)) {
			szInText += 3;
			byteCount -= 3;
		} else {
			szInText++;
			byteCount--;
			MSG_DEBUG("__WmLngStrlenByByte: utf8 incorrect range!\n");
		}
		nCount++;
	}
	return (nCount+1)*sizeof(MCHAR);
}

/**
 * This function convert character n'th byte UTF8 character to ProcessCode(unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText		:	Input  Buffer Pointer to UTF8
 * @param	byteCount		:	byte number for converting the character
 * @return					:	This function returns true on success, or false
 *								on failure.
 *
 * @see		WmConvertPCode2UTF
 */
bool WmConvertUTF2PCode (MCHAR *pmszOutText, int outBufSize, UCHAR *szInText, int byteCount)
{
	int length;
	if (byteCount == 0) {
		pmszOutText[0] = NULL;
		return true;
	}

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, __WmLngStrlenByByte((UCHAR*)szInText, byteCount)) == false) {
		MSG_DEBUG("WmConvertUTF2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	length = __WmLngUTF2Unicode (pmszOutText, szInText, byteCount);
	if (length == -1) {
		MSG_DEBUG("WmConvertUTF2PCode: __WmLngUTF2Unicode returns false!\n");
		return false;
	} else {
		return true;
	}
}


/*
 * change 1byte-encoded-UTF8 character to Unicode
 * @param	byte1	:	1byte character code
 * @return			:	result of bit operation
 *
 *
 * @see
 */
unsigned short __WmLngConvert1ByteChar (unsigned char byte1)
{
	unsigned short result;

	result = 0;
	result = (byte1 & 0x7F);

	return result;
}

/*
 * change 2byte-encoded-UTF8 character to Unicode
 * @param	byte1	:	1'st byte character code
 * @param	byte2	:	2'st byte character code
 * @return			:	result of bit operation
 *
 * @see
 */
unsigned short __WmLngConvert2ByteChar (unsigned char byte1, unsigned char byte2)
{
	unsigned short result;
	unsigned char hi;
	unsigned char lo;

	result = 0;

	hi = byte1 & 0x1F;
	lo = byte2 & 0x3F;

	result = (hi << 6) | lo;

	return result;
}

/*
 * change 3byte-encoded-UTF8 character to Unicode
 * @param	byte1	:	1'st character code
 * @param	byte2	:	2'st character code
 * @param	byte3	:	3'st character code
 * @return			:	result of bit operation
 *
 * @see
 */
unsigned short __WmLngConvert3ByteChar (unsigned char byte1, unsigned char byte2, unsigned char byte3)
{
	unsigned short result;
	unsigned char hi;
	unsigned char mid;
	unsigned char lo;

	result = 0;

	hi = byte1 & 0x0F;
	mid = byte2 & 0x3F;
	lo = byte3 & 0x3F;

	result = (hi << 12) | (mid << 6) | lo;

	return result;
}

/*
 * This function convert character UTF8 to ProcessCode(unicode)
 *
 * @param	des		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	src		:	Input  Buffer Pointer to UTF8
 * @param	nChar	:	number for convert n'th chararcter
 * @return			:	This function returns number for convert n'th chararcter on success, or -1
 *						on failure.
 *
 * @see
 */

int __WmLngUTF2Unicode (unsigned short *des,unsigned char *src, int nChar)
{
	unsigned short *org;

	org = des;

	while (nChar > 0 && (*src != '\0')) {
		if (*src < 0x80) {
			*des = __WmLngConvert1ByteChar (*src);

			des++;
			src++;
			nChar--;
		} else if  (((0xC0 <= *src) && (*src < 0xE0)) && (*(src+1) >= 0x80)) {
			*des = __WmLngConvert2ByteChar (*src, *(src+1));

			des++;
			src += 2;
			nChar -= 2;
		} else if  ((*src >= 0xE0) && (*(src+1) >= 0x80) && (*(src+2) >= 0x80)) {
			*des = __WmLngConvert3ByteChar (*src, *(src+1), *(src+2));

			des++;

			src += 3;
			nChar -= 3;
		} else {
			*des = __WmLngConvert1ByteChar (*src);
			des++;
			src++;
			nChar--;
			MSG_DEBUG("__WmLngUTF2Unicode: utf8 incorrect range!\n");
		}
	}
	*des = 0;
	return (des - org);
}


