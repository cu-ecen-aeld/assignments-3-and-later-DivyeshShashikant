
CC=${CROSS_COMPILE}gcc
CCFLAGS=-Wall -Werror
all : writer

writer: writer.o
	${CC} ${CCFLAGS}  writer.c -o writer

clean:
	-rm -f *.o writer
