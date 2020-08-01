#ifndef __MERKLE_H__
#define __MERKLE_H__

#include "sha256.h"
#define LENGTH_HASH_AS_STRING (SHA256_BLOCK_SIZE * 2 + 1)
void print_hash(BYTE* hashes);
void as_string(BYTE* hash, char* string_hash, int length_hash_as_string);
void run_merkle(int num_hashes_to_process, char **string_hashes);
#endif
