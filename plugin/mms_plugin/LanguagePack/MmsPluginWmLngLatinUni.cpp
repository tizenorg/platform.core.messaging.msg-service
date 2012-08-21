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
#include "MmsPluginMessage.h"
#include <stdlib.h>


#define		MAX_ASCII			256
#define		MAX_ASCII_NUM		255

static void	 __WmLngReverseKeyCode(UCHAR *pOutStr, MCHAR *pInputStr, UINT len);
static UINT	 __WmLngLatin12UnicodeN(MCHAR *pOutStr, UCHAR *pInputStr, UINT len);
static int	 __WmLngConvertLatin12Unicode (MCHAR *des, UCHAR *str, UINT len);

#define tabLatin2UniMax 93
#define tabLatin3UniMax 87
#define tabLatin4UniMax 96
#define tabLatin8UniMax 60
#define tabLatin15UniMax 96
#define tabLatin5UniMax 96
#define tabWin1251UniMax 128
#define tabKoi8rUniMax 128
#define tabKoi8uUniMax 128

#define	SIM_SMSP_ALPHA_ID_MAX_LEN 30
#define	SMS_MCHAR_TO_SIM_ALPHA_MASK 0x80
#define	SMS_SIM_ALPHA_TO_MCHAR_MASK 0x7F


/* latin2 table */
typedef struct {
	UCHAR latin2;
	MCHAR unicode;
}Latin2UniTable;

const Latin2UniTable tabLatin2Uni[] = {
	{0xA0,0x0020},{0xA1,0x0104},{0xA3,0x0141},{0xA4,0x00A4},{0xA5,0x013D},
	{0xA6,0x015A},{0xA7,0x00A7},{0xA8,0x00A8},{0xA9,0x0160},{0xAA,0x015E},
	{0xAB,0x0164},{0xAC,0x0179},{0xAD,0x00AD},{0xAE,0x017D},{0xAF,0x017B},
	{0xB0,0x00B0},{0xB1,0x0105},{0xB2,0x00B8},{0xB3,0x0142},{0xB4,0x00B4},
	{0xB5,0x012D},{0xB7,0x015B},{0xB8,0x00B8},{0xB9,0x0161},{0xBA,0x015F},
	{0xBB,0x0165},{0xBC,0x017A},{0xBE,0x017E},{0xBF,0x017C},{0xC0,0x0154},
	{0xC1,0x00C1},{0xC2,0x00C2},{0xC3,0x0102},{0xC4,0x00C4},{0xC5,0x0139},
	{0xC6,0x0106},{0xC7,0x00C7},{0xC8,0x010C},{0xC9,0x00C9},{0xCA,0x0118},
	{0xCB,0x00CB},{0xCC,0x0114},{0xCD,0x00CD},{0xCE,0x00CE},{0xCF,0x010E},
	{0xD0,0x00D0},{0xD1,0x0143},{0xD2,0x0147},{0xD3,0x00D3},{0xD4,0x00D4},
	{0xD5,0x0150},{0xD6,0x00D6},{0xD7,0x00D7},{0xD8,0x0158},{0xD9,0x016E},
	{0xDA,0x00DA},{0xDB,0x0170},{0xDC,0x00DC},{0xDD,0x00DD},{0xDE,0x0162},
	{0xDF,0x00DF},{0xE0,0x0155},{0xE1,0x00E1},{0xE2,0x00E2},{0xE3,0x0103},
	{0xE4,0x00E4},{0xE5,0x013A},{0xE6,0x0107},{0xE7,0x00E7},{0xE8,0x010D},
	{0xE9,0x00E9},{0xEA,0x0119},{0xEB,0x00EB},{0xEC,0x011B},{0xED,0x00ED},
	{0xEE,0x00EE},{0xEF,0x010F},{0xF0,0x0111},{0xF1,0x0144},{0xF2,0x0148},
	{0xF3,0x00F3},{0xF4,0x00F4},{0xF5,0x0151},{0xF6,0x00F6},{0xF7,0x00F7},
	{0xF8,0x0159},{0xF9,0x016F},{0xFA,0x00FA},{0xFB,0x0171},{0xFC,0x00FC},
	{0xFD,0x00FD},{0xFE,0x0163},{0xFF,0x00B7},
};

/* latin3 table */
typedef struct {
	UCHAR latin3;
	MCHAR unicode;
}Latin3UniTable;

const Latin3UniTable tabLatin3Uni[] = {
	{0xA0,0x0020},{0xA1,0x0126},{0xA2,0x02D8},{0xA3,0x00A3},{0xA4,0x00A4},
	{0xA6,0x0124},{0xA7,0x00A7},{0xA8,0x00A8},{0xA9,0x0130},{0xAA,0x015E},
	{0xAB,0x011E},{0xAC,0x0134},{0xAD,0x00AD},{0xAF,0x017B},{0xB0,0x00B0},
	{0xB1,0x0127},{0xB2,0x00B2},{0xB3,0x00B3},{0xB4,0x00B4},{0xB5,0x00B5},
	{0xB6,0x0125},{0xB7,0x00B7},{0xB8,0x00B8},{0xB9,0x0131},{0xBA,0x015F},
	{0xBB,0x011F},{0xBC,0x0135},{0xBD,0x00BD},{0xBF,0x017C},{0xC0,0x00C0},
	{0xC1,0x00C1},{0xC2,0x00C2},{0xC4,0x00C4},{0xC5,0x010A},{0xC6,0x0108},
	{0xC7,0x00C7},{0xC8,0x00C8},{0xC9,0x00C9},{0xCA,0x00CA},{0xCB,0x00CB},
	{0xCC,0x00CC},{0xCD,0x00CD},{0xCE,0x00CE},{0xCF,0x00CF},{0xD1,0x00D1},
	{0xD2,0x00D2},{0xD3,0x00D3},{0xD4,0x00D4},{0xD6,0x00D6},{0xD7,0x00D7},
	{0xD8,0x011C},{0xD9,0x00D9},{0xDA,0x00DA},{0xDB,0x00DB},{0xDC,0x00DC},
	{0xDE,0x015C},{0xDF,0x00DF},{0xE0,0x00E0},{0xE1,0x00E1},{0xE2,0x00E2},
	{0xE4,0x00E4},{0xE5,0x010B},{0xE6,0x0109},{0xE7,0x00E7},{0xE8,0x00E8},
	{0xE9,0x00E9},{0xEA,0x00EA},{0xEB,0x00EB},{0xEC,0x00EC},{0xED,0x00ED},
	{0xEE,0x00EE},{0xEF,0x00EF},{0xF1,0x00F1},{0xF2,0x00F2},{0xF3,0x00F3},
	{0xF4,0x00F4},{0xF5,0x0121},{0xF6,0x00F6},{0xF7,0x00F7},{0xF8,0x011D},
	{0xF9,0x00F9},{0xFA,0x00FA},{0xFB,0x00FB},{0xFC,0x00FC},{0xFD,0x016D},
	{0xFE,0x015D},{0xFF,0x02D9},
};
/* latin4 table */
typedef struct {
	UCHAR latin4;
	MCHAR unicode;
}Latin4UniTable;

