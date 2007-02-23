/*************************************
 * NetSan - (c) N.Sauzede 2007 *
 *************************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <netdb.h>

#include <pthread.h>

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#define sleep _sleep
/* win32 doesn't flush stdout on a line-out-basis */
#define printf(...) \
do \
{ \
printf(__VA_ARGS__); \
fflush(stdout); \
} \
while (0)
#endif

#define NETSAN_VERSION	"0.1pre"

#define PROXY_PORT	1234

#define DEF_HOST	"127.0.0.1"
#define DEF_S_PORT	5000
#define DEF_C_PORT	5001
#define BUF_SIZE	(10 * 1024)
#define MAX_HOST	64
//#define CMD_PRL		"\\"
#define CMD_PRL		""
#define CMD_HELP	CMD_PRL "help"		// help
#define CMD_PROXY	CMD_PRL "proxy"		// s_host s_port [c_port]
#define CMD_DISC	CMD_PRL "disc"		// disc
#define CMD_QUIT	CMD_PRL "quit"		// client
#define CMD_EXIT	CMD_PRL "exit"		// app
#define ARG_DISC	"-disc"

int end = 0;
int init = 0;
int disc = 0;
char *ch = 0;
int cp = 0;
int iscurse = 1;		// iscurse=1 means we can kbhit()/select() on stdin (default on linux)
int istty = 1;		// istty=1 means we can output ANSI codes (default on linux)

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

void *fn( void *opaque)
{
	void * result = 0;
//#endif
	struct hostent *he;
	int css = (int)(long int)opaque;
//	int pid = getpid();

	int cs = 0, max = 0;
	struct sockaddr_in ca;
	int cscol = 32, ccol = 31;
	int n;
	int local_end = 0;
	int kill_global = 0;

	if (ch)
	{
//		printf( "[%d]++connecting server..\n", (int)pid);
		printf( "++connecting to %s:%d..\n", ch, cp);
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
			perror( "++connect");
			goto connect_error;
		}

//		printf( "[%d]++connected !!\n", (int)pid);
		printf( "++connected !!\n");
		max = cs;
	}
	else
	{
//		printf( "[%d] LOCAL MODE\n", (int)pid);
//		printf( "++LOCAL MODE\n");
	}
//	printf( "++ch=%p css=%d cs=%d\n", ch, css, cs);
	if (css > max)
		max = css;
	max++;
