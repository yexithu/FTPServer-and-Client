#include "defs.h"


int client_readresp(char* buf, int len) {
	memset(buf, 0, len);
	char line_buf[1024];
	int line_len = 1024;
	while(1) {
		if (bs_readline(clientinfo.controlfd, line_buf, line_len) < 0) {
			return -1;
		}
		strcat(buf, line_buf);
		if(line_buf[3]=='-') {
			continue;
		}
		break;
	}
	return 0;
}

int client_sendandcheck(char* req, char* buf, int len, char* expect) {
	if (req != NULL) {
		bs_sendstr(clientinfo.controlfd, req);
	}
	//Wait for greeting
	if (client_readresp(buf, len) < 0) {
		return -1;
	}
	printf("Clinet Recv %s\n", buf);
	if (strncmp(buf, expect, 3) != 0) {
		return -1;
	} else {
		return 0;
	}
}
int client_mainloop() {
	char buffer[4096];
	int len = 4096;
	char command[10];
	char parameters[4][1024];
	int paramlen = 1024;
	int argc;

	client_sendandcheck(NULL, buffer, len, "220");
	client_sendandcheck("USER anonymous\n", buffer, len, "331");
	client_sendandcheck("PASS test@163.com\n", buffer, len, "230");
	client_sendandcheck("SYST\n", buffer, len, "215");
	client_sendandcheck("TYPE I\n", buffer, len, "200");

	while(1) {
		scanf("%s", buffer);
		bs_parserequest(buffer, command, (char *) parameters, paramlen, &argc);
		// printf("CMD [%s] [%u]", buffer, strlen(buffer));
	}
	return 0;
}

int startclient() {
	//Build socket connect
	struct addrinfo hints, *res;
	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use IPv4
	hints.ai_socktype = SOCK_STREAM;
	char port_str[10];
	sprintf(port_str, "%d", server_port);

	if (getaddrinfo(server_ipstr, port_str, &hints, &res) != 0) {
		printf("ERROR PORT stor getaddrinfo\n");
		return -1;
	}

	if ((clientinfo.controlfd = 
		socket(res->ai_family, res->ai_socktype, IPPROTO_TCP)) < 0) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	//Socketfd has been opened, need closing when living
	if (connect(clientinfo.controlfd, res->ai_addr, res->ai_addrlen) < 0) {
		printf("ERROR Connection\n");
		close(clientinfo.controlfd);
		return -1;
	}

	client_mainloop();

	close(clientinfo.controlfd);
	printf("Client close\n");
	return 0;
}

int main(int argc, char **argv) {
	server_port = 21;
	strcpy(server_ipstr, "127.0.0.1");

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-port") == 0) {
			++i;
			server_port = atoi(argv[i]);
		} else if (strcmp(argv[i], "-ip") == 0) {
			++i;
			strcpy(server_ipstr, argv[i]);
		} else {
			printf("Invalid arguements %s\n", argv[i]);
			exit(-1);
		}
	}

	printf("Client IP %s\n", server_ipstr);
	printf("Client Port %d\n", server_port);
	startclient();
	return 0;
}