const Latin4UniTable tabLatin4Uni[] = {
	{0xA0,0x0020},{0xA1,0x0104},{0xA2,0x0138},{0xA3,0x0156},{0xA4,0x00A4},
	{0xA5,0x0128},{0xA6,0x013B},{0xA7,0x00A7},{0xA8,0x00A8},{0xA9,0x0160},
	{0xAA,0x0112},{0xAB,0x0122},{0xAC,0x0166},{0xAD,0x00AD},{0xAE,0x017D},
	{0xAF,0x00AF},{0xB0,0x00B0},{0xB1,0x0105},{0xB2,0x02DB},{0xB3,0x0157},
	{0xB4,0x00B4},{0xB5,0x0129},{0xB6,0x013C},{0xB7,0x02C7},{0xB8,0x00B8},
	{0xB9,0x0161},{0xBA,0x0113},{0xBB,0x0123},{0xBC,0x0167},{0xBD,0x014A},
	{0xBE,0x017E},{0xBF,0x014B},{0xC0,0x0100},{0xC1,0x00C1},{0xC2,0x00C2},
	{0xC3,0x00C3},{0xC4,0x00C4},{0xC5,0x00C5},{0xC6,0x00C6},{0xC7,0x012E},
	{0xC8,0x010C},{0xC9,0x00C9},{0xCA,0x0118},{0xCB,0x00CB},{0xCC,0x0116},
	{0xCD,0x00CD},{0xCE,0x00CE},{0xCF,0x012A},{0xD0,0x0110},{0xD1,0x0145},
	{0xD2,0x014C},{0xD3,0x0136},{0xD4,0x00D4},{0xD5,0x00D5},{0xD6,0x00D6},
	{0xD7,0x00D7},{0xD8,0x00D8},{0xD9,0x0172},{0xDA,0x00DA},{0xDB,0x00DB},
	{0xDC,0x00DC},{0xDD,0x0168},{0xDE,0x016A},{0xDF,0x00DF},{0xE0,0x0101},
	{0xE1,0x00E1},{0xE2,0x00E2},{0xE3,0x00E3},{0xE4,0x00E4},{0xE5,0x00E5},
	{0xE6,0x00E6},{0xE7,0x012F},{0xE8,0x010D},{0xE9,0x00E9},{0xEA,0x0119},
	{0xEB,0x00EB},{0xEC,0x0117},{0xED,0x00ED},{0xEE,0x00EE},{0xEF,0x012B},
	{0xF0,0x0111},{0xF1,0x0146},{0xF2,0x014D},{0xF3,0x0137},{0xF4,0x00F4},
	{0xF5,0x00F5},{0xF6,0x00F6},{0xF7,0x00F7},{0xF8,0x00F8},{0xF9,0x0173},
	{0xFA,0x00FA},{0xFB,0x00FB},{0xFC,0x00FC},{0xFD,0x0169},{0xFE,0x016B},
	{0xFF,0x02D9},

};
/* latin8 table */
typedef struct {
	UCHAR latin8;
	MCHAR unicode;
}Latin8UniTable;

const Latin8UniTable tabLatin8Uni[] = {
	{0xA0,0x0020},{0xA2,0x00A2},{0xA3,0x00A3},{0xA4,0x00A4},{0xA5,0x00A5},
	{0xA6,0x00A6},{0xA7,0x00A7},{0xA8,0x00A8},{0xA9,0x00A9},{0xAA,0x00D7},
	{0xAB,0x00AB},{0xAC,0x00AC},{0xAD,0x00AD},{0xAE,0x00AE},{0xAF,0x203E},
	{0xB0,0x00B0},{0xB1,0x00B1},{0xB2,0x00B2},{0xB3,0x00B3},{0xB4,0x00B4},
	{0xB5,0x00B5},{0xB6,0x00B6},{0xB7,0x00B7},{0xB8,0x00B8},{0xB9,0x00B9},
	{0xBA,0x00F7},{0xBB,0x00BB},{0xBC,0x00BC},{0xBD,0x00BD},{0xBE,0x00BE},
	{0xDF,0x2017},{0xE0,0x05D0},{0xE1,0x05D1},{0xE2,0x05D2},{0xE3,0x05D3},
	{0xE4,0x05D4},{0xE5,0x05D5},{0xE6,0x05D6},{0xE7,0x05D7},{0xE8,0x05D8},
	{0xE9,0x05D9},{0xEA,0x05DA},{0xEB,0x05DB},{0xEC,0x05DC},{0xED,0x05DD},
	{0xEE,0x05DE},{0xEF,0x05DF},{0xF0,0x05E0},{0xF1,0x05E1},{0xF2,0x05E2},
	{0xF3,0x05E3},{0xF4,0x05E4},{0xF5,0x05E5},{0xF6,0x05E6},{0xF7,0x05E7},
	{0xF8,0x05E8},{0xF9,0x05E9},{0xFA,0x05EA},{0xFD,0x200E},{0xFE,0x200F},
};

