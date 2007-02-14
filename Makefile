PREFIX=/opt/bin

ifndef CC
CC:=gcc
endif
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep "mingw")),)
WIN32=true
endif

TARGET=	tproxy.exe proxy.exe
ifndef WIN32
#TARGET+=	gproxy
endif

CFLAGS=	-Wall -g -O0 -m32
LDFLAGS=	-m32

THREADF=
ifdef WIN32
LDFLAGS+= -lwsock32
CFLAGS+= -mno-cygwin
INSTALL= install
else
THREADF+=-lpthread
endif


GTK_FLAGS=	$(CFLAGS) `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

all:	$(TARGET)

tproxy.exe:	tproxy.o
	$(CC) -o $@ $^ $(LDFLAGS)

proxy.exe:	LDFLAGS+=$(THREADF)

proxy.exe:	proxy.o
	$(CC) -o $@ $^ $(LDFLAGS)

gproxy.exe:	gproxy.c
	$(CC) $(GTK_FLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

install:
	$(INSTALL) $(TARGET) $(PREFIX)

