SHM_DIR := ../

CC := gcc
CFLAGS  := -D TEST_THE_TEST=0 -Wall -Wextra -fsanitize=address -I$(SHM_DIR)

test_shm_alloc.out: test_shm_alloc.c $(SHM_DIR)libshm_alloc.so rand_string_generator.o
	cp $(SHM_DIR)libshm_alloc.so .
	$(CC) $(CFLAGS) -o test_shm_alloc.out test_shm_alloc.c rand_string_generator.o -L. -lshm_alloc

rand_string_generator.o: rand_string_generator.c rand_string_generator.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(SHM_DIR)libshm_alloc.so:
	make -C $(SHM_DIR)

clean:
	rm *.dbgfl *.so *.out *.o .*.swp shm_file

