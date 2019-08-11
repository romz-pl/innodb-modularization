IMPLEMENTATION OF THE BUFFER POOL
=================================

Buffer frames and blocks
------------------------
Following the terminology of Gray and Reuter, we call the memory
blocks where file pages are loaded buffer frames. For each buffer
frame there is a control block, or shortly, a block, in the buffer
control array. The control info which does not need to be stored
in the file along with the file page, resides in the control block.

Buffer pool struct
------------------
The buffer buf_pool contains several mutexes which protect all the
control data structures of the buf_pool. The content of a buffer frame is
protected by a separate read-write lock in its control block, though.

buf_pool->chunks_mutex protects the chunks, n_chunks during resize;
it also protects buf_pool_should_madvise:
- readers of buf_pool_should_madvise hold any buf_pool's chunks_mutex
- writers hold all buf_pools' chunk_mutex-es;
it is useful to think that it also protects the status of madvice() flags set
for chunks in this pool, even though these flags are handled by OS, as we only
modify them why holding this latch;
buf_pool->LRU_list_mutex protects the LRU_list;
buf_pool->free_list_mutex protects the free_list and withdraw list;
buf_pool->flush_state_mutex protects the flush state related data structures;
buf_pool->zip_free mutex protects the zip_free arrays;
buf_pool->zip_hash mutex protects the zip_hash hash and in_zip_hash flag.

Control blocks
--------------

The control block contains, for instance, the bufferfix count
which is incremented when a thread wants a file page to be fixed
in a buffer frame. The bufferfix operation does not lock the
contents of the frame, however. For this purpose, the control
block contains a read-write lock.

The buffer frames have to be aligned so that the start memory
address of a frame is divisible by the universal page size, which
is a power of two.

The control blocks containing file pages are put to a hash table
according to the file address of the page.
We could speed up the access to an individual page by using
"pointer swizzling": we could replace the page references on
non-leaf index pages by direct pointers to the page, if it exists
in the buf_pool. We could make a separate hash table where we could
chain all the page references in non-leaf pages residing in the buf_pool,
using the page reference as the hash key,
and at the time of reading of a page update the pointers accordingly.
Drawbacks of this solution are added complexity and,
possibly, extra space required on non-leaf pages for memory pointers.
A simpler solution is just to speed up the hash table mechanism
in the database, using tables whose size is a power of 2.

Lists of blocks
---------------

There are several lists of control blocks.

The free list (buf_pool->free) contains blocks which are currently not
used.

The common LRU list contains all the blocks holding a file page
except those for which the bufferfix count is non-zero.
The pages are in the LRU list roughly in the order of the last
access to the page, so that the oldest pages are at the end of the
list. We also keep a pointer to near the end of the LRU list,
which we can use when we want to artificially age a page in the
buf_pool. This is used if we know that some page is not needed
again for some time: we insert the block right after the pointer,
causing it to be replaced sooner than would normally be the case.
Currently this aging mechanism is used for read-ahead mechanism
of pages, and it can also be used when there is a scan of a full
table which cannot fit in the memory. Putting the pages near the
end of the LRU list, we make sure that most of the buf_pool stays
in the main memory, undisturbed.

The unzip_LRU list contains a subset of the common LRU list.  The
blocks on the unzip_LRU list hold a compressed file page and the
corresponding uncompressed page frame.  A block is in unzip_LRU if and
only if the predicate buf_page_belongs_to_unzip_LRU(&block->page)
holds.  The blocks in unzip_LRU will be in same order as they are in
the common LRU list.  That is, each manipulation of the common LRU
list will result in the same manipulation of the unzip_LRU list.

The chain of modified blocks (buf_pool->flush_list) contains the blocks
holding file pages that have been modified in the memory
but not written to disk yet. The block with the oldest modification
which has not yet been written to disk is at the end of the chain.
The access to this list is protected by buf_pool->flush_list_mutex.

