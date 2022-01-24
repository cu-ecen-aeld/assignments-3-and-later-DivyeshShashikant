writer: writer.o
	$(CROSS_COMPILE)gcc -o writer writer.o


writer.o: writer.c
	$(CROSS_COMPILE)gcc -c -Wall -Werror writer.c

clean:
	-rm *.o writer
