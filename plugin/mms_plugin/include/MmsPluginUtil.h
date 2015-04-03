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

#ifndef MMS_PLUGIN_UTIL_H
#define MMS_PLUGIN_UTIL_H

#include <stdio.h>

//File Util
bool MmsMakeImageThumbnail(char *srcPath, char *dstPath);

bool MmsMakeVideoThumbnail(char *srcPath, char *dstPath);

FILE *MmsFileOpen(char *pFileName);

//Text Util
bool MmsIsAsciiString(const char *szSrc);

bool MmsIsUtf8String(const unsigned char *szSrc, int nChar);

void MmsRemoveLessGreaterChar(const char *szSrc, char *szDest, int destSize);

bool MmsRemoveMarkup(const char *szSrc, char *szDest, int dstsize);

bool MmsReplaceSpaceChar(char *pszText);

char *MmsReplaceNonAsciiUtf8(const char *szSrc, char replaceChar);

#endif //MMS_PLUGIN_UTIL_H
