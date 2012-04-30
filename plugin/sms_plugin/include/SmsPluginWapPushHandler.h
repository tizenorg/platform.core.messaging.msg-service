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

