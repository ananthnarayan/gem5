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
#include <math.h>

void print_hash(BYTE* hashes)
{
	int i = 0;
	for(int i=0;i<SHA256_BLOCK_SIZE;i++)
		printf("%02x",*(hashes+i));
	printf("\n");

}

void as_string(BYTE* hash)
{
	int i = 0;
	char hash_string[64];
	for(int i = 0; i < SHA256_BLOCK_SIZE; i++)
		sprintf((hash_string + i), "%02x", *(hash + i));
	printf("%s\n", hash_string);
	return ;
	
}
int main(int argc, char** argv)
{
/*	SHA256_CTX ctx;
	BYTE hashes[SHA256_BLOCK_SIZE];
	int block_size;
	int INPUT_SIZE = 32; // 1500000;
	int text_length;
	BYTE* text = (BYTE*)malloc(sizeof(BYTE)*(INPUT_SIZE+1));
	FILE *input_file;
	input_file = fopen(argv[1],"r");
	if(input_file == NULL) 
	text[INPUT_SIZE]='\0';
	//text_length = fread(text, INPUT_SIZE, 1, input_file);
	text_length = fread(text, sizeof(BYTE), INPUT_SIZE, input_file);
	fprintf(stdout, "Read %d\n", text_length);
	text[text_length]='\0';

	sha256_init(&ctx);
	sha256_update(&ctx, text, strlen(text));
	sha256_final(&ctx, hashes);
	
	for(int i=0;i<32;i++)
	  printf("%x",*(hashes+i));
	printf("\n");
	
	fclose(input_file);
	*/
	char filename[BUFSIZ];
	BYTE* text;
	SHA256_CTX ctx;
	unsigned int text_length;
	BYTE **hashes;//[SHA256_BLOCK_SIZE];
	int i = 0;
	unsigned int block_size = 0;
	unsigned int file_size = 0;
	unsigned int num_blocks = 0;
	FILE *file = NULL;

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
	memset((void*)filename, 0, sizeof(char) * BUFSIZ);
	strncpy(filename, argv[1], BUFSIZ);
	block_size = strtol(argv[2], NULL, 0);
	struct stat statbuf;
	memset(&statbuf, 0, sizeof(struct stat));
	if(stat(filename, &statbuf) != 0)
	{
		perror("Unable to obtain file size");
		exit(1);
	}
	file_size = statbuf.st_size;
	num_blocks = (unsigned int) ceil((file_size * 1.0)/block_size);
	hashes = (BYTE**) calloc( (size_t) num_blocks, (size_t)sizeof(BYTE*));
	memset((void*) hashes, 0, sizeof(BYTE*) * (size_t)num_blocks);
	for(int j = 0; j < num_blocks; j++)
	{
		hashes[j] = (BYTE*) malloc(sizeof(BYTE) * SHA256_BLOCK_SIZE);
		memset((void*)hashes[j], 0, sizeof(BYTE) * SHA256_BLOCK_SIZE);
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
		i = 0;
		memset(text, 0, sizeof(BYTE) * block_size);
		text[block_size] = '\0';
		bytes_read = fread((void*)text, sizeof(BYTE), (size_t)block_size, file);
		text[bytes_read] = '\0';
		fprintf(stdout, "%d\t", bytes_read);
		sha256_init(&ctx);
		sha256_update(&ctx, text, bytes_read);
		sha256_final(&ctx, hashes[i]);
		print_hash(hashes[i]);
		i++;
		as_string(hashes[i]);
	}
	free(text);
	for(int j = 0; j < num_blocks; j++)
		free(hashes[j]);
	free(hashes);
	fclose(file);
	return(0);
	//
}


/*
 
 * */
