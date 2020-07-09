
/*************************** HEADER FILES ***************************/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "sha256.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

void print_hash(BYTE hashes[])
{
	int i = 0;
	for(int i=0;i<SHA256_BLOCK_SIZE;i++)
		printf("%x",*(hashes+i));
	printf("\n");

}
int main(int argc, char** argv)
{
	char filename[BUFSIZ];
	BYTE* text;
	SHA256_CTX ctx;
	unsigned int text_length;
	BYTE hashes[SHA256_BLOCK_SIZE];
	struct stat input_file_stat;
	int i;
	int block_size = 0;
	FILE *file;

	if(argc < 2)
	{
		fprintf(stderr, "Insufficient parameters.\n");
		exit(1);
	}

	block_size = strtol(argv[2], NULL, 0);
	if(block_size > (128 * 1024 * 1024))
	{
		block_size = 128 * 1024 * 1024;
	}
	memset(hashes, 0, sizeof(BYTE) *  SHA256_BLOCK_SIZE); 
	memset((void*)filename, 0, sizeof(char) * BUFSIZ);
	strncpy(filename, argv[1], BUFSIZ);
	if(stat(filename, &input_file_stat) != 0)
	{
		perror("Stat failed:");
		exit(1);
	}
	text = (BYTE*) malloc(sizeof(BYTE) * block_size + 1);
	if(text == NULL)
	{
		perror("Unable to allocate memory");
		exit(1);
	}
	file = fopen(filename, "r");
	if(file == NULL)
	{
		perror("Error reading file");
		exit(1);
	}
	while(!feof(file))
	{
		size_t bytes_read;
		memset(text, 0, sizeof(BYTE) * block_size);
		bytes_read = fread((void*)text, sizeof(BYTE), (size_t)block_size,file);
		text[bytes_read] = '\0';
		fprintf(stdout, "%d\t", bytes_read);
		sha256_init(&ctx);
		sha256_update(&ctx, text, bytes_read);
		sha256_final(&ctx, hashes);
		print_hash(text);
	}
	free(text);
	fclose(file);
	return(0);
}
