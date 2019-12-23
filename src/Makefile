CC := gcc
CFLAGS := -Wall -Wextra

CFLAGS += $(USER_FLAGS)

SHM_LIB_NAME := libshm_alloc.so

$(SHM_LIB_NAME): shm_alloc.c shm_alloc.h shm_util_funcs.o shm_constants.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< shm_util_funcs.o shm_constants.o

shm_util_funcs.o: shm_util_funcs.c shm_util_funcs.h shm_types.h shm_err.h
	$(CC) $(CFLAGS) -c -o $@ $<

shm_constants.o: $(SELF_DIR)shm_constants.c $(SELF_DIR)shm_constants.h
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY : clean

clean:
	rm *.so *.out .*.swp *.dbgfl *.o

# For debugging

debug: shm_alloc.c shm_alloc.h shm_util_funcs.o shm_constants.o libshm_debug.so
	$(CC) $(CFLAGS) -fPIC -shared -o $(SHM_LIB_NAME) $< shm_util_funcs.o shm_constants.o -L. -lshm_debug

libshm_debug.so: shm_debug.c shm_debug.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

