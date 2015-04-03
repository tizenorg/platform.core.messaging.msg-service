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

#ifndef MSG_MMS_SERIALIZE_H
#define MSG_MMS_SERIALIZE_H

#include "MsgMmsTypes.h"

int MsgSerializeMmsJsonData(const MMS_DATA_S *pMsgData, char **pValue);
int MsgDeserializeMmsJsonData(char* value, int value_len, MMS_DATA_S **ppMmsData);

int MsgSerializeMms(const MMS_DATA_S *pMsgData, char **pValue);
int MsgDeserializeMmsData(char* value, int value_len, MMS_DATA_S **ppMmsData);

#endif //MSG_MMS_SERIALIZE_H

