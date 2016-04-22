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
#include "MsgMutex.h"

/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/

class MsgHandle
{
	public:
		MsgHandle();
		virtual ~MsgHandle();

		/* Control */
		void openHandle();
		void closeHandle(MsgHandle* pHandle);

		/* Transport */
		msg_error_t submitReq(MSG_REQUEST_S* pReq);

		msg_error_t regSentStatusCallback(msg_sent_status_cb onStatusChanged,  void *pUserParam);
		msg_error_t regSmsMessageCallback(msg_sms_incoming_cb onMsgIncoming, unsigned short port, void *pUserParam);
		msg_error_t regMmsConfMessageCallback(msg_mms_conf_msg_incoming_cb onMMSConfMsgIncoming, const char *pAppId, void *pUserParam);
		msg_error_t regSyncMLMessageCallback(msg_syncml_msg_incoming_cb onSyncMLMsgIncoming, void *pUserParam);
		msg_error_t regLBSMessageCallback(msg_lbs_msg_incoming_cb onLBSMsgIncoming, void *pUserParam);
		msg_error_t regPushMessageCallback(msg_push_msg_incoming_cb onPushMsgIncoming, const char *pAppId, void *pUserParam);
		msg_error_t regCBMessageCallback(msg_cb_incoming_cb onCBIncoming, bool bSave, void *pUserParam);
		msg_error_t regSyncMLMessageOperationCallback(msg_syncml_msg_operation_cb onSyncMLMsgOperation, void *pUserParam);
		msg_error_t regReportMessageCallback(msg_report_msg_incoming_cb onReportMsgCB, void *pUserParam);

		msg_error_t operateSyncMLMessage(msg_message_id_t msgId);

