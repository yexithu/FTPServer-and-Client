#include "defs.h"

int sendstr(int fd, char* resp) {
	int len = strlen(resp);
	/**sentence = htonl(*sentence);*/
	int p = 0;
	while (p < len) {
		int n = send(fd, resp + p, len - p, 0);
		printf("Sent Byte %d\n", n);
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			return -1;
 		} else {
			p += n;
		}
	}
	return 0;
}