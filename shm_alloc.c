#include <sys/mman.h>
#include <sys/stat.h>

#include <assert.h>

#include "cas.h"

#include <fcntl.h>
#include <pthread.h>

#include "shm_alloc.h"
#include "shm_constants.h"
#include "shm_debug.h"
#include "shm_err.h"
#include "shm_types.h"
#include "shm_util_funcs.h"

#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint8_t *user_shm_base;

static _Atomic(struct shm_manager *) manager = NULL;


#define ACCESS_SHM_MGMT(offset) \
	((struct shm_block_mgmt *)((uint8_t *)manager->shm_mapping.base + get_shm_mgmt_base_offt() + (offset)))

#define ACCESS_SHM_DATA_TABLE(offset) \
	((struct shm_data_table *)((uint8_t *)manager->shm_mapping.base + get_shm_data_table_offt() + (offset)))

#define ACCESS_SHM_MGMT_BY_BITMAP_NO(bitmap_no) \
	ACCESS_SHM_MGMT((bitmap_no) * sizeof(struct shm_block_mgmt))

#define ACCESS_SHM_MAPPING(offset) \
	((void *)((uint8_t *)manager->shm_mapping.base + (offset)))

/*
 * shm base for user starts from offset to null.
 * Means if we return offset 0 to user, it is null.
 *
 * Allocatable region is followed by null region
 */
#define ACCESS_SHM_FOR_USER(offset) \
	((uint8_t *)manager->shm_mapping.base + get_shm_null_base_offt() + (offset))



/*
 * retval:
 *  If success returns `true` else `false`
 *
 * param1 :
 *  Memory required
 *
 * param2 :
 *  Pass address of a `struct bmp_data_mgr` which gets filled
 *  with the allocated bitmap data
 *
 * Description:
 *  The function iterates over the mgmt mapping area,
 *  checking the bitmaps for availability of the required memory.
 *  If a bitmap with desired memory is found, allocation is done
 *  by setting the bit and the bitmap data is filled in param2.
 */
bool search_all_bitmaps_for_mem(size_t, struct bmp_data_mgr *);

/*
 * retval:
 *  Returns the address of blk mgr that is stored in
 *  shm data table region.
 *
 * Description:
 *  This is the blk mgr that is supposed to have the first free
 *  memory not considering the ones freed by shm_free().
 *  Coming straight to it saves the unnecessary iterations
 *  to reach here.
 */
struct shm_block_mgmt * get_start_blk_mgr();

/*
 * retval:
 *  If the update was made, true is returned else false
 *
 * Description:
 *  Updates the start blk mgr if param1 is greater than
 *  previous value of address.
 *
 * param1:
 *  New blk mgr to be set.
 */
bool update_start_blk_mgr(struct shm_block_mgmt *);

/*
 * retval:
 *  Returns relative bit posn for memory level passed as param2
 *  if allocation is successfully performed in the buddy bitmap passed
 *  as param1.
 *  Returns -1 in case of failure.
 *
 * param1:
 *  Address of a `struct shm_block_mgmt` type in which allocation needs
 *  to be performed.
 *
 * param2:
 *  Amount of memory to be allocated
 */
int occupy_mem_in_bitmap(struct shm_block_mgmt *, size_t);

/*
 * retval:
 *  A `struct mem_offt_mgr` type is returned with all elements filled
 *  in accordance with param1.
 *
 * param1:
 *  A `struct bmp_data_mgr` type with all elements filled.
 *
 * Description:
 *  Evaluates the offsets in accordance with the passed bitmap data
 *  w.r.t the user's shm base.
 *
 */
struct mem_offt_mgr get_offset_for_user_by_bmp_data(struct bmp_data_mgr);

/*
 * retval:
 *  A `struct mem_offt_mgr` type is returned with all elements filled
 *  in accordance with param1.
 *
 * param1:
 *  The net offset that was returned to the user.
 */
struct mem_offt_mgr convert_offset_to_mem_offt_mgr(shm_offt);

