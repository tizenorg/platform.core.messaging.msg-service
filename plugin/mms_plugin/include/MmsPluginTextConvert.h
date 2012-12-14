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

#ifndef MMS_PLUGIN_TEXT_CONVERT_H
#define MMS_PLUGIN_TEXT_CONVERT_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *MmsPluginTextConvertGetCharSet(int MIBEnum);

bool MmsPluginTextConvert(const char *pToCodeSet, const char *pFromCodeSet, const char *pSrc, int srcLen, char **ppDest, int *pDestLen);

#ifdef __cplusplus
}
#endif

#endif