/* latin15 table */
typedef struct {
	UCHAR latin15;
	MCHAR unicode;
}Latin15UniTable;

const Latin15UniTable tabLatin15Uni[] = {
	{0xA0,0x0020},{0xA1,0x00A1},{0xA2,0x00A2},{0xA3,0x00A3},{0xA4,0x20AC},
	{0xA5,0x00A5},{0xA6,0x0160},{0xA7,0x00A7},{0xA8,0x0161},{0xA9,0x00A9},
	{0xAA,0x00AA},{0xAB,0x00AB},{0xAC,0x00AC},{0xAD,0x00AD},{0xAE,0x00AE},
	{0xAF,0x00AF},{0xB0,0x00B0},{0xB1,0x00B1},{0xB2,0x00B2},{0xB3,0x00B3},
	{0xB4,0x017D},{0xB5,0x00B5},{0xB6,0x00B6},{0xB7,0x00B7},{0xB8,0x017E},
	{0xB9,0x00B9},{0xBA,0x00BA},{0xBB,0x00BB},{0xBC,0x0152},{0xBD,0x0153},
	{0xBE,0x0178},{0xBF,0x00BF},{0xC0,0x00C0},{0xC1,0x00C1},{0xC2,0x00C2},
	{0xC3,0x00C3},{0xC4,0x00C4},{0xC5,0x00C5},{0xC6,0x00C6},{0xC7,0x00C7},
	{0xC8,0x00C8},{0xC9,0x00C9},{0xCA,0x00CA},{0xCB,0x00CB},{0xCC,0x00CC},
	{0xCD,0x00CD},{0xCE,0x00CE},{0xCF,0x00CF},{0xD0,0x00D0},{0xD1,0x00D1},
	{0xD2,0x00D2},{0xD3,0x00D3},{0xD4,0x00D4},{0xD5,0x00D5},{0xD6,0x00D6},
	{0xD7,0x00D7},{0xD8,0x00D8},{0xD9,0x00D9},{0xDA,0x00DA},{0xDB,0x00DB},
	{0xDC,0x00DC},{0xDD,0x00DD},{0xDE,0x00DE},{0xDF,0x00DF},{0xE0,0x00E0},
	{0xE1,0x00E1},{0xE2,0x00E2},{0xE3,0x00E3},{0xE4,0x00E4},{0xE5,0x00E5},
	{0xE6,0x00E6},{0xE7,0x00E7},{0xE8,0x00E8},{0xE9,0x00E9},{0xEA,0x00EA},
	{0xEB,0x00EB},{0xEC,0x00EC},{0xED,0x00ED},{0xEE,0x00EE},{0xEF,0x00EF},
	{0xF0,0x00F0},{0xF1,0x00F1},{0xF2,0x00F2},{0xF3,0x00F3},{0xF4,0x00F4},
	{0xF5,0x00F5},{0xF6,0x00F6},{0xF7,0x00F7},{0xF8,0x00F8},{0xF9,0x00F9},
	{0xFA,0x00FA},{0xFB,0x00FB},{0xFC,0x00FC},{0xFD,0x00FD},{0xFE,0x00FE},
	{0xFF,0x00FF},
};


/* Win1251 table */
typedef struct {
	UCHAR win1251;
	MCHAR unicode;
}Win1251UniTable;
const Win1251UniTable tabWin1251Uni[] = {
	{0x80,0x0402},{0x81,0x0403},{0x82,0x201A},{0x83,0x0453},{0x84,0x201E},
	{0x85,0x2026},{0x86,0x2020},{0x87,0x2021},{0x88,0x20AC},{0x89,0x2030},
	{0x8A,0x0409},{0x8B,0x2039},{0x8C,0x040A},{0x8D,0x040C},{0x8E,0x040B},
	{0x8F,0x040F},{0x90,0x0452},{0x91,0x2018},{0x92,0x2019},{0x93,0x201C},
	{0x94,0x201D},{0x95,0x2022},{0x96,0x2013},{0x97,0x2014},{0x98,0x0000},
	{0x99,0x2122},{0x9A,0x0459},{0x9B,0x203A},{0x9C,0x045A},{0x9D,0x045C},
	{0x9E,0x045B},{0x9F,0x045F},{0xA0,0x00a0},{0xA1,0x040e},{0xA2,0x045e},
	{0xA3,0x0408},{0xA4,0x00a4},{0xA5,0x0490},{0xA6,0x00a6},{0xA7,0x00a7},
	{0xA8,0x0401},{0xA9,0x00a9},{0xAA,0x0404},{0xAB,0x00ab},{0xAC,0x00ac},
	{0xAD,0x00ad},{0xAE,0x00ae},{0xAF,0x0407},{0xB0,0x00b0},{0xB1,0x00b1},
	{0xB2,0x0406},{0xB3,0x0456},{0xB4,0x0491},{0xB5,0x00b5},{0xB6,0x00b6},
	{0xB7,0x00b7},{0xB8,0x0451},{0xB9,0x2116},{0xBA,0x0454},{0xBB,0x00bb},
	{0xBC,0x0458},{0xBD,0x0405},{0xBE,0x0455},{0xBF,0x0457},{0xC0,0x0410},
	{0xC1,0x0411},{0xC2,0x0412},{0xC3,0x0413},{0xC4,0x0414},{0xC5,0x0415},
	{0xC6,0x0416},{0xC7,0x0417},{0xC8,0x0418},{0xC9,0x0419},{0xCA,0x041a},
	{0xCB,0x041b},{0xCC,0x041c},{0xCD,0x041d},{0xCE,0x041e},{0xCF,0x041f},
	{0xD0,0x0420},{0xD1,0x0421},{0xD2,0x0422},{0xD3,0x0423},{0xD4,0x0424},
	{0xD5,0x0425},{0xD6,0x0426},{0xD7,0x0427},{0xD8,0x0428},{0xD9,0x0429},
	{0xDA,0x042a},{0xDB,0x042b},{0xDC,0x042c},{0xDD,0x042d},{0xDE,0x042e},
	{0xDF,0x042f},{0xE0,0x0430},{0xE1,0x0431},{0xE2,0x0432},{0xE3,0x0433},
	{0xE4,0x0434},{0xE5,0x0435},{0xE6,0x0436},{0xE7,0x0437},{0xE8,0x0438},
	{0xE9,0x0439},{0xEA,0x043a},{0xEB,0x043b},{0xEC,0x043c},{0xED,0x043d},
	{0xEE,0x043e},{0xEF,0x043f},{0xF0,0x0440},{0xF1,0x0441},{0xF2,0x0442},
	{0xF3,0x0443},{0xF4,0x0444},{0xF5,0x0445},{0xF6,0x0446},{0xF7,0x0447},
	{0xF8,0x0448},{0xF9,0x0449},{0xFA,0x044a},{0xFB,0x044b},{0xFC,0x044c},
	{0xFD,0x044d},{0xFE,0x044e},{0xFF,0x044f}
};


