<h1>Examples</h1>

The examples here are just showing basic usage. How they work in case with multiple processes or threads is tested in <a href="tests/">tests</a>
and also in some separate programs which are not listed in this repository.

# example.c

<a href="example/example.c#L15">Line 15</a>, we initialize the shared memory:

<pre>
retval = shm_init(NULL, shm_file_name);
</pre>

<a href="example/example.c#L26">Line 26</a>, get the user's shared memory base for the current process:

<pre>
shm_base = get_shm_user_base();
</pre>

<a href="example/example.c#L34">Line 34</a>, allocate space in shared memory and get the offset to that space:

<pre>
str = shm_calloc(string_len, sizeof(char));
</pre>

<a href="example/example.c#L41">Line 41</a>, to access the allocated area, offset is added to the shm base:

<pre>
strcpy((char *)(shm_base + str), "My test string!");
</pre>


# example_with_ptr.c

This is just a copy of <code>example.c</code>. Here we define <code>my_calloc()</code> and <code>my_free()</code>.
Both of them are exactly same as <a href="docs/man.md#ptr_calloc"><code>ptr_calloc()</code></a> and
<a href="docs/man.md#ptr_free"><code>ptr_free()</code></a>. This is done to show how it actually works.

Further, we just use <a href="docs/man.md#ptr_calloc"><code>ptr_calloc()</code></a> and
<a href="docs/man.md#ptr_free"><code>ptr_free()</code></a> instead of <a href="docs/man.md#shm_calloc"><code>shm_calloc()</code></a> and
<a href="docs/man.md#shm_free"><code>shm_free()</code></a>.
