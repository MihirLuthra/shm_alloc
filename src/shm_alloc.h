#if !defined(__SHM_ALLOC_H__)
#define __SHM_ALLOC_H__

#include "shm_user_types.h"
#include <stdint.h>

/*
 * Base that gets set in shm_init()
 */
extern uint8_t *user_shm_base;


/*
 * This is the constructor function which does the setting up
 * of shared mappings.
 *
 * It would first create/open a file that is to be used as
 * the shared memory, ftruncate(2) it to desired size(which also fills the file with zeros)
 * and mmap(2) the file into the current process.
 * Note:
 *  This function is only called before main() as per its attribute
 *  and child processes copy the mappings created by this function.
 *  In case if `__attribute__((constructor))` isn't usable, this function can handle
 *  being called within multiple processes/threads.
 */
#if SHM_DO_NOT_USE_CONSTRUCTOR
void shm_init(void);
#else
void __attribute__((constructor)) shm_init(void);
#endif

void shm_deinit(void);

void * get_shm_user_base(void);

shm_offt shm_malloc(size_t size);
shm_offt shm_calloc(size_t count, size_t size);
void     shm_free(shm_offt shm_ptr);

size_t get_shm_max_allocatable_size();
size_t get_shm_min_allocatable_size();

#endif /* !defined(__SHM_ALLOC_H__) */
