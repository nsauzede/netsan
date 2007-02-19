#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#ifndef NO_COMPAT_SOCKET
#ifdef WIN32
#include <winsock2.h>
#endif

inline void compat_FD_SET(a,b)
{
//	printf( "%s: a=%d\n", __func__, a);
	if (a)
		FD_SET( a, b);
}

#undef FD_SET
#define FD_SET(a,b) compat_FD_SET(a,b)
#endif

#endif

