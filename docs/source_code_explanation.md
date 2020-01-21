<h1>Abstract</h1>
The basic working of code goes as follows:
<ol>
	<li>
		File at path given by env variable <code>SHM_FILE</code> is <a href="http://man7.org/linux/man-pages/man2/mmap.2.html"><code>mmap(2)</code></a>'d into process' address space.
		If the file doesn't exist it is created. As the file is given its size by <a href="https://linux.die.net/man/2/ftruncate"><code>ftruncate(2)</code></a>,
		it is initialised with all 0.
	</li>
	<li>
		When <code>shm_malloc()</code> is asked for memory, it is rounded up to the next power of 2. Then management blocks are 
		checked for free bits as per the requirement. The bits are set and offset corresponding to them in the allocatable region
		is returned.
	</li>
	<li>
		To access the allocated shared memory, user needs to get the base of allocatable region by <code>get_shm_user_base()</code>.
		Then offset needs to be added to the base, while typecasting base to <code>uint8_t *</code>.
	</li>
	<li>
		To free the memory, <code>shm_free()</code> is called and it unsets the bits corresponding to the offset supplied.
	</li>
</ol>
	

<h1>Explanation</h1>

<img src="shared_memory.png" alt="shared memory file structure" width="600" height="400"><br><br><br>
To start with, consider a file. The file's size is obtained by a <code>get_shm_mapping_size()</code> defined in
<code>shm_constants.c</code>. The file contains 4 regions just as shown in the image above.

<ul>
	<li>
		<a href="#shm-allocatable-region">
			Shm Allocatable Region
		</a>
	</li>
	<li>
		<a href="#shm-management-region">
			Shm Management Region
		</a>
	</li>
	<li>
		<a href="#shm-null">
			Shm Null
		</a>
	</li>
	<li>
		<a href="#shm-data-table">
			Shm Data Table
		</a>
	</li>
</ul>

<h2>Shm Allocatable Region</h2>
In the file, the area that is for actual data storage is called the <em>shm allocatable region</em>. Its size is
defined by the macro <code>MAX_ALLOCATABLE_SHM_SIZE</code>. By default it is 512 MB. This region is thought as a collection
of blocks of size <code>MAX_ALLOCATABLE_SIZE</code>. By default it is 1024 bytes. So there are 262144 blocks in the allocatable
region by default.

<h2>Shm Management Region</h2>
For every block in allocatable region, there exists a management block in <em>shm management region</em>. These management
blocks contain a bitmap and a variable that counts the memory used in that block. The management block is a variable of
type <code>struct shm_block_mgmt</code> which is defined as follows:

<pre>
struct shm_block_mgmt {
    _Atomic(shm_bitmap) mgmt_bmp;
    _Atomic(size_t)     mem_used;
};
</pre>

<ul>
	<li>
		<h4>mgmt_bmp</h4>
			<ol>
				<li>
				Bits in <code>mgmt_bmp</code> manage memory allocation in the corresponding block in allocatable region. 
				The bitmap looks like:
<pre>
0000000000000000000000000000000000000000000000000000000000000000
</pre>
				Well but you need not look it like that. Think of it like:
<pre>
0 --> unused bit
0 --> 1024 
00 --> 512 
0000 --> 256 
00000000 --> 128 
0000000000000000 --> 64 
00000000000000000000000000000000 --> 32 
</pre>
				</li>
				<li>
					This bitmap is used as the <a href="https://en.wikipedia.org/wiki/Buddy_memory_allocation">buddy system</a>.
				</li>
				<li>
					Here <code>mgmt_bmp</code> has 64 bits although only 63 bits are needed. Number of bits being used are defined in <code>BITS</code>.
					<code>shm_bitmap</code> is set to <code>unsigned long</code> if <a href="https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.cbclx01/atomicmacros.htm"><code>ATOMIC_LONG_LOCK_FREE</code></a>
					is 2 i.e. always lock free. Otherwise it checks the same for <code>unsigned int</code>. If <code>unsigned int</code>
					is also not always lock free, the program terminates.
				</li>
				<li>
					To allocate memory in the block, the corresponding bit along with all its children need to be set.<br>
					As its just a single 64 bits bitmap, its pretty simple to do bit setting. First of all, create a mask for the bit range
					for that particular memory level. e.g., For memory level 128, mask would look like:
<pre>
0 --> 1024 
00 --> 512 
0000 --> 256 
11111111 --> 128 
0000000000000000 --> 64 
00000000000000000000000000000000 --> 32
</pre>
					In this mask, we unset the bits that are already set in <code>mgmt_bmp</code> and then we find the posn
					of first set bit in the mask. Let's say first set bit is 8, then we make another mask called the <em>set mask</em>,
					that has bit no. 8 and all its children set like:
<pre>
0 --> 1024 
00 --> 512 
0000 --> 256 
10000000 --> 128 
1100000000000000 --> 64 
11110000000000000000000000000000 --> 32
</pre>
					Above is the <em>set mask</em>. We ensure none of these bits are set in <code>mgmt_bmp</code> and then we
					set all these bits via atomic CAS.
				</li>
				<li>
					To free the memory, the <em>set mask</em> is obtained for that particular bit. Then we unset all the <em>set mask</em> bits
					in <code>mgmt_bmp</code> via atomic CAS.
				</li>
			</ol>
	</li>
	<li>
		<h4>mem_used</h4>
			<code>mem_used</code> holds the amount of memory used in block. This variable is only for optimising the code. If the 
			<code>memory needed + mem_used > max allocatable size</code>, then we skip this block.
	</li>
</ul>

<h2>Shm Null</h2>

This is a part of <em>shm allocatable region</em> and is located in the start of it i.e. offset is 0. It's a single page
mapped with <code>PROT_READ</code>. When mapping
the file into the process, this region is made readonly. Management blocks for shm null exists as well, as it is a part of
allocatable region. They are changed to mark shm null blocks as allocated while initialising shared memory.

<h2>Shm Data Table</h2>

This is placed at the very starting of mapping(which doesn't really make a difference). Its purpose is to make memory
search faster. It is accessed by a variable of type <code>struct shm_data_table</code> which is defined as follows:

<pre>
struct shm_data_table {
    _Atomic(shm_offt) start_blk_mgr_offt;
};
</pre>

<ul>
	<li>
	<h4>start_blk_mgr_offt</h4>
	For a fresh shared memory i.e. all memory is unused, It stores offset to the first management block that is not full.
	Just think for once that no memory is being freed and blocks keep getting filled one by one. To allocate memory
	when n blocks are full, it would take n iterations to reach n+1<sup>th</sup>. So to avoid that, whenever a block
	gets full, this stores the offset to the next block. When next request for memory comes, search starts from
	the management block at offset <code>start_blk_mgr_offt</code>. Although, as memory keeps getting allocated,
	<code>start_blk_mgr_offt</code> eventually points to the last management block which is full as well.
	So if memory is not found in the scan from <code>start_blk_mgr_offt</code> to the end, second scan is from
	start of the management area upto <code>start_blk_mgr_offt</code> to search for memory, if any was freed by
	<code>shm_free()</code>.<br>
	To optimize the second scan, we need to implement a freelist as well(#TODO).
	</li>
</ul>
