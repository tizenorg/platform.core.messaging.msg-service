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

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>	//sync()

#include "MsgStorageTypes.h"
#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "MsgMmsTypes.h"
#include "MsgInternalTypes.h"
#include "MsgDrmWrapper.h"


/*==================================================================================================
                                     FUNCTION IMPLEMENTATION
==================================================================================================*/
// File operation wrappers
FILE *MsgOpenFile(const char *filepath, const char *opt)
{
	if (!filepath || !opt) {
		MSG_FATAL("Null parameter");
		return NULL;
	}

	MSG_DEBUG("[FILE] filepath : [%s], opt [%s]", filepath, opt);

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
		ret = fseek(pFile, offset, origin); 		// return 0, if success.
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

	long int ret = -1L; // -1L return if error occured.

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
		ret = fflush(pFile);		// return 0 if success
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
		ret = fdatasync(pFile->_fileno);	// return 0 if success
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
			MSG_DEBUG("clock_gettime() error: %s", strerror(errno));
			return false;
		}

		// Create Random Number
		srandom((unsigned int)ts.tv_nsec);

		MSG_DEBUG("ts.tv_nsec : %d", ts.tv_nsec);

		// between 1 - 1000000000
		snprintf(pFileName, MSG_FILENAME_LEN_MAX, "MSG_%lu.DATA", random()%1000000000+1);
	} catch (exception& e) {
		MSG_FATAL("%s", e.what());
		return false;
	}

	return true;
}


bool MsgOpenAndReadFile(const char *pFileName, char **ppData, int *pDataSize)
{
	MSG_DEBUG("MsgOpenAndReadFile");

	FILE *pFile = NULL;

	char fullPath[MAX_FULL_PATH_SIZE] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE, MSG_IPC_DATA_PATH"%s", pFileName);
	MSG_DEBUG("open file name: %s", fullPath);


	pFile = MsgOpenFile(fullPath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", strerror(errno));
		return false;
	}

	int FileSize = MsgFtell(pFile);

	if (FileSize <= 0) {
		MSG_DEBUG("Filesize is error : %d", FileSize);
		*pDataSize = 0;
		MsgCloseFile(pFile);
		return false;
	}

	*ppData = new char[FileSize];

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File seek Error: %s", strerror(errno));
		return false;
	}

	if (MsgReadFile(*ppData, sizeof(char), FileSize, pFile) != (size_t)FileSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", strerror(errno));
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
		MSG_DEBUG("File Seek Error: %s", strerror(errno));
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

	snprintf(fullPath, MAX_FULL_PATH_SIZE, MSG_IPC_DATA_PATH"%s", pFileName);

	FILE *pFile = MsgOpenFile(fullPath, "wb+");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Write Error: %s", strerror(errno));
		return false;
	}

	MsgFflush(pFile);
	MsgCloseFile(pFile);

	if (chmod(fullPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) != 0) {
		MSG_DEBUG("File chmod Error: %s", strerror(errno));
	}

	if (chown(fullPath, 0, 6502 ) != 0) {
		MSG_DEBUG("File chown Error: %s", strerror(errno));
	}

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

	snprintf(fullPath, MAX_FULL_PATH_SIZE, MSG_SMIL_FILE_PATH"%s", pFileName);

	MSG_DEBUG("open file name: %s", fullPath);

	FILE *pFile = MsgOpenFile(fullPath, "rb");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", strerror(errno));
		return -1;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", strerror(errno));
		return -1;
	}

	int FileSize = MsgFtell(pFile);

	if (FileSize <= 0) {
		MSG_DEBUG("Filesize is error : %d", FileSize);
		MsgCloseFile(pFile);
		return FileSize;
	}

	*ppData = new char[FileSize + 1];

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Sead Error: %s", strerror(errno));
		return -1;
	}

	if (MsgReadFile(*ppData, sizeof(char), FileSize, pFile) != (size_t)FileSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", strerror(errno));
		return -1;
	}

	ppData[FileSize] = '\0';

	nSize = FileSize;

	MsgCloseFile(pFile);

	return nSize;
}


