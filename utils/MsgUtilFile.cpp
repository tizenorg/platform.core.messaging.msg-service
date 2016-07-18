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

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/smack.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include <csr-content-screening.h>
#include <thumbnail_util.h>
#include <image_util.h>

#include "MsgStorageTypes.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgMmsTypes.h"
#include "MsgInternalTypes.h"
#include "MsgDrmWrapper.h"
#include "MsgMutex.h"

extern "C" {
	#include <aul.h>
}

MsgMutex g_mx;
MsgCndVar g_cv;

void thumbnail_completed_cb(thumbnail_util_error_e error, const char *request_id,
									int thumb_width, int thumb_height,
									unsigned char *thumb_data, int thumb_size, void *user_data)
{
	MSG_BEGIN();

	g_mx.lock();

	if (!user_data) {
		MSG_WARN("dstPath is NULL");
		g_cv.signal();
		g_mx.unlock();
		return;
	}

	MSG_DEBUG("=================[RESULT]");
	MSG_DEBUG("error_code [%d]", error);
	MSG_DEBUG("request id [%s]", request_id);
	MSG_DEBUG("width [%d], height [%d]", thumb_width, thumb_height);
	MSG_DEBUG("size [%d]", thumb_size);

	int ret = 0;
	ret = image_util_encode_jpeg(thumb_data, thumb_width, thumb_height, IMAGE_UTIL_COLORSPACE_BGRA8888, 100, (char *)user_data);
	if (ret != IMAGE_UTIL_ERROR_NONE)
		MSG_WARN("image_util_encode_jpeg() is failed");

	g_cv.signal();
	g_mx.unlock();

	MSG_END();
}

/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
bool MakeThumbnail(char *srcPath, char *dstPath)
{
	if (srcPath == NULL || dstPath == NULL) {
		MSG_SEC_DEBUG("Invalid Param src = %p, dst = %p", srcPath, dstPath);
		return false;
	}

	if (MsgAccessFile(srcPath, R_OK) == false) {
		MSG_SEC_DEBUG("not exist source file [%s]", srcPath);
		return false;
	}

	g_mx.lock();

	int time_ret = 0;

	int ret = THUMBNAIL_UTIL_ERROR_NONE;
	char *req_id = NULL;
	thumbnail_h thumb_h;
	thumbnail_util_create(&thumb_h);
	thumbnail_util_set_path(thumb_h, srcPath);

	ret = thumbnail_util_extract(thumb_h, thumbnail_completed_cb, dstPath, &req_id);
	thumbnail_util_destroy(thumb_h);
	if (req_id) {
		g_free(req_id);
		req_id = NULL;
	}

	if (ret != THUMBNAIL_UTIL_ERROR_NONE) {
		MSG_ERR("thumbnail_util_extract is failed");
		g_mx.unlock();
		return false;
	}

	time_ret = g_cv.timedwait(g_mx.pMsgMutex(), 5);

	g_mx.unlock();

	if (time_ret == ETIMEDOUT) {
		MSG_ERR("@@ WAKE by timeout@@");
		return false;
	}

	if (MsgAccessFile(dstPath, F_OK) == false) {
		MSG_SEC_DEBUG("not exist result file [%s]", dstPath);
		return false;
	}

	MSG_SEC_DEBUG("Make thumbnail: success [%s]", dstPath);
	return true;
}

/* File operation wrappers */
FILE *MsgOpenFile(const char *filepath, const char *opt)
{
	if (!filepath || !opt) {
		MSG_FATAL("Null parameter");
		return NULL;
	}

	MSG_SEC_DEBUG("[FILE] filepath : [%s], opt [%s]", filepath, opt);

	FILE *pFile = NULL;

	try {
		pFile = fopen(filepath, opt);
		MSG_DEBUG("[FILE] pFile [%p]", pFile);
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
		return NULL;
	}

	return pFile;
}

void MsgCloseFile(FILE *pFile)
{
	if (!pFile) {
		MSG_FATAL("NULL parameter");
		return;
	}

	MSG_DEBUG("[FILE] pFile [%p]", pFile);

	try {
		fclose(pFile);
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}
}

