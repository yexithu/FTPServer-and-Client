#include "defs.h"

int random_port() {
	return rand() % (65535 - 20000) + 20000;
}

int ftpthread_sendstr(struct ftpthread_info* t_info, int fd,  char* resp) {
	int len = bs_sendstr(fd, resp);
	if (len >= 0) {
		t_info->download_traffic += len;
		return 0;
	} else {
		return -1;
	}
}

int ftpthread_readline(struct ftpthread_info* t_info, int fd, 
						char* buffer, int len) {
	int rlen = bs_readline(fd, buffer, len);
	if (rlen >= 0) {
		t_info->upload_traffic += rlen;
		return 0;
	} else {
		return -1;
	}
}

int ftpthread_sendfile(struct ftpthread_info* t_info, int fd,
						 FILE* fp) {
	int slen = bs_sendfile(fd, fp);
	if (slen >= 0) {
		t_info->download_traffic += slen;
		t_info->download_filecount += 1;
		t_info->download_filebytes += slen;
		return 0;
	} else {
		return -1;
	}
}

int ftpthread_recvfile(struct ftpthread_info* t_info, int fd,
						 FILE* fp) {
	int rlen = bs_recvfile(fd, fp);
	if (rlen >= 0) {
		t_info->upload_traffic += rlen;
		t_info->upload_filecount += 1;
		t_info->upload_filebytes += rlen;
		return 0;
	} else {
		return -1;
	}
}

int ftpthread_check(char *path) {
	if ( strstr(path, "..") ||
		(strlen(path) == 0)) {
		return -1;
	}
	return 0;
}

int ftpthread_exsistfile(char *file) {
	if( access( file, F_OK) != -1 ) {
    	return 0;
	} else {
    	return -1;
	}
}

int ftpthread_exsistdir(char *path) {
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
	char temp[128];
	ftpthread_parseworkdir(pwd, input, temp);
	strcat(ouput, temp);
}

