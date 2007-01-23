/******************************
 * Proxy - (c) N.Sauzede 2005 *
 ******************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#define basename(foo)	foo	/* win32 su^H^Hlacks basename support */
#define socklen_t int
/* win32 doesn't flush stdout on a line-out-basis */
#define printf(...) \
do \
{ \
printf(__VA_ARGS__); \
fflush(stdout); \
} \
while (0)
#define read(a,b,c) recv(a,b,c,0)
/* win32 su^H^Hlacks standard socket file descriptor */
#define close(...) \
do \
{ \
closesocket(__VA_ARGS__); \
} \
while (0)
#define pthread_create(ptid,pattr,pfn,parg) !CreateThread(0,0,pfn,parg,0,(LPDWORD)ptid)
#define pthread_t	void *
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <netdb.h>
#include <pthread.h>
#define SOCKET_ERROR	-1
#endif

#define PROXY_PORT	1234

#define DEF_HOST	"127.0.0.1"
#define DEF_S_PORT	5000
#define DEF_C_PORT	5001
#define BUF_SIZE	(10 * 1024)
#define MAX_HOST	64
#define CMD_PROXY       "proxy"         // s_host s_port [c_port]
#define CMD_QUIT        "quit"          // client
#define CMD_EXIT        "exit"          // app

void asciify( char *ptr, int n)
{
	while (n > 0)
	{
//		printf( "** chr=%02X (%d) '%c'\n", *ptr, *ptr, *ptr);
		if ((*ptr < ' ') && (*ptr != '\t') && (*ptr != '\r') && (*ptr != '\n'))
			*ptr = '.';
		ptr++;
		n--;
	}
	*ptr = 0;
}

int end = 0;
int init = 0;
int disc = 0;
char *ch = "127.0.0.1";
int sp = 5001, cp = 5000;

