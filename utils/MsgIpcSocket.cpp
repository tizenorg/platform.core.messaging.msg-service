/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>
#include <errno.h>

#include "MsgDebug.h"
#include "MsgException.h"
#include "MsgIpcSocket.h"


/*==================================================================================================
                                     IMPLEMENTATION OF MsgIpcClientSocket - Member Functions
==================================================================================================*/
MsgIpcClientSocket::MsgIpcClientSocket() : sockfd(-1), remotefd(-1), maxfd(-1)
{
	FD_ZERO(&fds);
}


msg_error_t MsgIpcClientSocket::connect(const char* path)
{
	MSG_BEGIN();

	if (!path || strlen(path) > strlen(MSG_SOCKET_PATH)) {
		THROW(MsgException::IPC_ERROR, "path is null");
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd < 0) {
		THROW(MsgException::IPC_ERROR,"socket not opened %s",strerror(errno));
	}

	struct sockaddr_un serverSA = {0, };
	serverSA.sun_family = AF_UNIX;

	memset(serverSA.sun_path, 0x00, sizeof(serverSA.sun_path));
	strncpy(serverSA.sun_path, path, sizeof(serverSA.sun_path)-1);  /* // "./socket" */

	int len = strlen(serverSA.sun_path) + sizeof(serverSA.sun_family);

	if (::connect(sockfd, (struct sockaddr *)&serverSA, len) == CUSTOM_SOCKET_ERROR) {
		THROW(MsgException::IPC_ERROR,"cannot connect server %s", strerror(errno));
	}

	/* add fd for select() */
	addfd(sockfd);

	/* read remote fd for reg func */
	char *rfd = NULL;
	AutoPtr<char> wrap(&rfd);
	unsigned int rlen;

	read(&rfd, &rlen);

	if (rfd == NULL) {
		THROW(MsgException::IPC_ERROR,"rfd is NULL %s", strerror(errno));
	}

	memcpy(&remotefd, rfd, sizeof(rlen));

	MSG_DEBUG("Connected: client fd [%d] <----> remote fd [%d]", sockfd, remotefd);

	MSG_END();

	return MSG_SUCCESS;
}


msg_error_t MsgIpcClientSocket::close()
{
	if (sockfd < 0) {
		MSG_FATAL("Client socket is not opened (check if you call close twice by accident) [%d]", sockfd);
		return MSG_ERR_UNKNOWN;
	}

	/* it means that client is going to close the connection.*/
	int cmd = CLOSE_CONNECTION_BY_USER;
	int len = sizeof(cmd);

	char cmdbuf[len];
	bzero(cmdbuf, len);
	memcpy(cmdbuf, &cmd, len);

	::close(sockfd);
	sockfd = CUSTOM_SOCKET_ERROR;

	return MSG_SUCCESS;
}

void MsgIpcClientSocket::addfd(int fd)
{
	MSG_DEBUG("%d added", fd);
	FD_SET(fd, &fds);
	if (fd > maxfd)
		maxfd = fd;
}

int MsgIpcClientSocket::writen (const char *buf, unsigned int len)
{
	unsigned int nleft;
	int nwrite;

	nleft = len;
	while (nleft > 0) {
		nwrite = ::write(sockfd, (const void*) buf, nleft);
		if (nwrite < 0) {
			MSG_FATAL("writen: sockfd [%d] error [%s]",  sockfd, strerror(errno));
			return nwrite;
		} else if (nwrite == 0) {
			break;
		}

		nleft -= nwrite;
		buf += nwrite;
	}
	return (len-nleft);
}

int MsgIpcClientSocket::write(const char* buf, unsigned int len)
{
	if (sockfd < 0) {
		MSG_FATAL("sockfd is not opened [%d]", sockfd);
		return CUSTOM_SOCKET_ERROR;
	}

	if (!buf || len == 0) {
		MSG_FATAL("buf[%p]	and len[%d] MUST NOT NULL", buf, len);
		return CUSTOM_SOCKET_ERROR;
	}

	/* send the data size first */
	int n = writen((const char*)&len, sizeof(len));
	if (n != sizeof(len)) {
		MSG_FATAL("WARNING: write header_size[%d] not matched [%d]", n, sizeof(len));
		return CUSTOM_SOCKET_ERROR;
	}

	/* send the data in subsequence */
	n = writen(buf, len);
	if ((unsigned int)n != len) {
		MSG_FATAL("WARNING: write data_size[%d] not matched [%d]", n, len);
		return CUSTOM_SOCKET_ERROR;
	}

	return len;
}

int MsgIpcClientSocket::readn( char *buf, unsigned int len )
{
	unsigned int nleft;
	int nread;

	nleft = len;
	while (nleft > 0) {
		nread = ::read(sockfd, (void*) buf, nleft);
		if (nread < 0) {
			MSG_FATAL("WARNING read value %d: %s", nread, strerror(errno));
			return nread;
		} else if( nread == 0 ) {
			break;
		}

		nleft -= nread;
		buf += nread;
	}

	return (len-nleft);
}


