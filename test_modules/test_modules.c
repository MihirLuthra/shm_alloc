#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include "shm_module_test.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
	_Atomic(shm_bitmap) bmp[BMP_ARR_SIZE] = {0};
	shm_bitmap bmp2[BMP_ARR_SIZE] = {0};

	int i, rel, rel2;
	size_t mem, mem2;

	mem = atoi(argv[1]);

	for ( i = 1 ; i <= BITMAP_SIZE; ++i)
	{
		printf("for %d rel pos = %d\n", i, get_rel_bit_pos(i));
	}

	/*
	for (i = get_start_bit_pos_for_mem_level(mem) ; i < get_end_bit_pos_for_mem_level(mem) ; ++i) {
		printf("i = %d\n\n", i);

		set_bit(bmp2, i);

		print_buddy_bitmap(bmp2, stdout);

		rel = get_rel_bit_pos(i);

		mem2 = MAX_ALLOCATABLE_SIZE;

		printf("\n\nrel bit posn of main mem level = %d\n\n\n", rel);

		while (mem2 >= MIN_ALLOCATABLE_SIZE) {
			rel2 = normalize_bit_pos_for_level(rel, mem, mem2);
			printf("rel bit posn rel2 of mem level %zu = %d\n", mem2, rel2);
			mem2 /= 2;
		}
		unset_bit(bmp2, i);
	}
	*/

    return 0;
}
