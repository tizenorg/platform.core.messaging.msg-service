/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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

#include <VCard.h>
#include <VTypes.h>
#include <glib.h>

#define MT_CHECK_EQUAL(arg1, arg2) arg1 != arg2 ? \
	fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d NOT EQUAL\n",__FILE__,__FUNCTION__,__LINE__) :\
	fprintf(stderr,"[VOBJECT-TEST-CASE] %s %s:%d EQUAL\n",__FILE__,__FUNCTION__,__LINE__);

#define VDATA_TEST_TRACE(fmt, arg...) \
	do { {fprintf(stderr, "[VOBJECT-TEST-CASE] %s %s:%d: " fmt,__FILE__,__FUNCTION__, __LINE__, ##arg);}}while (false);


/** decode a raw buffer to a tree */
void utc_vdata_vcard_decode_1()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE("call executed \n");

	if(!(tree != NULL)) return;

	VDATA_TEST_TRACE("treetype is %d \n", tree->treeType);

	if (!(tree->pTop)){
		VDATA_TEST_TRACE("decodeing faili - utc_vdata_vcard_decode_%d",1);
		return;
	}
	VDATA_TEST_TRACE ("top property is %d \n", tree->pTop->property);
	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));

	if (tree->pTop->pParam){
		VDATA_TEST_TRACE ("top param is %d \n", tree->pTop->pParam->parameter);
		VDATA_TEST_TRACE ("top param val is %d \n", tree->pTop->pParam->paramValue);
	}

	if (tree->pCur){
		VDATA_TEST_TRACE ("cur property is %d \n", tree->pCur->property);
		VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pCur->pszValue));
	}

	if (!(tree->pTop->pSibling)){
		VDATA_TEST_TRACE("decodeing faili - utc_vdata_vcard_decode_%d",1);
		return;
	}

	VDATA_TEST_TRACE ("sibling not NULL \n");
	VDATA_TEST_TRACE ("sibling property is %d \n", tree->pTop->pSibling->property);
	VDATA_TEST_TRACE ("sibling property value is %s \n", *(tree->pTop->pSibling->pszValue));

	if (tree->pTop->pSibling->pParam){
		VDATA_TEST_TRACE ("sibling param is %d \n", tree->pTop->pSibling->pParam->parameter);
		VDATA_TEST_TRACE ("sibling param val is %d \n", tree->pTop->pSibling->pParam->paramValue);
	}

	VDATA_TEST_TRACE("decodeing success - utc_vdata_vcard_decode_%d\n",1);

	VObject* pSibling2 = tree->pTop->pSibling->pSibling;
	if (!pSibling2)
		return;

	VDATA_TEST_TRACE ("sibling2 not NULL \n");
	VDATA_TEST_TRACE ("sibling2 property is %d \n", pSibling2->property);
	VDATA_TEST_TRACE ("sibling2 property value is %s \n", *(pSibling2->pszValue));

	if (pSibling2->pParam){
		VDATA_TEST_TRACE ("sibling param is %d \n", pSibling2->pParam->parameter);
		VDATA_TEST_TRACE ("sibling param val is %d \n", pSibling2->pParam->paramValue);
	}

	if (tree->pTop->pParent)
		VDATA_TEST_TRACE ("parent not NULL \n");
	if (tree->pTop->pChild)
		VDATA_TEST_TRACE ("child not NULL \n");


	if(buffer != NULL)
		g_free(buffer);
}

/** decode a raw buffer without begin token to a tree */
void utc_vdata_vcard_decode_2()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("VERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE("call executed \n");

	if(!(tree != NULL)) VDATA_TEST_TRACE("tree is null");

	if(buffer != NULL)
		g_free(buffer);
}

/*decode a raw buffer without end token to a tree*/
void utc_vdata_vcard_decode_3()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531");

	VDATA_TEST_TRACE("%s\n", buffer);
	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE("call executed \n");

	if(!tree) return;

	VDATA_TEST_TRACE ("treetype is %d \n", tree->treeType);


	if(buffer != NULL)
		g_free(buffer);
}
/*decode a raw buffer having invalid version tag to a tree*/
void utc_vdata_vcard_decode_4()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVE:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);

	VDATA_TEST_TRACE ("call executed \n");
	VDATA_TEST_TRACE ("treetype is %d \n", tree->treeType);

	if (!(tree->pTop)){
		VDATA_TEST_TRACE ("tree->pTop null. \n");
		return;
	}

	VDATA_TEST_TRACE ("top property is %d \n", tree->pTop->property);
	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));

	MT_CHECK_EQUAL(tree->pTop->property, (int)VCARD_TYPE_N);


	if(buffer != NULL)
		g_free(buffer);
}

/** decode a raw buffer without having crlf in between two tokens to a tree */
void utc_vdata_vcard_decode_5()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARDVERSION:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE ("call executed \n");
	VDATA_TEST_TRACE ("treetype is %d \n", tree->treeType);

	if (!(tree->pTop)){
		VDATA_TEST_TRACE ("tree->pTop null. \n");
		return;
	}

	VDATA_TEST_TRACE ("top property is %d \n", tree->pTop->property);
	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));

	MT_CHECK_EQUAL(tree->pTop->property, (int)VCARD_TYPE_N);

	if(buffer != NULL)
		g_free(buffer);
}

/*decode a raw buffer having some invalid token to a tree*/
void utc_vdata_vcard_decode_6()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	buffer = g_strdup("BEGIN:VCARD\r\nVERSIOOOONNNN:2.1\r\nN:Martin;Stephen\r\nTEL;HOME;VOICE:+1 (210) 555-1357\r\nTEL;HOME;FAX:+1 (210) 555-0864\r\nADR;WORK;PARCEL;POSTAL;DOM:123 Cliff Ave.;Big Town;CA;97531\r\nEND:VCARD");

	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE ("call executed \n");
	VDATA_TEST_TRACE ("treetype is %d \n", tree->treeType);

	if (!(tree->pTop)){
		VDATA_TEST_TRACE ("tree->pTop null. \n");
		return;
	}

	VDATA_TEST_TRACE ("top property is %d \n", tree->pTop->property);
	VDATA_TEST_TRACE ("top property value is %s \n", *(tree->pTop->pszValue));

	MT_CHECK_EQUAL(tree->pTop->property, (int)VCARD_TYPE_N);

	if(buffer != NULL)
		g_free(buffer);
}

/*decode a NULL raw buffer to a tree*/
void utc_vdata_vcard_decode_7()
{
	gchar *buffer = NULL;
	VTree *tree = NULL;

	tree = VCardDecode(buffer);
	VDATA_TEST_TRACE ("call executed \n");

	MT_CHECK_EQUAL((VTree*)NULL, tree);

	if(buffer != NULL)
		g_free(buffer);
}
