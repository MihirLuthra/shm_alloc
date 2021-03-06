# shm_alloc

<h1>Shared memory allocation for cache data</h1>

<ul>
	<li><a href="#what-is-it">What is it?</a></li>
	<li><a href="#when-to-use">When to use?</a></li>
	<li><a href="#tested-on">Tested on</a></li>
	<li><a href="#important-notes-for-optimisation">Important Notes For Optimisation</a></li>
	<li><a href="#use-case">Use case</a></li>
	<li><a href="example/">Quick and short example</a></li>
	<li><a href="docs/how_to_use.md">How to use?</a></li>
	<li><a href="docs/man.md">Manpage</a></li>
	<li><a href="docs/source_code_explanation.md">Source code explanation</a></li>
</ul>

# What is it?

A shared memory allocation library that mainly provides <a href="docs/man.md#shm_malloc"><code>shm_malloc()</code></a>,
<a href="docs/man.md#shm_calloc"><code>shm_calloc()</code></a> and <a href="docs/man.md#shm_free"><code>shm_free()</code></a>.
<ol>
    <li>
        They are to be used just like <code>malloc(2)</code>, <code>calloc(2)</code> and <code>free(2)</code> except that they 
        allocate space in shared memory and return offset from the start of shared memory instead.
    </li>
    <li>
        The shared memory is just a regular file which is <code>mmap(2)</code>'d into the processes' address space. This is done in <a href="docs/man.md#shm_init"><code>shm_init()</code></a>.
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
        It uses C11's atomic library. It has been ensured that this branch works with <code>(_POSIX_C_SOURCE >= 200809L)</code>.
		It uses gcc/clang extensions if available. The Makefile used to build the code in NOT posix compatible. The user will
		will have to rely on their own build system if needed. The current Makefile has only been tested on macOS Catalina and
		ubuntu 18.04. As far as I remember, current Makefile failed on FreeBSD.
    </li>
	<li>
		If you are ok with allocating memory in a certain range. e.g., our use case would let us allocate memory
		in range 32 bytes - 1024 bytes (which is default). Main point here is how big can the range be?
		Max and min sizes need to be power of 2 and difference of these powers should be <= 5 if 64 bit compiler and
		<= 4 if 32 bit compiler.<br>
		32 = 2<sup>5</sup> and 1024 = 2<sup>10</sup>. The requirement is satisified as 10 - 5 = 5. It could have been
		2<sup>15</sup> to 2<sup>20</sup>. (See <a href="docs/how_to_use.md#changing-default-settings">changing defaults section</a>). <br>
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

# Important Notes For Optimisation

<ol>
	<li>
		It is a good idea to have shared memory file in file system made by <code>tmpfs</code>.
		There are many alternatives for <code>macOS</code> which you can find by googling.
	</li>
	<li>
		<code>malloc(2)</code> has the privilege of using huge pages which makes it faster
		because of TLB. But in case of <code>shm_malloc()</code> that uses <code>mmap(2)</code>
		with <code>MAP_SHARED</code> doesn't have huge page support.<br>
		The starting point of shared memory is page aligned. Afterwards, there are blocks
		whose size can be altered by you(see <a href="docs/how_to_use.md#changing-default-settings">changing defaults section</a>).
		By default max size is 1024 bytes. So if page size is 4096, 4 blocks of memory constitute a single page.
	</li>
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

Find examples in <a href="example">example directory<a>.

# How to use?

Find the instructions to use the library in 
<a href="docs/how_to_use.md">how_to_use.md<a>
	
# Manpage

Find the documentation of all library functions in
<a href="docs/man.md">man.md<a>

# Source code explanation

Find the source code explanation in 
<a href="docs/source_code_explanation.md">source_code_explanation.md</a>.
