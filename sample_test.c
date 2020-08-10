#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "gem5/m5ops.h"
#include "gem5/asm/generic/m5ops.h"


int main(int argc, char **argv)
{	
	FILE* file = NULL;
	char* text;
	size_t bytes_read;
	size_t size = 600;
	text = (char*)malloc(sizeof(char)*(size+1));
	printf("OPENING THE FILE\n\n");
	file = fopen("./input_gen.txt", "r");
	printf("OPENED THE FILE\n");
	bytes_read = fread((void*)text, sizeof(char), (size_t)size, file);
	printf("%ld\n",bytes_read);
	text[bytes_read-1] = '\0';
	int k = 0;
	uint64_t i = 0;
	printf("starting to print\n");
	m5_cpu_print();
	//printf("Tranfering control to PIM\n");
	m5_pim_process(0);
	m5_cpu_print();
	while(text[i]!='\0')
	{
		printf("%c",text[i]);
		i++;
	}
	printf("Done PRINTING STUFF\n");
	printf("\n\n\n\n");
	//printf("Tranfering control to host\n");
	m5_host_process();
	m5_cpu_print();
	//free(text);
	return 0;
}
