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

#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgGconfWrapper.h"

#include "MmsPluginDebug.h"
#include "MmsPluginUserAgent.h"
#include "MmsPluginHttp.h"
#include "MmsPluginConnManWrapper.h"
#include "MmsPluginEventHandler.h"
#include "MmsPluginInternal.h"
#include "MmsPluginCodec.h"
#include "MmsPluginDrm.h"
#include "MmsPluginStorage.h"
#include "MmsPluginUtil.h"

void PRINT_PDU_TYPE(MMS_PDU_TYPE_T pduType)
{
	switch (pduType) {
	case eMMS_SEND_REQ:
		MSG_DEBUG("[SEND_REQ]");
		break;
	case eMMS_SEND_CONF:
		MSG_DEBUG("[MMS_SEND_CONF]");
		break;
	case eMMS_RETRIEVE_AUTO:
		MSG_DEBUG("[MMS_RETRIEVE_AUTO]");
		break;
	case eMMS_RETRIEVE_MANUAL:
		MSG_DEBUG("[MMS_RETRIEVE_MANUAL]");
		break;
	case eMMS_RETRIEVE_AUTO_CONF:
		MSG_DEBUG("[MMS_RETRIEVE_AUTO_CONF]");
		break;
	case eMMS_RETRIEVE_MANUAL_CONF:
		MSG_DEBUG("[MMS_RETRIEVE_MANUAL_CONF]");
		break;
	case eMMS_DELIVERY_IND:
		MSG_DEBUG("[MMS_DELIVERY_IND]");
		break;
	case eMMS_NOTIFICATION_IND:
		MSG_DEBUG("[MMS_NOTIFICATION_IND]");
		break;
	case eMMS_NOTIFYRESP_IND:
		MSG_DEBUG("[MMS_NOTIFYRESP_IND]");
		break;
	case eMMS_ACKNOWLEDGE_IND:
		MSG_DEBUG("[MMS_ACKNOWLEDGE_IND]");
		break;
	case eMMS_FORWARD_REQ:
		MSG_DEBUG("[MMS_FORWARD_REQ]");
		break;
	case eMMS_FORWARD_CONF:
		MSG_DEBUG("[MMS_FORWARD_CONF]");
		break;
	case eMMS_CANCEL_REQ:
		MSG_DEBUG("[MMS_CANCEL_REQ]");
		break;
	case eMMS_CANCEL_CONF:
		MSG_DEBUG("[MMS_CANCEL_CONF]");
		break;
	case eMMS_DELETE_REQ:
		MSG_DEBUG("[MMS_DELETE_REQ]");
		break;
	case eMMS_DELETE_CONF:
		MSG_DEBUG("[MMS_DELETE_CONF]");
		break;
	case eMMS_READREC_IND:
		MSG_DEBUG("[MMS_READREC_IND]");
		break;
	case eMMS_READORIG_IND:
		MSG_DEBUG("[MMS_READORIG_IND]");
		break;
	case eMMS_MBOX_STORE_REQ:
		MSG_DEBUG("[MMS_MBOX_STORE_REQ]");
		break;
	case eMMS_MBOX_STORE_CONF:
		MSG_DEBUG("[MMS_MBOX_STORE_CONF]");
		break;
	case eMMS_MBOX_VIEW_REQ:
		MSG_DEBUG("[MMS_MBOX_VIEW_REQ]");
		break;
	case eMMS_MBOX_VIEW_CONF:
		MSG_DEBUG("[MMS_MBOX_VIEW_CONF]");
		break;
	case eMMS_MBOX_UPLOAD_REQ:
		MSG_DEBUG("[MMS_MBOX_UPLOAD_REQ]");
		break;
	case eMMS_MBOX_UPLOAD_CONF:
		MSG_DEBUG("[MMS_MBOX_UPLOAD_CONF]");
		break;
	case eMMS_MBOX_DELETE_REQ:
		MSG_DEBUG("[MMS_MBOX_DELETE_REQ]");
		break;
	case eMMS_MBOX_DELETE_CONF:
		MSG_DEBUG("[MMS_MBOX_DELETE_CONF]");
		break;
	default:
		MSG_DEBUG("[Unknown PDU Type]");
		break;
	}
}


