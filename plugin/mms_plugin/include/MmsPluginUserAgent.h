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

#ifndef MMS_PLUGIN_USERAGENT_H
#define MMS_PLUGIN_USERAGENT_H

#include "MsgThread.h"
#include "MsgMutex.h"
#include "MsgQueue.h"
#include "MmsPluginTypes.h"

class MmsPluginUaManager: public MsgThread {
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

		/* condition values */
		bool running;		/* flag for thread running */

		MsgSimpleQ<mmsTranQEntity> mmsTranQ; /* transaction q for mms plugin */
};

#endif /* MMS_PLUGIN_USERAGENT_H */
