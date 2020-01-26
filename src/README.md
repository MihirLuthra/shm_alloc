<strong>Makefile</strong> : For generating `shm_alloc.dylib` on `macOS` and `shm_alloc.so` on others.

<strong>builtin_alternatives.(c|h)</strong> : For using in case of absense of gcc builtins.

<strong>shm_alloc.c</strong> : Contains the main logic for the shared memory

<strong>shm_alloc.h</strong> : The only file that needs to be included by the user to use the library functions.

<strong>shm_bit_fiddler.h</strong> : Contains inline functions for bit manipulation

<strong>shm_constants.h</strong> : Contains constants that are set before compiling(like `MAX_ALLOC_POW2`) and inline getters.

<strong>shm_debug.(c|h)</strong> : This is for debugging "this code".

<strong>shm_err.h</strong> : Contains macro definitions for printing errors.

<strong>shm_types.h</strong> : It contains type definitions that are used in the shm code.

<strong>shm_user_types.h</strong> : It only contains type definitions and constants that are visible to user as well. It is 
as subset of <strong>shm_types.h</strong> and is included in it.
