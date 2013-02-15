/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <glib.h>

#include "MsgDebug.h"
#include "MsgTextConvert.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgConvertText - Member Functions
==================================================================================================*/

MsgTextConvert::MsgTextConvert()
{
	extCharList.clear();
	ucs2toGSM7DefList.clear();
	ucs2toGSM7ExtList.clear();
	ucs2toTurkishList.clear();
	ucs2toSpanishList.clear();
	ucs2toPortuList.clear();

	extCharList[0x000C] = MSG_CHAR_GSM7EXT;
	extCharList[0x005B] = MSG_CHAR_GSM7EXT;
	extCharList[0x005C] = MSG_CHAR_GSM7EXT;
	extCharList[0x005D] = MSG_CHAR_GSM7EXT;
	extCharList[0x005E] = MSG_CHAR_GSM7EXT;
	extCharList[0x007B] = MSG_CHAR_GSM7EXT;
	extCharList[0x007C] = MSG_CHAR_GSM7EXT;
	extCharList[0x007D] = MSG_CHAR_GSM7EXT;
	extCharList[0x007E] = MSG_CHAR_GSM7EXT;
	extCharList[0x20AC] = MSG_CHAR_GSM7EXT; // ��

	extCharList[0x00E7] = MSG_CHAR_TURKISH;
	extCharList[0x011E] = MSG_CHAR_TURKISH;
	extCharList[0x011F] = MSG_CHAR_TURKISH;
	extCharList[0x01E6] = MSG_CHAR_TURKISH;
	extCharList[0x01E7] = MSG_CHAR_TURKISH;
	extCharList[0x0130] = MSG_CHAR_TURKISH;
	extCharList[0x0131] = MSG_CHAR_TURKISH;
	extCharList[0x015E] = MSG_CHAR_TURKISH;
	extCharList[0x015F] = MSG_CHAR_TURKISH;

	extCharList[0x00C1] = MSG_CHAR_SPANISH;
	extCharList[0x00E1] = MSG_CHAR_SPANISH;
	extCharList[0x00CD] = MSG_CHAR_SPANISH;
	extCharList[0x00ED] = MSG_CHAR_SPANISH;
	extCharList[0x00D3] = MSG_CHAR_SPANISH;
	extCharList[0x00F3] = MSG_CHAR_SPANISH;
	extCharList[0x00DA] = MSG_CHAR_SPANISH;
	extCharList[0x00FA] = MSG_CHAR_SPANISH;

	extCharList[0x00D4] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00F4] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00CA] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00EA] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00C0] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00E7] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00C3] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00E3] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00D5] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00F5] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00C2] = MSG_CHAR_PORTUGUESE;
	extCharList[0x00E2] = MSG_CHAR_PORTUGUESE;

	for (unsigned char i = 0; i < 128; i++)
	{
		ucs2toGSM7DefList[g_GSM7BitToUCS2[i]] = i;
	}

	// GSM 7 bit Extension
	ucs2toGSM7ExtList[0x005B] = 0x3C; // [
	ucs2toGSM7ExtList[0x005D] = 0x3E; // ]
	ucs2toGSM7ExtList[0x007B] = 0x28; // {
	ucs2toGSM7ExtList[0x007D] = 0x29; // }
	ucs2toGSM7ExtList[0x000C] = 0x0A; // Page Break
	ucs2toGSM7ExtList[0x005C] = 0x2F; /* \ */
	ucs2toGSM7ExtList[0x005E] = 0x14; // ^
	ucs2toGSM7ExtList[0x007C] = 0x40; // |
	ucs2toGSM7ExtList[0x007E] = 0x3D; // ~
	ucs2toGSM7ExtList[0x20AC] = 0x65; // ��

	// Turkish
	ucs2toTurkishList[0x005B] = 0x3C; // [
	ucs2toTurkishList[0x005D] = 0x3E; // ]
	ucs2toTurkishList[0x007B] = 0x28; // {
	ucs2toTurkishList[0x007D] = 0x29; // }
	ucs2toTurkishList[0x000C] = 0x0A; // Page Break
	ucs2toTurkishList[0x005C] = 0x2F; /* \ */
	ucs2toTurkishList[0x005E] = 0x14; // ^
	ucs2toTurkishList[0x007C] = 0x40; // |
	ucs2toTurkishList[0x007E] = 0x3D; // ~
	ucs2toTurkishList[0x20AC] = 0x65; // ��
	ucs2toTurkishList[0x00E7] = 0x63; // c LATIN SMALL LETTER S WITH CEDILLA *
	ucs2toTurkishList[0x011E] = 0x47; // G LATIN CAPITAL LETTER G WITH BREVE
	ucs2toTurkishList[0x011F] = 0x67; // g LATIN SMALL LETTER G WITH BREVE
	ucs2toTurkishList[0x01E6] = 0x47; // G LATIN CAPITAL LETTER G WITH CARON
	ucs2toTurkishList[0x01E7] = 0x67; // g LATIN SMALL LETTER G WITH CARON
	ucs2toTurkishList[0x0130] = 0x49; // I LATIN CAPITAL LETTER I WITH DOT ABOVE
	ucs2toTurkishList[0x0131] = 0x69; // i LATIN SMALL LETTER DOTLESS
	ucs2toTurkishList[0x015E] = 0x53; // S LATIN CAPITAL LETTER S WITH CEDILLA *
	ucs2toTurkishList[0x015F] = 0x73; // s LATIN SMALL LETTER S WITH CEDILLA *

	// Spanish
	ucs2toSpanishList[0x005B] = 0x3C; // [
	ucs2toSpanishList[0x005D] = 0x3E; // ]
	ucs2toSpanishList[0x007B] = 0x28; // {
	ucs2toSpanishList[0x007D] = 0x29; // }
	ucs2toSpanishList[0x000C] = 0x0A; // Page Break
	ucs2toSpanishList[0x005C] = 0x2F; /* \ */
	ucs2toSpanishList[0x005E] = 0x14; // ^
	ucs2toSpanishList[0x007C] = 0x40; // |
	ucs2toSpanishList[0x007E] = 0x3D; // ~
	ucs2toSpanishList[0x20AC] = 0x65; // ��
	ucs2toSpanishList[0x00C1] = 0x41; // A
	ucs2toSpanishList[0x00E1] = 0x61; // a
	ucs2toSpanishList[0x00CD] = 0x49; // I
	ucs2toSpanishList[0x00ED] = 0x69; // i
	ucs2toSpanishList[0x00D3] = 0x4F; // O
	ucs2toSpanishList[0x00F3] = 0x6F; // o
	ucs2toSpanishList[0x00DA] = 0x55; // U
	ucs2toSpanishList[0x00FA] = 0x75; // u

	// Portuguese
	ucs2toPortuList[0x005B] = 0x3C; // [
	ucs2toPortuList[0x005D] = 0x3E; // ]
	ucs2toPortuList[0x007B] = 0x28; // {
	ucs2toPortuList[0x007D] = 0x29; // }
	ucs2toPortuList[0x000C] = 0x0A; // Page Break
	ucs2toPortuList[0x005C] = 0x2F; /* \ */
	ucs2toPortuList[0x005E] = 0x14; // ^
	ucs2toPortuList[0x007C] = 0x40; // |
	ucs2toPortuList[0x007E] = 0x3D; // ~
	ucs2toPortuList[0x20AC] = 0x65; // ��
	ucs2toPortuList[0x00D4] = 0x0B; // O
	ucs2toPortuList[0x00F4] = 0x0C; // o
	ucs2toPortuList[0x00C1] = 0x0E; // A
	ucs2toPortuList[0x00E1] = 0x0F; // a
	ucs2toPortuList[0x00CA] = 0x1F; // E
	ucs2toPortuList[0x00EA] = 0x05; // e
	ucs2toPortuList[0x00C0] = 0x41; // A
	ucs2toPortuList[0x00E7] = 0x09; // c
	ucs2toPortuList[0x00CD] = 0x49; // I
	ucs2toPortuList[0x00ED] = 0x69; // i
	ucs2toPortuList[0x00D3] = 0x4F; // O
	ucs2toPortuList[0x00F3] = 0x6F; // o
	ucs2toPortuList[0x00DA] = 0x55; // U
	ucs2toPortuList[0x00FA] = 0x75; // u
	ucs2toPortuList[0x00C3] = 0x61; // A
	ucs2toPortuList[0x00E3] = 0x7B; // a
	ucs2toPortuList[0x00D5] = 0x5C; // O
	ucs2toPortuList[0x00F5] = 0x7C; // o
	ucs2toPortuList[0x00C2] = 0x61; // A
	ucs2toPortuList[0x00E2] = 0x7F; // a
	ucs2toPortuList[0x03A6] = 0x12; // ��
	ucs2toPortuList[0x0393] = 0x13; // ��
	ucs2toPortuList[0x03A9] = 0x15; // ��
	ucs2toPortuList[0x03A0] = 0x16; // ��
	ucs2toPortuList[0x03A8] = 0x17; // ��
	ucs2toPortuList[0x03A3] = 0x18; // ��
	ucs2toPortuList[0x0398] = 0x19; // ��
}


