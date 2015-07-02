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

#include <glib.h>

#include "MsgDebug.h"
#include "MsgTextConvert.h"
#include "MsgCppTypes.h"

using namespace std;
/*==================================================================================================
                                     IMPLEMENTATION OF MsgConvertText - Member Functions
==================================================================================================*/
MsgTextConvert* MsgTextConvert::pInstance = NULL;

MsgTextConvert::MsgTextConvert()
{
	extCharList.clear();
	ucs2toGSM7DefList.clear();
	ucs2toGSM7ExtList.clear();
	ucs2toTurkishList.clear();
	ucs2toSpanishList.clear();
	ucs2toPortuList.clear();
	replaceCharList.clear();

	extCharList[0x000C] = MSG_CHAR_GSM7EXT;
	extCharList[0x005B] = MSG_CHAR_GSM7EXT;
	extCharList[0x005C] = MSG_CHAR_GSM7EXT;
	extCharList[0x005D] = MSG_CHAR_GSM7EXT;
	extCharList[0x005E] = MSG_CHAR_GSM7EXT;
	extCharList[0x007B] = MSG_CHAR_GSM7EXT;
	extCharList[0x007C] = MSG_CHAR_GSM7EXT;
	extCharList[0x007D] = MSG_CHAR_GSM7EXT;
	extCharList[0x007E] = MSG_CHAR_GSM7EXT;
	extCharList[0x20AC] = MSG_CHAR_GSM7EXT;

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
	ucs2toGSM7ExtList[0x20AC] = 0x65; // €

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
	ucs2toTurkishList[0x20AC] = 0x65; // €
	ucs2toTurkishList[0x00E7] = 0x63; // c LATIN SMALL LETTER S WITH CEDILLA
	ucs2toTurkishList[0x011E] = 0x47; // G LATIN CAPITAL LETTER G WITH BREVE
	ucs2toTurkishList[0x011F] = 0x67; // g LATIN SMALL LETTER G WITH BREVE
	ucs2toTurkishList[0x01E6] = 0x47; // G LATIN CAPITAL LETTER G WITH CARON
	ucs2toTurkishList[0x01E7] = 0x67; // g LATIN SMALL LETTER G WITH CARON
	ucs2toTurkishList[0x0130] = 0x49; // I LATIN CAPITAL LETTER I WITH DOT ABOVE
	ucs2toTurkishList[0x0131] = 0x69; // i LATIN SMALL LETTER DOTLESS
	ucs2toTurkishList[0x015E] = 0x53; // S LATIN CAPITAL LETTER S WITH CEDILLA
	ucs2toTurkishList[0x015F] = 0x73; // s LATIN SMALL LETTER S WITH CEDILLA

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
	ucs2toSpanishList[0x20AC] = 0x65; // €
	ucs2toSpanishList[0x00C1] = 0x41; // A LATIN CAPITAL LETTER A WITH ACUTE
	ucs2toSpanishList[0x00E1] = 0x61; // a LATIN SMALL LETTER A WITH ACUTE
	ucs2toSpanishList[0x00CD] = 0x49; // I LATIN CAPITAL LETTER I WITH ACUTE
	ucs2toSpanishList[0x00ED] = 0x69; // i LATIN SMALL LETTER I WITH ACUTE
	ucs2toSpanishList[0x00D3] = 0x4F; // O LATIN CAPITAL LETTER O WITH ACUTE
	ucs2toSpanishList[0x00F3] = 0x6F; // o LATIN SMALL LETTER O WITH ACUTE
	ucs2toSpanishList[0x00DA] = 0x55; // U LATIN CAPITAL LETTER U WITH ACUTE
	ucs2toSpanishList[0x00FA] = 0x75; // u LATIN SMALL LETTER U WITH ACUTE

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
	ucs2toPortuList[0x20AC] = 0x65; // €
	ucs2toPortuList[0x00D4] = 0x0B; // O LATIN CAPITAL LETTER O WITH CIRCUMFLEX
	ucs2toPortuList[0x00F4] = 0x0C; // o LATIN SMALL LETTER O WITH CIRCUMFLEX
	ucs2toPortuList[0x00C1] = 0x0E; // A LATIN CAPITAL LETTER A WITH ACUTE
	ucs2toPortuList[0x00E1] = 0x0F; // a LATIN SMALL LETTER A WITH ACUTE
	ucs2toPortuList[0x00CA] = 0x1F; // E LATIN CAPITAL LETTER E WITH CIRCUMFLEX
	ucs2toPortuList[0x00EA] = 0x05; // e LATIN SMALL LETTER E WITH CIRCUMFLEX
	ucs2toPortuList[0x00C0] = 0x41; // A LATIN CAPITAL LETTER A WITH GRAVE
	ucs2toPortuList[0x00E7] = 0x09; // c LATIN SMALL LETTER C WITH CEDILLA
	ucs2toPortuList[0x00CD] = 0x49; // I LATIN CAPITAL LETTER I WITH ACUTE
	ucs2toPortuList[0x00ED] = 0x69; // i LATIN SMALL LETTER I WITH ACUTE
	ucs2toPortuList[0x00D3] = 0x4F; // O LATIN CAPITAL LETTER O WITH ACUTE
	ucs2toPortuList[0x00F3] = 0x6F; // o LATIN SMALL LETTER O WITH ACUTE
	ucs2toPortuList[0x00DA] = 0x55; // U LATIN CAPITAL LETTER U WITH ACUTE
	ucs2toPortuList[0x00FA] = 0x75; // u LATIN SMALL LETTER U WITH ACUTE
	ucs2toPortuList[0x00C3] = 0x61; // A LATIN CAPITAL LETTER A WITH TILDE
	ucs2toPortuList[0x00E3] = 0x7B; // a LATIN SMALL LETTER A WITH TILDE
	ucs2toPortuList[0x00D5] = 0x5C; // O LATIN CAPITAL LETTER O WITH TILDE
	ucs2toPortuList[0x00F5] = 0x7C; // o LATIN SMALL LETTER O WITH TILDE
	ucs2toPortuList[0x00C2] = 0x61; // A LATIN CAPITAL LETTER A WITH CIRCUMFLEX
	ucs2toPortuList[0x00E2] = 0x7F; // a LATIN SMALL LETTER A WITH CIRCUMFLEX
	ucs2toPortuList[0x03A6] = 0x12; // Φ GREEK CAPITAL LETTER PHI
	ucs2toPortuList[0x0393] = 0x13; // Γ GREEK CAPITAL LETTER GAMMA
	ucs2toPortuList[0x03A9] = 0x15; // Ω GREEK CAPITAL LETTER OMEGA
	ucs2toPortuList[0x03A0] = 0x16; // Π GREEK CAPITAL LETTER PI
	ucs2toPortuList[0x03A8] = 0x17; // Ψ GREEK CAPITAL LETTER PSI
	ucs2toPortuList[0x03A3] = 0x18; // Σ GREEK CAPITAL LETTER SIGMA
	ucs2toPortuList[0x0398] = 0x19; // Θ GREEK CAPITAL LETTER THETA


	// character replacement table
	replaceCharList[0x00E0] = 0x61; // a LATIN SMALL LETTER A WITH GRAVE
	replaceCharList[0x00E1] = 0x61; // a LATIN SMALL LETTER A WITH ACUTE
	replaceCharList[0x00E2] = 0x61; // a LATIN SMALL LETTER A WITH CIRCUMFLEX
	replaceCharList[0x00E3] = 0x61; // a LATIN SMALL LETTER A WITH TILDE
	replaceCharList[0x00E4] = 0x61; // a LATIN SMALL LETTER A WITH DIAERESIS
	replaceCharList[0x00E5] = 0x61; // a LATIN SMALL LETTER A WITH RING ABOVE
	replaceCharList[0x00E6] = 0x61; // a LATIN SMALL LETTER AE
	replaceCharList[0x0101] = 0x61; // a LATIN SMALL LETTER A WITH MACRON
	replaceCharList[0x0103] = 0x61; // a LATIN SMALL LETTER A WITH BREVE
	replaceCharList[0x0105] = 0x61; // a LATIN SMALL LETTER A WITH OGONEK
	replaceCharList[0x01CE] = 0x61; // a LATIN SMALL LETTER A WITH CARON

	replaceCharList[0x00C0] = 0x41; // A LATIN CAPITAL LETTER A WITH GRAVE
	replaceCharList[0x00C1] = 0x41; // A LATIN CAPITAL LETTER A WITH ACUTE
	replaceCharList[0x00C2] = 0x41; // A LATIN CAPITAL LETTER A WITH CIRCUMFLEX
	replaceCharList[0x00C3] = 0x41; // A LATIN CAPITAL LETTER A WITH TILDE
	replaceCharList[0x00C4] = 0x41; // A LATIN CAPITAL LETTER A WITH DIAERESIS
	replaceCharList[0x00C5] = 0x41; // A LATIN CAPITAL LETTER A WITH RING ABOVE
	replaceCharList[0x00C6] = 0x41; // A LATIN CAPITAL LETTER AE
	replaceCharList[0x0100] = 0x41; // A LATIN CAPITAL LETTER A WITH MACRON
	replaceCharList[0x0102] = 0x41; // A LATIN CAPITAL LETTER A WITH BREVE
	replaceCharList[0x0104] = 0x41; // A LATIN CAPITAL LETTER A WITH OGONEK
	replaceCharList[0x01CD] = 0x41; // A LATIN CAPITAL LETTER A WITH CARON

	replaceCharList[0x00E7] = 0x63; // c LATIN SMALL LETTER C WITH CEDILLA
	replaceCharList[0x0107] = 0x63; // c LATIN SMALL LETTER C WITH ACUTE
	replaceCharList[0x0109] = 0x63; // c LATIN SMALL LETTER C WITH CIRCUMFLEX
	replaceCharList[0x010B] = 0x63; // c LATIN SMALL LETTER C WITH DOT ABOVE
	replaceCharList[0x010D] = 0x63; // c LATIN SMALL LETTER C WITH CARON

	replaceCharList[0x00C7] = 0x43; // C LATIN CAPITAL LETTER C WITH CEDILLA
	replaceCharList[0x0106] = 0x43; // C LATIN CAPITAL LETTER C WITH ACUTE
	replaceCharList[0x0108] = 0x43; // C LATIN CAPITAL LETTER C WITH CIRCUMFLEX
	replaceCharList[0x010A] = 0x43; // C LATIN CAPITAL LETTER C WITH DOT ABOVE
	replaceCharList[0x010C] = 0x43; // C LATIN CAPITAL LETTER C WITH CARON

	replaceCharList[0x010F] = 0x64; // d LATIN SMALL LETTER D WITH CARON
	replaceCharList[0x0111] = 0x64; // d LATIN SMALL LETTER D WITH STROKE

	replaceCharList[0x010E] = 0x44; // D LATIN CAPITAL LETTER D WITH CARON
	replaceCharList[0x0110] = 0x44; // D LATIN CAPITAL LETTER D WITH STROKE

	replaceCharList[0x00E8] = 0x65; // e LATIN SMALL LETTER E WITH GRAVE
	replaceCharList[0x00E9] = 0x65; // e LATIN SMALL LETTER E WITH ACUTE
	replaceCharList[0x00EA] = 0x65; // e LATIN SMALL LETTER E WITH CIRCUMFLEX
	replaceCharList[0x00EB] = 0x65; // e LATIN SMALL LETTER E WITH DIAERESIS
	replaceCharList[0x0113] = 0x65; // e LATIN SMALL LETTER E WITH MACRON
	replaceCharList[0x0115] = 0x65; // e LATIN SMALL LETTER E WITH BREVE
	replaceCharList[0x0117] = 0x65; // e LATIN SMALL LETTER E WITH DOT ABOVE
	replaceCharList[0x0119] = 0x65; // e LATIN SMALL LETTER E WITH OGONEK
	replaceCharList[0x011B] = 0x65; // e LATIN SMALL LETTER E WITH CARON
	replaceCharList[0x0259] = 0x65; // e LATIN SMALL LETTER SCHWA

	replaceCharList[0x00C8] = 0x45; // E LATIN CAPITAL LETTER E WITH GRAVE
	replaceCharList[0x00C9] = 0x45; // E LATIN CAPITAL LETTER E WITH ACUTE
	replaceCharList[0x00CA] = 0x45; // E LATIN CAPITAL LETTER E WITH CIRCUMFLEX
	replaceCharList[0x00CB] = 0x45; // E LATIN CAPITAL LETTER E WITH DIAERESIS
	replaceCharList[0x0112] = 0x45; // E LATIN CAPITAL LETTER E WITH MACRON
	replaceCharList[0x0114] = 0x45; // E LATIN CAPITAL LETTER E WITH BREVE
	replaceCharList[0x0116] = 0x45; // E LATIN CAPITAL LETTER E WITH DOT ABOVE
	replaceCharList[0x0118] = 0x45; // E LATIN CAPITAL LETTER E WITH OGONEK
	replaceCharList[0x011A] = 0x45; // E LATIN CAPITAL LETTER E WITH CARON
	replaceCharList[0x018F] = 0x45; // E LATIN CAPITAL LETTER SCHWA

	replaceCharList[0x011D] = 0x67; // g LATIN SMALL LETTER G WITH CIRCUMFLEX
	replaceCharList[0x011F] = 0x67; // g LATIN SMALL LETTER G WITH BREVE
	replaceCharList[0x0121] = 0x67; // g LATIN SMALL LETTER G WITH DOT ABOVE
	replaceCharList[0x0123] = 0x67; // g LATIN SMALL LETTER G WITH CEDILLA
	replaceCharList[0x01E7] = 0x67; // g LATIN SMALL LETTER G WITH CARON
	replaceCharList[0x01F5] = 0x67; // g LATIN SMALL LETTER G WITH ACUTE
	replaceCharList[0x1E21] = 0x67; // g LATIN SMALL LETTER G WITH MACRON

	replaceCharList[0x011C] = 0x47; // G LATIN CAPITAL LETTER G WITH CIRCUMFLEX
	replaceCharList[0x011E] = 0x47; // G LATIN CAPITAL LETTER G WITH BREVE
	replaceCharList[0x0120] = 0x47; // G LATIN CAPITAL LETTER G WITH DOT ABOVE
	replaceCharList[0x0122] = 0x47; // G LATIN CAPITAL LETTER G WITH CEDILLA
	replaceCharList[0x01E6] = 0x47; // G LATIN CAPITAL LETTER G WITH CARON
	replaceCharList[0x01F4] = 0x47; // G LATIN CAPITAL LETTER G WITH ACUTE
	replaceCharList[0x1E20] = 0x47; // G LATIN CAPITAL LETTER G WITH MACRON

	replaceCharList[0x00EC] = 0x69; // i LATIN SMALL LETTER I WITH GRAVE
	replaceCharList[0x00ED] = 0x69; // i LATIN SMALL LETTER I WITH ACUTE
	replaceCharList[0x00EE] = 0x69; // i LATIN SMALL LETTER I WITH CIRCUMFLEX
	replaceCharList[0x00EF] = 0x69; // i LATIN SMALL LETTER I WITH DIAERESIS
	replaceCharList[0x0129] = 0x69; // i LATIN SMALL LETTER I WITH TILDE
	replaceCharList[0x012B] = 0x69; // i LATIN SMALL LETTER I WITH MACRON
	replaceCharList[0x012D] = 0x69; // i LATIN SMALL LETTER I WITH BREVE
	replaceCharList[0x012F] = 0x69; // i LATIN SMALL LETTER I WITH OGONEK
	replaceCharList[0x01D0] = 0x69; // i LATIN SMALL LETTER I WITH CARON
	replaceCharList[0x0131] = 0x69; // i LATIN SMALL LETTER DOTLESS I

	replaceCharList[0x00CC] = 0x49; // I LATIN CAPITAL LETTER I WITH GRAVE
	replaceCharList[0x00CD] = 0x49; // I LATIN CAPITAL LETTER I WITH ACUTE
	replaceCharList[0x00CE] = 0x49; // I LATIN CAPITAL LETTER I WITH CIRCUMFLEX
	replaceCharList[0x00CF] = 0x49; // I LATIN CAPITAL LETTER I WITH DIAERESIS
	replaceCharList[0x0128] = 0x49; // I LATIN CAPITAL LETTER I WITH TILDE
	replaceCharList[0x012A] = 0x49; // I LATIN CAPITAL LETTER I WITH MACRON
	replaceCharList[0x012C] = 0x49; // I LATIN CAPITAL LETTER I WITH BREVE
	replaceCharList[0x012E] = 0x49; // I LATIN CAPITAL LETTER I WITH OGONEK
	replaceCharList[0x0130] = 0x49; // I LATIN CAPITAL LETTER I WITH DOT ABOVE

	replaceCharList[0x0137] = 0x6B; // k LATIN SMALL LETTER K WITH CEDILLA

	replaceCharList[0x0136] = 0x4B; // K LATIN CAPITAL LETTER K WITH CEDILLA

	replaceCharList[0x013A] = 0x6C; // l LATIN SMALL LETTER L WITH ACUTE
	replaceCharList[0x013C] = 0x6C; // l LATIN SMALL LETTER L WITH CEDILLA
	replaceCharList[0x013E] = 0x6C; // l LATIN SMALL LETTER L WITH CARON
	replaceCharList[0x0140] = 0x6C; // l LATIN SMALL LETTER L WITH MIDDLE DOT
	replaceCharList[0x0142] = 0x6C; // l LATIN SMALL LETTER L WITH STROKE

	replaceCharList[0x0139] = 0x4C; // L LATIN CAPITAL LETTER L WITH ACUTE
	replaceCharList[0x013B] = 0x4C; // L LATIN CAPITAL LETTER L WITH CEDILLA
	replaceCharList[0x013D] = 0x4C; // L LATIN CAPITAL LETTER L WITH CARON
	replaceCharList[0x013F] = 0x4C; // L LATIN CAPITAL LETTER L WITH MIDDLE DOT
	replaceCharList[0x0141] = 0x4C; // L LATIN CAPITAL LETTER L WITH STROKE

	replaceCharList[0x00F1] = 0x6E; // n LATIN SMALL LETTER N WITH TILDE
	replaceCharList[0x0144] = 0x6E; // n LATIN SMALL LETTER N WITH ACUTE
	replaceCharList[0x0146] = 0x6E; // n LATIN SMALL LETTER N WITH CEDILLA
	replaceCharList[0x0148] = 0x6E; // n LATIN SMALL LETTER N WITH CARON

	replaceCharList[0x00D1] = 0x4E; // N LATIN CAPITAL LETTER N WITH TILDE
	replaceCharList[0x0143] = 0x4E; // N LATIN CAPITAL LETTER N WITH ACUTE
	replaceCharList[0x0145] = 0x4E; // N LATIN CAPITAL LETTER N WITH CEDILLA
	replaceCharList[0x0147] = 0x4E; // N LATIN CAPITAL LETTER N WITH CARON

	replaceCharList[0x00F2] = 0x6F; // o LATIN SMALL LETTER O WITH GRAVE
	replaceCharList[0x00F3] = 0x6F; // o LATIN SMALL LETTER O WITH ACUTE
	replaceCharList[0x00F4] = 0x6F; // o LATIN SMALL LETTER O WITH CIRCUMFLEX
	replaceCharList[0x00F5] = 0x6F; // o LATIN SMALL LETTER O WITH TILDE
	replaceCharList[0x00F6] = 0x6F; // o LATIN SMALL LETTER O WITH DIAERESIS
	replaceCharList[0x00F8] = 0x6F; // o LATIN SMALL LETTER O WITH STROKE
	replaceCharList[0x014D] = 0x6F; // o LATIN SMALL LETTER O WITH MACRON
	replaceCharList[0x014F] = 0x6F; // o LATIN SMALL LETTER O WITH BREVE
	replaceCharList[0x01D2] = 0x6F; // o LATIN SMALL LETTER O WITH CARON
	replaceCharList[0x01EB] = 0x6F; // o LATIN SMALL LETTER O WITH OGONEK
	replaceCharList[0x0151] = 0x6F; // o LATIN SMALL LETTER O WITH DOUBLE ACUTE
	replaceCharList[0x0153] = 0x6F; // LATIN SMALL LIGATURE OE

	replaceCharList[0x00D2] = 0x4F; // O LATIN CAPITAL LETTER O WITH GRAVE
	replaceCharList[0x00D3] = 0x4F; // O LATIN CAPITAL LETTER O WITH ACUTE
	replaceCharList[0x00D4] = 0x4F; // O LATIN CAPITAL LETTER O WITH CIRCUMFLEX
	replaceCharList[0x00D5] = 0x4F; // O LATIN CAPITAL LETTER O WITH TILDE
	replaceCharList[0x00D6] = 0x4F; // O LATIN CAPITAL LETTER O WITH DIAERESIS
	replaceCharList[0x00D8] = 0x4F; // O LATIN CAPITAL LETTER O WITH STROKE
	replaceCharList[0x014C] = 0x4F; // O LATIN CAPITAL LETTER O WITH MACRON
	replaceCharList[0x014E] = 0x4F; // O LATIN CAPITAL LETTER O WITH BREVE
	replaceCharList[0x01D1] = 0x4F; // O LATIN CAPITAL LETTER O WITH CARON
	replaceCharList[0x01EA] = 0x4F; // O LATIN CAPITAL LETTER O WITH OGONEK
	replaceCharList[0x0150] = 0x4F; // O LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
	replaceCharList[0x0152] = 0x4F; // LATIN CAPITAL LIGATURE OE

	replaceCharList[0x0155] = 0x72; // r LATIN SMALL LETTER R WITH ACUTE
	replaceCharList[0x0157] = 0x72; // r LATIN SMALL LETTER R WITH CEDILLA
	replaceCharList[0x0159] = 0x72; // r LATIN SMALL LETTER R WITH CARON

	replaceCharList[0x0154] = 0x52; // R LATIN CAPITAL LETTER R WITH ACUTE
	replaceCharList[0x0156] = 0x52; // R LATIN CAPITAL LETTER R WITH CEDILLA
	replaceCharList[0x0158] = 0x52; // R LATIN CAPITAL LETTER R WITH CARON

	replaceCharList[0x015B] = 0x73; // s LATIN SMALL LETTER S WITH ACUTE
	replaceCharList[0x015D] = 0x73; // s LATIN SMALL LETTER S WITH CIRCUMFLEX
	replaceCharList[0x015F] = 0x73; // s LATIN SMALL LETTER S WITH CEDILLA
	replaceCharList[0x0161] = 0x73; // s LATIN SMALL LETTER S WITH CARON

	replaceCharList[0x015A] = 0x53; // S LATIN CAPITAL LETTER S WITH ACUTE
	replaceCharList[0x015C] = 0x53; // S LATIN CAPITAL LETTER S WITH CIRCUMFLEX
	replaceCharList[0x015E] = 0x53; // S LATIN CAPITAL LETTER S WITH CEDILLA
	replaceCharList[0x0160] = 0x53; // S LATIN CAPITAL LETTER S WITH CARON

	replaceCharList[0x00FE] = 0x74; // t LATIN CAPITAL LETTER THORN
	replaceCharList[0x0163] = 0x74; // t LATIN SMALL LETTER T WITH CEDILLA
	replaceCharList[0x0165] = 0x74; // t LATIN SMALL LETTER T WITH CARON
	replaceCharList[0x0167] = 0x74; // t LATIN SMALL LETTER T WITH STROKE
	replaceCharList[0x021B] = 0x74; // t LATIN SMALL LETTER T WITH COMMA BELOW

	replaceCharList[0x00DE] = 0x54; // T LATIN CAPITAL LETTER THORN
	replaceCharList[0x0162] = 0x54; // T LATIN CAPITAL LETTER T WITH CEDILLA
	replaceCharList[0x0164] = 0x54; // T LATIN CAPITAL LETTER T WITH CARON
	replaceCharList[0x0166] = 0x54; // T LATIN CAPITAL LETTER T WITH STROKE

	replaceCharList[0x00F9] = 0x75; // u LATIN SMALL LETTER U WITH GRAVE
	replaceCharList[0x00FA] = 0x75; // u LATIN SMALL LETTER U WITH ACUTE
	replaceCharList[0x00FB] = 0x75; // u LATIN SMALL LETTER U WITH CIRCUMFLEX
	replaceCharList[0x00FC] = 0x75; // u LATIN SMALL LETTER U WITH DIAERESIS
	replaceCharList[0x0169] = 0x75; // u LATIN SMALL LETTER U WITH TILDE
	replaceCharList[0x016B] = 0x75; // u LATIN SMALL LETTER U WITH MACRON
	replaceCharList[0x016D] = 0x75; // u LATIN SMALL LETTER U WITH BREVE
	replaceCharList[0x016F] = 0x75; // u LATIN SMALL LETTER U WITH RING ABOVE
	replaceCharList[0x0171] = 0x75; // u LATIN SMALL LETTER U WITH DOUBLE ACUTE
	replaceCharList[0x0173] = 0x75; // u LATIN SMALL LETTER U WITH OGONEK
	replaceCharList[0x01D4] = 0x75; // u LATIN SMALL LETTER U WITH CARON

	replaceCharList[0x00D9] = 0x55; // U LATIN CAPITAL LETTER U WITH GRAVE
	replaceCharList[0x00DA] = 0x55; // U LATIN CAPITAL LETTER U WITH ACUTE
	replaceCharList[0x00DB] = 0x55; // U LATIN CAPITAL LETTER U WITH CIRCUMFLEX
	replaceCharList[0x00DC] = 0x55; // U LATIN CAPITAL LETTER U WITH DIAERESIS
	replaceCharList[0x0168] = 0x55; // U LATIN CAPITAL LETTER U WITH TILDE
	replaceCharList[0x016A] = 0x55; // U LATIN CAPITAL LETTER U WITH MACRON
	replaceCharList[0x016C] = 0x55; // U LATIN CAPITAL LETTER U WITH BREVE
	replaceCharList[0x016E] = 0x55; // U LATIN CAPITAL LETTER U WITH RING ABOVE
	replaceCharList[0x0170] = 0x55; // U LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
	replaceCharList[0x0172] = 0x55; // U LATIN CAPITAL LETTER U WITH OGONEK
	replaceCharList[0x01D3] = 0x55; // U LATIN CAPITAL LETTER U WITH CARON

	replaceCharList[0x00FD] = 0x79; // y LATIN SMALL LETTER Y WITH ACUTE
	replaceCharList[0x00FF] = 0x79; // y LATIN SMALL LETTER Y WITH DIAERESIS
	replaceCharList[0x0177] = 0x79; // y LATIN SMALL LETTER Y WITH CIRCUMFLEX
	replaceCharList[0x0233] = 0x79; // y LATIN SMALL LETTER Y WITH MACRON
	replaceCharList[0x1EF3] = 0x79; // y LATIN SMALL LETTER Y WITH GRAVE
	replaceCharList[0x1EF9] = 0x79; // y LATIN SMALL LETTER Y WITH TILDE

	replaceCharList[0x00DD] = 0x59; // Y LATIN CAPITAL LETTER Y WITH ACUTE
	replaceCharList[0x0176] = 0x59; // Y LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
	replaceCharList[0x0178] = 0x59; // Y LATIN CAPITAL LETTER Y WITH DIAERESIS
	replaceCharList[0x0232] = 0x59; // Y LATIN CAPITAL LETTER Y WITH MACRON
	replaceCharList[0x1EF2] = 0x59; // Y LATIN CAPITAL LETTER Y WITH GRAVE
	replaceCharList[0x1EF8] = 0x59; // Y LATIN CAPITAL LETTER Y WITH TILDE

	replaceCharList[0x017A] = 0x7A; // z LATIN SMALL LETTER Z WITH ACUTE
	replaceCharList[0x017C] = 0x7A; // z LATIN SMALL LETTER Z WITH DOT ABOVE
	replaceCharList[0x017E] = 0x7A; // z LATIN SMALL LETTER Z WITH CARON

	replaceCharList[0x0179] = 0x5A; // Z LATIN CAPITAL LETTER Z WITH ACUTE
	replaceCharList[0x017B] = 0x5A; // Z LATIN CAPITAL LETTER Z WITH DOT ABOVE
	replaceCharList[0x017D] = 0x5A; // Z LATIN CAPITAL LETTER Z WITH CARON
}


