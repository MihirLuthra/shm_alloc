# shm_alloc

README NOT READY, NEEDS TO BE UPDATED

<h1>Shared memory caching library</h1>

<ul>
    <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#what-is-it">What is it?</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#when-to-use">When to use?</a></li>
     <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#tested-on">Tested on</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#use-case">Use case</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#how-to-use">How to use?</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc/tree/fast_small_cache#source-code-explanation">Source code explanation</a></li>
</ul>

# What is it?

A shared memory cache allocation library that provides 3 main functions:

```
shm_malloc()
shm_calloc()
shm_free()
```

<ol>
    <li>
        They are to be used just like <code>malloc(2)</code>, <code>calloc(2)</code> and <code>free(2)</code> except that they 
        allocate space in shared memory and return offset from the start of shared memory instead.
    </li>
    <li>
        The shared memory is just a regular file which is <code>mmap(2)</code>'d in the processes. This is done before
        <code>main()</code> is called via 
        <a href="https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Function-Attributes.html"><code>__attribute__((constructor))</code></a>.
		Although this behaviour maybe modified as explained in <a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md#changing-default-settings">changing defaults section</a>.
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
        It uses C11's atomic library. Also, it uses a lot of gcc extensions, which are available in clang as well.
    </li>
    <li>
        It is meant to be used for caching. It can't allocate memory beyond its max limit. No matter how big is the shared 
        memory itself, it is still divided into blocks. The block size is the max allocatable size. Like, if the max 
        allocatable size is set to 1024, any requests for memory greater than that from <code>shm_malloc()</code> will return 
        null(even if the shared memory itself was 256 MB). The code needs to be compiled with max and min limits. Defaults for 
        min and max are 32 bytes(2^5) and 1024(2^10) bytes respectively. In order to change these, code needs to be recompiled 
        with new limits which need to be power of 2.(described in <a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md#changing-default-settings">changing defaults section</a>).
        This branch strictly needs a power difference of less than or equal to 5 if 64 bit machine and 4 if 32 bit machine.
		Like 10 - 5 = 5 (as 2^10 = 1024 and 2^5 = 32). While testing on macOS, <code>shm_malloc</code> takes time really close to <code>malloc(2)</code>.
    </li>
    <li> 
        The block size (i.e. max allocatable size) need not be small. Max allocatable size and min allocatable size always 
        need to be powers of 2 and difference needs to be <= 5. Code is efficient even on larger values.
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
        reason why shared memory initialisation that requires <code>mmap(2)</code> is done with <a href="https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Function-Attributes.html"><code>__attribute__((constructor))</code></a>
        (as it is not considered asyc safe at the present time).
    </li>
</ol>

# How to use?

Find the instructions to use the library in 
<a href="https://github.com/MihirLuthra/shm_alloc/blob/fast_small_cache/docs/how_to_use.md">how_to_use.md<a>

# Source code explanation

Find the source code explanation in 
<a href="https://github.com/MihirLuthra/shm_alloc/blob/fast_small_cache/docs/source_code_explanation.md">source_code_explanation.md</a>.
