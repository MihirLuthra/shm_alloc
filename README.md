# shm_alloc

<h1>Shared memory allocation for cache data</h1>

<ul>
	<li><a href="#what-is-it">What is it?</a></li>
	<li><a href="#when-to-use">When to use?</a></li>
	<li><a href="#tested-on">Tested on</a></li>
	<li><a href="#use-case">Use case</a></li>
	<li><a href="#quick-and-short-example">Quick and short example</a></li>
	<li><a href="#how-to-use">How to use?</a></li>
	<li><a href="#source-code-explanation">Source code explanation</a></li>
</ul>

# What is it?

A shared memory cache allocation library that provides 3 main functions:

<ul>
	<li>
		shm_malloc()
	</li>
	<li>
		shm_calloc()
	</li>
	<li>
		shm_free()
	</li>
</ul>


<ol>
    <li>
        They are to be used just like <code>malloc(2)</code>, <code>calloc(2)</code> and <code>free(2)</code> except that they 
        allocate space in shared memory and return offset from the start of shared memory instead.
    </li>
    <li>
        The shared memory is just a regular file which is <code>mmap(2)</code>'d into the processes' address space. This is done in <code>shm_init()</code>. It's recommended to call <code>shm_init()</code> in
        <a href="https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Function-Attributes.html"><code>__attribute__((constructor))</code></a> if it doesn't disturb your code to avoid unecessary clashes among threads.
    </li>
    <li>
        The shared memory is divided into small blocks and allocation of each block is managed by a buddy system in form of 
        a bitmap. Bitmaps for every block lie in the initial region of file and corresponding to them are the allocatable 
        regions just after the bitmaps region ends.
    </li>
</ol>

# When to use?

<ol>
    <li>
        It uses C11's atomic library. Also, it uses some of gcc extensions, which are available in clang as well.
		So if you want standard C, good luck modifying.
    </li>
	<li>
		If you are ok with allocating memory in a certain range. e.g., our use case would let us allocate memory
		in range 32 bytes - 1024 bytes (which is default). Main point here is how big can the range be?
		Max and min sizes need to be power of 2 and difference of these powers should be <= 5 if 64 bit compiler and
		<= 4 if 32 bit compiler.<br>
		32 = 2<sup>5</sup> and 1024 = 2<sup>10</sup>. The requirement is satisified as 10 - 5 = 5. It could have been
		2<sup>15</sup> to 2<sup>20</sup>. (See <a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md#changing-default-settings">changing defaults section</a>). <br>
	</li>
	<li>
		The memory allocated on request of <em>n</em> bytes will be next closest power of 2 after <em>n</em> within range.
		e.g., If 158 bytes are requested, 256 bytes are allocated.<br>
		If 9 bytes are requested, 32 bytes are allocated (not 16 because range is 32 to 1024).<br>
		If more than 1024 bytes are requested, <code>SHM_NULL</code> is returned.
	</li>
</ol>

# Tested on

<ol>
	<li>macOS Catalina</li>
	<li>Ubuntu 18.04(with and without -m32)</li>
</ol>

# Use case

<ol>
    <li>
        It is meant to be used alongside a <a href="https://en.wikipedia.org/wiki/Ctrie">ctrie</a> data structure into which 
        multiple processes may insert path names. Memory requests are generally not more than 1024(as they are paths) in our case and even if they are, 
        it's ok to ignore such requests as its just for caching.
    </li>
    <li>
        This library needs to be injected into anonymous processes where it would intercept various system calls & library 
        functions and perform operations with shared memory. So using locks is not a good idea as the process may get 
        interrupted by a signal or maybe even called inside a signal handler or any such similar situation may cause a 
        deadlock. As it may even be called inside a signal handler, its essential to use 
        <a href="http://man7.org/linux/man-pages/man7/signal-safety.7.html">asyc-signal-safe functions</a>. That's one more
        reason why shared memory initialisation that requires <code>mmap(2)</code> is done with <a href="https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Function-Attributes.html"><code>__attribute__((constructor))</code></a> in our use case
        (as it is not considered asyc safe at the present time).
    </li>
</ol>

# Quick and short example

<pre>
//
// example.c
//
#include "shm_alloc.h"
#include &ltstdlib.h&gt
#include &ltstring.h&gt

#define PTR(type)             shm_offt
#define ACCESS(offset, type)  ((type *)((uint8_t *)get_shm_user_base() + (offset)))

int main()
{
	PTR(char) str;<br>
	size_t string_len = 100;
	str = shm_calloc(string_len, sizeof(char));<br>
	if (str == SHM_NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}<br>
	strcpy(ACCESS(str, char), "My test string!");<br>
	printf("%s\n", ACCESS(str, char));<br>
	shm_free(str);<br>
	return 0;
}
</pre>

Then,
On mac:
<pre>
cd &ltrepository_path&gt/shm_alloc/src && make
export SHM_FILE="/tmp/example_shm_file"
export DYLD_FALLBACK_LIBRARY_PATH="&ltrepository_path&gt/shm_alloc/src/:$DYLD_FALLBACK_LIBRARY_PATH"
clang -I&ltrepository_path&gt/shm_alloc/src/ example.c -L&ltrepository_path&gt/shm_alloc/src/ -lshm_alloc
</pre>

On linux:
<pre>
cd &ltrepository_path&gt/shm_alloc/src && make
export SHM_FILE="/tmp/example_shm_file"
export LD_LIBRARY_PATH="&ltrepository_path&gt/shm_alloc/src/:$DYLD_FALLBACK_LIBRARY_PATH"
gcc -I&ltrepository_path&gt/shm_alloc/src/ example.c -L&ltrepository_path&gt/shm_alloc/src/ -lshm_alloc
</pre>


# How to use?

Find the instructions to use the library in 
<a href="docs/how_to_use.md">how_to_use.md<a>

# Source code explanation

Find the source code explanation in 
<a href="docs/source_code_explanation.md">source_code_explanation.md</a>.