MsgTextConvert::~MsgTextConvert()
{
	extCharList.clear();
	ucs2toGSM7DefList.clear();
	ucs2toGSM7ExtList.clear();
	ucs2toTurkishList.clear();
	ucs2toSpanishList.clear();
	ucs2toPortuList.clear();
}



int MsgTextConvert::convertUTF8ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *pLangId, OUT bool *bIncludeAbnormalChar)
{
	int utf8Length = 0;
	int gsm7bitLength = 0;
	int ucs2Length = 0;

	if (srcTextLen <= 0)
	{
		utf8Length = strlen((char*)pSrcText);
		srcTextLen = utf8Length;
	}
	else
	{
		utf8Length = srcTextLen;
	}

	int maxUCS2Length = utf8Length;		// max # of UCS2 chars, NOT bytes. when all utf8 chars are only one byte, UCS2Length is maxUCS2 Length. otherwise (ex: 2 bytes of UTF8 is one char) UCS2Length must be  less than utf8Length
	WCHAR pUCS2Text[maxUCS2Length];
	memset(pUCS2Text, 0x00, maxUCS2Length * sizeof(WCHAR));

	MSG_DEBUG("srcTextLen = %d", srcTextLen);
	MSG_DEBUG("temp buffer size = %d", maxUCS2Length * sizeof(WCHAR));
	MSG_DEBUG("max dest Length = %d", maxLength);

	ucs2Length = convertUTF8ToUCS2((unsigned char*)pUCS2Text, maxUCS2Length * sizeof(WCHAR), pSrcText, srcTextLen);
	gsm7bitLength = convertUCS2ToGSM7bit(pDestText, maxLength, (unsigned char*)pUCS2Text, ucs2Length, pLangId, bIncludeAbnormalChar);

	return gsm7bitLength;
}


