#include "defs.h"

int random_port() {
	return rand() % (65535 - 20000) + 20000;
}

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


int client_pasvupload(char* src, char* dst) {
	
	return 0;
}

int client_portupload(char* src, char* dst) {

	return 0;
}


int client_upload(char* src, char* dst) {
	// printf("UPLOAD\n");
	if (clientinfo.mode == CLIENT_MODE_NON) {
		printf("You should set a mode before upload\n");
		return -1;
	}
	if (clientinfo.mode == CLIENT_MODE_PASV) {
		client_pasvupload(src, dst);
	}
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		client_portupload(src, dst);
	}
	clientinfo.mode = CLIENT_MODE_NON;
	return 0;
}

int client_pasvdownload(char* src, char* dst) {

	return 0;
}

int client_portdownload(char* src, char* dst) {

	return 0;
}

int client_download(char* src, char* dst) {
		// printf("UPLOAD\n");
	if (clientinfo.mode == CLIENT_MODE_NON) {
		printf("You should set a mode before download\n");
		return -1;
	}
	if (clientinfo.mode == CLIENT_MODE_PASV) {
		client_pasvdownload(src, dst);
	}
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		client_portdownload(src, dst);
	}
	clientinfo.mode = CLIENT_MODE_NON;
	return 0;
}

int client_setpasv() {
	// printf("SETPASV\n");
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		close(clientinfo.transferfd);
	}
	clientinfo.mode = CLIENT_MODE_PASV;

	bs_sendstr(clientinfo.controlfd, "PASV\n");

	char buffer[4096];
	int len = 4096;
	client_readresp(buffer, len);

	bs_parseipandport(buffer+5, clientinfo.ipv4, &clientinfo.transferport);

	// char showip[20];
	// sprintf(showip, "%d.%d.%d.%d.%d.%d", 
	// 	clientinfo.ipv4[0],clientinfo.ipv4[1],clientinfo.ipv4[2],clientinfo.ipv4[3],
	// 	clientinfo.transferport / 256, clientinfo.transferport % 256);
	// printf("CLIENT PASV %s\n", showip);
	return 0;
}

int client_setport() {
	// printf("SETPORT\n");

	printf("SET MODE PASV\n");
	if (clientinfo.mode == CLIENT_MODE_PASV) {
		close(clientinfo.transferfd);
	}
	clientinfo.mode  = CLIENT_MODE_PASV;

	
	if ((clientinfo.transferfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error Client PORT with socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	struct sockaddr_in addr;
	unsigned short int port;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	while (1) {
		port = random_port();
		addr.sin_port = htons(port);
		if (bind(clientinfo.transferfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
			continue;
		}

		if (listen(clientinfo.transferfd, 1) == -1) {
			continue;
		}
		break;
	}
	memcpy(clientinfo.ipv4, local_ipv4, 4);
	clientinfo.transferport = port;

	char req[40];
	sprintf(req, "PORT %d,%d,%d,%d,%d,%d\n", local_ipv4[0], local_ipv4[1],
		local_ipv4[2], local_ipv4[3], port / 256, port % 256);

	bs_sendstr(clientinfo.controlfd, req);
	char buffer[4096];
	int len = 4096;
	client_readresp(buffer, len);
	printf("Recv %s\n", buffer);

	return 0;
}

int client_exit() {	
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
		printf(">>");
		scanf("%s", buffer);
		bs_parserequest(buffer, command, (char *) parameters, paramlen, &argc);
		// printf("CMD [%s] [%u]", buffer, strlen(buffer));
		if (strcmp(command, "help") == 0) {
			printf("Usage\n"
				   "-----------------------------------------\n"
				   "help                            show help\n"
				   "upload [src] [dst]          upload a file\n"
				   "download [src] [dst]      download a file\n"
				   "setpasv                     set mode pasv\n"
				   "setport                     set mode port\n"
				   "-----------------------------------------\n\n");
		}
		else if (strcmp(command, "upload") == 0) {
			client_upload(parameters[0], parameters[1]);
		}	
		else if (strcmp(command, "download") == 0) {
			client_download(parameters[0], parameters[1]);
		}
		else if (strcmp(command, "setpasv") == 0) {
			client_setpasv();
		}
		else if (strcmp(command, "setport") == 0) {
			client_setport();
		}
		else if (strcmp(command, "exit") == 0) {
			client_exit();
			client_sendandcheck("USER anonymous\n", buffer, len, "221");			
			break;
		}
		else {
			printf("Command not supported\n");
		}
	}
	return 0;
}

int startclient() {

	clientinfo.mode = CLIENT_MODE_NON;

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


	struct ifaddrs *addrs;
	getifaddrs(&addrs);
	struct ifaddrs * tmp = addrs;

	while (tmp) 
	{
	    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
	    {
	        struct sockaddr_in *p_addr = (struct sockaddr_in *)tmp->ifa_addr;
	        uint32_t ipbytes = (p_addr->sin_addr).s_addr;
	        memcpy(local_ipv4, (unsigned char *) &ipbytes, 4);
	        if (local_ipv4[0] != 127) {
	        	char ipv4[20];
	        	sprintf(ipv4, "%d.%d.%d.%d", local_ipv4[0], local_ipv4[1],
	        		 local_ipv4[2], local_ipv4[3]);
	        	printf("Loacal IP: %s\n", ipv4);
	        	break;
	        }
	    }
	    tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);

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
