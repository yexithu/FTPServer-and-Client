#include "defs.h"

int random_port() {
	return rand() % (65535 - 20000) + 20000;
}

int client_sendstr(char* req) {
	char req_new[1024];
	int reqlen = strlen(req);
	strcpy(req_new, req);
	req_new[reqlen - 1] = '\r';
	req_new[reqlen] = '\n';
	req_new[reqlen + 1] = '\0';
	bs_sendstr(clientinfo.controlfd, req_new);
	return 0;
}

int client_sendandcheck(char* req, char* buf, int len, char* expect) {
	if (req != NULL) {
		char req_new[1024];
		int reqlen = strlen(req);
		strcpy(req_new, req);
		req_new[reqlen - 1] = '\r';
		req_new[reqlen] = '\n';
		req_new[reqlen + 1] = '\0';
		bs_sendstr(clientinfo.controlfd, req_new);
	}
	if (client_readresp(buf, len) < 0) {
		return -1;
	}
	if (strncmp(buf, expect, 3) != 0) {
		return -1;
	} else {
		return 0;
	}
}

int client_sendandprint(char* req, char* buf, int len, char* expect) {
	if (req != NULL) {
		char req_new[1024];
		int reqlen = strlen(req);
		strcpy(req_new, req);
		req_new[reqlen - 1] = '\r';
		req_new[reqlen] = '\n';
		req_new[reqlen + 1] = '\0';
		bs_sendstr(clientinfo.controlfd, req_new);
	}
	if (client_readresp(buf, len) < 0) {
		return -1;
	}
	printf("%s\n", buf);
	if (strncmp(buf, expect, 3) != 0) {
		return -1;
	} else {
		return 0;
	}
}

int client_showresult(int status, char* command) {
	if (status < 0) {
		printf("%s failure\n", command);
	} else {
		printf("%s success\n", command);
	}
	return 0;
}

int client_init() {
	clientinfo.mode = CLIENT_MODE_PASV;
	clientinfo.transferfd = -1;
	return 0;
}

int client_info() {
	if (clientinfo.mode == CLIENT_MODE_NON) {
		printf("Mode not set\n");
	} else if (clientinfo.mode == CLIENT_MODE_PASV) {
		printf("Mode pasv\n");
	} else {
		printf("Mode port\n");
	}
	return 0;
}

int client_readresp(char* buf, int len) {
	memset(buf, 0, len);
	char line_buf[1024];
	int line_len = 1024;
	char pattern[5];
	memset(pattern, 0, 5);
	while(1) {
		if (bs_readline(clientinfo.controlfd, line_buf, line_len) < 0) {
			return -1;
		}
		strcat(buf, line_buf);
		strncpy(pattern, line_buf, 3);
		strcat(pattern, " ");
		if (strstr(line_buf, pattern)) {
			break;
		}
	}
	return 0;
}

int client_sendpasv() {
	// printf("SETPASV\n");
	bs_sendstr(clientinfo.controlfd, "PASV\r\n");

	char buffer[4096];
	int len = 4096;
	client_readresp(buffer, len);
	char* temp = strchr(buffer, '(');
	bs_parseipandport(temp + 1, clientinfo.ipv4, &clientinfo.transferport);

	// char req[1024];
	// sprintf(req, "PASV [%d,%d,%d,%d,%d,%d]\n", clientinfo.ipv4[0], clientinfo.ipv4[1],
	// 	clientinfo.ipv4[2], clientinfo.ipv4[3], 
	// 	clientinfo.transferport / 256, clientinfo.transferport % 256);
	// printf("%s\n", req);
	return 0;
}

int client_sendport() {
	// printf("SETPORT\n");
	//Set info transferfd, port, ip
	if (ftpcommon_openandlisten(&(clientinfo.transferfd), 
							&(clientinfo.transferport)) < 0) {
		return -1;
	}
	memcpy(clientinfo.ipv4, local_ipv4, 4);

	char req[40];
	sprintf(req, "PORT %d,%d,%d,%d,%d,%d\n", clientinfo.ipv4[0], clientinfo.ipv4[1],
		clientinfo.ipv4[2], clientinfo.ipv4[3], 
		clientinfo.transferport / 256, clientinfo.transferport % 256);

	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(req, buffer, len, "200") < 0) {
		return -1;
	}
	return 0;
}