/* Koi8-r table */
typedef struct {
	UCHAR koi8r;
	MCHAR unicode;
}Koi8rUniTable;
const Koi8rUniTable tabKoi8rUni[] = {
	{0x80,0x2500},{0x81,0x2502},{0x82,0x250C},{0x83,0x2510},{0x84,0x2514},
	{0x85,0x2518},{0x86,0x251C},{0x87,0x2524},{0x88,0x252C},{0x89,0x2534},
	{0x8A,0x253C},{0x8B,0x2580},{0x8C,0x2584},{0x8D,0x2588},{0x8E,0x258C},
	{0x8F,0x2590},{0x90,0x2591},{0x91,0x2592},{0x92,0x2593},{0x93,0x2320},
	{0x94,0x25A0},{0x95,0x2219},{0x96,0x221A},{0x97,0x2248},{0x98,0x2264},
	{0x99,0x2265},{0x9A,0x00A0},{0x9B,0x2321},{0x9C,0x00B0},{0x9D,0x00B2},
	{0x9E,0x00B7},{0x9F,0x00F7},{0xA0,0x2550},{0xA1,0x2551},{0xA2,0x2552},
	{0xA3,0x0451},{0xA4,0x2553},{0xA5,0x2554},{0xA6,0x2555},{0xA7,0x2556},
	{0xA8,0x2557},{0xA9,0x2558},{0xAA,0x2559},{0xAB,0x255A},{0xAC,0x255B},
	{0xAD,0x255C},{0xAE,0x255D},{0xAF,0x255E},{0xB0,0x255F},{0xB1,0x2560},
	{0xB2,0x2561},{0xB3,0x0401},{0xB4,0x2562},{0xB5,0x2563},{0xB6,0x2564},
	{0xB7,0x2565},{0xB8,0x2566},{0xB9,0x2567},{0xBA,0x2568},{0xBB,0x2569},
	{0xBC,0x256A},{0xBD,0x256B},{0xBE,0x256C},{0xBF,0x00A9},{0xC0,0x044E},
	{0xC1,0x0430},{0xC2,0x0431},{0xC3,0x0446},{0xC4,0x0434},{0xC5,0x0435},
	{0xC6,0x0444},{0xC7,0x0433},{0xC8,0x0445},{0xC9,0x0438},{0xCA,0x0439},
	{0xCB,0x043A},{0xCC,0x043B},{0xCD,0x043C},{0xCE,0x043D},{0xCF,0x043E},
	{0xD0,0x043F},{0xD1,0x044F},{0xD2,0x0440},{0xD3,0x0441},{0xD4,0x0442},
	{0xD5,0x0443},{0xD6,0x0436},{0xD7,0x0432},{0xD8,0x044C},{0xD9,0x044B},
	{0xDA,0x0437},{0xDB,0x0448},{0xDC,0x044D},{0xDD,0x0449},{0xDE,0x0447},
	{0xDF,0x044A},{0xE0,0x042E},{0xE1,0x0410},{0xE2,0x0411},{0xE3,0x0426},
	{0xE4,0x0414},{0xE5,0x0415},{0xE6,0x0424},{0xE7,0x0413},{0xE8,0x0425},
	{0xE9,0x0418},{0xEA,0x0419},{0xEB,0x041A},{0xEC,0x041B},{0xED,0x041C},
	{0xEE,0x041D},{0xEF,0x041E},{0xF0,0x041F},{0xF1,0x042F},{0xF2,0x0420},
	{0xF3,0x0421},{0xF4,0x0422},{0xF5,0x0423},{0xF6,0x0416},{0xF7,0x0412},
	{0xF8,0x042C},{0xF9,0x042B},{0xFA,0x0417},{0xFB,0x0428},{0xFC,0x042D},
	{0xFD,0x0429},{0xFE,0x0427},{0xFF,0x042A}
};



/* Koi8-u table */
typedef struct {
	UCHAR koi8u;
	MCHAR unicode;
}Koi8uUniTable;

