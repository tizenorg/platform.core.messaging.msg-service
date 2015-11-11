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

#ifndef MMS_PLUGIN_APPBASE_H
#define MMS_PLUGIN_APPBASE_H

#include "MmsPluginTypes.h"

/* Set Data for Message App */
class MmsPluginAppBase {
public:
	MmsPluginAppBase();
	MmsPluginAppBase(MMS_DATA_S *pMsgData);
	MmsPluginAppBase(MmsMsg *pMmsMsg);
	~MmsPluginAppBase();

	void setMmsData(MMS_DATA_S *pMmsData);
	void setMmsData(MmsMsg *pMmsMsg);

	void makePreviewInfo(msg_message_id_t  msgId, bool allow_malware, const char *raw_filepath);

	void getFirstPageTextFilePath(char *textBuf, int textBufSize);
private:
	MMS_MESSAGE_DATA_S mmsMsgData;
};
#endif /* MMS_PLUGIN_APPBASE_H */
