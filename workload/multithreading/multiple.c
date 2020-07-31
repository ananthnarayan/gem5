#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include "gem5/m5ops.h"
// #include "gem5/asm/generic/m5ops.h"

struct trd
{
	int tid;
	long long num;
	long long res;
};
typedef struct trd trd;

void* thread_function(void* arg)
{
	trd *struct_ptr = (trd*)arg;
	printf("Inside thread %d\n", struct_ptr->tid);
	long long sum = 0;
	for(long long i=1;i<=struct_ptr->num;++i) {
		sum = sum+i;
	}
	struct_ptr->res = sum;
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	// m5_pim_process(1);
	// m5_host_process();
	// int nargs = atoi(argv[0]);
	int nargs = argc - 1;

	pthread_t threads[nargs];

	trd arg[nargs];

	for(int i=0;i<nargs;++i) {
		arg[i].tid = i;
		arg[i].num = atoll(argv[i+1]);
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&threads[i],&attr,thread_function,&arg[i]);
	}

	for(int i=0;i<nargs;++i) {
		pthread_join(threads[i], NULL);
		printf("Result from thread %d is: %lld\n",arg[i].tid,arg[i].res);
	}

	return 0;
}