MsgTextConvert::~MsgTextConvert()
{
	extCharList.clear();
	ucs2toGSM7DefList.clear();
	ucs2toGSM7ExtList.clear();
	ucs2toTurkishList.clear();
	ucs2toSpanishList.clear();
	ucs2toPortuList.clear();
	replaceCharList.clear();
}


MsgTextConvert* MsgTextConvert::instance()
{
	if (!pInstance) {
		MSG_DEBUG("pInstance is NULL. Now creating instance.");
		pInstance = new MsgTextConvert();
	}

	return pInstance;
}


int MsgTextConvert::convertUTF8ToGSM7bit(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *pLangId, OUT bool *bIncludeAbnormalChar)
{
	int utf8Length = 0;
	int gsm7bitLength = 0;
	int ucs2Length = 0;

	if (srcTextLen <= 0 && pSrcText) {
		utf8Length = strlen((char*)pSrcText);
		srcTextLen = utf8Length;
	} else {
		utf8Length = srcTextLen;
	}

	int maxUCS2Length = utf8Length;		// max # of UCS2 chars, NOT bytes. when all utf8 chars are only one byte, UCS2Length is maxUCS2 Length. otherwise (ex: 2 bytes of UTF8 is one char) UCS2Length must be  less than utf8Length

	WCHAR *pUCS2Text = NULL;
	unique_ptr<WCHAR*, void(*)(WCHAR**)> buf(&pUCS2Text, unique_ptr_deleter);
	pUCS2Text = (WCHAR *)new char[maxUCS2Length * sizeof(WCHAR)];
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
	int textLen;
	unsigned char *unicodeTemp = (unsigned char*)pDestText;
	int ucs2Length = 0;
	int remainedBuffer = maxLength;

#ifdef CONVERT_DUMP
	int srcLen = srcTextLen;
	const unsigned char * pTempSrcText = pSrcText;
	const unsigned char * pTempDestText = pDestText;
#endif

	if(maxLength == 0 || pSrcText == NULL || pDestText ==  NULL) {
		MSG_DEBUG("UTF8 to UCS2 Failed as text length is 0\n");
		return -1;
	}

	// null terminated string
	if ( srcTextLen == -1 ) {
		textLen = strlen((char*)pSrcText);
		srcTextLen = textLen;
	} else {
		textLen = srcTextLen;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open("UTF16BE", "UTF8");

	if (cd > 0) {
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&textLen, (char**)&unicodeTemp, (gsize*)&remainedBuffer);
	}

	if(err < 0) {
		MSG_DEBUG("Error in g_iconv.");
		ucs2Length = -1;
	} else {
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

#ifndef FEATURE_SMS_CDMA
int MsgTextConvert::convertUTF8ToAuto(OUT unsigned char *pDestText, IN int maxLength,  IN const unsigned char *pSrcText, IN int srcTextLen, OUT MSG_LANGUAGE_ID_T *pLangId, OUT msg_encode_type_t *pCharType)
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

	if(ucs2Length < 0) {
		*pCharType = MSG_ENCODE_8BIT;

		if (srcTextLen <= maxLength) {
			memcpy(pDestText, pSrcText, srcTextLen);
			return srcTextLen;
		} else {
			memcpy(pDestText, pSrcText, maxLength);
			return maxLength;
		}
	} else {
		gsm7bitLength = convertUCS2ToGSM7bit(pDestText, maxLength, (unsigned char*)pUCS2Text, ucs2Length, pLangId, &bUnknown);

		if (bUnknown == true) {
			*pCharType = MSG_ENCODE_UCS2;

			if (ucs2Length > 0) {
				if(ucs2Length <= maxLength) {
					memcpy(pDestText, pUCS2Text, ucs2Length);
					return ucs2Length;
				} else {
					memcpy(pDestText, pUCS2Text, maxLength);
					return maxLength;
				}
			}
		} else {
			*pCharType = MSG_ENCODE_GSM7BIT;
		}

		return gsm7bitLength;
	}
}
#else

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

	if(ucs2Length < 0) {
		*pCharType = MSG_ENCODE_8BIT;

		if (srcTextLen <= maxLength) {
			memcpy(pDestText, pSrcText, srcTextLen);
			return srcTextLen;
		} else {
			memcpy(pDestText, pSrcText, maxLength);
			return maxLength;
		}
	} else {
		gsm7bitLength = convertUCS2ToASCII(pDestText, maxLength, (unsigned char*)pUCS2Text, ucs2Length, &bUnknown);

		if (bUnknown == true) {
			*pCharType = MSG_ENCODE_UCS2;

			if (ucs2Length > 0) {
				if (ucs2Length <= maxLength) {
					memcpy(pDestText, pUCS2Text, ucs2Length);
					return ucs2Length;
				} else {
					memcpy(pDestText, pUCS2Text, maxLength);
					return maxLength;
				}
			}
		} else {
			*pCharType = MSG_ENCODE_ASCII7BIT;
		}

		return gsm7bitLength;
	}
}
#endif

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
	const unsigned char *pTempSrcText = pSrcText;