/* what if the buf is shorter than data? */
int MsgIpcClientSocket::read(char** buf, unsigned int* len)
{
	if (sockfd < 0) {
		MSG_FATAL("socket is not opened [%d]", sockfd);
		return CUSTOM_SOCKET_ERROR;
	}

	if (!buf || !len) {
		MSG_FATAL("rbuf and rlen MUST NOT NULL");
		return CUSTOM_SOCKET_ERROR;
	}

	/* read the data size first */
	int n = readn((char*) len, sizeof(int));
	if (n == CLOSE_CONNECTION_BY_SIGNAL) { /* if msgfw gets down, it signals to all IPC clients */
		MSG_FATAL("sockfd [%d] CLOSE_CONNECTION_BY_SIGNAL", sockfd);
		return n;
	} else if (n != sizeof(int)) {
		MSG_FATAL("WARNING: read header_size[%d] not matched [%d]", n, sizeof(int));
		return CUSTOM_SOCKET_ERROR;
	}

	/*  read the data in subsequence */
	unsigned int ulen = (unsigned int)*len;
	*buf = new char[ulen];
	bzero(*buf, ulen);
	n = readn(*buf, ulen);

	if ((unsigned int)n !=  ulen) {
		MSG_FATAL("WARNING: read data_size [%d] not matched [%d]", n, ulen);
		return CUSTOM_SOCKET_ERROR;
	}

	return n;
}


/*==================================================================================================
                                     IMPLEMENTATION OF MsgIpcServerSocket - Member Functions
==================================================================================================*/
MsgIpcServerSocket::MsgIpcServerSocket() : sockfd(-1), maxfd(-1)
{
	FD_ZERO(&fds);
}

void MsgIpcServerSocket::addfd(int fd)
{
	MSG_DEBUG("%d added", fd);
	FD_SET(fd, &fds);

	std::map<int, int>::iterator it = mapFds.find(fd);
	if (it != mapFds.end())
		MSG_FATAL("Duplicate FD %d", fd);
	else
		mapFds[fd] = fd;

	if (fd > maxfd)
		maxfd = fd;
}

