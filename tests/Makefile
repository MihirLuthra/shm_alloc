CC=clang

CFLAGS= -I.. -D DEBUG_SHM=0 -Wall -fsanitize=address -L.

test_shm_alloc.out: test_shm_alloc.c libshm_alloc.so librand_string_generator.so Makefile
	$(CC) $(CFLAGS) -o test_shm_alloc.out test_shm_alloc.c -lshm_alloc -lrand_string_generator

librand_string_generator.so: rand_string_generator.c rand_string_generator.h Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ rand_string_generator.c

libshm_alloc.so: ../shm_alloc.c ../shm_alloc.h libshm_debug.so libshm_util_funcs.so libshm_constants.so Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ../shm_alloc.c -lc -L. -lshm_util_funcs -lshm_constants -lshm_debug

libshm_debug.so: ../shm_debug.h ../shm_debug.c Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ../shm_debug.c -lc -L.

libshm_util_funcs.so: ../shm_util_funcs.h ../shm_util_funcs.c libshm_constants.so libshm_debug.so ../shm_types.h ../shm_err.h Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ../shm_util_funcs.c -lc -L. -lshm_constants -lshm_debug

libshm_constants.so: ../shm_constants.h ../shm_constants.c Makefile
	$(CC) $(CFLAGS) -fPIC -shared -o $@ ../shm_constants.c -lc

clean:
	rm *.dbgfl *.so *.out .*.swp shm_file
