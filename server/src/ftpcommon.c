#include "defs.h"

int ftpcommon_randomport() {
	return rand() % (65535 - 20000) + 20000;
}

int ftpcommon_openandlisten(int * in_fd, unsigned short int* in_port) {

	int transferfd;
	unsigned short int port;

	if ((transferfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error Open socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	while (1) {
		port = ftpcommon_randomport();
		addr.sin_port = htons(port);
		if (bind(transferfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
			continue;
		}

		if (listen(transferfd, 1) == -1) {
			continue;
		}
		break;
	}

	*in_fd = transferfd;
	*in_port = port;

	return 0;
}

int ftpcommon_setpassive() {
	return 0;
}