/*
*
* Copyright (c) 2000-2012 Samsung Electronics Co., Ltd. All Rights Reserved.
*
* This file is part of msg-service.
*
* Contact: Jaeyun Jeong <jyjeong@samsung.com>
*          Sangkoo Kim <sangkoo.kim@samsung.com>
*          Seunghwan Lee <sh.cat.lee@samsung.com>
*          SoonMin Jung <sm0415.jung@samsung.com>
*          Jae-Young Lee <jy4710.lee@samsung.com>
*          KeeBum Kim <keebum.kim@samsung.com>
*
* PROPRIETARY/CONFIDENTIAL
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered
* into with SAMSUNG ELECTRONICS.
*
* SAMSUNG make no representations or warranties about the suitability
* of the software, either express or implied, including but not limited
* to the implied warranties of merchantability, fitness for a particular
* purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or
* distributing this software or its derivatives.
*
*/

#include "MsgTypes.h"

#define	FILEEXT_MAX	5

typedef enum {
	AV_CODEC_NONE						= 0x00000000,
	AV_CODEC_AUDIO_REAL					= 0x00000001,
	AV_CODEC_AUDIO_AMR					= 0x00000002,		// AMR
	AV_CODEC_AUDIO_G723_1				= 0x00000004,		// G723.1
	AV_CODEC_AUDIO_MP3					= 0x00000008,		// MP3
	AV_CODEC_AUDIO_AAC					= 0x00000010,		// AAC
	AV_CODEC_AUDIO_MIDI					= 0x00000020,		// MIDI
	AV_CODEC_AUDIO_MMF					= 0x00000040,		// MMF
	AV_CODEC_AUDIO_MMF_PHRASE_L1		= 0x00000080,		// YAMAHA Phrase L1
	AV_CODEC_AUDIO_ADPCM				= 0x00000100,		// ADPCM
	AV_CODEC_AUDIO_WAVE					= 0x00000200,		// WAVE
	AV_CODEC_AUDIO_IMELODY				= 0x00000400,		// IMELODY
	AV_CODEC_AUDIO_DTMF_TONE			= 0x00000800,		// DTMF
	AV_CODEC_AUDIO_DTMF_TONE_ID			= 0x00001000,
	AV_CODEC_AUDIO_DTMF_TONE_FREQ		= 0x00002000,
	AV_CODEC_AUDIO_DTMF_SOUND_ID		= 0x00004000,
	AV_CODEC_AUDIO_DTMF_TONE_FLEX		= 0x00008000,		// DTMF
	AV_CODEC_AUDIO_MPEG4				= 0x00010000,		// MPEG4 audio
	AV_CODEC_AUDIO_SPM					= 0x00020000,		// SPM(SP-MIDI)
	AV_CODEC_AUDIO_PCM					= 0x00040000,
	AV_CODEC_AUDIO_XMF					= 0x00080000,		// XMF
	AV_CODEC_VIDEO_H263					= 0x00400000,		// H.26L
	AV_CODEC_VIDEO_MPEG4				= 0x02000000,		// MPEG4 video
	AV_CODEC_VIDEO_AVI					= 0x04000000,		// MPEG4 video
	AV_CODEC_VIDEO_MPEG1				= 0x08000000,		// MPEG4 video
	AV_CODEC_VIDEO_REAL					= 0x10000000,
	AV_CODEC_VIDEO_PVX					= 0x20000000,            // for Fast Track .PVX type

	AV_ENC_AUDIO_AMR					= AV_CODEC_AUDIO_AMR,		// AMR
	AV_ENC_AUDIO_G723_1					= AV_CODEC_AUDIO_G723_1,	// G723.1
	AV_ENC_AUDIO_MP3					= AV_CODEC_AUDIO_MP3,		// MP3
	AV_ENC_AUDIO_AAC					= AV_CODEC_AUDIO_AAC,		// AAC
	AV_ENC_AUDIO_ADPCM					= AV_CODEC_AUDIO_ADPCM,		// ADPCM
	AV_ENC_AUDIO_WAVE					= AV_CODEC_AUDIO_WAVE,		// WAVE
	AV_ENC_VIDEO_H263					= AV_CODEC_VIDEO_H263,		// H.263
	AV_ENC_VIDEO_MPEG4					= AV_CODEC_VIDEO_MPEG4,		// MPEG4
	AV_ENC_VIDEO_AVI					= AV_CODEC_VIDEO_AVI,		// AVI
	AV_ENC_VIDEO_MPEG1					= AV_CODEC_VIDEO_MPEG1,		// MPEG1

	AV_DEC_AUDIO_DTMF_TONE_FLEX			= AV_CODEC_AUDIO_DTMF_TONE_FLEX,	// DTMF
	AV_DEC_AUDIO_AMR					= AV_CODEC_AUDIO_AMR,		// AMR
	AV_DEC_AUDIO_G723_1					= AV_CODEC_AUDIO_G723_1,	// G723.1
	AV_DEC_AUDIO_MP3					= AV_CODEC_AUDIO_MP3,		// MP3
	AV_DEC_AUDIO_AAC					= AV_CODEC_AUDIO_AAC,		// AAC
	AV_DEC_AUDIO_MIDI					= AV_CODEC_AUDIO_MIDI,		// MIDI
	AV_DEC_AUDIO_MMF					= AV_CODEC_AUDIO_MMF,		// MMF
	AV_DEC_AUDIO_MMF_PHRASE_L1			= AV_CODEC_AUDIO_MMF_PHRASE_L1,		// YAMAHA Phrase L1
	AV_DEC_AUDIO_ADPCM					= AV_CODEC_AUDIO_ADPCM,		// ADPCM
	AV_DEC_AUDIO_WAVE					= AV_CODEC_AUDIO_WAVE,		// WAVE
	AV_DEC_AUDIO_IMELODY				= AV_CODEC_AUDIO_IMELODY,		// IMELODY
	AV_DEC_AUDIO_DTMF_TONE				= AV_CODEC_AUDIO_DTMF_TONE,		// DTMF
	AV_DEC_AUDIO_DTMF_TONE_ID			= AV_CODEC_AUDIO_DTMF_TONE_ID,
	AV_DEC_AUDIO_DTMF_TONE_FREQ			= AV_CODEC_AUDIO_DTMF_TONE_FREQ,
	AV_DEC_AUDIO_DTMF_SOUND_ID			= AV_CODEC_AUDIO_DTMF_SOUND_ID,
	AV_DEC_AUDIO_MPEG4					= AV_CODEC_AUDIO_MPEG4,		// MPEG4 audio
	AV_DEC_AUDIO_SPM					= AV_CODEC_AUDIO_SPM,		// SPM(SP-MIDI)
	AV_DEC_AUDIO_XMF					= AV_CODEC_AUDIO_XMF,		// XMF
	AV_DEC_AUDIO_REAL					= AV_CODEC_AUDIO_REAL,		// MPEG4 video

	AV_DEC_VIDEO_H263					= AV_CODEC_VIDEO_H263,		// H.26L
	AV_DEC_VIDEO_MPEG4					= AV_CODEC_VIDEO_MPEG4,		// MPEG4 video
	AV_DEC_VIDEO_AVI					= AV_CODEC_VIDEO_AVI,		// AVI
	AV_DEC_VIDEO_MPEG1					= AV_CODEC_VIDEO_MPEG1,		// MPEG1

	AV_DEC_VIDEO_REAL					= AV_CODEC_VIDEO_REAL,			// REAL VIDEO DEC

	AV_DEC_VIDEO_PVX					= AV_CODEC_VIDEO_PVX,                //for .PVX

} AvCodecType;

AvCodecType	AvGetFileCodecType(const char *szFileName);