void ftpthread_parseworkdir(char* pwd, char* input, char* ouput) {
	if (input[0] == '/') {
		strcpy(ouput, input);

	} else {
		strcpy(ouput, pwd);
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

int ftpthread_verifyuserpwd(char* user, char* pwd) {
	if (strcmp(user, "anonymous") == 0) {
		return 0;
	}
	struct user_listnode *p, *q;
    p = q = NULL;
    p = usertable;
    while (p != NULL) {
    	q = p->next;
    	// free(p);
    	if (strcmp(user, p->username) == 0 &&
    	   strcmp(pwd, p->password) == 0) {
    		return 0;
    	}
    	p = q;
    }
    return -1;
}

int ftp_authentication(struct ftpthread_info * t_info) {
	// authentication;
	char buffer[4096];
	int len = 4096;
	char verb[10];
	char parameters[4][1024];
	int paramlen = 1024;
	int argc;
	
	int pass_status = 0;
	char uname[20];
	char pwd[20];
	while (1) {
		if (ftpthread_readline(t_info, t_info->controlfd, buffer, len) < 0) {
			return -1;
		}
#ifdef LOG_ON		
		printf("[REQU] %s\n", buffer);
#endif
		if (pass_status > 0) {
			pass_status -= 1;
		}
		if (strncmp(buffer, "USER", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			} else {
				ftpthread_sendstr(t_info, t_info->controlfd, "331 Send your password\n");
				strcpy(uname, parameters[0]);
				pass_status = 2;
			}
		}
		else if (strncmp(buffer, "PASS", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			} else if (pass_status == 0) {
				ftpthread_sendstr(t_info, t_info->controlfd, "530 Pass should follow USER\n");
			} else {
				strcpy(pwd, parameters[0]);
				if (ftpthread_verifyuserpwd(uname, pwd) == 0) {
					ftpthread_sendstr(t_info, t_info->controlfd, 
						  "230-\n230-Welcome to FTP\n230 Guest login ok\n");
					return 0;
				} else {
					ftpthread_sendstr(t_info, t_info->controlfd, "530 Invalid user info\n");
				}
			}
		}
		else if (strcmp(buffer, "SYST") == 0) {
			ftpthread_sendstr(t_info, t_info->controlfd, "215 UNIX Type: L8\n");
		} 
		else if (strncmp(buffer, "QUIT", 4) == 0) {
			ftpthread_close(t_info);
			return -1;
		}
		else {
			ftpthread_sendstr(t_info, t_info->controlfd, "530 Please login first\n");
		}
	}
	return 0;
}

void *ftpthread_main(void * args) {
	struct ftpthread_info * t_info = (struct ftpthread_info *) args;
#ifdef LOG_ON	
	printf("Thread %d start\n", t_info->index);
#endif
	ftpthread_init(t_info);

	char buffer[4096];
	int len = 4096;
	char verb[10];
	char parameters[4][1024];
	int paramlen = 1024;
	int argc;
	
	// authentication;
	if (ftp_authentication(t_info) < 0) {
		ftpthread_close(t_info);
#ifdef LOG_ON		
		printf("Thread %d end\n", t_info->index);
#endif
		return 0;
	}

	while (1) {
		if (ftpthread_readline(t_info, t_info->controlfd, buffer, len) < 0) {
			//teminate
			break;
		}
		if (t_info->rnfrset > 0) {
			t_info->rnfrset -= 1;
		}
#ifdef LOG_ON		
		printf("[REQU] %s\n", buffer);
#endif		
		if (strcmp(buffer, "SYST") == 0) {
			ftpthread_sendstr(t_info, t_info->controlfd, "215 UNIX Type: L8\n");
		} 
		else if (strcmp(buffer, "TYPE I") == 0) {
			ftpthread_sendstr(t_info, t_info->controlfd, "200 Type set to I.\n");
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
			ftpthread_sendstr(t_info, t_info->controlfd, resp);
		}
		else if (strncmp(buffer, "CWD", 3) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_cwd(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "CDUP", 4) == 0 ) {
			ftpthread_cdup(t_info);
		}
		else if (strncmp(buffer, "MKD", 3) == 0 ) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_mkd(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "RMD", 3) == 0 ) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_rmd(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "DELE", 4) == 0 ) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_dele(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "RNFR", 4) == 0 ) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_rnfr(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "RNTO", 4) == 0 ) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc < 1) {
				ftpthread_sendstr(t_info, t_info->controlfd, "500 Synatic error\n");
			}
			ftpthread_rnto(t_info, parameters[0]);
		}
		else if (strncmp(buffer, "LIST", 4) == 0) {
			bs_parserequest(buffer, verb, (char *) parameters, paramlen, &argc);
			if (argc == 0) {
				ftpthread_list(t_info, NULL);
			} else {
				ftpthread_list(t_info, parameters[0]);
			}
		}
		else if ((strncmp(buffer, "QUIT", 4) == 0 ) || 
			     (strncmp(buffer, "ABOR", 4) == 0)) {
			char goodbye[1024];
			sprintf(goodbye,
				"221-You have upload %d bytes in %d files\n"
				"221-You have download %d bytes in %d files(including LIST)\n"
				"221-Total traffic for this session was %d bytes in %d transfer\n"
				"221-Sesion upload traffic %d bytes. Session download trafic %d bytes\n"
				"221-Thank you for using FTP\n"
				"221 Goodbye\n",
				t_info->upload_filebytes, t_info->upload_filecount,
				t_info->download_filebytes, t_info->download_filecount,
				t_info->download_traffic + t_info->upload_traffic,
				t_info->download_filecount + t_info->upload_filecount,
				t_info->upload_traffic, t_info->download_traffic);
			ftpthread_sendstr(t_info, t_info->controlfd, goodbye);
			ftpthread_close(t_info);
			break;
		}
		else {
			ftpthread_sendstr(t_info, t_info->controlfd, "202 Command not implemented\n");
			continue;
		}
	}
#ifdef LOG_ON
	printf("Thread %d end\n", t_info->index);
#endif	
	return 0;
}

int ftpthread_list(struct ftpthread_info* t_info, char* dir) {
	char real_dir[256];
	if (dir == NULL) {
		strcpy(real_dir, servermain_root);
		strcat(real_dir, t_info->pwd);
	} else {
		ftpthread_parserealdir(t_info->pwd, dir, real_dir);
	}
	if (ftpthread_exsistfile(real_dir) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Dir not exsist\n");
		t_info->mode = THREAD_MODE_NON;
		return -1;
	}

	if (t_info->mode == THREAD_MODE_NON) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Mode not set\n");
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PORT) {
		ftpthread_portlist(t_info, real_dir);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	if (t_info->mode == THREAD_MODE_PASV) {
		ftpthread_pasvlist(t_info, real_dir);
		t_info->mode = THREAD_MODE_NON;
		return 0;
	}
	return 0;
}