/*
 * retval:
 *  A `struct bmp_data_mgr` type is returned with all elements filled
 *  in accordance with param1.
 *
 * param1:
 *  A `struct mem_offt_mgr` type with all elements filled.
 *
 * Description:
 *  Evaluates the bitmap data in accordance with the passed offset data.
 */
struct bmp_data_mgr get_bmp_data_by_mem_offt_data(struct mem_offt_mgr);

/*
 * retval:
 *  Returns a `struct blk_hdr` loaded with header info
 *
 * param1:
 *  An offset allocated by shm_(m|c)alloc.
 *
 * NOTE:
 *  The size stored in the header includes
 *  the size occupied by header aswell.
 */
struct blk_hdr get_blk_hdr(shm_offt);

/*
 * param1:
 *  Offset as per the user.
 *
 * param2:
 *  The value to be stored in header.
 *
 * Description:
 *  Sets the value in the header area of allocated
 *  memory to the value of param2
 */
void set_blk_hdr(shm_offt, struct blk_hdr);

/*
 * retval:
 *  Returns `true` of parent bits of param2 are set
 *  else returns `false`.
 *
 * param1:
 *  The buddy bitmap to be checked.
 *
 * param2:
 *  Relative bit posn of the bit whose parents are to be checked.
 *
 * param3:
 *  Memory level of the bit.
 *
 * param4:
 *  Address of a `size_t` type which gets filled with the value of
 *  parent's memory level IF parent is set.
 */
bool check_if_parents_are_set(shm_bitmap [], int, size_t, size_t *);

/*
 * retval:
 *  Returns `true` of child bits of param2 are set
 *  else returns `false`.
 *
 * param1:
 *  The buddy bitmap to be checked.
 *
 * param2:
 *  Relative bit posn of the bit whose children are to be checked.
 *
 * param3:
 *  Memory level of the bit.
 */
bool check_if_children_are_set(shm_bitmap [], int, size_t);

/*
 * This is the constructor function which does the setting up
 * of shared mappings.
 *
 * It would first create/open a file that is to be used as
 * the shared memory, ftruncate(2) it to desired size(which also fills the file with zeros)
 * and mmap(2) the file into the current process.
 * Note:
 *  This function is only called before main() as per its attribute
 *  and child processes copy the mappings created by this function.
 *  In case if `__attribute__((constructor))` isn't usable, this function can handle
 *  being called within multiple processes/threads.
 */
