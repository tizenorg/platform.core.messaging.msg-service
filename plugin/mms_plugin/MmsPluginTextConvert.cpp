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
#include <glib.h>
#include "MmsPluginMIME.h"
#include "MmsPluginCodec.h"
#include "MmsPluginTextConvert.h"
#include "MsgDebug.h"
#include "MmsPluginUtil.h"

const char *MmsPluginTextConvertGetCharSet(int MIBEnum)
{
	const char * result_str = NULL;

	result_str = MmsGetTextByCode(MmsCodeCharSet, (unsigned short int)MIBEnum);
	MSG_DEBUG("[MIBEnum = %d, Charset = %s]", MIBEnum, result_str);

	return result_str;
}

bool MmsPluginTextConvert(const char *pToCodeSet, const char *pFromCodeset, const char *pSrc, int srcLen, char **ppDest, int *pDestLen)
{
	char *pDest = NULL;
	gsize bytes_read = 0;
	gsize bytes_written = 0;
	GError *error = NULL;

	if (pToCodeSet == NULL || pFromCodeset == NULL) {
		MSG_DEBUG("Error input parameter Codeset to = %s, from = %s", pToCodeSet, pFromCodeset);
		goto __CATCH;
	}

	MSG_DEBUG("Codeset to = %s, from = %s", pToCodeSet, pFromCodeset);

	if (pSrc == NULL || ppDest == NULL || pDestLen == NULL) {
		MSG_DEBUG("Error input parameter pSrc = %p, ppDest = %p, pDestLen = %p", pSrc, ppDest, pDestLen);
		goto __CATCH;
	}

	pDest = g_convert (pSrc, srcLen,
			pToCodeSet, pFromCodeset,
	       &bytes_read, &bytes_written,
	       &error);

	if (error != NULL) {
		printf("Error in g_convert, GError = %d:%s, pSrc = %s\n", error->code, error->message, pSrc);
		goto __CATCH;
	}

	if (pDest == NULL || bytes_written == 0 || bytes_read == 0) {
		printf("Error in g_convert, pDest = %s, bytes_written = %d, bytes_read = %d\n", pDest, bytes_written, bytes_read);
		goto __CATCH;
	}

	*ppDest = pDest;
	*pDestLen = bytes_written;

	return true;

__CATCH:
	if (pDest)
		g_free(pDest);

	return false;
}
