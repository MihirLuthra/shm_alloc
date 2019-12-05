#include <assert.h>

#include <string.h>

#include "shm_debug.h"
#include "shm_err.h"
#include "shm_util_funcs.h"
#include "shm_types.h"
#include "shm_constants.h"

#include <stdatomic.h>
#include <stdbool.h>

bool is_power_of_two(unsigned long long num)
{
	return (1 == __builtin_popcountll(num));
}

unsigned long long get_prev_power_of_two(unsigned long long num)
{
	if (num == 0)
		num = -1;

	return (1ULL << ((sizeof(unsigned long long)*8) - 1 - __builtin_clzll(num)));
}


unsigned long long get_next_power_of_two(unsigned long long num)
{
	if (num == 1 || num > (-1ULL)/2 + 1)
		return 1;

	return (1ULL << ((sizeof(unsigned long long)*8) - __builtin_clzll(num-1)));
}


int shm_bitmap_ffs(shm_bitmap bmp[BMP_ARR_SIZE])
{
	int pos;

	for (int i = (BMP_ARR_SIZE-1), pos_offt = 0 ; i >= 0 ; --i, pos_offt += BITS)
		if ((pos = __builtin_ffsll(bmp[i])) != 0)
				return (BITMAP_SIZE - (pos_offt + (pos-1)));

	return 0;
}

int get_abs_bit_pos(int rel_bit_pos, size_t mem_level)
{
	MEM_RANGE_ASSERT(mem_level);
	assert(rel_bit_pos >= 0);

	return (get_start_bit_pos_for_mem_level(mem_level) + rel_bit_pos);
}


int get_rel_bit_pos(int abs_bit_pos)
{
	assert(abs_bit_pos >= 1 && abs_bit_pos < BITMAP_SIZE);
	return (abs_bit_pos - get_prev_power_of_two(abs_bit_pos));
}

bool is_bit_set(shm_bitmap bmp[BMP_ARR_SIZE], int pos)
{
	assert(pos >= 0 && pos < BITMAP_SIZE);
	int idx = pos/BITS;
	return (bmp[idx] & (1ULL << (BITS - (pos % BITS) - 1)));
}

void set_bit(shm_bitmap bmp[BMP_ARR_SIZE], int pos)
{
	assert(pos >= 0 && pos < BITMAP_SIZE);
	int idx = pos/BITS;
	bmp[idx] |= 1ULL << (BITS - (pos % BITS) - 1);
}

bool set_bit_race_free(_Atomic(shm_bitmap) bmp[BMP_ARR_SIZE], int pos)
{
	assert(pos >= 0 && pos < BITMAP_SIZE);

	shm_bitmap old_bmp[BMP_ARR_SIZE], new_bmp[BMP_ARR_SIZE] = {0};

	set_bit(new_bmp, pos);

	bool cas_result;

	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i) {

		cas_result = true;

		do {
			old_bmp[i] = bmp[i];
			new_bmp[i] |= old_bmp[i];

			if (old_bmp[i] == new_bmp[i]) {

				cas_result = false;
				break;
			}

		} while ( !atomic_compare_exchange_strong_explicit(&bmp[i], &old_bmp[i], new_bmp[i], memory_order_relaxed, memory_order_relaxed) );


		if (cas_result == true)
			break;
	}

	return (cas_result);
}

bool unset_bit_race_free(_Atomic(shm_bitmap) bmp[BMP_ARR_SIZE], int pos)
{
	assert(pos >= 0 && pos < BITMAP_SIZE);

	shm_bitmap old_bmp[BMP_ARR_SIZE], new_bmp[BMP_ARR_SIZE] = {0};

	set_bit(new_bmp, pos);

	bool cas_result;

	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i) {

		cas_result = true;

		do {
			old_bmp[i] = bmp[i];
			new_bmp[i] = old_bmp[i] & ~(new_bmp[i]);

			if (old_bmp[i] == new_bmp[i]) {

				cas_result = false;
				break;
			}

		} while ( !atomic_compare_exchange_strong_explicit(&bmp[i], &old_bmp[i], new_bmp[i], memory_order_relaxed, memory_order_relaxed) );

		if (cas_result == true)
			break;
	}

	return (cas_result);
}

void unset_bit(shm_bitmap bmp[BMP_ARR_SIZE], int pos)
{
	assert(pos >= 0 &&  pos < BITMAP_SIZE);
	int idx = pos/BITS;
	bmp[idx] &= ~((shm_bitmap)1 << (BITS - (pos % BITS) - 1));
}


