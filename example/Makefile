SHM_DIR := $(abspath ../src/)

detected_OS := $(shell uname 2>/dev/null || echo Unknown)

ifeq ($(detected_OS), Darwin)
CC                := clang
DYLIB_EXT         := dylib
DYNAMIC_LIB_FLAGS := -dynamiclib
else
CC                := gcc
DYLIB_EXT         := so
DYNAMIC_LIB_FLAGS := -fPIC -shared
LIBS              := -latomic
endif

CFLAGS  := -O3 -D_POSIX_C_SOURCE=199506L -Wall -Wextra -fsanitize=address -I$(SHM_DIR)

example.out: example.c $(SHM_DIR)/libshm_alloc.$(DYLIB_EXT)
	$(CC) $(CFLAGS) -o $@ $<  -L$(SHM_DIR) -lshm_alloc

example_with_ptr.out: example_with_ptr.c $(SHM_DIR)/libshm_alloc.$(DYLIB_EXT)
	$(CC) $(CFLAGS) -o $@ $<  -L$(SHM_DIR) -lshm_alloc

$(SHM_DIR)/libshm_alloc.$(DYLIB_EXT)::
	make -C $(SHM_DIR)

clean:
	rm *.dbgfl *.so *.out *.o .*.swp *.dylib shm_file_*
