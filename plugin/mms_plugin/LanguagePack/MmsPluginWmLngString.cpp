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

#include "MsgDebug.h"
#include "MmsPluginWmLngPack.h"


/**
 * return length of a string (character count of a string)
 * @param	mszInText [in] 	input string pointer
 * @return	lenght of string
 *
 */
int WmStrlen(const MCHAR *mszInText)
{
	register int n;

	n=0;

	while(*(mszInText+n) != '\0')
		n++;

	return n;
}

