#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <linux/socket.h>

#define LOG_TAG "Cells"
#include <cutils/log.h>

#include "celld.h"

/* Initialize given sockaddr_un structure.
 * Returns address_length.
 */
int init_addr(struct sockaddr_un *addr)
{
	int pathlen = strlen(SOCKET_PATH);
	memset(addr, 0, sizeof(struct sockaddr_un));
	addr->sun_family = AF_UNIX;
	addr->sun_path[0] = 0; /* Make abstract */
	strncpy(&addr->sun_path[1], SOCKET_PATH, pathlen);
	return pathlen + 1 + offsetof(struct sockaddr_un, sun_path);
}

int _send_msg(int fd, const char *fmt, va_list ap)
{
	char buf[MAX_MSG_LEN];
	char *ptr;
	int remain, ret;
	int len = vsnprintf(buf, MAX_MSG_LEN, fmt, ap) + 1;
	if (len > MAX_MSG_LEN) {
		ALOGE("Failed to send response: Message is too long");
		return -1; /* msg too long */
	}

	remain = len;
	ptr = buf;
	while (remain > 0) {
		if ((ret = write(fd, ptr, remain)) < 0) {
			ALOGE("Failed to send response: %s", strerror(errno));
			return -1;
		}
		remain -= ret;
		ptr += ret;
	}
	return 0;
}

int send_msg(int fd, const char *fmt, ...)
{
	int ret;

	va_list ap;
	va_start(ap, fmt);
	ret = _send_msg(fd, fmt, ap);
	va_end(ap);

	return ret;
}

int recv_msg_len(int fd, char **code, char **msg, int maxlen)
{
	char buf[MAX_MSG_LEN+1];
	int remain, ret, n, i;

	if (maxlen > MAX_MSG_LEN)
		return -1;

	memset(buf, 0, sizeof(buf));

	n = 0;
	remain = maxlen;
	while (remain > 0) {
		ret = read(fd, buf + n, remain);
		if (ret < 0)
			return -1;
		if (ret == 0)
			break;
		n += ret;
		remain -= ret;
	}

	if (n == 0)
		return -1;

	/*
	 * messages are generally in the form:
	 * "CODE MESSAGE..."
	 */
	char *space_pos = strchr(buf, ' ');
	if (!space_pos) {
		/*
		 * only a 1 part message: that's OK,
		 * but we always want to free the msg buf, so
		 * we'll create a 1-byte empty string.
		 */
		*msg = malloc(1);
		*msg[0] = '\0';
	} else {
		*msg = malloc(n);
		strcpy(*msg, space_pos+1);
		/* formal separation */
		*space_pos = '\0';
	}
	*code = malloc(strlen(buf));
	strcpy(*code, buf);
	/*printf("recv: |%s| |%s|\n", *code, *msg);*/
	return 0;
}

/* Receives a message. Mallocs space and stores data in code and msg.
 * Returns 0 on success, -1 on failure */
int recv_msg(int fd, char **code, char **msg)
{
	return recv_msg_len(fd, code, msg, MAX_MSG_LEN);
}

/* Code adapted from: UNIX Network Programming, Volume 1, Second Edition:
 * Networking APIs: Sockets and XTI, Prentice Hall, 1998, W. Richard Stevens */
int recv_fd(int conn_fd)
{
	struct msghdr msg;
	struct iovec iov[1];
	char c = 'x';

	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	/* Need at least 1 data byte so recvmsg will block */
	iov[0].iov_base = &c;
	iov[0].iov_len = sizeof(c);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (recvmsg(conn_fd, &msg, 0) <= 0) {
		perror("recvmsg failed");
		return -1;
	}

	cmptr = CMSG_FIRSTHDR(&msg);

	if (cmptr == NULL && cmptr->cmsg_len != CMSG_LEN(sizeof(int)))
		return -1;
	if (cmptr->cmsg_level != SOL_SOCKET || cmptr->cmsg_type != SCM_RIGHTS)
		return -1;
	return *((int *)CMSG_DATA(cmptr));
}

/* Code adapted from: UNIX Network Programming, Volume 1, Second Edition:
 * Networking APIs: Sockets and XTI, Prentice Hall, 1998, W. Richard Stevens */
int send_fd(int conn_fd, int fd)
{
	struct msghdr msg;
	struct iovec iov[1];
	char c = 'x';

	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *)CMSG_DATA(cmptr)) = fd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = &c;
	iov[0].iov_len = sizeof(c);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (sendmsg(conn_fd, &msg, 0) <= 0)
		return -1;
	return 0;
}