const Koi8uUniTable tabKoi8uUni[] = {
	{0x80,0x2500},{0x81,0x2502},{0x82,0x250C},{0x83,0x2510},{0x84,0x2514},
	{0x85,0x2518},{0x86,0x251C},{0x87,0x2524},{0x88,0x252C},{0x89,0x2534},
	{0x8A,0x253C},{0x8B,0x2580},{0x8C,0x2584},{0x8D,0x2588},{0x8E,0x258C},
	{0x8F,0x2590},{0x90,0x2591},{0x91,0x2592},{0x92,0x2593},{0x93,0x2320},
	{0x94,0x25A0},{0x95,0x2219},{0x96,0x221A},{0x97,0x2248},{0x98,0x2264},
	{0x99,0x2265},{0x9A,0x00A0},{0x9B,0x2321},{0x9C,0x00B0},{0x9D,0x00B2},
	{0x9E,0x00B7},{0x9F,0x00F7},{0xA0,0x2550},{0xA1,0x2551},{0xA2,0x2552},
	{0xA3,0x0451},{0xA4,0x0454},{0xA5,0x2554},{0xA6,0x0456},{0xA7,0x0457},
	{0xA8,0x2557},{0xA9,0x2558},{0xAA,0x2559},{0xAB,0x255A},{0xAC,0x255B},
	{0xAD,0x0491},{0xAE,0x255D},{0xAF,0x255E},{0xB0,0x255F},{0xB1,0x2560},
	{0xB2,0x2561},{0xB3,0x0401},{0xB4,0x0404},{0xB5,0x2563},{0xB6,0x0406},
	{0xB7,0x0407},{0xB8,0x2566},{0xB9,0x2567},{0xBA,0x2568},{0xBB,0x2569},
	{0xBC,0x256A},{0xBD,0x0490},{0xBE,0x256C},{0xBF,0x00A9},{0xC0,0x044E},
	{0xC1,0x0430},{0xC2,0x0431},{0xC3,0x0446},{0xC4,0x0434},{0xC5,0x0435},
	{0xC6,0x0444},{0xC7,0x0433},{0xC8,0x0445},{0xC9,0x0438},{0xCA,0x0439},
	{0xCB,0x043A},{0xCC,0x043B},{0xCD,0x043C},{0xCE,0x043D},{0xCF,0x043E},
	{0xD0,0x043F},{0xD1,0x044F},{0xD2,0x0440},{0xD3,0x0441},{0xD4,0x0442},
	{0xD5,0x0443},{0xD6,0x0436},{0xD7,0x0432},{0xD8,0x044C},{0xD9,0x044B},
	{0xDA,0x0437},{0xDB,0x0448},{0xDC,0x044D},{0xDD,0x0449},{0xDE,0x0447},
	{0xDF,0x044A},{0xE0,0x042E},{0xE1,0x0410},{0xE2,0x0411},{0xE3,0x0426},
	{0xE4,0x0414},{0xE5,0x0415},{0xE6,0x0424},{0xE7,0x0413},{0xE8,0x0425},
	{0xE9,0x0418},{0xEA,0x0419},{0xEB,0x041A},{0xEC,0x041B},{0xED,0x041C},
	{0xEE,0x041D},{0xEF,0x041E},{0xF0,0x041F},{0xF1,0x042F},{0xF2,0x0420},
	{0xF3,0x0421},{0xF4,0x0422},{0xF5,0x0423},{0xF6,0x0416},{0xF7,0x0412},
	{0xF8,0x042C},{0xF9,0x042B},{0xFA,0x0417},{0xFB,0x0428},{0xFC,0x042D},
	{0xFD,0x0429},{0xFE,0x0427},{0xFF,0x042A}
};


/* Latin5 table */
typedef struct {
	UCHAR Latin5;
	MCHAR unicode;
}Latin5UniTable;

const Latin5UniTable tabLatin5Uni[] = {
	{0xA0,0x00A0},{0xA1,0x0401},{0xA2,0x0402},{0xA3,0x0403},{0xA4,0x0404},
	{0xA5,0x0405},{0xA6,0x0406},{0xA7,0x0407},{0xA8,0x0408},{0xA9,0x0409},
	{0xAA,0x040A},{0xAB,0x040B},{0xAC,0x040C},{0xAD,0x00AD},{0xAE,0x040E},
	{0xAF,0x040F},{0xB0,0x0410},{0xB1,0x0411},{0xB2,0x0412},{0xB3,0x0413},
	{0xB4,0x0414},{0xB5,0x0415},{0xB6,0x0416},{0xB7,0x0417},{0xB8,0x0418},
	{0xB9,0x0419},{0xBA,0x041A},{0xBB,0x041B},{0xBC,0x041C},{0xBD,0x041D},
	{0xBE,0x041E},{0xBF,0x041F},{0xC0,0x0420},{0xC1,0x0421},{0xC2,0x0422},
	{0xC3,0x0423},{0xC4,0x0424},{0xC5,0x0425},{0xC6,0x0426},{0xC7,0x0427},
	{0xC8,0x0428},{0xC9,0x0429},{0xCA,0x042A},{0xCB,0x042B},{0xCC,0x042C},
	{0xCD,0x042D},{0xCE,0x042E},{0xCF,0x042F},{0xD0,0x0430},{0xD1,0x0431},
	{0xD2,0x0432},{0xD3,0x0433},{0xD4,0x0434},{0xD5,0x0435},{0xD6,0x0436},
	{0xD7,0x0437},{0xD8,0x0438},{0xD9,0x0439},{0xDA,0x043A},{0xDB,0x043B},
	{0xDC,0x043C},{0xDD,0x043D},{0xDE,0x043E},{0xDF,0x043F},{0xE0,0x0440},
	{0xE1,0x0441},{0xE2,0x0442},{0xE3,0x0443},{0xE4,0x0444},{0xE5,0x0445},
	{0xE6,0x0446},{0xE7,0x0447},{0xE8,0x0448},{0xE9,0x0449},{0xEA,0x044A},
	{0xEB,0x044B},{0xEC,0x044C},{0xED,0x044D},{0xEE,0x044E},{0xEF,0x044F},
	{0xF0,0x2116},{0xF1,0x0451},{0xF2,0x0452},{0xF3,0x0453},{0xF4,0x0454},
	{0xF5,0x0455},{0xF6,0x0456},{0xF7,0x0457},{0xF8,0x0458},{0xF9,0x0459},
	{0xFA,0x045A},{0xFB,0x045B},{0xFC,0x045C},{0xFD,0x00A7},{0xFE,0x045E},
	{0xFF,0x045F},

};

