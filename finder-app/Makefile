
all : writer

writer: writer.o
	$(CROSS_COMPILE)gcc -Wall -Werror writer.c -o writer

clean:
	-rm -f *.o writer
