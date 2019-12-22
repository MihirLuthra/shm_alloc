<h2>How to use?</h2>

Set environment variable `SHM_FILE` to contain path to the file which you would like to use as the shared memory.

For example,

```
export SHM_FILE="path/to/shm/file"
```

The file shouldn't exist before.

In main code, include the file `shm_alloc.h`:

```
#include "shm_alloc.h"
```

Now to use shared memory, first get shared memory base by calling:

```
void *base = get_shm_user_base();
```

Let's say we want `100` bytes in shared memory. To allocate memory:

```
shm_offt mem = shm_malloc(100);
```

To access the memory:

```
(uint8_t *)base + mem
```

To simplify this process, we define 2 macros:

```
#define PTR(type)             shm_offt
#define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))
```

Now let's say we want to allocate a string in shared memory:

```
PTR(char) str;

size_t string_len = 100;
str = shm_calloc(string_len, sizeof(char));

if (str == SHM_NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
}

strcpy(ACCESS(str, char), "My test string!");

printf("%s\n", ACCESS(str, char));
```

Change `MAX_ALLOCATABLE_SIZE` and `MIN_ALLOCATABLE_SIZE` in `shm_constants.h` to the max/min size that can be allocated via 
`shm_malloc()`.
To change maximum size of shared memory, change `MAX_ALLOCATABLE_SHM_SIZE` in `shm_constants.h`.

(Will soon implement a better way of defining these)

If the implementation requires using `malloc(2)` and `shm_malloc()` being used interchangably, definations could be made like:

```
#if defined(USE_SHARED_MEMORY)
#    define PTR(type)             shm_offt
#    define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))
#else
#    define PTR(type)             type *
#    define ACCESS(ptr, type)     ((type *)ptr)
#endif
```