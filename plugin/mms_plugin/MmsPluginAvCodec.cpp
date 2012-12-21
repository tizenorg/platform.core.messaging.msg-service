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

#include <string.h>
#include "MmsPluginAvCodec.h"

AvCodecType	AvGetFileCodecType(const char *szFileName)
{
	char szFileExt[FILEEXT_MAX] = {0, };
	int iPos = 0;
	int iStrLen = 0;
	bool bMIME = false;

	iStrLen = strlen(szFileName);

	iPos = iStrLen;

	while (iPos > 0) {
		iPos--;

		if (szFileName[iPos] == '.')
			break;
		if (szFileName[iPos] == '/') {
			bMIME = true;
			break;
		}
	}

	if (iPos == 0) {
		return AV_CODEC_NONE;
	}

	if (FILEEXT_MAX > (iStrLen - iPos - 1)) {
		strncpy(szFileExt, szFileName + iPos + 1, (iStrLen - iPos - 1));
		szFileExt[iStrLen - iPos - 1] = '\0';
	} else
		return AV_CODEC_NONE;

	if (strcasecmp(szFileExt, "mp3") == 0 || strcasecmp(szFileExt, "MP3") == 0)
		return AV_DEC_AUDIO_MP3;

	if (strcasecmp(szFileExt, "pvx") == 0 || strcasecmp(szFileExt, "PVX") == 0)
		return AV_DEC_VIDEO_PVX;
	else if (strcasecmp(szFileExt, "wav") == 0 || strcasecmp(szFileExt, "WAV") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidWAVE(szFileName) == true)
				return AV_DEC_AUDIO_WAVE;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_WAVE;
	} else if (strcasecmp(szFileExt, "aac") == 0 || strcasecmp(szFileExt, "AAC") == 0)
		return AV_DEC_AUDIO_AAC;
#ifdef AV_DEBUG_MODE
	else if (strcasecmp(szFileExt, "wma") == 0 || strcasecmp(szFileExt, "WMA") == 0)
		return AvWMFFGetFileType(szFileName);
	else if (strcasecmp(szFileExt, "wmv") == 0 || strcasecmp(szFileExt, "WMV") == 0)
		return AvWMFFGetFileType(szFileName);
	else if (strcasecmp(szFileExt, "asf") == 0 || strcasecmp(szFileExt, "ASF") == 0)
		return AvWMFFGetFileType(szFileName);
#endif
	else if (strcasecmp(szFileExt, "amr") == 0 || strcasecmp(szFileExt, "AMR") == 0 ||
			strcasecmp(szFileExt, "x-amr") == 0 || strcasecmp(szFileExt, "X-AMR") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidAMR(szFileName) == true)
				return AV_DEC_AUDIO_AMR;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_AMR;
	} else if (strcasecmp(szFileExt, "g7231") == 0 || strcasecmp(szFileExt, "G7231") == 0)
		return AV_DEC_AUDIO_G723_1;
	else if ((strcasecmp(szFileExt, "mid") == 0 || strcasecmp(szFileExt, "MID") == 0) ||
			(strcasecmp(szFileExt, "midi") == 0 || strcasecmp(szFileExt, "MIDI") == 0)) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidMIDI(szFileName) == true)
				return AV_DEC_AUDIO_MIDI;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_MIDI;
	} else if (strcasecmp(szFileExt, "mmf") == 0 || strcasecmp(szFileExt, "MMF") == 0 || strcasecmp(szFileExt, "x-mmf") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidMMF(szFileName) == true)
				return AV_DEC_AUDIO_MMF;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_MMF;
	} else if (strcasecmp(szFileExt, "pmd") == 0 || strcasecmp(szFileExt, "PMD") == 0) {
			return AV_DEC_AUDIO_MIDI;
	} else if (strcasecmp(szFileExt, "xmf") == 0 || strcasecmp(szFileExt, "XMF") == 0 ||
				strcasecmp(szFileExt,  "mxmf") == 0 || strcasecmp(szFileExt,  "MXMF") == 0) {
		return AV_DEC_AUDIO_XMF;
	} else if (strcasecmp(szFileExt, "smp") == 0 || strcasecmp(szFileExt, "SMP") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidSMP(szFileName) == true)
				return AV_DEC_AUDIO_ADPCM;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_ADPCM;
	} else if (strcasecmp(szFileExt, "spm") == 0 || strcasecmp(szFileExt, "SPM") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidMIDI(szFileName) == true)
				return AV_DEC_AUDIO_SPM;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_SPM;
	} else if (strcasecmp(szFileExt, "spf") == 0 || strcasecmp(szFileExt, "SPF") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidMMF(szFileName) == true)
				return AV_DEC_AUDIO_MMF_PHRASE_L1;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_MMF_PHRASE_L1;
	} else if (strcasecmp(szFileExt, "imy") == 0 || strcasecmp(szFileExt, "IMY") == 0 ||
		strcasecmp(szFileExt, "iMelody") == 0 || strcasecmp(szFileExt, "x-iMelody") == 0 ||
		strcasecmp(szFileExt, "Melody") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			if (__AvValidIMELODY(szFileName) == true)
				return AV_DEC_AUDIO_IMELODY;
			else
				return AV_CODEC_NONE;
		} else