#define __LOCALCODE_TO_UTF8

#ifdef __LOCALCODE_TO_UTF8

bool __WmConvertCodeBufferSizeCheck (char *ftnName, int outBufSize, int requiredBufSize)
{
	if (outBufSize < requiredBufSize) {

		if (outBufSize == sizeof(void*)) {  // if it is not array
			MSG_DEBUG("__WmConvertCodeBufferSizeCheck: Out buffer size seems to be small (%s)\n",ftnName);
		} else {
			MSG_DEBUG("__WmConvertCodeBufferSizeCheck: Buffer size is too small %s: OutBuffer(%d), RequiredBufSize(%d)\n",ftnName,outBufSize,requiredBufSize);
		}
		return false;
	}
	return true;
}



MCHAR __WmLngSwapShort(MCHAR aShort)
{
	return ((aShort << 8) + (aShort >> 8));
}

/**
 * converting byte ordering between Network and device
 *
 * @param	mszOutput	[out] converted[destination] MCHAR buffer
 * @param	mszInput		[in]	source MCHAR buffer
 * @param	length		[in]	source MCHAR's string length (not byte count)
 *
 * @return	This function returns a true on success, or false on failure.
 */
MCHAR* __WmLngSwapPCode(MCHAR *mszOutput, MCHAR *mszInput, int length)
{
	int	i;

	for (i = 0; i < length; i++) {
		if (mszInput[i] == (MCHAR)NULL)
			break;

		mszOutput[i] = __WmLngSwapShort(mszInput[i]);
	}

	mszOutput[i] = '\0';

	return mszOutput;
}


/**
 * This function convert character Processcode(Unicode) to Localcode(UTF8)
 *
 * @param	pszOutText	:	Output Buffer Pointer to LocalCode(UTF8)
 * @param	mszInText	:	Input Buffer Pointer to ProcessCode(UniCode)
 * @return	This function returns true on success, or false on failure.
 * @see		WmConvert2LCodeN
 */
bool WmConvert2LCode(char *pszOutText, int outBufSize, MCHAR *mszInText)
{
	int charCount;
	charCount = WmStrlen(mszInText);
	if (charCount == 0)
		pszOutText[0] = '\0';

	if ((WmConvertPCode2UTF((UCHAR*)pszOutText, outBufSize, mszInText, charCount) == true)) {
		return true;
	} else {
		MSG_DEBUG("WmConvert2LCode: Converting Unicode(%x) to utf8 code failed\n",mszInText);
		return false;
	}
}

/**
 * This function convert character Localcode(UTF8) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText		:	Input Buffer Pointer to LocalCode(UTF8)
 * @return	This function returns true on success, or false on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvert2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int byteCount;

	byteCount = strlen((char*) szInText);

	if (byteCount == 0)
		pmszOutText[0] = '\0';

	if((WmConvertUTF2PCode(pmszOutText, outBufSize,(UCHAR*)szInText, byteCount) == true)) {
		return true;
	} else {
		MSG_DEBUG("WmConvert2PCode: Converting UTF8code(%x) to Unicode failed\n",szInText);
		return false;
	}
}

/**
 * This function convert N'th byte Localcode(UTF8) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to  ProcessCode(UniCode)
 * @param	szInText		:	Input Buffer Pointer to LocalCode(KSC5601)
 * @param	byteCount		:	byte number for converting character
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCode
 */
bool WmConvert2PCodeN(MCHAR *pmszOutText, int outBufSize, char *szInText, int byteCount)
{
	if ((WmConvertUTF2PCode (pmszOutText, outBufSize, (UCHAR*)szInText, byteCount) == true)) {
		return true;
	} else {
		MSG_DEBUG("WmConvert2PCodeN: Converting UTF8code(%x) to Unicode failed\n",szInText);
		return false;
	}
}

/**
 * This function convert N'th character Processcode(Unicode) to Localcode(UTF8)
 *
 * @param	pszOutText		:	Output Buffer Pointer to LocalCode(UTF8)
 * @param	mszInText		:	Input Buffer Pointer to ProcessCode(UniCode)
 * @param	charCount		:	number for convert n'th chararcter
 * @return	This function returns true on success, or false on failure.
 *
 * @see		WmConvert2LCode
 */
bool WmConvert2LCodeN(char *pszOutText, int outBufSize, MCHAR *mszInText, int charCount)
{
	if ( (WmConvertPCode2UTF((UCHAR*)pszOutText, outBufSize, mszInText, charCount) == true)) {
		return true;
	} else {
		MSG_DEBUG("WmConvert2LCodeN: Converting Unicode(%x) to utf8 code failed\n",mszInText);
		return false;
	}
}

/**
 * This function return output LocalCode Buffer Size
 *
 * @param	mszText	:	Input ProcessCode String Pointer
 * @return	This function returns BufferSize for LocalCode
 *
 * @see		WmGetPcodeSize
 */
int WmGetLCodeSize(MCHAR *mszText)
{
	int nBuffSize = 0;

	if (mszText[0] == '\0')
		return 0;

	while (*mszText != '\0') {

		if (0x0001 <= *mszText && *mszText <= 0x007F) {
			nBuffSize++;
			mszText++;
		} else if ((*mszText == 0x0000) || (0x0080 <= *mszText && *mszText <= 0x07FF)) {
			nBuffSize += 2;
			mszText++;
		} else {
			nBuffSize += 3;
			mszText++;
		}
	}
	return nBuffSize;
}


/**
 * This function return output LocalCode Buffer Size
 *
 * @param	mszText	:	Input ProcessCode String Pointer
 * @return	This function returns BufferSize for LocalCode
 *
 * @see		WmGetPcodeSize
 */
