#ifndef __SHM_USER_TYPES_H__
#define __SHM_USER_TYPES_H__

#include <stdio.h>
#include <stdatomic.h>

#if ATOMIC_LONG_LOCK_FREE == 2

typedef unsigned long shm_offt;
#define PRIu_shm_offt "lu"

#elif ATOMIC_INT_LOCK_FREE == 2

typedef unsigned int shm_offt;
#define PRIu_shm_offt "u"

#else /* ATOMIC_INT_LOCK_FREE == 2 */

#	error "Neither ATOMIC_LONG_LOCK_FREE nor ATOMIC_INT_LOCK_FREE is 2, can't proceed"

#endif

#define SHM_NULL ((shm_offt)0)

#endif /* __SHM_USER_TYPES_H__ */
