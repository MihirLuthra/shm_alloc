
/*
 * Well if this crappy multithreaded lock free program needed debugging,
 * the functions here are really helpful.
 *
 * The main essence here is to use `pid_file`, which is a file named
 * after process' pid and thread's tid. Before using it, open_pid_file()
 * should be called.
 * The directory in which these files are created can be defined by macro
 * DEBUG_DIR whose value is a path to be enclosed in double quotes.
 * e.g., #define DEBUG_DIR "some/path".
 * If not defined, it defaults to PWD.
 *
 * TODO: override pthread_create() and main thread or whatever
 * so that when a new thread gets created, open_pid_file() is
 * automatically called.
 */

#if !defined(__SHM_DEBUG_H__)
#define __SHM_DEBUG_H__

#include <sys/param.h>
#include "shm_types.h"

extern __thread char pid_file_name[MAXPATHLEN];
extern __thread FILE *pid_file;

void open_pid_file();

#define BTS(boolean) ((boolean) ? "true" : "false")

#define PRINT_IN_PID_FILE(fmt, ...) \
	open_pid_file(); fprintf(pid_file, fmt, ##__VA_ARGS__);

#define PTPRINTF(format, ...) \
	printf("[%d,%ld]:" format, getpid(), (long)pthread_self(), ##__VA_ARGS__)

void print_buddy_bitmap(shm_bitmap [], FILE *) ;
void print_all_bits(shm_bitmap, FILE *);

void print_bmp_data(struct bmp_data_mgr, FILE *);
void print_mem_offt_data(struct mem_offt_mgr, FILE *);

#endif /* __SHM_DEBUG_H__ */
