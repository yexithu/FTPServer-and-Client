#include "defs.h"

int bs_sendstr(int fd, char* str) {
	int len = strlen(str);
	/**sentence = htonl(*sentence);*/
	if (bs_sendbytes(fd, str, len) < 0) {
		return -1;
	}
#ifdef LOG_ON	
	printf("[RESP] %s\n", str);
#endif
	return len;
}

int bs_sendbytes(int fd, char* info, int len) {
	int p = 0;
	while (p < len) {
		int n = send(fd, info + p, len - p, 0);
		if (n < 0) {
			return -1;
 		} if (n == 0) {
 			break;
 		}else {
			p += n;
		}
	}
	return p;
}

int bs_readline(int fd, char* buffer, int len) {
	int p = 0;
	memset(buffer, 0, len);
	while (1) {
		int n = recv(fd, buffer + p, len - p, 0);
		if (n < 0) {
			return -1;
		} else if (n == 0) {
			return -1;
		} else {
			p += n;
			if (buffer[p - 1] == '\n') {
				if(buffer[p - 2] == '\r') {
					buffer[p - 2] = '\0';
				}
				buffer[p - 1] = '\0';
				break;
			}
		}
	}
	return p;
}

int bs_readbytes(int fd, char* buffer, int len) {
	int p = 0;
	memset(buffer, 0, len);
	while (1) {
		int n = recv(fd, buffer + p, len - p, 0);
		if (n < 0) {
			return -1;
		} else if (n == 0) {
			break;
		} else {
			p += n;
			if (p == len) {
				break;
			}
		}
	}
	return p;
}

int bs_parseipandport(char* param, unsigned char* ipv4, unsigned short int *port) {
	char num[4];
	int j = 0, index = 0, len = strlen(param);
	unsigned char nums[6];
	for (int i = 0; i < len; ++i) {
		if (param[i] == ',') {
			nums[index] = atoi(num);
			++index;
			memset(num, 0, 4);
			j = 0;
			continue;
		}
		num[j] = param[i];
		++j;
	}
	nums[index] = atoi(num);	
	for (int i = 0; i < 4; ++i) {
		ipv4[i] = nums[i];
	}
	*port = nums[4];
	*port = ((*port) << 8) + nums[5];

	return 0;
}

int bs_parserequest(char* sentence, char* verb, 
					char* parameters, int paramlen, int *argc) {
	//Sentence not contrain \n
	int len = strlen(sentence);
	*argc = 0;
	int i = 0, j = 0;
	for (; i < len; ++i, ++j) {
		if (sentence[i] == ' ') {
			verb[j] = '\0';
			break;
		}
		verb[j] = sentence[i];
	}

	if (i == len) {
		verb[j] = '\0';
		return 0;
	}
	while (1) {
		j = 0;
		++i;
		char *head = parameters + (*argc) * paramlen;
		for (; i < len; ++i, ++j) {
			if (sentence[i] == ' ') {
				head[j] = '\0';
				break;
			}
			head[j] = sentence[i];
		}
		*argc = (*argc + 1);
		if (i == len) {
			head[j] = '\0';
			break;
		}
	}

	return 0;
}

int bs_sendfile(int fd, FILE* fp) {
	char fbuffer[TRANS_BUF_SIZE];
	int fb_len = TRANS_BUF_SIZE;
    int status = 0;
	while (1) {
		memset(fbuffer, 0, fb_len);
		int len = fread(fbuffer, sizeof (char), fb_len, fp);
		status += len;
		bs_sendbytes(fd, fbuffer, len);
		if (len < fb_len) {
			if (ferror(fp)) {
				status = -1;
				break;
			} else if (feof(fp)) {
				// status = 0;
				break;
			}
		}
	}
	return status;
}

int bs_recvfile(int fd, FILE* fp) {
	char fbuffer[TRANS_BUF_SIZE];
	int fb_len = TRANS_BUF_SIZE;
    int status = 0;
	while (1) {
		memset(fbuffer, 0, fb_len);
		int len = bs_readbytes(fd, fbuffer, fb_len);
		status += len;
		if (len < fb_len) {
			if (len < 0) {
				status = -1;
				break;
			} else {
				if(fwrite(fbuffer, sizeof (char), len, fp) < len) {
					status = -1;
					break;
				}
				// status = 0;
				break;
			}
		}
		fwrite(fbuffer, sizeof (char), len, fp);
	}
	return status;
}

int bs_ipv4tostr(unsigned char* ipv4, char* str) {
	return 0;
}