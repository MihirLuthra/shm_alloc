#if !defined(__SHM_ERR_H__)
#define __SHM_ERR_H__

#include <assert.h>
#include <errno.h>

#include "shm_types.h"
#include "shm_util_funcs.h"
#include "shm_alloc.h"

#include <stdio.h>

#define errfile (stderr)

#define P_ERR(format, ...) \
	fprintf(errfile, "FILE(%s) : FUNC(%s) : LINE(%d) : errno(%d -> %s): " format "\n", __FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)


#define MEM_RANGE_ASSERT(mem) \
	assert((mem) >= MIN_ALLOCATABLE_SIZE && (mem) <= MAX_ALLOCATABLE_SIZE && is_power_of_two(mem))

#endif /* !defined(__SHM_ERR_H__) */
