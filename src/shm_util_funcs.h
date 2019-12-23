#if !defined(__SHM_UTIL_FUNCS_H__)
#define __SHM_UTIL_FUNCS_H__

#include "shm_types.h"
#include "shm_constants.h"

#include <stdbool.h>

/* general operations */
bool is_power_of_two(unsigned long long);
unsigned long long get_prev_power_of_two(unsigned long long);
unsigned long long get_next_power_of_two(unsigned long long);

/*
 * EXAMPLE BUDDY BITMAP:
 *
 * MAX_ALLOCATABLE_SIZE = 4096
 * MIN_ALLOCATABLE_SIZE = 32
 * BMP_ARR_SIZE         = 4
 * BITS                 = 64 (implies shm_bitmap is uint64_t)
 * BITMAP_SIZE          = 256
 * 
 * First bit is unused, remaining 255 bits:
 *
 * 0 --> 4096 
 * 00 --> 2048 
 * 0000 --> 1024 
 * 00000000 --> 512 
 * 0000000000000000 --> 256 
 * 00000000000000000000000000000000 --> 128 
 * 0000000000000000000000000000000000000000000000000000000000000000 --> 64 
 * 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 --> 32 
 *
 */


/* operations on shm_bitmap */

/*
 * retval:
 *  Returns one plus the index of the least significant 1-bit of param1,
 *  or if not bit is set, zero is returned.
 *
 * param1:
 *  A shm buddy bitmap
 */
int shm_bitmap_ffs(shm_bitmap []);

/*
 * Description:
 *  Sets param3 i.e. the mask to all 0 and then sets only those bits
 *  to 1 that are not set in param1 for memory level given by param2.
 *
 * param1:
 *  Buddy bitmap to be checked
 *
 * param2:
 *  Memory level for which mask needs to be set
 *
 * param3:
 *  The buddy bitmap which will be set (the mask)
 */
void get_settable_bits_mask(shm_bitmap [], size_t, shm_bitmap []);

/*
 * Description:
 *  Sets param1 to all zeros and sets bits in range
 *  param2 to param2
 *
 * param1:
 *  The mask which is set bit the function.
 *
 * param2:
 *  Lower limit of bit position(abs bit posn)
 *
 * param3:
 *  Upper limit of bit posn(abs bit posn)
 */
void set_mask_for_range(shm_bitmap [], int, int);

/*
 * Description:
 *  Checks if the bit posn given by param2 is set
 *  in param1 or no.
 *
 * retval:
 *  Returns true if set else false.
 *
 * param1:
 *  The buddy bitmap in which check needs to
 *  be made.
 *
 * param2:
 *  The bit posn to be checked(abs bit posn)
 */
bool is_bit_set(shm_bitmap bmp[], int);

/*
 * Description:
 *  Checks if the bit posn range given by param2
 *  and param3 is set in param1 or no.
 *
 * retval:
 *  Returns true if all bits in range are zero else false.
 *
 * param1:
 *  The buddy bitmap in which check needs to
 *  be made.
 *
 * param2:
 *  Lower limit of range of bit posn(abs bit posn)
 *
 * param3:
 *  Upper limit of range of bit posn(abs bit posn)
 */
bool is_bit_range_zero(shm_bitmap [], int, int);

/*
 * Description:
 *  Sets bit posn given by param2 in param1
 *
 * param1:
 *  The buddy bitmap in which bit needs to be set
 *
 * param2:
 *  Bit posn(abs bit posn)
 */
void set_bit(shm_bitmap [], int);

/*
 * Description:
 *  Sets bit posn given by param2 in param1 by CAS
 *  defined in cas.h
 *  It won't tamper with bitmap if bit is already set
 *  and will return false.
 *
 * retval:
 *  false if bit is already set else true after setting
 *  the bit in bitmap.
 *
 * param1:
 *  The buddy bitmap in which bit needs to be set
 *
 * param2:
 *  Bit posn(abs bit posn)
 */
bool set_bit_race_free(_Atomic(shm_bitmap) [], int);