void PRINT_QUEUE_ENTITY(mmsTranQEntity *entity)
{
	MSG_DEBUG("Entity: msgId: %d", entity->msgId);
	MSG_DEBUG("Entity: completed: %d", entity->isCompleted);
	MSG_DEBUG("Entity: eMmsPduType: %d", entity->eMmsPduType);
	MSG_DEBUG("Entity: eHttpCmdType: %d", entity->eHttpCmdType);
	MSG_DEBUG("Entity: GetLen: %d", entity->getDataLen);
	MSG_SEC_DEBUG("Entity: GetData: (%s)", entity->pGetData);
	MSG_DEBUG("Entity: postLen: %d", entity->postDataLen);
	MSG_DEBUG("Entity: pPostData: (%s)", entity->pPostData);
}

void updatePduType(mmsTranQEntity *qEntity)
{
	switch(qEntity->eMmsPduType) {
	case eMMS_SEND_REQ:
		qEntity->eMmsPduType = eMMS_SEND_CONF;
		break;
	case eMMS_RETRIEVE_AUTO:
		qEntity->eMmsPduType = eMMS_RETRIEVE_AUTO_CONF;
		break;
	case eMMS_RETRIEVE_MANUAL:
		qEntity->eMmsPduType = eMMS_RETRIEVE_MANUAL_CONF;
		break;
	case eMMS_RETRIEVE_AUTO_CONF:
		qEntity->eMmsPduType = eMMS_NOTIFYRESP_IND;
		break;
	case eMMS_READREC_IND:
		qEntity->eMmsPduType = eMMS_SEND_CONF;
		break;
	case eMMS_READREPORT_REQ:
		qEntity->eMmsPduType = eMMS_READREPORT_CONF;
		break;
	case eMMS_RETRIEVE_MANUAL_CONF:
		qEntity->eMmsPduType = eMMS_ACKNOWLEDGE_IND;
		break;
	case eMMS_DELETE_REQ:
		qEntity->eMmsPduType = eMMS_DELETE_CONF;
		break;
	case eMMS_FORWARD_REQ:
		qEntity->eMmsPduType = eMMS_FORWARD_CONF;
		break;
	case eMMS_MBOX_STORE_REQ:
		qEntity->eMmsPduType = eMMS_MBOX_STORE_CONF;
		break;
	case eMMS_MBOX_VIEW_REQ:
		qEntity->eMmsPduType = eMMS_MBOX_VIEW_CONF;
		break;
	case eMMS_MBOX_UPLOAD_REQ:
		qEntity->eMmsPduType = eMMS_MBOX_UPLOAD_CONF;
		break;
	case eMMS_MBOX_DELETE_REQ:
		qEntity->eMmsPduType = eMMS_MBOX_DELETE_CONF;
		break;
	default:
		break;
	}

	MSG_DEBUG("Update PDU Type:");
	PRINT_PDU_TYPE(qEntity->eMmsPduType);
}

bool compare_func(mmsTranQEntity const &a, mmsTranQEntity const &b)
{
	if (a.msgId == b.msgId) {
		if ((a.eMmsPduType == eMMS_RETRIEVE_MANUAL || a.eMmsPduType == eMMS_RETRIEVE_AUTO)
				&&(b.eMmsPduType == eMMS_RETRIEVE_MANUAL || b.eMmsPduType == eMMS_RETRIEVE_AUTO)) {
			return true;
		}
	}

	return false;
}

bool compare_func_for_removal(mmsTranQEntity const &a, mmsTranQEntity const &b)
{
	if (a.reqID == b.reqID &&
			a.msgId == b.msgId &&
			a.sessionId == b.sessionId &&
			a.simId == b.simId) {
			return true;
	}
	return false;
}

MmsPluginUaManager *MmsPluginUaManager::pInstance = NULL;

MmsPluginUaManager::MmsPluginUaManager()
{
	lock();
	running = false;
	mmsTranQ.clear();
	unlock();
}

