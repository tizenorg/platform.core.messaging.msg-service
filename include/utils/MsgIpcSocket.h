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

#ifndef __IPCSocket_H__
#define __IPCSocket_H__

/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include <sys/types.h>
#include <map>

#include "MsgTypes.h"
#include "MsgCppTypes.h"


/*==================================================================================================
                                         DEFINES
==================================================================================================*/
#define MSG_MAX_IPC_SIZE 50000 // 50 * 1000 = sizeof(msg common info) * max message count
#define MAX_NUM_IPC_CLIENT 10

#define MSG_SOCKET_PATH "/tmp/.msgfw_socket"

#define CUSTOM_SOCKET_ERROR		-1
#define CUSTOM_SOCKET_BACKLOG	10


/*==================================================================================================
                                         ENUM
==================================================================================================*/
typedef enum
{
	CLOSE_CONNECTION_BY_SIGNAL = 0,
	CLOSE_CONNECTION_BY_USER = -17,
} IPC_CONTROL_E;


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class MsgIpcClientSocket
{
public:
	MsgIpcClientSocket();
//	~MsgIpcClientSocket();

	int 	maxFd() { return (maxfd+1); }
	fd_set 	fdSet() { return fds; }
	int 	fd() { return sockfd; }

	MSG_ERROR_T connect(const char *path);
	MSG_ERROR_T close();
	/* write msg to ipc server */
	int 	write(const char* buf, int len);
	/* read msg from ipc server */
	int 	read(char** buf, int* len);
	void 	addfd(int fd);
	int		getRemoteFd() {return remotefd; }
private:
	int readn(char *buf, int len );
	int writen (const char *buf, int len);

	int sockfd, remotefd, maxfd;
	fd_set fds;
};


class MsgIpcServerSocket
{
public:
	MsgIpcServerSocket();
	~MsgIpcServerSocket() { mapFds.clear(); }
	int 	maxFd() { return (maxfd+1); }
	fd_set 	fdSet() { return fds; }
	int 	fd() { return sockfd; }

	MSG_ERROR_T open(const char *path);
	MSG_ERROR_T accept();
	void 		close(int fd);

	/* read msg from client of fd */
	int 	read(int fd, char** buf, int* len );
	/* write msg to ipc client */
	int 	write(int fd, const char* buf, int len);
	void 	addfd(int fd);
	void    setSockfd(int fd) { sockfd = fd; }

private:
	int readn(int fd, char *buf, int len );
	int writen (int fd, const char *buf, int len);

	/* server socket fd */
	int sockfd;

	/* information about IPC clients, it is used for select() */
	fd_set 				fds;
	int 				maxfd;
	std::map<int, int> 	mapFds;
};

#endif //__IPCSocket_H__