//	printf( "max=%d\n", max);
	while (!local_end)
	{
#define MAX_BUF	(400 * 1024)
		char buf[MAX_BUF + 2], *ptr;
		fd_set rfds;
		struct timeval tv;
		int src, dst;
		int col = 0, size;
		static int header = 1;

		if (!(css && cs) && header && !istty)
		{
			printf( "<");fflush( stdout);
			header = 0;
		}

		FD_ZERO( &rfds);
		FD_SET( 0, &rfds);
		if (cs)
			FD_SET( cs, &rfds);
		if (css)
			FD_SET( css, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
	
		ptr = buf;
		size = sizeof( buf);
//		printf( "selecting..\n");
		n = select( max, &rfds, NULL, NULL, &tv);
//		printf( "select returned %d\n", n);
		if (n == -1)
		{
			perror( "++select");
//			printf( "[%d]leaving because select error\n", (int)pid);
			printf( "++leaving because select error\n");
			n = 0;
			break;
		}
#ifdef WIN32
		else if (!n)
		{//timeout : let's look at stdin
			if (kbhit())
				n = 1;
		}
#endif
		if (n)
		{
//			printf( "Ahh, sg to read.. cs=%d css=%d\n", cs, css);
			if (cs && FD_ISSET( cs, &rfds))
			{
				src = cs;
				dst = css;
				col = ccol;
				buf[0] = disc ? ']' : '>';
				ptr++;
				size--;
			}
			else if (css && FD_ISSET( css, &rfds))
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
//				printf( "[%d]**select woken by unknown\n", (int)pid);
				src = -1; // to differentiate win32 stdin handle from win32 stdin fd
			}
	
			if (src >= 0)
			{
//				printf( "reading from src=%d\n", src);
				if (!src)
					n = read( src, ptr, size);
				else
					n = recv( src, ptr, size, 0);
			}
#ifdef WIN32
			else
			{
//				printf( "kbhit !!!!\n");
				gets( ptr);
				strcat( ptr, "\n");
				n = strlen( ptr);
			}
#endif
			if (n < 0)
			{
				perror( "++read");
				break;
			}
			else if (n == 0)
			{
/*				if (src == css)
					end = 1;*/
				break;
			}
			else
			{
//				printf( "positive size (n=%d) buf=[%s] src=%d css=%d cs=%d\n", n, buf, src, css, cs);
				if (n > size)
					n = size;
				if (src <= 0)
				{
/*
 * proxy : cs && css : cmds directes, ser < proxy > cli
 * client: cs        : ser direct, cmds \
 * server: css       : cli direct, cmds \
 *
 */
					if (((!css || !cs) && (buf[0] == '\\')) || ((css && cs) && (buf[0] != '>') && (buf[0] != '<')))
					{
						if ((!css || !cs) && (buf[0] == '\\'))
							ptr++;
//						printf( "* inspecting input.. ptr=[%s]\n", ptr);
						if (!strncmp( ptr, CMD_DISC, strlen( CMD_DISC)))
						{
							disc = !disc;
							printf( "++connections %s\n", disc ? "disabled" : "enabled");
						}
						else if (!strncmp( ptr, CMD_QUIT, strlen( CMD_QUIT)))
						{
							local_end = 1;
							break;
						}
						else if (!strncmp( ptr, CMD_EXIT, strlen( CMD_EXIT)))
						{
							local_end = 1;
							kill_global = 1;
							break;
						}
						else if (!strncmp( ptr, CMD_HELP, strlen( CMD_HELP)))
						{
							printf( "++%s\tget help about available commands\n", CMD_HELP);
							printf( "++%s\t[dis]connect client and server\n", CMD_DISC);
							printf( "++%s\tterminate current client connection\n", CMD_QUIT);
							printf( "++%s\tterminate current client connection\n", CMD_EXIT);
							if (cs && css)
							{
								printf( "++<'msg'\tsend 'msg' to server\n");
								printf( "++>'msg'\tsend 'msg' to client\n");
							}
							else
								printf( "++'msg'\tsend 'msg' to client\n");
							printf( "++\n");
						}
						n = 0;	// don't write anything afterwards
					}
					else if (!cs || !css)
					{
						if (!cs)
						{
							dst = css;
							col = ccol;
						}
						else if (!css)
						{
							dst = cs;
							col = ccol;
						}
					}
					else if (buf[0] == '>')
					{
						dst = css;
						col = ccol;
						ptr++;
						n--;
					}
					else if (buf[0] == '<')
					{
						dst = cs;
						col = cscol;
						ptr++;
						n--;
					}
				}
				if (n)
				{
//					printf( "disc=%d src=%d\n", disc, src);
					if (!disc /*&& (src <= 0)*/ && (dst > 0))
					{
//						printf( "Sending n=%d ptr=[%s] to dst=%d cs=%d css=%d..\n", n, ptr, dst, cs, css);
						send( dst, ptr, n, 0);
					}
					if (src > 0)
					{
						asciify( buf, n + 1);
						if (istty)
							ptr = buf + 1;
						else
							ptr = buf;
						printf( "\r");
						if (col)
						{
//							printf( "[%d]", (int)pid);
							if (istty)
								printf( "\x1b[01;%02dm", col);
							printf( "%s", ptr);
							if (istty)
								printf( "\x1b[00m");
						}
						else
						{
//							printf( "[%d]", (int)pid);
							if (istty)
								printf( "\x1b[00m");
							printf( "%s", ptr);
							if (istty)
								printf( "\x1b[00m");
						}
						if (istty)
							printf( "\x1b[m");
						fflush( stdout);
					}
				}
						header = 1;
			}
		}
	}
connect_error:
	if (css)
	{
//		printf( "[%d]++closing client\n", (int)pid);
		printf( "++closing client\n");
		close( css);
	}
	if (cs)
	{
//		printf( "[%d]++closing server\n", (int)pid);
		printf( "++closing server\n");
		close( cs);
	}

	if (kill_global)
	{
		printf( "++kill global..\n");
		end = 1;
	}

	return result;
}

int isdignum( const char *str)
{
	int result = 0;
	char *endptr;
	long val;

	errno = 0;
	val = strtol( str, &endptr, 10);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      || (errno != 0 && val == 0))
	{
		printf( "%s: error somewhere\n", __func__);
	}
	else
	{
		if (endptr == str)
		{
//			printf( "%s: no digits found\n", __func__);
		}
		else
		{
			if (*endptr != '\0')
			{
//				printf( "%s: trailing garbage\n", __func__);
			}
			else
			{
//				printf( "%s: [%s] is a genuine dignum : %ld\n", __func__, str, val);
				result = 1;
			}
		}
	}
	
	return result;
}