//PASV connect to there
int client_pasvupload(char* src, char* dst) {
		//Here we have connect to the client	
    FILE* fp = fopen(src, "r");
    if(!fp) {
    	printf("Upload File Not Found\n");
		return -1;
    }

	client_sendpasv();

	char req[1024];
	strcpy(req, "STOR ");
	strcat(req, dst);
	strcat(req, "\n");
	client_sendstr(req);

	if (ftpcommon_connectandgetsock(&(clientinfo.transferfd), clientinfo.ipv4, 
	clientinfo.transferport) < 0) {
		return -1;
	}

	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(NULL, buffer, len, "150") < 0) {
		return -1;
	}



	//Here we have connect to the client	
    int status = bs_sendfile(clientinfo.transferfd, fp);
    close(clientinfo.transferfd);
	fclose(fp);
	status = client_sendandcheck(NULL, buffer, len, "226");
	if (status == -1) {
		printf("File upload failure\n");
	} else if (status == 0) {
		printf("File upload success\n");
	}
	return status;
}

int client_portupload(char* src, char* dst) {
			//Here we have connect to the client	
    FILE* fp = fopen(src, "r");
    if(!fp) {
    	printf("Upload File Not Found\n");
		return -1;
    }

	client_sendport();
	char req[1024];
	strcpy(req, "STOR ");
	strcat(req, dst);
	strcat(req, "\n");

	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(req, buffer, len, "150") < 0) {
		printf("File upload failure\n");
		return -1;
	}

	int newfd;
    if ((newfd = accept(clientinfo.transferfd, NULL, NULL)) == -1) {
		close(clientinfo.transferfd);
		printf("File upload failure\n");
		return -1;
	}

	int status = bs_sendfile(newfd, fp);
	close(newfd);
    close(clientinfo.transferfd);
	fclose(fp);
	status = client_sendandcheck(NULL, buffer, len, "226");
	if (status == -1) {
		printf("File upload failure\n");
	} else if (status == 0) {
		printf("File upload success\n");
	}

	return status;
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
	// clientinfo.mode = CLIENT_MODE_NON;
	return 0;
}

int client_pasvdownload(char* src, char* dst) {
	//Here we have connect to the client	
    FILE* fp = fopen(dst, "w");
    if(!fp) {
    	printf("Download file cannot write\n");
		return -1;
    }

	client_sendpasv();


	char req[1024];
	strcpy(req, "RETR ");
	strcat(req, src);
	strcat(req, "\n");

	client_sendstr(req);


	if (ftpcommon_connectandgetsock(&(clientinfo.transferfd), clientinfo.ipv4, 
	clientinfo.transferport) < 0) {
		return -1;
	}


	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(NULL, buffer, len, "150") < 0) {
		return -1;
	}

	//Here we have connect to the client	
    int status = bs_recvfile(clientinfo.transferfd, fp);
    close(clientinfo.transferfd);
		
	status = client_sendandcheck(NULL, buffer, len, "226");
	if (status == -1) {
		printf("File download failure\n");
	} else if (status == 0) {
		printf("File download success\n");
	}
	fclose(fp);
	return status;
}

int client_list(char* src) {
	if (clientinfo.mode == CLIENT_MODE_NON) {
		printf("You should set a mode before download\n");
		return -1;
	}
	char req[1024];
	if (src == NULL) {
		strcpy(req, "LIST\r\n");
	} else {
		sprintf(req, "LIST %s\r\n", src);
	}
	if (clientinfo.mode == CLIENT_MODE_PASV) {
		return client_pasvlist(req);
	}
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		return client_portlist(req);
	}
	return 0;
}

