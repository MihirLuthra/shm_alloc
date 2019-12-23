SHM_DIR := ..

include $(SHM_DIR)/Makefile

CC := gcc

CFLAGS  := -D TEST_THE_TEST=0 -Wall -Wextra -fsanitize=address -I$(SHM_DIR)

.DEFAULT_GOAL = test_shm_alloc.out

test_shm_alloc.out: test_shm_alloc.c libshm_alloc.so rand_string_generator.o
	$(CC) $(CFLAGS) -o test_shm_alloc.out test_shm_alloc.c rand_string_generator.o -L. -lshm_alloc

rand_string_generator.o: rand_string_generator.c rand_string_generator.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.dbgfl *.so *.out *.o .*.swp shm_file

