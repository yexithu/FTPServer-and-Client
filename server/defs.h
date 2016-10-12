#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>


//Constants
#define MAX_THREAD 10
#define THREAD_MODE_NON 0
#define THREAD_MODE_PORT 1
#define THREAD_MODE_PASV 2

//Type define
struct ftpthread_info {
	pthread_t threadid;
	int index;
	int isset;
	int controlfd;
	int transferfd;
	int mode;
};

//Global variables
struct ftpthread_info thread_pool[MAX_THREAD];

//server.c
void init_globalvar();
int get_avalible_thread();
int start_ftpthread(int new_threadid);

//response.c
int sendstr(int fd, char* resp);

//ftpthread.c
void *ftpthread_main(void * args);

#endif