void __attribute__((constructor)) shm_init(void)
{
	if (manager != NULL) {
		return;
	}

	assert(get_shm_min_allocatable_size() < get_shm_max_allocatable_size());

	int err;
	struct stat st;

	/*
	 * In case of failures, remember to set manager = NULL
	 * because that's the way it will be detected in (m|c)alloc
	 * code
	 */
	manager = malloc(sizeof(struct shm_manager));
	if (manager == NULL) {
		P_ERR("malloc(2) failed");
		return;
	}

	shm_null_init();

	manager->shm_file.name = getenv(SHM_PATH_ENV_NAME);

	if (manager->shm_file.name == NULL) {

		P_ERR("getenv(3) failed for \"%s\"", SHM_PATH_ENV_NAME);
		exit(EXIT_FAILURE);
	}

	/* would create file if doesn't exist yet */
	manager->shm_file.fd = open(manager->shm_file.name, O_CREAT | O_RDWR, 0666);

	if (manager->shm_file.fd == -1) {
		P_ERR("open(2) failed");
		free(manager);
		manager = NULL;
		return;
	}

	err = fstat(manager->shm_file.fd, &st);

	if (err == -1) {
		P_ERR("fstat(2) failed");
		free(manager);
		manager = NULL;
		return;
	}

	manager->shm_file.size = st.st_size;
	manager->shm_mapping.size = get_shm_mapping_size();

	if (manager->shm_file.size == 0) {

		err = ftruncate(manager->shm_file.fd, manager->shm_mapping.size);

		if (err == -1) {
			free(manager);
			manager = NULL;
			P_ERR("ftruncate(2) failed");
			return;
		}

		 manager->shm_file.size = manager->shm_mapping.size;
	}

	manager->shm_mapping.base =
	    mmap(NULL, manager->shm_file.size, PROT_READ | PROT_WRITE, MAP_SHARED,
	    manager->shm_file.fd, 0);

	if (manager->shm_mapping.base == MAP_FAILED) {
		free(manager);
		manager = NULL;
		P_ERR("mmap(2) failed");
		return;
	}

	/*
	 * Make the region called as shm null readonly
	 * This would make it easier to debug such that
	 * if null region is accessed, error is generated
	 * saying readonly memory accessed
	 */
	void * shm_null_base = ACCESS_SHM_MAPPING(get_shm_null_base_offt());
	const size_t shm_null_size = get_shm_null_size();

	if (mmap(shm_null_base, shm_null_size, PROT_READ, MAP_SHARED | MAP_FIXED,
	    manager->shm_file.fd, 0) == MAP_FAILED) {

		P_ERR("mmap(2) failed while setting shm null");
		free(manager);
		manager = NULL;
		return;
	}

	/*
	 * As shm null exists in the allocatable region,
	 * we need to set it manager to indicate that its completely
	 * used
	 */
	int num_mgrs;
	struct shm_block_mgmt *null_blk_mgr;

	num_mgrs = shm_null_size/MAX_ALLOCATABLE_SIZE + (shm_null_size % MAX_ALLOCATABLE_SIZE > 0 ? 1 : 0);

	for (int mgr = 0 ; mgr < num_mgrs ; ++mgr) {

		null_blk_mgr = ACCESS_SHM_MGMT_BY_BITMAP_NO(mgr);

		(void)set_bit_race_free(null_blk_mgr->mgmt_bmp, get_start_bit_pos_for_mem_level(MAX_ALLOCATABLE_SIZE));
		atomic_store(&null_blk_mgr->mem_used, MAX_ALLOCATABLE_SIZE);
	}

	user_shm_base = ACCESS_SHM_FOR_USER(0);
}

size_t get_shm_max_allocatable_size()
{
	return (MAX_ALLOCATABLE_SIZE - sizeof(struct blk_hdr));
}

size_t get_shm_min_allocatable_size()
{
	return (MIN_ALLOCATABLE_SIZE);
}

void * get_shm_user_base(void)
{
	if (manager == NULL) {
		P_ERR("manager == NULL");
		return (NULL);
	}
	return (ACCESS_SHM_FOR_USER(0));
}

shm_offt shm_malloc(size_t size)
{
	if (manager == NULL) {
		P_ERR("manager == NULL");
		return (SHM_NULL);
	}

	size_t reqd_size;
	struct blk_hdr allocated_blk_hdr;
	struct bmp_data_mgr allocated_bmp_data;
	struct mem_offt_mgr mem_offt_data;
	bool found;
	shm_offt allocated_offset;

	/* evaluate reqd mem at least equal to MIN_ALLOCATABLE_SIZE */
	reqd_size = get_next_power_of_two(size + sizeof(allocated_blk_hdr));

	if (reqd_size < MIN_ALLOCATABLE_SIZE)
		reqd_size = MIN_ALLOCATABLE_SIZE;

	if (reqd_size > MAX_ALLOCATABLE_SIZE) {
		P_ERR("Can't allocate %zu bytes, MAX_ALLOCATABLE_SIZE = %zu", reqd_size, MAX_ALLOCATABLE_SIZE);
		return (SHM_NULL);
	}

	/*
	 * if memory is available, `allocated_bitmap`
	 * gets filled with the bitmap pos and bit pos
	 */
	found = search_all_bitmaps_for_mem(reqd_size, &allocated_bmp_data);

	if (found) {

		/* get offset to allocated memory */
		mem_offt_data = get_offset_for_user_by_bmp_data(allocated_bmp_data);

		allocated_offset = mem_offt_data.offt_to_allocated_mem;

		/* user's offset is memory after the header */
		allocated_offset += sizeof(allocated_blk_hdr);

		allocated_blk_hdr.mem = reqd_size;

		set_blk_hdr(allocated_offset, allocated_blk_hdr);

	} else {
		P_ERR("Out of memory");
		allocated_offset = SHM_NULL;
	}

	return (allocated_offset);
}

