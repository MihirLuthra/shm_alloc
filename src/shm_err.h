#ifndef __SHM_ERR_H__
#define __SHM_ERR_H__

#include <assert.h>
#include <errno.h>

#include "shm_types.h"
#include "shm_alloc.h"

#include <stdio.h>

#define errfile (stderr)

#define P_ERR(format) \
	fprintf(errfile, "FILE(%s) : FUNC(%s) : LINE(%d) : errno(%d -> %s): " format "\n", __FILE__, __func__, __LINE__, errno, strerror(errno))

#define P_ERR_WITH_VARGS(format, ...) \
	fprintf(errfile, "FILE(%s) : FUNC(%s) : LINE(%d) : errno(%d -> %s): " format "\n", __FILE__, __func__, __LINE__, errno, strerror(errno), __VA_ARGS__)


#endif /* __SHM_ERR_H__ */
