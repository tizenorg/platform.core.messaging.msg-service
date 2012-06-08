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

#ifndef SMS_PLUGIN_WAPPUSH_HANDLER_H
#define SMS_PLUGIN_WAPPUSH_HANDLER_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <wbxml/wbxml.h>
#include <libxml/parser.h>


#include "SmsPluginTypes.h"


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define WSP_STANDARD_STR_LEN_MAX        255
#define LENGTH_QUOTE  0x1F
#define	NO_VALUE						0x00

#define WSP_CODE_BUFFER_LEFT_LEN_MAX	1024
#define WSP_CODE_BUFFER_RIGHT_LEN_MAX	2048

#define  MemFree(x)  {if(x != NULL) free(x);x=NULL;}
#define  AcStrlen(x) ((x==NULL)?0:strlen(x))
#define MIN(a,b)  (((a)  <  (b)) ? (a)  :  (b))


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginWapPushHandler
{
public:
	static SmsPluginWapPushHandler* instance();

	//void registerPushCallback();
	bool IsWapPushMsg(SMS_USERDATA_S *pUserData);

	void copyDeliverData(SMS_DELIVER_S *pDeliver);
	void handleWapPushMsg(const char *pUserData, int DataSize);
	void handleWapPushCallback(char* pPushHeader, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen, char* pWspBody, int WspBodyLen);

private:
	SmsPluginWapPushHandler();
	virtual ~SmsPluginWapPushHandler();

	static SmsPluginWapPushHandler* pInstance;

	SMS_WAP_APP_CODE_T getAppCode(const char *pPushHeader);

	void handleMMSNotification(const char *pPushBody, int PushBodyLen);
	void handleSIMessage(char* pPushBody, int PushBodyLen, bool isText);
	void handleSLMessage(char* pPushBody, int PushBodyLen, bool isText);
	void handleCOMessage(char* pPushBody, int PushBodyLen, bool isText);
	void handleDrmVer1(char* pPushBody, int PushBodyLen);
	void getXmlDoc(const char* pPushBody, const int PushBodyLen, xmlDocPtr *pXmlDoc, const bool isText);
	void createMsgInfo(MSG_MESSAGE_INFO_S* pMsgInfo);
	unsigned long convertXmlCharToSec(char* pDate);
	MSG_PUSH_ACTION_T convertSIActionStrToEnum(char* pAction);
	MSG_PUSH_ACTION_T convertSLActionStrToEnum(char* pAction);

	unsigned long wspRetriveUintvarDecode( unsigned char* sourceData, unsigned long* currentPointer );
	unsigned long wspDecodeUintvar(unsigned long length, unsigned char* userVar );
	void wspDecodeHeader( unsigned char* sEncodedHeader, unsigned long encodedHeaderLen, unsigned long contentsLength, bool fContentType, char** pHeader);
	unsigned long wspHeaderDecodeInteger( unsigned char* data );
	void wspHeaderDecodeQValue( unsigned long length, unsigned char* data, char** pDecodedString);
	unsigned long wspHeaderDecodeIntegerByLength(unsigned char* data, unsigned long length );
	char* wspExtendedDecodeType(char contentType  );
	void wspHeaderDecodeParameter( unsigned char* data, unsigned long length, char** pParam);
	void wspHeaderDecodeCharset( unsigned long length, unsigned char* data, char**pDecodedString);
	void wspHeaderDecodeVersion( unsigned long length, unsigned char* data, char** pDecodedString );
	void wspHeaderDecodeDateValue( unsigned long length, unsigned char* data, char** pDecodedString );
	void wspHeaderCopyDecodedString( unsigned char* szDecodedString, unsigned long* currentLen, char** pTemper );
	void wspHeaderDecodeAuth(unsigned long fieldValueLen, unsigned char* fieldValue, char** pDecodedString );
	void wspHeaderDecodeChallenge(unsigned long fieldValueLen, unsigned char* fieldValue, char** pDecodedString );
	void wspHeaderDecodeCacheControl(unsigned char* fieldValue, unsigned long fieldValueLen, char** pCacheString);


	SMS_ADDRESS_S	tmpAddress;
	SMS_TIMESTAMP_S	tmpTimeStamp;
};

#endif //SmsPluginWapPushHandler