msg_error_t MsgIpcServerSocket::open(const char* path)
{
	MSG_BEGIN();

	if (!path || strlen(path) > strlen(MSG_SOCKET_PATH)) {
		MSG_FATAL("path is null");
		return MSG_ERR_INVALID_PARAMETER;
	}

	if (sockfd != CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("WARNING: server_socket already opened %d at %p", sockfd,  &sockfd);
		return MSG_ERR_UNKNOWN;
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("socket failed: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	MSG_DEBUG("server_socket(%p) opened : %d", &sockfd, sockfd);

	struct sockaddr_un local = {0, };

	local.sun_family = AF_UNIX;
	memset(local.sun_path, 0x00, sizeof(local.sun_path));
	strncpy(local.sun_path, path, sizeof(local.sun_path)-1);

	unlink(local.sun_path);

	int len = strlen(local.sun_path) + sizeof(local.sun_family);

	if (bind(sockfd, (struct sockaddr *)&local, len) == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("bind: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	/**
	 * determine permission of socket file
	 *
	 *  - S_IRWXU : for user, allow read and write and execute
	 *  - S_IRWXG : for group, allow read and write and execute
	 *  - S_IRWXO : for other, allow read and write and execute
	 *
	 *  - S_IRUSR, S_IWUSR, S_IXUSR : for user, allow only read, write, execute respectively
	 *  - S_IRGRP, S_IWGRP, S_IXGRP : for group, allow only read, write, execute respectively
	 *  - S_IROTH, S_IWOTH, S_IXOTH : for other, allow only read, write, execute respectively
	 */
	mode_t sock_mode = (S_IRWXU | S_IRWXG | S_IRWXO); /* has 777 permission */

	if (chmod(path, sock_mode) == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("chmod: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	if (listen(sockfd, CUSTOM_SOCKET_BACKLOG) == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("listen: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	addfd(sockfd);

	MSG_END();

	return MSG_SUCCESS;
}

msg_error_t MsgIpcServerSocket::accept()
{
	MSG_BEGIN();

	if (sockfd == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("server_socket not init");
		return MSG_ERR_UNKNOWN;
	}

	struct sockaddr_un remote;

	int t = sizeof(remote);
	int fd = ::accept(sockfd, (struct sockaddr *)&remote, (socklen_t*) &t);
	if (fd < 0) {
		MSG_FATAL("accept: %s", strerror(errno));
		return MSG_ERR_UNKNOWN;
	}

	addfd(fd);
	MSG_DEBUG("%d is added", fd);

	/* write the registerd fd */
	write(fd, (const char*) &fd, sizeof(fd));

	MSG_END();

	return MSG_SUCCESS;
}

void MsgIpcServerSocket::close(int fd)
{
	MSG_BEGIN();

	if (sockfd == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("server_socket not init");
		return;
	}

	MSG_DEBUG("%d to be removed", fd);
	FD_CLR(fd, &fds);

	std::map<int, int>::iterator it = mapFds.find(fd);
	if (it == mapFds.end())
		MSG_FATAL("No FD %d", fd);
	else
		mapFds.erase(it);

	if (fd == maxfd) {
		int newmax = 0;
		for (it = mapFds.begin() ; it != mapFds.end() ; it++)
			newmax = (it->second > newmax )? it->second : newmax;
		maxfd = newmax;
	}
	MSG_DEBUG("fd %d removal done", fd);
	::close(fd);

	MSG_END();
}

int MsgIpcServerSocket::readn( int fd, char *buf, unsigned int len )
{
	size_t nleft;
	int nread;

	nleft = (size_t)len;
	while (nleft > 0) {
		nread = ::read(fd, (void*)buf, nleft);
		if (nread < 0) {
			MSG_FATAL("read: %s", strerror(errno));
			return nread;
		}
		else if (nread == 0)
			break;

		nleft -= nread;
		buf += nread;
	}
	return (len-nleft);
}

int MsgIpcServerSocket::read(int fd, char** buf, int* len )
{
	if (sockfd == CUSTOM_SOCKET_ERROR) {
		MSG_FATAL("server_socket(%p) is not initd %d", &sockfd, sockfd);
		return CUSTOM_SOCKET_ERROR;
	}

	if (!buf || !len) {
		MSG_FATAL("buf[%p] and len[%p] MUST NOT NULL", buf, len);
		return CUSTOM_SOCKET_ERROR;
	}

	/* read the data size first */
	int n = readn(fd, (char*) len, sizeof(int));

	if (n == CLOSE_CONNECTION_BY_SIGNAL) {
		MSG_FATAL("fd %d CLOSE_CONNECTION_BY_SIGNAL", fd);
		return n;
	}

	else if (n != sizeof(int)) {
		MSG_FATAL("readn %d(%d)", n, sizeof(int));
		return CUSTOM_SOCKET_ERROR;
	}

	MSG_DEBUG("MsgLen %d", *len);
	if (*len == CLOSE_CONNECTION_BY_USER)
		return *len;

	/* read the data in subsequence */
	if (*len > 0) {
		unsigned int ulen = (unsigned int)*len;
		*buf = new char[ulen+1];
		bzero(*buf, ulen+1);
		n = readn(fd, *buf, ulen);

		if ((unsigned int)n != ulen) {
			MSG_FATAL("WARNING: read data_size [%d] not matched [%d]", n, ulen);
			return CUSTOM_SOCKET_ERROR;
		}
	}

	return n;
}

int MsgIpcServerSocket::writen(int fd, const char *buf, unsigned int len)
{
	unsigned int nleft;
	int nwrite;

	nleft = len;

	while (nleft > 0) {
		/*  MSG_NOSIGNAL to prevent SIGPIPE Error */
		/*  MSG_DONTWAIT to avoid socket block */
		nwrite = ::send(fd, (const void*) buf, nleft, MSG_NOSIGNAL|MSG_DONTWAIT);

		if (nwrite < 0) {
			MSG_FATAL("write: %s", strerror(errno));
			return nwrite;
		} else if (nwrite == 0) { /* Nothing is send. */
			break;
		} else {
			nleft -= nwrite;
			buf += nwrite;
		}
	}

	return (len-nleft);
}


int MsgIpcServerSocket::write(int fd, const char* buf, unsigned int len)
{
	MSG_BEGIN();

	if (!buf || len <= 0) {
		MSG_FATAL("buf [%p] and len [%d] MUST NOT NULL", buf, len);
		return CUSTOM_SOCKET_ERROR;
	}

	MSG_DEBUG("for debug - fd : [%d], buf : [%p], len : [%d]", fd, buf, len);

	/* send the data size first */
	int n = writen(fd, (const char*)&len, sizeof(len));

	if (n != sizeof(len)) {
		MSG_FATAL("WARNING: write header_size[%d] not matched [%d]", n, sizeof(len));
		return CUSTOM_SOCKET_ERROR;
	}

	/*  send the data in subsequence */
	n = writen(fd, buf, len);

	MSG_DEBUG("Writing %d bytes", n);

	if ((unsigned int)n != len) {
		MSG_FATAL("Written byte (%d) is not matched to input byte (%d)", n, len);
		return CUSTOM_SOCKET_ERROR;
	}

	MSG_END();

	return len;
}