int MsgFseek(FILE *pFile, long int offset, int origin)
{
	if (!pFile) {
		MSG_FATAL("pFile NULL");
		return -1;
	}

	int ret = -1;

	MSG_DEBUG("[FILE] pFile [%p], offset [%d], origin [%d]", pFile, offset, origin);

	try {
		ret = fseek(pFile, offset, origin); /* return 0, if success. */
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
		return -1;
	}

	return ret;
}

size_t MsgWriteFile(const char *pData, size_t size, size_t count, FILE *pFile)
{
	if (!pData || !pFile) {
		MSG_FATAL("pData or pFile NULL");
		return 0;
	}

	size_t nWrite = 0;

	MSG_DEBUG("[FILE] pData [%p], size [%d], count [%d], pFile [%p]", pData, size, count, pFile);

	try {
		nWrite = fwrite(pData, size, count, pFile);
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}

	return nWrite;
}

size_t MsgReadFile(void *pData, size_t size, size_t count, FILE *pFile)
{
	if (!pData || !pFile) {
		MSG_FATAL("pData or pFile NULL");
		return 0;
	}

	size_t nRead = 0;

	try {
		nRead = fread(pData, size, count, pFile);
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}

	return nRead;
}

long int MsgFtell(FILE *pFile)
{
	if (!pFile) {
		MSG_FATAL("pFile NULL");
		return -1L;
	}

	long int ret = -1L; /* -1L return if error occured. */

	try {
		ret = ftell(pFile);
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}

	return ret;
}

int MsgFflush(FILE *pFile)
{
	if(!pFile) {
		MSG_FATAL("pFile NULL");
		return -1;
	}

	int ret = -1;

	try {
		ret = fflush(pFile); /* return 0 if success */
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}

	return ret;
}

int MsgFsync(FILE *pFile)
{
	if(!pFile) {
		MSG_FATAL("pFile NULL");
		return -1;
	}

	int ret = -1;

	try {
		ret = fdatasync(pFile->_fileno); /* return 0 if success */
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}

	return ret;
}

bool MsgCreateFileName(char *pFileName)
{
	if (pFileName == NULL) {
		MSG_DEBUG("[ERROR] pFileName is NULL");
		return false;
	}

	struct timespec ts;

	try {
		if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
			MSG_DEBUG("clock_gettime() error: %s", g_strerror(errno));
			return false;
		}

		/* Create Random Number */
		srandom((unsigned int)ts.tv_nsec);

		MSG_DEBUG("ts.tv_nsec : %d", ts.tv_nsec);

		/* between 1 - 1000000000 */
		snprintf(pFileName, MSG_FILENAME_LEN_MAX, "MSG_%lu.DATA", random()%1000000000+1);
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return false;
	}

	return true;
}


bool MsgOpenAndReadFile(const char *pFileName, char **ppData, int *pDataSize)
{
	if (!pFileName || !ppData || !pDataSize) {
		MSG_ERR("Invalid params!! pFileName=%x, ppData=%x, pDataSize=%x", pFileName, ppData, pDataSize);
		return false;
	}

	MSG_DEBUG("MsgOpenAndReadFile");

	FILE *pFile = NULL;

	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s%s", MSG_IPC_DATA_PATH, pFileName);
	MSG_SEC_DEBUG("open file name: %s", fullPath);

	struct stat st;
	if (stat(fullPath, &st) != 0) {
		MSG_SEC_ERR("stat(%s, &st) != 0", fullPath);
		return false;
	}
	if (S_ISDIR(st.st_mode)) {
		MSG_ERR("S_ISDIR(st.st_mode)");
		return false;
	}

	pFile = MsgOpenFile(fullPath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", g_strerror(errno));
		return false;
	}

	int FileSize = MsgFtell(pFile);

	if (FileSize <= 0) {
		MSG_DEBUG("Filesize is error : %d", FileSize);
		*pDataSize = 0;
		MsgCloseFile(pFile);
		return false;
	}

	*ppData = new char[FileSize+1];
	memset(*ppData, 0x00, (FileSize+1));

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File seek Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgReadFile(*ppData, sizeof(char), FileSize, pFile) != (size_t)FileSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", g_strerror(errno));
		return false;
	}

	*pDataSize = FileSize;

	MsgCloseFile(pFile);

	return true;
}