#endif
			return AV_DEC_AUDIO_IMELODY;
	} else if (strcasecmp(szFileExt, "dtmf") == 0)
		return AV_DEC_AUDIO_DTMF_TONE;
	else if (strcasecmp(szFileExt, "h263") == 0 || strcasecmp(szFileExt, "H263") == 0)
		return AV_DEC_VIDEO_H263;
	else if (strcasecmp(szFileExt, "mp4") == 0 || strcasecmp(szFileExt, "MP4") == 0 ||
			strcasecmp(szFileExt, "mpeg4") == 0 || strcasecmp(szFileExt, "MPEG4") == 0 ||
			strcasecmp(szFileExt, "m4a") == 0 || strcasecmp(szFileExt, "M4A") == 0) {
#ifdef AV_DEBUG_MODE
		if (!bMIME) {
			AvCodecType avCodecType = AV_CODEC_NONE;
			FmFileAttribute fileAttrib;
			size_t strLen;
			UINT32	dateTime = 0;

			if (!DrmGetFileAttributes(szFileName, &fileAttrib)) {
				return AV_CODEC_NONE;
			} else {
				dateTime =  TmDateTimeToSeconds(&fileAttrib.dt);
				strLen = strlen(szFileName);
				if ((strnicmp(lastAccessFileInfo.szFileName, szFileName, strLen) == 0) &&
					(lastAccessFileInfo.fileSize == fileAttrib.fileSize) &&
					(lastAccessFileInfo.dateTime == dateTime)) {
					return lastAccessFileInfo.codecType;
				} else {
					avCodecType = AvMP4FFGetFileType(szFileName);

					if (strcasecmp(szFileExt, "3ga") == 0 || strcasecmp(szFileExt, "3GA") == 0 ||
						strcasecmp(szFileExt, "m4a") == 0 || strcasecmp(szFileExt, "M4A") == 0) {
						if (avCodecType | AV_DEC_AUDIO_MPEG4)
							avCodecType = AV_DEC_AUDIO_MPEG4;
					}

					if (avCodecType != AV_CODEC_NONE) {
						strncpy(lastAccessFileInfo.szFileName, szFileName, strLen);
						lastAccessFileInfo.szFileName[strLen] = '\0';
						lastAccessFileInfo.fileSize = fileAttrib.fileSize;
						lastAccessFileInfo.dateTime = dateTime;
						lastAccessFileInfo.codecType = avCodecType;
					}
					return avCodecType;
				}
			}
		}
#endif
		return AV_DEC_VIDEO_MPEG4;
	} else if (strcasecmp(szFileExt, "3gp") == 0 || strcasecmp(szFileExt, "3GP") == 0 ||
		strcasecmp(szFileExt, "3ga") == 0 || strcasecmp(szFileExt, "3GA") == 0 ||
		strcasecmp(szFileExt, "3gpp") == 0 || strcasecmp(szFileExt, "3GPP") == 0) {
		return AV_CODEC_NONE;
	} else if (strcasecmp(szFileExt, "sdp") == 0 || strcasecmp(szFileExt, "SDP") == 0)
		return AV_DEC_VIDEO_MPEG4;
	else
		return AV_CODEC_NONE;
}
