# shm_alloc

<h1>Shared memory caching library</h1>

<ul>
    <li><a href="https://github.com/MihirLuthra/shm_alloc#what-is-it">What is it?</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc#when-to-use">When to use?</a></li>
    <li><a href="https://github.com/MihirLuthra/shm_alloc#how-to-use">How to use?</a></li>
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
        This has been tested on macOS till now. 
    </li>
    <li>
        It is meant to be used for caching. It can't allocate memory beyond its max limit. Like, if the max allocatable size
        is set to 4096, any requests for memory greater than that from <code>shm_malloc()</code> will return null. The code 
        needs to be compiled with max and min limits. Defaults for min and max are 32 bytes(2^5) and 4096(2^12) bytes
        respectively. In order to change these, code needs to be recompiled with new limits which need to be power of 2.(described in <a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md#changing-default-settings">How to use</a>).
    The code will be efficient if the the difference in the powers is less. Like currently 12(2^12 = 4096) - 5(2^5 = 32) = 7.
    </li>
</ol>


# How to use?

Find the instructions to use the library in 
<a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md">how_to_use.md<a>
