TARGET=	tproxy gproxy
CFLAGS=	-Wall -g -O0

GTK_FLAGS=	$(CFLAGS) `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

all:	$(TARGET)

gproxy:	gproxy.c
	$(CC) $(GTK_FLAGS) $< -o $@

clean:
	$(RM) $(TARGET)
