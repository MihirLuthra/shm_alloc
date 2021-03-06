#ifndef __SHM_TYPES_H__
#define __SHM_TYPES_H__

#include "shm_user_types.h"
#include <stdatomic.h>
#include <stdint.h>

#if ATOMIC_LONG_LOCK_FREE == 2

typedef unsigned long shm_bitmap;
typedef unsigned long lock_free_int;

#elif ATOMIC_INT_LOCK_FREE == 2

typedef unsigned int shm_bitmap;
typedef unsigned int lock_free_int;

#else /* ATOMIC_INT_LOCK_FREE == 2 */

#	error "Neither ATOMIC_LONG_LOCK_FREE nor ATOMIC_INT_LOCK_FREE is 2, can't proceed"

#endif

struct file_data {
	int          fd;
	const char * name;
	size_t       size;
};

struct mmap_data {
	void * base;
	size_t size;
};

struct shm_manager {
	struct mmap_data shm_mapping;
	struct file_data shm_file;
};

struct shm_data_table {
	_Atomic(shm_offt) start_blk_mgr_offt;
};

struct shm_block_mgmt {
	_Atomic(shm_bitmap) mgmt_bmp;
	_Atomic(size_t)     mem_used;
};

struct blk_hdr {
	size_t mem;
};

struct bmp_data_mgr {
	int    bitmap_no;
	int    relative_bit_pos;
	int    abs_bit_pos;
	size_t mem_level;
};

struct mem_offt_mgr {
	shm_offt offt_to_blk;
	shm_offt internal_offt;
	shm_offt offt_to_allocated_mem;
	size_t   mem;
};

#endif /* __SHM_TYPES_H__ */