bool MsgReadFileForDecode(FILE *pFile, char *pBuf, int length, int *nSize)
{
	MSG_BEGIN();

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MSG_DEBUG("File Seek Error: %s", g_strerror(errno));
		MsgCloseFile(pFile);
		return false;
	}

	*nSize = MsgReadFile(pBuf, sizeof(char), length, pFile);

	MSG_END();
	return true;
}


bool MsgWriteIpcFile(const char *pFileName, const char *pData, int DataSize)
{
	if (!pFileName) {
		MSG_DEBUG("NULL parameter, pFileName [%p], pData [%p]", pFileName, pData);
		return false;
	}

	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s%s", MSG_IPC_DATA_PATH, pFileName);

	FILE *pFile = MsgOpenFile(fullPath, "wb+");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Write Error: %s", g_strerror(errno));
		return false;
	}

	MsgFflush(pFile);
	MsgCloseFile(pFile);

	return true;
}

int MsgReadSmilFile(const char *pFileName, char **ppData)
{
	if (!pFileName) {
		MSG_DEBUG("pFileName is NULL");
		return -1;
	}

	int	nSize = 0;
	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s%s", MSG_SMIL_FILE_PATH, pFileName);

	MSG_SEC_DEBUG("open file name: %s", fullPath);

	FILE *pFile = MsgOpenFile(fullPath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", g_strerror(errno));
		return -1;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", g_strerror(errno));
		return -1;
	}

	int FileSize = MsgFtell(pFile);

	if (FileSize <= 0) {
		MSG_DEBUG("Filesize is error : %d", FileSize);
		MsgCloseFile(pFile);
		return FileSize;
	}

	*ppData = new char[FileSize + 1];
	memset(*ppData, 0x00, (FileSize+1));

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Sead Error: %s", g_strerror(errno));
		return -1;
	}

	if (MsgReadFile(*ppData, sizeof(char), FileSize, pFile) != (size_t)FileSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", g_strerror(errno));
		return -1;
	}

	/* ppData[FileSize] = '\0'; */

	nSize = FileSize;
	MsgCloseFile(pFile);

	return nSize;
}


bool MsgWriteSmilFile(const char *pFilePath, char *pData, int DataSize)
{
	if(!pFilePath) {
		MSG_DEBUG("pFilePath is NULL");
		return false;
	}

#if 0
	if (mkdir(MSG_SMIL_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
		if (errno == EEXIST) {
			MSG_SEC_DEBUG("The %s already exists", MSG_SMIL_FILE_PATH);
		} else {
			MSG_SEC_DEBUG("Error while mkdir %s", MSG_SMIL_FILE_PATH);
		}
	}
#endif

	FILE *pFile = MsgOpenFile(pFilePath, "wb+");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Write Error: %s", g_strerror(errno));
		return false;
	}

	MsgFflush(pFile);
	MsgCloseFile(pFile);

	return true;
}


void MsgDeleteFile(const char *pFileName)
{
	if (!pFileName) {
		MSG_FATAL("pFileName is NULL");
		return;
	}

	if (strlen(pFileName) == 0) {
		MSG_FATAL("pFileName has zero length");
		return;
	}

	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	try {
		snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s%s", MSG_IPC_DATA_PATH, pFileName);

		MSG_SEC_DEBUG("%s", fullPath);

		if (remove(fullPath) != 0)
			MSG_SEC_ERR("File Delete Error [%s]: %s", fullPath, g_strerror(errno));
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}
}


void MsgDeleteSmilFile(const char *pFileName)
{
	if (!pFileName) {
		MSG_FATAL("pFileName NULL");
		return;
	}

	try {
		char fullPath[MAX_FULL_PATH_SIZE] = {0};

		snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s%s", MSG_SMIL_FILE_PATH, pFileName);

		if (remove(fullPath) != 0)
			MSG_SEC_ERR("File Delete Error [%s]: %s", fullPath, g_strerror(errno));
	} catch (exception &e) {
		MSG_FATAL("%s", e.what());
	}
}


bool MsgGetFileSize(const char *pFilePath, int *nSize)
{
	if (!pFilePath) {
		MSG_FATAL("pFileName NULL");
		return false;
	}

	FILE *pFile = NULL;

	pFile = MsgOpenFile(pFilePath, "rb");

	if (!pFile) {
		MSG_DEBUG("File Open error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Read Error: %s", g_strerror(errno));
		return false;
	}

	*nSize = MsgFtell(pFile);

	MsgCloseFile(pFile);

	return true;
}