The chain of unmodified compressed blocks (buf_pool->zip_clean)
contains the control blocks (buf_page_t) of those compressed pages
that are not in buf_pool->flush_list and for which no uncompressed
page has been allocated in the buffer pool.  The control blocks for
uncompressed pages are accessible via buf_block_t objects that are
reachable via buf_pool->chunks[].

The chains of free memory blocks (buf_pool->zip_free[]) are used by
the buddy allocator (buf0buddy.cc) to keep track of currently unused
memory blocks of size sizeof(buf_page_t)..UNIV_PAGE_SIZE / 2.  These
blocks are inside the UNIV_PAGE_SIZE-sized memory blocks of type
BUF_BLOCK_MEMORY that the buddy allocator requests from the buffer
pool.  The buddy allocator is solely used for allocating control
blocks for compressed pages (buf_page_t) and compressed page frames.

Loading a file page
-------------------

First, a victim block for replacement has to be found in the
buf_pool. It is taken from the free list or searched for from the
end of the LRU-list. An exclusive lock is reserved for the frame,
the io_fix field is set in the block fixing the block in buf_pool,
and the io-operation for loading the page is queued. The io-handler thread
releases the X-lock on the frame and resets the io_fix field
when the io operation completes.

A thread may request the above operation using the function
buf_page_get(). It may then continue to request a lock on the frame.
The lock is granted when the io-handler releases the x-lock.

Read-ahead
----------

The read-ahead mechanism is intended to be intelligent and
isolated from the semantically higher levels of the database
index management. From the higher level we only need the
information if a file page has a natural successor or
predecessor page. On the leaf level of a B-tree index,
these are the next and previous pages in the natural
order of the pages.

Let us first explain the read-ahead mechanism when the leafs
of a B-tree are scanned in an ascending or descending order.
When a read page is the first time referenced in the buf_pool,
the buffer manager checks if it is at the border of a so-called
linear read-ahead area. The tablespace is divided into these
areas of size 64 blocks, for example. So if the page is at the
border of such an area, the read-ahead mechanism checks if
all the other blocks in the area have been accessed in an
ascending or descending order. If this is the case, the system
looks at the natural successor or predecessor of the page,
checks if that is at the border of another area, and in this case
issues read-requests for all the pages in that area. Maybe
we could relax the condition that all the pages in the area
have to be accessed: if data is deleted from a table, there may
appear holes of unused pages in the area.

A different read-ahead mechanism is used when there appears
to be a random access pattern to a file.
If a new page is referenced in the buf_pool, and several pages
of its random access area (for instance, 32 consecutive pages
in a tablespace) have recently been referenced, we may predict
that the whole area may be needed in the near future, and issue
the read requests for the whole area.


Let us list the consistency conditions for different control block states.
=========================================================================

NOT_USED:	is in free list, not in LRU list, not in flush list, nor
                page hash table
READY_FOR_USE:	is not in free list, LRU list, or flush list, nor page
                hash table
MEMORY:		is not in free list, LRU list, or flush list, nor page
                hash table
FILE_PAGE:	space and offset are defined, is in page hash table
                if io_fix == BUF_IO_WRITE,
                        pool: no_flush[flush_type] is in reset state,
                        pool: n_flush[flush_type] > 0

                (1) if buf_fix_count == 0, then
                        is in LRU list, not in free list
                        is in flush list,
                                if and only if oldest_modification > 0
                        is x-locked,
                                if and only if io_fix == BUF_IO_READ
                        is s-locked,
                                if and only if io_fix == BUF_IO_WRITE

                (2) if buf_fix_count > 0, then
                        is not in LRU list, not in free list
                        is in flush list,
                                if and only if oldest_modification > 0
                        if io_fix == BUF_IO_READ,
                                is x-locked
                        if io_fix == BUF_IO_WRITE,
                                is s-locked

State transitions:

NOT_USED => READY_FOR_USE
READY_FOR_USE => MEMORY
READY_FOR_USE => FILE_PAGE
MEMORY => NOT_USED
FILE_PAGE => NOT_USED	NOTE: This transition is allowed if and only if
                                (1) buf_fix_count == 0,
                                (2) oldest_modification == 0, and
                                (3) io_fix == 0.

