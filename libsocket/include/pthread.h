#ifndef _PTHREAD_H
#define _PTHREAD_H

#ifdef WIN32
#include <windows.h>

#define pthread_create(ptid,pattr,pfn,parg) (!CreateThread(0,0,(LPTHREAD_START_ROUTINE)pfn,parg,0,(LPDWORD)ptid))
#define pthread_t   void *
#endif

#endif