shm_offt shm_calloc(size_t cnt, size_t size)
{
	if (manager == NULL) {
		P_ERR("manager == NULL");
		return (SHM_NULL);
	}

	shm_offt allocated_offt;
	struct blk_hdr allocated_blk_hdr;

	allocated_offt = shm_malloc(cnt * size);

	if (allocated_offt == SHM_NULL)
		return (SHM_NULL);

	allocated_blk_hdr = get_blk_hdr(allocated_offt);

	memset((void *)ACCESS_SHM_FOR_USER(allocated_offt), 0, (allocated_blk_hdr.mem - sizeof(allocated_blk_hdr)));

	return (allocated_offt);
}

void shm_free(shm_offt shm_ptr)
{
	struct bmp_data_mgr bmp_data;
	struct shm_block_mgmt *blk_mgr;
	struct mem_offt_mgr mem_offt_data;
	bool did_unset;

	mem_offt_data = convert_offset_to_mem_offt_mgr(shm_ptr);

	bmp_data = get_bmp_data_by_mem_offt_data(mem_offt_data);

	blk_mgr = ACCESS_SHM_MGMT_BY_BITMAP_NO(bmp_data.bitmap_no);

	did_unset = unset_bit_race_free(blk_mgr->mgmt_bmp, bmp_data.abs_bit_pos);
	assert(did_unset);

	atomic_fetch_sub(&blk_mgr->mem_used, bmp_data.mem_level);
}

struct blk_hdr get_blk_hdr(shm_offt offset)
{
	return *((struct blk_hdr *)ACCESS_SHM_FOR_USER(offset) - 1);
}

void set_blk_hdr(shm_offt offset, struct blk_hdr hdr)
{
	struct blk_hdr *temp_hdr;
	temp_hdr = ((struct blk_hdr *)ACCESS_SHM_FOR_USER(offset) - 1);
	*temp_hdr = hdr;
}

struct mem_offt_mgr convert_offset_to_mem_offt_mgr(shm_offt offset)
{
	struct mem_offt_mgr mem_offt_data;
	struct blk_hdr hdr;

	hdr = get_blk_hdr(offset);
	mem_offt_data.mem = hdr.mem;

	/* subtract size of header*/
	offset -= sizeof(hdr);

	mem_offt_data.offt_to_allocated_mem = offset;

	mem_offt_data.internal_offt = offset % MAX_ALLOCATABLE_SIZE;
	mem_offt_data.offt_to_blk   = offset - mem_offt_data.internal_offt;

	return (mem_offt_data);
}

struct bmp_data_mgr get_bmp_data_by_mem_offt_data(struct mem_offt_mgr mem_offt_data)
{
	struct bmp_data_mgr bmp_data;

	bmp_data.mem_level = mem_offt_data.mem;

	bmp_data.bitmap_no = mem_offt_data.offt_to_blk/MAX_ALLOCATABLE_SIZE;
	bmp_data.relative_bit_pos = mem_offt_data.internal_offt/mem_offt_data.mem;
	bmp_data.abs_bit_pos = get_abs_bit_pos(bmp_data.relative_bit_pos, mem_offt_data.mem);

	return (bmp_data);
}

struct mem_offt_mgr get_offset_for_user_by_bmp_data(struct bmp_data_mgr bmp_data)
{
	struct mem_offt_mgr mem_offt_data;

	mem_offt_data.mem = bmp_data.mem_level;

	assert(bmp_data.relative_bit_pos >= 0);

