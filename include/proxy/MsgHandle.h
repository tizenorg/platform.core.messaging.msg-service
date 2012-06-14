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

#ifndef MSG_HANDLE_H
#define MSG_HANDLE_H

#ifndef __cplusplus
#error "This is a C++ header file; it requires C++ to compile."
#endif

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "MsgStorageTypes.h"
#include "MsgTransportTypes.h"
#include "MsgSettingTypes.h"
#include "MsgCmdTypes.h"
#include "MsgInternalTypes.h"
#include "MsgIpcSocket.h"

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/

class MsgHandle
{
	public:
		MsgHandle();
		virtual ~MsgHandle();

		// Control
        	void openHandle();
        	void closeHandle(MsgHandle* pHandle);

		// Transport
		MSG_ERROR_T submitReq(MSG_REQUEST_S* pReq);
		MSG_ERROR_T cancelReq(MSG_REQUEST_ID_T reqId);

		MSG_ERROR_T regSentStatusCallback(msg_sent_status_cb onStatusChanged,  void *pUserParam);
		MSG_ERROR_T regSmsMessageCallback(msg_sms_incoming_cb onMsgIncoming, unsigned short port, void *pUserParam);
		MSG_ERROR_T regMmsConfMessageCallback(msg_mms_conf_msg_incoming_cb onMMSConfMsgIncoming, const char *pAppId, void *pUserParam);
		MSG_ERROR_T regSyncMLMessageCallback(msg_syncml_msg_incoming_cb onSyncMLMsgIncoming, void *pUserParam);
		MSG_ERROR_T regLBSMessageCallback(msg_lbs_msg_incoming_cb onLBSMsgIncoming, void *pUserParam);

		MSG_ERROR_T regSyncMLMessageOperationCallback(msg_syncml_msg_operation_cb onSyncMLMsgOperation, void *pUserParam);

		MSG_ERROR_T operateSyncMLMessage(MSG_MESSAGE_ID_T msgId);

		// Storage
		int addMessage(const MSG_MESSAGE_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		MSG_ERROR_T addSyncMLMessage(const MSG_SYNCML_MESSAGE_S *pSyncMLMsg);
		MSG_ERROR_T updateMessage(const MSG_MESSAGE_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		MSG_ERROR_T updateReadStatus(MSG_MESSAGE_ID_T MsgId, bool bRead);
		MSG_ERROR_T updateProtectedStatus(MSG_MESSAGE_ID_T MsgId, bool bProtected);
		MSG_ERROR_T deleteMessage(MSG_MESSAGE_ID_T MsgId);
		MSG_ERROR_T deleteAllMessagesInFolder(MSG_FOLDER_ID_T FolderId, bool bOnlyDB);
		MSG_ERROR_T moveMessageToFolder(MSG_MESSAGE_ID_T MsgId, MSG_FOLDER_ID_T DestFolderId);
		MSG_ERROR_T moveMessageToStorage(MSG_MESSAGE_ID_T MsgId, MSG_STORAGE_ID_T DestStorageId);
		MSG_ERROR_T countMessage(MSG_FOLDER_ID_T FolderId, MSG_COUNT_INFO_S *pCountInfo);
		MSG_ERROR_T countMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount);
		MSG_ERROR_T countMsgByContact(const MSG_THREAD_LIST_INDEX_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList);
		MSG_ERROR_T getMessage(MSG_MESSAGE_ID_T MsgId, MSG_MESSAGE_S *pMsg, MSG_SENDINGOPT_S *pSendOpt);
		MSG_ERROR_T getFolderViewList(MSG_FOLDER_ID_T FolderId, const MSG_SORT_RULE_S *pSortRule, MSG_LIST_S *pMsgFolderViewList);
		MSG_ERROR_T addFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		MSG_ERROR_T updateFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		MSG_ERROR_T deleteFolder(MSG_FOLDER_ID_T FolderId);
		MSG_ERROR_T getFolderList(MSG_FOLDER_LIST_S *pFolderList);

		MSG_ERROR_T getThreadViewList(const MSG_SORT_RULE_S *pSortRule, MSG_THREAD_VIEW_LIST_S *pThreadViewList);
		MSG_ERROR_T getConversationViewList(MSG_THREAD_ID_T ThreadId, MSG_LIST_S *pConvViewList);
		MSG_ERROR_T deleteThreadMessageList(MSG_THREAD_ID_T thread_id);
		MSG_ERROR_T getQuickPanelData(MSG_QUICKPANEL_TYPE_T Type, MSG_MESSAGE_S *pMsg);
		MSG_ERROR_T resetDatabase();
		MSG_ERROR_T getMemSize(unsigned int* memsize);

		// Setting
		MSG_ERROR_T setConfig(const MSG_SETTING_S *pSetting);
		MSG_ERROR_T getConfig(MSG_SETTING_S *pSetting);

		// ETC
		MSG_ERROR_T searchMessage(const char *pSearchString, MSG_THREAD_VIEW_LIST_S *pThreadViewList);
		MSG_ERROR_T searchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, MSG_LIST_S *pMsgList);
		MSG_ERROR_T getMsgIdList(MSG_REFERENCE_ID_T RefId, MSG_MSGID_LIST_S *pMsgIdList);
		MSG_ERROR_T getRejectMsgList(const char *pNumber, MSG_REJECT_MSG_LIST_S *pRejectMsgList);
		MSG_ERROR_T regStorageChangeCallback(msg_storage_change_cb onStorageChange, void *pUserParam);
		MSG_ERROR_T getReportStatus(MSG_MESSAGE_ID_T msg_id, MSG_REPORT_STATUS_INFO_S *pReport_status);

		void convertMsgStruct(const MSG_MESSAGE_INFO_S *pSource, MSG_MESSAGE_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_INFO_S* pSrc, MSG_SENDINGOPT_S* pDest, MSG_MESSAGE_TYPE_S msgType);

	private:
		void connectSocket();
		void	disconnectSocket();
        	void write(const char *pCmd, int CmdSize, char **ppEvent);
		void read(char **ppEvent);
        	void generateConnectionId(char *ConnectionId);
		void convertMsgStruct(const MSG_MESSAGE_S *pSource, MSG_MESSAGE_INFO_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_S* pSrc, MSG_SENDINGOPT_INFO_S* pDest, MSG_MESSAGE_TYPE_S msgType);
		int getSettingCmdSize(MSG_OPTION_TYPE_T optionType);

		char		mConnectionId[20];
		short	mCounter;

		char 				mCookie[MAX_COOKIE_LEN];

		MsgIpcClientSocket	mClientSock;
};

#endif // MSG_HANDLE_H