int client_pasvlist(char* req) {
	//Here we have connect to the client	
	client_sendpasv();

	// char buffer[1024];
	// int len = 1024;
	// if (client_sendandprint(req, buffer, len, "150") < 0) {
	// 	return -1;
	// }
	client_sendstr(req);
	if (ftpcommon_connectandgetsock(&(clientinfo.transferfd), clientinfo.ipv4, 
	clientinfo.transferport) < 0) {
		printf("PASV LIST\n");
		return -1;
	}

	char buffer[1024];
	int len = 1024;
	if (client_sendandprint(NULL, buffer, len, "150") < 0) {
		return -1;
	}


	FILE* fp = stdout;
    if(!fp) {
    	// printf("Popen file cannot write\n");
		return -1;
    }
    int status = bs_recvfile(clientinfo.transferfd, fp);
    close(clientinfo.transferfd);
	// fclose(fp);
	status = client_sendandcheck(NULL, buffer, len, "226");
	// if (status == -1) {
	// 	printf("LIST failure\n");
	// } else if (status == 0) {
	// 	printf("LIST success\n");
	// }
	
	return status;
}

int client_portlist(char* req) {
	client_sendport();

	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(req, buffer, len, "150") < 0) {
		return -1;
	}

	int newfd;
    if ((newfd = accept(clientinfo.transferfd, NULL, NULL)) == -1) {
		close(clientinfo.transferfd);
		// printf("File upload failure\n");
		return -1;
	}

	FILE* fp = stdout;
    if(!fp) {
    	// printf("Download file cannot write\n");
		return -1;
    }

	int status = bs_recvfile(newfd, fp);
	close(newfd);
    close(clientinfo.transferfd);
	// fclose(fp);
	status = client_sendandcheck(NULL, buffer, len, "226");
	// if (status == -1) {
	// 	printf("LIST failure\n");
	// } else if (status == 0) {
	// 	printf("LIST success\n");
	// }

	return status;
}

int client_rename(char* src, char* dst) {
	char req[1024];
	int req_len = 1024;
	memset(req, 0, req_len);
	sprintf(req, "RNFR %s\n", src);

	char buffer[1024];
	int buf_len = 1024;
	if (client_sendandcheck(req, buffer, buf_len, "350") < 0) {
		return -1;
	}
	memset(req, 0, req_len);
	sprintf(req, "RNTO %s\n", dst);
	return client_sendandcheck(req, buffer, buf_len, "250");
}

int client_portdownload(char* src, char* dst) {

	FILE* fp = fopen(dst, "w");
    if(!fp) {
    	printf("Download file cannot write\n");
		return -1;
    }
	client_sendport();

	char req[1024];
	strcpy(req, "RETR ");
	strcat(req, src);
	strcat(req, "\n");

	char buffer[1024];
	int len = 1024;
	if (client_sendandcheck(req, buffer, len, "150") < 0) {
		return -1;
	}

	int newfd;
    if ((newfd = accept(clientinfo.transferfd, NULL, NULL)) == -1) {
		close(clientinfo.transferfd);
		printf("File upload failure\n");
		return -1;
	}

	int status = bs_recvfile(newfd, fp);
	close(newfd);
    close(clientinfo.transferfd);
	fclose(fp);
	status = client_sendandcheck(NULL, buffer, len, "226");
	if (status == -1) {
		printf("File download failure\n");
	} else if (status == 0) {
		printf("File download success\n");
	}

	return status;
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
	// clientinfo.mode = CLIENT_MODE_NON;
	return 0;
}
// int client_rename(char)
int client_setport() {
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		if (clientinfo.transferfd > 0) {
			close(clientinfo.transferfd);
			clientinfo.transferfd = -1;
		}		
	}
	clientinfo.mode = CLIENT_MODE_PORT;
	return 0;
}

