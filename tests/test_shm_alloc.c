#include <assert.h>

#include "shm_alloc.h"

#include <errno.h>
#include <libkern/OSAtomicQueue.h>
#include <pthread.h>

#include "rand_string_generator.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KNRM  "\x1B[0m"

#define errfile stderr

#define P_ERR(format, ...) \
	fprintf(errfile, "FILE(%s) : FUNC(%s) : LINE(%d) : errno(%d -> %s): " format "\n", __FILE__, __func__, __LINE__, errno, strerror(errno), ##__VA_ARGS__)



#if TEST_THE_TEST

#	define PTR(type)             type *
#	define ACCESS(ptr, type)     ((type *)ptr)
#	define shm_malloc            malloc
#	define shm_calloc            calloc
#	define shm_free              free

#else

#	define PTR(type)             shm_offt
#	define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))

#endif

char **rand_strings;
long max_idx;
_Atomic(long) cur_idx = 0;

OSFifoQueueHead queue = OS_ATOMIC_FIFO_QUEUE_INIT;

struct inserted_data_mgr {
	PTR(char) in_shm;
	int       idx;

	struct inserted_data_mgr * link; 
};

struct test_results_mgr {
	size_t mem_used;
	long tid;
	bool status;
};


void   generate_processes(int process_cnt);
struct test_results_mgr ** spawn_threads_for_test(int thrd_cnt, void *(*tester_func)(void *));
void  *tester_func(void *arg);
void   display_test_results(struct test_results_mgr **test_results_per_thrd, int thrd_cnt);


int main(int argc, char *argv[])
{
#if TEST_THE_TEST
	printf("Testing the working of test with malloc(2), calloc(2) and free(2)\n");
#else
	printf("Max allocatable size by shm_(m|c)alloc() = %zu\n", get_shm_max_allocatable_size());
	printf("Min allocatable size by shm_(m|c)alloc() = %zu\n", get_shm_min_allocatable_size());
#endif

	if (argc != 4) {
		fprintf(stderr, "usage: %s process_count thread_count num_strings\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	errno = 0;
	const int process_count = (int)strtol(argv[1], (char **)NULL, 10);
	if (process_count <= 0) {
		if (errno != 0)
			P_ERR("Invalid process count");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	const int thread_count  = (int)strtol(argv[2], (char **)NULL, 10);
	if (thread_count <= 0) {
		if (errno != 0)
			P_ERR("Invalid thread count");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	max_idx = strtol(argv[3], (char **)NULL, 10);
	if (max_idx <= 0) {
		if (errno != 0)
			P_ERR("Invalid string count");
		exit(EXIT_FAILURE);
	}

	generate_processes(process_count);

	rand_strings =
	    generate_rand_arr_of_strs(max_idx, 1, get_shm_max_allocatable_size(), 65, 91);

	if (rand_strings == NULL) {
		P_ERR("generate_rand_arr_of_strs() failed");
		exit(EXIT_FAILURE);
	}

	struct test_results_mgr **test_results;
	test_results = spawn_threads_for_test(thread_count, tester_func);

	display_test_results(test_results, thread_count);

	free_rand_strings(rand_strings, max_idx);

	int status = 0;
	while (wait(&status) > 0);

	return 0;
}

void generate_processes(int process_cnt)
{
	for (int i = 0 ; i < process_cnt - 1 ; ++i)
		if (fork() == 0)
			break;
}


struct test_results_mgr ** spawn_threads_for_test(int thrd_cnt, void *(*tester_func)(void *))
{
	int i;
	struct test_results_mgr ** test_results;
	pthread_t * thrds;
	
	thrds = malloc(sizeof(pthread_t) * thrd_cnt);

	if (thrds == NULL) {
		P_ERR("malloc(2) failed");
		exit(EXIT_FAILURE);
	}

	test_results = malloc(sizeof(struct test_results_mgr *) * thrd_cnt);

	if (test_results == NULL) {
		P_ERR("malloc(2) failed");
		exit(EXIT_FAILURE);
	}

	for (i = 0 ; i < thrd_cnt ; ++i) {

		test_results[i] = malloc(sizeof(struct test_results_mgr));
		
		if (test_results[i] == NULL) {
			P_ERR("malloc(2) failed");
			exit(EXIT_FAILURE);
		}

		pthread_create(&thrds[i], NULL, tester_func, test_results[i]);
	}

	for (i = 0 ; i < thrd_cnt ; ++i)
		pthread_join(thrds[i], NULL);

	return (test_results);
}


void *tester_func(void *arg)
{
	struct test_results_mgr * test_result = arg;

	struct inserted_data_mgr *inserted_data, *dequed_data;

	PTR(char) str;
	int idx;

	test_result->status = true;
	
	while ((idx = atomic_fetch_add(&cur_idx, 1)) < max_idx) {
	
		str = shm_calloc(1, strlen(rand_strings[idx]) + 1);

		if (str == SHM_NULL) {
			P_ERR("shm_calloc() : out of memory\n");
			exit(EXIT_FAILURE);
		}

		strcpy(ACCESS(str, char), rand_strings[idx]);

		inserted_data = malloc(sizeof(struct inserted_data_mgr));

		if (inserted_data == NULL) {
			P_ERR("malloc(2) failed");
			exit(EXIT_FAILURE);
		}

		inserted_data->idx      = idx;
		inserted_data->in_shm   = str;

		OSAtomicFifoEnqueue(&queue, inserted_data, offsetof(struct inserted_data_mgr, link));

		sleep(0);

		dequed_data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));

		if (dequed_data == NULL) {
			continue;
		}

		idx = dequed_data->idx;
		str = dequed_data->in_shm;

		if (strcmp(ACCESS(str, char) , rand_strings[idx])) {

			test_result->status = false;
			break;

		} else {

			/*
			 * Just to make it more random,
			 * generate a random number and if its divisible by 2,
			 * shm_free() the dequed offset else enqueue it again
			 */

			if (rand() % 3 == 0) {
				shm_free(str);
				free(dequed_data);
			} else {
				OSAtomicFifoEnqueue(&queue, dequed_data, offsetof(struct inserted_data_mgr, link));
				
			}
		}
	
	}


	dequed_data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));

	while(dequed_data != NULL) {

		idx = dequed_data->idx;
		str = dequed_data->in_shm;

		if (strcmp(ACCESS(str, char) , rand_strings[idx])) {

			test_result->status = false;
			break;

		} else {
			shm_free(str);
			free(dequed_data);
		}

		dequed_data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));
	}

	test_result->tid = (long)pthread_self();

	return NULL;
}


void display_test_results(struct test_results_mgr **test_results, int thrd_cnt)
{
	for (int i = 0 ; i < thrd_cnt ; ++i) {
		if (test_results[i]->status == true) {
			fprintf(stderr, KGRN "[%d, %ld] : Test passed\n" KNRM, getpid(), test_results[i]->tid);
		} else {
			fprintf(stderr, KRED "[%d, %ld] : Test failed\n" KNRM, getpid(), test_results[i]->tid);
		}
	}
}