	/* shm base for user starts from shm null followed by allocatable shm region */
	mem_offt_data.offt_to_blk   = MAX_ALLOCATABLE_SIZE * bmp_data.bitmap_no;
	mem_offt_data.internal_offt = bmp_data.relative_bit_pos * bmp_data.mem_level;

	mem_offt_data.offt_to_allocated_mem = mem_offt_data.offt_to_blk + mem_offt_data.internal_offt;

	return (mem_offt_data);
}

bool search_all_bitmaps_for_mem(size_t mem, struct bmp_data_mgr *bmp_data)
{
	struct shm_block_mgmt *cur_blk_mgr, *first_blk_mgr, *last_blk_mgr, *start_blk_mgr, *end_blk_mgr;
	int relative_bit_pos;
	bool did_find;

	first_blk_mgr  = ACCESS_SHM_MGMT(0);
	last_blk_mgr   = ACCESS_SHM_MGMT(SHM_MGMT_SIZE) - 1;

	bmp_data->bitmap_no        = -1;
	bmp_data->relative_bit_pos = -1;
	bmp_data->abs_bit_pos      = -1;
	bmp_data->mem_level        = mem;

	did_find = false;

	for (int scans = 0 ; scans < 2 ; ++scans) {

		if (scans == 0) {
			start_blk_mgr  = get_start_blk_mgr();
			end_blk_mgr    = last_blk_mgr;
		} else if (scans == 1) {
			end_blk_mgr    = start_blk_mgr - 1;
			start_blk_mgr  = first_blk_mgr;
		}

		for (cur_blk_mgr = start_blk_mgr ; cur_blk_mgr <= end_blk_mgr ; ++cur_blk_mgr) {

			if (atomic_load(&cur_blk_mgr->mem_used) + mem > MAX_ALLOCATABLE_SIZE) {
				continue;
			}

			relative_bit_pos = occupy_mem_in_bitmap(cur_blk_mgr, mem);

			if (relative_bit_pos != -1) {

				size_t mem_used;

				bmp_data->bitmap_no = (cur_blk_mgr - first_blk_mgr);
				bmp_data->relative_bit_pos = relative_bit_pos;
				bmp_data->abs_bit_pos = get_abs_bit_pos(relative_bit_pos, mem);

				mem_used = atomic_fetch_add(&cur_blk_mgr->mem_used, mem);

				if (mem_used + mem == MAX_ALLOCATABLE_SIZE && scans == 0) {
					/* Because this blk mgr is full, set start to next one */
					(void)update_start_blk_mgr(cur_blk_mgr + 1);
				}

				did_find = true;
				break;
			}
		}

		if (did_find) {
			break;
		}
	}


	return (did_find);
}

struct shm_block_mgmt * get_start_blk_mgr()
{
	struct shm_data_table * data_table;
	shm_offt start_blk_mgr_offt;

	data_table = ACCESS_SHM_DATA_TABLE(0);
	start_blk_mgr_offt = atomic_load(&data_table->start_blk_mgr_offt);

	return (ACCESS_SHM_MGMT(start_blk_mgr_offt));
}

bool update_start_blk_mgr(struct shm_block_mgmt * new_blk_mgr)
{
	shm_offt old, new;

	struct shm_data_table * data_table;
	data_table = ACCESS_SHM_DATA_TABLE(0);

	do {
		old = atomic_load(&data_table->start_blk_mgr_offt);
		new = (new_blk_mgr - ACCESS_SHM_MGMT(0)) * sizeof(struct shm_block_mgmt);

		if (new < old) {
			return (false);
		}

	}while(!CAS(&data_table->start_blk_mgr_offt, &old, new));

	return (true);
}

