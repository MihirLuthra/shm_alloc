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
 *   Just use NULL if you don't care where in the address space
 *   shared memory is present.
 *  param2 : This is also optional "if" filename has been
 *   defined by environment variable SHM_FILE.
 */
bool shm_init(void *optional_addr, const char *shm_filename);
void shm_deinit(void);

void * get_shm_user_base(void);

shm_offt shm_malloc(size_t size);
shm_offt shm_calloc(size_t count, size_t size);
void     shm_free(shm_offt shm_ptr);

/*
 * The following 3 are just wrappers around
 * shm_(m|c)alloc() and shm_free().
 *
 * ptr_(m|c)alloc() call shm_(m|c)alloc() internally
 * and return shm base address + the offset returned from
 * shm_(m|c)alloc. In case of failures NULL is returned(not SHM_NULL).
 *
 * ptr_free() evaluates offset by its argument via SHM_ADDR_TO_OFFT()
 * and frees that offset using shm_free().
 *
 * These can be used instead of their shm_* versions.
 * Wherever common data among processes is required,
 * SHM_ADDR_TO_OFFT() and SHM_OFFT_TO_ADDR() can be used to get or set
 * offsets instead of pointers.
 *
 * These can also be helpful when `optional_addr` param is used in
 * shm_init() and all process have their mappings on the exact same place.
 * In that case ptr_*() can be used without dealing with offsets.
 */
void * ptr_malloc(size_t size);
void * ptr_calloc(size_t count, size_t size);
void   ptr_free(void *ptr);

/*
 * To get the address in the current process with the help
 * of offset.
 */
#define SHM_OFFT_TO_ADDR(offset) ((void *)((uint8_t *)get_shm_user_base() + offset))

/*
 * To get the address in the current process with the help
 * of offset.
 */
#define SHM_ADDR_TO_OFFT(address) ((shm_offt)((uint8_t *)address - (uint8_t *)get_shm_user_base()))


size_t get_shm_max_allocatable_size();
size_t get_shm_min_allocatable_size();
size_t get_sizeof_block_header();

#endif /* __SHM_ALLOC_H__ */