FILE *MsgOpenMMSFile(char *pFileName)
{
	int len;

	if (!pFileName) {
		MSG_DEBUG("pFileName NULL: %s", g_strerror(errno));
		return NULL;
	}

	len = strlen(pFileName);

	for (int i = 0; i < len; i++) {
		switch (pFileName[i]) {
		case '*':
			pFileName[i] = '-';
			break;
		}
	}

	MSG_SEC_DEBUG("pFileName = %s", pFileName);

	char fullPath[MAX_FULL_PATH_SIZE+1] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE+1, "%s.mms", pFileName);

	FILE *pFile = MsgOpenFile(fullPath, "wb+");

	if (pFile == NULL) {
		MSG_ERR("File Open Error: %s", g_strerror(errno));
		return NULL;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_ERR("File Read Error: %s", g_strerror(errno));
		return NULL;
	}

	return pFile;
}


bool MsgWriteDataFromEncodeBuffer(FILE *pFile, char *pInBuffer, int *pPtr, int maxLen, int *pOffset )
{
	if (!pFile || !pPtr || !pInBuffer || !pOffset) {
		MSG_FATAL(" NULL parameter passed");
		return false;
	}

	MSG_DEBUG("MsgWriteDataFromEncodeBuffer:");
	MSG_DEBUG("pInBuffer %x", pInBuffer);
	MSG_DEBUG("pPtr %d",  (*pPtr));
	MSG_DEBUG("before to fwite %x", pFile);

	if (MsgWriteFile(pInBuffer, sizeof(char), (*pPtr), pFile) != (size_t)(*pPtr)) {
		MSG_FATAL("MsgWriteFile failed");
		return false;
	}

	MSG_DEBUG("after to fwite \n");

	MsgFflush(pFile);

	memset(pInBuffer, 0, maxLen);

	*pPtr = 0;

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MSG_FATAL("MsgFseek failed");
		return false;
	}

	*pOffset = MsgFtell(pFile);

	if (*pOffset == -1L) {
		MSG_FATAL("MsgFtell failed");
		return false;
	}

	return true;
}


bool MsgOpenCreateAndOverwriteFile(char *pFullPath, char *pBuff, int TotalLength)
{
	FILE *pFile = NULL ;

	if ((pFile = MsgOpenFile(pFullPath, "wb+")) == NULL) {
		MSG_FATAL("MsgOpenFile errer");
		return false;
	}

	if (MsgWriteFile(pBuff, sizeof(char), TotalLength, pFile) != (size_t)TotalLength) {
		MsgCloseFile(pFile);
		return false;
	}

	MsgFsync(pFile);	/* file is written to device immediately, it prevents missing file data from unexpected power off */
	MsgFflush(pFile);
	MsgCloseFile(pFile);

	return true;
}


char *MsgOpenAndReadMmsFile(const char *szFilePath, int offset, int size, int *npSize)
{
	FILE *pFile = NULL;
	char *pData = NULL;
	int	readSize = 0;

	if (szFilePath == NULL) {
		MSG_ERR("szFilePath id NULL");
		goto __CATCH;
	}

	*npSize = 0;

	pFile = MsgOpenFile(szFilePath, "rb");

	if (pFile == NULL) {
		MSG_ERR("Can't open file: %s", g_strerror(errno));
		goto __CATCH;
	}

	if (size == -1) {
		if (MsgGetFileSize(szFilePath, & readSize) == false) {
			MSG_DEBUG("MsgGetFileSize: failed");
			goto __CATCH;
		}
	} else {
		readSize = size;
	}
/* restore Kies backup data size greater than FM_READ_WRITE_BUFFER_MAX */
#if 0
	if (readSize > FM_READ_WRITE_BUFFER_MAX) {
		MSG_DEBUG("MsgOpenAndReadMmsFile: File size tried to read too big");
		goto __CATCH;
	}
#endif

	pData = (char *)calloc(1, readSize + 1);
	if ( NULL == pData ) {
		MSG_ERR("pData MemAlloc Fail : %s", g_strerror(errno) );
		goto __CATCH;
	}

	if (MsgFseek(pFile, offset, SEEK_SET) < 0) {
		MSG_ERR("FmSeekFile failed : %s", g_strerror(errno) );
		goto __CATCH;
	}

	*npSize = MsgReadFile(pData, sizeof(char), readSize, pFile);

	MsgCloseFile(pFile);

	pFile = NULL;

	*(pData + (*npSize)) = '\0';

	return pData;

__CATCH:

	if (pData) {
		free(pData);
		pData = NULL;
	}

	*npSize = 0;

	if (pFile != NULL) {
		MsgCloseFile(pFile);
		pFile = NULL;
	}

	return NULL;
}

