#include "defs.h"

int ftpcommon_randomport() {
	return rand() % (65535 - 20000) + 20000;
}

int ftpcommon_openandlisten(int * in_fd, unsigned short int* in_port) {

	int transferfd;
	unsigned short int port;

	if ((transferfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
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

int ftpcommon_connectandgetsock(int *in_fd, unsigned char* host_ipv4, 
	unsigned short int host_port) {

	struct addrinfo hints, *res;
	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4
	hints.ai_socktype = SOCK_STREAM;
	char ipv4_str[20];
	char port_str[10];
	unsigned char* ipv4 = host_ipv4;
	sprintf(ipv4_str, "%d.%d.%d.%d", ipv4[0], ipv4[1],
					ipv4[2], ipv4[3]);
	sprintf(port_str, "%d", host_port);

	if (getaddrinfo(ipv4_str, port_str, &hints, &res) != 0) {
		return FTPCM_ERR_GETADDR;
	}

	int transferfd;
	if ((transferfd = 
		socket(res->ai_family, res->ai_socktype, IPPROTO_TCP)) < 0) {
		return FTPCM_ERR_BLDSOCKET;
	}
	*in_fd = transferfd;
	//Socketfd has been opened, need closing when living
	if (connect(transferfd, res->ai_addr, res->ai_addrlen) < 0) {
		close(transferfd);
		return FTPCM_ERR_CONNECT;
	}
	return 0;
}


int ftpcommon_setpassive() {
	return 0;
}