#endif
	unsigned char *pTempDestText = pDestText;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
		MSG_ERR("UCS2 to UTF8 Failed as text length is 0\n");
		return false;
	}

	GIConv cd;
	int err = 0;

	cd = g_iconv_open( "UTF8", "UTF16BE" );

	if (cd > 0) {
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&srcTextLen, (char**)&pDestText, (gsize*)&remainedBuffer);
	}

	if (err != 0)
		MSG_ERR("g_iconv() return value = %d", err);

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

	if(srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
		MSG_DEBUG("EUCKR to UTF8 Failed as text length is 0\n");
		return false;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open( "UTF8", "EUCKR" );

	if (cd > 0) {
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&srcTextLen, (char**)&pDestText, (gsize*)&remainedBuffer);
	}

	MSG_DEBUG("g_iconv() return value = %d", err);

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


int MsgTextConvert::convertSHIFTJISToUTF8(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN  int srcTextLen)
{
	int remainedBuffer = maxLength;
	int utf8Length;

#ifdef CONVERT_DUMP
	int srcLen = srcTextLen;
	const unsigned char * pTempSrcText = pSrcText;
#endif
	unsigned char * pTempDestText = pDestText;

	if(srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
		MSG_DEBUG("EUCKR to UTF8 Failed as text length is 0\n");
		return false;
	}

	GIConv cd;
	int err=0;

	cd = g_iconv_open( "UTF8", "SHIFT-JIS" );

	if (cd > 0) {
		err = g_iconv(cd, (char**)&pSrcText, (gsize*)&srcTextLen, (char**)&pDestText, (gsize*)&remainedBuffer);
	}

	MSG_DEBUG("g_iconv() return value = %d", err);

	utf8Length = maxLength - remainedBuffer;
	pTempDestText[utf8Length] = 0x00;

#ifdef CONVERT_DUMP
	MSG_DEBUG("\n########## Dump SHIFT-JIS -> UTF8\n");
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

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
		MSG_DEBUG("Invalid parameter.");
		return -1;
	}

	std::map<unsigned short, unsigned char>::iterator itChar;
	std::map<unsigned short, unsigned char>::iterator itExt;
	std::map<unsigned short, unsigned char>::iterator itReplace;

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

		itExt = extCharList.find(inText);

		if (itExt != extCharList.end()) {
			newType = (MSG_CHAR_TYPE_T)itExt->second;

			if (newType >= currType) {
				if (inText == 0x00e7 && currType <= MSG_CHAR_TURKISH)
					currType = MSG_CHAR_TURKISH;
				else
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

		// Check Default Char
		itChar = ucs2toGSM7DefList.find(inText);

		if (itChar != ucs2toGSM7DefList.end()) {
			pDestText[outTextLen++] = (unsigned char)itChar->second;
		} else {
			if (currType == MSG_CHAR_GSM7EXT) {
				itExt = ucs2toGSM7ExtList.find(inText);

				if (itExt != ucs2toGSM7ExtList.end()) {
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1) {
						MSG_DEBUG("Buffer Full.");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				} else {
					itReplace = replaceCharList.find(inText);
					if (itReplace != replaceCharList.end()) {
						pDestText[outTextLen++] = (unsigned char)itReplace->second;
					} else {
						pDestText[outTextLen++] = 0x3F;
					}
					*bIncludeAbnormalChar = true;
					MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
				}
			} else if (currType == MSG_CHAR_TURKISH) {
				*pLangId = MSG_LANG_ID_TURKISH;

				itExt = ucs2toTurkishList.find(inText);

				if (itExt != ucs2toTurkishList.end()) {
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1) {
						MSG_DEBUG("Buffer Full.");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				} else {
					itReplace = replaceCharList.find(inText);
					if (itReplace != replaceCharList.end()) {
						pDestText[outTextLen++] = (unsigned char)itReplace->second;
					} else {
						pDestText[outTextLen++] = 0x3F;
					}
					*bIncludeAbnormalChar = true;
					MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
				}
			} else if (currType == MSG_CHAR_SPANISH) {
				*pLangId = MSG_LANG_ID_SPANISH;

				itExt = ucs2toSpanishList.find(inText);

				if (itExt != ucs2toSpanishList.end()) {
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1) {
						MSG_DEBUG("Buffer Full.");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				} else {
					itReplace = replaceCharList.find(inText);
					if (itReplace != replaceCharList.end()) {
						pDestText[outTextLen++] = (unsigned char)itReplace->second;
					} else {
						pDestText[outTextLen++] = 0x3F;
					}
					*bIncludeAbnormalChar = true;
					MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
				}
			} else if (currType == MSG_CHAR_PORTUGUESE) {
				*pLangId = MSG_LANG_ID_PORTUGUESE;

				itExt = ucs2toPortuList.find(inText);

				if (itExt != ucs2toPortuList.end()) {
					// prevent buffer overflow
					if (maxLength <= outTextLen + 1) {
						MSG_DEBUG("Buffer Full.");
						break;
					}

					pDestText[outTextLen++] = 0x1B;
					pDestText[outTextLen++] = (unsigned char)itExt->second;
				} else {
					itReplace = replaceCharList.find(inText);
					if (itReplace != replaceCharList.end()) {
						pDestText[outTextLen++] = (unsigned char)itReplace->second;
					} else {
						pDestText[outTextLen++] = 0x3F;
					}
					*bIncludeAbnormalChar = true;
					MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
				}
			} else {
				itReplace = replaceCharList.find(inText);
				if (itReplace != replaceCharList.end()) {
					pDestText[outTextLen++] = (unsigned char)itReplace->second;
				} else {
					pDestText[outTextLen++] = 0x3F;
				}
				*bIncludeAbnormalChar = true;
				MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
			}
		}

		// prevent buffer overflow
		if (maxLength <= outTextLen) {
			MSG_DEBUG("Buffer full.");
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


int MsgTextConvert::convertUCS2ToASCII(OUT unsigned char *pDestText, IN int maxLength, IN const unsigned char *pSrcText, IN int srcTextLen, OUT bool *pUnknown)
{
	// for UNICODE
	int outTextLen = 0;
	unsigned char lowerByte, upperByte;

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
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

		// Check Default Char
		if (inText > 0x007f) {
			MSG_DEBUG("Abnormal character is included. inText : [%04x]", inText);
			*pUnknown = true;
			return 0;
		}

		pDestText[outTextLen++] = (unsigned char)inText;

		// prevent buffer overflow
		if (maxLength <= outTextLen) {
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

	if (srcTextLen == 0 || pSrcText == NULL || pDestText ==  NULL || maxLength == 0) {
		MSG_ERR("UCS2 to GSM7bit Failed as text length is 0\n");
		return -1;
	}

	for (int i = 0; i<srcTextLen; i++)
	{
		if (maxLength == 0) {
			break;
		}

		if (pSrcText[i] >= 0x80) {
			//error
			MSG_ERR(">>>>>>>a_pTextString[i]=%x, The alpha isn't the gsm 7bit code, Never Come here!!!\n", pSrcText[i]);
			return -1;
		}

		if (pLangInfo->bLockingShift == true) { // National Language Locking Shift
			MSG_DEBUG("Locking Shift [%d]", pLangInfo->lockingLang);

			if (pLangInfo->lockingLang == MSG_LANG_ID_TURKISH) {
				// Check Escape
				if (g_TurkishLockingToUCS2[pSrcText[i]] == 0x001B) {
					i++;

					if (pLangInfo->bSingleShift == true) { // National Language Single Shift
						MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

						if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH) {
							lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH) {
							lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE) {
							lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else {
							lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
					} else { // GSM 7 bit Default Alphabet Extension Table
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				} else { // TURKISH - National Language Locking Shift
					lowerByte = g_TurkishLockingToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_TurkishLockingToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			} else if (pLangInfo->lockingLang == MSG_LANG_ID_PORTUGUESE) {
				// Check Escape
				if (g_PortuLockingToUCS2[pSrcText[i]] == 0x001B) {
					i++;

					if (pLangInfo->bSingleShift == true) { // National Language Single Shift
						MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

						if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH) {
							lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH) {
							lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE) {
							lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						} else {
							lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
							upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
						}
					} else { // GSM 7 bit Default Alphabet Extension Table
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				} else { // PORTUGUESE - National Language Locking Shift
					lowerByte = g_PortuLockingToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_PortuLockingToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			}
		} else {
			// Check Escape
			if (g_GSM7BitToUCS2[pSrcText[i]] == 0x001B) {
				i++;

				if (pLangInfo->bSingleShift == true) { // National Language Single Shift
					MSG_DEBUG("Single Shift [%d]", pLangInfo->singleLang);

					if (pLangInfo->singleLang == MSG_LANG_ID_TURKISH) {
						lowerByte = g_TurkishSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_TurkishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					} else if (pLangInfo->singleLang == MSG_LANG_ID_SPANISH) {
						lowerByte = g_SpanishSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_SpanishSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					} else if (pLangInfo->singleLang == MSG_LANG_ID_PORTUGUESE) {
						lowerByte = g_PortuSingleToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_PortuSingleToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					} else {
						lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
						upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
					}
				} else { // GSM 7 bit Default Alphabet Extension Table
					lowerByte = g_GSM7BitExtToUCS2[pSrcText[i]] & 0x00FF;
					upperByte = (g_GSM7BitExtToUCS2[pSrcText[i]] & 0xFF00) >> 8;
				}
			} else {
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
	MSG_DEBUG("=======================================");
	MSG_DEBUG("   Dump Text To Hex - Length :%d\n", length);
	MSG_DEBUG("=======================================");

	for (int i = 0; i < length; i++ )
	{
		MSG_DEBUG("[%02x]", pText[i]);
	}

	MSG_DEBUG("=======================================");
}