MmsPluginUaManager::~MmsPluginUaManager()
{

}

MmsPluginUaManager *MmsPluginUaManager::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginUaManager();

	return pInstance;
}

void MmsPluginUaManager::start()
{
/*	bool bStart = true; */

	MutexLocker lock(mx);

	if (!running) {

		running = true;
		MsgThread::start();
	}
}

MMS_NET_ERROR_T MmsPluginUaManager::submitHandler(mmsTranQEntity *qEntity)
{
	MSG_BEGIN();

	MMS_NET_ERROR_T ret = eMMS_UNKNOWN;
	http_request_info_s request_info = {};
	char *http_url = NULL;
	const char *home_url = NULL;
	const char *proxy_addr = NULL;
	const char *dns_list = NULL;
	const char *interfaceName = NULL;
	bool cm_ret;

	PRINT_PDU_TYPE(qEntity->eMmsPduType);

	PRINT_QUEUE_ENTITY(qEntity);

	cm_ret = MmsPluginCmAgent::instance()->getProxyAddr(&proxy_addr);
	if (cm_ret == false)
		return eMMS_EXCEPTIONAL_ERROR;

	cm_ret = MmsPluginCmAgent::instance()->getInterfaceName(&interfaceName);
	if (cm_ret == false)
		return eMMS_EXCEPTIONAL_ERROR;


	cm_ret = MmsPluginCmAgent::instance()->getHomeUrl(&home_url);
	if (cm_ret == false)
		return eMMS_EXCEPTIONAL_ERROR;

	cm_ret = MmsPluginCmAgent::instance()->getDnsAddrList(&dns_list);
	if (cm_ret == false)
		return eMMS_EXCEPTIONAL_ERROR;

	memset(&request_info, 0x00, sizeof(request_info));

	if (qEntity->eHttpCmdType == eHTTP_CMD_POST_TRANSACTION) {

		request_info.transaction_type = MMS_HTTP_TRANSACTION_TYPE_POST;

		request_info.url = home_url;

		request_info.proxy = proxy_addr;

		request_info.dns_list = dns_list;

		request_info.interface = interfaceName;

		request_info.post_data = qEntity->pPostData;

		request_info.post_data_len = qEntity->postDataLen;

	} else {
		request_info.transaction_type = MMS_HTTP_TRANSACTION_TYPE_GET;

		http_url = (char *)calloc(1, qEntity->getDataLen + 1);

		if (http_url)
			memcpy(http_url, qEntity->pGetData, qEntity->getDataLen);

		request_info.url = http_url;

		request_info.proxy = proxy_addr;

		request_info.dns_list = dns_list;

		request_info.interface = interfaceName;
	}

	MMS_HTTP_ERROR_E http_ret;
	MmsPluginHttpAgent*	httpAgent = MmsPluginHttpAgent::instance();
	http_ret = httpAgent->httpRequest(request_info);

	if (http_ret == MMS_HTTP_ERROR_NONE) {
		MSG_DEBUG("Submit request sent");

		if (qEntity->pGetData) {
			free(qEntity->pGetData);
			qEntity->pGetData = NULL;
		}

		qEntity->pGetData = request_info.response_data;
		qEntity->getDataLen = request_info.response_data_len;
		ret = eMMS_SUCCESS;
	} else {
		bool cm_status = MmsPluginCmAgent::instance()->getCmStatus();
		if (cm_status == false) {
			MSG_INFO("PDP disconnected while MMS transaction in progress. cm status [%d]", cm_status);
			ret = eMMS_CM_DISCONNECTED;
		} else {
			MSG_DEBUG("Unexpected Error http_ret = [%d]", http_ret);
			ret = eMMS_HTTP_ERROR_NETWORK;
		}
	}

	if (http_url)
		free(http_url);

	MSG_END();
	return ret;
}

MMS_NET_ERROR_T MmsPluginUaManager::waitingConf(mmsTranQEntity *qEntity)
{
	return eMMS_HTTP_CONF_SUCCESS;
}

