#if !defined(__SHM_MODULE_TEST_H__)
#define __SHM_MODULE_TEST_H__

#include "shm_alloc.h"
#include "shm_constants.h"
#include "shm_debug.h"
#include "shm_err.h"
#include "shm_util_funcs.h"
#include "shm_types.h"

struct mem_offt_mgr convert_offset_to_mem_offt_mgr(shm_offt);
struct bmp_data_mgr get_bmp_data_by_mem_offt_data(struct mem_offt_mgr);
struct mem_offt_mgr get_offset_for_user_by_bmp_data(struct bmp_data_mgr);
bool search_all_bitmaps_for_mem(size_t, struct bmp_data_mgr *);
bool check_if_children_are_set(shm_bitmap [], int, size_t);
bool check_if_parents_are_set(shm_bitmap [], int, size_t, size_t *);
int occupy_mem_in_bitmap( struct shm_block_mgmt *, size_t);

#endif /* !defined(__SHM_MODULE_TEST_H__) */