int client_setpasv() {
	// clientinfo.mode = CLIENT_MODE_PASV;	
	if (clientinfo.mode == CLIENT_MODE_PORT) {
		if (clientinfo.transferfd > 0) {
			close(clientinfo.transferfd);
			clientinfo.transferfd = -1;
		}		
	}
	clientinfo.mode = CLIENT_MODE_PASV;
	return 0;
}

int client_exit() {
	close(clientinfo.controlfd);
	printf("The client will be closed\n");
	return 0;
}

int client_login() {
	char buffer[1024];
	char req[1024];
	int len = 1024;

	// client_sendandcheck(NULL, buffer, len, "220");
	if (client_sendandprint(NULL, buffer, len, "220") < 0) {
		printf("Server reject connection\n");
		return -1;
	}

	int looptime = 3;
	int login_status = -1;
	while(looptime > 0) {
		--looptime;
		printf("Username: ");
		fgets(buffer, len, stdin);
		buffer[strlen(buffer) - 1] = 0;
		sprintf(req, "USER %s\n", buffer);
		if (client_sendandprint(req, buffer, len, "331") < 0) {
			continue;
		}

		printf("Password: ");
		fgets(buffer, len, stdin);
		buffer[strlen(buffer) - 1] = 0;
		sprintf(req, "PASS %s\n", buffer);
		if (client_sendandprint(req, buffer, len, "230") < 0) {
			continue;
		}
		login_status = 0;
		break;
	}
	if (login_status < 0) {
		printf("Invalid username and password\n");
		return -1;
	}
	client_sendandprint("SYST\n", buffer, len, "215");
	client_sendandprint("TYPE I\n", buffer, len, "200");

	return 0;
}

