#if !defined(__SHM_UTIL_FUNCS_H__)
#define __SHM_UTIL_FUNCS_H__

#include "shm_types.h"
#include "shm_constants.h"

#include <stdbool.h>

/* general operations */
bool is_power_of_two(unsigned long long);
unsigned long long get_prev_power_of_two(unsigned long long);
unsigned long long get_next_power_of_two(unsigned long long);


/* operations on shm_bitmap */
int shm_bitmap_ffs(shm_bitmap []);

void set_mask_for_range(shm_bitmap [], int, int);
bool is_bit_set(shm_bitmap bmp[], int);
bool is_bit_range_zero(shm_bitmap [], int, int);

void set_bit(shm_bitmap [], int);
bool set_bit_race_free(_Atomic(shm_bitmap) [], int);
void set_bits_in_range(shm_bitmap [], int, int);

void unset_bit(shm_bitmap [], int);
bool unset_bit_race_free(_Atomic(shm_bitmap) [], int);
void unset_bits_in_range(shm_bitmap [], int, int);

void shm_bitmap_and(shm_bitmap [], shm_bitmap [], shm_bitmap []);
void shm_bitmap_nand(shm_bitmap [], shm_bitmap [], shm_bitmap []);
void shm_bitmap_or(shm_bitmap [], shm_bitmap [], shm_bitmap []);

void shm_bitmap_not(shm_bitmap bmp[BMP_ARR_SIZE], shm_bitmap res[BMP_ARR_SIZE]);

/* operations specific to buddy system bitmap */
int get_start_bit_pos_for_mem_level(size_t);
int get_end_bit_pos_for_mem_level(size_t);
int get_bit_cnt_for_mem_level(size_t);
int normalize_bit_pos_for_level(int, size_t, size_t);
int get_abs_bit_pos(int, size_t);
int get_rel_bit_pos(int);

#endif /* !defined(__SHM_UTIL_FUNCS_H__) */
