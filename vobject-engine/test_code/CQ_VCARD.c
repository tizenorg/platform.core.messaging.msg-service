/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>

extern void utc_vdata_vcard_decode_1();
extern void utc_vdata_vcard_decode_2();
extern void utc_vdata_vcard_decode_3();
extern void utc_vdata_vcard_decode_4();
extern void utc_vdata_vcard_decode_5();
extern void utc_vdata_vcard_decode_6();
extern void utc_vdata_vcard_decode_7();
extern void utc_vdata_vcard_encode_1();
extern void utc_vdata_vcard_encode_0();
extern void utc_vdata_vcard_encode_2();
extern void utc_vdata_vcard_encode_3();
extern void utc_vdata_vcard_encode_4();
extern void utc_vdata_vcard_encode_5();
extern void utc_vdata_vcard_encode_6();
extern void utc_vdata_vcard_encode_7();
extern void utc_vdata_vcard_encode_8();

int CQ_TEST() {
#if 0
	printf("\n################################### VCard CQ Test  ##################################\n");
	printf("[TEST CASE] utc_vdata_vcard_decode_1 : decode a raw buffer to a tree.\n");
	utc_vdata_vcard_decode_1();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_2 :  decode a raw buffer without begin token to a tree.\n");
	utc_vdata_vcard_decode_2();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_3 : decode a raw buffer without end token to a tree.\n");
	utc_vdata_vcard_decode_3();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_4 : decode a raw buffer having invalid version tag to a tree.\n");
	utc_vdata_vcard_decode_4();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_5 : decode a raw buffer without having crlf in between two tokens to a tree.\n");
	utc_vdata_vcard_decode_5();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_6 : decode a raw buffer having some invalid token to a tree.\n");
	utc_vdata_vcard_decode_6();
	printf("\n\n[TEST CASE] utc_vdata_vcard_decode_7 : decode a NULL raw buffer to a tree.\n");
	utc_vdata_vcard_decode_7();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_0 : \n");
	utc_vdata_vcard_encode_0();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_1 : encode a tree to a buffer.\n");
	utc_vdata_vcard_encode_1();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_2 : encode a tree with tree type as vcalendar to a buffer.\n");
	utc_vdata_vcard_encode_2();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_3 : encode a tree with all the vobjects as NULL to a buffer.\n");
	utc_vdata_vcard_encode_3();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_4 : encode a tree with an object having property as very high value to a buffer.\n");
	utc_vdata_vcard_encode_4();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_5 : encode a NULL tree to a buffer.\n");
	utc_vdata_vcard_encode_5();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_6 : encode a tree with an object having property but no other value to a buffer.\n");
	utc_vdata_vcard_encode_6();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_7 : encode a tree with an object having value count greater than actual vales to a buffer.\n");
	utc_vdata_vcard_encode_7();
	printf("\n\n[TEST CASE] utc_vdata_vcard_encode_8 : encode a tree with tree type as a high value to a buffer.\n");
	utc_vdata_vcard_encode_8();
	printf("\n################################### VCard CQ Test  ##################################\n\n");
#endif

	return 0;
}

int main(int argc, char** argv) {

	CQ_TEST();
}
