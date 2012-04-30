/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#include "MsgDebug.h"
#include "MmsPluginWmLngPack.h"


/* Local function */
int __WmLngUnicode2UTF (unsigned char *des, unsigned short *src, int nChar);


int __WmLngGetLCodeSizeN(MCHAR *mszText, int charCount)
{
	int nBuffSize = 0;

	if (mszText[0] == '\0')
		return 0;

	while (charCount > 0 && *mszText != '\0') {

		if (0x0001 <= *mszText && *mszText <= 0x007F) {
			nBuffSize++;
			mszText++;
			charCount--;
		} else if((*mszText == 0x0000) || (0x0080 <= *mszText && *mszText <= 0x07FF)) {
			nBuffSize += 2;
			mszText++;
			charCount--;
		} else {
			nBuffSize += 3;
			mszText++;
			charCount--;
		}
	}
	return nBuffSize+1;	/* include NULL  */
}

/**
 * This function convert character ProcessCode(unicode) to UTF8
 *
 * @param	pszOutText	:	Output Buffer Pointer to UTF8
 * @param	mszInText	:	Input  Buffer Pointer to ProcessCode(UniCode)
 * @param	charCount	:	number for convert n'th chararcter
 * @return	This function returns true on success, or false
 *			on failure.
 *
 * @see		WmConvertPCode2UTF
 */
bool WmConvertPCode2UTF(UCHAR *pszOutText, int outBufSize, MCHAR *mszInText, int charCount)
{
	int length;

	if (charCount == 0) {
		pszOutText[0] = '\0';
		return true;
	}

	if (__WmConvertCodeBufferSizeCheck((char*)pszOutText, outBufSize, __WmLngGetLCodeSizeN(mszInText, charCount)) == false) {
		MSG_DEBUG("WmConvertPCode2UTF: Out buffer size seems to be small!\n");
		return false;
	}

	length = __WmLngUnicode2UTF(pszOutText, mszInText, charCount);
	if(length == -1) {
		MSG_DEBUG("WmConvertPCode2UTF: __WmLngUnicode2UTF returns false!\n");
		return false;
	} else {
		return true;
	}
}

/*
 * This function convert character ProcessCode(unicode) to UTF8
 *
 * @param	des		:	Output Buffer Pointer to UTF8
 * @param	src		:	Input  Buffer Pointer to ProcessCode(UniCode)
 * @param	nChar	:	number for convert n'th chararcter
 * @return	This function returns number for convert n'th chararcter on success, or -1
 *			on failure.
 *
 * @see		WmConvertPCode2UTF
 */
int __WmLngUnicode2UTF (unsigned char *des, unsigned short *src, int nChar)
{
	unsigned char *org;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	org = des;

	while (nChar > 0 && *src != '\0') {
		if (0x0001 <= *src && *src <= 0x007F) {
			*des = (unsigned char) (*src & 0x007F);

			des++;
			src++;
			nChar--;
		} else if  ((*src == 0x0000) || (0x0080 <= *src && *src <= 0x07FF)) {
			t2 = (unsigned char) (*src & 0x003F);				//	right most 6 bit
			t1 = (unsigned char) ((*src & 0x07C0) >> 6);			//	right most 5 bit

			*des = 0xC0 | (t1 & 0x1F);
			*(des+1) = 0x80 | (t2 & 0x3F);

			des += 2;
			src += 1;
			nChar -= 1;
		} else {
			t3 = (unsigned char) (*src & 0x003F);					//	right most 6 bit
			t2 = (unsigned char) ((*src & 0x0FC0) >> 6);			//	right most 6 bit
			t1 = (unsigned char) ((*src & 0xF000) >> 12);			//	right most 4 bit

			*des = 0xE0 | (t1 & 0x0F);
			*(des+1) = 0x80 | (t2 & 0x3F);
			*(des+2) = 0x80 | (t3 & 0x3F);

			des += 3;
			src += 1;
			nChar -= 1;
		}
	}

	*des = 0;
	return (des - org);
}



