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