int WmGetLCodeSizeN(MCHAR *mszText, int charCount)
{
	int nBuffSize = 0;
	int i = 0;

	if (mszText[0] == '\0')
		return 0;

	while ((*mszText != '\0') && (i < charCount)) {

		if (0x0001 <= *mszText && *mszText <= 0x007F) {
			nBuffSize++;
			mszText++;
		} else if ((*mszText == 0x0000) || (0x0080 <= *mszText && *mszText <= 0x07FF)) {
			nBuffSize += 2;
			mszText++;
		} else {
			nBuffSize += 3;
			mszText++;
		}
		i++;
	}
	return nBuffSize;
}
#endif


/**
 * This function convert character Localcode(Latin2) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin2 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin2Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin2Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin2Table = false;
		for (j = 0; j < tabLatin2UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin2Uni[j].latin2) {
				pmszOutText[i] = tabLatin2Uni[j].unicode;
				bInLatin2Table = true;
			}
		}
		if (bInLatin2Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin2 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


/**
 * This function convert character Localcode(Latin3) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin2 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin3Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin3Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin3Table = false;
		for (j = 0; j < tabLatin3UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin3Uni[j].latin3) {
				pmszOutText[i] = tabLatin3Uni[j].unicode;
				bInLatin3Table = true;
			}
		}
		if (bInLatin3Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin3 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}

/**
 * This function convert character Localcode(Latin4) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin2 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin4Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin4Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin4Table = false;
		for (j = 0; j < tabLatin4UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin4Uni[j].latin4) {
				pmszOutText[i] = tabLatin4Uni[j].unicode;
				bInLatin4Table = true;
			}
		}
		if (bInLatin4Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin4 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}

/**
 * This function convert character Localcode(Latin8) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin2 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin8Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin8Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin8Table = false;
		for (j = 0; j < tabLatin8UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin8Uni[j].latin8) {
				pmszOutText[i] = tabLatin8Uni[j].unicode;
				bInLatin8Table = true;
			}
		}
		if (bInLatin8Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin8 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


/**
 * This function convert character Localcode(Win1251) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Win1251 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertWin1251Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInWin1251Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertWin1251Code2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInWin1251Table = false;
		for (j = 0; j < tabWin1251UniMax; j++) {
			if ((UCHAR)szInText[i] == tabWin1251Uni[j].win1251) {
				pmszOutText[i] = tabWin1251Uni[j].unicode;
				bInWin1251Table = true;
			}
		}
		if (bInWin1251Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Win1251 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


/**
 * This function convert character Localcode(Koi8-r) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Koi8-r code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertKoi8rCode2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInKoi8rTable = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertKoi8rCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInKoi8rTable = false;
		for (j = 0; j < tabKoi8rUniMax; j++) {
			if ((UCHAR)szInText[i] == tabKoi8rUni[j].koi8r) {
				pmszOutText[i] = tabKoi8rUni[j].unicode;
				bInKoi8rTable = true;
			}
		}
		if (bInKoi8rTable == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* bInKoi8-rTable -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


/**
 * This function convert character Localcode(Koi8-u) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Koi8-u code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertKoi8uCode2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInKoi8uTable = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertKoi8uCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInKoi8uTable = false;
		for (j = 0; j < tabKoi8uUniMax; j++) {
			if ((UCHAR)szInText[i] == tabKoi8uUni[j].koi8u) {
				pmszOutText[i] = tabKoi8uUni[j].unicode;
				bInKoi8uTable = true;
			}
		}
		if (bInKoi8uTable == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* bInKoi8uTable -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


/**
 * This function convert character Localcode(Latin15) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin2 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin15Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin15Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin15Table = false;
		for (j = 0; j < tabLatin15UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin15Uni[j].latin15) {
				pmszOutText[i] = tabLatin15Uni[j].unicode;
				bInLatin15Table = true;
			}
		}
		if (bInLatin15Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin15 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}




/**
 * This function convert character Localcode(Latin5) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to Latin5 code
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatin5Code2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	int i = 0;
	int j = 0;
	int strLen = 0;
	bool bInLatin5Table = false;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	for (i = 0; i < strLen; i++) {
		bInLatin5Table = false;
		for (j = 0; j < tabLatin5UniMax; j++) {
			if ((UCHAR)szInText[i] == tabLatin5Uni[j].Latin5) {
				pmszOutText[i] = tabLatin5Uni[j].unicode;
				bInLatin5Table = true;
			}
		}
		if (bInLatin5Table == false)
			pmszOutText[i] = (MCHAR)(UCHAR)szInText[i];
	}
	/* Latin5 -> UNICODE */
	pmszOutText[strLen] = '\0';

	return true;
}


int WmGetLatin32UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int bufferSize = 0;
	int latin3Size = -1;
	MCHAR *pmszText;

	bufferSize = nChar*3+1;
	pmszText = (MCHAR *)malloc(bufferSize);
	if (!pmszText) {
		MSG_DEBUG("WmGetLatin32UTFCodeSize: memory allocation is failed!\n");
		return -1;
	}
	WmConvertLatin3Code2PCode(pmszText, bufferSize, (char*)szSrc);
	latin3Size = WmGetLCodeSize(pmszText);
	free(pmszText);
	return latin3Size;
}


int WmGetLatin42UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int bufferSize = 0;
	int latin4Size = -1;
	MCHAR *pmszText;

	bufferSize = nChar*3+1;
	pmszText = (MCHAR *)malloc(bufferSize);
	if (!pmszText) {
		MSG_DEBUG("WmGetLatin42UTFCodeSize: memory allocation is failed!\n");
		return -1;
	}
	WmConvertLatin4Code2PCode(pmszText, bufferSize, (char*)szSrc);
	latin4Size = WmGetLCodeSize(pmszText);
	free(pmszText);
	return latin4Size;
}


int WmGetLatin82UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int bufferSize = 0;
	int latin8Size = -1;
	MCHAR *pmszText;

	bufferSize = nChar*3+1;
	pmszText = (MCHAR *)malloc(bufferSize);
	if (!pmszText) {
		MSG_DEBUG("WmGetLatin82UTFCodeSize: memory allocation is failed!\n");
		return -1;
	}
	WmConvertLatin8Code2PCode(pmszText, bufferSize, (char*)szSrc);
	latin8Size = WmGetLCodeSize(pmszText);
	free(pmszText);
	return latin8Size;
}

