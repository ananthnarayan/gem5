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
#include "gem5/m5ops.h"
#include "gem5/asm/generic/m5ops.h"

#define LENGTH_HASH_AS_STRING (SHA256_BLOCK_SIZE * 2 + 1)  
int main(int argc, char** argv)
{
	char filename[BUFSIZ];
	BYTE* text;
	SHA256_CTX ctx;
	unsigned int text_length;
	BYTE **hashes;
	char **hashes_as_strings;
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
	hashes_as_strings = (char**) calloc((size_t)num_blocks, (size_t) sizeof(char*));
	memset((void*) hashes, 0, sizeof(BYTE*) * (size_t)num_blocks);
	memset((void*) hashes_as_strings, 0, sizeof(char*) * (size_t)num_blocks);
	for(int j = 0; j < num_blocks; j++)
	{
		hashes[j] = (BYTE*) malloc(sizeof(BYTE) * SHA256_BLOCK_SIZE + 1);
		memset((void*)hashes[j], 0, sizeof(BYTE) * SHA256_BLOCK_SIZE + 1);
		
		hashes_as_strings[j] = (char*) malloc(sizeof(char) * LENGTH_HASH_AS_STRING);
		memset((void*)hashes_as_strings[j], 0, sizeof(char) * LENGTH_HASH_AS_STRING);
		//fprintf(stderr, "Allocated: %d H:%p  SH:%p\n", j, hashes[j], hashes_as_strings[j]);
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
	/*
	 * Open the file, read block_size bytes from it and calculate the hash
	 * for that block. 
	 * Store the hash as a string for further processing.
	 * This is because the hashes are repeatedly concatenated and hashed
	 * as we go up the merkle tree. 
	 *
	 * */
	// m5_cpu_print();
	m5_pim_process(0);
	while(!feof(file))
	{
		size_t bytes_read;
		i = 0;
		memset(text, 0, sizeof(BYTE) * block_size);
		text[block_size] = '\0';
		bytes_read = fread((void*)text, sizeof(BYTE), (size_t)block_size, file);
		text[bytes_read] = '\0';
		// m5_pim_process(0);
		sha256_init(&ctx);
		sha256_update(&ctx, text, bytes_read);
		sha256_final(&ctx, hashes[i]);
		// m5_host_process();
		//print_hash(hashes[i]);
		as_string(hashes[i], hashes_as_strings[i], LENGTH_HASH_AS_STRING);
		//fprintf(stdout, "\t%s\n", hashes_as_strings[i]);
		i++;
	}
	fclose(file);
	// m5_cpu_print();

	unsigned int num_hashes_to_process = num_blocks;
	fprintf(stderr, "invoking run_merkle\n");
	run_merkle(num_hashes_to_process, hashes_as_strings);
	fprintf(stdout, "Blocks %d\n", num_blocks);
	fprintf(stdout, "Final hash: %s\n", hashes_as_strings[0]);
	// m5_host_process();
	for(int j = 0; j < num_blocks; j++)
	{
		//fprintf(stderr, "Freeing hashes pointer (%d): %p %p\n", j, hashes[j], hashes_as_strings[j]);
		free(hashes[j]);
		free(hashes_as_strings[j]);
	}
	free(text);
	free(hashes);
	free(hashes_as_strings);
	m5_host_process();
	return(0);
	//
}
/*
 * Calculate the merkle root hash.
 * Iterate through the hashes, combining two hash strings
 * to one, hashing that string and storing it. 
 * Repeat this till only one hash value remains. 
 *
 * Note that internet documentation seems to indicate 
 * that merkle trees are balanced binary trees, though
 * non balanced binary trees are ok. 
 * All the examples easily take  2^n blocks and build a tree
 * which naturally becomes balanced. 
 * How to handle an odd or not 2^n blocks is not discussed. 
 *
 * In this implementation, we iteratively combine 2 hashes, 
 * and calculate the hash. It doesn't imply that the tree structure
 * formed by this combination will be balanced. 
 * */
void run_merkle(int num_hashes_to_process, char **hashes_as_strings)
{
	SHA256_CTX ctx;

	while(num_hashes_to_process > 1)
	{
#define TWO_HASH_LENGTH 128 //64 + 64 
		int LENGTH_CONCATENATED_HASH_STRINGS = TWO_HASH_LENGTH + 1;
		int src_hash_idx = 0;
		int dest_hash_idx = 0;
		char concatenated_hash[LENGTH_CONCATENATED_HASH_STRINGS];
		int next_num_hashes;
		char one_hash_as_string[SHA256_BLOCK_SIZE * 2 + 1];
		BYTE one_hash[SHA256_BLOCK_SIZE];
		//fprintf(stdout, "Num hashes to process: %d\n", num_hashes_to_process);	
		dest_hash_idx = 0;
		next_num_hashes = 0;
		for(src_hash_idx = 0; src_hash_idx < num_hashes_to_process;src_hash_idx += 2)
		{
			memset(concatenated_hash, 0, sizeof(char) * LENGTH_CONCATENATED_HASH_STRINGS);
			if(src_hash_idx == num_hashes_to_process)
				break;//we have hit the end of this iteration. 
			if (src_hash_idx == num_hashes_to_process - 1)
			{
				//copy the current hash value to the new location and break the loop
				strcpy(hashes_as_strings[dest_hash_idx], hashes_as_strings[src_hash_idx]);
				memset(hashes_as_strings[src_hash_idx], 0,  sizeof(char) * SHA256_BLOCK_SIZE + 1);
				dest_hash_idx++;
				next_num_hashes++;
				break;
			}
			strncpy(concatenated_hash, hashes_as_strings[src_hash_idx], LENGTH_HASH_AS_STRING);
			memset(hashes_as_strings[src_hash_idx],     0, sizeof(char) * LENGTH_HASH_AS_STRING);
			strncat(concatenated_hash, hashes_as_strings[src_hash_idx + 1], LENGTH_HASH_AS_STRING);
			memset(hashes_as_strings[src_hash_idx + 1], 0, sizeof(char) * LENGTH_HASH_AS_STRING);

			sha256_init(&ctx);
			sha256_update(&ctx, concatenated_hash, LENGTH_CONCATENATED_HASH_STRINGS);
			sha256_final(&ctx, one_hash);

			as_string(one_hash, one_hash_as_string, LENGTH_HASH_AS_STRING);
			strcpy(hashes_as_strings[dest_hash_idx], one_hash_as_string);
			hashes_as_strings[dest_hash_idx][LENGTH_CONCATENATED_HASH_STRINGS] = '\0';
			next_num_hashes++;
			dest_hash_idx++;
		}
		num_hashes_to_process = next_num_hashes;

	}
}
/*
 
 * */
