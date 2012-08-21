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

#include <stdlib.h>
#include <ctype.h>
#include "MsgDebug.h"
#include "MmsPluginCodec.h"
#include "MmsPluginCodec.h"
#include "MmsPluginMIME.h"
#include "MmsPluginMessage.h"
#include "MmsPluginWmLngPack.h"

const int MSG_MAX_CH_PER_LINE = 75;

/* ==================================================================
 *	Decode/Encode inline base64 string
 *
 * base64 : 3*8bit -> 4*6bit & convert the value into A~Z, a~z, 0~9, +, or /
 * pad(=) is needed when the end of the string is < 24bit.
 *
 *     Value Encoding  Value Encoding  Value Encoding  Value Encoding
 *         0 A            17 R            34 i            51 z
 *         1 B            18 S            35 j            52 '0'
 *         2 C            19 T            36 k            53 1
 *         3 D            20 U            37 l            54 2
 *         4 E            21 V            38 m            55 3
 *         5 F            22 W            39 n            56 4
 *         6 G            23 X            40 o            57 5
 *         7 H            24 Y            41 p            58 6
 *         8 I            25 Z            42 q            59 7
 *         9 J            26 a            43 r            60 8
 *        10 K            27 b            44 s            61 9
 *        11 L            28 c            45 t            62 +
 *        12 M            29 d            46 u            63 /
 *        13 N            30 e            47 v
 *        14 O            31 f            48 w         (pad) =
 *        15 P            32 g            49 x
 *        16 Q            33 h            50 y
 *
 * (1) the final quantum = 24 bits : no "=" padding,
 * (2) the final quantum = 8 bits : two "=" + two characters
 * (3) the final quantum = 16 bits : one "=" + three characters
 * ================================================================== */

