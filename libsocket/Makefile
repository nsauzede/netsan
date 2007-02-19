PREFIX=/opt/bin

ifndef CC
CC:=gcc
endif
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep "mingw")),)
WIN32=true
endif

ifdef WIN32
EXT=.exe
else
EXT=
endif

TARGET=	libsocket.a ns$(EXT)
ifndef WIN32
#TARGET+=	gproxy$(EXT)
endif

CFLAGS=	-Wall -Werror -g -O0
#CFLAGS+=-m32
#LDFLAGS=	-m32

THREADF=
ifdef WIN32
#LDFLAGS+= -L. -Wl,--whole-archive -lsocket -Wl,--no-whole-archive -lwsock32
LDFLAGS+= -L. -lsocket -lwsock32
CFLAGS+= -I./include
#CFLAGS+= -mno-cygwin
INSTALL= install
else
THREADF+=-lpthread
endif


GTK_FLAGS=	$(CFLAGS) `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

all:	$(TARGET)

ns$(EXT):	ns.o
	$(CC) -o $@ $^ $(LDFLAGS) $(THREADF)

libsocket.o: src/socket.c
	$(CC) -c -o $@ $(CFLAGS) $^

libsocket_ctor.o: src/socket_ctor.cpp
	$(CC) -c -o $@ $(CFLAGS) $^

#libsocket.a: libsocket.o libsocket_ctor.o
libsocket.a: libsocket.o
	$(AR) cru $@ $^

clean:
	$(RM) $(TARGET) *.o

install: all
	$(INSTALL) $(TARGET) $(PREFIX)