/**
	if srcTextLen ispSrcText should be null terminated
return :
 		byte length of converted UCS2 characters
			-1 : converting error
*/
int MsgTextConvert::convertUTF8ToUCS2(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen)
{
	int i, j;
	int textLen;
	unsigned char *unicodeTemp = (unsigned char*)pDestText;
	int ucs2Length = 0;
	int remainedBuffer = maxLength;

#ifdef CONVERT_DUMP
	int srcLen = srcTextLen;
	const unsigned char * pTempSrcText = pSrcText;
	const unsigned char * pTempDestText = pDestText;
#endif

	i = j = 0;

	if(maxLength == 0 || pSrcText == NULL || pDestText ==  NULL)
	{
		MSG_DEBUG("UTF8 to UCS2 Failed as text length is 0\n");
		return -1;
	}

	// null terminated string
	if ( srcTextLen == -1 )
	{
		textLen = strlen((char*)pSrcText);
		srcTextLen = textLen;
	}
	else
	{
		textLen = srcTextLen;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open("UCS-2BE", "UTF8");

	if (cd > 0)
	{
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&textLen, (char**)&unicodeTemp, (gsize*)&remainedBuffer);
	}

	if(err < 0)
	{
		MSG_DEBUG("Error in g_iconv.");
		ucs2Length = -1;
	}
	else
	{
		ucs2Length = maxLength - remainedBuffer;
	}

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump UTF8 -> UCS2\n");
	convertDumpTextToHex((unsigned char*)pTempSrcText, srcLen);
	convertDumpTextToHex((unsigned char*)pTempDestText, ucs2Length);
#endif

	g_iconv_close(cd);

	return ucs2Length;
}