int client_mainloop() {
	char buffer[4096];
	int len = 4096;
	char command[10];
	char parameters[4][1024];
	int paramlen = 1024;
	int argc;

	char req[1024];
	int len_req = 1024;
	// char cmd[256];
	if (client_login() < 0) {
		client_exit();
		return 0;
	}

	while(1) {
		printf("FTP /> ");
		// fflush(stdin);
		fgets(buffer, len, stdin);
		buffer[strlen(buffer) - 1] = 0;
		bs_parserequest(buffer, command, (char *) parameters, paramlen, &argc);
		// printf("CMD [%s] [%d]", buffer, (int)strlen(buffer));
		if (strcmp(command, "help") == 0) {
			printf("\nUsage\n"
				   "----------------------------------------------------------\n"
				   "help                                             show help\n"
				   "info                                             show info\n"
				   "pwd(host) | lpwd(local)                           show pwd\n"
				   "cwd(host) | lcwd(local)                         cwd to dst\n"
				   "cdup                                                 cd up\n"
				   "mkdir(host) | lmkdir(local) [dir]                    mkdir\n"
				   "rmdir(host) | lrmdir(local) [dir]                    rmdir\n"
				   "rm(host) | lrm(local) [file]                   delete file\n"
				   "list(host) | llist(local) [dir]                   list dir\n"
				   "rename(host) | lrename(local) [src] [dst]    rename a file\n"
				   "upload [src] [dst]                           upload a file\n"
				   "download [src] [dst]                       download a file\n"
				   "setpasv                                      set mode pasv\n"
				   "setport                                      set mode port\n"
				   "----------------------------------------------------------\n\n");
		}
		else if (strcmp(command, "info") == 0) {
			client_info();
		}
		else if (strcmp(command, "upload") == 0) {
			if (argc < 2) {
				printf("Too few arguements\n");
				continue;
			}
			client_upload(parameters[0], parameters[1]);
		}	
		else if (strcmp(command, "download") == 0) {
			if (argc < 2) {
				printf("Too few arguements\n");
				continue;
			}
			client_download(parameters[0], parameters[1]);
		}
		else if (strcmp(command, "setpasv") == 0) {
			client_showresult(client_setpasv(), "SETPASV");
		}
		else if (strcmp(command, "setport") == 0) {
			client_showresult(client_setport(), "SETPORT");
		}
		else if (strcmp(command, "pwd") == 0) {
			client_sendandprint("PWD\n", buffer, len, "257");
		}
		else if (strcmp(command, "cwd") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			}
			memset(req, 0, len_req);
			sprintf(req, "CWD %s\n", parameters[0]);
			client_sendandprint(req, buffer, len, "250");
		}
		else if (strcmp(command, "cdup") == 0) {
			client_sendandprint("CDUP\n", buffer, len, "250");
		}
		else if (strcmp(command, "mkdir") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			}
			memset(req, 0, len_req);
			sprintf(req, "MKD %s\n", parameters[0]);
			client_sendandprint(req, buffer, len, "250");
		}
		else if (strcmp(command, "rmdir") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			}
			memset(req, 0, len_req);
			sprintf(req, "RMD %s\n", parameters[0]);
			client_sendandprint(req, buffer, len, "250");
		}
		else if (strcmp(command, "rm") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			}
			memset(req, 0, len_req);
			sprintf(req, "DELE %s\n", parameters[0]);
			client_sendandprint(req, buffer, len, "250");
		}
		else if (strcmp(command, "rename") == 0) {
			if (argc < 2) {
				printf("Too few arguements\n");
				continue;
			}
			client_showresult(client_rename(parameters[0], parameters[1]), "RENAME");
		}
		else if (strcmp(command, "list") == 0) {
			int status = -1;
			if (argc < 1) {
				status = client_list(NULL);
			} else {
				status = client_list(parameters[0]);
			}
			client_showresult(status, "LIST");
		}
		else if (strncmp(command, "exit", 4) == 0) {
			client_sendandprint("QUIT\n", buffer, len, "221");
			break;
		}
		else if (strcmp(command, "lpwd") == 0) {
			char cwd[256];
			getcwd(cwd, 256);
			printf("%s\n", cwd);
		}
		else if (strcmp(command, "llist") == 0) {
			system("ls -p");
		}
		else if (strcmp(command, "lcwd") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			} else {
				client_showresult(chdir(parameters[0]), "LCWD");
			}
		}
		else if (strcmp(command, "lmkdir") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			} else {
				client_showresult(mkdir(parameters[0], 0777), "LMKDIR");
			}
		}
		else if (strcmp(command, "lrmdir") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			} else {
				client_showresult(rmdir(parameters[0]), "LRMDIR");
			}
		}
		else if (strcmp(command, "lrm") == 0) {
			if (argc < 1) {
				printf("Too few arguements\n");
				continue;
			} else {
				client_showresult(remove(parameters[0]), "LRM");
			}
		}
		else if (strcmp(command, "lrename") == 0) {
			if (argc < 2) {
				printf("Too few arguements\n");
				continue;
			} else {
				client_showresult(rename(parameters[0], parameters[1]), "LRENAME");
			}
		}
		else {
			if (strlen(command) > 0) {
				printf("Command not supported\n");
			}
		}
	}
	client_exit();
	return 0;
}

int startclient() {
	client_init();

	if (ftpcommon_connectandgetsock(&clientinfo.controlfd, 
		      server_ipv4, server_port) < 0) {
		printf("Error: Cannot conncet to server\n");
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
			printf("Error: Invalid arguements %s\n", argv[i]);
			exit(-1);
		}
	}

	struct sockaddr_in antelope;
	if (inet_aton(server_ipstr, &antelope.sin_addr) == 0) {
		printf("Error: Invalid ip address %s\n", server_ipstr);
		exit(-1);
	} else {
		memcpy(server_ipv4, &antelope.sin_addr.s_addr, 4);
	}

	printf("Host IP %d.%d.%d.%d\n", server_ipv4[0], server_ipv4[1], 
		server_ipv4[2], server_ipv4[3]);
	printf("Host Port %d\n", server_port);
	printf("Loacal IP: %d.%d.%d.%d\n", local_ipv4[0], local_ipv4[1],
	 	local_ipv4[2], local_ipv4[3]);
	startclient();
	return 0;
}
