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
#include <sys/stat.h>
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

//#define LOG_ON
//Constants
#define MAX_THREAD 10
#define THREAD_MODE_NON 0
#define THREAD_MODE_PORT 1
#define THREAD_MODE_PASV 2
#define TRANS_BUF_SIZE 8192
#define USER_BUF_SIZE 20
#define CONFIG_FILE "user.config"
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
	char pwd[128];

	int rnfrset;
	char rnfrname[128];
	char rntoname[128];

	int upload_filecount;
	int upload_filebytes;
	int download_filecount;
	int download_filebytes;
	int upload_traffic;
	int download_traffic;
};

struct user_listnode {
	char username[USER_BUF_SIZE];
	char password[USER_BUF_SIZE];
	struct user_listnode *next;
};

//Global variables
int servermain_port;
char servermain_root[128];
unsigned char servermain_ipv4[4];
struct ftpthread_info thread_pool[MAX_THREAD];
struct user_listnode* usertable;

//servermain.c
void build_usertable();
void free_usertable();
void init_globalvar();
int get_avalible_thread();
int startserver();
int start_ftpthread(int new_threadid, int controlfd);

//ftpthread.c
void *ftpthread_main(void * args);
int ftpthread_verifyuserpwd(char* user, char* pwd);
void ftpthread_init(struct ftpthread_info * t_info);
void ftpthread_setmodeport(struct ftpthread_info* t_info, char* param);
void ftpthread_setmodepasv(struct ftpthread_info* t_info);
int ftpthread_retr(struct ftpthread_info* t_info, char* fname);
int ftpthread_stor(struct ftpthread_info* t_info, char* fname);
int ftpthread_portretr(struct ftpthread_info* t_info, char* fname);
int ftpthread_pasvretr(struct ftpthread_info* t_info, char* fname);
int ftpthread_portstor(struct ftpthread_info* t_info, char* fname);
int ftpthread_pasvstor(struct ftpthread_info* t_info, char* fname);
int ftpthread_cwd(struct ftpthread_info* t_info, char* dir);
int ftpthread_close(struct ftpthread_info* t_info);
void ftpthread_parserealdir(char* pwd, char* input, char* ouput);
void ftpthread_parseworkdir(char* pwd, char* input, char* ouput);
int ftpthread_exsistdir(char *path);
int ftpthread_cdup(struct ftpthread_info* t_info);
int ftpthread_mkd(struct ftpthread_info* t_info, char* dir);
int ftpthread_rmd(struct ftpthread_info* t_info, char* dir);
int ftpthread_rnfr(struct ftpthread_info* t_info, char* name);
int ftpthread_rnto(struct ftpthread_info* t_info, char* name);
int ftpthread_list(struct ftpthread_info* t_info, char* dir);
int ftpthread_portlist(struct ftpthread_info* t_info, char* dir);
int ftpthread_pasvlist(struct ftpthread_info* t_info, char* dir);
int ftpthread_dele(struct ftpthread_info* t_info, char* name);
int random_port();

int ftpthread_sendstr(struct ftpthread_info* t_info, int fd,  char* resp);
int ftpthread_readline(struct ftpthread_info* t_info, int fd,
						char* buffer, int len);
int ftpthread_sendfile(struct ftpthread_info* t_info, int fd,
						 FILE* fp);
int ftpthread_recvfile(struct ftpthread_info* t_info, int fd,
						 FILE* fp);

//bytestream.c
int bs_sendstr(int fd, char* resp);
int bs_readline(int fd, char* buffer, int len);
int bs_parserequest(char* sentence, char* verb,
					char* parameters, int paramlen, int *argc);
int bs_parseipandport(char* pram, unsigned char* ipv4, unsigned short int *port);
int bs_sendbytes(int fd, char* info, int len);
int bs_sendfile(int fd, FILE* fp);
int bs_recvfile(int fd, FILE* fp);

// ftpcommon
#define FTPCM_ERR_GETADDR   -1
#define FTPCM_ERR_BLDSOCKET -2
#define	FTPCM_ERR_CONNECT   -3
#define FTPCM_ERR_BIND      -4

int ftpcommon_randomport();
int ftpcommon_openandlisten(int * in_fd, unsigned short int* in_port);
int ftpcommon_connectandgetsock(int *in_fd, unsigned char* host_ipv4,
	unsigned short int host_port);
int ftpcommon_setpassive();

#endif