bool MsgWriteSmilFile(const char *pFilePath,char *pData, int DataSize)
{
	if(!pFilePath) {
		MSG_DEBUG("pFilePath is NULL");
		return false;
	}

	if (mkdir(MSG_SMIL_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
		if (errno == EEXIST) {
			MSG_DEBUG("The %s already exists", MSG_SMIL_FILE_PATH);
		} else {
			MSG_DEBUG("Error while mkdir %s", MSG_SMIL_FILE_PATH);
		}
	}

	FILE *pFile = MsgOpenFile(pFilePath, "wb+");

	if (pFile == NULL) {
		MSG_DEBUG("File Open Error: %s", strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_SET) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Seek Error: %s", strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Write Error: %s", strerror(errno));
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
		snprintf(fullPath, MAX_FULL_PATH_SIZE, MSG_IPC_DATA_PATH"%s", pFileName);

		MSG_DEBUG("%s", fullPath);

		if (remove(fullPath) != 0)
			MSG_FATAL("File Delete Error [%s]: %s", fullPath, strerror(errno));
	} catch (exception &e) {
		MSG_FATAL ("%s", e.what());
	}

}


void MsgDeleteSmilFile(const char *pFileName)
{
	if (!pFileName ) {
		MSG_FATAL("pFileName NULL");
		return;
	}

	try {
		char fullPath[MAX_FULL_PATH_SIZE] = {0};

		snprintf(fullPath, MAX_FULL_PATH_SIZE, MSG_SMIL_FILE_PATH"%s", pFileName);

		if (remove(fullPath) != 0)
			MSG_FATAL("File Delete Error [%s]: %s", fullPath, strerror(errno));
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
		MSG_DEBUG("File Open error: %s", strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_END) < 0) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Read Error: %s", strerror(errno));
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
		MSG_DEBUG("pFileName NULL: %s", strerror(errno));
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

	MSG_DEBUG("pFileName = %s", pFileName);

	char fullPath[MAX_FULL_PATH_SIZE+1] = {0};

	snprintf(fullPath, MAX_FULL_PATH_SIZE+1, "%s.mms", pFileName);

	FILE *pFile = MsgOpenFile(fullPath, "wb+");

	if (pFile == NULL) {
		MSG_FATAL("File Open Error: %s", strerror(errno));
		return NULL;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_DEBUG("File Read Error: %s", strerror(errno));
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

	memset( pInBuffer, 0, maxLen );

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
	mode_t file_mode;

	file_mode = (S_IRUSR | S_IWUSR);

	if ((pFile = MsgOpenFile(pFullPath, "wb+")) == NULL) {
		MSG_FATAL("MsgOpenFile errer");
		return false;
	}

	if (MsgWriteFile(pBuff, sizeof(char), TotalLength, pFile) != (size_t)TotalLength) {
		MsgCloseFile( pFile );
		return false;
	}

	MsgFsync(pFile);	//file is written to device immediately, it prevents missing file data from unexpected power off
	MsgFflush(pFile);
	MsgCloseFile(pFile);

	if (chmod(pFullPath, file_mode) < 0)
		MSG_FATAL("File chmod Error: %s", strerror(errno));

	return true;
}


char *MsgOpenAndReadMmsFile( const char *szFilePath, int offset, int size, int *npSize )
{
	FILE *pFile = NULL;
	char *pData = NULL;
	int	readSize = 0;

	if (szFilePath == NULL) {
		MSG_DEBUG("MsgOpenAndReadMmsFile: [ERROR] szFilePath id NULL");
		goto __CATCH;
	}

	*npSize = 0;

	pFile = MsgOpenFile( szFilePath, "rb" );

	if (pFile == NULL) {
		MSG_DEBUG("MsgOpenAndReadMmsFile: [ERROR] Can't open filepath", strerror(errno));
		goto __CATCH;
	}

	if( size == -1 ) {
		if (MsgGetFileSize(szFilePath, & readSize) == false) {
			MSG_DEBUG("MsgGetFileSize: failed");
			goto __CATCH;
		}
	} else {
		readSize = size;
	}

	if (readSize > FM_READ_WRITE_BUFFER_MAX) {
		MSG_DEBUG("MsgOpenAndReadMmsFile: File size tried to read too big");
		goto __CATCH;
	}

	pData = (char *)malloc(readSize + 1);
	if ( NULL == pData ) {
		MSG_DEBUG( "MsgOpenAndReadMmsFile: [ERROR] pData MemAlloc Fail", strerror(errno) );
		goto __CATCH;
	}
	memset( pData, 0, readSize + 1 );

	if (MsgFseek( pFile, offset, SEEK_SET) < 0) {
		MSG_DEBUG( "MsgOpenAndReadMmsFile: [ERROR] FmSeekFile failed", strerror(errno) );
		goto __CATCH;
	}

	*npSize = MsgReadFile(pData, sizeof(char), readSize, pFile);

	MsgCloseFile(pFile);

	pFile = NULL;

	*(pData + (*npSize)) = '\0';

	return pData;

__CATCH:

	if (pData) {
		free( pData );
		pData = NULL;
	}

	*npSize = 0;

	if (pFile != NULL) {
		MsgCloseFile( pFile );
		pFile = NULL;
	}

	return NULL;
}

// it is equivalent to "rm -rf pDirPath"
int MsgRmRf(char *pDirPath)
{
	struct dirent *d;
	DIR *dir;

	dir = opendir(pDirPath);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", strerror(errno));
		return -1;
	}

	int size = strlen(pDirPath) + 256;

	char *path = (char*)malloc(size);

	if (path == NULL) {
		MSG_DEBUG("path is NULL");
		return -1;
	}

	bzero(path, size);

	while ((d = readdir(dir)) != NULL) {
		if (d->d_type == DT_DIR) {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);

			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			MsgRmRf(path);

			if (rmdir(path) != 0) {

				if (path != NULL)
					free(path);

				closedir(dir);

				MSG_FATAL("error rmdir: %s", strerror(errno));

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

				MSG_FATAL("error remove: %s", strerror(errno));

				return -1;
			}
		}
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
		MSG_FATAL("error lstat: %s", strerror(errno));
		return -1;
	}

	return file_stat.st_size;
}