int MsgTextConvert::convertUTF8ToAuto(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT msg_encode_type_t *pCharType)
{
	int utf8Length = 0;
	int gsm7bitLength = 0;
	int ucs2Length = 0;

	bool bUnknown = false;

	utf8Length = srcTextLen;

	int maxUCS2Length = utf8Length;		// max # of UCS2 chars, NOT bytes. when all utf8 chars are only one byte, UCS2Length is maxUCS2 Length. otherwise (ex: 2 bytes of UTF8 is one char) UCS2Length must be  less than utf8Length
	WCHAR pUCS2Text[maxUCS2Length];
	memset(pUCS2Text, 0x00, maxUCS2Length * sizeof(WCHAR));

	MSG_DEBUG("srcTextLen = %d", srcTextLen);
	MSG_DEBUG("temp buffer size = %d", maxUCS2Length * sizeof(WCHAR));
	MSG_DEBUG("max dest Length = %d", maxLength);

	ucs2Length = convertUTF8ToUCS2((unsigned char*)pUCS2Text, maxUCS2Length * sizeof(WCHAR), pSrcText, srcTextLen);

	if(ucs2Length < 0)
	{
		*pCharType = MSG_ENCODE_8BIT;

		memcpy(pDestText, pSrcText, srcTextLen);
		return srcTextLen;
	}
	else
	{
		gsm7bitLength = convertUCS2ToGSM7bitAuto(pDestText, maxLength, (unsigned char*)pUCS2Text, ucs2Length, &bUnknown);

		if (bUnknown == true)
		{
			*pCharType = MSG_ENCODE_UCS2;

			if (ucs2Length > 0)
				memcpy(pDestText, pUCS2Text, ucs2Length);

			return ucs2Length;
		}
		else
		{
			*pCharType = MSG_ENCODE_GSM7BIT;
		}

		return gsm7bitLength;
	}
}


/**
return:
		bytelength of UTF8 text
*/
int MsgTextConvert::convertGSM7bitToUTF8(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, IN MSG_LANG_INFO_S *pLangInfo)
{
	int utf8Length = 0;
	int ucs2Length = 0;
	int maxUCS2Length = srcTextLen;		// max # of UCS2 chars, NOT bytes. when all gsm7 chars are only one byte(-there is no extenstion), UCS2Length is maxUCS2 Length. otherwise(ex: gsm7 char starts with 0x1b) UCS2Length must be less than gsm7 legnth

	WCHAR pUCS2Text[maxUCS2Length];
	memset(pUCS2Text, 0x00, maxUCS2Length * sizeof(WCHAR));

	MSG_DEBUG("srcTextLen = %d\n", srcTextLen);
	MSG_DEBUG("max dest Length = %d\n", maxLength);

	ucs2Length = convertGSM7bitToUCS2((unsigned char*)pUCS2Text, maxUCS2Length * sizeof(WCHAR), pSrcText, srcTextLen, pLangInfo);
	utf8Length = convertUCS2ToUTF8(pDestText, maxLength, (unsigned char*)pUCS2Text, ucs2Length);

	return utf8Length;
}


/**
args:
	OUT unsigned char *pDestText
	IN int maxLength		: max byte length of destination text
	IN const unsigned char *pSrcText
	IN  int srcTextLen		: byte length of UCS2 source text
return :
 		byte length of converted UTF8 characters
			-1 : The alpha isn't the gsm 7bit code
*/
int MsgTextConvert::convertUCS2ToUTF8(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN  int srcTextLen)
{
	int remainedBuffer = maxLength;
	int utf8Length;

#ifdef CONVERT_DUMP
	int srcLen = srcTextLen;
	const unsigned char * pTempSrcText = pSrcText;
#endif
	unsigned char * pTempDestText = pDestText;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0)
	{
		MSG_DEBUG("UCS2 to UTF8 Failed as text length is 0\n");
		return false;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open( "UTF8", "UCS-2BE" );

	if (cd > 0)
	{
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&srcTextLen, (char**)&pDestText, (gsize*)&remainedBuffer);
	}

	utf8Length = maxLength - remainedBuffer;
	pTempDestText[utf8Length] = 0x00;

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump UCS2 -> UTF8\n");
	convertDumpTextToHex((unsigned char*)pTempSrcText, srcLen);
	convertDumpTextToHex((unsigned char*)pTempDestText, utf8Length);
