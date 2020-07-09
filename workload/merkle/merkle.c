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

#include "sha256.h"
#include "merkle.h"

int main(int argc, char** argv)
{
	char filename[BUFSIZ];
	BYTE* text;
	SHA256_CTX ctx;
	unsigned int text_length;
	BYTE **hashes;//[SHA256_BLOCK_SIZE];
	char **string_hashes;
	int i = 0;
	unsigned int block_size = 0;
	unsigned int file_size = 0;
	unsigned int num_blocks = 0;
	FILE *file = NULL;
	unsigned int MAX_BLOCK_SIZE = 128 * 1024 * 1024;

	if(argc < 2)
	{
		fprintf(stderr, "Insufficient parameters.\n");
		exit(1);
	}
	block_size = strtol(argv[2], NULL, 0);
	if(block_size > MAX_BLOCK_SIZE)
	{
		block_size = MAX_BLOCK_SIZE;

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
	string_hashes = (char**) calloc((size_t)num_blocks, (size_t) sizeof(char*));
	memset((void*) hashes, 0, sizeof(BYTE*) * (size_t)num_blocks);
	memset((void*) string_hashes, 0, sizeof(char*) * (size_t)num_blocks);
	for(int j = 0; j < num_blocks; j++)
	{
		hashes[j] = (BYTE*) malloc(sizeof(BYTE) * SHA256_BLOCK_SIZE + 1);
		memset((void*)hashes[j], 0, sizeof(BYTE) * SHA256_BLOCK_SIZE + 1);
		
		string_hashes[j] = (char*) malloc(sizeof(char) * SHA256_BLOCK_SIZE + 1);
		memset((void*)string_hashes[j], 0, sizeof(char) * SHA256_BLOCK_SIZE + 1);
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
		//print_hash(hashes[i]);
		as_string(hashes[i], string_hashes[i]);
		fprintf(stdout, "\t%s\n", string_hashes[i]);
		i++;
	}
	fclose(file);

	unsigned int num_hashes_to_process = num_blocks;

	while(num_hashes_to_process > 1)
	{
#define TWO_HASH_LENGTH 128 //64 + 64 
		int length = TWO_HASH_LENGTH;
		int src_hash_idx = 0;
		int dest_hash_idx = 0;
		char concatenated_hash[TWO_HASH_LENGTH + 1];
		int next_num_hashes;
		char one_hash_as_string[SHA256_BLOCK_SIZE * 2 + 1];
		BYTE one_hash[SHA256_BLOCK_SIZE];
		//fprintf(stdout, "Num hashes to process: %d\n", num_hashes_to_process);	
		dest_hash_idx = 0;
		next_num_hashes = 0;
		for(src_hash_idx = 0; src_hash_idx < num_hashes_to_process;src_hash_idx += 2)
		{
			memset(concatenated_hash, 0, sizeof(char) * TWO_HASH_LENGTH + 1);
			if(src_hash_idx == num_hashes_to_process)
				break;//we have hit the end of this iteration. 
			if (src_hash_idx == num_hashes_to_process - 1)
			{
				//copy the current hash value to the new location and break the loop
				strcpy(string_hashes[dest_hash_idx], string_hashes[src_hash_idx]);
				memset(string_hashes[src_hash_idx], 0, sizeof(char) * SHA256_BLOCK_SIZE);
				dest_hash_idx++;
				next_num_hashes++;
				break;
			}
			strncpy(concatenated_hash, string_hashes[src_hash_idx], SHA256_BLOCK_SIZE + 1);
			memset(string_hashes[src_hash_idx], 0, sizeof(char) * SHA256_BLOCK_SIZE);
			strcat(concatenated_hash, string_hashes[src_hash_idx + 1]);
			memset(string_hashes[src_hash_idx + 1], 0, sizeof(char) * SHA256_BLOCK_SIZE);

			sha256_init(&ctx);
			sha256_update(&ctx, concatenated_hash, TWO_HASH_LENGTH);
			sha256_final(&ctx, one_hash);

			as_string(one_hash, one_hash_as_string);
			strcpy(string_hashes[dest_hash_idx], one_hash_as_string);
			next_num_hashes++;
			dest_hash_idx++;
		}
		num_hashes_to_process = next_num_hashes;

	}
	fprintf(stdout, "Blocks %d\n", num_blocks);
	fprintf(stdout, "\t\t%s\n", string_hashes[0]);
	for(int j = 0; j < num_blocks; j++)
	{
		//fprintf(stderr, "%d ", j);
		//free(hashes[j]);
		//free(string_hashes[j]);
	}
	free(text);
	free(hashes);
	free(string_hashes);
	return(0);
	//
}

void run_merkle()
{

}
/*
 
 * */
