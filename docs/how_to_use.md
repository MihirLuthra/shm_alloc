<h2>How to use?</h2>

<ol>
	<li>
		<h3>Getting the library</h3>
		Go to shm_alloc directory, then:
<pre>
cd src && make
</pre>
		This would produce the library <code>libshm_alloc.dylib</code> if macOS else <code>libshm_malloc.so</code>.
		To use it, link <code>-lshm_alloc</code> while compiling you code. For example,
<pre>
gcc program.c -L/path/to/lib -lshm_alloc
</pre>
		Library should be in library search path otherwise it won't work. To set that, follow instructions in next point.
	</li>
	<li>
		<h3>Set library search path</h3>
		You may skip this step if you moved library to a path that compiler searches by default.<br><br>
		<ol>
			<li>
				On macOS, add path of the directory where libshm_alloc.dylib file resides to the environment variable
				<code>DYLD_FALLBACK_LIBRARY_PATH</code> as:
<pre>
export DYLD_FALLBACK_LIBRARY_PATH="/path/to/lib:$DYLD_FALLBACK_LIBRARY_PATH"
</pre>
			</li>
			<li>
				On linux, add path of the directory where libshm_alloc.so file resides to the environment variable
				<code>LD_LIBRARY_PATH</code> as:
<pre>
export LD_LIBRARY_PATH="/path/to/lib:$LD_LIBRARY_PATH"
</pre>
			</li>
		</ol>
	</li>
    <li>
        <h3>Set the file to be used for shared memory[optional]</h3>
        <ol>
            <li>
				Set environment variable <code>SHM_FILE</code> to contain path to the file which you would like to use as the 
				shared memory.
                For example,
<pre>
export SHM_FILE="path/to/shm/file"
</pre>     
			</li>
            <li>
                The file shouldn't exist before.
            </li>
        </ol>
    </li>
	<li>
		<h3>Using library functions[find documentation in <a href="man.md">manpage</a>]</h3>
		<ol>
			<li>
				Include the header <code>shm_alloc.h</code> as:
<pre>
#include "shm_alloc.h"
</pre>
			</li>
			<li>
				<h4>shm_init()</h4>
					To setup all shared memory mappings. It can handle being called in multiple threads/processes.
					It maybe a good idea to call it in a function marked as <code>__attribute__((constructor))</code> to
					avoid unnecessary clashes among threads.
			</li>
			<li>
				<h4>shm_deinit()</h4>
					This is used to free the mapping and other resources held for shared memory usage.
				    This doesn't delete the shared memory file(the file obtained from SHM_FILE env varibale).
					Its entirely the user's responsibility to delete the file after use.
			</li>
			<li>
				<h4>shm_malloc()</h4>
					To allocate <code>n</code> bytes(where n < max allocatable size) in shared memory:
<pre>
assert(n <= get_shm_max_allocatable_size() + get_sizeof_block_header());<br>
shm_offt mem_offset = shm_malloc(n);<br>		
if (mem_offset == SHM_NULL) {
	//Couldn't get memory
	//Handle here
}
</pre>
			</li>
			<li>
				<h4>get_shm_max_allocatable_size()</h4>
					To get max allocatable size, call <code>get_shm_max_allocatable_size()</code>.
					Use this function to get max allocatable size and do NOT assume it on the basis of
					<code>MAX_ALLOC_POW2</code> as some size is used for headers.
			</li>
			<li>
				<h4>SHM_NULL</h4>
					<code>shm_(m|c)alloc()</code> returns <code>SHM_NULL</code> if it couldn't allocate memory.<br>
					It has a value of 0 and at that offset of shared memory, mapping is readonly.
			</li>
			<li>
				<h4>shm_offt</h4>
					It is the return type of <code>shm_(m|c)alloc()</code>.
					Format specifier for printing it is <code>PRIu_shm_offt</code>.
			</li>
			<li>
				<h4>shm_calloc()</h4>
					<code>shm_calloc()</code> can be used similarly like shm_malloc():
<pre>
assert(n <= get_shm_max_allocatable_size() + get_sizeof_block_header());<br>
shm_offt mem_offset = shm_calloc(1, n);<br>		
if (mem_offset == SHM_NULL) {
	//Couldn't get memory
	//Handle here
}
</pre>
					<code>shm_calloc()</code> also sets the allocated region to all 0.	
			</li>
			<li>
				<h4>Accessing allocated memory: get_shm_user_base() and user_shm_base</h4>
					To access allocated memory, you need shm base first:
<pre>
void *shm_base = get_shm_user_base();
</pre>
					Then access the previously allocated memory as:
<pre>
void * mem = (uint8_t *)shm_base + mem_offset;
</pre>
			</li>
			<li>
				<h4>shm_free()</h4>
					When done with using the memory, free it like:
<pre>
shm_free(mem_offset);
</pre>
			</li>
		</ol>
	</li>
	<li>
		<h3>Changing default settings</h3>
		<ol>
			<li>
				Changing default env variable name:<br>
                If you want to use a different env variable, compile the code defining <code>SHM_PATH_ENV_NAME</code> to a
				string with new env variable name as follows:      
<pre>
make USER_FLAGS='-D SHM_PATH_ENV_NAME=&lt;env_name&gt;'
</pre>
                This macro is to be defined as a string. So for instance, if the name of the environment variable is 
                NEW_SHM_FILE, then do like:       
<pre>
make USER_FLAGS='-D SHM_PATH_ENV_NAME=\"NEW_SHM_FILE\"'
</pre>
            </li>
			<li>
				Changing default max allocatable size:
<pre>
make USER_FLAGS='-D MAX_ALLOC_POW2=&lt;new_pow&gt;'
</pre>
				For example, setting new power to 15 means max allocatable size is 2^15 = 32768
			</li>
			<li>
				Changing default min allocatable size:
<pre>
make USER_FLAGS='-D MIN_ALLOC_POW2=&lt;new_pow&gt;'
</pre>
				For example, setting new power to 10 means min allocatable size is 2^10 = 1024
			</li>
			<li>
				Changing default shared memory size:
<pre>
make USER_FLAGS='-D MAX_ALLOCATABLE_SHM_SIZE=&lt;new_size&gt;'
</pre>
				For example, setting new size to 262144 means 262144 bytes are allocatable.
			</li>
		</ol>
	</li>
	<li>
		<h3>Making code readable</h3>
		Preferred way would be to use wrappers <a href="man.md#ptr_malloc"><code>ptr_malloc()</code></a>,
		<a href="man.md#ptr_calloc"><code>ptr_calloc</code></a> and <a href="man.md#ptr_free"><code>ptr_free()</code></a>.
<pre>
char *str;
size_t string_len = 100;
str = ptr_malloc(string_len);<br>
if (str == NULL) {
    fprintf(stderr, "ptr_malloc(): out of memory");
}
</pre>
		For example, you are dealing with a shared memory
		linked list that looks like:
<pre>
struct llist {
    int data;
    shm_offt next;
};
</pre>
<pre>
struct llist lnode;
lnode.next = SHM_ADDR_TO_OFFT(str); // would extract the offset and store
</pre>
	</li>
</ol>
