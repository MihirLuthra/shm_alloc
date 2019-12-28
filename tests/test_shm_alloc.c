#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>

#if defined __has_include
#	if __has_include (<libkern/OSAtomicQueue.h>)
#		include <libkern/OSAtomicQueue.h>
#	define HAVE_OSATOMICQUEUE
#	endif
#endif

#if !defined(HAVE_OSATOMICQUEUE)
#	include "lstack.h"
#endif

#include <pthread.h>

#include "rand_string_generator.h"
#include "shm_alloc.h"

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

#if defined(HAVE_OSATOMICQUEUE)
OSFifoQueueHead queue = OS_ATOMIC_FIFO_QUEUE_INIT;
#else
lstack_t stack;
#endif

struct inserted_data_mgr {
	PTR(char) in_shm;
	int       idx;

#if defined(HAVE_OSATOMICQUEUE)
	struct inserted_data_mgr * link;
#endif
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

#if !defined(HAVE_OSATOMICQUEUE)
	if (lstack_init(&stack, max_idx) != 0) {
		P_ERR("lstack_init() failed");
		exit(EXIT_FAILURE);
	}
#endif

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

#if !defined(HAVE_OSATOMICQUEUE)
	lstack_free(&stack);
#endif

	free_rand_strings(rand_strings, max_idx);
	shm_deinit();

	for (int i = 0 ; i < thread_count ; ++i)
		free(test_results[i]);

	free(test_results);

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

	free(thrds);

	return (test_results);
}


void *tester_func(void *arg)
{
	struct test_results_mgr * test_result = arg;

	struct inserted_data_mgr *data;

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

		data = malloc(sizeof(struct inserted_data_mgr));

		if (data == NULL) {
			P_ERR("malloc(2) failed");
			exit(EXIT_FAILURE);
		}

		data->idx    = idx;
		data->in_shm = str;

#if defined(HAVE_OSATOMICQUEUE)
		OSAtomicFifoEnqueue(&queue, data, offsetof(struct inserted_data_mgr, link));
#else
		if (lstack_push(&stack, data) != 0) {
			P_ERR("lstack_push(2) failed");
			exit(EXIT_FAILURE);
		}
#endif

		sleep(0);

#if defined(HAVE_OSATOMICQUEUE)
		data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));
#else
		data = lstack_pop(&stack);
#endif
		/*
		 * The data being popped/dequed maybe the same that was pushed/enqued
		 * or different if thread got changed. In any case, even if the same data
		 * is got back, below data is checked for correctness and then if
		 * a randomly generated number is divisible by 3, only then this data is freed,
		 * otherwise its pushed/enqued again for getting checked again.
		 */

		if (data == NULL) {
			continue;
		}

		idx = data->idx;
		str = data->in_shm;

		if (strcmp(ACCESS(str, char) , rand_strings[idx])) {

			test_result->status = false;
			break;

		} else {

			/*
			 * Just to make it more random,
			 * generate a random number and if its divisible by 3,
			 * shm_free() the dequed offset else enqueue it again
			 */

			if (rand() % 3 == 0) {
				shm_free(str);
				free(data);
			} else {
#if defined(HAVE_OSATOMICQUEUE)
				OSAtomicFifoEnqueue(&queue, data, offsetof(struct inserted_data_mgr, link));
#else
				lstack_push(&stack, data);
#endif
			}
		}

	}

#if defined(HAVE_OSATOMICQUEUE)
		data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));
#else
		data = lstack_pop(&stack);
#endif

	while(data != NULL) {

		idx = data->idx;
		str = data->in_shm;

		if (strcmp(ACCESS(str, char), rand_strings[idx])) {

			test_result->status = false;
			break;

		} else {
			shm_free(str);
			free(data);
		}
#if defined(HAVE_OSATOMICQUEUE)
		data = OSAtomicFifoDequeue(&queue, offsetof(struct inserted_data_mgr, link));
#else
		data = lstack_pop(&stack);
#endif
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