bool check_if_children_are_set(shm_bitmap bmp[], int relative_bit_pos, size_t mem_level)
{
	assert(relative_bit_pos >= 0 && relative_bit_pos < get_bit_cnt_for_mem_level(mem_level));

	int children_cnt, cur_lv_rel_bit_pos, abs_bit_pos;

	children_cnt = 2;

	while ((mem_level / children_cnt) >= MIN_ALLOCATABLE_SIZE) {

		cur_lv_rel_bit_pos = normalize_bit_pos_for_level(relative_bit_pos, mem_level, mem_level / children_cnt);

		abs_bit_pos = get_abs_bit_pos(cur_lv_rel_bit_pos, mem_level / children_cnt);

		if (is_bit_range_zero(bmp, abs_bit_pos, abs_bit_pos + children_cnt) == false)
			return true;


		children_cnt *= 2;
	}

	return false;
}

bool check_if_parents_are_set(shm_bitmap bmp[], int relative_bit_pos, size_t mem_level, size_t *parent_mem_level)
{
	assert(relative_bit_pos >= 0 && relative_bit_pos < get_bit_cnt_for_mem_level(mem_level));

	int cur_lv_rel_bit_pos, abs_bit_pos;

	int mul = MAX_ALLOCATABLE_SIZE/mem_level;

	while ((mem_level * mul) > mem_level) {

		cur_lv_rel_bit_pos = normalize_bit_pos_for_level(relative_bit_pos, mem_level, mem_level * mul);

		abs_bit_pos = get_abs_bit_pos(cur_lv_rel_bit_pos, mem_level * mul);

		if (is_bit_set(bmp, abs_bit_pos) == true) {
			*parent_mem_level = mem_level * mul;
			return true;
		}

		mul /= 2;
	}

	return (false);
}

int occupy_mem_in_bitmap(struct shm_block_mgmt *blk_mgr, size_t mem)
{
	MEM_RANGE_ASSERT(mem);

	int first_set_bit, rel_pos_first_set_bit, rel_bit_pos;
	bool did_set_bit, are_children_set, are_parents_set, did_unset;
	size_t parent_mem_lv;

	shm_bitmap mask[BMP_ARR_SIZE];

	get_settable_bits_mask((shm_bitmap *)blk_mgr->mgmt_bmp, mem, mask);

	rel_bit_pos = -1;
	first_set_bit = -1;

	do {

		first_set_bit = BITMAP_SIZE - (shm_bitmap_ffs(mask) ? : BITMAP_SIZE + 1);

		/* memory not available in this bitmap */
		if (first_set_bit == -1)
			break;

		did_set_bit = set_bit_race_free(blk_mgr->mgmt_bmp, first_set_bit);

		if (did_set_bit == false) {
			unset_bit(mask, first_set_bit);
			first_set_bit = -1;
			continue;
		}

		rel_pos_first_set_bit = get_rel_bit_pos(first_set_bit);

		are_parents_set =
		    check_if_parents_are_set((shm_bitmap *)blk_mgr->mgmt_bmp, rel_pos_first_set_bit, mem, &parent_mem_lv);

		if (are_parents_set == true) {
			int nbpos = normalize_bit_pos_for_level(rel_pos_first_set_bit, mem, parent_mem_lv);
			nbpos = normalize_bit_pos_for_level(nbpos, parent_mem_lv, mem);
			nbpos = get_abs_bit_pos(nbpos, mem);
			unset_bits_in_range(mask, nbpos, nbpos + parent_mem_lv/mem);

			did_unset = unset_bit_race_free(blk_mgr->mgmt_bmp, first_set_bit);
			assert(did_unset);

			continue;
		}

		are_children_set = check_if_children_are_set((shm_bitmap *)blk_mgr->mgmt_bmp, rel_pos_first_set_bit, mem);


		if (are_children_set == true) {
			unset_bit(mask, first_set_bit);
			did_unset = unset_bit_race_free(blk_mgr->mgmt_bmp, first_set_bit);
			assert(did_unset);

			continue;
		}

		/* reaching here means bit setting is done, memory occupied */
		rel_bit_pos = rel_pos_first_set_bit;

	} while (rel_bit_pos == -1);

	return (rel_bit_pos);
}
