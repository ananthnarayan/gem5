#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "sha256.h"
#include <math.h>
#include <sys/stat.h>
#include <memory.h>
#include "merkle.h"
// #include "gem5/m5ops.h"
// #include "gem5/asm/generic/m5ops.h"

unsigned int block_number ; 
//once set by the main thread, these are 'constant'
//for other threads.
unsigned int block_size;
unsigned int num_blocks;
unsigned int file_size;
BYTE **hashes;
struct ThreadParams
{
	unsigned int thread_id;
	unsigned int thread_number;
    FILE* file;
    pthread_mutex_t *file_access_mutex;
    pthread_mutex_t *hashes_mutex;
};
typedef struct  ThreadParams tparams;

void* thread_function(void* arg)
{
    //obtain the mutex. 
    //read the file contents
    //release the mutex
	BYTE* text;
    SHA256_CTX ctx;
    BYTE hash[SHA256_BLOCK_SIZE];
    size_t bytes_read;
    
    tparams *me = (tparams*)arg;
    text = (BYTE*) malloc(sizeof(BYTE) * block_size + 1);
   // me->thread_id = pthread_self();
    fprintf(stdout, "%s:%d Thread id: %d ; Thread number: %d \n",__FILE__, __LINE__, 
            me->thread_id, me->thread_number);
    while(!feof(me->file))
    {
        memset(text, 0, sizeof(BYTE) * block_size);
        text[block_size] = '\0';
        //lock the file access mutex
        pthread_mutex_lock(me->file_access_mutex);
        int my_block_number = block_number;
        if(feof(me->file))
        {
            pthread_mutex_unlock(me->file_access_mutex);
            pthread_exit(0);
        }
         bytes_read = fread((void*)text, sizeof(BYTE), (size_t)block_size, me->file);
        fprintf(stdout, "%s:%d Thread id: %d ; Thread number: %u Block number: %d \n",__FILE__, __LINE__, 
            me->thread_id, me->thread_number, block_number);
        block_number = block_number + 1; 
        //the next thread that does the file processing must use this block number as an index
        //into the hashes array.
        //unlock the file access mutex
        pthread_mutex_unlock(me->file_access_mutex);
        if(my_block_number > num_blocks)
        {
            fprintf(stderr, "Block number is > num blocks");
            free(text);
            pthread_exit(0);
        }
        text[bytes_read] = '\0';
        sha256_init(&ctx);
        sha256_update(&ctx, text, bytes_read);
        // m5_host_process();
        //print_hash(hashes[i]);
        //lock the hashes mutex
        pthread_mutex_lock(me->hashes_mutex);
        sha256_final(&ctx, hash);
        memcpy(hashes[my_block_number], hash, SHA256_BLOCK_SIZE);
        pthread_mutex_unlock(me->hashes_mutex);
        //as_string(hashes[my_block_number], hashes_as_strings[my_block_number], LENGTH_HASH_AS_STRING);
        //unlock the hashes mutex
    }
    free(text);
    fprintf(stdout, "%s:%d Thread id: %u ; Thread number: %d Exit\n",__FILE__, __LINE__, 
            me->thread_id, me->thread_number);
	pthread_exit(0);
}

//#define LENGTH_HASH_AS_STRING (SHA256_BLOCK_SIZE * 2 + 1)  
#define MAX_THREADS 32
int main(int argc, char **argv)
{

	char **hashes_as_strings;
    pthread_t threads[MAX_THREADS];
	tparams thread_args[MAX_THREADS];
    int num_threads = 0;
   	char filename[BUFSIZ];
    pthread_mutex_t file_access_mutex;
    pthread_mutex_t hashes_mutex;
    
	FILE *file = NULL;
	unsigned int MAX_BLOCK_SIZE = 128 * 1024 * 1024;
    num_threads = MAX_THREADS;
    
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
	file = fopen(filename, "r");
	if(file == NULL)
	{
		perror("Error reading file");
		exit(1);
	}
	/*This array will hold the hashes as as sequence of bytes*/
	hashes = (BYTE**) calloc( (size_t) num_blocks, (size_t)sizeof(BYTE*));
    /*This will hold the same set of hashes, in sequence, as strings.*/
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
	block_number = 0;
    pthread_mutex_init(&file_access_mutex, NULL);
    pthread_mutex_init(&hashes_mutex, NULL);
    for(int i = 0; i < MAX_THREADS; i++)
    {
            int tid;
            thread_args[i].thread_number = i;
            thread_args[i].file_access_mutex = &file_access_mutex;
            thread_args[i].hashes_mutex = &hashes_mutex;
            thread_args[i].file = file;            
            tid = pthread_create(&(threads[i]), NULL, thread_function, &(thread_args[i]));
            if(tid != 0 )
            {
                //error creating thread. 
                threads[i]  = -1;
                thread_args[i].thread_id = -1;
            }
            else
                thread_args[i].thread_id = threads[i];

            //the thread will do a pthread_self to set the thread_id
           
    }

	for(int i = 0;i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	fprintf(stdout, "%s %d Join done. Calculating top level merkle hash\n", __FILE__, __LINE__);
	for(int i = 0; i < num_blocks;  i++)
        as_string(hashes[i], hashes_as_strings[i], LENGTH_HASH_AS_STRING);   
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
	free(hashes);
	free(hashes_as_strings);
    fclose(file);
	return 0;
}
