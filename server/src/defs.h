#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>

//Constants
#define MAX_THREAD 10
#define THREAD_MODE_NON 0
#define THREAD_MODE_PORT 1
#define THREAD_MODE_PASV 2
#define TRANS_BUF_SIZE 8192
//Type define
struct ftpthread_info {
	//Controlled by servermain
	pthread_t threadid;
	int index;
	int isset;
	int controlfd;

	//Controlled by threadmain
	int mode;
	int transferfd;
	unsigned char ipv4[4];
	unsigned short int transferport;
};

//Global variables
int servermain_port;
char servermain_root[128];
unsigned char servermain_ipv4[4];
struct ftpthread_info thread_pool[MAX_THREAD];

//servermain.c
void init_globalvar();
int get_avalible_thread();
int startserver();
int start_ftpthread(int new_threadid, int controlfd);

//ftpthread.c
void *ftpthread_main(void * args);
void ftpthread_init(struct ftpthread_info * t_info);
void ftpthread_setmodeport(struct ftpthread_info* t_info, char* param);
void ftpthread_setmodepasv(struct ftpthread_info* t_info);
int ftpthread_retr(struct ftpthread_info* t_info, char* fname);
int ftpthread_stor(struct ftpthread_info* t_info, char* fname);
int ftpthread_portretr(struct ftpthread_info* t_info, char* fname);
int ftpthread_pasvretr(struct ftpthread_info* t_info, char* fname);
int ftpthread_portstor(struct ftpthread_info* t_info, char* fname);
int ftpthread_pasvstor(struct ftpthread_info* t_info, char* fname);
int ftpthread_close(struct ftpthread_info* t_info);
int random_port();

//bytestream.c
int bs_sendstr(int fd, char* resp);
int bs_readline(int fd, char* buffer, int len);
int bs_parserequest(char* sentence, char* verb, 
					char* parameters, int paramlen, int *argc);
int bs_parseipandport(char* pram, unsigned char* ipv4, unsigned short int *port);
int bs_sendbytes(int fd, char* info, int len);
int bs_sendfile(int fd, FILE* fp);
int bs_recvfile(int fd, FILE* fp);

#endif