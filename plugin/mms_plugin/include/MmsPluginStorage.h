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

#ifndef MMS_PLUGIN_STORAGE_H
#define MMS_PLUGIN_STORAGE_H

#include "MsgSqliteWrapper.h"
#include "MmsPluginCodecTypes.h"

class MmsPluginStorage
{
public:
	static MmsPluginStorage *instance();

	MmsPluginStorage();
	~MmsPluginStorage();

	//MMS message operation
	msg_error_t addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pSerializedMms);
	msg_error_t getMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char **pSerializedMms);
	msg_error_t updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pSerializedMms);
	msg_error_t updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo);//malware allowed

	msg_error_t deleteMmsMessage(int msgId);

	//MMS message preview info
	msg_error_t insertPreviewInfo(int msgId, int type, const char *value, int count = 0);
	msg_error_t removePreviewInfo(int msgId);

	//MMS message multipart list
	msg_error_t insertMultipart(msg_message_id_t msgId, MMS_MULTIPART_DATA_S *pMultipart);
	msg_error_t updateMultipart(msg_message_id_t msgId, int allow_malware, MMS_MULTIPART_DATA_S *pMultipart);
	msg_error_t getMultipartList(msg_message_id_t msgId, MMSList **multipart_list);
	msg_error_t deleteMultipartList(int msgId);

	//MMS message report
	msg_error_t insertDeliveryReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus);
	msg_error_t insertReadReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus);

	//etc
	void getMmsMessage(MmsMsg **pMmsMsg);
	msg_error_t getMmsMessageId(msg_message_id_t selectedMsgId, MmsMsg *pMmsMsg);
	void composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

	int searchMsgId(char *toNumber, char *szMsgID);
	int getMmsVersion(msg_message_id_t selectedMsgId);

	msg_error_t updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t updateMmsAttrib(msg_message_id_t msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType);
	msg_error_t updateMmsAttachCount(msg_message_id_t msgId, int count);

	msg_error_t getMmsRawFilePath(msg_message_id_t msgId, char *pFilepath, size_t filePathLen);

	msg_error_t getTrID(MSG_MESSAGE_INFO_S *pMsgInfo, char *pszTrID, int nBufferLen); /* reject_msg_support */

	msg_error_t getAddressInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo);

	msg_error_t updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo);

	msg_error_t getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText);

	int checkDuplicateNotification(char* pszTrID, char* pszContentLocation);

private:
	void getMmsFromDB(msg_message_id_t msgId, MmsMsg *pMmsMsg);
	msg_error_t addMmsMsgToDB(MmsMsg *pMmsMsg, const char *raw_filepath);
	msg_error_t addMmsData(msg_message_id_t msgId, const char *raw_filepath, MMS_DATA_S *pMmsData);
	msg_error_t getMmsData(msg_message_id_t msgId, MMS_DATA_S *pMmsData);

	msg_error_t updateRetriveConf(msg_message_id_t msgId, MMS_DATA_S *pMmsData);

	static MmsPluginStorage *pInstance;

	MmsMsg mmsMsg;
};

#endif //MMS_PLUGIN_STORAGE_H

