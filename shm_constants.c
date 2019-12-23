#include <assert.h>

#include "shm_constants.h"
#include "shm_types.h"

#include <unistd.h>

/*
 * Access offset to shm null via
 * get_shm_null_base_offt() and size of shm null by
 * get_shm_null_size()
 *
 * shm_null is set in shm_null_init()
 *
 * shm_null.size is initially -1 so that if
 * get_shm_null_base_offt() is used before shm_null_init()
 * error gets reported
 *
 */
static struct {
	shm_offt offt;
	ssize_t  size;
} shm_null = {.size = -1};

void shm_null_init()
{
	/* shm null comes after MGMT area */
	shm_null.offt = get_shm_mgmt_base_offt() + get_shm_mgmt_size();

	/* align it to page boundary */
	shm_null.offt += DIFF_NEXT_PAGE_BOUNDARY(shm_null.offt);

	shm_null.size = getpagesize();
}

size_t get_shm_mapping_size()
{
	assert(shm_null.size != -1);

	/*
	 * mapping comprises of 3 parts:
	 *
	 * (1)
	 *     shm data table
	 *
	 * (2)
	 *     management area of size SHM_MGMT_SIZE
	 *
	 * (3)
	 *     shm null of pagesize
	 *
	 * (4)
	 *     allocatable area of shm of size MAX_ALLOCATABLE_SHM_SIZE
	 *
	 * NOTE:
	 *  The base returned to the user starts from shm null offset
	 */
	size_t mapping_size = 0;

	mapping_size += get_shm_data_table_size();
	mapping_size = get_shm_mgmt_size();

	mapping_size += DIFF_NEXT_PAGE_BOUNDARY(mapping_size);

	mapping_size += shm_null.size;
	mapping_size += MAX_ALLOCATABLE_SHM_SIZE;

	return (mapping_size);
}

shm_offt get_shm_data_table_offt()
{
	return (0);
}

shm_offt get_shm_data_table_size()
{
	return (sizeof(struct shm_data_table));
}

shm_offt get_shm_mgmt_base_offt()
{
	return (get_shm_data_table_offt() + get_shm_data_table_size());
}

shm_offt get_shm_mgmt_size()
{
	return (SHM_MGMT_SIZE);
}

shm_offt get_shm_null_base_offt()
{
	assert(shm_null.size != -1);
	return (shm_null.offt);
}

shm_offt get_shm_user_base_offt()
{
	assert(shm_null.size != -1);
	return (shm_null.offt);
}

size_t get_shm_null_size()
{
	assert(shm_null.size != -1);
	return (shm_null.size);
}

shm_offt get_allocatable_shm_base_offt()
{
	assert(shm_null.size != -1);

	shm_offt ret_offt = get_shm_null_base_offt() + get_shm_null_size();

	return (ret_offt);
}
