CC=${CROSS_COMPILE}gcc
CFLAGS=-Wall -Werror
all : aesdsocket

aesdsocket: aesdsocket.o
	${CC} ${CFLAGS} aesdsocket.c -o aesdsocket

clean:
	-rm -f *.o aesdsocket
