#include "defs.h"

void init_globalvar() {
	srand((unsigned int) time(NULL));

	for (int i = 0; i < MAX_THREAD; ++i) {
		thread_pool[i].index = i;
		thread_pool[i].isset = 0;
	}

	struct ifaddrs *addrs;
	getifaddrs(&addrs);
	struct ifaddrs * tmp = addrs;

	while (tmp) 
	{
	    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
	    {
	        struct sockaddr_in *p_addr = (struct sockaddr_in *)tmp->ifa_addr;
	        uint32_t ipbytes = (p_addr->sin_addr).s_addr;
	        memcpy(servermain_ipv4, (unsigned char *) &ipbytes, 4);
	        if (servermain_ipv4[0] != 127) {
	        	char ipv4[20];
	        	sprintf(ipv4, "%d.%d.%d.%d", servermain_ipv4[0], servermain_ipv4[1],
	        		 servermain_ipv4[2], servermain_ipv4[3]);
	        	printf("Loacal IP: %s\n", ipv4);
	        	break;
	        }
	    }
	    tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);

	build_usertable();
}

void build_usertable() {
	FILE* fp = fopen(CONFIG_FILE, "r");
    if(!fp) {
    	usertable = NULL;
    	return;
    }
    struct user_listnode *p, *q;
    p = q = NULL;

   	char buf[100];
   	int buf_len = 100;
    while (fgets(buf, buf_len, fp) != NULL) {
    	if (usertable == NULL) {
    		usertable = (struct user_listnode *) malloc(sizeof (struct user_listnode));
    		sscanf(buf, "%s %s\n", usertable->username, usertable->password);
    		// printf("ARGS [%s] [%s]\n", usertable->username, usertable->password);
    		usertable->next = NULL;
    		p = usertable;	
    	} else {
    		q = (struct user_listnode *) malloc(sizeof (struct user_listnode));
    		sscanf(buf, "%s %s\n", q->username, q->password);
    		// printf("ARGS [%s] [%s]\n", q->username, q->password);
    		q->next = NULL;
    		p->next = q;
    		p = q;
    	}
    }
 
    fclose(fp);
}

void free_usertable() {
	struct user_listnode *p, *q;
    p = q = NULL;
    p = usertable;
    while (p != NULL) {
    	q = p->next;
    	free(p);
    	p = q;
    }
}

int get_avaliable_thread() {
	for (int i = 0; i < MAX_THREAD; ++i) {
		if (thread_pool[i].isset == 0) {
			return i;
		}
	}
	return -1;
}

int start_ftpthread(int new_threadid, int controlfd) {
	int i = new_threadid;

	thread_pool[i].isset = 1;
	thread_pool[i].controlfd = controlfd;

	int ret_val = pthread_create(&thread_pool[i].threadid, NULL, 
		ftpthread_main, (void *) &thread_pool[i]);
	if (ret_val) {
		printf("Error pthread_create %d return code %d\n",
			i, ret_val);
		return -1;
	}
	return 0;
}

int startserver() {
	printf("Server started\n");
	int listenfd;
	struct sockaddr_in addr;
	// char sentence[8192];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        printf("Error set: %s(%d)\n", strerror(errno), errno);
        return 1;
    }

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(servermain_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	int newfd, new_threadid;
	while (1) {
		if ((newfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}

		new_threadid = get_avaliable_thread();		
		//No left thread refuse request
		if (new_threadid < 0) {
			bs_sendstr(newfd, "421 Too busy\n");
			close(newfd);
		} else {
			bs_sendstr(newfd, "220 FTP server ready\n");
			start_ftpthread(new_threadid, newfd);
		}
	}
	close(listenfd);
}

int main(int argc, char** argv) {
	servermain_port = 21;
	strcpy(servermain_root, "/tmp");
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-port") == 0) {
			++i;
			servermain_port = atoi(argv[i]);
		} else if (strcmp(argv[i], "-root") == 0) {
			++i;
			strcpy(servermain_root, argv[i]);
		} else {
			printf("Invalid arguements %s\n", argv[i]);
			exit(-1);
		}
	}
	printf("port %d\n", servermain_port);
	printf("root %s\n", servermain_root);
	init_globalvar();
	startserver();
	free_usertable();
	return 0;
}