		/* Storage */
		int addMessage(MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t addSyncMLMessage(const MSG_SYNCML_MESSAGE_S *pSyncMLMsg);
		msg_error_t updateMessage(const MSG_MESSAGE_HIDDEN_S *pMsg, const MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t updateReadStatus(msg_message_id_t MsgId, bool bRead);
		msg_error_t setConversationToRead(msg_thread_id_t ThreadId);
		msg_error_t updateProtectedStatus(msg_message_id_t MsgId, bool bProtected);
		msg_error_t deleteMessage(msg_message_id_t MsgId);
		msg_error_t deleteAllMessagesInFolder(msg_folder_id_t FolderId, bool bOnlyDB);
		msg_error_t deleteMessagesByList(msg_id_list_s *pMsgIdList);
		msg_error_t moveMessageToFolder(msg_message_id_t MsgId, msg_folder_id_t DestFolderId);
		msg_error_t moveMessageToStorage(msg_message_id_t MsgId, msg_storage_id_t DestStorageId);
		msg_error_t countMessage(msg_folder_id_t FolderId, MSG_COUNT_INFO_S *pCountInfo);
		msg_error_t countMsgByType(const MSG_MESSAGE_TYPE_S *pMsgType, int *pMsgCount);
		msg_error_t countMsgByContact(const MSG_THREAD_LIST_INDEX_INFO_S *pAddrInfo, MSG_THREAD_COUNT_INFO_S *pMsgThreadCountList);
		msg_error_t getMessage(msg_message_id_t MsgId, MSG_MESSAGE_HIDDEN_S *pMsg, MSG_SENDINGOPT_S *pSendOpt);
		msg_error_t getConversationViewItem(msg_message_id_t MsgId, MSG_CONVERSATION_VIEW_S *pConv);
		msg_error_t addFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		msg_error_t updateFolder(const MSG_FOLDER_INFO_S *pFolderInfo);
		msg_error_t deleteFolder(msg_folder_id_t FolderId);
		msg_error_t getFolderList(msg_struct_list_s *pFolderList);

		msg_error_t getThreadViewList(const MSG_SORT_RULE_S *pSortRule, msg_struct_list_s *pThreadViewList);
		msg_error_t getConversationViewList(msg_thread_id_t ThreadId, msg_struct_list_s *pConvViewList);
		msg_error_t deleteThreadMessageList(msg_thread_id_t thread_id, bool include_protected_msg);
		msg_error_t getQuickPanelData(msg_quickpanel_type_t Type, MSG_MESSAGE_HIDDEN_S *pMsg);
		msg_error_t resetDatabase();
		msg_error_t getMemSize(unsigned int* memsize);

		msg_error_t getAddressList(const msg_thread_id_t threadId, msg_struct_list_s *pAddrList);

		/* Filter */
		msg_error_t addFilter(const MSG_FILTER_S *pFilter);
		msg_error_t updateFilter(const MSG_FILTER_S *pFilter);
		msg_error_t deleteFilter(msg_filter_id_t FilterId);
		msg_error_t getFilterList(msg_struct_list_s *pFilterList);
		msg_error_t setFilterOperation(bool bSetFlag);
		msg_error_t getFilterOperation(bool *pSetFlag);
		msg_error_t setFilterActivation(msg_filter_id_t filter_id, bool active);

		/*setting */
		msg_error_t getSMSCOption(msg_struct_t msg_struct);
		msg_error_t setSMSCOption(msg_struct_t msg_struct);
		msg_error_t getCBOption(msg_struct_t msg_struct);
		msg_error_t setCBOption(msg_struct_t msg_struct);
		msg_error_t getSmsSendOpt(msg_struct_t msg_struct);
		msg_error_t setSmsSendOpt(msg_struct_t msg_struct);
		msg_error_t getMmsSendOpt(msg_struct_t msg_struct);
		msg_error_t setMmsSendOpt(msg_struct_t msg_struct);
		msg_error_t getMmsRecvOpt(msg_struct_t msg_struct);
		msg_error_t setMmsRecvOpt(msg_struct_t msg_struct);
		msg_error_t getPushMsgOpt(msg_struct_t msg_struct);
		msg_error_t setPushMsgOpt(msg_struct_t msg_struct);
		msg_error_t getVoiceMsgOpt(msg_struct_t msg_struct);
		msg_error_t setVoiceMsgOpt(msg_struct_t msg_struct);
		msg_error_t getGeneralOpt(msg_struct_t msg_struct);
		msg_error_t setGeneralOpt(msg_struct_t msg_struct);
		msg_error_t getMsgSizeOpt(msg_struct_t msg_struct);
		msg_error_t setMsgSizeOpt(msg_struct_t msg_struct);

		/*Backup & Restore */
		msg_error_t backupMessage(msg_message_backup_type_t type, const char *backup_filepath);
		msg_error_t restoreMessage(const char *backup_filepath);
		msg_error_t getVobject(msg_message_id_t MsgId, void** encodedData);
		/* ETC */
		msg_error_t searchMessage(const char *pSearchString, msg_struct_list_s *pThreadViewList);


		msg_error_t dbSelectWithQuery(const char *query, char ***db_res, int *row_count, int *col_count);
		void dbFree(char **db_res);

		msg_error_t getRejectMsgList(const char *pNumber, msg_struct_list_s *pRejectMsgList);
		msg_error_t regStorageChangeCallback(msg_storage_change_cb onStorageChange, void *pUserParam);
		msg_error_t getReportStatus(msg_message_id_t msg_id, msg_struct_list_s *report_list);
		msg_error_t getThreadIdByAddress(msg_struct_list_s *pAddrList, msg_thread_id_t *pThreadId);
		msg_error_t getThreadIdByAddress(msg_list_handle_t msg_address_list, msg_thread_id_t *pThreadId);
		msg_error_t getThread(msg_thread_id_t threadId, MSG_THREAD_VIEW_S* pThreadInfo);
		msg_error_t getMessageList(const MSG_LIST_CONDITION_S *pListCond, msg_struct_list_s *pMsgList);
		msg_error_t getMediaList(const msg_thread_id_t thread_id, msg_list_handle_t *pMediaList);

		/* Push Event */
		msg_error_t addPushEvent(MSG_PUSH_EVENT_INFO_S *push_event);
		msg_error_t deletePushEvent(MSG_PUSH_EVENT_INFO_S *push_event);
		msg_error_t updatePushEvent(MSG_PUSH_EVENT_INFO_S *pSrc, MSG_PUSH_EVENT_INFO_S *pDst);

		void convertMsgStruct(const MSG_MESSAGE_INFO_S *pSource, MSG_MESSAGE_HIDDEN_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_INFO_S* pSrc, MSG_SENDINGOPT_S* pDest, MSG_MESSAGE_TYPE_S msgType);

		void connectSocket();
		void disconnectSocket();

	private:
		void write(const char *pCmd, int CmdSize, char **ppEvent);
		void read(char **ppEvent);
		void generateConnectionId(char *ConnectionId);
		void convertMsgStruct(const MSG_MESSAGE_HIDDEN_S *pSource, MSG_MESSAGE_INFO_S *pDest);
		void convertSendOptStruct(const MSG_SENDINGOPT_S* pSrc, MSG_SENDINGOPT_INFO_S* pDest, MSG_MESSAGE_TYPE_S msgType);
		int getSettingCmdSize(MSG_OPTION_TYPE_T optionType);
		bool checkEventData(char *pEventData);
		msg_error_t checkPermission(void);

		char mConnectionId[20];
		short mCounter;

		char mCookie[MAX_COOKIE_LEN];

		MsgIpcClientSocket mClientSock;
		MsgMutex mx;
};

#endif /* MSG_HANDLE_H */

