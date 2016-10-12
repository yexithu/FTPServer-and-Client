#include "defs.h"

void *ftpthread_main(void * args) {
	struct ftpthread_info * t_info = (struct ftpthread_info *) args;
	printf("Thread %d start\n", t_info->index);

	int count = 0;
	while (1) {
		sleep(1); // sleep 1 sec
		printf("Thread %d count %d\n",t_info->index, count);
		++count;
	}
}