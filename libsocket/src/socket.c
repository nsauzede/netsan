#include <stdio.h>

#define NO_COMPAT_SOCKET
#include "sys/socket.h"
#undef NO_COMPAT_SOCKET
#include <errno.h>

int socket_init()
{
	int result = 0;
	printf( "%s\n", __func__);
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
	return result;
}

int compat_select( int nfds, fd_set *readfds, fd_set *writefds,
	fd_set *exceptfds, struct timeval *timeout)
{
	int result;

	errno = 0;
	if ((result = select( nfds, readfds, writefds, exceptfds, timeout)) == SOCKET_ERROR)
	{
		errno = WSAGetLastError();
		result = -1;
	}

	return result;
}

int compat_error()
{
	int _errno;

		int err = WSAGetLastError();
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
				socket_init();
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

int compat_connect( int  sockfd,  const  struct sockaddr *serv_addr, socklen_t
       addrlen)
{
	int result = 0;

	errno = 0;
	if (connect( sockfd, serv_addr, addrlen) == SOCKET_ERROR)
	{
		result = -1;
	}

	return result;
}

int compat_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int result;

	errno = 0;
	if ((result = accept( sockfd, addr, addrlen)) == INVALID_SOCKET)
	{
		errno = WSAGetLastError();
		result = -1;
	}

	return result;
}

