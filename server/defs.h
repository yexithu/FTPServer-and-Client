#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

//Type define
struct ftpthread_info {
	pthread_t threadid;
	int isset;
	int controlfd;
	int transferfd;
	int mode;
};

//Constants
#define MAX_THREAD 10
#define THREAD_MODE_NON 0
#define THREAD_MODE_PORT 1
#define THREAD_MODE_PASV 2

// pthread_t thread_array[MAX_THREAD];
// int thread_flags[MAX_THREAD];

struct ftpthread_info thread_pool[MAX_THREAD];

//server.c
void init_globalvar();
int get_avalible_thread();


//response.c
int responseto(int fd, char* resp);

#endif