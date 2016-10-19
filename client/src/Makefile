GCC = gcc
CFLAGS = -std=gnu99 -Wall
# LIBS = -pthread

client: client.o bytestream.o ftpcommon.o
	$(GCC) -o client $(CFLAGS)\
		client.o bytestream.o ftpcommon.o

client.o: client.c defs.h
	$(GCC) -c $(CFLAGS) client.c

bytestream.o: bytestream.c defs.h	
	$(GCC) -c $(CFLAGS) bytestream.c

ftpcommon.o: ftpcommon.c defs.h	
	$(GCC) -c $(CFLAGS) ftpcommon.c

.PHONY: clean
clean:
	rm -rf *.o	
	rm -rf client
