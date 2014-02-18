/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <sys/utsname.h>

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
	MSG_DEBUG("Entity: GetData: (%s)", entity->pGetData);
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

MmsPluginUaManager *MmsPluginUaManager::pInstance = NULL;

MmsPluginUaManager::MmsPluginUaManager()
{
	running = false;
	mmsTranQ.clear();
}

MmsPluginUaManager::~MmsPluginUaManager()
{
	if (pInstance) {
		delete pInstance;
		pInstance = NULL;
	}
}

MmsPluginUaManager *MmsPluginUaManager::instance()
{
	if (!pInstance)
		pInstance = new MmsPluginUaManager();

	return pInstance;
}

void MmsPluginUaManager::start()
{
//	bool bStart = true;

	MutexLocker lock(mx);

	if (!running) {

		running = true;
		MsgThread::start();
	}
}

MMS_NET_ERROR_T MmsPluginUaManager::submitHandler(mmsTranQEntity *qEntity)
{
	MMS_NET_ERROR_T ret = eMMS_UNKNOWN;
	int retryCount = 0;

	MSG_DEBUG("request Submit:");
	PRINT_PDU_TYPE(qEntity->eMmsPduType);
	PRINT_QUEUE_ENTITY(qEntity);

	MmsPluginHttpAgent*	httpAgent = MmsPluginHttpAgent::instance();

	http_request_info_s request_info = {};

	char *http_url = NULL;

	memset(&request_info, 0x00, sizeof(request_info));

	const char *home_url = NULL;
	const char *proxy_addr = NULL;
	const char *interfaceName = NULL;

	MmsPluginCmAgent::instance()->getProxyAddr(&proxy_addr);
	MmsPluginCmAgent::instance()->getInterfaceName(&interfaceName);
	MmsPluginCmAgent::instance()->getHomeUrl(&home_url);

	if (qEntity->eHttpCmdType == eHTTP_CMD_POST_TRANSACTION) {

		request_info.transaction_type = MMS_HTTP_TRANSACTION_TYPE_POST;

		request_info.url = home_url;

		request_info.proxy = proxy_addr;

		request_info.interface = interfaceName;

		request_info.post_data = qEntity->pPostData;

		request_info.post_data_len = qEntity->postDataLen;

	} else {
		request_info.transaction_type = MMS_HTTP_TRANSACTION_TYPE_GET;

		http_url = (char *)calloc(1, qEntity->getDataLen + 1);

		memcpy(http_url, qEntity->pGetData, qEntity->getDataLen);

		request_info.url = http_url;

		request_info.proxy = proxy_addr;

		request_info.interface = interfaceName;
	}


	while (retryCount < RETRY_MAX) {

		ret = httpAgent->httpRequest(request_info);

		// Process result
		if (ret == MMS_HTTP_ERROR_NONE) {
			MSG_DEBUG("Submit request sent");

			if (qEntity->pGetData) {
				free(qEntity->pGetData);
				qEntity->pGetData = NULL;
			}

			qEntity->pGetData = request_info.response_data;
			qEntity->getDataLen = request_info.response_data_len;

			break;
		} else if (ret == eMMS_HTTP_ERROR_NETWORK) {
			retryCount++;
			MSG_DEBUG("HTTP sent timeout and try again: %d", retryCount);
			continue;
		} else {
			MSG_DEBUG("Unexpected Error %d", ret);
			break;
		}
	}

	if (http_url)
		free(http_url);

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

	int msgId;

	while (1) {
		if (mmsTranQ.empty()) {
			lock();
			wait();
			unlock();
		}


		{ // Check is it emulator or not.
			struct utsname buf;
			int ret = uname(&buf);

			if (ret == 0) {
				MSG_DEBUG("System runs on [%s].", buf.machine);
				//if (strncmp(buf.machine, "i686", 4) == 0) {
				if(strcasestr(buf.machine, "emulated")) {
					MSG_DEBUG("Running on Emulator mode.");

					int mmsResult = MsgSettingGetInt(VCONFKEY_TELEPHONY_MMS_SENT_STATUS);

					MSG_DEBUG("MMS result has to be [%d]", mmsResult);

					while (!mmsTranQ.empty()) {
						MSG_DEBUG("###### mmsTranQ.size [%d]", mmsTranQ.size());

						mmsTranQEntity reqEntity;
						memset(&reqEntity, 0, sizeof(mmsTranQEntity));

						mmsTranQ.front(&reqEntity);

						if (mmsResult > 0) {
							// For MMS send fail.
							MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
							mmsTranQ.pop_front();
						} else {
							// For MMS send success.
							MSG_DEBUG("conf received successfully");

							reqEntity.eMmsPduType = eMMS_SEND_CONF;

							try {
								MmsPluginEventHandler::instance()->handleMmsReceivedData(&reqEntity, NULL);
							} catch (MsgException& e) {
								MSG_FATAL("%s", e.what());
								break;
							} catch (exception& e) {
								MSG_FATAL("%s", e.what());
								break;
							}

							mmsTranQ.pop_front();
						}
					}

					mmsTranQ.clear();
					MutexLocker locker(mx);
					running = false;

					return;
				}
			}
		}


		// Request CM Open
		if (!(cmAgent->open())) {
			MSG_DEBUG("Cm Open Failed");

			// delete all request from reqQEntities
			goto CLEANUP;
		}

		while (!mmsTranQ.empty()) {

			MSG_DEBUG("###### mmsTranQ.size [%d]", mmsTranQ.size());

			mmsTranQEntity reqEntity;
			memset(&reqEntity, 0, sizeof(mmsTranQEntity));

			mmsTranQ.front(&reqEntity);

			reqEntity.isCompleted = false;

			PRINT_QUEUE_ENTITY(&reqEntity);

			if (reqEntity.eMmsPduType == eMMS_RETRIEVE_AUTO) {
				msgId = reqEntity.msgId;
				MmsPluginStorage::instance()->updateNetStatus(msgId, MSG_NETWORK_RETRIEVING);
			}

			// MMS Transaction
			MSG_DEBUG("\n\n ===================  MMS Transaction Start ========================");

			do {

				if (submitHandler(&reqEntity) != MMS_HTTP_ERROR_NONE) {
					MSG_DEBUG("Transaction Error: submit failed");

					MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
					mmsTranQ.pop_front();
					//try to next mmsTranQ
					break;
				}

				MSG_DEBUG("submitHandler(&reqEntity) success.");

				MSG_DEBUG("#### MMS PDU TYPE = %d ####", reqEntity.eMmsPduType);

				if (reqEntity.eMmsPduType == eMMS_NOTIFYRESP_IND ||
					reqEntity.eMmsPduType == eMMS_ACKNOWLEDGE_IND ||
					reqEntity.eMmsPduType == eMMS_READREC_IND ||
					reqEntity.eMmsPduType == eMMS_CANCEL_CONF) {
					reqEntity.isCompleted = true;

					mmsTranQ.pop_front();

					MSG_DEBUG("Transaction Completed");
					break;
				} else {
					// change MmsPduType from XXX.req to XXX.conf for waiting
					MSG_DEBUG("Update Pdutype");
					updatePduType(&reqEntity);
					MSG_DEBUG("Waiting Conf");
				}

				mmsTranQ.pop_front();

				//////// Waiting Conf //////////////////////
				MMS_NET_ERROR_T networkErr;

				if ((networkErr = waitingConf(&reqEntity)) == eMMS_HTTP_CONF_SUCCESS) {
					bool bReportAllowed;
					char retrievedFilePath[MAX_FULL_PATH_SIZE+1] = {0,};

					// process Http data
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

					// send NotifyResponseInd
					if (reqEntity.eMmsPduType == eMMS_RETRIEVE_AUTO_CONF) {
						char filepath[MAX_FULL_PATH_SIZE] = {0};
						// change MmsPduType for ind or ack
						// make the PDU and then attach to reqEntity also.
						updatePduType(&reqEntity);

						MSG_DEBUG("#### eMmsPduType:%d ####", reqEntity.eMmsPduType);

						//update http command type & encode m-notify-response-ind
						reqEntity.eHttpCmdType = eHTTP_CMD_POST_TRANSACTION;

						try {
							MmsPluginInternal::instance()->encodeNotifyRespInd(reqEntity.transactionId, MMS_MSGSTATUS_RETRIEVED, bReportAllowed, filepath);
							//m-notification-resp-ind encoding	if err is not MSG_SUCCESS then should set x-mms-status to deferred
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

						mmsTranQ.push_front(reqEntity);

						if (remove(filepath) != 0) {
							MSG_DEBUG("Error removing file");
						}

						MSG_DEBUG("Submit Ind");
					} else if (reqEntity.eMmsPduType == eMMS_RETRIEVE_MANUAL_CONF) {
						/* saved msg trId should be checked  */
						// Send Acknowledge Ind
						char filepath[MAX_FULL_PATH_SIZE] = {0};
						// change MmsPduType for ind or ack
						// make the PDU and then attach to reqEntity also.
						updatePduType(&reqEntity);

						MSG_DEBUG("#### eMmsPduType:%d ####", reqEntity.eMmsPduType);

						//update http command type & encode m-notify-response-ind
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

						mmsTranQ.push_front(reqEntity);

						remove(filepath); // not ipc

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

		// Request CM Close
		cmAgent->close();

	}

CLEANUP:
	MSG_DEBUG("CLEANUP");

	while (!mmsTranQ.empty()) {
		MSG_DEBUG("clear mmsTranQ");

		mmsTranQEntity reqEntity;
		memset(&reqEntity, 0, sizeof(mmsTranQEntity));

		mmsTranQ.front(&reqEntity);

		// notify send fail to APP
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

		mmsTranQ.pop_front();
	}

	mmsTranQ.clear();
	MutexLocker locker(mx);
	running = false;

	MSG_END();

	return;
}

void MmsPluginUaManager::getMmsPduData(mmsTranQEntity *qEntity)
{
	mmsTranQ.front(qEntity);
}

void MmsPluginUaManager::addMmsReqEntity(mmsTranQEntity req)
{
	if (mmsTranQ.checkExist(req, compare_func) == true) {
		MSG_DEBUG("request Already Exist, req_id = %d", req.msgId);
		THROW(MsgException::REQ_EXIST_ERROR, "MMS request already exist");
	}

	MSG_DEBUG("New MMS Tran Added");
	mmsTranQ.push_back(req);
	lock();
	signal();
	unlock();
}

bool MmsPluginUaManager::processReceivedData(int msgId, char *pRcvdBody, int rcvdBodyLen, char *retrievedFilePath)
{
	MSG_BEGIN();

	char fileName[MSG_FILENAME_LEN_MAX] = {0};

	MSG_DEBUG(":::%d :%s ", rcvdBodyLen, pRcvdBody);

	MmsInitHeader();
	MmsRegisterDecodeBuffer();

	if (MsgCreateFileName(fileName) == false)
		return false;

	snprintf(retrievedFilePath, MSG_FILEPATH_LEN_MAX, MSG_DATA_PATH"%s", fileName);

	MSG_DEBUG("retrievedFilePaths [%s]", retrievedFilePath);

	// create temp file
	if (!MsgOpenCreateAndOverwriteFile(retrievedFilePath, (char *)pRcvdBody, rcvdBodyLen)) {
		MSG_DEBUG( "_MmsUaInitMsgDecoder: creating temporary file failed(msgID=%d)\n", msgId);
		return false;
	}

	if (MmsReadMsgBody(msgId, true, true, retrievedFilePath) == false) {
		MSG_DEBUG("The MMS Message might include drm contents!!!");
		goto ERR_MMS_UA_PROCESS_CONF;
	}

	MSG_END();

	return true;

ERR_MMS_UA_PROCESS_CONF:
	{
		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);


		MmsInitHeader();
		MmsUnregisterDecodeBuffer();

#ifdef __SUPPORT_DRM__
		MmsReleaseMsgDRMInfo(&pMsg->msgType.drmInfo);
#endif
		MmsReleaseMsgBody(&pMsg->msgBody, pMsg->msgType.type);

		return false;
	}
}


