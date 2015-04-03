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

#ifndef MSG_SMIL_H
#define MSG_SMIL_H

#include "MsgMmsTypes.h"

/**	@fn	bool MsgSmilParseSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, const char *pSmilDoc)
 *	@brief	Parse Smil Document using MMS_MESSAGE_DATA_S struct.
 *	@param[in]	pstMsgMmsBody
 *	@param[in]	pSmilDoc
 *	@retval	TRUE	In case of Success.
 *	@retval	FALSE	In case of failure.
 */
bool MsgSmilParseSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, const char *pSmilDoc);

/**	@fn	bool MsgSmilGenerateSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, char **ppSmilDoc)
 *	@brief	Generate Smil Document using MMS_MESSAGE_DATA_S struct.
 *	@param[in]	pstMsgMmsBody
 *	@param[out]	ppSmilDoc
 *	@retval	TRUE	In case of Success.
 *	@retval	FALSE	In case of failure.
 */
bool MsgSmilGenerateSmilDoc(MMS_MESSAGE_DATA_S *pstMsgMmsBody, char **ppSmilDoc);

#endif//MSG_SMIL_H
