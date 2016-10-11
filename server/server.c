#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
	printf("Hello Server\n");

	char root[128] = "/tmp";
	int port = 21;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-port") == 0) {
			++i;
			port = atoi(argv[i]);
		} else if (strcmp(argv[i], "-root") == 0) {
			++i;
			strcpy(root, argv[i]);
		} else {
			printf("Invalid arguements %s\n", argv[i]);
			exit(-1);
		}
	}
	printf("port %d\n", port);
	printf("root %s\n", root);
	return 0;
}
