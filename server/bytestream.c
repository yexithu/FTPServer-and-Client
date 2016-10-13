#include "defs.h"

int bs_sendstr(int fd, char* str) {
	int len = strlen(str);
	/**sentence = htonl(*sentence);*/
	if (bs_sendbytes(fd, str, len) < 0) {
		return -1;
	}
	printf("Sent Str %s", str);
	return 0;
}

int bs_sendbytes(int fd, char* info, int len) {
	int p = 0;
	while (p < len) {
		int n = send(fd, info + p, len - p, 0);
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			return -1;
 		} else {
			p += n;
		}
	}
	return 0;
}

int bs_readline(int fd, char* buffer, int len) {
	int p = 0;
	memset(buffer, 0, len);
	while (1) {
		int n = read(fd, buffer + p, len - p);
		if (n < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			return -1;
		} else if (n == 0) {
			continue;
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
	return 0;
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
	// printf("IP: [%d.%d.%d.%d]\n", nums[0], nums[1], nums[2], nums[3]);
	// printf("Port: %d %d [%d]\n", nums[4], nums[5], (*port));

	return 0;
}

int bs_parserequest(char* sentence, char* verb, 
					char* parameters, int paramlen, int *argc) {
	//Sentence not contrain \n
	printf("Parse Result\n");
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

	// printf("Verb [%s]\n", verb);
	// printf("Argc %d\n", (*argc));
	// for (int i = 0; i < (*argc); ++i) {
	// 	char *head = parameters + i * paramlen;
	// 	printf("Arg: [%s]\n", head);
	// }
	return 0;
}

int bs_writefile(int fd, char* fname) {
	return 0;
}

int bs_ipv4tostr(unsigned char* ipv4, char* str) {
	return 0;
}