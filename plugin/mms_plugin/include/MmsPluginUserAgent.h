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

#ifndef MMSPLUGINUSERAGENT_H
#define MMSPLUGINUSERAGENT_H

#include <pthread.h>
#include <vector>

#include "MsgDebug.h"
#include "MsgMutex.h"
#include "MsgQueue.h"
#include "MmsPluginTypes.h"
#include "MmsPluginConnManWrapper.h"
#include "MmsPluginHttp.h"
#include "MsgThread.h"

class MmsPluginUaManager: public MsgThread
{
	public:
		static MmsPluginUaManager *instance();
		virtual void start();

		void addMmsReqEntity(mmsTranQEntity req);
		void getMmsPduData(mmsTranQEntity *qEntity);
		bool processReceivedData(int msgId, char *pRcvdBody, int rcvdBodyLen, char *retrievedFilePath);

		void lock(){ mx.lock(); }
		void unlock(){ mx.unlock(); }
		void wait(){ cv.wait(mx.pMutex()); }
		void signal(){ cv.signal(); }

	private:
		MmsPluginUaManager();
		~MmsPluginUaManager();

		static MmsPluginUaManager *pInstance;
		virtual void run();

		Mutex mx;
		CndVar cv;

		MMS_NET_ERROR_T submitHandler(mmsTranQEntity *qEntity);
		MMS_NET_ERROR_T waitingConf(mmsTranQEntity *qEntity);

		// condition values
		bool running;		// flag for thread running

		MsgThdSafeQ <mmsTranQEntity> mmsTranQ; // transaction q for mms plugin
};

bool MsgOpenCreateAndOverwriteFile(char *szFullPath, char *szBuff, int totalLength);

#endif // MMSPLUGINUSERAGENT_H
