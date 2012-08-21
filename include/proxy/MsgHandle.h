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
#include "MsgFilterTypes.h"
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
		msg_error_t submitReq(MSG_REQUEST_S* pReq);
		msg_error_t cancelReq(msg_request_id_t reqId);

		msg_error_t regSentStatusCallback(msg_sent_status_cb onStatusChanged,  void *pUserParam);
		msg_error_t regSmsMessageCallback(msg_sms_incoming_cb onMsgIncoming, unsigned short port, void *pUserParam);
		msg_error_t regMmsConfMessageCallback(msg_mms_conf_msg_incoming_cb onMMSConfMsgIncoming, const char *pAppId, void *pUserParam);
		msg_error_t regSyncMLMessageCallback(msg_syncml_msg_incoming_cb onSyncMLMsgIncoming, void *pUserParam);
		msg_error_t regLBSMessageCallback(msg_lbs_msg_incoming_cb onLBSMsgIncoming, void *pUserParam);

		msg_error_t regSyncMLMessageOperationCallback(msg_syncml_msg_operation_cb onSyncMLMsgOperation, void *pUserParam);

		msg_error_t operateSyncMLMessage(msg_message_id_t msgId);

		// Storage
		int addMessage(MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t addSyncMLMessage(const MSG_SYNCML_MESSAGE_S *pSyncMLMsg);
		msg_error_t updateMessage(const MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t updateReadStatus(msg_message_id_t MsgId, bool bRead);
		msg_error_t updateProtectedStatus(msg_message_id_t MsgId, bool bProtected);
		msg_error_t deleteMessage(msg_message_id_t MsgId);
		msg_error_t deleteAllMessagesInFolder(msg_folder_id_t FolderId, bool bOnlyDB);
		msg_error_t moveMessageToFolder(msg_message_id_t MsgId, msg_folder_id_t DestFolderId);
		msg_error_t moveMessageToStorage(msg_message_id_t MsgId, msg_storage_id_t DestStorageId);
		msg_error_t countMessage(msg_folder_id_t FolderId, MSG_COUNT_INFO_S *pCountInfo);
		msg_error_t countMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount);
		msg_error_t countMsgByContact(const MSG_THREAD_LIST_INDEX_INFO_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList);
		msg_error_t getMessage(msg_message_id_t MsgId, MSG_MESSAGE_HIDDEN_S *pMsg, MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t getFolderViewList(msg_folder_id_t FolderId, const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pMsgFolderViewList);
		msg_error_t addFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		msg_error_t updateFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		msg_error_t deleteFolder(msg_folder_id_t FolderId);
		msg_error_t getFolderList(msg_struct_list_s *pFolderList);

		msg_error_t getThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList);
		msg_error_t getConversationViewList(msg_thread_id_t ThreadId, msg_struct_list_s *pConvViewList);
		msg_error_t deleteThreadMessageList(msg_thread_id_t thread_id);
		msg_error_t getQuickPanelData(msg_quickpanel_type_t Type, MSG_MESSAGE_HIDDEN_S *pMsg);
		msg_error_t resetDatabase();
		msg_error_t getMemSize(unsigned int* memsize);

		msg_error_t getAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList);

		// Filter
		msg_error_t addFilter(const MSG_FILTER_S *pFilter);
		msg_error_t updateFilter(const MSG_FILTER_S *pFilter);
		msg_error_t deleteFilter(msg_filter_id_t FilterId);
		msg_error_t getFilterList(msg_struct_list_s *pFilterList);
		msg_error_t setFilterOperation(bool bSetFlag);
		msg_error_t getFilterOperation(bool *pSetFlag);

		//setting
		msg_error_t getSMSCOption(msg_struct_t msg_struct);
		msg_error_t setSMSCOption(msg_struct_t msg_struct);
		msg_error_t getCBOption(msg_struct_t msg_struct);
		msg_error_t setCBOption(msg_struct_t msg_struct);
		msg_error_t getSmsSendOpt(msg_struct_t msg_struct);
		msg_error_t setSmsSendOpt(msg_struct_t msg_struct);
		msg_error_t getMmsSendOpt(msg_struct_t msg_struct);
		msg_error_t setMmsSendOpt(msg_struct_t msg_struct);
		msg_error_t getMmsRecvOpt(msg_struct_t msg_struct);
		msg_error_t setMmsRecvOpt(msg_struct_t msg_struct);;
		msg_error_t getPushMsgOpt(msg_struct_t msg_struct);
		msg_error_t setPushMsgOpt(msg_struct_t msg_struct);
		msg_error_t getVoiceMsgOpt(msg_struct_t msg_struct);
		msg_error_t setVoiceMsgOpt(msg_struct_t msg_struct);
		msg_error_t getGeneralOpt(msg_struct_t msg_struct);
		msg_error_t setGeneralOpt(msg_struct_t msg_struct);
		msg_error_t getMsgSizeOpt(msg_struct_t msg_struct);
		msg_error_t setMsgSizeOpt(msg_struct_t msg_struct);

		//Backup & Restore
		msg_error_t backupMessage();
		msg_error_t restoreMessage();

		// ETC
		msg_error_t searchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList);
		msg_error_t searchMessage(const MSG_SEARCH_CONDITION_S *pSearchCon, int offset, int limit, msg_struct_list_s *pMsgList);
		msg_error_t getRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList);
		msg_error_t regStorageChangeCallback(msg_storage_change_cb onStorageChange, void *pUserParam);
		msg_error_t getReportStatus(msg_message_id_t msg_id, MSG_REPORT_STATUS_INFO_S *pReport_status);
		msg_error_t getThreadIdByAddress(msg_struct_list_s *pAddrList, msg_thread_id_t *pThreadId);
		msg_error_t getThread(msg_thread_id_t threadId, MSG_THREAD_VIEW_S* pThreadInfo);
		msg_error_t getMessageList(msg_folder_id_t folderId, msg_thread_id_t threadId, msg_message_type_t msgType, msg_storage_id_t storageId, msg_struct_list_s *pMsgList);

		void convertMsgStruct(const MSG_MESSAGE_INFO_S *pSource, MSG_MESSAGE_HIDDEN_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_INFO_S* pSrc, MSG_SENDINGOPT_S* pDest, MSG_MESSAGE_TYPE_S msgType);

	private:
		void connectSocket();
		void	disconnectSocket();
        	void write(const char *pCmd, int CmdSize, char **ppEvent);
		void read(char **ppEvent);
        	void generateConnectionId(char *ConnectionId);
		void convertMsgStruct(const MSG_MESSAGE_HIDDEN_S *pSource, MSG_MESSAGE_INFO_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_S* pSrc, MSG_SENDINGOPT_INFO_S* pDest, MSG_MESSAGE_TYPE_S msgType);
		int getSettingCmdSize(MSG_OPTION_TYPE_T optionType);

		char		mConnectionId[20];
		short	mCounter;

		char 				mCookie[MAX_COOKIE_LEN];

		MsgIpcClientSocket	mClientSock;
};

#endif // MSG_HANDLE_H

