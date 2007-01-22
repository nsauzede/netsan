ifndef CC
CC:=gcc
endif
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep "mingw")),)
WIN32=true
endif

TARGET=	tproxy
ifndef WIN32
TARGET+=	gproxy
endif

CFLAGS=	-Wall -g -O0

ifdef WIN32
LDFLAGS+= -lwsock32
CFLAGS+= -mno-cygwin
endif


GTK_FLAGS=	$(CFLAGS) `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

all:	$(TARGET)

tproxy:	tproxy.o
	$(CC) -o $@ $^ $(LDFLAGS)

gproxy:	gproxy.c
	$(CC) $(GTK_FLAGS) $< -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) *.o

