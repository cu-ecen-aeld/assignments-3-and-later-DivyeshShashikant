CC=${CROSS_COMPILE}gcc
CFLAGS=-g -Wall -Werror 

default: aesdsocket

all : aesdsocket

aesdsocket: aesdsocket.o
	${CC} ${CFLAGS} aesdsocket.c -o aesdsocket -lpthread

clean:
	-rm -f *.o aesdsocket
