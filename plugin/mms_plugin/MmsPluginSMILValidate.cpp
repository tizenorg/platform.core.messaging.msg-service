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

#include "MsgMmsMessage.h"
#include "MmsPluginDebug.h"
#include "MmsPluginSMILValidate.h"
#include "MmsPluginSmil.h"


#define MSG_RETURN_VAL_IF_FAIL(Expr, Val)			 		\
		if (!(Expr)) {										\
			MSG_DEBUG("%s:[%s] Failed - %d\n", __FUNCTION__, __LINE__, Val); \
			return Val;										\
		};

MSG_SMIL_ERR_E MsgMMSCreateSMIL(MMS_MESSAGE_DATA_S *pMsgData)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_UNKNOWN;
	bool bRegAdded = false;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	MSG_RETURN_VAL_IF_FAIL((strlen(pMsgData->szSmilFilePath)> 0), MSG_SMIL_ERR_INVALID_SMIL_FILE_PATH);

	eRet = _MsgMMSValidateSMILRootLayout(pMsgData);
	if (eRet != MSG_SMIL_SUCCESS) {
		MSG_DEBUG("MsgMMSCreateSMIL: Root Layout Information Invalid eRet =%d\n", eRet);
		return eRet;
	}

	eRet = _MsgMMSValidateSMILRegion(pMsgData, &bRegAdded);
	if (eRet != MSG_SMIL_SUCCESS) {
		MSG_DEBUG("MsgMMSCreateSMIL: Region Information Invalid eRet =%d\n", eRet);
		return eRet;
	}

	eRet = _MsgMMSValidateSMILPage(pMsgData, bRegAdded);
	if (eRet != MSG_SMIL_SUCCESS) {
		MSG_DEBUG("MsgMMSCreateSMIL: Page Information Invalid eRet =%d\n", eRet);
		return eRet;
	}

	if (MMSGenerateSmilBuffer(pMsgData)) {
		MSG_DEBUG("MsgMMSCreateSMIL: Generate SMIL Buffer is success eRet =%d\n", eRet);
		eRet = MSG_SMIL_SUCCESS;
	} else {
		MSG_DEBUG("MsgMMSCreateSMIL: Generate SMIL Buffer failed eRet =%d\n", eRet);
		eRet = MSG_SMIL_ERR_UNKNOWN;
	}

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E _MsgMMSValidateSMILRootLayout(MMS_MESSAGE_DATA_S *pMsgData)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_ROOTLAYOUT;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);

	if ((pMsgData->rootlayout.width.value >  0) &&
		(pMsgData->rootlayout.height.value> 0)) {
		if (pMsgData->rootlayout.width.bUnitPercent == pMsgData->rootlayout.height.bUnitPercent) {
			MSG_DEBUG("_MsgMMSValidateSMILRootLayout: Root Layout Information Valid \n");
			eRet = MSG_SMIL_SUCCESS;
		}
	} else  {
		MSG_DEBUG("_MsgMMSValidateSMILRootLayout: Root Layout Information not Present \n");
		eRet = _MsgMMSAddDefaultSMILRootLayout(pMsgData);
	}

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E _MsgMMSAddDefaultSMILRootLayout(MMS_MESSAGE_DATA_S *pMsgData)
{
	MSG_BEGIN();

	char *pContent = (char *)MSG_SMIL_ROOT_LAYOUT_BG_COLOR;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);

	pMsgData->rootlayout.width.value = MSG_SMIL_ROOT_LAYOUT_WIDTH;
	pMsgData->rootlayout.width.bUnitPercent = MSG_SMIL_ROOT_LAYOUT_IN_PERCENT;

	pMsgData->rootlayout.height.value = MSG_SMIL_ROOT_LAYOUT_HEIGHT;
	pMsgData->rootlayout.height.bUnitPercent = MSG_SMIL_ROOT_LAYOUT_IN_PERCENT;

	pMsgData->rootlayout.bgColor = MmsSmilAtoIHexa((char *)&pContent[1]);

	MSG_END();
	return MSG_SMIL_SUCCESS;
}

