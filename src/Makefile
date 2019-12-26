detected_OS := $(shell uname 2>/dev/null || echo Unknown)

ifeq ($(detected_OS), Darwin)
CC                := clang
DYLIB_EXT         := dylib
DYNAMIC_LIB_FLAGS := -dynamiclib
else
CC                := gcc
DYLIB_EXT         := so
DYNAMIC_LIB_FLAGS := -fPIC -shared
endif

CFLAGS := -O3 -Wall -Wextra

CFLAGS += $(USER_FLAGS)

SHM_LIB_NAME := libshm_alloc.$(DYLIB_EXT)


$(SHM_LIB_NAME): shm_alloc.c shm_alloc.h shm_bit_fiddler.h shm_constants.h shm_err.h shm_types.h shm_constants.o
	$(CC) $(CFLAGS) $(DYNAMIC_LIB_FLAGS) -o $@ $< shm_constants.o

shm_constants.o: shm_constants.c shm_constants.h shm_user_types.h shm_types.h shm_err.h
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY : clean debug

clean:
	rm *.so *.out .*.swp *.dbgfl *.o *.dylib

# For debugging

debug: shm_alloc.c shm_alloc.h shm_bit_fiddler.h shm_types.h shm_err.h shm_constants.o libshm_debug.$(DYLIB_EXT)
	$(CC) $(CFLAGS) $(DYNAMIC_LIB_FLAGS) -o $(SHM_LIB_NAME) $<  shm_constants.o -L. -lshm_debug

# TODO: Want to LD_PRELOAD this to intercept thread creation functions
# So kept this a dylib
libshm_debug.$(DYLIB_EXT): shm_debug.c shm_debug.h shm_constants.h shm_types.h shm_err.h
	$(CC) $(CFLAGS) $(DYNAMIC_LIB_FLAGS) -o $@ $<

