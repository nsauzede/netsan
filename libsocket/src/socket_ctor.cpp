/******************************************************************************
 *                     Copyright STMicroelectronics
 *              All rights reserved, COMPANY CONFIDENTIAL
 *     Unauthorized reproduction and communication strictly prohibited
 *-----------------------------------------------------------------------------
 * 	            System Platforms Group - MMC/IP&Design/SPG
 *-----------------------------------------------------------------------------
 * $Id: trackbootstrap.cpp.rca 1.1 Fri Jan 26 09:46:34 2007 sn24 Experimental $
 *-----------------------------------------------------------------------------
 * libsocket ctor
 *-----------------------------------------------------------------------------
 * Created by Nicolas SAUZEDE on February 2007
 * $Log$
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * Includes
 *---------------------------------------------------------------------------*/

#include "socket_ctor.h"

extern "C" {
#define NO_COMPAT_SOCKET
#include "sys/socket.h"
#undef NO_COMPAT_SOCKET
}

/*-----------------------------------------------------------------------------
 * Global Variables
 *---------------------------------------------------------------------------*/

static SocketCtor socketctor;

/*-----------------------------------------------------------------------------
 * Functions
 *----------------------------------------------------------------------------*/

SocketCtor::SocketCtor()
{
	socket_init();
}

/* EOF */
