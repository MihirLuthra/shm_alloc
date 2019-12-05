#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>

#include "shm_alloc.h"
#include "shm_debug.h"
#include "shm_err.h"
#include "shm_constants.h"

#include <errno.h>
#include <fcntl.h>
#include <libkern/OSAtomic.h>
#include <pthread.h>
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

#define PTR(type)             shm_offt
#define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))

char **strings;
long max_idx;
_Atomic(int) cur_idx = 0;

OSQueueHead queue = OS_ATOMIC_QUEUE_INIT;

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


//PROTO

void generate_processes(int process_cnt);

char *  generate_rand_str(unsigned long long min_len, unsigned long long max_len);

char ** generate_rand_arr_of_strs(unsigned long long num_str, unsigned long long min_len,
    unsigned long long max_len);

struct test_results_mgr ** spawn_threads_for_test(int thrd_cnt, void *(*tester_func)(void *));

void *tester_func(void *arg);

void display_test_results(struct test_results_mgr **test_results_per_thrd, int thrd_cnt);


int main(int argc, char *argv[])
{
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

	strings = generate_rand_arr_of_strs(max_idx, 1, 300);

	if (strings == NULL) {
		P_ERR("generate_rand_arr_of_strs() failed");
		exit(EXIT_FAILURE);
	}

	struct test_results_mgr **test_results;
	test_results = spawn_threads_for_test(thread_count, tester_func);

	display_test_results(test_results, thread_count);

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

char * generate_rand_str(unsigned long long min_len, unsigned long long max_len)
{
	char *str;
	size_t str_len;

	str_len = rand() % (max_len - min_len + 1) + min_len;

	str = malloc(sizeof(char) * (str_len));

	if (str == NULL) {
		P_ERR("malloc(2) failed");
		exit(EXIT_FAILURE);
	}

	unsigned long long i;
	for (i = 0 ; i < (str_len-1) ; ++i)
		str[i] = rand() % (91 - 65 + 1) + 65;

	str[i] = '\0';

	return str;
}


char ** generate_rand_arr_of_strs(unsigned long long num_str, unsigned long long min_len,
    unsigned long long max_len)
{
	char **str_array = malloc(sizeof(char *) * num_str);
	
	if (str_array == NULL) {
		P_ERR("malloc(2) failed");
		exit(EXIT_FAILURE);
	}

	for (unsigned long long i = 0 ; i < num_str ; ++i)
		str_array[i] = generate_rand_str(min_len, max_len);

	return str_array;
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

	return test_results;
}


void *tester_func(void *arg)
{
	struct test_results_mgr * test_result = arg;

	struct inserted_data_mgr inserted_data, *dequed_data;

	PTR(char) str;
	int idx;

	test_result->status = true;
	
	while ((idx = atomic_fetch_add(&cur_idx, 1)) < max_idx) {
	

		str = shm_calloc(1, strlen(strings[idx]) + 1);

		//PTPRINTF("mem = %zu, mem allocated = %zu, offt = %zu and idx = %d\n", strlen(strings[idx]) + 1, *(ACCESS(str, size_t) - 1), str, idx);

		if (str == SHM_NULL) {
			P_ERR("shm_malloc() : out of memory\n");
			exit(EXIT_FAILURE);
		}

		strcpy(ACCESS(str, char), strings[idx]);

		inserted_data.idx     = idx;
		inserted_data.in_shm  = str;

		OSAtomicEnqueue(&queue, &inserted_data, offsetof(struct inserted_data_mgr, link));

		sleep(0);

		dequed_data = OSAtomicDequeue(&queue, offsetof(struct inserted_data_mgr, link));

		if (dequed_data == NULL)
			continue;

		idx = dequed_data->idx;
		str = dequed_data->in_shm;

		if (strcmp(ACCESS(str, char) , strings[idx])) {
			test_result->status = false;
			break;

		} else {
			shm_free(str);

		}
	
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
