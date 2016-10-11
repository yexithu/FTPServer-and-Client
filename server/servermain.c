#include "defs.h"

void init_globalvar() {
	for (int i = 0; i < MAX_THREAD; ++i) {
		thread_pool[i].isset = 0;
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

int startserver(int port, char* root) {
	int listenfd;
	struct sockaddr_in addr;
	// char sentence[8192];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	int newfd;
	while (1) {
		if ((newfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}

		int new_threadid = get_avaliable_thread();

		//No left thread refuse request
		if (new_threadid < 0) {
			
		} else {
			responseto(newfd, "220\n");
		}
	}

	close(listenfd);

}

int main(int argc, char** argv) {
	char root[128] = "/tmp";
	int port = 21;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-port") == 0) {
			++i;
			port = atoi(argv[i]);
		} else if (strcmp(argv[i], "-root") == 0) {
			++i;
			strcpy(root, argv[i]);
		} else {
			printf("Invalid arguements %s\n", argv[i]);
			exit(-1);
		}
	}
	printf("port %d\n", port);
	printf("root %s\n", root);
	init_globalvar();
	startserver(port, root);

	return 0;
}