bool _MsgEncodeBase64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret)
{
	unsigned char *d = NULL;
	unsigned char *s = (unsigned char *)pSrc;

	char *v = (char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned long i = ((srcLen + 2) / 3) * 4;

	i += 2 * ((i / 60) + 1);
	*len = i;

	if (ret == NULL) {
		MSG_DEBUG("_MsgEncodeBase64: ret Memory Alloc Fail \n");
		return false;
	}
	memset(ret, 0, i);

	d = ret;

	/* Convert 3*8bit into 4*6bit */
	for (i = 0; srcLen > 0; s += 3) {
		*d++ = v[s[0] >> 2];															// byte 1: high 6 bits of character-1
		*d++ = v[((s[0] << 4) + (--srcLen ? (s[1] >> 4) : 0)) & 0x3f];					// byte 2: low 2 bits of character-1 and high 4 bits of character-2
		*d++ = srcLen ? v[((s[1] << 2) + (--srcLen ? (s[2] >> 6) : 0)) & 0x3f] : '=';	// byte 3: low 4 bits of charcter-2 and high 2 bits of character-3
		*d++ = srcLen ? v[s[2] & 0x3f] : '=';											// byte 4: low 6 bits of character-3

		if (srcLen)
			srcLen--;

		/* Insert CRLF at every 60 characters */
		if ((++i) == 15) {
			i = 0;
			*d++ = '\015';
			*d++ = '\012';
		}
	}

	if (i == 15) {
		*d++ = '\015';
		*d++ = '\012';
	}
	*d = '\0';

	if (((unsigned long)(d - ret)) != *len) {
		*len = d - ret;
		MSG_DEBUG("base64 encoding length = %d \n", *len);
	}

	return true;
}


void *_MsgDecodeBase64(unsigned char *pSrc, unsigned long srcLen, unsigned long *len)
{
	char c;
	void *ret = NULL;
	char *d = NULL;
	short e = 0;

	ret = malloc((size_t)(*len = 4 + ((srcLen * 3) / 4)));
	d = (char *)ret;

	if (ret == NULL) {
		MSG_DEBUG("_MsgDecodeBase64: ret malloc Fail \n");
		return NULL;
	}

	memset(ret, 0, (size_t)*len);
	*len = 0;

	while (srcLen-- > 0) {
		c = *pSrc++;

		/* Convert base64 character into original value */

		if (isupper(c))
			c -= 'A';
		else if (islower(c))
			c -= 'a' - 26;
		else if (isdigit(c))
			c -= '0' - 52;
		else if (c == '+')
			c = 62;
		else if (c == '/')
			c = 63;
		else if (c == '=') {
			switch (e++) {
			case 2:
				if (*pSrc != '=') {
					*len = d - (char *)ret;
					return ret;
				}
				break;
			case 3:
				e = 0;
				break;
			default:
				*len = d - (char *)ret;
				return ret;
			}
			continue;
		} else
			continue;					// Actually, never get here

		/* Pad 4*6bit character into 3*8bit character */

		switch (e++) {
		case 0:
			*d = c << 2;			// byte 1: high 6 bits
			break;

		case 1:
			*d++ |= c >> 4;			// byte 1: low 2 bits
			*d = c << 4;			// byte 2: high 4 bits
			break;

		case 2:
			*d++ |= c >> 2;			// byte 2: low 4 bits
			*d = c << 6;			// byte 3: high 2 bits
			break;

		case 3:
			*d++ |= c;				// byte 3: low 6 bits
			e = 0;					// Calculate next unit.
			break;

		default:
			MSG_DEBUG("_MsgDecodeBase64: Unknown paremeter\n");
			break;
		}
	}

	*len = d - (char *)ret;			// Calculate the size of decoded string.

	return ret;
}



/* ==========================================
 *	Decode/Encode inline base64 string
 *
 * quoted-printable := ([*(ptext / SPACE / TAB) ptext] ["="] CRLF)
 *       ; Maximum line length of 76 characters excluding CRLF
 *
 * ptext := octet /<any ASCII character except "=", SPACE, or TAB>
 *       ; characters not listed as "mail-safe" in Appendix B
 *       ; are also not recommended.
 *
 * octet := "=" 2(DIGIT / "A" / "B" / "C" / "D" / "E" / "F")
 *       ; octet must be used for characters > 127, =, SPACE, or TAB.
 *
 * ==========================================*/

bool _MsgEncodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret)
{
	unsigned long lp = 0;
	unsigned char *d = ret;
	char *hex = (char *)"0123456789ABCDEF";
	unsigned char c;

	if (ret == NULL) {
		MSG_DEBUG("_MsgEncodeQuotePrintable: ret malloc Fail \n");
		return false;
	}

	d = ret;

	/*
	 * The type of srcLen is unsigned long
	 * The value of srcLen is decreased by 1 -> We can't check by "srcLen > 0".
	 */
	while (srcLen-- > 0) {
		/* Just copy CRLF */
		if (((c = *pSrc++) == '\015') && (*pSrc == '\012') && srcLen) {
			*d++ = '\015';
			*d++ = *pSrc++;
			srcLen--;
			lp = 0;
		} else {
			if (iscntrl(c) || (c == 0x7f) || (c & 0x80) || (c == '=') || ((c == ' ') && (*pSrc == '\015'))) {
				if ((lp += 3) > (unsigned long)MSG_MAX_CH_PER_LINE) {
					*d++ = '=';
					*d++ = '\015';
					*d++ = '\012';
					lp = 3;
				}

				*d++ = '=';				/* quote character */
				*d++ = hex[c >> 4];		/* high order 4 bits */
				*d++ = hex[c & 0xf];	/* low order 4 bits */
			} else {
				/* Just copy ASCII character */
				if ((++lp) > (unsigned long)MSG_MAX_CH_PER_LINE) {
					*d++ = '=';
					*d++ = '\015';
					*d++ = '\012';
					lp = 1;
				}
				*d++ = c;
			}
		}
	}

	*d = '\0';
	*len = d - ret;

	return true;
}