#endif

	g_iconv_close(cd);

	return utf8Length;
}


int MsgTextConvert::convertEUCKRToUTF8(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN  int srcTextLen)
{
	int remainedBuffer = maxLength;
	int utf8Length;

#ifdef CONVERT_DUMP
	int srcLen = srcTextLen;
	const unsigned char * pTempSrcText = pSrcText;
#endif
	unsigned char * pTempDestText = pDestText;

	if(srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0)
	{
		MSG_DEBUG("EUCKR to UTF8 Failed as text length is 0\n");
		return false;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open( "UTF8", "EUCKR" );

	if (cd > 0)
	{
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&srcTextLen, (char**)&pDestText, (gsize*)&remainedBuffer);
	}

	utf8Length = maxLength - remainedBuffer;
	pTempDestText[utf8Length] = 0x00;

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump EUCKR -> UTF8\n");
	convertDumpTextToHex((unsigned char*)pTempSrcText, srcLen);
	convertDumpTextToHex((unsigned char*)pTempDestText, utf8Length);
#endif

	g_iconv_close(cd);

	return utf8Length;
}


/**

args:
		unsigned char *pDestText
		int maxLength				: max destination buffer size
		const unsigned char *pSrcText
		int srcTextLen				: BYTE length of src text (UCS2)
return:
		bytelength of gsm7bit text
		-1 : converting error
*/
int MsgTextConvert::convertUCS2ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *pLangId, OUT bool *bIncludeAbnormalChar)
{
	// for UNICODE
	int outTextLen = 0;
	unsigned char lowerByte, upperByte;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0)
	{
		MSG_DEBUG("UCS2 to GSM7bit Failed as text length is 0\n");
		return -1;
	}

	std::map<unsigned short, unsigned char>::iterator itChar;
	std::map<unsigned short, unsigned char>::iterator itExt;

	MSG_CHAR_TYPE_T currType = MSG_CHAR_DEFAULT;
	MSG_CHAR_TYPE_T newType = MSG_CHAR_DEFAULT;

	unsigned short inText;

	// Get Language Type by checking each character
	for (int index = 0; index < srcTextLen; index++)
	{
		upperByte = pSrcText[index++];
		lowerByte = pSrcText[index];

		inText = (upperByte << 8) & 0xFF00;

		inText = inText | lowerByte;

//MSG_DEBUG("inText : [%04x]", inText);

		itExt = extCharList.find(inText);

		if (itExt != extCharList.end())
		{
			newType = (MSG_CHAR_TYPE_T)itExt->second;

			if (newType >= currType)
			{
				currType = newType;
			}
		}
	}

