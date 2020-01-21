#ifndef __SHM_ALLOC_H__
#define __SHM_ALLOC_H__

#include "shm_user_types.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * Can be used before shm_init() to get
 * the mapping size that would be needed.
 */
size_t get_mapping_size_needed_by_shm();

/*
 * shm_init() is thread safe.
 * Although still the preferred way is to call it
 * inside a function which is labelled as __attribute__((constructor))
 *
 * OPTIONAL:
 *  param1 : should be a page aligned address that will be used as
 *   first argument in mmap(2) with MAP_FIXED.
 *  Just use NULL if you don't care where in the address space
 *  shared memory is present.
 */
bool shm_init(void *optional_addr);
void shm_deinit(void);

void * get_shm_user_base(void);

shm_offt shm_malloc(size_t size);
shm_offt shm_calloc(size_t count, size_t size);
void     shm_free(shm_offt shm_ptr);

void * ptr_malloc(size_t size);
void * ptr_calloc(size_t count, size_t size);
void   ptr_free(void *ptr);


size_t get_shm_max_allocatable_size();
size_t get_shm_min_allocatable_size();

#endif /* __SHM_ALLOC_H__ */