MSG_SMIL_ERR_E _MsgMMSValidateSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool *pbRegAdded)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_REGION_INFO;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	MSG_RETURN_VAL_IF_FAIL((pbRegAdded!= NULL), MSG_SMIL_ERR_INVALID_PARAM);

	if (pMsgData->regionCnt == 0) {
		bool bTextReg = MSG_SMIL_TEXT_ON_TOP;
		bool bfullReg = false;
		bool btwoReg = false;

		MSG_DEBUG("_MsgMMSValidateSMILRegion: Region Information not Present \n");

		for (int PgIdx = 0; PgIdx < pMsgData->pageCnt; ++PgIdx) {
			MMS_PAGE_S *pPage = _MsgMmsGetPage(pMsgData, PgIdx);
			int nMediaCnt = 0;

			MSG_RETURN_VAL_IF_FAIL((NULL != pPage), MSG_SMIL_ERR_INVALID_PAGE_INFO);

			for (int MediaIdx = 0; MediaIdx < pPage->mediaCnt; ++MediaIdx) {
				MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, MediaIdx);

				MSG_RETURN_VAL_IF_FAIL((NULL != pMedia), MSG_SMIL_ERR_INVALID_PAGE_INFO);

				if (pMedia->mediatype != MMS_SMIL_MEDIA_AUDIO)
					nMediaCnt++;
			}

			if (nMediaCnt == 1)
				bfullReg = true;
			else if (nMediaCnt == 2)
				btwoReg = true;
			else if (nMediaCnt > 2) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region Information\n");
				eRet = MSG_SMIL_ERR_INVALID_PAGE_INFO;
				return eRet;
			}
		}

		if (bfullReg) {
			eRet = _MsgMMSAddDefaultFullSMILRegion(pMsgData);
			if (eRet != MSG_SMIL_SUCCESS) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region Information\n");
				return eRet;
			}
		}
		if (btwoReg) {
			eRet = _MsgMMSAddDefaultFirstSMILRegion(pMsgData, bTextReg);
			if (eRet != MSG_SMIL_SUCCESS) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region Information\n");
				return eRet;
			}

			eRet = _MsgMMSAddDefaultSecondSMILRegion(pMsgData,!bTextReg);
			if (eRet != MSG_SMIL_SUCCESS) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region Information\n");
				return eRet;
			}
		}
		*pbRegAdded  = true;
		eRet = MSG_SMIL_SUCCESS;
	} else {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Region Information Present \n");

		for (int Idx = 0; Idx< pMsgData->regionCnt; ++Idx) {
			MMS_SMIL_REGION *pRegion = _MsgMmsGetSmilRegion(pMsgData, Idx);

			if ((pRegion->width.value == 0) ||
					(pRegion->height.value == 0)) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region information\n");

				return eRet;
			}

			if (pMsgData->rootlayout.width.bUnitPercent == true) {
				if ((pRegion->width.value > 100) ||
					(pRegion->height.value > 100) ||
					(pRegion->nLeft.value > 100) ||
					(pRegion->nTop.value > 100)) {
					MSG_DEBUG("_MsgMMSValidateSMILRegion: Region and root layout information does not map\n");

					return eRet;
				}
			} else {
				if (((pRegion->width.bUnitPercent == true) && (pRegion->width.value > 100)) ||
					((pRegion->height.bUnitPercent == true) && (pRegion->height.value > 100)) ||
					((pRegion->nLeft.bUnitPercent == true) && (pRegion->nLeft.value > 100)) ||
					((pRegion->nTop.bUnitPercent == true) && (pRegion->nTop.value > 100))) {
					MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region information\n");

					return eRet;
				}
			}
		}

		eRet = MSG_SMIL_SUCCESS;
	}

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E _MsgMMSAddDefaultFirstSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_REGION_INFO;
	MMS_SMIL_REGION *pRegion = NULL;
	char *pszfit = (char *)MSG_SMIL_REG_FIT_TYPE;
	char *pContent = (char *)MSG_SMIL_REG_BG_COLOR;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);

	if (pRegion == NULL) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Memory Allocation failed\n");
		return eRet;
	}

	if (!strcmp(pszfit, "meet"))
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_MEET;
	else
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_HIDDEN;
	pRegion->bgColor = MmsSmilAtoIHexa((char *)&pContent[1]);

	pRegion->nLeft.value = MSG_SMIL_FIRST_REG_LEFT;
	pRegion->nLeft.bUnitPercent = true;
	pRegion->nTop.value = MSG_SMIL_FIRST_REG_TOP;
	pRegion->nTop.bUnitPercent = true;
	pRegion->width.value = MSG_SMIL_FIRST_REG_WIDTH;
	pRegion->width.bUnitPercent = true;
	pRegion->height.value = MSG_SMIL_FIRST_REG_HEIGHT;
	pRegion->height.bUnitPercent = true;

	if (bTextReg)
		snprintf(pRegion->szID, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_TXT_REG);
	else
		snprintf(pRegion->szID, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_IMG_REG);

	if (_MsgMmsAddRegion(pMsgData,pRegion) != MSG_SUCCESS) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Add Region failed\n");
		free(pRegion);

		return eRet;
	}

	eRet = MSG_SMIL_SUCCESS;

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E _MsgMMSAddDefaultSecondSMILRegion(MMS_MESSAGE_DATA_S *pMsgData, bool bTextReg)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_REGION_INFO;
	MMS_SMIL_REGION *pRegion = NULL;
	char *pszfit = (char *)MSG_SMIL_REG_FIT_TYPE;
	char *pContent = (char *)MSG_SMIL_REG_BG_COLOR;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);

	if (pRegion == NULL) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Memory Allocation failed\n");
		return eRet;
	}

	if (!strcmp(pszfit, "meet"))
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_MEET;
	else
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_HIDDEN;
	pRegion->bgColor = MmsSmilAtoIHexa((char *)&pContent[1]);

	pRegion->nLeft.value = MSG_SMIL_SECOND_REG_LEFT;
	pRegion->nLeft.bUnitPercent = true;
	pRegion->nTop.value = MSG_SMIL_SECOND_REG_TOP;
	pRegion->nTop.bUnitPercent = true;
	pRegion->width.value = MSG_SMIL_SECOND_REG_WIDTH;
	pRegion->width.bUnitPercent = true;
	pRegion->height.value = MSG_SMIL_SECOND_REG_HEIGHT;
	pRegion->height.bUnitPercent = true;

	if (bTextReg)
		snprintf(pRegion->szID, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_TXT_REG);
	else
		snprintf(pRegion->szID, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_IMG_REG);

	if (_MsgMmsAddRegion(pMsgData,pRegion) != MSG_SUCCESS) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Add Region failed\n");
		free(pRegion);

		return eRet;
	}

	eRet = MSG_SMIL_SUCCESS;

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E _MsgMMSAddDefaultFullSMILRegion(MMS_MESSAGE_DATA_S *pMsgData)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_REGION_INFO;
	MMS_SMIL_REGION *pRegion = NULL;
	char *pszfit = (char *)MSG_SMIL_REG_FIT_TYPE;
	char *pContent = (char *)MSG_SMIL_REG_BG_COLOR;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	pRegion = (MMS_SMIL_REGION *)calloc(sizeof(MMS_SMIL_REGION), 1);

	if (pRegion == NULL) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Memory Allocation failed\n");
		return eRet;
	}

	if (!strcmp(pszfit, "meet"))
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_MEET;
	else
		pRegion->fit = MMSUI_IMAGE_REGION_FIT_HIDDEN;
	pRegion->bgColor = MmsSmilAtoIHexa((char *)&pContent[1]);

	pRegion->nLeft.value = MSG_SMIL_FULL_REG_LEFT;
	pRegion->nLeft.bUnitPercent = true;
	pRegion->nTop.value = MSG_SMIL_FULL_REG_TOP;
	pRegion->nTop.bUnitPercent = true;
	pRegion->width.value = MSG_SMIL_FULL_REG_WIDTH;
	pRegion->width.bUnitPercent = true;
	pRegion->height.value = MSG_SMIL_FULL_REG_HEIGHT;
	pRegion->height.bUnitPercent = true;

	snprintf(pRegion->szID, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_FULL_REG);

	if (_MsgMmsAddRegion(pMsgData,pRegion) != MSG_SUCCESS) {
		MSG_DEBUG("_MsgMMSValidateSMILRegion: Add Region failed\n");
		free(pRegion);

		return eRet;
	}

	eRet = MSG_SMIL_SUCCESS;

	MSG_END();
	return eRet;
}

