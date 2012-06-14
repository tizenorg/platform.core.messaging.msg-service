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

#ifndef _MMS_PLUGIN_WM_LNG_PACK_H_
#define _MMS_PLUGIN_WM_LNG_PACK_H_

#include "MmsPluginMessage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define LOCALCODE_BYTE_MAX	3

#define WmSizeof(size)		\
		sizeof(size)/2

/* Code Convert */
bool  WmConvert2PCode( MCHAR* pmszOutText, int outBufSize, char* szInText);
bool  WmConvert2LCode( char* pszOutText, int outBufSize, MCHAR* mszInText);
bool  WmConvert2PCodeN( MCHAR* pmszOutText, int outBufSize, char*  szInText, int byteCount );
bool  WmConvert2LCodeN( char* pszOutText, int outBufSize, MCHAR* mszInText, int charCount );
bool  WmConvertPCode2UTF( UCHAR *pszOutText, int outBufSize, MCHAR *mszInText, int charCount );
bool  WmConvertUTF2PCode( MCHAR *pmszOutText,int outBufSize, UCHAR *szInText, int byteCount );
int   WmGetLCodeSize( MCHAR * mszText );
int   WmGetLCodeSizeN( MCHAR *mszText, int charCount );
bool  WmConvertLatinCode2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);
bool  WmConvertLatinCode2PCodeN( MCHAR* pmszOutText, int outBufSize, char*  szInText, int charCount );


/* latin2 <-> PCode */
bool	WmConvertLatin2Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* latin3 <-> PCode */
bool	WmConvertLatin3Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* latin4 <-> PCode */
bool	WmConvertLatin4Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* latin8 <-> PCode */
bool	WmConvertLatin8Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* latin15 <-> PCode */
bool	WmConvertLatin15Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* latin5 <-> PCode */
bool	WmConvertLatin5Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* win1251 <-> PCode */
bool	WmConvertWin1251Code2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* Koi8-r <-> PCode */
bool	WmConvertKoi8rCode2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);

/* Koi8-u <-> PCode */
bool	WmConvertKoi8uCode2PCode( MCHAR* pmszOutText, int outBufSize, char*  szInText);


/* String Handling */
int	WmStrlen( const MCHAR* mszInText );
int	WmStrncmp( const MCHAR* mszInText1, const MCHAR* mszInText2, UINT charCount );


int WmGetLatin32UTFCodeSize( unsigned char* szSrc, int nChar ); //ISO 8859-3
int WmGetLatin42UTFCodeSize( unsigned char* szSrc, int nChar ); //ISO 8859-4
int WmGetLatin82UTFCodeSize( unsigned char* szSrc, int nChar ); //ISO 8859-8
int WmGetLatin152UTFCodeSize( unsigned char* szSrc, int nChar ); //ISO 8859-15
int WmGetLatin52UTFCodeSize( unsigned char* szSrc, int nChar ); //ISO 8859-9


bool __WmConvertCodeBufferSizeCheck( char* ftnName, int outBufSize, int requiredBufSize );
bool _WmT9ChangeUniToGSMCode( MCHAR* pmszOutText, MCHAR* mszInText, int length );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WM_LNG_PACK_H_ */


