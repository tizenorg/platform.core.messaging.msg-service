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

#include "MsgPluginConfig.h"
#include "MsgException.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgPlgToken - Member Functions
==================================================================================================*/
int MsgPlgToken::tokenize(char *pStr)
{
	MSG_BEGIN();

	if (pStr == NULL)
		THROW(MsgException::INVALID_PARAM, "pstr is NULL");

	char *pFirst = index(pStr, '[');

	if (pFirst) // title is enclosed by [ ]
	{
		pFirst++; // excluding '['

		char *pEnd = index(pStr, ']');

		if (pEnd)
		{
			pEnd--;

			tokenType = TOK_PLG_TITLE;
			int len = pEnd-pFirst+1;

			memcpy(tokenVal, pFirst, len);
			tokenVal[len] = '\0';

			MSG_DEBUG("%s", tokenVal);
		}
		else
			THROW(MsgException::INVALID_RESULT, "']' not found");
	}
	else // (attribute, value) pair
	{
		char tokStr[strlen(pStr)+1];
		strncpy(tokStr, pStr, strlen(pStr));

		char *attr = strtok(tokStr, "=");
		char *val = strtok(NULL, "=");

		if (!attr || !val)
		{
			tokenType = TOK_UNDEFINED;
		}
		else
		{
			// trimming enter key
			char *tmp = index(val,'\n');

			if (tmp)
				*tmp = '\0';

			MSG_DEBUG("attr \"%s\", val \"%s\" ", attr, val);

			// classifying the attribute
			if (strcmp(attr, "type") == 0)
			{
				tokenType = TOK_PLG_TYPE;
				strncpy(tokenVal, val, 255);
			}
			else if (strcmp(attr, "path") == 0)
			{
				tokenType = TOK_PLG_PATH;
				strncpy(tokenVal, val, 255);
			}
			else
			{
				tokenType = TOK_UNDEFINED;
			}
		}
	}

	MSG_END();

	return tokenType;
}


/*==================================================================================================
                                     IMPLEMENTATION OF MsgPlgConfig - Member Functions
==================================================================================================*/
MsgPlgConfig::MsgPlgConfig(FILE* fp)
{
	MSG_BEGIN();

	if (fp == NULL)
		THROW(MsgException::INVALID_PARAM, "fp is NULL");

	MsgPlgToken tokTitle, tokMsgType, tokLibPath;

	char str[256];
	memset(str, 0x00, sizeof(str));

	MsgPlgToken tok = MsgPlgToken();

	while (fgets(str, 255, fp))
	{
		tok.tokenize(str); // parsing the line into tokens

		switch ( tok.getType() )
		{
			case MsgPlgToken::TOK_PLG_TITLE:
				// processing previous items
				if( tokTitle && tokMsgType && tokLibPath )
				{
					insert(tokTitle, tokMsgType, tokLibPath);
				}

				tokTitle = tok;
				tokMsgType.reset();
				tokLibPath.reset();

				break;

			case MsgPlgToken::TOK_PLG_TYPE:
				tokMsgType = tok;
				break;

			case MsgPlgToken::TOK_PLG_PATH:
				tokLibPath = tok;
				break;

			default:
				MSG_DEBUG("the line \"%s\" is not accecpted", str);
				break;
		}
	}

	if (tokTitle && tokMsgType && tokLibPath)
	{
		insert(tokTitle, tokMsgType, tokLibPath);
	}

	MSG_END();
}


void MsgPlgConfig::insert(const MsgPlgToken& tokTitle, const MsgPlgToken& tokMsgType, const MsgPlgToken& tokLibPath)
{
	MSG_BEGIN();

	MsgPlgTokenVec item2add;
	item2add.push_back(tokMsgType);
	item2add.push_back(tokLibPath);

	CharVector titleVec;
	tokTitle.getVal(titleVec);
	MsgConfigMap::iterator it=configMap.find(titleVec);

	if (it == configMap.end())
		configMap.insert(std::make_pair(titleVec, item2add));
	else
		THROW(MsgException::PLUGIN_ERROR, "duplicated plugin title");

	MSG_DEBUG("item inserted");
	MSG_DEBUG("token:%d,value:%s", tokTitle.getType(), tokTitle.getVal());
	MSG_DEBUG("token:%d,value:%s", tokMsgType.getType(), tokMsgType.getVal());
	MSG_DEBUG("token:%d,value:%s", tokLibPath.getType(), tokLibPath.getVal());

	MSG_END();
}


const CharVector& MsgPlgConfig::title(unsigned int pos)
{
	if (pos >= configMap.size() || pos < 0)
		THROW(MsgException::OUT_OF_RANGE, "Input Parameter is not valid [%d]", pos);

	MsgConfigMap::iterator it = configMap.begin();

	unsigned int i=0;

	while (it != configMap.end())
	{
		if (i++ == pos) break;
		it++;
	}

	if (it == configMap.end())
		THROW(MsgException::INVALID_RESULT, "no title");

	MSG_DEBUG("searched title:%s", &(it->first)[0]);

	return it->first;
}


void MsgPlgConfig::token(const CharVector& key, unsigned int pos, MsgPlgToken& retTok)
{
	MsgConfigMap::iterator it = configMap.find(key);

	if (it != configMap.end()) //found
	{
		MSG_DEBUG("searched title:%s", &(it->first)[0]);

		MsgPlgTokenVec tokVec = it->second;
		retTok = tokVec[pos];

		MSG_DEBUG("searched token:%d,value:%s", retTok.getType(), retTok.getVal());
	}
	else
	{
		THROW(MsgException::INVALID_RESULT, "no title");
	}
}


void MsgPlgConfig::token(int i, unsigned int pos, MsgPlgToken& retTok)
{
	const CharVector& key = title(i);

	token(key, pos, retTok);

	MSG_DEBUG("returned token:%d,value:%s", retTok.getType(), retTok.getVal());
}