MSG_SMIL_ERR_E  _MsgMMSValidateSMILPage(MMS_MESSAGE_DATA_S *pMsgData, bool bRegAdded)
{
	MSG_BEGIN();

	MSG_SMIL_ERR_E eRet = MSG_SMIL_ERR_INVALID_PAGE_INFO;

	MSG_RETURN_VAL_IF_FAIL((pMsgData!= NULL), MSG_SMIL_ERR_INVALID_PARAM);
	MSG_RETURN_VAL_IF_FAIL((pMsgData->pageCnt > 0), eRet);

	for (int PgIdx = 0; PgIdx < pMsgData->pageCnt; ++PgIdx) {
		MMS_PAGE_S *pPage = _MsgMmsGetPage(pMsgData, PgIdx);
		int PageRegionCnt = 0;
		int PageRegionIdx = 0;
		bool bVidExists = false;
		bool bImgExists = false;
		bool bTxtExists = false;
		bool bAudExists = false;
		bool bImgOrVidExists = false;
		bool bResetRegion = false;

		MSG_RETURN_VAL_IF_FAIL((NULL != pPage), MSG_SMIL_ERR_INVALID_PAGE_INFO);
		MSG_RETURN_VAL_IF_FAIL((pPage->nDur >= 0), MSG_SMIL_ERR_INVALID_PAGE_DUR);

		for (int MediaIdx = 0; MediaIdx < pPage->mediaCnt; ++MediaIdx) {
			MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, MediaIdx);

			MSG_RETURN_VAL_IF_FAIL((NULL != pMedia), MSG_SMIL_ERR_INVALID_PAGE_INFO);

			bResetRegion = false;
			if ((pMedia->mediatype != MMS_SMIL_MEDIA_AUDIO)) {
				if (bRegAdded) {
					bResetRegion = true;
				} else if ((strlen(pMedia->regionId) == 0)) {
					MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Region Information\n");
					return MSG_SMIL_ERR_INVALID_REGION_INFO;
				}
				PageRegionCnt++;
			}
		}

		for (int MediaIdx = 0; MediaIdx < pPage->mediaCnt; ++MediaIdx) {
			MMS_MEDIA_S *pMedia = _MsgMmsGetMedia(pPage, MediaIdx);

			MSG_RETURN_VAL_IF_FAIL((NULL != pMedia), MSG_SMIL_ERR_INVALID_PAGE_INFO);
			MSG_RETURN_VAL_IF_FAIL((strlen(pMedia->szFilePath) > 0), MSG_SMIL_ERR_INVALID_PAGE_INFO);

			switch (pMedia->mediatype) {
			case MMS_SMIL_MEDIA_IMG:
				{
					if ((bImgExists == true) || (bVidExists == true) ||
						(bImgOrVidExists== true)) {
						eRet = MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS;
					} else
						bImgExists = true;
				}
				break;
			case MMS_SMIL_MEDIA_AUDIO:
				{
					if ((bAudExists == true) || (bVidExists == true) ||
						(bImgOrVidExists== true)) {
						eRet = MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS;
					} else
						bAudExists = true;
				}
				break;
			case MMS_SMIL_MEDIA_VIDEO:
				{
					if ((bImgExists == true) || (bVidExists == true) ||
						(bImgOrVidExists== true)) {
						eRet = MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS;
					} else
						bVidExists = true;
				}
				break;
			case MMS_SMIL_MEDIA_TEXT:
				{
					if (bTxtExists == true) {
						eRet = MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS;
					} else
						bTxtExists = true;
				}
				break;
			case MMS_SMIL_MEDIA_IMG_OR_VIDEO:
				{
					if ((bImgExists == true) || (bVidExists == true) ||
						(bImgOrVidExists== true)) {
						eRet = MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS;
					} else
						bImgOrVidExists = true;
				}
				break;
			default:
				{
					MSG_DEBUG("_MsgMMSValidateSMILRegion: Invalid Media Information\n");
					return eRet;
				}
				break;
			}

			if (eRet == MSG_SMIL_ERR_SIMILAR_MEDIA_EXISTS) {
				MSG_DEBUG("_MsgMMSValidateSMILRegion: Similar Media Exists\n");
				return eRet;
			}

			if (bResetRegion) {
				if (PageRegionCnt > 1) {
					if (pMedia->mediatype == MMS_SMIL_MEDIA_AUDIO) {
						memset(pMedia->regionId,0x00, MAX_SMIL_REGION_ID);
						continue;
					}

					if (pMedia->mediatype == MMS_SMIL_MEDIA_TEXT)
						snprintf(pMedia->regionId, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_TXT_REG);
					else
						snprintf(pMedia->regionId, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_IMG_REG);
				} else
					snprintf(pMedia->regionId, MAX_SMIL_REGION_ID, "%s", MSG_SMIL_DEFAULT_FULL_REG);
			}
			PageRegionIdx++;
		}
	}

	eRet = MSG_SMIL_SUCCESS;

	MSG_END();

	return eRet;
}