unsigned char *_MsgDecodeQuotePrintable(unsigned char *pSrc, unsigned long srcLen, unsigned long *len)
{
	unsigned char *ret = NULL;
	unsigned char *d = NULL;
	unsigned char *s = NULL;					/* last non-blank */
	unsigned char c;
	unsigned char e;

	d = s = ret = (unsigned char *)malloc((size_t)srcLen + 1);
	if (ret == NULL) {
		MSG_DEBUG("_MsgDecodeQuotePrintable: ret malloc Fail \n");
		return NULL;
	}

	*len = 0;
	pSrc[srcLen] = '\0';

	while ((c = *pSrc++)!= '\0') {
		switch (c) {
		case '=':							/* octet characters (> 127, =, SPACE, or TAB) */
			switch (c = *pSrc++) {
			case '\0':					/* end of string -> postpone to while */
				break;

			case '\015':				/* CRLF */
				if (*pSrc == '\012')
					pSrc++;
				break;

			default:					/* two hexes */
				if (!isxdigit(c)) {
					*d = '\0';
					*len = d - ret;
					return ret;
				}

				if (isdigit(c))
					e = c - '0';
				else
					e = c - (isupper(c) ? 'A' - 10 : 'a' - 10);

				c = *pSrc++;
				if (!isxdigit(c)) {
					*d = '\0';
					*len = d - ret;
					return ret;
				}

				if (isdigit(c))
					c -= '0';
				else
					c -= (isupper(c) ? 'A' - 10 : 'a' - 10);

				*d++ = c + (e << 4);
				s = d;
				break;
			}
			break;

		case ' ':							/* skip the blank */
			*d++ = c;
			break;

		case '\015':						/* Line Feedback : to last non-blank character */
			d = s;
			break;

		default:
			*d++ = c;						/* ASCII character */
			s = d;
			break;
		}
	}

	*d = '\0';
	*len = d - ret;

	return ret;
}


/* ========================================
 * Decode/Encode inline base64 string
 * Inline base64 has no "\r\n" in it,
 * and has charset and encoding sign in it
 * ======================================== */
