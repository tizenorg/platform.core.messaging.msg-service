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

#ifndef MSG_UTIL_FILE_H
#define MSG_UTIL_FILE_H


/*==================================================================================================
					DEFINES
==================================================================================================*/
#define FM_READ_WRITE_BUFFER_MAX (1024*1024*3)

#define TPDU_LOG_FILE "/opt/etc/msg-service/tpduLog.txt"


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


#endif // MSG_UTIL_FILE_H

