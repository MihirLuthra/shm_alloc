#if !defined(__SHM_CONSTANTS_H__)
#define __SHM_CONSTANTS_H__

#include "shm_user_types.h"

/*
 * Description:
 *  Sets shm null.
 *  get_shm_null_base_offt() and get_shm_null_size()
 *  should be used after this.
 */
void shm_null_init();
size_t get_shm_null_size();
shm_offt get_shm_null_base_offt();

/*
 * Description:
 *  Returns the offset from the mapping base,
 *  from where the allocatable region of shm starts.
 */
shm_offt get_allocatable_shm_base_offt();

/*
 * Description:
 *  Returns the offset from the mapping base,
 *  from where the mgmt region of shm starts.
 */
shm_offt get_shm_mgmt_base_offt();

/*
 * Description:
 *  Returns the offset from the mapping base,
 *  from where the shm data table region of shm starts.
 */
shm_offt get_shm_data_table_offt();

/*
 * Description:
 *  Returns shm data table size
 */
shm_offt get_shm_data_table_size();

/*
 * Description:
 *  Returns size of shm mgmt
 */
shm_offt get_shm_mgmt_size();

/*
 * Description:
 *  Returns the complete mapping size.
 */
size_t get_shm_mapping_size();

#define DIFF_NEXT_PAGE_BOUNDARY(offset) (getpagesize() - ((offset) % getpagesize() ? : getpagesize()))

/*
 * This branch uses a single shm_bitmap variable to implement
 * buddy system.  (MAX_ALLOC_POW2 - MIN_ALLOC_POW2 <= 5) should hold true
 * if bitmap is of 64 bits.
 * If bitmap is of 32 bits, (MAX_ALLOC_POW2 - MIN_ALLOC_POW2 <= 4)
 */
#if !defined(MAX_ALLOC_POW2)
#	define MAX_ALLOC_POW2 (10) /* 1UL << 10 = 1024   */
#endif

#if !defined(MIN_ALLOC_POW2)
#	if defined(SHM_USE_LONG)
#		define MIN_ALLOC_POW2 (5)  /* 1UL << 5  = 32     */
#	else
#		define MIN_ALLOC_POW2 (6)  /* 1UL << 6  = 64     */
#	endif
#endif


/*
 * The size of mapping that is for allocating.
 * This need not necessarily be power of 2.
 * 1UL << 28 = 256 MB
 */
#if !defined(MAX_ALLOCATABLE_SHM_SIZE)
#	define MAX_ALLOCATABLE_SHM_SIZE (size_t)(1UL << (8+10+10))
#endif /* !defined(SHM_CUSTOM_ALLOCATABLE_SIZE) */

/*
 * The environment variable that is supposed to contain
 * the name of shm file
 */
#if !defined(SHM_PATH_ENV_NAME)
#	define SHM_PATH_ENV_NAME "SHM_FILE"
#endif

/*
 * Maximum and minumum sizes that can be allocated
 * via shm_(m|c)alloc
 * Should always be a power of 2, that why they are to be set via
 * M(AX|IN)_ALLOC_POW2
 */
#define MAX_ALLOCATABLE_SIZE (size_t)(1UL << MAX_ALLOC_POW2)
#define MIN_ALLOCATABLE_SIZE (size_t)(1UL << MIN_ALLOC_POW2)

#define BITMAP_SIZE  (int)((MAX_ALLOCATABLE_SIZE/MIN_ALLOCATABLE_SIZE) * 2)

#define SHM_MGMT_SIZE ((MAX_ALLOCATABLE_SHM_SIZE/MAX_ALLOCATABLE_SIZE) * (sizeof(struct shm_block_mgmt)))


#endif /* !defined(__SHM_CONSTANTS_H__) */