/* it is equivalent to "rm -rf pDirPath" */
int MsgRmRf(char *pDirPath)
{
	struct dirent *d = NULL;
	struct dirent entry;
	DIR *dir;

	dir = opendir(pDirPath);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", g_strerror(errno));
		return -1;
	}

	int size = strlen(pDirPath) + 256;

	char *path = (char*)malloc(size);

	if (path == NULL) {
		MSG_DEBUG("path is NULL");
		closedir(dir);
		return -1;
	}

	bzero(path, size);

	for (readdir_r(dir, &entry, &d); d != NULL; readdir_r(dir, &entry, &d)) {
		if (d->d_type == DT_DIR) {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);

			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			MsgRmRf(path);

			if (rmdir(path) != 0) {
				if (path != NULL)
					free(path);

				closedir(dir);

				MSG_FATAL("error rmdir: %s", g_strerror(errno));

				return -1;
			}
		} else {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);

			if (MsgDrmIsDrmFile(path))
				MsgDrmUnregisterFile(path);

			if (remove(path) != 0) {
				if (path != NULL)
					free(path);

				closedir(dir);

				MSG_FATAL("error remove: %s", g_strerror(errno));

				return -1;
			}
		}
		d = NULL;
	}

	closedir(dir);

	if (path != NULL)
		free(path);

	return 0;
}


int MsgGetFileSize(const char *pFileName)
{
	struct stat file_stat;

	if (lstat(pFileName, &file_stat)) {
		MSG_FATAL("error lstat: %s", g_strerror(errno));
		return -1;
	}

	return file_stat.st_size;
}


/* it is equivalent to "du dir_path" */
unsigned int MsgDu(const char *pDirPath)
{
	struct dirent *d = NULL;
	struct dirent entry;
	DIR *dir;

	dir = opendir(pDirPath);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", g_strerror(errno));
		return -1;
	}

	int size = strlen(pDirPath) + 256;
	char *path = (char*)malloc(size);
	if (path == NULL) {
		closedir(dir);
		return -1;
	}

	bzero(path, size);

	unsigned int totalFileSize = 0;

	for (readdir_r(dir, &entry, &d); d != NULL; readdir_r(dir, &entry, &d)) {
		if(d->d_type == DT_DIR) {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);

			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			unsigned int dirSize = MsgDu(path);

			if (dirSize == 0) {
				MSG_FATAL("error MsgDu");
				closedir(dir);
				free(path);
				return dirSize;
			}

			totalFileSize += dirSize;
		} else {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);
			int fileSize = MsgGetFileSize(path);

			if (fileSize < 0) {
				MSG_FATAL("error MsgGetFileSize");
				closedir(dir);
				free(path);
				return fileSize;
			}

			totalFileSize += fileSize;
		}
		d = NULL;
	}

	closedir(dir);

	free(path);

	return totalFileSize;
}


bool MsgAppendFile(const char *pFilePath, const char *pData, int DataSize)
{
	if (!pFilePath) {
		MSG_FATAL("NULL check error, pFileName %p, pData %p", pFilePath, pData);
		return false;
	}

	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE, "%s", pFilePath);

	FILE *pFile = MsgOpenFile(fullPath, "a+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Sead Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Write Error: %s", g_strerror(errno));
		return false;
	}

	MsgFsync(pFile);	/*file is written to device immediately, it prevents missing file data from unexpected power off */
	MsgFflush(pFile);
	MsgCloseFile(pFile);
	return true;
}

