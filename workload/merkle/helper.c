#include <stdio.h>
#include <string.h>
#include "sha256.h"
#include "merkle.h"

#define LENGTH_HASH_AS_STRING (SHA256_BLOCK_SIZE * 2 + 1)

void print_hash(BYTE* hashes)
{
	int i = 0;
	for(int i=0;i<SHA256_BLOCK_SIZE;i++)
		printf("%02x",*(hashes+i));
	printf("\n");

}

void as_string(BYTE* hash, char* hash_as_string, int length_hash_as_string)
{
	int i = 0;
	char hash_string[64];//each byte of the 32-byte hash 
	//requires 2 bytes to be stored as character
	for(int i = 0; i < SHA256_BLOCK_SIZE; i++)
		sprintf((hash_string + (i * 2)), "%02x", *(hash + i));
	//printf("%s\n", hash_string);
	strncpy(hash_as_string, hash_string, length_hash_as_string);
	hash_as_string[length_hash_as_string] = '\0';
	return ;
	
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
