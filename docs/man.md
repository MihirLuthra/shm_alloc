<h2>NAME</h2>

<strong>get_mapping_size_needed_by_shm, get_shm_max_allocatable_size, get_shm_min_allocatable_size, get_sizeof_block_header, get_shm_user_base, ptr_calloc, ptr_free, ptr_malloc, shm_calloc, shm_deinit, shm_free, shm_init, shm_malloc</strong> 
-- shared memory allocation

<br>

<h2>SYNOPSIS</h2>

<strong>#include "shm_alloc.h"</strong>

size_t<br>
<strong>get_mapping_size_needed_by_shm</strong>
(void);

size_t<br>
<strong>get_shm_max_allocatable_size</strong>
(void);

size_t<br>
<strong>get_shm_min_allocatable_size</strong>
(void);

size_t<br>
<strong>get_sizeof_block_header</strong>
(void);

void *<br>
<strong>get_shm_user_base</strong>
(void)

void *<br>
<strong>ptr_calloc</strong>
(size_t count, size_t size);

void<br>
<strong>ptr_free</strong>
(void *ptr);

void *<br>
<strong>ptr_malloc</strong>
(size_t size);

shm_offt<br>
<strong>shm_calloc</strong>
(size_t count, size_t size);

void<br>
<strong>shm_free</strong>
(shm_offt shm_ptr);

shm_offt<br>
<strong>shm_malloc</strong>
(size_t size);

bool<br>
<strong>shm_init</strong>
(void *optional_addr, const char *shm_filename);

void<br>
<strong>shm_deinit</strong>
(void);

<br>

<strong>SHM_OFFT_TO_ADDR</strong>
(offset)

<strong>SHM_ADDR_TO_OFFT</strong>
(address)

<br>

<h2>DESCRIPTION</h2>

<br>

<h4>get_mapping_size_needed_by_shm():</h4>

It returns the mapping space that would be required by the shared memory. It can be called before <code>shm_init()</code>.

<br>

<h4>get_shm_max_allocatable_size():</h4>

It returns the maximum size that can be allocated via <code>shm_malloc()</code>/<code>shm_calloc()</code>/<code>ptr_malloc()</code>/<code>ptr_calloc()</code>.
<strong>IMPORTANT</strong>: <code>shm_malloc()</code>/<code>shm_calloc()</code>/<code>ptr_malloc()</code>/<code>ptr_calloc()</code> allocate extra space for
the header as well which can be obtained by <code>get_sizeof_block_header()</code>.

<br>

<h4>get_shm_min_allocatable_size():</h4>

It returns the minimun size that will be allocated via <code>shm_malloc()</code>/<code>shm_calloc()</code>/<code>ptr_malloc()</code>/<code>ptr_calloc()</code>.
The implementation uses a buddy system for allocation, so generally the memory allocated is the next power of 2 of what was
requested. But if that next power of 2 is smaller than the min allocatable size, then memory allocated is equal to
min allocatable size.

<br>

<h4>get_sizeof_block_header():</h4>

Returns the extra size that is allocated by <code>shm_malloc()</code>/<code>shm_calloc()</code>/<code>ptr_malloc()</code>/<code>ptr_calloc()</code>
for the header.

<br>

<h4>get_shm_user_base():</h4>

Returns the shared memory base for the current process where the allocations are done. Its return value should be casted to
<code>uint8_t *</code> and added to an offset returned by <code>shm_malloc()</code> or <code>shm_calloc()</code> to access it.

<br>

<h4>shm_init():</h4>

Sets up the shared memory for the current process.