/*
 * Description:
 *  Sets bits ranging from param2 to param3.
 *
 * param1:
 *  The buddy bitmap in which bit needs to be set
 *
 * param2:
 *  Lower limit of bit position(abs bit posn)
 *
 * param3:
 *  Upper limit of bit posn(abs bit posn)
 */
void set_bits_in_range(shm_bitmap [], int, int);

/*
 * Description:
 *  Unsets bit posn given by param2 in param1
 *
 * param1:
 *  The buddy bitmap in which bit needs to be unset
 *
 * param2:
 *  Bit posn(abs bit posn)
 */
void unset_bit(shm_bitmap [], int);

/*
 * Description:
 *  Unets bit posn given by param2 in param1 by CAS
 *  defined in cas.h
 *  It won't tamper with bitmap if bit is already unset
 *  and will return false.
 *
 * retval:
 *  false if bit is already unset else true after unsetting
 *  the bit in bitmap.
 *
 * param1:
 *  The buddy bitmap in which bit needs to be unset
 *
 * param2:
 *  Bit posn(abs bit posn)
 */
bool unset_bit_race_free(_Atomic(shm_bitmap) [], int);

/*
 * Description:
 *  Unsets bits ranging from param2 to param3.
 *
 * param1:
 *  The buddy bitmap in which bit needs to be unset
 *
 * param2:
 *  Lower limit of bit position(abs bit posn)
 *
 * param3:
 *  Upper limit of bit posn(abs bit posn)
 */
void unset_bits_in_range(shm_bitmap [], int, int);

/* operations specific to buddy system bitmap */

/*
 * Description:
 *  Returns abs bit posn where memory level given
 *  by param1 starts.
 *  For example, refer to example buddy bitmap above
 *  and see start posn of memory level 512 will be 8
 *  counting from the beginning.
 *
 * retval:
 *  Start abs bit posn where memory level given by
 *  param1 starts.
 *
 * param1:
 *  Memory level
 */
int get_start_bit_pos_for_mem_level(size_t);

/*
 * Same as above, just returns ending posn,
 * e.g., refer to example bitmap above, for 512 returns 16
 */
int get_end_bit_pos_for_mem_level(size_t);

/*
 * Returns number of bits in a particular memory
 * level.
 * e.g., refer to example bitmap above,
 * for level 4096, it would return 1,
 * for 2048 returns 2, for 1024 returns 4 and so on.
 */
int get_bit_cnt_for_mem_level(size_t);

/*
 * Description:
 *  This function deals with relative bit posn.
 *  e.g., See example buddy bitmap above,
 *  if 3rd bit in memory level 512 is set, i.e.,
 *  relative posn of that bit w.r.t mem level 512 is 2 (count starts from 0)
 *  Now if we want parent relative bit posn of this bit at memory level 2048,
 *  we call function like:
 *   int at_lv_2048 = normalize_bit_pos_for_level(2, 512, 2048);
 *  This would return 0 i.e. first bit in mem level 2048.
 *  If we call it to obtain starting child at mem level 32,
 *   int at_lv_32 = normalize_bit_pos_for_level(2, 512, 32);
 *  This would return 32 as 32nd bit in mem level 32 corresponds to
 *  2nd bit in mem level 512.
 *
 * retval:
 *  Relative bit posn at mem level param2
 *
 * param1:
 *  Relative bit posn at memory level param2
 *
 * param2:
 *  Current memory level
 *
 * param3:
 *  New memory level 
 */
int normalize_bit_pos_for_level(int, size_t, size_t);

/*
 * Description:
 *  Convert relative bit to abs bit posn.
 *
 * retval:
 *  Abs bit posn corresponding to param1 at memory
 *  level param2.
 *
 * param1:
 *  Relative bit posn at memory level given by param2.
 *
 * param2:
 *  Memory level
 */
int get_abs_bit_pos(int, size_t);

/*
 * Converts given abs bit pos to relative bit posn
 */
int get_rel_bit_pos(int);

#endif /* !defined(__SHM_UTIL_FUNCS_H__) */
