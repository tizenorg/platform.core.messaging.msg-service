/*
 * msg-service
 *
 * Copyright (c) 2000 - 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef MMS_PLUGIN_UTIL_H
#define MMS_PLUGIN_UTIL_H

#include <stdio.h>

bool makeImageThumbnail(char *srcPath, char *dstPath);
bool makeVideoThumbnail(char *srcPath, char *dstPath);

bool MsgIsASCII(char *pszText);
bool MsgReplaceNonAscii(char *szInText, char **szOutText, char replaceChar);
bool MsgIsSpace(char *pszText);
bool MsgReplaceSpecialChar(char *szInText, char **szOutText, char specialChar);
char *MsgStrAppend(char *szInputStr1, char *szInputStr2);
char *MsgStrCopy(const char *string);
char *MsgStrNCopy(const char *string, int length);
int	MsgStrlen(char *pStr);
bool MsgConvertCharToHex(char pSrc, char *pDest);
FILE *MmsFileOpen(char *pFileName);
void removeLessGreaterMark(const char *szSrcID, char *szDest, int destSize);
#endif //MMS_PLUGIN_UTIL_H