void MsgMmsInitDir()
{
	struct dirent *d = NULL;
	struct dirent entry;
	DIR* dir = NULL;

	dir = opendir(MSG_DATA_PATH);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", g_strerror(errno));
		return;
	}

	/* Remove temporal Mms folder */
	for (readdir_r(dir, &entry, &d); d != NULL; readdir_r(dir, &entry, &d)) {
		if (d->d_type == DT_DIR) {
			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			if(strstr(d->d_name, ".dir") != NULL) {
				char filePath[MSG_FILEPATH_LEN_MAX] = {0, };
				snprintf(filePath, MSG_FILEPATH_LEN_MAX, "%s%s", MSG_DATA_PATH, d->d_name);

				MsgRmRf(filePath);
				rmdir(filePath);
			}
		}
		d = NULL;
	}

	closedir(dir);
}

/* mode : R_OK, W_OK, X_OK, or the existence test F_OK. */
bool MsgAccessFile(const char *filepath, int mode)
{
	int ret;
	if (filepath == NULL) {
		MSG_DEBUG("filepath is NULL");
		return false;
	}

	MSG_SEC_DEBUG("request access path = %s, mode = %d", filepath, mode);

	ret = access(filepath, mode);


	if (ret) {
		MSG_DEBUG("Fail to access file, ret = %d", ret);
		return false;
	}

	return true;
}


bool MsgChmod(const char *filepath, int mode)
{
#if 0
	struct stat lstat_info;
	struct stat fstat_info;
	int fd;

	if (lstat(filepath, &lstat_info) == -1) {
		MSG_SEC_DEBUG("No such file as [%s].", filepath);
		return false;
	}

	fd = open(filepath, O_RDONLY);

	if (fd == -1) {
		MSG_SEC_DEBUG("Fail to open [%s].", filepath);
		return false;
	}

	if (fstat(fd, &fstat_info) == -1) {
		MSG_SEC_DEBUG("Fail to fstat [%s].", filepath);
		close(fd);
		return false;
	}

	if (lstat_info.st_mode == fstat_info.st_mode &&
			lstat_info.st_ino == fstat_info.st_ino  &&
			lstat_info.st_dev == fstat_info.st_dev) {
		if (fchmod(fd, mode) < 0) {
			MSG_SEC_DEBUG("Fail to fchmod [%s].", filepath);
			close(fd);
			return false;
		}
	}

	close(fd);
#endif
	return true;
}


bool MsgChown(const char *filepath, int uid, int gid)
{
	struct stat lstat_info;
	struct stat fstat_info;
	int fd;

	if (lstat(filepath, &lstat_info) == -1) {
		MSG_SEC_INFO("No such file as [%s].", filepath);
		return false;
	}

	fd = open(filepath, O_RDONLY);

	if (fd == -1) {
		MSG_SEC_INFO("Fail to open [%s].", filepath);
		return false;
	}

	if (fstat(fd, &fstat_info) == -1) {
		MSG_SEC_INFO("Fail to fstat [%s].", filepath);
		close(fd);
		return false;
	}

	if (lstat_info.st_mode == fstat_info.st_mode &&
			lstat_info.st_ino == fstat_info.st_ino  &&
			lstat_info.st_dev == fstat_info.st_dev) {
		if (fchown(fd, uid, gid) < 0) {
			MSG_SEC_INFO("Fail to fchown [%s].", filepath);
			close(fd);
			return false;
		}
	}

	close(fd);
	return true;
}