void set_mask_for_range(shm_bitmap mask[BMP_ARR_SIZE], int from, int to)
{
	assert(from >= 0 && from <= BITMAP_SIZE \
	    && to >= 0 && to <= BITMAP_SIZE);

	memset(mask, 0, sizeof(shm_bitmap) * BMP_ARR_SIZE);

	int start_idx = from/BITS;
	int end_idx   = (to-1)/BITS;

	for (int idx = start_idx ; idx <= end_idx ; ++idx)
		mask[idx] = (shm_bitmap)-1;

	mask[end_idx]    = mask[end_idx] << ( BITS - ((to-1) % BITS) - 1);
	mask[start_idx] &= (shm_bitmap)-1 >> (from % BITS);
}

void shm_bitmap_and(shm_bitmap bmp1[BMP_ARR_SIZE], shm_bitmap bmp2[BMP_ARR_SIZE], shm_bitmap res[BMP_ARR_SIZE])
{
	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i)
		res[i] = bmp1[i] & bmp2[i];
}

void shm_bitmap_nand(shm_bitmap bmp1[BMP_ARR_SIZE], shm_bitmap bmp2[BMP_ARR_SIZE], shm_bitmap res[BMP_ARR_SIZE])
{
	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i)
		res[i] = ~(bmp1[i] & bmp2[i]);
}

void shm_bitmap_or(shm_bitmap bmp1[BMP_ARR_SIZE], shm_bitmap bmp2[BMP_ARR_SIZE], shm_bitmap res[BMP_ARR_SIZE])
{
	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i)
		res[i] = bmp1[i] | bmp2[i];
}

void shm_bitmap_not(shm_bitmap bmp[BMP_ARR_SIZE], shm_bitmap res[BMP_ARR_SIZE])
{
	for (int i = 0 ; i < BMP_ARR_SIZE ; ++i)
        res[i] = ~bmp[i];
}

bool is_bit_range_zero(shm_bitmap bmp[BMP_ARR_SIZE], int from, int to)
{
	assert(from >= 0 && from <= BITMAP_SIZE 
	    && to >= 0 && to <= BITMAP_SIZE);

	shm_bitmap mask[BMP_ARR_SIZE];
	set_mask_for_range(mask, from, to);

	for (int idx = 0 ; idx < BMP_ARR_SIZE ; ++idx)
		if ((bmp[idx] & mask[idx]) != 0ULL)
			return (false);
	
	return (true);
}

void set_bits_in_range(shm_bitmap bmp[BMP_ARR_SIZE], int from, int to)
{
	assert(from >= 0 && from <= BITMAP_SIZE \
	    && to >= 0 && to <= BITMAP_SIZE);

	shm_bitmap mask[BMP_ARR_SIZE];
	set_mask_for_range(mask, from, to);

	for (int idx = 0 ; idx < BMP_ARR_SIZE ; ++idx)
		bmp[idx] |= mask[idx];
}


void unset_bits_in_range(shm_bitmap bmp[BMP_ARR_SIZE], int from, int to)
{
	assert(from >= 0 && from <= BITMAP_SIZE \
	    && to >= 0 && to <= BITMAP_SIZE);

	shm_bitmap mask[BMP_ARR_SIZE];
	set_mask_for_range(mask, from, to);

	for (int idx = 0 ; idx < BMP_ARR_SIZE ; ++idx)
		bmp[idx] &= ~mask[idx];
}

int get_start_bit_pos_for_mem_level(size_t mem)
{
	MEM_RANGE_ASSERT(mem);

	return (MAX_ALLOCATABLE_SIZE/mem);
}

int get_end_bit_pos_for_mem_level(size_t mem)
{
	MEM_RANGE_ASSERT(mem);

	return (MAX_ALLOCATABLE_SIZE/mem * 2);
}

int get_bit_cnt_for_mem_level(size_t mem)
{
	MEM_RANGE_ASSERT(mem);

	return (MAX_ALLOCATABLE_SIZE/mem);
}


int normalize_bit_pos_for_level(int cur_relative_bit_pos, size_t cur_mem_level, size_t new_mem_level)
{
	MEM_RANGE_ASSERT(cur_mem_level);
	MEM_RANGE_ASSERT(new_mem_level);
	assert(cur_relative_bit_pos >= 0);

	int bit_pos_in_new_mem_lv, bit_cnt_in_cur_mem_lv, bit_cnt_in_new_mem_lv;
	long double ratio_pos_to_bit_cnt;

	bit_cnt_in_cur_mem_lv = get_bit_cnt_for_mem_level(cur_mem_level);
	bit_cnt_in_new_mem_lv = get_bit_cnt_for_mem_level(new_mem_level);

	if (cur_relative_bit_pos >= bit_cnt_in_cur_mem_lv)
		return -1;

	ratio_pos_to_bit_cnt = (long double)cur_relative_bit_pos / bit_cnt_in_cur_mem_lv;

	bit_pos_in_new_mem_lv = ratio_pos_to_bit_cnt * bit_cnt_in_new_mem_lv;

	return (bit_pos_in_new_mem_lv);
}
