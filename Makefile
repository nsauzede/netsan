ifndef PREFIX
PREFIX=/opt
endif

LIBSOCKET=/home/sauzeden/hg/libsocket

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

TARGET=	tproxy$(EXT) ns$(EXT)
ifndef WIN32
#TARGET+=	gproxy$(EXT)
endif

CFLAGS=	-Wall -Werror -g -O0
#CFLAGS+=-m32
#LDFLAGS=	-m32

THREADF=
ifdef WIN32
LDFLAGS+= -L$(LIBSOCKET) -lsocket -lws2_32
#LDFLAGS+= -L./libsocket -lsocket -lws2_32
CFLAGS+= -I$(LIBSOCKET)/include
else
THREADF+=-lpthread
endif
INSTALL= install


all:	$(TARGET)

tproxy$(EXT):	tproxy.o
	$(CC) -o $@ $^ $(LDFLAGS)

ns$(EXT):	LDFLAGS+=$(THREADF)

ns$(EXT):	ns.o
	$(CC) -o $@ $^ $(LDFLAGS)

gproxy$(EXT):	gproxy.c
	$(CC) $(GTK_FLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

install: all
	$(INSTALL) $(TARGET) $(PREFIX)/bin

