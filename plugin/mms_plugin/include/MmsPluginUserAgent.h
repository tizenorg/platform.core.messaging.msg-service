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
