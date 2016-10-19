#include "defs.h"

int random_port() {
	return rand() % (65535 - 20000) + 20000;
}

int ftpthread_check(char *path) {
	if ( strstr(path, "..") ||
		(strlen(path) == 0)) {
		return -1;
	}
	return 0;
}

int ftpthread_exsistdir(char *path) {
	// DIR* dir = opendir(path);
	// if (dir)
	// {
	//     closedir(dir);
	//     return 1;
	// }
	// else{
	//     return 0;
	// }
    //folderr = "C:\\Users\\SaMaN\\Desktop\\Ppln";
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        return 0;
    }
    else {
        return -1;
    }
}

void ftpthread_parserealdir(char* pwd, char* input, char* ouput) {
	strcpy(ouput, servermain_root);
	if (input[0] == '/') {
		strcat(ouput, input);
	} else {
		strcpy(ouput, pwd);
		int len = strlen(pwd);
		if (pwd[len - 1] != '/') {
			strcat(ouput, "/");
		}
		strcat(ouput, input);
	}
}

void ftpthread_parseworkdir(char* pwd, char* input, char* ouput) {
	if (input[0] == '/') {
		strcpy(ouput, input);

	} else {
		strcpy(ouput, pwd);
		// int len = strlen(pwd);
		// if (pwd[len - 1] != '/') {
		// 	strcat(ouput, "/");
		// }
		if (strlen(pwd) > 1) {
			strcat(ouput, "/");
		}
		strcat(ouput, input);
	}

	int out_len = strlen(ouput);
	if (out_len > 1) {
		if (ouput[out_len - 1] == '/') {
			ouput[out_len - 1] = 0;
		}
	}
}


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
		}
		else if (strncmp(buffer, "PASV", 4) == 0) {
			ftpthread_setmodepasv(t_info);	
		} 
		else if (strncmp(buffer, "RETR", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			ftpthread_retr(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "STOR", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			ftpthread_stor(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "PWD", 3) == 0) {
			char resp[150];
			sprintf(resp, "%s \"%s\"\n", "257", t_info->pwd);
			bs_sendstr(t_info->controlfd, resp);
		}
		else if (strncmp(buffer, "CWD", 3) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				bs_sendstr(t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_cwd(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "CDUP", 4) == 0 ) {
			ftpthread_cdup(t_info);
		}
		else if ((strncmp(buffer, "QUIT", 4) == 0 ) || 
			     (strncmp(buffer, "ABOR", 4) == 0)) {
			ftpthread_close(t_info);
			break;
		}
		else {
			bs_sendstr(t_info->controlfd, "202 Command not implemented\n");
			continue;
		}
	}
	printf("Thread %d end\n", t_info->index);
	return 0;
}

int ftpthread_cdup(struct ftpthread_info* t_info) {
	if(strcmp(t_info->pwd, "/") == 0) {
		bs_sendstr(t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	int p = strlen(t_info->pwd) - 1;
	while (1) {
		if (p == 1 || t_info->pwd[p] == '/') {
			break;
		}
		--p;
	}
	// printf("P [%d]\n", p);
	// printf("OLD [%s]\n", t_info->pwd);
	char new_pwd[128];
	memset(new_pwd, 0, 128);
	if (p != 0) {
		strncpy(new_pwd, t_info->pwd, p);
	}
	strcpy(t_info->pwd, new_pwd);
	// printf("NEW [%s]\n", t_info->pwd);
	bs_sendstr(t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_cwd(struct ftpthread_info* t_info, char* dir) {
	char new_pwd[128];
	ftpthread_parseworkdir(t_info->pwd, dir, new_pwd);

	char real_dir[258];
	strcpy(real_dir, servermain_root);
	strcat(real_dir, new_pwd);
	if (ftpthread_exsistdir(real_dir) < 0) {
		bs_sendstr(t_info->controlfd, "550 No such directory\n");
		return -1;
	}
	bs_sendstr(t_info->controlfd, "250 Okay\n");
	strcpy(t_info->pwd, new_pwd);
	return 0;
}

void ftpthread_setmodeport(struct ftpthread_info* t_info, char* param) {
	
	printf("SET MODE PORT %s\n", param);
	if (t_info->mode == THREAD_MODE_PASV) {
		close(t_info->transferfd);
	}

	t_info->mode = THREAD_MODE_PORT;
	bs_parseipandport(param, t_info->ipv4, &(t_info->transferport));
	bs_sendstr(t_info->controlfd, "200 PORT command successful\n");
}

void ftpthread_setmodepasv(struct ftpthread_info* t_info) {
	printf("SET MODE PASV\n");
	if (t_info->mode == THREAD_MODE_PASV) {
		close(t_info->transferfd);
	}
	t_info->mode = THREAD_MODE_PASV;

	ftpcommon_openandlisten(&(t_info->transferfd), &(t_info->transferport));
	memcpy(t_info->ipv4, servermain_ipv4, 4);

	char resp[40];
	sprintf(resp, "227 =%d,%d,%d,%d,%d,%d\n", servermain_ipv4[0], servermain_ipv4[1],
		servermain_ipv4[2], servermain_ipv4[3], 
		t_info->transferport / 256, t_info->transferport % 256);
	bs_sendstr(t_info->controlfd, resp);
}

void ftpthread_init(struct ftpthread_info * t_info) {
	//Set mode
	t_info->mode = 0;
	t_info->transferfd = 0;
	strcpy(t_info->pwd, "/");
}

int ftpthread_retr(struct ftpthread_info* t_info, char* fname) {
	//Check name
	// if ( strstr(fname, "..") ||
	// 	(strlen(fname) == 0)) {
	// 	bs_sendstr(t_info->controlfd, "550 Permission denied\n");
	// 	t_info->mode = THREAD_MODE_NON;
	// 	return 0;
	// }
	if (ftpthread_check(fname) < 0) {
		bs_sendstr(t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	char comname[1024];
	ftpthread_parserealdir(t_info->pwd, fname, comname);

 	if (t_info->mode == THREAD_MODE_NON) {
		bs_sendstr(t_info->controlfd, "550 Mode not set\n");
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PORT) {
		ftpthread_portretr(t_info, comname);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PASV) {
		ftpthread_pasvretr(t_info, comname);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}

	return 0;
}

int ftpthread_portretr(struct ftpthread_info* t_info, char* fname) {

	if (ftpcommon_connectandgetsock(&(t_info->transferfd), t_info->ipv4, 
	t_info->transferport) < 0) {
		bs_sendstr(t_info->controlfd, "425 Connection failed\n");
		return -1;
	}

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "r");
    if(!fp) {
    	printf("PORT RETR File Not Found\n");
		bs_sendstr(t_info->controlfd, "451 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }
    bs_sendstr(t_info->controlfd, "150 Open\n");
    int status = bs_sendfile(t_info->transferfd, fp);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		bs_sendstr(t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		bs_sendstr(t_info->controlfd, "226 File sent\n");
	}
	return 0;
}

int ftpthread_pasvretr(struct ftpthread_info* t_info, char* fname) {
		//Here we have connect to the client	
    FILE* fp = fopen(fname, "r");
    if(!fp) {
    	printf("PASV RETR File Not Opened\n");
		bs_sendstr(t_info->controlfd, "450 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }

    bs_sendstr(t_info->controlfd, "150 Open\n");

    int newfd;
    if ((newfd = accept(t_info->transferfd, NULL, NULL)) == -1) {
		bs_sendstr(t_info->controlfd, "425 Connection failed\n");
		close(t_info->transferfd);
		return -1;
	}

	int status = bs_sendfile(newfd, fp);
	close(newfd);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		bs_sendstr(t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		bs_sendstr(t_info->controlfd, "226 File sent\n");
	}

	return 0;
}

int ftpthread_stor(struct ftpthread_info* t_info, char* fname) {
	//Check name
	if (ftpthread_check(fname) < 0) {
		bs_sendstr(t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	char comname[1024];
	// strcpy(comname, servermain_root);
	// strcat(comname, "/");
	// strcat(comname, fname);
	ftpthread_parserealdir(t_info->pwd, fname, comname);

 	if (t_info->mode == THREAD_MODE_NON) {
		bs_sendstr(t_info->controlfd, "550 Mode not set\n");
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PORT) {
		ftpthread_portstor(t_info, comname);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PASV) {
		ftpthread_pasvstor(t_info, comname);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}

	return 0;
}

int ftpthread_portstor(struct ftpthread_info* t_info, char* fname) {

	if (ftpcommon_connectandgetsock(&(t_info->transferfd), t_info->ipv4, 
	t_info->transferport) < 0) {
		bs_sendstr(t_info->controlfd, "425 Connection failed\n");
		return -1;
	}

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "w");
    if(!fp) {
    	printf("PORT STOR File Not Found\n");
		bs_sendstr(t_info->controlfd, "451 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }
    bs_sendstr(t_info->controlfd, "150 Open\n");
    int status = bs_recvfile(t_info->transferfd, fp);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		bs_sendstr(t_info->controlfd, "451 File write error\n");
	} else if (status == 0) {
		bs_sendstr(t_info->controlfd, "226 File recieved\n");
	}
	
	return 0;
}

int ftpthread_pasvstor(struct ftpthread_info* t_info, char* fname) {

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "w");
    if(!fp) {
    	printf("PORT Sort File Not Opened\n");
		bs_sendstr(t_info->controlfd, "450 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }

    bs_sendstr(t_info->controlfd, "150 Open\n");

    int newfd;
    if ((newfd = accept(t_info->transferfd, NULL, NULL)) == -1) {
		bs_sendstr(t_info->controlfd, "425 Connection failed\n");
		close(t_info->transferfd);
		return -1;
	}

	int status = bs_recvfile(newfd, fp);
	close(newfd);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		bs_sendstr(t_info->controlfd, "451 File Write error\n");
	} else if (status == 0) {
		bs_sendstr(t_info->controlfd, "226 File recieved\n");
	}

	return 0;
}

int ftpthread_close(struct ftpthread_info* t_info) {
	bs_sendstr(t_info->controlfd, "221 Goodbye\n");
	close(t_info->controlfd);
	t_info->isset = 0;
	t_info->mode = THREAD_MODE_NON;
	return 0;
}