


/*
 * NOTE:
 *  Don't use functions from any other shm files here.
 *  It makes it difficult to link it if debugging gets needed.
 *
 */
#include <pthread.h>

#include "shm_err.h"
#include "shm_constants.h"
#include "shm_types.h"
#include "shm_debug.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__thread char pid_file_name[MAXPATHLEN];
__thread FILE *pid_file;

bool is_power_of_two(unsigned long long num)
{   
    return (1 == __builtin_popcountll(num));
}


void open_pid_file()
{
    char pid_file_name_alias[MAXPATHLEN];

#ifndef DEBUG_DIR
#define DEBUG_DIR getenv("PWD")
#endif

    sprintf(pid_file_name_alias, "%s/pid=%d,tid=%ld.dbgfl", DEBUG_DIR, getpid(), (long)pthread_self());
    
	if (strcmp(pid_file_name_alias, pid_file_name)) {
        pid_file = NULL;
        strcpy(pid_file_name, pid_file_name_alias);
    }

    if (pid_file == NULL)
        pid_file = fopen(pid_file_name, "a+");

    if (pid_file == NULL) {
        P_ERR("fopen(2) failed for file %s", pid_file_name);
        abort();
    }
}

void print_bmp_data(struct bmp_data_mgr bmp_data, FILE *outfile)
{
	fprintf(outfile, "bitmap_no : %d\n", bmp_data.bitmap_no);
	fprintf(outfile, "relative_bit_pos : %d\n", bmp_data.relative_bit_pos);
	fprintf(outfile, "abs_bit_pos : %d\n", bmp_data.abs_bit_pos);
	fprintf(outfile, "mem_level : %zu\n", bmp_data.mem_level);
}

void print_mem_offt_data(struct mem_offt_mgr mem_offt_data, FILE *outfile)
{
	fprintf(outfile, "offt_to_blk : %zu\n", mem_offt_data.offt_to_blk);
	fprintf(outfile, "internal_offt : %zu\n", mem_offt_data.internal_offt);
	fprintf(outfile, "offt_to_allocated_mem : %zu\n", mem_offt_data.offt_to_allocated_mem);
	fprintf(outfile, "mem : %zu\n", mem_offt_data.mem);
}

void print_buddy_bitmap(shm_bitmap bmp[BMP_ARR_SIZE], FILE *outfile)
{   
    size_t mem = MAX_ALLOCATABLE_SIZE;
    
    for (int i = 1 ; i < BITMAP_SIZE  ; ++i)
    {   
        fprintf(outfile, "%llu", (bmp[i/BITS] >> (BITS - (i % BITS) - 1) & (shm_bitmap)1));
        
        if (is_power_of_two(i+1)) {
            fprintf(outfile, " --> %zu \n", (mem));
            mem /= 2;
        }
    }
    fprintf(outfile, "\n");
}


void print_all_bits(shm_bitmap bmp, FILE *outfile)
{
    for(int i = BITS - 1 ; i >= 0 ; --i)
        fprintf(outfile, "%llu", ((shm_bitmap)bmp >> i) & (shm_bitmap)1);

	fprintf(outfile, "\n");
}