MSG_DEBUG("charType : [%d]", currType);

	for (int index = 0; index < srcTextLen; index++)
	{
		upperByte = pSrcText[index++];
		lowerByte = pSrcText[index];

		inText = (upperByte << 8) & 0xFF00;
		inText = inText | lowerByte;

MSG_DEBUG("inText : [%04x]", inText);

		// Check Default Char
		itChar = ucs2toGSM7DefList.find(inText);

		if (itChar != ucs2toGSM7DefList.end())
		{
MSG_DEBUG("default char");
			pDestText[outTextLen++] = (unsigned char)itChar->second;
		}
		else
		{
			if (currType == MSG_CHAR_GSM7EXT)
			{
				itExt = ucs2toGSM7ExtList.find(inText);

				if (itExt != ucs2toGSM7ExtList.end())
				{
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1)
					{
						MSG_DEBUG("Buffer Full");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				}
				else
				{
					pDestText[outTextLen++] = 0x20;
					*bIncludeAbnormalChar = true;
				}
			}
			else if (currType == MSG_CHAR_TURKISH)
			{
				*pLangId = MSG_LANG_ID_TURKISH;

				itExt = ucs2toTurkishList.find(inText);

				if (itExt != ucs2toTurkishList.end())
				{
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1)
					{
						MSG_DEBUG("Buffer Full");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				}
				else
				{
					pDestText[outTextLen++] = 0x20;
					*bIncludeAbnormalChar = true;
				}
			}
			else if (currType == MSG_CHAR_SPANISH)
			{
				*pLangId = MSG_LANG_ID_SPANISH;

				itExt = ucs2toSpanishList.find(inText);

				if (itExt != ucs2toSpanishList.end())
				{
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1)
					{
						MSG_DEBUG("Buffer Full");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				}
				else
				{
					pDestText[outTextLen++] = 0x20;
					*bIncludeAbnormalChar = true;
				}
			}
			else if (currType == MSG_CHAR_PORTUGUESE)
			{
				*pLangId = MSG_LANG_ID_PORTUGUESE;

				itExt = ucs2toPortuList.find(inText);

				if (itExt != ucs2toPortuList.end())
				{
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1)
					{
						MSG_DEBUG("Buffer Full");
						break;
					}

MSG_DEBUG("ucs2toPortuList : [%02x]", (unsigned char)itExt->second);

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				}
				else
				{
MSG_DEBUG("no char");
					pDestText[outTextLen++] = 0x20;
					*bIncludeAbnormalChar = true;
				}
			}
			else
			{
				pDestText[outTextLen++] = 0x20;
				*bIncludeAbnormalChar = true;
			}
		}

		// prevent buffer overflow
		if (maxLength <= outTextLen)
		{
			MSG_DEBUG("Buffer full\n");
			break;
		}
	}

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump UCS2 -> GSM7bit\n");
	convertDumpTextToHex((unsigned char*)pSrcText, srcTextLen);
	convertDumpTextToHex((unsigned char*)pDestText, outTextLen);
#endif

	return outTextLen;
}


int MsgTextConvert::convertUCS2ToGSM7bitAuto(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, OUT bool *pUnknown)
{
	// for UNICODE
	int outTextLen = 0;
	unsigned char lowerByte, upperByte;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0)
	{
		MSG_DEBUG("UCS2 to GSM7bit Failed as text length is 0\n");
		return -1;
	}

	std::map<unsigned short, unsigned char>::iterator itChar;
	std::map<unsigned short, unsigned char>::iterator itExt;

	unsigned short inText;

	for (int index = 0; index < srcTextLen; index++)
	{
		upperByte = pSrcText[index++];
		lowerByte = pSrcText[index];

		inText = (upperByte << 8) & 0xFF00;
		inText = inText | lowerByte;

//MSG_DEBUG("inText : [%04x]", inText);

		// Check Default Char
		itChar = ucs2toGSM7DefList.find(inText);

		if (itChar != ucs2toGSM7DefList.end())
		{
//MSG_DEBUG("default char");
			pDestText[outTextLen++] = (unsigned char)itChar->second;
		}
		else
		{
			itExt = ucs2toGSM7ExtList.find(inText);

			if (itExt != ucs2toGSM7ExtList.end())
			{
				// prevent buffer overflow
				if (maxLength <= outTextLen + 1)
				{
					MSG_DEBUG("Buffer Full");
					break;
				}

				pDestText[outTextLen++] = 0x1B;
				pDestText[outTextLen++] = (unsigned char)itExt->second;
			}
			else
			{
				*pUnknown = true;
				return 0;
			}
		}

		// prevent buffer overflow
		if (maxLength <= outTextLen)
		{
			MSG_DEBUG("Buffer full\n");
			break;
		}
	}

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump UCS2 -> GSM7bit\n");
	convertDumpTextToHex((unsigned char*)pSrcText, srcTextLen);
	convertDumpTextToHex((unsigned char*)pDestText, outTextLen);
#endif

	return outTextLen;
}


