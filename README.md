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

They are to be used just like `malloc(2)`, `calloc(2)` and `free(2)` except that they allocate space in shared memory and
return offset from the start of shared memory instead.

# When to use?

It is meant to be used for caching, so it is not efficient for large data.

Currently it is defaulted such that max memory available is `256 MB` and max size that can be allocated from `shm_malloc()` is
`4096` bytes.

# How to use?

Find the instructions to use the library in <a href="https://github.com/MihirLuthra/shm_alloc/blob/master/docs/how_to_use.md">how_to_use.md<a>