int WmGetLatin152UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int bufferSize = 0;
	int latin15Size = -1;
	MCHAR *pmszText;

	bufferSize = nChar*3+1;
	pmszText = (MCHAR *)malloc(bufferSize);
	if (!pmszText) {
		MSG_DEBUG("WmGetLatin152UTFCodeSize: memory allocation is failed!\n");
		return -1;
	}
	WmConvertLatin15Code2PCode(pmszText, bufferSize, (char*)szSrc);
	latin15Size = WmGetLCodeSize(pmszText);
	free(pmszText);
	return latin15Size;
}

int WmGetLatin52UTFCodeSize(unsigned char *szSrc, int nChar)
{
	int bufferSize = 0;
	int latin5Size = -1;
	MCHAR *pmszText;

	bufferSize = nChar*3+1;
	pmszText = (MCHAR *)malloc(bufferSize);
	if (!pmszText) {
		MSG_DEBUG("WmGetLatin52UTFCodeSize: memory allocation is failed!\n");
		return -1;
	}
	WmConvertLatin5Code2PCode(pmszText, bufferSize, (char*)szSrc);
	latin5Size = WmGetLCodeSize(pmszText);
	free(pmszText);
	return latin5Size;
}


/**
 * This function convert character Localcode(Latin1) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to LocalCode(KSC5601)
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCodeN
 */
bool WmConvertLatinCode2PCode(MCHAR *pmszOutText, int outBufSize, char *szInText)
{
	UINT strLen;

	strLen = strlen((char *)szInText);

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, strLen) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCode: Out buffer size seems to be small!\n");
		return false;
	}

	/* Latin1 -> UNICODE */
	__WmLngConvertLatin12Unicode(pmszOutText,(UCHAR*)szInText,strLen);
	pmszOutText[strLen] = '\0';

	return true;
}

/**
 * This function convert N'th character Localcode(Latin1) to Processcode(Unicode)
 *
 * @param	pmszOutText		:	Output Buffer Pointer to  ProcessCode(UniCode)
 * @param	szInText	:	Input Buffer Pointer to LocalCode(KSC5601)
 * @param	charCount		:	number for convert n'th chararcter
 * @return	This function returns true on success, or false
 *			on failure.
 * @see		WmConvert2PCode
 */
bool WmConvertLatinCode2PCodeN(MCHAR *pmszOutText, int outBufSize, char *szInText, int charCount)
{
	int strLen;

	if (charCount == 0) {
		MSG_DEBUG("WmConvert2PCodeN: charCount is0\n");
		pmszOutText[charCount] = '\0';
	}

	if (__WmConvertCodeBufferSizeCheck((char*)pmszOutText, outBufSize, charCount) == false) {
		MSG_DEBUG("WmConvertLatinCode2PCodeN: Out buffer size seems to be small!\n");
		return false;
	}

	/* Latin1 -> UNICODE */
	__WmLngLatin12UnicodeN (pmszOutText, (UCHAR*)szInText, charCount);

	strLen = strlen((char *)szInText);
	if(strLen < charCount) {
		pmszOutText[strLen] = '\0';
	} else {
		pmszOutText[charCount] = '\0';
	}

	return true;
}

/*
 * This function convert character Processcode(Unicode) to Localcode(Latin1)
 *
 * @param	pOutStr		:	Output Buffer Pointer to LocalCode(Latin1)
 * @param	pInputStr	:	Input Buffer Pointer to ProcessCode(UniCode)
 * @param	len			:	number for convert n'th chararcter
 * @return	void
 *
 * @see
 */
static void __WmLngReverseKeyCode(UCHAR *pOutStr, MCHAR *pInputStr, UINT len)
{
	UCHAR *rear=NULL;
	MCHAR *p;
	UCHAR temp;

	rear = pOutStr;

	for (p = pInputStr; len > 0 && p; len--) {
		if (*pInputStr < MAX_ASCII_NUM) {                  // ASCII String
			if (*p == 0)
				*rear = '\0';
			temp = (UCHAR)(*p);
			*rear = temp;
			rear++;
			p++;
			if(len == 1)
				*rear = '\0';
		} else {
			*rear = 0x3F;
			rear++;
			p++;
			if(len == 1)
				*rear = '\0';
		}
		pInputStr = p;
	}
}


/*
 * This function convert character Localcode(Latin1) to Processcode(Unicode)
 *
 * @param	des[in]		:	Output Buffer Pointer to ProcessCode(UniCode)
 * @param	str[in]		:	Input Buffer Pointer to LocalCode(Latin1)
 * @param	len[in]		:	number for convert n'th chararcter
 * @return	This function returns number for convert n'th chararcter on success
 *
 * @see
 */
static int __WmLngConvertLatin12Unicode (MCHAR *des, UCHAR *str, UINT len)
{
	MCHAR *org;
	org = des;
	while (len>0) {
		*des++ = *str++;
		len--;
	}

	return (des - org) ;
}

/*
 * This function convert N'th character Localcode(Latin1) to Processcode(Unicode)
 *
 * @param	pOutStr		:	Output Buffer Pointer to  ProcessCode(UniCode)
 * @param	pInputStr	:	Input Buffer Pointer to LocalCode(Latin1)
 * @param	len			:	number for convert n'th chararcter
 * @return	This function returns true on success, or false on failure.
 *
 * @see
 */
static UINT __WmLngLatin12UnicodeN(MCHAR *pOutStr, UCHAR *pInputStr, UINT len)
{
	UINT n;

	n = strlen((char*)pInputStr);

	if(len > n)
		len = n;

   	return __WmLngConvertLatin12Unicode (pOutStr, pInputStr, len);
}

