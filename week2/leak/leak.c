#include <stdlib.h>
// to have src line numbers listed in .log file
// it requires -ldmalloc option specified for gcc
#include <dmalloc.h> 

void leak(unsigned int n) {
	// to use "malloc" from  dmalloc lib, set LD_PRELOAD env var to "libdmalloc.so"
	malloc(n);
}

 int main(int argc, char **argv) {
	leak(1);
	leak(2);
	leak(3);

	void *m = malloc(1024);
	free(m);

	return 0;
}