bool _MsgEncode2Base64(void *pSrc, unsigned long srcLen, unsigned long *len, unsigned char *ret)
{
	unsigned char *d = NULL;
	unsigned char *s = (unsigned char *)pSrc;
	char *v = (char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned long i = ((srcLen + 2) / 3) * 4;

	i += 2 * ((i / 60) + 1);
	*len = i;

	if (ret == NULL) {
		MSG_DEBUG("_MsgEncode2Base64: ret Memory Alloc Fail \n");
		return false;
	}
	memset(ret, 0, i);

	d = ret;

	/* Convert 3*8bit into 4*6bit */
	for (i = 0; srcLen > 0; s += 3) {
		*d++ = v[s[0] >> 2];															// byte 1: high 6 bits of character-1
		*d++ = v[((s[0] << 4) + (--srcLen ? (s[1] >> 4) : 0)) & 0x3f];					// byte 2: low 2 bits of character-1 and high 4 bits of character-2
		*d++ = srcLen ? v[((s[1] << 2) + (--srcLen ? (s[2] >> 6) : 0)) & 0x3f] : '=';	// byte 3: low 4 bits of charcter-2 and high 2 bits of character-3
		*d++ = srcLen ? v[s[2] & 0x3f] : '=';											// byte 4: low 6 bits of character-3

		if (srcLen)
			srcLen--;
	}

	*d = '\0';

	if (((unsigned long)(d - ret)) != *len) {
		*len = d - ret;
		MSG_DEBUG("base64 encoding length = %d \n", *len);
	}

	return true;
}


char *_MsgDecodeText(char *pOri)
{
	MSG_BEGIN();

	int size = 0;
	int cnt = 0;
	char *pSrc = NULL;
	char *pTemp = NULL;
	char *pRe = NULL;
	char *pStrEnd = NULL;
	char *pDecStart = NULL;
	char *pDecEnd = NULL;
	char *pDecQ = NULL;
	char *pDecQ2 = NULL;
	bool bEncoding = false;
	int	nCharset = MSG_CHARSET_UTF8;
	int	nChar = 0;
	int	nByte = 0;
	int	nTemp = 0;
	char *pReturnStr = NULL;
	char *pConvertedStr = NULL;
	MCHAR *mszTempStr = NULL;

	MCHAR *pmszOutTextStr = NULL;

	char szTempBuf[MSG_LOCAL_TEMP_BUF_SIZE] = {0};

	// copy original string
	if (strlen(pOri) >= MSG_LOCAL_TEMP_BUF_SIZE) {
		pSrc = MsgStrCopy( pOri );
	} else {
		memset(szTempBuf, 0, MSG_LOCAL_TEMP_BUF_SIZE);
		strcpy(szTempBuf, pOri);

		pSrc = szTempBuf;
	}

	// it can be one or more encoding methods in a line
	while (1) {
		cnt++;

		bEncoding = false;

		/*
		  (ex) "=?euc-kr?B?Y2NqMjEyMw==?="

		  pDecStart: charset			(=?euc-kr?B?Y2NqMjEyMw==?=)
		  pDecQ	: Encoding type		(B?Y2NqMjEyMw==?=)
		  pDecQ2	: Encoded text		(Y2NqMjEyMw==?=)
		  pDecEnd	: Encoded of text	(?=)
		 */
		if (pSrc == NULL)
			goto __CATCH;

		if (((pDecStart = strstr(pSrc, MSG_STR_DEC_START)) != NULL) 	//"=?"
		     && ((pDecQ = strchr(pDecStart + 2, MSG_CH_QUESTION)) != NULL)	// '?'
		     && ((pDecQ2 = strchr(pDecQ + 1, MSG_CH_QUESTION))!= NULL)		// '?'
		     && ((pDecEnd = strstr(pDecQ2 + 1, MSG_STR_DEC_END))!= NULL)) {	//"=?"
			bEncoding = true;

			/* charset problem
			 * pDecStart ~ pDecQ : charSet & MSG_CHARSET_USC2 ~ MSG_CHARSET_UTF8 & LATIN
			 */

			*pDecQ = '\0';
			nCharset = _MsgGetCode(MSG_CHARSET, pDecStart + 2);
			*pDecQ = MSG_CH_QUESTION;
		}

		// End of encoding
		if (!bEncoding)
			goto __RETURN;

		// find end of string
		pStrEnd = pSrc + strlen(pSrc);

		// Decoding
		if ((*(pDecQ2 - 1) == MSG_CH_BASE64_UPPER) ||
			(*(pDecQ2 - 1) == MSG_CH_BASE64_LOWER) ||
			(*(pDecQ + 1) == MSG_CH_BASE64_UPPER) ||
			(*(pDecQ + 1) == MSG_CH_BASE64_LOWER)) {
			pTemp = (char *)_MsgDecodeBase64((UCHAR *)(pDecQ2 + 1), (ULONG)(pDecEnd - pDecQ2 - 1), (ULONG *)&size);

			if (pTemp != NULL) {
				pTemp[size] = MSG_CH_NULL;

				if(pRe) {
					free(pRe);
					pRe = NULL;
				}

				pRe = (char *)malloc((pDecStart-pSrc) + size + (pStrEnd - (pDecEnd + 2)) + 1);
				if (pRe == NULL) {
					MSG_DEBUG("_MsgDecodeText: pRemalloc fail \n");
					free(pTemp);
					pTemp = NULL;

					goto __RETURN;
				}

				memcpy(pRe, pSrc, pDecStart - pSrc);
				memcpy(&pRe[pDecStart-pSrc], pTemp, size);
				memcpy(&pRe[(pDecStart - pSrc) + size], pDecEnd + 2, pStrEnd - (pDecEnd + 2));
				pRe[(pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2))] = MSG_CH_NULL;

				free(pTemp);
				pTemp = NULL;

				if (pSrc != NULL && pSrc != szTempBuf) {
					free(pSrc);
					pSrc = NULL;
				}
			}
		} else if ((*(pDecQ2-1) == MSG_CH_QPRINT_UPPER) ||
				(*(pDecQ2-1) == MSG_CH_QPRINT_LOWER) ||
				(*(pDecQ+1) == MSG_CH_QPRINT_UPPER) ||
				(*(pDecQ+1) == MSG_CH_QPRINT_LOWER)) {

			pTemp = (char *)_MsgDecodeQuotePrintable((UCHAR *)( pDecQ2 + 1 ), (ULONG)(pDecEnd - pDecQ2 - 1), (ULONG *)&size);

			if (pTemp != NULL) {
				int i;
				pTemp[size] = MSG_CH_NULL;

				for (i = 0; i < size; i++) {
					if (pTemp[i] == MSG_CH_UNDERLINE) {
						pTemp[i] = MSG_CH_SP;	                      // change '_' to ' '
					}
				}

				if(pRe) {
					free(pRe);
					pRe = NULL;
				}

				pRe = (char *)malloc((pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2)) + 1);
				if (pRe == NULL) {
					MSG_DEBUG("_MsgDecodeText: pRemalloc fail \n");
					free(pTemp);
					pTemp = NULL;

					goto __RETURN;
				}

				memcpy(pRe, pSrc, pDecStart - pSrc);
				memcpy(&pRe[pDecStart - pSrc], pTemp, size);
				memcpy(&pRe[(pDecStart - pSrc) + size], pDecEnd + 2, pStrEnd - (pDecEnd + 2));
				pRe[(pDecStart - pSrc) + size + (pStrEnd - (pDecEnd + 2))] = MSG_CH_NULL;

				if (pTemp) {
					free(pTemp);
					pTemp = NULL;
				}

				if (pSrc != NULL && pSrc != szTempBuf) {
					free(pSrc);
					pSrc = NULL;
				}
			}
		} else {
			goto __RETURN;
		}
	}

