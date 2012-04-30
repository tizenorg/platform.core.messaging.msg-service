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

#ifndef MSG_PLUGIN_CONFIG_H
#define MSG_PLUGIN_CONFIG_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <map>
#include <list>
#include <stdio.h>
#include <string.h>
#include "MsgDebug.h"
#include "MsgTypes.h"
#include "MsgCppTypes.h"

class MsgPlgToken
{
	int tokenType; // not defined 0, title: 1, msgtype: 2, libpath: 3
	char tokenVal[256]; // null-terminated char array

public:
	MsgPlgToken(char* pstr=NULL) : tokenType(TOK_UNDEFINED)
	{
		if(pstr)
		{
			tokenize(pstr);
		}
		else
		{
			bzero(tokenVal, 256);
		}
	}

	MsgPlgToken& operator = (const MsgPlgToken& rhs)
	{
		if (this != &rhs)
		{
			tokenType = rhs.tokenType;
			strncpy(tokenVal, rhs.tokenVal, 255);
		}

		return *this;
	}

	int getType() const { return tokenType; } // not defined 0, title: 1, msgtype: 2, libpath: 3
	const char* getVal(void) const { return tokenVal; }
	void getVal(CharVector& vec) const { vec.assign(tokenVal, tokenVal+strlen(tokenVal));}
	int tokenize(char* pStr);

	enum { TOK_UNDEFINED=0, TOK_PLG_TITLE, TOK_PLG_TYPE, TOK_PLG_PATH };

	void reset() { tokenType = TOK_UNDEFINED; }
	operator void*() const {
		return (tokenType==TOK_UNDEFINED)? NULL:(void*) this;
	}
};

typedef std::vector<MsgPlgToken> MsgPlgTokenVec;
typedef std::map<CharVector, MsgPlgTokenVec> MsgConfigMap;

class MsgPlgConfig
{
	MsgConfigMap configMap;
	void insert(const MsgPlgToken& tokTitle, const MsgPlgToken& tokMsgType, const MsgPlgToken& tokLibPath);

public:
	MsgPlgConfig(FILE* fp);

	/* access method for tokens */
	const CharVector& title(unsigned int pos);// const; // iteration with ith position i=0, .. , itemCount-1
	inline int titleCount() const { return configMap.size(); }

	void token(const CharVector& key, unsigned int pos, MsgPlgToken& retTok);// const;
	void token(int i, unsigned int pos, MsgPlgToken& retTok);// const;
	int tokenCount(const CharVector& key) { return configMap[key].size(); } // const leads to error why?
};

#endif // MSG_PLUGIN_CONFIG_H
