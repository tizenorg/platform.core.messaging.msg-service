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
#define MSG_MAX_IPC_SIZE 50000 /* 50 * 1000 = sizeof(msg common info) * max message count */
#define MAX_NUM_IPC_CLIENT 10

#define MSG_SOCKET_PATH "/tmp/.msgfw_socket"

#define CUSTOM_SOCKET_ERROR		-1
#define CUSTOM_SOCKET_BACKLOG	10


/*==================================================================================================
											ENUM
==================================================================================================*/
typedef enum {
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
/*	~MsgIpcClientSocket(); */

	int maxFd() { return (maxfd+1); }
	fd_set fdSet() { return fds; }
	int fd() { return sockfd; }

	msg_error_t connect(const char *path);
	msg_error_t close();
	/* write msg to ipc server */
	int write(const char* buf, unsigned int len);
	/* read msg from ipc server */
	int read(char** buf, unsigned int* len);
	void addfd(int fd);
	int	getRemoteFd() {return remotefd; }
private:
	int readn(char *buf, unsigned int len);
	int writen(const char *buf, unsigned int len);
	bool wait_for_reply();

	int sockfd, remotefd, maxfd;
	fd_set fds;
};


class MsgIpcServerSocket
{
public:
	MsgIpcServerSocket();
	~MsgIpcServerSocket() { mapFds.clear(); }
	int maxFd() { return (maxfd+1); }
	fd_set fdSet() { return fds; }
	int fd() { return sockfd; }

	msg_error_t open(const char *path);
	msg_error_t accept();
	void close(int fd);

	/* read msg from client of fd */
	int read(int fd, char** buf, int* len);
	/* write msg to ipc client */
	int write(int fd, const char* buf, unsigned int len);
	void addfd(int fd);
	void setSockfd(int fd) { sockfd = fd; }

private:
	int readn(int fd, char *buf, unsigned int len);
	int writen(int fd, const char *buf, unsigned int len);

	/* server socket fd */
	int sockfd;

	/* information about IPC clients, it is used for select() */
	fd_set fds;
	int maxfd;
	std::map<int, int> mapFds;
};

#endif /*__IPCSocket_H__ */

