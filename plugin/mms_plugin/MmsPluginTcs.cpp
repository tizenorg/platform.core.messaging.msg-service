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

#include "TCSImpl.h"
#include "TCSErrorCodes.h"
#include "MmsPluginDebug.h"
#include "MsgUtilFile.h"
#include "MmsPluginTcs.h"

int MmsPluginTcsScanFile(const char *filepath, int *bLevel)
{
	MSG_BEGIN();
	TCSLIB_HANDLE hLib;
	TCSScanResult result;
	TCSDetected* pDetected;
	int rtn, i;
	int ret_b_level = -1;

	if (MsgAccessFile(filepath, R_OK) == false) {
		MSG_SEC_DEBUG("not exist source file [%s]", filepath);
		return -1;
	}

	MSG_SEC_DEBUG("Scanning file name : %s\n", filepath);

	hLib = TCSLibraryOpen();
	if(hLib == INVALID_TCSLIB_HANDLE) {
		MSG_DEBUG("TCSLibraryOpen error\n");
		return -1;
	}

	rtn = TCSScanFile(hLib, filepath, TCS_DTYPE_UNKNOWN, TCS_SA_SCANONLY, 1, &result);
	if(rtn == 0)
	{
		MSG_DEBUG("Detected malware number: %d\n", result.iNumDetected);
		i = result.iNumDetected;
		pDetected = result.pDList;
		while(i && pDetected)
		{
			int temp_b_level;
			int temp_s_class;
			MSG_SEC_DEBUG(" +-- Malware [%d] Name: %s\n", i, pDetected->pszName);
			MSG_DEBUG(" +-- Malware [%d] uAction: %u : 0x%04x\n", i, pDetected->uAction, pDetected->uAction);

			temp_b_level  = (pDetected->uAction & 0xFF00) >> 8;
			MSG_DEBUG(" +-- Malware [%d] Behavior level: %u\n", i, temp_b_level);

			if (ret_b_level == -1 || ret_b_level < temp_b_level) {
				ret_b_level = temp_b_level;
			}

			temp_s_class  = (pDetected->uAction & 0x00FF);
			MSG_DEBUG(" +-- Malware [%d] Severity class: %u\n", i, temp_s_class);

			pDetected = pDetected->pNext;
			i --;
		}

		result.pfFreeResult(&result);
	} else {
		MSG_DEBUG("TCSScanFile fail: err = %d\n", rtn);
	}

	TCSLibraryClose(hLib);

	if (bLevel)
		*bLevel = ret_b_level;

	MSG_END();

	return 0;
}
