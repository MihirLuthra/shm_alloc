#ifndef __BUILTIN_ALTERNATIVES_H__
#define __BUILTIN_ALTERNATIVES_H__

#include "shm_types.h"

int builtin_alternative_popcount(lock_free_int);
int builtin_alternative_clz(lock_free_int);
int builtin_alternative_ffsl(unsigned long);

#endif /* __BUILTIN_ALTERNATIVES_H__ */