bool MsgCreateFile(const char *pFilePath, char *pData, int DataSize)
{
	if(!pFilePath) {
		MSG_DEBUG("pFilePath is NULL");
		return false;
	}

	FILE *pFile = MsgOpenFile(pFilePath, "wb+");

	if (pFile == NULL) {
		MSG_ERR("File Open Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_ERR("File Seek Error: %s", g_strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_ERR("File Write Error: %s", g_strerror(errno));
		return false;
	}

	MsgFflush(pFile);
	MsgFsync(pFile);
	MsgCloseFile(pFile);

	return true;
}

char *MsgGetDirName(char *file_path)
{
	return g_path_get_dirname(file_path);
}


char *MsgGetFileName(char *file_path)
{
	return g_path_get_basename(file_path);
}


int MsgCheckFilepathSmack(const char *app_smack_label, char *file_path)
{
	int err = MSG_SUCCESS;

	char *path_smack_label = NULL;
	char *dir_smack_label = NULL;
	char *dir_name = NULL;

	if (!file_path || file_path[0] == '\0') {
		return MSG_SUCCESS;
	}

	struct stat st;
	if (stat(file_path, &st) != 0) {
		MSG_SEC_ERR("stat(%s, &st) != 0", file_path);
		return MSG_ERR_PERMISSION_DENIED;
	}
	if (S_ISDIR(st.st_mode)) {
		MSG_ERR("S_ISDIR(st.st_mode)");
		return MSG_ERR_INVALID_PARAMETER;
	}

	dir_name = MsgGetDirName(file_path);
	if (!dir_name || !g_strcmp0(dir_name, file_path)) {
		MSG_SEC_ERR("!dir_name || !g_strcmp0(dir_name, %s)", file_path);
		err = MSG_ERR_INVALID_PARAMETER;
		goto __RETURN;
	}

	smack_getlabel(dir_name, &dir_smack_label, SMACK_LABEL_ACCESS);
	if (dir_smack_label == NULL) {
		MSG_ERR("smack_getlabel failed (dir_smack_label)");
		err = MSG_ERR_PERMISSION_DENIED;
		goto __RETURN;
	}

	if (smack_have_access(app_smack_label, dir_smack_label, "RX") < 1) {
		MSG_ERR("smack_have_access failed (dir_smack_label)");
		err = MSG_ERR_PERMISSION_DENIED;
		goto __RETURN;
	}

	smack_getlabel(file_path, &path_smack_label, SMACK_LABEL_ACCESS);
	if (path_smack_label == NULL) {
		MSG_ERR("smack_getlabel failed (path_smack_label)");
		err = MSG_ERR_PERMISSION_DENIED;
		goto __RETURN;
	}

	if (smack_have_access(app_smack_label, path_smack_label, "R") < 1) {
		MSG_ERR("smack_have_access failed (path_smack_label)");
		err = MSG_ERR_PERMISSION_DENIED;
		goto __RETURN;
	}

	MSG_DEBUG("smack_have_access pass successfully");

__RETURN:
	MSG_FREE(path_smack_label);
	MSG_FREE(dir_smack_label);
	MSG_FREE(dir_name);
	return err;
}


void MsgGetMimeType(char *filePath, char *mimeType, int size)
{
	aul_get_mime_from_file(filePath, mimeType, size);
}


int MsgTcsScanFile(const char *filepath, int *bLevel)
{
	MSG_BEGIN();
	csr_cs_context_h csr_handle = NULL;
	csr_cs_malware_h detected = NULL;
	int ret = 0;
	int ret_b_level = -1;

	if (MsgAccessFile(filepath, R_OK) == false) {
		MSG_SEC_DEBUG("not exist source file [%s]", filepath);
		return -1;
	}

	MSG_SEC_DEBUG("Scanning file name : %s\n", filepath);

	ret = csr_cs_context_create(&csr_handle);
	if (ret != CSR_ERROR_NONE) {
		MSG_DEBUG("csr_cs_context_create error: err = %d\n", ret);
		return -1;
	}

	ret = csr_cs_scan_file(csr_handle, filepath, &detected);
	if (ret == CSR_ERROR_NONE) {
		if (detected) {
			csr_cs_severity_level_e severity;
			char *name = NULL;

			ret = csr_cs_malware_get_severity(detected, &severity);
			if (ret != CSR_ERROR_NONE) {
				MSG_DEBUG("csr_cs_malware_get_severity error: err = %d\n", ret);
			}

			ret = csr_cs_malware_get_name(detected, &name);
			if (ret != CSR_ERROR_NONE) {
				MSG_DEBUG("csr_cs_malware_get_name error: err = %d\n", ret);
			}

			MSG_SEC_DEBUG(" +-- Malware Name: [%s]\n", name);
			MSG_DEBUG(" +-- Malware Severity class: %d\n", severity);

			ret_b_level = (int)severity;

			if (name) {
				free(name);
				name = NULL;
			}
		}
	} else {
		MSG_DEBUG("csr_cs_scan_file fail: err = %d\n", ret);
	}

	ret = csr_cs_context_destroy(csr_handle);
	if (ret != CSR_ERROR_NONE) {
		MSG_DEBUG("csr_cs_context_destroy error: err = %d\n", ret);
	}

	if (bLevel)
		*bLevel = ret_b_level;

	MSG_END();

	return 0;
}
