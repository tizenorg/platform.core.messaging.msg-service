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

#ifndef _VOBJECT_H
#define _VOBJECT_H

#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#include <dlog.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define ORG_ENABLE_TRACE
#define FEATURE_SHIFT_JIS

#ifdef ORG_ENABLE_TRACE
#define USER_TAG "MSG_FW"
#define VDATA_TRACE(fmt, arg...) \
	do\
	{\
		SLOG(LOG_DEBUG, USER_TAG, "\n[calendarui] %s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##arg); \
	} while (0)


#define VDATA_TRACE_LINE() fprintf(stderr, "\n[calendarui] %s:%d\n", __FUNCTION__, __LINE__ )

#define SysRequireEx(expr, retValue) \
	if (!(expr)) { \
		fprintf(stderr,"INVALID_PARAM (%d lines in %s)\n", __LINE__,__FILE__); \
		return retValue; }\
	else {;}
#else
#define VDATA_TRACE(fmt, arg...)
#define VDATA_TRACE_LINE()
#define SysRequireEx(expr, retValue)
#endif

#define VDATA_TRACE_BEGINE //VDATA_TRACE(">>>>>> start. >>>>>>"); //usleep(1000);
#define VDATA_TRACE_END //VDATA_TRACE(">>>>>> end. >>>>>>");

/****************************************************************************************************/
/*										 FUNCTION DECLARATION										*/
/****************************************************************************************************/
int		_VIsSpace( char );
int		_VRLSpace( char * );
int		_VRTSpace( char * );
int		_VUnescape( char* );
int		_VEscape(char*);
int		_VManySpace2Space( char * );
int		_VB64Decode( char *, char * );
int		_VB64Encode( char *, char *, int );
int		_VUnfolding( char * );
void	_VFolding( char *, char * );
int		_VQPDecode( char * );
int		_VQPEncode( char *, char * );

void _VFoldingQP( char *result, char *contentline );
void _VFoldingNoSpace( char *result, char *contentline );
int _VManyCRLF2CRLF(char *pIn);
int _VUnfoldingNoSpec( char *string, int vType );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