// it is equivalent to "du dir_path"
unsigned int MsgDu(const char *pDirPath)
{
	struct dirent *d;
	DIR *dir;

	dir = opendir(pDirPath);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", strerror(errno));
		return -1;
	}

	int size = strlen(pDirPath) + 256;
	char *path = (char*)malloc(size);
	bzero(path, size);

	unsigned int totalFileSize = 0;

	while ((d = readdir(dir)) != NULL) {
		if( d->d_type == DT_DIR) {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);

			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			unsigned int dirSize = MsgDu(path);

			if (dirSize == 0) {
				MSG_FATAL("error MsgDu");
				return dirSize;
			}

			totalFileSize += dirSize;
		} else {
			snprintf(path, size, "%s/%s", pDirPath, d->d_name);
			int fileSize = MsgGetFileSize(path);

			if (fileSize < 0) {
				MSG_FATAL("error MsgGetFileSize");
				return fileSize;
			}

			totalFileSize += fileSize;
		}
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
		MSG_FATAL("File Open Error: %s", strerror(errno));
		return false;
	}

	if (MsgFseek(pFile, 0L, SEEK_CUR) < 0) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Sead Error: %s", strerror(errno));
		return false;
	}

	if (MsgWriteFile(pData, sizeof(char), DataSize, pFile) != (size_t)DataSize) {
		MsgCloseFile(pFile);
		MSG_FATAL("File Write Error: %s", strerror(errno));
		return false;
	}

	MsgFsync(pFile);	//file is written to device immediately, it prevents missing file data from unexpected power off
	MsgFflush(pFile);
	MsgCloseFile(pFile);
	return true;
}

void MsgMmsInitDir()
{
	struct dirent *d = NULL;
	DIR* dir = NULL;

	dir = opendir(MSG_DATA_PATH);

	if (dir == NULL) {
		MSG_FATAL("error opendir: %s", strerror(errno));
		return;
	}

	// Remove temporal Mms folder (/opt/usr/data/msg-service/msgdata/*.dir)
	while ((d = readdir(dir)) != NULL) {
		if (d->d_type == DT_DIR) {
			if ((strcmp(".", d->d_name) == 0) || (strcmp("..", d->d_name) == 0))
				continue;

			if(strstr(d->d_name, ".dir") != NULL) {
				char filePath[MSG_FILEPATH_LEN_MAX] = {0,};
				snprintf(filePath, MSG_FILEPATH_LEN_MAX, MSG_DATA_PATH"%s", d->d_name);

				MsgRmRf(filePath);
				rmdir(filePath);
			}
		}
	}

	closedir(dir);
}

//mode : R_OK, W_OK, X_OK, or the existence test F_OK.
bool MsgAccessFile(const char *filepath, int mode)
{
	int ret;
	if (filepath == NULL) {
		MSG_DEBUG("filepath is NULL");
		return false;
	}

	MSG_DEBUG("request access path = %s, mode = %d", filepath, mode);

	ret = access(filepath, mode);


	if (ret) {
		MSG_DEBUG("Fail to access file, ret = %d", ret);
		return false;
	}

	return true;
}