#ifdef WIN32
DWORD WINAPI fn( LPVOID opaque)
#else
void *fn( void *opaque)
#endif
{
	void * result = 0;
	struct hostent *he;
	int css = (int)opaque;
	int pid = getpid();

	int cs, max;
	struct sockaddr_in ca;
	int cscol = 32, ccol = 31;
	int n;

	printf( "[%d]++connecting server..\n", (int)pid);
	cs = socket( PF_INET, SOCK_STREAM, 0);
	memset( &ca, 0, sizeof( ca));
	he = gethostbyname( ch);
	if (he)
	{
		memcpy( &ca.sin_addr.s_addr, he->h_addr, sizeof( ca.sin_addr.s_addr));
	}
	else
	ca.sin_addr.s_addr = inet_addr( ch);
	ca.sin_port = htons( cp);
	ca.sin_family = AF_INET;
	n = connect( cs, (struct sockaddr *)&ca, sizeof( ca));
	if (n != 0)
	{
#ifdef WIN32
		printf( "[%d]connect returned n=%d : %d\n", (int)pid, n, WSAGetLastError());
#else
		perror( "connect");
#endif
//		exit( 1);
		goto connect_error;
	}

	printf( "[%d]++connected !!\n", (int)pid);
	max = cs;
	if (css > max)
		max = css;
	max++;
	while (1)
	{
#define MAX_BUF	(400 * 1024)
		char buf[MAX_BUF + 2], *ptr;
		fd_set rfds;
		struct timeval tv;
		int src, dst;
		int col = 0, size;

		FD_ZERO( &rfds);
#ifndef WIN32
		FD_SET( 0, &rfds);
#endif
		FD_SET( cs, &rfds);
		FD_SET( css, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
	
		ptr = buf;
		size = sizeof( buf);
		n = select( max, &rfds, NULL, NULL, &tv);
		if (n == SOCKET_ERROR)
		{
			perror( "select");
			printf( "[%d]leaving because select error\n", (int)pid);
			break;
		}
		else if (n)
		{
			if (FD_ISSET( cs, &rfds))
			{
				src = cs;
				dst = css;
				col = ccol;
				buf[0] = disc ? ']' : '>';
				ptr++;
				size--;
			}
			else if (FD_ISSET( css, &rfds))
			{
				src = css;
				dst = cs;
				col = cscol;
				buf[0] = disc ? '[' : '<';
				ptr++;
				size--;
			}
			else if (FD_ISSET( 0, &rfds))
			{
				src = 0;
			}
			else
			{
				printf( "[%d]**select woken by unknown\n", (int)pid);
				n = 0;
			}
	
			if (n)
			{
				n = read( src, ptr, size);
				if (n < 0)
				{
#ifdef WIN32
					printf( "[%d]read returned n=%d : %d\n", (int)pid, n, WSAGetLastError());
					break;
#else
					perror( "read");
#endif
				}
				else if (n == 0)
				{
/*					if (src == css)
						end = 1;*/
					break;
				}
				else
				{
					if (n > size)
						n = size;
					if (src == 0)
					{
						if (buf[0] == '<')
						{
							dst = cs;
							col = cscol;
							ptr++;
							n--;
						}
						else if (buf[0] == '>')
						{
							dst = css;
							col = ccol;
							ptr++;
							n--;
						}
						else
						{
//							printf( "* inspecting input.. ptr=%s\n", ptr);
							if (!strncmp( ptr, "disc", 4))
							{
								disc = !disc;
								printf( "* connections %s\n", disc ? "disabled" : "enabled");
							}
							else if (!strncmp( ptr, "quit", 4))
							{
								end = 1;
								break;
							}
							else if (!strncmp( ptr, "help", 4))
							{
								printf( "help\tget help about available commands\n");
								printf( "disc\t[dis]connect client and server\n");
								printf( "<'msg'\tsend 'msg' to server\n");
								printf( ">'msg'\tsend 'msg' to client\n");
								printf( "quit\tterminate connections and quit program\n");
								printf( "\n");
							}
							n = 0;	// don't write anything afterwards
						}
					}
					if (n)
					{
						if (!disc || !src)
							send( dst, ptr, n, 0);
						if (src != 0)
						{
							asciify( buf, n + 1);
							if (col)
								printf( "[%d]\x1b[01;%02dm%s\x1b[00m", (int)pid, col, buf);
							else
								printf( "[%d]\x1b[00m%s\x1b[00m", (int)pid, buf);
							printf( "\x1b[m");
							fflush( stdout);
						}
					}
				}
			}
		}
//		else
//		{//timeout
//		}
	}
connect_error:
	printf( "[%d]++closing client\n", (int)pid);
	close( css);
	printf( "[%d]++closing server\n", (int)pid);
	close( cs);
	return result;
}

int main( int argc, char *argv[])
{
	int css, ss;
	int sp = 5001;
	struct sockaddr_in csa, sa;
	int arg = 1, on;
	
        /* args : client_host client_port serveur_port */
	if (argc > arg)
	{
		ch = argv[arg++];
		if (argc > arg)
		{
			sscanf( argv[arg++], "%d", &cp);
			if (argc > arg)
			{
				sscanf( argv[arg++], "%d", &sp);
				init = 1;
				if (argc > arg)
				{
					sscanf( argv[arg++], "%d", &disc);
				}
			}
		}
	}
	if (!init)
	{
		printf( "Usage : %s cli_host cli_port ser_port [disc=1|0]\n", basename( argv[0]));
		return -1;
	}

#ifdef WIN32
{
        WSADATA wsaData;
        int iResult;

        iResult = WSAStartup( MAKEWORD( 2, 2), &wsaData);
        if (iResult)
        {
                printf( "WSAStartup failed: %d\n", iResult);
                return -4;
        }
}
#endif
	ss = socket( PF_INET, SOCK_STREAM, 0);
	memset( &sa, 0, sizeof( sa));
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons( sp);
	sa.sin_family = AF_INET;
	on = 1;
	setsockopt( ss, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof( on));
	bind( ss, (struct sockaddr *)&sa, sizeof( sa));
	listen( ss, 1);

	end = 0;
	while (!end)
	{
		pthread_t tid;
		socklen_t clen;
		
		printf( "++accepting.. local server port=%d\n", sp);
		clen = sizeof( csa);
		css = accept( ss, (struct sockaddr *)&csa, &clen);
	
		printf( "++accepted !!\n");
		if (!pthread_create( &tid, NULL, fn, (void *)css))
		{
			printf( "++succesfully created thread\n");
		}
		else
			perror( "pthread_create");
//		close( css);
		printf( "++closed css\n");fflush( stdout);
	}
	printf( "++closing listening server\n");
	close( ss);
	
	return 0;
}

