#if !defined(__CAS_H__)
#define __CAS_H__

#include <stdatomic.h>

#define CAS(mem, old_val, new_val) atomic_compare_exchange_weak(mem, old_val, new_val)

#endif
