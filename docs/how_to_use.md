<h2>How to use?</h2>

<ol>
    <li>
        <h3>Set the file to be used for shared memory</h3>
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
            <li>
                If you want to use a different env variable, compile the code defining <codeSHM_PATH_ENV_NAME</code> to a
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
        </ol>
    </li>
	<li>
		<h3>Set library search path</h3>
		You may skip this step if you moved library to a path that compiler searches by default.<br><br>
		<ol>
			<li>
				On macOS, add path of the directory where libshm_alloc.so file resides to the environment variable
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
		<h3>Using library functions</h3>
		<ol>
			<li>
				Include the header <code>shm_alloc.h</code> as:
<pre>
#include "shm_alloc.h"
</pre>
			</li>
			<li>
				To allocate <code>n</code> bytes(where n < max allocatable size) in shared memory:
<pre>
shm_offt mem_offset = shm_malloc(n);<br>		
if (mem_offset == SHM_NULL) {
	//Couldn't get memory
	//Handle here
}
</pre>
				To get max allocatable size, call <code>get_shm_max_allocatable_size()</code>.<br>
				<code>shm_malloc()</code> returns <code>SHM_NULL</code> if it couldn't allocate memory.<br>
				<code>shm_calloc()</code> can be used similarly:
<pre>
shm_offt mem_offset = shm_calloc(1, n);<br>		
if (mem_offset == SHM_NULL) {
	//Couldn't get memory
	//Handle here
}
</pre>
				<code>shm_calloc()</code> also sets the allocated region to all 0.			
			</li>
			<li>
				To access allocated memory, you need shm base first:
<pre>
void *shm_base = get_shm_user_base();
</pre>
				Then access the previously allocated memory as:
<pre>
void * mem = (uint8_t *)shm_base + mem_offset;
</pre>
				Instead of calling <code>get_shm_user_base()</code>, you can also use the global variable
				<code>user_shm_base</code> directly as:
<pre>
void * mem = user_shm_base + mem_offset;
</pre>
			</li>
			<li>
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
				For example, setting new power to 10 means max allocatable size is 2^10 = 1024
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
		One way to make code readable is:
<pre>
#define PTR(type)             shm_offt
#define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))
</pre>
		Usage example:
<pre>
PTR(char) str;<br>
size_t string_len = 100;
str = shm_calloc(string_len, sizeof(char));<br>
if (str == SHM_NULL) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
}<br>
strcpy(ACCESS(str, char), "My test string!");<br>
printf("%s\n", ACCESS(str, char));
</pre>
	This is one way to use it, better ways are always there.
	</li>
</ol>