__RETURN:

	pTemp = pSrc;
	nTemp = strlen(pSrc);

	switch (nCharset) {
	case MSG_CHARSET_UTF16:
	case MSG_CHARSET_USC2:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_USC2 \n");

		if (pTemp) {
			// mmf file name display patch
			if (((UINT8)pTemp[0] == 0xFF && (UINT8)pTemp[1] == 0xFE) || ((UINT8)pTemp[0] == 0xFE && (UINT8)pTemp[1] == 0xFF)) {

				nChar = (nTemp / 2 - 1);

				// Re check char-set
				if (MsgIsUTF8String((unsigned char *)pTemp + 2, nTemp - 2)) {
					strncpy(pTemp, pTemp + 2, strlen(pTemp + 2));
					nTemp = nTemp - 2;
					MSG_DEBUG("_MsgDecodeText: real char-set = MSG_CHARSET_UTF8.\n");
					break;
				}

				mszTempStr = (unsigned short *)malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("_MsgDecodeText: 1. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short *)pTemp + 1), nChar * sizeof(unsigned short));

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short *)pTemp + 1), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr) {
					MsgUnicode2UTF ((unsigned char *)pConvertedStr, nByte + 1, mszTempStr, nChar);
				}
			} else {
				nChar = (nTemp / 2);

				if (nChar == 0) {
					nChar = 2;
				}

				// Re check char-set
				if (MsgIsUTF8String((unsigned char *)pTemp, nTemp)) {
					MSG_DEBUG("_MsgDecodeText: real char-set = MSG_CHARSET_UTF8.\n");
					break;
				}

				mszTempStr = (unsigned short *)malloc(nChar * sizeof(unsigned short));
				if (mszTempStr == NULL) {
					MSG_DEBUG("_MsgDecodeText: 2. Memory Full !!! \n");
					goto __CATCH;
				}

				memcpy(mszTempStr, ((unsigned short *)pTemp), nChar * sizeof(unsigned short));

				nByte = MsgGetUnicode2UTFCodeSize(((unsigned short *)pTemp), nChar);

				pConvertedStr = (char *)malloc(nByte + 1);
				if (pConvertedStr) {
					MsgUnicode2UTF ((unsigned char *)pConvertedStr, nByte + 1, mszTempStr, nChar);
				}
			}
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_US_ASCII:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_US_ASCII \n");
		break;

	case MSG_CHARSET_UTF8:

		/* UTF8 is  default charset of Messenger */

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_UTF8 \n");

		break;

	case MSG_CHARSET_ISO_8859_1:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_1 \n");

		nByte = nTemp * 3;

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatinCode2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_2:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_2 \n");

		nByte = nTemp * 3;

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr =  (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin2Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_3:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_3 \n");

		nByte = WmGetLatin32UTFCodeSize((unsigned char *)pTemp, nTemp);

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin3Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_4:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_4 \n");

		nByte = WmGetLatin42UTFCodeSize((unsigned char *)pTemp, nTemp);

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin4Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_5:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_5 \n");

		nByte = WmGetLatin52UTFCodeSize((unsigned char *)pTemp, nTemp);

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin5Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_7:

		/* Greek */
		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_9 \n");

		nByte = MsgGetLatin72UTFCodeSize((unsigned char *)pTemp, nTemp);
		pConvertedStr = (char *)malloc( nByte + 1);
		if (pConvertedStr) {
			MsgLatin7code2UTF((unsigned char *)pConvertedStr, nByte + 1 , (unsigned char *)pTemp, nTemp);
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_8:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_8 \n");

		nByte = WmGetLatin82UTFCodeSize((unsigned char *)pTemp, nTemp);

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin8Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_9:
		/* Turkish */

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_9 \n");

		nByte = MsgGetLatin52UTFCodeSize((unsigned char *)pTemp, nTemp);
		pConvertedStr = (char *)malloc(nByte + 1);
		if (pConvertedStr) {
			MsgLatin5code2UTF((unsigned char *)pConvertedStr, nByte + 1 , (unsigned char *)pTemp, nTemp);
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_ISO_8859_15:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_ISO_8859_15 \n");

		nByte = WmGetLatin152UTFCodeSize((unsigned char *)pTemp, nTemp);

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertLatin15Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_WIN1251:
	case MSG_CHARSET_WINDOW_1251:
	case MSG_CHARSET_WINDOWS_1251:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_WINDOWS_1251 \n");

		nByte = nTemp * 3;

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertWin1251Code2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_KOI8_R:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_KOI8_R \n");

		nByte = nTemp * 3;

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertKoi8rCode2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	case MSG_CHARSET_KOI8_U:

		MSG_DEBUG("_MsgDecodeText: MSG_CHARSET_KOI8_U \n");

		nByte = nTemp * 3;

		pmszOutTextStr = (unsigned short *)malloc(sizeof(MCHAR *) * (nByte + 1));
		if (pmszOutTextStr == NULL) {
			MSG_DEBUG("_MsgDecodeText : Out Text String null !!! \n");
			goto __CATCH;
		}

		pConvertedStr = (char *)malloc(sizeof(char *) * (nByte + 1));
		if (pConvertedStr) {
			WmConvertKoi8uCode2PCode(pmszOutTextStr, sizeof(MCHAR *) * (nByte + 1), pTemp);
			WmConvert2LCode(pConvertedStr, sizeof(char *) * (nByte + 1), pmszOutTextStr);
		}

		if (pmszOutTextStr) {
			free(pmszOutTextStr);
			pmszOutTextStr = NULL;
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;

	default:

		MSG_DEBUG("_MsgDecodeText: Other charsets \n");

		nByte = MsgGetLatin2UTFCodeSize((unsigned char *)pTemp, nTemp);
		pConvertedStr = (char *)malloc(nByte + 1);
		if (pConvertedStr) {
			MsgLatin2UTF((unsigned char *)pConvertedStr, nByte + 1, (unsigned char *)pTemp, nTemp);
		}

		pTemp = pConvertedStr;
		nTemp = nByte;

		break;
	}

	pReturnStr = (char *)malloc(nTemp + 1);
	if (pReturnStr == NULL) {
		goto __CATCH;
	}
	memset(pReturnStr, 0, nTemp + 1);

	if (pTemp)
		memcpy(pReturnStr, pTemp, nTemp);

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	if (mszTempStr) {
		free(mszTempStr);
		mszTempStr = NULL;
	}

	if(pRe) {
		free(pRe);
		pRe = NULL;
	}

	if (pSrc != NULL && pSrc != szTempBuf) {
		free(pSrc);
		pSrc = NULL;
	}

	return pReturnStr;

__CATCH:

	if (pConvertedStr) {
		free(pConvertedStr);
		pConvertedStr = NULL;
	}

	if (mszTempStr)	{
		free(mszTempStr);
		mszTempStr = NULL;
	}

	if(pRe) {
		free(pRe);
		pRe = NULL;
	}

	if (pSrc != NULL && pSrc != szTempBuf) {
		free(pSrc);
		pSrc = NULL;
	}

	return NULL;
}


char *MsgEncodeText(char *pOri)
{
	ULONG nLen = 0;
	char *szBuff = NULL;
	int length  = 0;

	length = (strlen(pOri) * 4) / 3 + 2 + 12 + 1 + 30;
	szBuff = (char *)malloc(length + 1);

	if (szBuff == NULL) {
		// error handling
		MSG_DEBUG("_MsgEncodeText: szBuff alloc is failed \n");
		goto __CATCH;
	}

	memset(szBuff, 0 , length + 1);

	snprintf(szBuff, length+1, "%s%s%c%c%c", MSG_STR_DEC_START, "utf-8", MSG_CH_QUESTION, MSG_CH_BASE64_LOWER, MSG_CH_QUESTION);

	if (_MsgEncode2Base64((unsigned char *)pOri, strlen(pOri), &nLen, (unsigned char *)szBuff + 10) == false) {
		MSG_DEBUG("_MsgEncodeText: MsgEncodeBase64() is failed \n");
		goto __CATCH;
	}

	strcat(szBuff, MSG_STR_DEC_END);

	return szBuff;

__CATCH:
	if (szBuff) {
		free(szBuff);
	}

	return false;
}

