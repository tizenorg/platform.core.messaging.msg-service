/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
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

#ifndef MSG_UTIL_FILE_H
#define MSG_UTIL_FILE_H

#include <stdio.h>
/*==================================================================================================
					DEFINES
==================================================================================================*/
#define FM_READ_WRITE_BUFFER_MAX (1024*1024*3)

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/
/**  file operation wrapper - for avoding runtime error */
FILE* MsgOpenFile(const char* filepath, const char* opt);
void MsgCloseFile(FILE* pFile);
int MsgFseek(FILE* pFile, long int offset, int origin);
size_t MsgWriteFile(const char* pData, size_t size, size_t count, FILE  *pFile);
size_t MsgReadFile(void *pData, size_t size, size_t count, FILE *pFile);
long int MsgFtell(FILE *pFile);
int MsgFflush(FILE *pFile);
int MsgFsync(FILE *pFile);

bool MsgCreateFileName(char *pFileName);
bool MsgOpenAndReadFile(const char *pFileName, char **ppData, int *pDataSize);
bool MsgReadFileForDecode(FILE* pFile, char* pBuf, int length, int* nSize);
bool	MsgWriteDataFromEncodeBuffer(FILE* pFile, char* pInBuffer, int *pPtr, int maxLen, int* pOffset );
bool MsgWriteIpcFile(const char *pFileName, const char *pData, int DataSize);
void MsgDeleteFile(const char *pFileName);
bool MsgGetFileSize(const char *pFilePath, int* nSize);
int MsgGetFileSize(const char *pFileName);
FILE*  MsgOpenMMSFile(char *pFileName);
bool MsgOpenCreateAndOverwriteFile(char *pFullPath, char *pBuff, int TotalLength);
char* MsgOpenAndReadMmsFile(const char* szFilePath, int offset, int size, int* npSize );
bool MsgWriteSmilFile(const char *pFilePath,char *pData, int DataSize);
int	MsgReadSmilFile(const char *pFileName, char **ppData);
void MsgDeleteSmilFile(const char *pFilePath);
int MsgRmRf(char *pDirPath);
unsigned int MsgDu(const char *pDirPath);
bool MsgAppendFile(const char *pFilePath, const char *pData, int DataSize);
void MsgMmsInitDir();
bool MsgAccessFile(const char *filepath, int mode);

#endif // MSG_UTIL_FILE_H

