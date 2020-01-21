#include "shm_alloc.h"
#include <stdlib.h>
#include <string.h>

/* same as ptr_calloc() */
void *my_calloc(size_t count, size_t size)
{
	shm_offt offt_in_shm;

	offt_in_shm = shm_calloc(count, size);

	if (offt_in_shm == SHM_NULL) {
		return (NULL);
	}

	return ((uint8_t *)get_shm_user_base() + offt_in_shm);
}

/* same as ptr_free() */
void my_free(void *ptr)
{
	shm_offt offt_in_shm;

	if (ptr == NULL) {
		offt_in_shm = SHM_NULL;
	} else {
		offt_in_shm = SHM_ADDR_TO_OFFT(ptr);
	}

	shm_free(offt_in_shm);
}

int main()
{
	/*
	 * Initialize shm
	 */
	shm_init(NULL);

	char *str;
	size_t string_len = 100;

	str = my_calloc(string_len, sizeof(char));

	if (str == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	strcpy(str, "My test string!");

	printf("%s\n", str);

	/* free from the shared memory */
	my_free(str);

	/* release resources held by shared memory */
	shm_deinit();

	return 0;
}
