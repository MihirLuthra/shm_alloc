# TESTS

<ol>
	<li>
		<a href="#how-to-test">How to test</a>
	</li>
	<li>
		<a href="#how-test-works">How test works</a>
	</li>
</ol>


<h2>How to test</h2>

First, to setup environment variables(setting library path and `SHM_FILE`, you can do it by yourself if you wish):

```
source test_init
```

Then, to get the test executable:

```
make
```
 
Then, run the executable with args.

arg1 = <num_of_processes><br>
arg2 = <num_of_threads_per_process><br>
arg3 = <num_strings_per_process><br>

```
./test_shm_alloc.out 5 5 1000
```

Test with random values, multiple number of times.
 
 <h2>How test works</h2>
 
usage :  <num_of_processes> <num_of_threads_per_process> <num_strings_per_process>

arg1 = <num_of_processes><br>
arg2 = <num_of_threads_per_process><br>
arg3 = <num_strings_per_process><br>

The test spawns arg1 number of processes and in each of them, it generates arg3 number of
random strings. Then each process spawns arg2 number of threads which perform the tests.

The random strings are stored in a global array. A global counter is present which starts from `0` and goes upto `max_idx-1`.

In the test each thread extracts the value of global counter and increments the counter atomically. For the random string at the index given by the extracted value of counter, following operions are performed:

The threads allocates memory in the shared memory region and stores the random string in it.
The data about the random string(its index in array and offset in shared memory) is stored in an atomic queue(by 
`OSAtomicFifoEnqueue()`). Then an element is dequed from the atomic queue. The atomic queue is global, so it's common for all 
threads. As per the dequed data, the offset is accessed in shared memory and compared with the string in the array for 
checking correctness. If the data is incorrect, test failed, else this string is freed from the shared memory "randomly"(just 
generating a random num; if divisble by 3 then free). If it doesn't get freed, the data about this random string in enqued
back into the atomic queue. After this, next iteration starts and this sequence continues until global counter is equal to
`max_idx-1`.

Freeing randomly lets the data last in shared memory for long and helps in checking correctness.

On systems other than `macOS` where `OSAtomicFifoEnqueue()` is not available, `lfstack` submodule is used. The former is
fifo and the latter is lifo, rest is almost the same.

The test is pretty accurate, but check it multiple number of times with different arguments of 
for rarely occuring failures.

Define `TEST_THE_TEST=1` to test the working of test for multiple thresds. Setting it to `1` would cause the test
to use `(m|c)alloc(2)` and `free(2)` instead of `shm_(m|c)alloc()` and `shm_free()`. This can also
be used to compare speed of shared memory alternatives with general system calls.


PS: `(m|c)alloc(2)` and `free(2)` manage allocations in multithreaded programs, so when using `TEST_THE_TEST=1`, remember
to spawn multiple threads. Multiple process tests won't really get effected by `(m|c)alloc(2)`.
