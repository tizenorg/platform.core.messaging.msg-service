/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org
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

	void addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	void getMmsMessage(MmsMsg **pMmsMsg);
	void getMmsAttrib(msg_message_id_t msgId, MmsMsg *pMmsMsg);
	msg_error_t getMmsMessageId(msg_message_id_t selectedMsgId, MmsMsg *pMmsMsg);
	void composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

	int searchMsgId(char *toNumber, char *szMsgID);
	int getMmsVersion(msg_message_id_t selectedMsgId);

	msg_error_t updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	msg_error_t updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t updateMmsAttrib(msg_message_id_t msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType);
	msg_error_t updateMmsAttachCount(msg_message_id_t msgId, int count);
	msg_error_t updateNetStatus(msg_message_id_t msgId, msg_network_status_t netStatus);

	msg_error_t insertDeliveryReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus);
	msg_error_t insertReadReport(msg_message_id_t msgId, char *address, MmsMsgMultiStatus *pStatus);
	msg_error_t insertPreviewInfo(int msgId, int type, char *value, int count = 0);

	msg_error_t setReadReportSendStatus(msg_message_id_t msgId, int readReportSendStatus);
	msg_error_t plgGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg,  MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg);
	msg_error_t getContentLocation(MSG_MESSAGE_INFO_S *pMsgInfo);
	msg_error_t getMmsRawFilePath(msg_message_id_t msgId, char *pFilepath);

	/* reject_msg_support */
	msg_error_t getTrID(MSG_MESSAGE_INFO_S *pMsgInfo, char *pszTrID, int nBufferLen);
	/* reject_msg_support */
	msg_error_t getAddressInfo(msg_message_id_t msgId, MSG_ADDRESS_INFO_S *pAddrInfo);
	msg_error_t updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo);

	msg_error_t getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText);

private:
	msg_error_t addMmsMsgToDB(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, int attachCnt = 0);

	static MmsPluginStorage *pInstance;

	MsgDbHandler dbHandle;

	MmsMsg mmsMsg;
};

#endif //MMS_PLUGIN_STORAGE_H

