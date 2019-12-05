CC=clang
CFLAGS=-Wall -fsanitize=address

libshm_alloc.so: shm_alloc.c shm_alloc.h libshm_util_funcs.so libshm_constants.so Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ shm_alloc.c -lc -L. -lshm_util_funcs -lshm_constants

libshm_alloc_debug.so: shm_alloc.c shm_alloc.h libshm_debug.so libshm_util_funcs.so libshm_constants.so Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ shm_alloc.c -lc -L. -lshm_util_funcs -lshm_constants -lshm_debug

libshm_debug.so: shm_debug.h shm_debug.c Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ shm_debug.c -lc

libshm_util_funcs.so: shm_util_funcs.h shm_util_funcs.c libshm_constants.so shm_types.h shm_err.h Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ shm_util_funcs.c -lc -L. -lshm_constants

libshm_constants.so: shm_constants.h shm_constants.c Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ shm_constants.c -lc

clean:
	rm *.so *.out .*.swp *.dbgfl
