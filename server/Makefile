GCC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -lpthread

server: servermain.o ftpthread.o bytestream.o
	$(GCC) -o server $(CFLAGS) $(LIBS) \
		servermain.o ftpthread.o bytestream.o

servermain.o: servermain.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) servermain.c

ftpthread.o: ftpthread.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) ftpthread.c

bytestream.o: bytestream.c defs.h	
	$(GCC) -c $(CFLAGS) $(LIBS) bytestream.c

.PHONY: clean
clean:
	rm server
	rm *.o