/**
 args :
		unsigned char *pDestText				: destination text (UCS2) - byte order depends on local byte order
		const unsigned char *pSrcText		: source text (gsm7bit)
		int maxLength			: max destination buffer size
		int srcTextLen			: byte length of source text (gsm7bit)
 return :
 		byte length of converted UCS2 characters
			-1 : The alpha isn't the gsm 7bit code
*/
int MsgTextConvert::convertGSM7bitToUCS2(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, IN MSG_LANG_INFO_S *pLangInfo)
{
	int outTextLen = 0;
	unsigned char lowerByte = 0, upperByte = 0;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0)
	{
		MSG_DEBUG("UCS2 to GSM7bit Failed as text length is 0\n");
		return -1;
	}

	for (int i = 0; i<srcTextLen; i++)
	{
		if (maxLength == 0)
		{
			break;
		}

		if (pSrcText[i] >= 0x80)
		{
			//error
			MSG_DEBUG(">>>>>>>a_pTextString[i]=%x, The alpha isn't the gsm 7bit code, Never Come here!!!\n", pSrcText[i]);
			return -1;
		}

		if (pLangInfo->bLockingShift == true) // National Language Locking Shift
		{
			MSG_DEBUG("Locking Shift [%d]", pLangInfo->lockingLang);

			if (pLangInfo->lockingLang == MSG_LANG_ID_TURKISH)
			{
				// Check Escape
				if (g_TurkishLockingToUCS2[pSrcText[i]] == 0x001B)
				{
					i++;

					if (pLangInfo->bSingleShift == true) // National Language Single Shift
					{
						MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

						if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH)
						{
							lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH)
						{
							lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE)
						{
							lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else
						{
							lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
					}
					else // GSM 7 bit Default Alphabet Extension Table
					{
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				}
				else // TURKISH - National Language Locking Shift
				{
					lowerByte = g_TurkishLockingToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_TurkishLockingToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			}
			else if (pLangInfo->lockingLang == MSG_LANG_ID_PORTUGUESE)
			{
				// Check Escape
				if (g_PortuLockingToUCS2[pSrcText[i]] == 0x001B)
				{
					i++;

					if (pLangInfo->bSingleShift == true) // National Language Single Shift
					{
						MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

						if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH)
						{
							lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH)
						{
							lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE)
						{
							lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
						else
						{
							lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
					}
					else // GSM 7 bit Default Alphabet Extension Table
					{
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				}
				else // PORTUGUESE - National Language Locking Shift
				{
					lowerByte = g_PortuLockingToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_PortuLockingToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			}
		}
		else
		{
			// Check Escape
			if (g_GSM7BitToUCS2[pSrcText[i]] == 0x001B)
			{
				i++;

				if (pLangInfo->bSingleShift == true) // National Language Single Shift
				{
					MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

					if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH)
					{
						lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
					else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH)
					{
						lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
					else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE)
					{
						lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
					else
					{
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				}
				else // GSM 7 bit Default Alphabet Extension Table
				{
					lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			}
			else
			{
				lowerByte = g_GSM7BitToUCS2[pSrcText[i]] & 0x00FF;
				upperByte = (g_GSM7BitToUCS2[pSrcText[i]] & 0xFF00) >> 8;
	 		}
		}

		pDestText[outTextLen++] = upperByte;
		pDestText[outTextLen++] = lowerByte;
		maxLength -= 2;
	}

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump GSM7bit -> UCS2\n");
	convertDumpTextToHex((unsigned char*)pSrcText, srcTextLen);
	convertDumpTextToHex((unsigned char*)pDestText, outTextLen);
#endif

	return outTextLen;
}


void MsgTextConvert::convertDumpTextToHex(const unsigned char *pText, int length)
{
	printf("\n=======================================\n");
	printf("   Dump Text To Hex - Length :%d\n", length);
	printf("=======================================");

	for (int i = 0; i < length; i++ )
	{
		if ( i % MAX_DUMP_COLUMN == 0 )
		{
			printf("\n\t");
		}
		printf("%02x ", pText[i]);
	}

	printf("\n=======================================\n\n");
}