void MmsPluginUaManager::run()
{
	MSG_BEGIN();

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();

	while (1) {
		lock();
		while (mmsTranQ.empty()) {
			wait();
		}
		unlock();

		/* Request CM Open */
		if (!(cmAgent->open())) {
			MSG_FATAL("Cm Open Failed");
			/* delete all request from reqQEntities */
			lock();
			int qSize = mmsTranQ.size();
			unlock();
			if (qSize > 0) {
				MSG_DEBUG("remove an entity from mmsTranQ");

				mmsTranQEntity reqEntity;
				memset(&reqEntity, 0, sizeof(mmsTranQEntity));

				lock();
				mmsTranQ.front(&reqEntity);
				unlock();
				/* notify send fail to APP */
				MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);

				if (reqEntity.pGetData) {
					MSG_DEBUG("free pGetData");
					free(reqEntity.pGetData);
					reqEntity.pGetData = NULL;
				}

				if (reqEntity.pPostData) {
					MSG_DEBUG("free pPostData");
					free(reqEntity.pPostData);
					reqEntity.pPostData = NULL;
				}
				lock();
				mmsTranQ.remove(reqEntity, compare_func_for_removal);
/*				mmsTranQ.pop_front(); */
				unlock();
			}

			continue;
		}

		bool transaction = true;
		while (transaction) {
			lock();
			int qSize = mmsTranQ.size();
			unlock();
			if (qSize <= 0) {
				break;
			}

			MSG_DEBUG("###### mmsTranQ.size [%d]", qSize);

			mmsTranQEntity reqEntity;
			memset(&reqEntity, 0, sizeof(mmsTranQEntity));

			lock();
			mmsTranQ.front(&reqEntity);
			unlock();

			reqEntity.isCompleted = false;

			PRINT_QUEUE_ENTITY(&reqEntity);

			/* MMS Transaction */
			MSG_DEBUG("\n\n ===================  MMS Transaction Start ========================");

			do {

				MMS_NET_ERROR_T mms_net_status = submitHandler(&reqEntity);
				reqEntity.eMmsTransactionStatus = mms_net_status;
				if (mms_net_status != eMMS_SUCCESS) {
					MSG_DEBUG("Transaction Error: submit failed");

					MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
					lock();
					mmsTranQ.remove(reqEntity, compare_func_for_removal);
/*					mmsTranQ.pop_front(); */
					unlock();
					/* try to next mmsTranQ */
					break;
				}

				MSG_DEBUG("submitHandler(&reqEntity) success.");

				MSG_DEBUG("#### MMS PDU TYPE = %d ####", reqEntity.eMmsPduType);

				if (reqEntity.eMmsPduType == eMMS_NOTIFYRESP_IND ||
						reqEntity.eMmsPduType == eMMS_ACKNOWLEDGE_IND ||
						reqEntity.eMmsPduType == eMMS_READREC_IND ||
						reqEntity.eMmsPduType == eMMS_CANCEL_CONF) {
					reqEntity.isCompleted = true;
					lock();
					mmsTranQ.remove(reqEntity, compare_func_for_removal);
/*					mmsTranQ.pop_front(); */
					unlock();
					MSG_DEBUG("Transaction Completed");
					break;
				} else {
					/* change MmsPduType from XXX.req to XXX.conf for waiting */
					MSG_DEBUG("Update Pdutype");
					updatePduType(&reqEntity);
					MSG_DEBUG("Waiting Conf");
				}
				lock();
				mmsTranQ.remove(reqEntity, compare_func_for_removal);
/*				mmsTranQ.pop_front(); */
				unlock();
				//////// Waiting Conf //////////////////////
				MMS_NET_ERROR_T networkErr;

				if ((networkErr = waitingConf(&reqEntity)) == eMMS_HTTP_CONF_SUCCESS) {
					bool bReportAllowed;
					char retrievedFilePath[MAX_FULL_PATH_SIZE+1] = {0,};

					/* process Http data */
					try {
						if (processReceivedData(reqEntity.msgId, reqEntity.pGetData, reqEntity.getDataLen, retrievedFilePath) == false)	{
							MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
							break;
						}
					} catch (MsgException& e) {
						MSG_FATAL("%s", e.what());
						MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
						break;

					} catch (exception& e) {
						MSG_FATAL("%s", e.what());
						MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
						break;
					}

					if (reqEntity.eMmsPduType != (MMS_PDU_TYPE_T)mmsHeader.type) {
						if (!(reqEntity.eMmsPduType == eMMS_RETRIEVE_MANUAL_CONF && mmsHeader.type == MMS_MSGTYPE_RETRIEVE_CONF)) {
							MSG_DEBUG("FAIL::type mismatched req:%d received:%d", reqEntity.eMmsPduType, mmsHeader.type);

							MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
							break;
						}
					}

					MSG_DEBUG("conf received successfully");
					try {
						MmsPluginEventHandler::instance()->handleMmsReceivedData(&reqEntity, retrievedFilePath);
					} catch (MsgException& e) {
						MSG_FATAL("%s", e.what());
						break;
					} catch (exception& e) {
						MSG_FATAL("%s", e.what());
						break;
					}

					MsgSettingGetBool(MMS_SEND_REPORT_ALLOWED, &bReportAllowed);

					MSG_DEBUG("conf received successfully -2");
					MSG_DEBUG("reqEntity.eMmsPduType [%d]", reqEntity.eMmsPduType);

					/* send NotifyResponseInd */
					if (reqEntity.eMmsPduType == eMMS_RETRIEVE_AUTO_CONF) {
						char filepath[MAX_FULL_PATH_SIZE] = {0};
						/* change MmsPduType for ind or ack */
						/* make the PDU and then attach to reqEntity also. */
						updatePduType(&reqEntity);

						MSG_DEBUG("#### eMmsPduType:%d ####", reqEntity.eMmsPduType);

						/* update http command type & encode m-notify-response-ind */
						reqEntity.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;

						try {
							MmsPluginInternal::instance()->encodeNotifyRespInd(reqEntity.transactionId, MMS_MSGSTATUS_RETRIEVED, bReportAllowed, filepath);
							/* m-notification-resp-ind encoding	if err is not MSG_SUCCESS then should set x-mms-status to deferred */
							if (MsgGetFileSize(filepath, &reqEntity.postDataLen) == false) {
								MSG_DEBUG("MsgGetFileSize: failed");
								break;
							}
						} catch (MsgException& e) {
							MSG_FATAL("%s", e.what());
							break;
						} catch (exception& e) {
							MSG_FATAL("%s", e.what());
							break;
						}

						if (reqEntity.pPostData) {
							free(reqEntity.pPostData);
							reqEntity.pPostData = NULL;
						}

						reqEntity.pPostData = MsgOpenAndReadMmsFile(filepath, 0, -1, &reqEntity.postDataLen);
						lock();
						mmsTranQ.push_front(reqEntity);
						unlock();
						if (remove(filepath) != 0) {
							MSG_DEBUG("Error removing file");
						}

						MSG_DEBUG("Submit Ind");
					} else if (reqEntity.eMmsPduType == eMMS_RETRIEVE_MANUAL_CONF) {
						/* saved msg trId should be checked
						 * Send Acknowledge Ind */
						char filepath[MAX_FULL_PATH_SIZE] = {0};
						/* change MmsPduType for ind or ack
						 * make the PDU and then attach to reqEntity also. */
						updatePduType(&reqEntity);

						MSG_DEBUG("#### eMmsPduType:%d ####", reqEntity.eMmsPduType);

						/* update http command type & encode m-notify-response-ind */
						reqEntity.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;

						try {
							MmsPluginInternal::instance()->encodeAckInd(reqEntity.transactionId, bReportAllowed, filepath);
							if (MsgGetFileSize(filepath, &reqEntity.postDataLen) == false) {
								MSG_DEBUG("MsgGetFileSize: failed");
								break;
							}
						} catch (MsgException& e) {
							MSG_FATAL("%s", e.what());
							break;
						} catch (exception& e) {
							MSG_FATAL("%s", e.what());
							break;
						}

						if (reqEntity.pPostData) {
							free(reqEntity.pPostData);
							reqEntity.pPostData = NULL;
						}

						reqEntity.pPostData = MsgOpenAndReadMmsFile(filepath, 0, -1, &reqEntity.postDataLen);
						lock();
						mmsTranQ.push_front(reqEntity);
						unlock();
						remove(filepath); /* not ipc */

						MSG_DEBUG("Submit Ack");
					} else {
						reqEntity.isCompleted = true;

						MSG_DEBUG("Transaction complete");
					}
				} else {
					MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
					break;
				}

			} while (reqEntity.isCompleted == false);

			MSG_DEBUG("==== MMS Transaction Completed ====\n\n");

			if (reqEntity.pPostData) {
				free(reqEntity.pPostData);
				reqEntity.pPostData = NULL;
			}

			if (reqEntity.pGetData) {
				free(reqEntity.pGetData);
				reqEntity.pGetData = NULL;
			}

		}

		/* Request CM Close */
		cmAgent->close();

	}

	MSG_END();

	return;
}

