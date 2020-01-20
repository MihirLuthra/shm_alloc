<strong>Makefile</strong> : For generating `shm_alloc.dylib` on `macOS` and `shm_alloc.so` on others.

<strong>shm_alloc.c</strong> : Contains the main logic for the shared memory

<strong>shm_alloc.h</strong> : The only file that needs to be included by the user to use the library functions.

<strong>shm_bit_fiddler.h</strong> : Contains inline functions for bit manipulation

<strong>shm_constants.c</strong> : Contains code for setting shm_null and computing the offsets and 
sizes of various region in shared memory(like allocatable region, shm null etc).

<strong>shm_constants.h</strong> : Contains constants that are set before compiling(like `MAX_ALLOC_POW2`).

<strong>shm_debug.(c|h)</strong> : This is for debugging "this code".

<strong>shm_err.h</strong> : Contains macro definitions for printing errors.

<strong>shm_types.h</strong> : It only contains type definitions that are used in the shm code and are not for the user.

<strong>shm_user_types.h</strong> : It only contains type definitions that are accessible by the user.
