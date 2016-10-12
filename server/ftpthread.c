#include "defs.h"

//Thread info define
// struct ftpthread_info {
// 	//Controlled by servermain
// 	pthread_t threadid;
// 	int index;
// 	int isset;
// 	int controlfd;

// 	//Controlled by threadmain
// 	int mode;
// 	int transferfd;
//  char ip_addr[17];
//	unsigned short int transferport;
// };


void *ftpthread_main(void * args) {
	struct ftpthread_info * t_info = (struct ftpthread_info *) args;
	printf("Thread %d start\n", t_info->index);

	ftpthread_init(t_info);

	char buffer[4096];
	int len = 4096;
	char verb[10];
	char parameters[4][1024];
	int paramlen = 1024;
	int argc;

	//wait USER
	while (1) {
		if (bs_readline(t_info->controlfd, buffer, len) < 0) {
			//teminate
			break;
		}
		if (strcmp(buffer, "USER anonymous") == 0) {
			bs_sendstr(t_info->controlfd, "331 Login Ok! Send your email\n");
			break;
		} else {
			bs_sendstr(t_info->controlfd, "530 USER not accpted\n");
		}
	}

	//wati PASS
	while (1) {
		if (bs_readline(t_info->controlfd, buffer, len) < 0) {
			//teminate
			break;
		}
		if (strncmp(buffer, "PASS", 4) == 0) {
			bs_sendstr(t_info->controlfd, "230 Welcome\n");
			break;
		} else {
			bs_sendstr(t_info->controlfd, "530 PASS not accpted\n");
		}
	}

	while (1) {
		if (bs_readline(t_info->controlfd, buffer, len) < 0) {
			//teminate
			break;
		}
		printf("thread %d recive message\n%s\n", t_info->index, buffer);
		if (strcmp(buffer, "SYST") == 0) {
			bs_sendstr(t_info->controlfd, "215 UNIX Type: L8\n");
		} 
		else if (strcmp(buffer, "TYPE I") == 0) {
			bs_sendstr(t_info->controlfd, "200 Type set to I.\n");
		} 
		else if (strncmp(buffer, "PORT", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			ftpthread_setmodeport(t_info, parameters[0]);
			bs_sendstr(t_info->controlfd, "200 PORT command successful\n");
		} 
		else if (strncmp(buffer, "RETR", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			ftpthread_retr(t_info, parameters[0]);
		}
	}

	return 0;
}

void ftpthread_setmodeport(struct ftpthread_info* t_info, char* param) {
	//Get AddrInfo need IP and Port
	//Build Socket need things from AddrInfo
	//Connect need socket and things from AddrInfo
	//Save IP and Port
	printf("SET MODE PORT %s\n", param);
	t_info->mode = THREAD_MODE_PORT;
	bs_parseipandport(param, t_info->ipv4, &(t_info->transferport));
}

void ftpthread_init(struct ftpthread_info * t_info) {
	//Set mode
	t_info->mode = 0;
	t_info->transferfd = 0;
}

int ftpthread_retr(struct ftpthread_info* t_info, char* fname) {
	//Check name
	if ((strsrtr(fname, "..") == !NULL) ||
		(strlen(fname) == 0)) {
		bs_sendstr(t_info->controlfd, "550 Permission denied\n");
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}

	if (fname[0] != '/') {
		char temp[1024];
		strcpy(temp, fname);
		strcpy(fname, "/");
		strcat(fname, temp);
	}

 	if (t_info->mode == THREAD_MODE_NON) {
		bs_sendstr(t_info->controlfd, "550 Mode not set\n");
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PORT) {
		ftpthread_portretr(t_info, fname);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PASV) {
		ftpthread_pasvretr(t_info, fname);
		return 0;
	}

}

int ftpthread_portretr(struct ftpthread_info* t_info, char* fname) {

	return 0;
}

int ftpthread_pasvretr(struct ftpthread_info* t_info, char* fname) {
	return 0;
}