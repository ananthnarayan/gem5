#include <stdio.h>
#include <string.h>
#include "sha256.h"
#include "merkle.h"

void print_hash(BYTE* hashes)
{
	int i = 0;
	for(int i=0;i<SHA256_BLOCK_SIZE;i++)
		printf("%02x",*(hashes+i));
	printf("\n");

}

void as_string(BYTE* hash, char* string_hash)
{
	int i = 0;
	char hash_string[64];//each byte of the 32-byte hash 
	//requires 2 bytes to be stored as character
	for(int i = 0; i < SHA256_BLOCK_SIZE; i++)
		sprintf((hash_string + (i * 2)), "%02x", *(hash + i));
	//printf("%s\n", hash_string);
	strncpy(string_hash, hash_string, 64);
	return ;
	
}
