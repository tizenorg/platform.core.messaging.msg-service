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

#ifndef MMS_PLUGIN_STORAGE_H
#define MMS_PLUGIN_STORAGE_H

/*==================================================================================================
							INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgSqliteWrapper.h"
#include "MmsPluginMessage.h"


/*==================================================================================================
							CLASS DEFINITIONS
==================================================================================================*/
class MmsPluginStorage
{
public:
	static MmsPluginStorage *instance();

	MmsPluginStorage();
	~MmsPluginStorage();

	void addMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	void getMmsMessage(MmsMsg **pMmsMsg);
	void getMmsAttrib(MSG_MESSAGE_ID_T msgId, MmsMsg *pMmsMsg);
	MSG_ERROR_T getMmsMessageId(MSG_MESSAGE_ID_T selectedMsgId, MmsMsg *pMmsMsg);
	void composeReadReport(MSG_MESSAGE_INFO_S *pMsgInfo);

	int searchMsgId(char *toNumber, char *szMsgID);
	int	getMmsVersion(MSG_MESSAGE_ID_T selectedMsgId);

	MSG_ERROR_T	updateMessage(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo, char *pFileData);
	MSG_ERROR_T	updateConfMessage(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T	updateMmsAttrib(MSG_MESSAGE_ID_T msgId, MmsAttrib *attrib, MSG_SUB_TYPE_T msgSubType);
	MSG_ERROR_T updateMmsAttachCount(MSG_MESSAGE_ID_T msgId, int count);
	MSG_ERROR_T	updateNetStatus(MSG_MESSAGE_ID_T msgId, MSG_NETWORK_STATUS_T netStatus);
	MSG_ERROR_T updateDeliveryReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus);
	MSG_ERROR_T	updateReadReport(MSG_MESSAGE_ID_T msgId, MmsMsgMultiStatus *pStatus);
	MSG_ERROR_T	setReadReportSendStatus(MSG_MESSAGE_ID_T msgId, int readReportSendStatus);
	MSG_ERROR_T	plgGetMmsMessage(MSG_MESSAGE_INFO_S *pMsg,  MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg);
	MSG_ERROR_T	getContentLocation(MSG_MESSAGE_INFO_S *pMsgInfo);
	MSG_ERROR_T getMmsRawFilePath(MSG_MESSAGE_ID_T msgId, char *pFilepath);
	MSG_ERROR_T	plgGetRestoreMessage(MSG_MESSAGE_INFO_S *pMsg, MSG_SENDINGOPT_INFO_S *pSendOptInfo, MMS_MESSAGE_DATA_S *pMmsMsg, char **pDestMsg, char *filePath);
	/* reject_msg_support */
	MSG_ERROR_T getTrID(MSG_MESSAGE_INFO_S *pMsgInfo, char *pszTrID, int nBufferLen);
	/* reject_msg_support */

	MSG_ERROR_T updateMsgServerID(MSG_MESSAGE_INFO_S *pMsgInfo, MSG_SENDINGOPT_INFO_S *pSendOptInfo);

	MmsMsgMultiStatus *getMultiStatus(MSG_MESSAGE_ID_T msgId);
	MSG_ERROR_T	getMsgText(MMS_MESSAGE_DATA_S *pMmsMsg, char *pMsgText);
	MSG_ERROR_T	makeThumbnail(MMS_MESSAGE_DATA_S *pMmsMsg, char *pThumbnailPath, char *szFileName);
	MSG_ERROR_T addMmsNoti(MSG_MESSAGE_INFO_S *pMsgInfo);

private:
	bool checkExistedMessage(MSG_MESSAGE_ID_T msgId);
	MSG_ERROR_T addMmsMsgToDB(MmsMsg *pMmsMsg, const MSG_MESSAGE_INFO_S *pMsgInfo, int attachCnt = 0);

	static MmsPluginStorage *pInstance;

	MsgDbHandler dbHandle;

	MmsMsg mmsMsg;
};

#endif //MMS_PLUGIN_STORAGE_H

