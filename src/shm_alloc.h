#if !defined(__SHM_ALLOC_H__)
#define __SHM_ALLOC_H__

#include "shm_user_types.h"
#include <stdint.h>

extern uint8_t *user_shm_base;

void __attribute__((constructor)) shm_init(void);
void shm_deinit(void);

void * get_shm_user_base(void);

shm_offt shm_malloc(size_t size);
shm_offt shm_calloc(size_t count, size_t size);
void     shm_free(shm_offt shm_ptr);

size_t get_shm_max_allocatable_size();
size_t get_shm_min_allocatable_size();

#endif /* !defined(__SHM_ALLOC_H__) */
