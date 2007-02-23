ifndef PREFIX
PREFIX=/opt
endif

LIBUNIX=/home/sauzeden/hg/libunix

ifndef CC
CC:=gcc
endif
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep "mingw")),)
WIN32=true
else
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep -i "SunOS")),)
SOL8=true
endif
endif

ifdef WIN32
EXT=.exe
else
EXT=
endif

TARGET=	ns$(EXT)
ifndef WIN32
#TARGET+=	gproxy$(EXT)
endif

CFLAGS=	-Wall -Werror -g -O0
#CFLAGS+=-m32
#LDFLAGS=	-m32

INSTALL= install
THREADF=
ifdef WIN32
LDFLAGS+= -L$(LIBUNIX) -lunix -lws2_32
CFLAGS+= -I$(LIBUNIX)/include
else
ifdef SOL8
LDFLAGS+= -lsocket
INSTALL= cp -f
endif
THREADF+=-lpthread
endif


all:	$(TARGET)

tproxy$(EXT):	tproxy.o
	$(CC) -o $@ $^ $(LDFLAGS)

ns$(EXT):	LDFLAGS+=$(THREADF)

ns$(EXT):	netsan.o
	$(CC) -o $@ $^ $(LDFLAGS)

gproxy$(EXT):	gproxy.c
	$(CC) $(GTK_FLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

install: all
	$(INSTALL) $(TARGET) $(PREFIX)/bin