int ftpthread_portlist(struct ftpthread_info* t_info, char* real_dir) {
	if (ftpcommon_connectandgetsock(&(t_info->transferfd), t_info->ipv4, 
	t_info->transferport) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		return -1;
	}

	//get list
	char ls_cmd[1024];
    sprintf(ls_cmd, "ls %s -p", real_dir);
    FILE* fp = popen(ls_cmd, "r");
    if (!fp) {
        ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
        return -1;
    }
    ftpthread_sendstr(t_info, t_info->controlfd, 
    	"150 Opening BINARY mode data connection\n");
    int status = ftpthread_sendfile(t_info, t_info->transferfd, fp);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}
	return 0;
}

int ftpthread_pasvlist(struct ftpthread_info* t_info, char* real_dir) {
	//get list
	char ls_cmd[1024];
    sprintf(ls_cmd, "ls %s -p", real_dir);
    FILE* fp = popen(ls_cmd, "r");
    if(!fp) {
        ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		close(t_info->transferfd);
		return -1;
    }

    ftpthread_sendstr(t_info, t_info->controlfd, 
    	"150 Opening BINARY mode data connection\n");

    int newfd;
    if ((newfd = accept(t_info->transferfd, NULL, NULL)) == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		close(t_info->transferfd);
		return -1;
	}
	int status = ftpthread_sendfile(t_info, newfd, fp);
	close(newfd);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}

	return 0;
}

