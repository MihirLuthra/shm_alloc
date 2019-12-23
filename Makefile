CC := gcc
CFLAGS := -Wall -fsanitize=address

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

SHM_LIB_NAME := libshm_alloc.so

$(SHM_LIB_NAME): $(SELF_DIR)shm_alloc.c $(SELF_DIR)shm_alloc.h shm_util_funcs.o shm_constants.o
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< shm_util_funcs.o shm_constants.o


shm_util_funcs.o: $(SELF_DIR)shm_util_funcs.c $(SELF_DIR)shm_util_funcs.h \
                      $(SELF_DIR)shm_types.h $(SELF_DIR)shm_err.h
	$(CC) $(CFLAGS) -c -o $@ $<


shm_constants.o: $(SELF_DIR)shm_constants.c $(SELF_DIR)shm_constants.h
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY : clean

clean:
	rm *.so *.out .*.swp *.dbgfl *.o

# For debugging

debug: $(SELF_DIR)shm_alloc.c $(SELF_DIR)shm_alloc.h shm_util_funcs.o shm_constants.o \
	                  libshm_debug.so
	$(CC) $(CFLAGS) -fPIC -shared -o $(SHM_LIB_NAME) $< shm_util_funcs.o shm_constants.o -L. -lshm_debug

libshm_debug.so: $(SELF_DIR)shm_debug.c $(SELF_DIR)shm_debug.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