void MmsPluginUaManager::getMmsPduData(mmsTranQEntity *qEntity)
{
	lock();
	mmsTranQ.front(qEntity);
	unlock();
}

void MmsPluginUaManager::addMmsReqEntity(mmsTranQEntity req)
{
	lock();
	if (mmsTranQ.checkExist(req, compare_func) == true) {
		MSG_DEBUG("request Already Exist, req_id = %d", req.msgId);
		unlock();
		THROW(MsgException::REQ_EXIST_ERROR, "MMS request already exist");
	}
	mmsTranQ.push_back(req);
	signal();
	unlock();

	MSG_DEBUG("New MMS Tran Added");
}

bool MmsPluginUaManager::processReceivedData(int msgId, char *pRcvdBody, int rcvdBodyLen, char *retrievedFilePath)
{
	MSG_BEGIN();

	/* CID 317909 : replacing MSG_FILENAME_LEN_MAX with MAX_FULL_PATH_SIZE as the latter is max length for internal file path
	 * and size of retrievedFilePath in calling function is same i.e. MAX_FULL_PATH_SIZE+1
	 * CID 358483 : Making fileName smaller causes buffer overflow in MsgCreateFileName function.
	 * So We will keep it 1024 as before but only copy 320 out of it which is the size of retrievedFilePath buffer. */
	char fileName[MSG_FILENAME_LEN_MAX] = {0};

	MSG_DEBUG(":::%d :%s ", rcvdBodyLen, pRcvdBody);

	MmsInitHeader();
	MmsRegisterDecodeBuffer();

	if (MsgCreateFileName(fileName) == false)
		return false;

	/* CID 317909 : replacing MSG_FILENAME_LEN_MAX with MAX_FULL_PATH_SIZE as the latter is max length for internal file path
	 * and size of retrievedFilePath in calling function is same i.e. MAX_FULL_PATH_SIZE+1
	 * snprintf(retrievedFilePath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_DATA_PATH, fileName); */
	snprintf(retrievedFilePath, MAX_FULL_PATH_SIZE, "%s%s", MSG_DATA_PATH, fileName);

	MSG_SEC_INFO("retrievedFilePaths [%s]", retrievedFilePath);

	/* create temp file */
	if (!MsgOpenCreateAndOverwriteFile(retrievedFilePath, (char *)pRcvdBody, rcvdBodyLen)) {
		MSG_ERR( "_MmsUaInitMsgDecoder: creating temporary file failed(msgID=%d)\n", msgId);
		return false;
	}
#if 1
	MmsMsg *pMsg;

	MmsPluginStorage::instance()->getMmsMessage(&pMsg);

	memset(pMsg, 0, sizeof(MmsMsg));

	MmsPluginDecoder::instance()->decodeMmsPdu(pMsg, msgId, retrievedFilePath);
#else
	if (MmsReadMsgBody(msgId, true, true, retrievedFilePath) == false) {
		MSG_INFO("The MMS Message might include drm contents!!!");

		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);


		MmsInitHeader();
		MmsUnregisterDecodeBuffer();


		MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

		MSG_END();

		return false;
	}
#endif
	MSG_END();

	return true;
}


