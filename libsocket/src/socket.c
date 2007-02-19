#include <stdio.h>

#define NO_COMPAT_SOCKET
#include "sys/socket.h"
#undef NO_COMPAT_SOCKET
#include <errno.h>

static int is_init = 0;

int compat_socket_init()
{
	int result = 0;

//	printf( "%s: is_init=%d\n", __func__, is_init);
	if (!is_init)
	{
		is_init = 1;
#ifdef WIN32
	WSADATA wsaData;
    int iResult;

    iResult = WSAStartup( MAKEWORD( 2, 2), &wsaData);
    if (iResult)
    {
         printf( "WSAStartup failed: %d\n", iResult);
	     result = -1;
    }
#endif
	}

	return result;
}

int compat_errno()
{
	int _errno, err;

	err = WSAGetLastError();
//	printf( "%s: err=%d\n", __func__, err);
	switch (err)
		{
			case WSAEACCES:
				_errno = EACCES;
				break;
			case WSAEADDRINUSE:
				_errno = EADDRINUSE;
				break;
			case WSAEINTR:
				_errno = EINTR;
				break;
			case WSAEALREADY:
				_errno = EALREADY;
				break;
			case WSAECONNREFUSED:
				_errno = ECONNREFUSED;
				break;
			case WSAEFAULT:
				_errno = EFAULT;
				break;
			case WSAEISCONN:
				_errno = EISCONN;
				break;
			case WSAENETUNREACH:
				_errno = ENETUNREACH;
				break;

			case WSANOTINITIALISED:
//				printf( "WSA not initialized !!\n");
				compat_socket_init();
				_errno = EAGAIN;
				break;

			case WSAENETDOWN:
			case WSAEINPROGRESS:
			case WSAEADDRNOTAVAIL:
			case WSAEINVAL:
			case WSAEHOSTUNREACH:
			case WSAENOBUFS:
			case WSAENOTSOCK:
			case WSAETIMEDOUT:
			case WSAEWOULDBLOCK:
			default:
				_errno = -1;
				break;
		}
	return _errno;
}

int compat_select( int nfds, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout)
{
	int result;

//	printf( "%s\n", __func__);
	errno = 0;
	if ((result = select( nfds, readfds, writefds, exceptfds, timeout)) == SOCKET_ERROR)
	{
		errno = compat_errno();
		result = -1;
	}

	return result;
}

int compat_connect( int  sockfd,  const  struct sockaddr *serv_addr, socklen_t
       addrlen)
{
	int result = 0;

//	printf( "%s\n", __func__);
	errno = 0;
	if (connect( sockfd, serv_addr, addrlen) == SOCKET_ERROR)
	{
		errno = compat_errno();
		result = -1;
	}

	return result;
}

int compat_socket(int domain, int type, int protocol)
{
	int result;

//	printf( "%s\n", __func__);
	while (1)
	{
		errno = 0;
		if ((result = socket( domain, type, protocol)) == INVALID_SOCKET)
		{
			errno = compat_errno();
			if (errno == EAGAIN)
				continue;
			result = -1;
		}
		break;
	}

	return result;
}

int compat_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int result;

//	printf( "%s\n", __func__);
	errno = 0;
	if ((result = accept( sockfd, addr, addrlen)) == INVALID_SOCKET)
	{
		errno = compat_errno();
		result = -1;
	}

	return result;
}

