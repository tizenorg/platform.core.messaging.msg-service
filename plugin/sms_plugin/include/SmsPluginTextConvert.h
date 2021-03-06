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

#ifndef SMS_PLUGIN_TEXT_CONVERT_H
#define SMS_PLUGIN_TEXT_CONVERT_H


/*==================================================================================================
				INCLUDE FILES
==================================================================================================*/
#include <map>

#include "SmsPluginTypes.h"


/*==================================================================================================
                                DEFINES
==================================================================================================*/
#define IN
#define OUT
#define INOUT

#define MAX_DUMP_COLUMN	16

typedef unsigned short WCHAR;

typedef unsigned char SMS_CHAR_TYPE_T;


/*==================================================================================================
                                ENUMS
==================================================================================================*/
enum _SMS_CHAR_TYPE_E
{
	SMS_CHAR_DEFAULT = 0,
	SMS_CHAR_GSM7EXT,
	SMS_CHAR_TURKISH,
	SMS_CHAR_SPANISH,
	SMS_CHAR_PORTUGUESE
};


/*==================================================================================================
                                     STRUCTURES
==================================================================================================*/
// ETSI GSM 03.38 GSM 7 bit Default Alphabet Table -> UCS2
static const WCHAR g_GSM7BitToUCS2[] =
{
	/* @ */
	0x0040, 0x00A3, 0x0024, 0x00A5, 0x00E8, 0x00E9, 0x00F9, 0x00EC, 0x00F2, 0x00C7, 0x000A, 0x00D8, 0x00F8, 0x000D, 0x00C5, 0x00E5,
	/* ��*/
	0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9, 0x03A0, 	0x03A8, 0x03A3, 0x0398, 0x039E, 0x001B, 0x00C6, 0x00E6, 0x00DF, 0x00C9,
	/* SP */
	0x0020, 0x0021, 0x0022, 0x0023, 0x00A4, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	/* 0 */
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	/* ��*/
	0x00A1, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	/* P */
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00C4, 0x00D6, 0x00D1, 0x00DC, 0x00A7,
	/* ��*/
	0x00BF, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	/* p */
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00E4, 0x00F6, 0x00F1, 0x00FC, 0x00E0
};


//GSM 7 bit Default Alphabet Extension Table -> UCS2
static const WCHAR g_GSM7BitExtToUCS2[] =
{
	// 0x0020 -> (SP) for invalid code
																     /* Page Break */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x000C, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
							  /* ^ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x005E, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x001B, 0x0020, 0x0020, 0x0020, 0x0020,
													    /* { */ /* } */                                                               /* \ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x007B, 0x007D, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005C,
													                                                      /* [ */ /* ~ */ /* ] */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005B, 0x007E, 0x005D, 0x0020,
	/* | */
	0x007C, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
								      /* ��*/
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x20AC, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020
};


//Turkish National Language Single Shift Table -> UCS2
static const WCHAR g_TurkishSingleToUCS2[] =
{
	// 0x0020 -> (SP) for invalid code
																     /* Page Break */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x000C, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
							  /* ^ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x005E, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x001B, 0x0020, 0x0020, 0x0020, 0x0020,
													    /* { */ /* } */                                                               /* \ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x007B, 0x007D, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005C,
													                                                      /* [ */ /* ~ */ /* ] */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005B, 0x007E, 0x005D, 0x0020,
	/* | */
	0x007C, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x011E, 0x0020, 0x0130, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
	0x0020, 0x0020, 0x0020, 0x015E, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
								      /* ��*/
	0x0020, 0x0020, 0x0020, 0x00E7, 0x0020, 0x20AC, 0x0020, 0x011F, 0x0020, 0x0131, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
	0x0020, 0x0020, 0x0020, 0x015F, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020
};


// Turkish National Language Locking Shift Table -> UCS2
static const WCHAR g_TurkishLockingToUCS2[] =
{
	/* @ */
	0x0040, 0x00A3, 0x0024, 0x00A5, 0x20AC, 0x00E9, 0x00F9, 0x00EC, 0x00F2, 0x00C7, 0x000A, 0x011E, 0x011F, 0x000D, 0x00C5, 0x00E5,
	/* ��*/
	0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9, 0x03A0, 	0x03A8, 0x03A3, 0x0398, 0x039E, 0x001B, 0x015E, 0x015F, 0x00DF, 0x00C9,
	/* SP */
	0x0020, 0x0021, 0x0022, 0x0023, 0x00A4, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	/* 0 */
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	/* ��*/
	0x0130, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	/* P */
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00C4, 0x00D6, 0x00D1, 0x00DC, 0x00A7,
	/* c */
	0x00E7, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	/* p */
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00E4, 0x00F6, 0x00F1, 0x00FC, 0x00E0
};


