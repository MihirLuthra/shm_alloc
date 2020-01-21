#include <assert.h>
#include "builtin_alternatives.h"
#include "shm_types.h"


int builtin_alternative_popcount(lock_free_int num)
{
	int retval;
	num = num - ((num >> 1) & (lock_free_int)~(lock_free_int)0/3);
	num = (num & (lock_free_int)~(lock_free_int)0/15*3) + ((num >> 2) & (lock_free_int)~(lock_free_int)0/15*3);
	num = (num + (num >> 4)) & (lock_free_int)~(lock_free_int)0/255*15;
	retval = (lock_free_int)(num * ((lock_free_int)~(lock_free_int)0/255)) >> (sizeof(lock_free_int) - 1) * 8;
	return (retval);
}

int builtin_alternative_clz(lock_free_int num)
{
	if (num == 0) {
		return 0;
	}

	for (lock_free_int pow2 = 1, i = 0 ; pow2 <= (sizeof(num)*8/2) ; (pow2 = 1 << i++)) {
		num |= (num >> pow2);
	}

	return (builtin_alternative_popcount(~num));
}

int builtin_alternative_ffsl(unsigned long num)
{
	int posn;

	if (num == 0)
		return (0);

	for (posn = 1 ; (num & 1) == 0 ; ++posn) {
		num = num >> 1;
	}

	return (posn);
}
