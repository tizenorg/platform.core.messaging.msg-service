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
#include "MmsPluginDebug.h"
#include "MmsPluginMIME.h"
#include "MmsPluginCodec.h"
#include "MmsPluginTextConvert.h"
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
	MSG_BEGIN();
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

	if (strcasecmp("utf-16", pFromCodeset) == 0) {//check utf-8 str though utf-16

		MSG_DEBUG("Codeset [%s] check utf-8 type", pFromCodeset);

		if (((UINT8)pSrc[0] == 0xFF && (UINT8)pSrc[1] == 0xFE) || ((UINT8)pSrc[0] == 0xFE && (UINT8)pSrc[1] == 0xFF)) {

			char *pTemp = (char *)calloc(1, srcLen + 1);
			memcpy(pTemp, pSrc, srcLen);

			for (int i = 0; i < srcLen - 1; i++) {
				if ((UINT8)pTemp[i] == 0xFF && (UINT8)pTemp[i+1] == 0xFD) {
					MSG_DEBUG("exist 0xFFFD convert to 0x5F5F");
					pTemp[i] = 0x5F;
					pTemp[i+1] = 0x5F;
				}
			}

			if (MmsIsUtf8String((unsigned char *)pTemp+2, srcLen-2) == true) {
				MSG_DEBUG("UTF-8 string");
				pDest = (char *)calloc(1, srcLen - 2 + 1);
				strncpy(pDest, pTemp + 2, srcLen - 2);
				bytes_written = srcLen - 2;

				if (pTemp) {
					free(pTemp);
					pTemp = NULL;
				}

				goto __RETURN;
			}

			if (pTemp) {
				free(pTemp);
				pTemp = NULL;
			}

		}
	}

	pDest = g_convert (pSrc, srcLen,
			pToCodeSet, pFromCodeset,
	       &bytes_read, &bytes_written,
	       &error);

	if (error != NULL) {
		MSG_DEBUG("Error in g_convert, GError = %d:%s, pSrc = %s\n", error->code, error->message, pSrc);
		goto __CATCH;
	}

	if (pDest == NULL || bytes_written == 0 || bytes_read == 0) {
		MSG_DEBUG("Error in g_convert, pDest = %s, bytes_written = %d, bytes_read = %d\n", pDest, bytes_written, bytes_read);
		goto __CATCH;
	}

__RETURN:
	*ppDest = pDest;
	*pDestLen = bytes_written;

	MSG_END();
	return true;

__CATCH:
	if (pDest)
		g_free(pDest);

	MSG_END();
	return false;
}
