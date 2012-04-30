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


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
//#include <time.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
using namespace std;

#include "MsgConvertText.h"
#include "MsgTestConvert.h"
#include "main.h"

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/

#define MAX_DUMP_COLUMN	16
void MsgDumpTextToHex(const unsigned char *pText, int length)
{
	printf("\n\n=======================================\n");
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


const int MAX_TEST_BUFFER = 1000;

void MsgTestConvertLocaleToUTF8()
{
	int localeStrLen = 0;
	unsigned char localeString[MAX_TEST_BUFFER];
	int utf8StrLen;
	unsigned char utf8String[MAX_TEST_BUFFER];

	printf("Input :\n");
	cin.getline((char*)localeString, MAX_TEST_BUFFER);

	localeStrLen = strlen((char*)localeString);
	utf8StrLen = MsgConvertLocaleToUTF8(utf8String, MAX_TEST_BUFFER, localeString, localeStrLen);
	if ( utf8StrLen <= 0 )
	{
		printf("MsgConvertLocaleToUTF8() failed. src string [%s]\n", localeString);
		return;
	}

	printf("=== STRING DUMP ===\n");
	printf("src : [%s]\n", localeString);
	printf("dst : [%s]\n", utf8String);
	MsgDumpTextToHex(localeString, localeStrLen);
	MsgDumpTextToHex(utf8String, utf8StrLen);
}


/*
File Format Example
------
(total bytes)
(hexadecimal integer for 1 byte)s
------
4
4D 61 69 6E
------

*/
/**
return:
	byte length
*/
int MsgTestConvertReadTextFromFile(unsigned char *pStr)
{
	FILE *fp;
	char filename[100];
	int byteLength;
	int i;
	int c;

	printf("Input file name: \n");
	scanf("%s", filename);
	if ( (fp = fopen(filename, "r")) == NULL )
	{
		printf("File Open failed. file name [%s]\n", filename);
		return -1;
	}

	// read byte length of input text
	fscanf(fp, "%d", &byteLength);

	printf("read text from file[%s] - size:%d\n", filename, byteLength);
	// read text
	for ( i = 0; i < byteLength; i++ )
	{
		if ( fscanf(fp, "%x", &c) == EOF )
			break;
		if ( c > 0xFF )
		{
			printf("Wrong Input : index[%d] value [%x]\n", i, c);
			byteLength = -1;
			break;
		}
		pStr[i] = (char)(c & 0xFF);
		//printf("%x\n", pStr[i]);
	}

	fclose(fp);

	return byteLength;
}

void MsgTestConvertUTF8ToUCS2()
{
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];
	int ucs2StrLen = 0;
	WCHAR ucs2String[MAX_TEST_BUFFER];

	utf8StrLen = MsgTestConvertReadTextFromFile(utf8String);
	if ( utf8StrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	//ucs2StrLen = MsgConvertUTF8toUCS2(ucs2String, MAX_TEST_BUFFER, utf8String);	// for null terminated utf8 string
	ucs2StrLen = MsgConvertUTF8toUCS2((unsigned char *)ucs2String, MAX_TEST_BUFFER, utf8String, utf8StrLen);

	printf("=== STRING DUMP ===\n");
	printf("src: [%s]", utf8String);
	MsgDumpTextToHex(utf8String, utf8StrLen);
	MsgDumpTextToHex((unsigned char*)ucs2String, ucs2StrLen);

}

void MsgTestConvertUCS2ToUTF8()
{
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];
	int ucs2StrLen = 0;
	WCHAR ucs2String[MAX_TEST_BUFFER];

	ucs2StrLen = MsgTestConvertReadTextFromFile((unsigned char*)ucs2String);
	if ( ucs2StrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	utf8StrLen = MsgConvertUCS2toUTF8(utf8String, MAX_TEST_BUFFER, (unsigned char *)ucs2String, ucs2StrLen);

	printf("=== STRING DUMP ===\n");
	printf("dst: [%s]", utf8String);
	MsgDumpTextToHex((unsigned char*)ucs2String, ucs2StrLen);
	MsgDumpTextToHex(utf8String, utf8StrLen);
}

void MsgTestConvertGSM7bitToUCS2()
{
	int gsm7bitStrLen = 0;
	unsigned char gsm7bitString[MAX_TEST_BUFFER];
	int ucs2StrLen = 0;
	WCHAR ucs2String[MAX_TEST_BUFFER];

	gsm7bitStrLen = MsgTestConvertReadTextFromFile(gsm7bitString);
	if ( gsm7bitStrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	ucs2StrLen = MsgConvertGSM7bitToUCS2((unsigned char *)ucs2String, MAX_TEST_BUFFER, gsm7bitString, gsm7bitStrLen);

	printf("=== STRING DUMP ===\n");
	MsgDumpTextToHex(gsm7bitString, gsm7bitStrLen);
	MsgDumpTextToHex((unsigned char*)ucs2String, ucs2StrLen);
}

void MsgTestConvertUCS2ToGSM7bit()
{
	int ucs2StrLen = 0;
	WCHAR ucs2String[MAX_TEST_BUFFER];
	int gsm7bitStrLen = 0;
	unsigned char gsm7bitString[MAX_TEST_BUFFER];

	ucs2StrLen = MsgTestConvertReadTextFromFile((unsigned char*)ucs2String);
	if ( ucs2StrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	bool bUnknown = false;
	MSG_LANGUAGE_ID_T langId = MSG_LANG_ID_RESERVED;
	gsm7bitStrLen = MsgConvertUCS2toGSM7bit(gsm7bitString, MAX_TEST_BUFFER, (unsigned char *)ucs2String, ucs2StrLen, &bUnknown, &langId);

	printf("=== STRING DUMP ===");
	MsgDumpTextToHex((unsigned char*)ucs2String, ucs2StrLen);
	MsgDumpTextToHex(gsm7bitString, gsm7bitStrLen);
}

void MsgTestConvertGSM7BitToUTF8()
{
	int gsm7bitStrLen = 0;
	unsigned char gsm7bitString[MAX_TEST_BUFFER];
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];

	gsm7bitStrLen = MsgTestConvertReadTextFromFile(gsm7bitString);
	if ( gsm7bitStrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	memset(utf8String, 0x00, sizeof(utf8String));
	utf8StrLen = MsgConvertGSM7bitToUTF8(utf8String, MAX_TEST_BUFFER, gsm7bitString, gsm7bitStrLen);

	printf("=== STRING DUMP ===\n");
	printf("dest(UTF8) : [%s]\n", utf8String);
	MsgDumpTextToHex(gsm7bitString, gsm7bitStrLen);
	MsgDumpTextToHex(utf8String, utf8StrLen);
}

void MsgTestConvertUTF8ToGSM7bit()
{
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];
	int gsm7bitStrLen = 0;
	unsigned char gsm7bitString[MAX_TEST_BUFFER];

	utf8StrLen = MsgTestConvertReadTextFromFile(utf8String);
	if ( utf8StrLen < 0 )
	{
		printf("Error - Read Text from File\n");
		return;
	}

	MSG_LANGUAGE_ID_T langId = MSG_LANG_ID_RESERVED;
	gsm7bitStrLen = MsgConvertUTF8ToGSM7bit(gsm7bitString, MAX_TEST_BUFFER, utf8String, utf8StrLen, &langId);

	printf("=== STRING DUMP ===\n");
	MsgDumpTextToHex(utf8String, utf8StrLen);
	MsgDumpTextToHex(gsm7bitString, gsm7bitStrLen);
}

void MsgTestConvertUTF8ToUCS2Key()
{
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];
	int ucs2StrLen = 0;
	WCHAR ucs2String[MAX_TEST_BUFFER];

	printf("Input :\n");
	cin.getline((char *)utf8String, MAX_TEST_BUFFER);

	utf8StrLen = strlen((char *)utf8String);

	ucs2StrLen = MsgConvertUTF8toUCS2((unsigned char *)ucs2String, MAX_TEST_BUFFER, utf8String);	// for null terminated utf8 string
	//ucs2StrLen = MsgConvertUTF8toUCS2(ucs2String, MAX_TEST_BUFFER, utf8String, utf8StrLen);

	printf("=== STRING DUMP ===\n");
	printf("src: [%s]", utf8String);
	MsgDumpTextToHex(utf8String, utf8StrLen);
	MsgDumpTextToHex((unsigned char*)ucs2String, ucs2StrLen);

}

void MsgTestConvertUTF8ToGSM7bitKey()
{
	int utf8StrLen = 0;
	unsigned char utf8String[MAX_TEST_BUFFER];
	int gsm7bitStrLen = 0;
	unsigned char gsm7bitString[MAX_TEST_BUFFER];

	printf("Input :\n");
	cin.getline((char *)utf8String, MAX_TEST_BUFFER);

	utf8StrLen = strlen((char *)utf8String);

	MSG_LANGUAGE_ID_T langId = MSG_LANG_ID_RESERVED;
	gsm7bitStrLen = MsgConvertUTF8ToGSM7bit(gsm7bitString, MAX_TEST_BUFFER, utf8String, utf8StrLen, &langId);	// for null terminated utf8 string

	printf("=== STRING DUMP ===\n");
	MsgDumpTextToHex(utf8String, utf8StrLen);
	MsgDumpTextToHex(gsm7bitString, gsm7bitStrLen);
}


void MsgTestConvertSelectMenu( char *pMenu)
{
	int menuNum = atoi(pMenu);

	switch ( menuNum )
	{
		case 0:	// quit
			return;
		case 1:	// Locale -> UTF8
			MsgTestConvertLocaleToUTF8();
			break;
		case 2:	// UTF-8 -> UCS2
			MsgTestConvertUTF8ToUCS2();
			break;
		case 3:	// UCS2 -> UTF-8
			MsgTestConvertUCS2ToUTF8();
			break;
		case 4:	// GSM 7bit -> UCS2
			MsgTestConvertGSM7bitToUCS2();
			break;
		case 5:	// UCS2 -> GSM 7bit
			MsgTestConvertUCS2ToGSM7bit();
			break;
		case 6:	// GSM 7bit -> UTF-8
			MsgTestConvertGSM7BitToUTF8();
			break;
		case 7:	// UTF-8 -> GSM 7bit
			MsgTestConvertUTF8ToGSM7bit();
			break;
		case 8:
			MsgTestConvertUTF8ToUCS2Key();
			break;
		case 9:
			MsgTestConvertUTF8ToGSM7bitKey();
			break;
		default:
			printf("Invalid Menu. Select Again\n");
			break;
	}
}

// main function for testing converting
MSG_ERROR_T MsgTestConvertMain()
{
	char menu[3];

	do
	{
//		system("clear");

		printf("=======================\n");
		printf("    Convert Test                     \n");
		printf("=======================\n");
		printf("[1] locale -> UTF8 - (Keyboard input)\n");
		printf("[2] UTF8 -> UCS2 - (File)\n");
		printf("[3] UCS2 -> UTF8 - (File)\n");
		printf("[4] GSM 7bit -> UCS2 - (File)\n");
		printf("[5] UCS2 -> GSM 7bit - (File)\n");
		printf("[6] GSM 7bit -> UTF8 - (File)\n");
		printf("[7] UTF8 -> GSM 7bit - (File)\n");
		printf("[8] UTF8 -> UCS2 - (Keyboard)\n");
		printf("[9] UTF8 -> GSM 7bit - (Keyboard)\n");
		printf("[0] Return to Main Menu\n");
		printf("=======================\n");

		printf("Select Test Menu : ");

		memset(menu, 0x00, sizeof(menu));
		cin.getline(menu, 3);

		MsgTestConvertSelectMenu(menu);
	}
	while ( strcmp(menu, "0") != 0 );

	return 0;
}


