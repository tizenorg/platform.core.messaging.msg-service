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

#include <glib.h>
#include <curl/curl.h>
#include "MmsPluginUserAgent.h"
#include "MmsPluginEventHandler.h"
#include "MsgGconfWrapper.h"
#include "MmsPluginInternal.h"
#include "MsgUtilFile.h"
#include "MmsPluginCodec.h"
#include "MsgException.h"
#include "MmsPluginDrm.h"
#include "MmsPluginStorage.h"


extern MmsHeader mmsHeader;

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

	MSG_DEBUG("Update PDU Type:");
	PRINT_PDU_TYPE(qEntity->eMmsPduType);

	default:
		break;
	}

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

	while (retryCount < RETRY_MAX) {
		ret = httpAgent->cmdRequest(qEntity->eHttpCmdType);

		// Process result
		if (ret == eMMS_HTTP_SENT_SUCCESS) {
			MSG_DEBUG("Submit request sent");
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

	return ret;
}

MMS_NET_ERROR_T MmsPluginUaManager::waitingConf(mmsTranQEntity *qEntity)
{
	MMS_NET_ERROR_T ret = eMMS_HTTP_ERROR_UNKNOWN;
	MmsPluginHttpAgent *pHttpAgent = MmsPluginHttpAgent::instance();
	MMS_PLUGIN_HTTP_CONTEXT_S *pMmsPldCd = NULL;

	pMmsPldCd = pHttpAgent->getMmsPldCd();

	if (qEntity->pGetData) {
		free(qEntity->pGetData);
		qEntity->pGetData = NULL;
	}
	qEntity->getDataLen = pMmsPldCd->bufOffset;
	qEntity->pGetData = (char *)calloc(1, pMmsPldCd->bufOffset + 1);

	memcpy(qEntity->pGetData, pMmsPldCd->final_content_buf, pMmsPldCd->bufOffset);
	free(pMmsPldCd->final_content_buf);
	pMmsPldCd->final_content_buf = NULL;
	pMmsPldCd->bufOffset = 0;

	MSG_DEBUG("dataLen:%d  pData:(%s)", qEntity->getDataLen, qEntity->pGetData);

	ret = eMMS_HTTP_CONF_SUCCESS;

	return ret;
}

void MmsPluginUaManager::run()
{
	MSG_BEGIN();

	MmsPluginCmAgent *cmAgent = MmsPluginCmAgent::instance();
	MmsPluginHttpAgent *httpAgent = MmsPluginHttpAgent::instance();

	int trId;
//	CURL *session = NULL;

	int msgId;

	while (1) {
		if (mmsTranQ.empty()) {
			lock();
			wait();
			unlock();
		}

		// Request CM Open
		if (!(cmAgent->open())) {
			MSG_DEBUG("Cm Open Failed");

			// delete all request from reqQEntities
			goto CLEANUP;
		}

		httpAgent->SetMMSProfile();

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

			if (httpAgent->cmdRequest(eHTTP_CMD_INIT_SESSION) == eMMS_HTTP_SESSION_OPEN_FAILED) {
				MSG_DEBUG("HTTP session open failed");
				// cm close
				cmAgent->close();
				// delete all request from reqQEntities
				goto CLEANUP;
			}

			// MMS Transaction
			MSG_DEBUG("\n\n ===================  MMS Transaction Start ========================");

			do {
				httpAgent->setSession(&reqEntity);

				if (submitHandler(&reqEntity) != eMMS_HTTP_SENT_SUCCESS) {
					MSG_DEBUG("Transaction Error: submit failed");

					MmsPluginEventHandler::instance()->handleMmsError(&reqEntity);
					mmsTranQ.pop_front();
					//try to next mmsTranQ
					break;
				}

				MSG_DEBUG("submitHandler(&reqEntity) success.");
	 			trId = httpAgent->getHttpConfigData()->transactionId;

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

					MsgSettingGetBool(MMS_RECV_DELIVERY_RECEIPT, &bReportAllowed);

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

						remove(filepath);

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

			// Http Session Close
			httpAgent->clearSession();
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

	_MmsInitHeader();
	_MmsRegisterDecodeBuffer(gszMmsLoadBuf1,  gszMmsLoadBuf2, MSG_MMS_DECODE_BUFFER_MAX);

	if (MsgCreateFileName(fileName) == false)
		return false;

	snprintf(retrievedFilePath, MSG_FILEPATH_LEN_MAX, MSG_DATA_PATH"%s", fileName);

	MSG_DEBUG("retrievedFilePaths [%s]", retrievedFilePath);

	// create temp file
	if (!MsgOpenCreateAndOverwriteFile(retrievedFilePath, (char *)pRcvdBody, rcvdBodyLen)) {
		MSG_DEBUG( "_MmsUaInitMsgDecoder: creating temporary file failed(msgID=%d)\n", msgId);
		return false;
	}

	if (_MmsReadMsgBody(msgId, true, true, retrievedFilePath) == false) {
		MSG_DEBUG("The MMS Message might include drm contents!!!");

#ifdef __SUPPORT_DRM__
		if (MmsDrm2GetConvertState() == MMS_DRM2_CONVERT_REQUIRED) {
			bool bRetToConvert = true;
			MSG_MESSAGE_INFO_S pMsg = {0, };

			pMsg.msgId = msgId;

			bRetToConvert = MmsDrm2ConvertMsgBody(mmsHeader.msgType.szOrgFilePath);

			MmsDrm2SetConvertState(MMS_DRM2_CONVERT_FINISH);

			if (bRetToConvert) {
				remove(mmsHeader.msgType.szOrgFilePath);
				rename(MMS_DECODE_DRM_CONVERTED_TEMP_FILE, mmsHeader.msgType.szOrgFilePath);

				if (MmsDrm2ReadMsgConvertedBody(&pMsg, true, true, retrievedFilePath) == false) {
					MSG_DEBUG("MmsLoadMsg:MmsDrm2ReadMsgConvertedBody() returns false\n");
					goto ERR_MMS_UA_PROCESS_CONF;
				}
			}
		}
#endif
	}

	MSG_END();

	return true;

ERR_MMS_UA_PROCESS_CONF:
	{
		MmsMsg *pMsg;
		MmsPluginStorage::instance()->getMmsMessage(&pMsg);


		_MmsInitHeader();
		_MmsUnregisterDecodeBuffer();

#ifdef __SUPPORT_DRM__
		_MsgFreeDRMInfo(&pMsg->msgType.drmInfo);
#endif
		_MsgFreeBody(&pMsg->msgBody, pMsg->msgType.type);

		return false;
	}
}