// Spanish National Language Single Shift Table -> UCS2
static const WCHAR g_SpanishSingleToUCS2[] =
{
	// 0x0020 -> (SP) for invalid code
																     /* Page Break */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00E7, 0x000C, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
							  /* ^ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x005E, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x001B, 0x0020, 0x0020, 0x0020, 0x0020,
													    /* { */ /* } */                                                               /* \ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x007B, 0x007D, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005C,
													                                                      /* [ */ /* ~ */ /* ] */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005B, 0x007E, 0x005D, 0x0020,
	/* | */
	0x007C, 0x00C1, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00CD, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00D3,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00DA, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
								      /* ��*/
	0x0020, 0x00E1, 0x0020, 0x0020, 0x0020, 0x20AC, 0x0020, 0x0020, 0x0020, 0x00ED, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00F3,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00FA, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020
};


// Portuguese National Language Single Shift Table -> UCS2
static const WCHAR g_PortuSingleToUCS2[] =
{
	// 0x0020 -> (SP) for invalid code
																     /* Page Break */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00EA, 0x0020, 0x0020, 0x0020, 0x00E7, 0x000C, 0x00D4, 0x00F4, 0x0020, 0x00C1, 0x00E1,
							  /* ^ */
	0x0020, 0x0020, 0x03A6, 0x0393, 0x005E, 0x03A9, 0x03A0, 0x03A8, 0x03A3, 0x0398, 0x0020, 0x001B, 0x0020, 0x0020, 0x0020, 0x00CA,
													    /* { */ /* } */                                                               /* \ */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x007B, 0x007D, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005C,
													                                                      /* [ */ /* ~ */ /* ] */
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x005B, 0x007E, 0x005D, 0x0020,
	/* | */
	0x007C, 0x00C0, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00CD, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00D3,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00DA, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00C3, 0x00D5, 0x0020, 0x0020, 0x0020,
								      /* ��*/
	0x0020, 0x00C2, 0x0020, 0x0020, 0x0020, 0x20AC, 0x0020, 0x0020, 0x0020, 0x00ED, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00F3,
	0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00FA, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x00E3, 0x00F5, 0x0020, 0x0020, 0x00E2
};


// Portuguese National Language Locking Shift Table -> UCS2
static const WCHAR g_PortuLockingToUCS2[] =
{
	/* @ */
	0x0040, 0x00A3, 0x0024, 0x00A5, 0x00EA, 0x00E9, 0x00FA, 0x00ED, 0x00F3, 0x00E7, 0x000A, 0x00D4, 0x00F4, 0x000D, 0x00C1, 0x00E1,
	/* ��*/
	0x0394, 0x005F, 0x0020, 0x00C7, 0x00C0, 0x0020, 0x005E, 	0x005C, 0x20AC, 0x00D3, 0x007C, 0x001B, 0x00C2, 0x00E2, 0x00CA, 0x00C9,
	/* SP */
	0x0020, 0x0021, 0x0022, 0x0023, 0x00A4, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	/* 0 */
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	/* ��*/
	0x00CD, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	/* P */
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00C3, 0x00D5, 0x00DA, 0x00DC, 0x00A7,
	/* ��*/
	0x00BF, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	/* p */
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00E3, 0x00F5, 0x0020, 0x00FC, 0x00E0
};


/*==================================================================================================
                                CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginTextConvert
{
public:
	static SmsPluginTextConvert* instance();

	int convertUTF8ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT SMS_LANGUAGE_ID_T *pLangId);
	int convertUTF8ToUCS2(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen);
	int convertUTF8ToAuto(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT msg_encode_type_t *pCharType);

	int convertGSM7bitToUTF8(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, IN SMS_LANG_INFO_S *pLangInfo);
	int convertUCS2ToUTF8(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN  int srcTextLen);
	int convertEUCKRToUTF8(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN  int srcTextLen);

private:
	SmsPluginTextConvert();
	virtual ~SmsPluginTextConvert();

	static SmsPluginTextConvert* pInstance;

	int convertUCS2ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, OUT SMS_LANGUAGE_ID_T *pLangId);
	int convertUCS2ToGSM7bitAuto(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, OUT bool *pUnknown);

	int convertGSM7bitToUCS2(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, IN SMS_LANG_INFO_S *pLangInfo);

	void convertDumpTextToHex(const unsigned char *pText, int length);

	std::map<unsigned short, unsigned char> extCharList;
	std::map<unsigned short, unsigned char> ucs2toGSM7DefList;
	std::map<unsigned short, unsigned char> ucs2toGSM7ExtList;
	std::map<unsigned short, unsigned char> ucs2toTurkishList;
	std::map<unsigned short, unsigned char> ucs2toSpanishList;
	std::map<unsigned short, unsigned char> ucs2toPortuList;
};

#endif //SMS_PLUGIN_TEXT_CONVERT_H

