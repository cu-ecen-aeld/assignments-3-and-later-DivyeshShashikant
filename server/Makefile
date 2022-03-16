ifeq ($(CC),)
	CC = ${CROSS_COMPILE}gcc
endif
ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror 
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

default: aesdsocket

all : aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LDFLAGS)

clean: 
	-rm -f *.o aesdsocket
	
