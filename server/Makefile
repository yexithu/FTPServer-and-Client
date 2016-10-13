GCC = gcc
CFLAGS = -std=gnu99 -Wall
LIBS = -pthread

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
	rm *.o	
	rm server