int main( int argc, char *argv[])
{
	int css, ss;
	int sp = 0;
	struct sockaddr_in csa, sa;
	int arg = 1, on;
	
#ifdef WIN32
//	HandlerRoutine
	DWORD mode;
//	unsigned char buf[100];
//	DWORD count;
	if (GetConsoleMode( GetStdHandle(STD_INPUT_HANDLE), &mode))
	{
//		printf( "GetConsoleMode returned TRUE : mode=%08lx\n", mode);
		iscurse = 1;
		istty = 0;
	}
	else
	{
//		printf( "GetConsoleMode returned FALSE\n");
		iscurse = 0;
		istty = 1;
	}
/*
	if (ReadConsole( GetStdHandle(STD_INPUT_HANDLE), buf, sizeof( buf), &count, NULL))
	{
		printf( "ReadConsole returned TRUE : count=%08lx\n", count);
	}
	else
	{
		printf( "ReadConsole returned FALSE\n");
	}
*/
#endif
	/* XXX */
	while (argc > arg)
	{
		if (!strcmp( argv[arg], "-l") || !strcmp( argv[arg], "-p"))
		{
			arg++;
			continue;
		}
		break;
	}
	if (argc > arg)
	{
		ch = argv[arg++];
		if (isdignum( ch))
		{
			sscanf( ch, "%d", &sp);
			ch = 0;
			init = 1;
			if (argc > arg)
			{
				ch = argv[arg++];
//				printf( "looking up cp/disc\n");
				while (argc > arg)
				{
					if (!strcmp( argv[arg], ARG_DISC))
					{
						disc = 1;
						arg++;
					}
					else
					{
						sscanf( argv[arg++], "%d", &cp);
						break;
					}
				}
				if (!cp)
					cp = sp;
			}
		}
		else
		{
			if (argc > arg)
			{
				sscanf( argv[arg++], "%d", &cp);
				init = 1;
			}
		}
	}
//	printf( "testing init\n");
	if (!init)
	{
		printf( "\nNetSan aka Mr Net version %s - (c) N.Sauzede 2007\n", NETSAN_VERSION);
		printf( "\n");
		printf( "Usage :\n");
//		printf( "\n(telnet mode) :");
		printf( " %s remote_host remote_port\n", basename( argv[0]));
//		printf( "\n(server/proxy mode) :");
		printf( " %s -l -p local_port [remote_host [remote_port=local_port] [-disc]]\n", basename( argv[0]));
		printf( "\n\n");
		return -1;
	}

	if (sp)
	{
		ss = socket( PF_INET, SOCK_STREAM, 0);
		memset( &sa, 0, sizeof( sa));
		sa.sin_addr.s_addr = INADDR_ANY;
		sa.sin_port = htons( sp);
		sa.sin_family = AF_INET;
		on = 1;
		setsockopt( ss, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof( on));
		if (bind( ss, (struct sockaddr *)&sa, sizeof( sa)))
		{
			perror( "bind");
			goto bailout;
		}
		if (listen( ss, 1))
		{
			perror( "listen");
			goto bailout;
		}
		end = 0;
		while (!end)
		{
			pthread_t tid;
			socklen_t clen;
			struct hostent *he;
		
			printf( "--accepting on localhost:%d", sp);
			if (ch)
				printf( " <= proxy => %s:%d", ch, cp);
			printf( "\n");
			clen = sizeof( csa);
			css = accept( ss, (struct sockaddr *)&csa, &clen);
			if (css < 0)
				continue;
		
			he = gethostbyaddr( (char *)&csa.sin_addr, sizeof( csa.sin_addr), AF_INET);
			printf( "--accepted !! client=%s (%s)\n", inet_ntoa( csa.sin_addr), he ? he->h_name : "*UNRESOLVED*");
			if (!pthread_create( &tid, NULL, fn, (void *)(long int)css))
			{
				printf( "--succesfully created thread\n");
			}
			else
				perror( "--pthread_create");
	//		close( css);
			printf( "--closed css\n");fflush( stdout);
		}
		printf( "--closing listening server\n");
bailout:
		close( ss);
	}
	else
	{
		printf( "--telnet mode - remote server is %s:%d\n", ch, cp);
		fn( (void *)0);
	}
	
	return 0;
}

