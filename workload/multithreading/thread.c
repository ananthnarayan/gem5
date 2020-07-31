#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


/*Thread function to find the factorial of n*/
void *thread_function(void* arg)
{
	long long *num_ptr = (long long*)arg;
	long long num = *num_ptr;
	long long sum = 1;

	printf("Thread! %lld\n",num);

	for(long long i = 1;i<=num;++i) {
		sum = sum+i;
	}

	printf("Thread!! %lld\n",sum);
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	long long n = 100;
	long long n1 = 10;
	long long n2 = 20;

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_create(&tid, &attr, thread_function, &n);

	long long temp = 0;
	for(long long j = 1;j<=10000;++j) {
		temp = temp+j;
	}

	printf("HELLO!!\n");
	printf("result is: %lld\n",temp);

	pthread_t tid1;
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);

	pthread_create(&tid1, &attr1, thread_function, &n1);

	printf("HELLO1\n");

	pthread_t tid2;
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);

	pthread_create(&tid2, &attr2, thread_function, &n2);
	printf("HELLO2\n");

	pthread_join(tid, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	
	return 0;
}