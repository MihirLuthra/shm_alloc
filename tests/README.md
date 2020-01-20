
<h1>Just do the test</h1>

First, to setup environment variables:

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
 
 <h1>Summary on how test works</h1>
 
usage :  <num_of_processes> <num_of_threads_per_process> <num_strings_per_process>

arg1 = <num_of_processes><br>
arg2 = <num_of_threads_per_process><br>
arg3 = <num_strings_per_process><br>

The test spawns arg1 number of processes and in each of them, it generates arg3 number of
random strings. Then in each process it spawns arg2 number of threads which perform the tests.

In the test each thread allocates memory in the shared memory and stores a random string in it.
The data about the random string is stored in an atomic queue(by `OSAtomicFifoEnqueue()`) and dequed
by some other(or maybe even the same thread). The dequed data checks the shared mem for correctness.
If the data is incorrect, test failed, else this string is freed from the shared mem randomly and 
if not freed, then enqued back for check again(see code).

On systems other than `macOS` where `OSAtomicFifoEnqueue()` is not available, `lfstack` submodule is used.

The test is pretty accurate, but check it a some number with different arguments of 
for rarely occuring failures.

Define `TEST_THE_TEST=1` to test the working of test for multiple thresds. Setting it to `1` would cause the test
to use `(m|c)alloc(2)` and `free(2)` instead of `shm_(m|c)alloc()` and `shm_free()`. This can also
be used to compare speed of shared memory alternatives with general system calls. 


PS: `(m|c)alloc(2)` and `free(2)` manage allocations in multithreaded programs, so when using `TEST_THE_TEST=1`, remember
to spawn multiple threads. Multiple process tests won't really get effected by `(m|c)alloc(2)`.
