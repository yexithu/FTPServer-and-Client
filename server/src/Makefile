GCC = gcc
CFLAGS = -std=gnu99 -Wall
LIBS = -pthread

server: servermain.o ftpthread.o bytestream.o ftpcommon.o
	$(GCC) -o server $(CFLAGS) $(LIBS) \
		servermain.o ftpthread.o bytestream.o ftpcommon.o

servermain.o: servermain.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) servermain.c

ftpthread.o: ftpthread.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) ftpthread.c

bytestream.o: bytestream.c defs.h	
	$(GCC) -c $(CFLAGS) $(LIBS) bytestream.c

ftpcommon.o: ftpcommon.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) ftpcommon.c

.PHONY: clean
clean:
	rm -rf *.o	
	rm -rf server
