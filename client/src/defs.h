#ifndef CLIENT_H
#define CLIENT_H

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
// #define MAX_THREAD 10
#define CLIENT_MODE_NON 0
#define CLIENT_MODE_PORT 1
#define CLIENT_MODE_PASV 2
#define TRANS_BUF_SIZE 8192

struct client_info {
	int controlfd;
	int transferfd;

	int mode;
	unsigned char ipv4[4];
	unsigned short int transferport;
};

//Global variables
int server_port;
char server_ipstr[20];
unsigned char server_ipv4[4];
unsigned char local_ipv4[4];
struct client_info clientinfo;

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