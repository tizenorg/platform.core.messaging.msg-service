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

#include <VCalendar.h>
#include <VCard.h>
#include <VTypes.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MT_CHECK_EQUAL(arg1, arg2) arg1 != arg2 ? \
  fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d NOT EQUAL",__FILE__,__FUNCTION__,__LINE__) :\
  fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d EQUAL",__FILE__,__FUNCTION__,__LINE__);

#define MT_CHECK_NOT_EQUAL(arg1, arg2) arg1 != arg2 ? \
  fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d NOT EQUAL",__FILE__,__FUNCTION__,__LINE__) :\
  fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d EQUAL",__FILE__,__FUNCTION__,__LINE__);

#define VDATA_TEST_TRACE(fmt, arg...) \
  do { {fprintf(stderr, "[VOBJECT-TEST-CASE] %s %s:%d: " fmt "\n" ,__FILE__,__FUNCTION__, __LINE__, ##arg);}}while (false);

#define null NULL

/* */
void
utc_vdata_vcard_encode_0() {

	FILE* fp;
	char* buffer;
	char* temp;
	VTree *tree = NULL;
	VObject*	pTemp = NULL;
	VParam*		pTmpParam = NULL;

	struct stat stat_object;
	VDATA_TEST_TRACE("START");

	if(lstat("sample.vcf", &stat_object) != 0) {
		VDATA_TEST_TRACE("error get file status..");
		return ;
	}

	if( (buffer = (char*) malloc(stat_object.st_size)) == NULL )
		return ;

	VDATA_TEST_TRACE("sample.vcf file size : %d", stat_object.st_size);

	if( (fp = fopen("sample.vcf",  "r")) == NULL )
		return ;
	fread(buffer, stat_object.st_size, sizeof(char), fp);
	fclose(fp);

	VDATA_TEST_TRACE("~~~~~");

	tree = VCardDecode(buffer);

	pTemp = tree->pTop;
	pTmpParam = pTemp->pParam;

	while(1) {
		//VDATA_TEST_TRACE("%d %d",pTemp->property, VCARD_TYPE_PHOTO);
		temp = pTemp->pszValue[0];
		VDATA_TEST_TRACE("%s",temp);

		if (pTemp->pSibling != NULL) {
			pTemp = pTemp->pSibling;
			pTmpParam = pTemp->pParam;
		}
		else {
			break;
		}
	}

	VDATA_TEST_TRACE("END");


	return ;

}


/*encode a tree to a buffer*/
void utc_vdata_vcard_encode_1()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);
	tree->pTop->property = 32;

	char *temp = g_strdup("2.2");;
	tree->pTop->pszValue[0] = temp;

	if(buffer != NULL) {
	g_free(buffer);
	buffer = NULL;
	}

	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));
	buffer = VCardEncode(tree);

	VCalFreeVTreeMemory(tree);
	tree = VCardDecode(buffer);

	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_NOT_EQUAL((gchar *)NULL, buffer);

	if(buffer != NULL)
		g_free(buffer);

	if(temp != NULL)
		g_free(temp);

	buffer = NULL;
	temp = NULL;
}
/*encode a tree with tree type as vcalendar to a buffer*/
void utc_vdata_vcard_encode_2()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");
	tree = VCardDecode(buffer);

	tree->treeType = VCALENDAR;

	if(buffer != NULL)
	g_free(buffer);
	buffer = NULL;

	buffer = VCardEncode(tree);
	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_EQUAL((gchar *)NULL, buffer);
	VCalFreeVTreeMemory(tree);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

/*encode a tree with all the vobjects as NULL to a buffer*/
void utc_vdata_vcard_encode_3()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	tree = g_new0(VTree, 1);
	tree->treeType = VCARD;
	tree->pTop = NULL;
	tree->pCur = NULL;

	buffer = VCardEncode(tree);
	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_NOT_EQUAL((gchar *)NULL, buffer);
	VCalFreeVTreeMemory(tree);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

/*encode a tree with an object having property as very high value to a buffer*/
void utc_vdata_vcard_encode_4()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");
	tree = VCardDecode(buffer);

	tree->pTop->property = 65;

	if(buffer != NULL)
	g_free(buffer);

	buffer = NULL;

	buffer = VCardEncode(tree);

	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_NOT_EQUAL((gchar*)NULL, buffer);

	VCalFreeVTreeMemory(tree);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

/*encode a NULL tree to a buffer*/
void utc_vdata_vcard_encode_5()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = VCardEncode(tree);
	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_EQUAL((gchar *)NULL, buffer);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

/*encode a tree with an object having property but no other value to a buffer*/
void utc_vdata_vcard_encode_6()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");
	tree = VCardDecode(buffer);

	tree->pTop->property = 14;
	tree->pTop->pszValue[0] = NULL;

	if(buffer != NULL)
	g_free(buffer);

	buffer = NULL;

	buffer = VCardEncode(tree);
	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_NOT_EQUAL((gchar *)NULL, buffer);
	VCalFreeVTreeMemory(tree);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

/*encode a tree with an object having value count greater than actual vales to a buffer*/
void utc_vdata_vcard_encode_7()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;
	gchar *temp;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");
	tree = VCardDecode(buffer);

	tree->pTop->property = VCARD_TYPE_N;
	tree->pTop->valueCount = 5;
	temp = g_strdup( "amit");
	tree->pTop->pszValue[0] = temp;


	if(buffer != NULL)
	g_free(buffer);

	buffer = NULL;

	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));
	buffer = VCardEncode(tree);

	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_NOT_EQUAL((gchar *)NULL, buffer);
	VCalFreeVTreeMemory(tree);


	if(buffer != NULL)
		g_free(buffer);

	if(temp != NULL)
		g_free(temp);

	buffer = NULL;
	temp = NULL;
}

/*encode a tree with tree type as a high value to a buffer*/
void utc_vdata_vcard_encode_8()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);

	tree->treeType = 15;

	if(buffer != NULL)
	g_free(buffer);

	buffer = NULL;

	buffer = VCardEncode(tree);
	VDATA_TEST_TRACE ("call executed buffer is %s\n", buffer);
	MT_CHECK_EQUAL((gchar*)NULL, buffer);
	VCalFreeVTreeMemory(tree);

	if(buffer != NULL)
		g_free(buffer);

	buffer = NULL;
}

