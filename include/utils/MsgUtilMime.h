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

#ifndef MSG_UTIL_MIME_H
#define MSG_UTIL_MIME_H

#include "msg_types.h"

#define UNDEFINED_BINARY	0xFF

typedef enum _MimeMainType {
	MIME_MAINTYPE_UNKNOWN = -1,
	MIME_MAINTYPE_APPLICATION,
	MIME_MAINTYPE_AUDIO,
	MIME_MAINTYPE_IMAGE,
	MIME_MAINTYPE_MESSAGE,
	MIME_MAINTYPE_MULTIPART,
	MIME_MAINTYPE_TEXT,
	MIME_MAINTYPE_VIDEO,
	MIME_MAINTYPE_THEME,
	MIME_MAINTYPE_ETC
} MimeMainType;

char *MimeGetExtFromMimeInt(MimeType mime);

MimeMainType MimeGetMainTypeInt(MimeType mime);

MimeType MimeGetMimeIntFromBi(int binCode);

int MimeGetBinaryValueFromMimeInt(MimeType mime);

MimeType MimeGetMimeIntFromMimeString(char *szMimeStr);

char *MimeGetMimeStringFromMimeInt(int mimeType);

bool MsgGetMimeTypeFromExt(MimeMainType mainType, const char *pExt, MimeType *pMimeType, const char **ppszMimeType);

bool MsgGetMimeTypeFromFileName(MimeMainType mainType, const char *pFileName, MimeType *pMimeType, const char **ppszMimeType);

bool MsgGetMainTypeFromMetaData(const char *pFileName, MimeMainType *mainType);

#endif /* MSG_UTIL_MIME_H */
