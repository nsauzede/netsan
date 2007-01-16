/******************************
 * Proxy - (c) N.Sauzede 2005 *
 ******************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <libgen.h>
#include <netdb.h>

#define PROXY_PORT	1234

#define DEF_HOST	"127.0.0.1"
#define DEF_S_PORT	5000
#define DEF_C_PORT	5001
#define BUF_SIZE	(10 * 1024)
#define MAX_HOST	64
#define CMD_PROXY	"proxy"		// s_host s_port [c_port]
#define CMD_QUIT	"quit"		// client
#define CMD_EXIT	"exit"		// app

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

int main( int argc, char *argv[])
{
	int css, ss, cs, max;
	char *ch = "127.0.0.1";
	int sp = 5001, cp = 5000;
	struct sockaddr_in csa, sa, ca;
	int cscol = 32, ccol = 31;
	int end, arg = 1, n, on;
	int init = 0;
	int disc = 0;
	
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
	printf( "++remote client=%s port=%d, local server port=%d\n", ch, cp, sp);

	ss = socket( PF_INET, SOCK_STREAM, 0);
	memset( &sa, 0, sizeof( sa));
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons( sp);
	sa.sin_family = AF_INET;
	on = 1;
	setsockopt( ss, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on));
	bind( ss, (struct sockaddr *)&sa, sizeof( sa));
	listen( ss, 1);

	end = 0;
	while (!end)
	{
		socklen_t clen;
		struct hostent *he;
		
		printf( "++accepting..\n");
		clen = sizeof( csa);
		css = accept( ss, (struct sockaddr *)&csa, &clen);
	
		printf( "++accepted !!\n");

		printf( "++connecting server..\n");
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
			perror( "connect");
			exit( 1);
		}

		printf( "++connected !!\n");
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
			FD_SET( 0, &rfds);
			FD_SET( cs, &rfds);
			FD_SET( css, &rfds);
			tv.tv_sec = 0;
			tv.tv_usec = 1000;
	
			ptr = buf;
			size = sizeof( buf);
			n = select( max, &rfds, NULL, NULL, &tv);
			if (n == -1)
				perror( "select");
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
					printf( "**select woken by unknown\n");
					n = 0;
				}
	
				if (n)
				{
					n = read( src, ptr, size);
					if (n < 0)
						perror( "read");
					else if (n == 0)
					{
/*						if (src == css)
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
//								printf( "* inspecting input.. ptr=%s\n", ptr);
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
								write( dst, ptr, n);
							if (src != 0)
							{
								asciify( buf, n + 1);
								if (col)
									printf( "\x1b[01;%02dm%s\x1b[00m", col, buf);
								else
									printf( "\x1b[00m%s\x1b[00m", buf);
								printf( "\x1b[m");
								fflush( stdout);
							}
						}
					}
				}
			}
//			else
//			{//timeout
//			}
		}
		printf( "++closing client\n");
		close( css);
		printf( "++closing server\n");
		close( cs);
	}
	printf( "++closing listening server\n");
	close( ss);
	
	return 0;
}
