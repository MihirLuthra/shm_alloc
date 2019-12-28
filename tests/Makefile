SHM_DIR := ../src/
LSTACK_DIR := ./lock_free_stack/

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

CFLAGS  := -O3 -D TEST_THE_TEST=0 -Wall -Wextra -fsanitize=address -I$(SHM_DIR) -I$(LSTACK_DIR)

test_shm_alloc.out: test_shm_alloc.c $(SHM_DIR)libshm_alloc.$(DYLIB_EXT) rand_string_generator.o \
	              $(LSTACK_DIR)lstack.o
	cp $(SHM_DIR)libshm_alloc.$(DYLIB_EXT) .; \
	$(CC) $(CFLAGS) -o $@ $< rand_string_generator.o $(LSTACK_DIR)lstack.o -L. -lshm_alloc

rand_string_generator.o: rand_string_generator.c rand_string_generator.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(LSTACK_DIR)lstack.o:
	make lstack.o -C $(LSTACK_DIR)

$(SHM_DIR)libshm_alloc.$(DYLIB_EXT)::
	make -C $(SHM_DIR)

clean:
	rm *.dbgfl *.so *.out *.o .*.swp *.dylib shm_file

