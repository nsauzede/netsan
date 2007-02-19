#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include "sys/compat_types.h"

#ifdef WIN32
#include <winsock2.h>

#define EADDRINUSE      98      /* Address already in use */
#define ENETUNREACH     101     /* Network is unreachable */
#define EISCONN         106     /* Transport endpoint is already connected */
#define ECONNREFUSED    111     /* Connection refused */
#define EALREADY        114     /* Operation already in progress */

#define socklen_t int
#ifndef NO_COMPAT_SOCKET
#define close(...) \
do \
{ \
closesocket(__VA_ARGS__); \
} \
while (0)
#define select compat_select
#define connect compat_connect
#define accept compat_accept
#define socket compat_socket

extern int compat_select(int nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval *timeout);
extern int compat_connect(int  sockfd,  const  struct sockaddr *serv_addr, socklen_t addrlen);
extern int compat_accept( int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int compat_socket( int domain, int type, int protocol);
#else
extern int socket_init();
#endif
#endif

#endif