<ul>
	<li>
		<strong>param1</strong>: void *optional_addr<br>
		First argument to <code>shm_init()</code>, which is an optional address is used as first argument of <code>mmap(2)</code>
		with <code>MAP_FIXED</code>. If <code>NULL</code> is passed, <code>mmap(2)</code> chooses an address by itself(which is recommended).
		This maybe useful if all processes map the memory at the same location. Then you can use <code>ptr_malloc()</code>
		just as regular <code>malloc(2)</code> without getting into complexities.
		<em>Also</em>, it is NOT the same as what is returned by <code>get_shm_user_base()</code>.
	</li>
	<li>
		<strong>param2</strong>: const char *shm_filename<br>
		The filename of which is used as shared memory. if <code>NULL</code> is passed here, then environment variable
		<code>SHM_FILE</code> will be checked for filename.
	</li>
	<li>
		<strong>Thread safety and return value</strong>: It can be called in multiple threads.
		If calling it in multiple threads, it maybe possible that one thread returns <code>true</code>
		while the other returns <code>false</code>. If any one thread in the current process returned true, setup is done.
		A clean way to check for success in such cases would be to call <code>get_shm_user_base()</code> after <code>shm_init()</code> and if it returns <code>NULL</code>,
		shared memory initialization has failed.
	</li>
	<li>
		<strong>On fork()</strong>:
		No need to call it in <code>fork(2)</code>'d processes if <code>shm_init()</code> was called successfully in parent.
		But no harm if you do call it in children, it will simply return <code>true</code without doing anything.
	</li>
	<li>
		<strong>Signal Safety</strong>:
		It isn't <a href="http://man7.org/linux/man-pages/man7/signal-safety.7.html">aync-signal-safe</a>
		as it calls <code>mmap(2)</code>. Well if <code>mmap(2)</code> becomes signal safe in future,
		then this function is signal safe aswell.
	</li>
</ul>

<br>

<h4>shm_deinit():</h4>

It will release all recources held for shared memory for the current process.

<br>

<h4>shm_malloc():</h4>

Allocates memory in shared memory. Some extra memory is allocated for header aswell. That extra size can be obtained by <code>get_sizeof_block_header()</code>

<ul>
	<li>
		<strong>param1</strong>: size_t size<br>
		The amount of memory to be allocated. This should be less than or equal to the return value of <code>get_shm_max_allocatable_size()</code>.
		If it exceeds the max allocatable size, <code>SHM_NULL</code> is returned.
	</li>
	<li>
		To access the allocated memory, the returned offset needs to be added to the shared memory base for the process, which
		is retuned by <code>get_shm_user_base()</code>.
	</li>
</ul>

<br>

<h4>shm_calloc():</h4>

Allocates memory in shared memory and sets it to all zero. That extra size can be obtained by <code>get_sizeof_block_header()</code>

<ul>
	<li>
		<strong>params</strong>: size_t count, size_t size<br>
		<code>count * size</code> = The amount of memory to be allocated. This should be less than or equal to the return value of <code>get_shm_max_allocatable_size()</code>.
		If it exceeds the max allocatable size, <code>SHM_NULL</code> is returned.
	</li>
	<li>
		To access the allocated memory, the returned offset needs to be added to the shared memory base for the process, which
		is retuned by <code>get_shm_user_base()</code>.
	</li>
	<li>
		It sets the allocated memory to all zero via <code>memset(3)</code>.
	</li>
</ul>

<br>

<h4>shm_free():</h4>

Frees memory in shared memory.

<ul>
	<li>
		<strong>param1</strong>: shm_offt shm_ptr<br>
		param1 defines the offset of the memory in the shared memory that needs to be freed.
	</li>
</ul>


<h4>ptr_malloc():</h4>

Allocates memory in shared memory. Some extra memory is allocated for header aswell. That extra size can be obtained by <code>get_sizeof_block_header()</code>
It is just a wrapper around <code>shm_malloc()</code>.
It allocates via <code>shm_malloc()</code> in the shared memory, and returns SHM_OFFT_TO_ADDR(offset).
If <code>shm_malloc()</code> returns <code>SHM_NULL</code>, then <code>ptr_malloc()</code> returns <code>NULL</code>.

<br>

<h4>ptr_calloc():</h4>

Allocates memory in shared memory and sets it to all zero. That extra size can be obtained by <code>get_sizeof_block_header()</code>
It is just a wrapper around <code>shm_calloc()</code>.
It allocates via <code>shm_calloc()</code> in the shared memory, and returns SHM_OFFT_TO_ADDR(offset).
If <code>shm_calloc()</code> returns <code>SHM_NULL</code>, then <code>ptr_calloc()</code> returns <code>NULL</code>.

<br>

<h4>ptr_free():</h4>

Frees memory in shared memory.
It is just a wrapper around <code>shm_free()</code>.
It calls <code>shm_free()</code> with argument as </code>SHM_ADDR_TO_OFFT(ptr)</code>


<h2>RETURN VALUES</h2>


<code>shm_malloc()</code> and <code>shm_calloc()</code> return <code>SHM_NULL</code> on failure.

<code>ptr_malloc()</code> and <code>ptr_calloc()</code> return <code>NULL</code> on failure.

<code>shm_init()</code> returns true on success else false. If calling it in multiple threads, see it's <a href="#shm_init">description</a>
for more information on its return value in multiple threads.