int ftpthread_dele(struct ftpthread_info* t_info, char* name) {
	char comname[256];
	ftpthread_parserealdir(t_info->pwd, name, comname);
	if (remove(comname) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_rnfr(struct ftpthread_info* t_info, char* name) {
	ftpthread_parserealdir(t_info->pwd, name, t_info->rnfrname);
	if (ftpthread_exsistfile(t_info->rnfrname) < 0) {
		t_info->rnfrset = 0;
		ftpthread_sendstr(t_info, t_info->controlfd, "550 File not exsist\n");
		return -1;
	}
	t_info->rnfrset = 2;
	ftpthread_sendstr(t_info, t_info->controlfd, "350 Okey\n");
	return 0;
}

int ftpthread_rnto(struct ftpthread_info* t_info, char* name) {
	if (t_info->rnfrset == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "503 RNTO should follow RNFT\n");
		return -1;
	}
	t_info->rnfrset = 0;
	ftpthread_parserealdir(t_info->pwd, name, t_info->rntoname);
	if (rename(t_info->rnfrname, t_info->rntoname) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_mkd(struct ftpthread_info* t_info, char* dir) {
	char comname[256];
	ftpthread_parserealdir(t_info->pwd, dir, comname);
	if (mkdir(comname, 0777) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_rmd(struct ftpthread_info* t_info, char* dir) {
	char comname[256];
	ftpthread_parserealdir(t_info->pwd, dir, comname);
	if (rmdir(comname) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_cdup(struct ftpthread_info* t_info) {
	if(strcmp(t_info->pwd, "/") == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	int p = strlen(t_info->pwd) - 1;
	while (1) {
		if (p == 1 || t_info->pwd[p] == '/') {
			break;
		}
		--p;
	}
	char new_pwd[128];
	memset(new_pwd, 0, 128);
	if (p != 0) {
		strncpy(new_pwd, t_info->pwd, p);
	}
	strcpy(t_info->pwd, new_pwd);
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	return 0;
}

int ftpthread_cwd(struct ftpthread_info* t_info, char* dir) {
	char new_pwd[128];
	ftpthread_parseworkdir(t_info->pwd, dir, new_pwd);

	char real_dir[258];
	strcpy(real_dir, servermain_root);
	strcat(real_dir, new_pwd);
	if (ftpthread_exsistdir(real_dir) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 No such directory\n");
		return -1;
	}
	ftpthread_sendstr(t_info, t_info->controlfd, "250 Okay\n");
	strcpy(t_info->pwd, new_pwd);
	return 0;
}

void ftpthread_setmodeport(struct ftpthread_info* t_info, char* param) {
	
	if (t_info->mode == THREAD_MODE_PASV) {
		close(t_info->transferfd);
	}

	t_info->mode = THREAD_MODE_PORT;
	bs_parseipandport(param, t_info->ipv4, &(t_info->transferport));
	ftpthread_sendstr(t_info, t_info->controlfd, "200 PORT command successful\n");
}

void ftpthread_setmodepasv(struct ftpthread_info* t_info) {
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
	ftpthread_sendstr(t_info, t_info->controlfd, resp);
}

void ftpthread_init(struct ftpthread_info * t_info) {
	//Set mode
	t_info->mode = 0;
	t_info->transferfd = 0;
	t_info->rnfrset = 0;
	strcpy(t_info->pwd, "/");

	t_info->upload_filecount = 0;
	t_info->upload_filebytes = 0;
	t_info->download_filecount = 0;
	t_info->download_filebytes = 0;
	t_info->upload_traffic = 0;
	t_info->download_traffic = 0;
}

int ftpthread_retr(struct ftpthread_info* t_info, char* fname) {
	if (ftpthread_check(fname) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	char comname[256];
	ftpthread_parserealdir(t_info->pwd, fname, comname);
 	if (t_info->mode == THREAD_MODE_NON) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Mode not set\n");
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
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		return -1;
	}

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "r");
    if(!fp) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }
    ftpthread_sendstr(t_info, t_info->controlfd, 
    	"150 Opening BINARY mode data connection\n");
    int status = ftpthread_sendfile(t_info, t_info->transferfd, fp);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}
	return 0;
}

int ftpthread_pasvretr(struct ftpthread_info* t_info, char* fname) {
		//Here we have connect to the client	
    FILE* fp = fopen(fname, "r");
    if(!fp) {
		ftpthread_sendstr(t_info, t_info->controlfd, "450 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }

    ftpthread_sendstr(t_info, t_info->controlfd,
     "150 Opening BINARY mode data connection\n");

    int newfd;
    if ((newfd = accept(t_info->transferfd, NULL, NULL)) == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		close(t_info->transferfd);
		return -1;
	}

	int status = ftpthread_sendfile(t_info, newfd, fp);
	close(newfd);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File read error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}

	return 0;
}

int ftpthread_stor(struct ftpthread_info* t_info, char* fname) {
	//Check name
	if (ftpthread_check(fname) < 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Permission denied\n");
		return -1;
	}
	char comname[256];
	// strcpy(comname, servermain_root);
	// strcat(comname, "/");
	// strcat(comname, fname);
	ftpthread_parserealdir(t_info->pwd, fname, comname);
 	if (t_info->mode == THREAD_MODE_NON) {
		ftpthread_sendstr(t_info, t_info->controlfd, "550 Mode not set\n");
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
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		return -1;
	}

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "w");
    if(!fp) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }
    ftpthread_sendstr(t_info, t_info->controlfd, 
    	"150 Opening BINARY mode data connection\n");
    int status = ftpthread_recvfile(t_info, t_info->transferfd, fp);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File write error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}
	
	return 0;
}

int ftpthread_pasvstor(struct ftpthread_info* t_info, char* fname) {

	//Here we have connect to the client	
    FILE* fp = fopen(fname, "w");
    if(!fp) {
		ftpthread_sendstr(t_info, t_info->controlfd, "450 File not opened\n");
		close(t_info->transferfd);
		return -1;
    }

    ftpthread_sendstr(t_info, t_info->controlfd, 
    	"150 Opening BINARY mode data connection\n");

    int newfd;
    if ((newfd = accept(t_info->transferfd, NULL, NULL)) == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "425 Connection failed\n");
		close(t_info->transferfd);
		return -1;
	}

	int status = ftpthread_recvfile(t_info, newfd, fp);
	close(newfd);
    close(t_info->transferfd);
	fclose(fp);
	if (status == -1) {
		ftpthread_sendstr(t_info, t_info->controlfd, "451 File Write error\n");
	} else if (status == 0) {
		ftpthread_sendstr(t_info, t_info->controlfd, "226 Transfer complete\n");
	}

	return 0;
}

int ftpthread_close(struct ftpthread_info* t_info) {
	close(t_info->controlfd);
	t_info->isset = 0;
	t_info->mode = THREAD_MODE_NON;
	return 0;
}