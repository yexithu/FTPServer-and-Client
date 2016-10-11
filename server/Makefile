GCC = gcc
CFLAGS = -std=c99 -Wall
LIBS = -lpthread

server: servermain.o ftpthread.o response.o
	$(GCC) -o server $(CFLAGS) $(LIBS) \
		servermain.o ftpthread.o response.o

servermain.o: servermain.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) servermain.c

ftpthread.o: ftpthread.c defs.h
	$(GCC) -c $(CFLAGS) $(LIBS) ftpthread.c

response.o: response.c defs.h	
	$(GCC) -c $(CFLAGS) $(LIBS) response.c

.PHONY: clean
clean:
	rm server
	rm *.o
