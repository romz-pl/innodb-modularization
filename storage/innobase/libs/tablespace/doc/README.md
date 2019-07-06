
IMPLEMENTATION OF THE TABLESPACE MEMORY CACHE
=============================================

The tablespace cache is responsible for providing fast read/write access to
tablespaces and logs of the database. File creation and deletion is done
in other modules which know more of the logic of the operation, however.

Only the system  tablespace consists of a list  of files. The size of these
files does not have to be divisible by the database block size, because
we may just leave the last incomplete block unused. When a new file is
appended to the tablespace, the maximum size of the file is also specified.
At the moment, we think that it is best to extend the file to its maximum
size already at the creation of the file, because then we can avoid dynamically
extending the file when more space is needed for the tablespace.

Non system tablespaces contain only a single file.

A block's position in the tablespace is specified with a 32-bit unsigned
integer. The files in the list  are thought to be catenated, and the block
corresponding to an address n is the nth block in the catenated file (where
the first block is named the 0th block, and the incomplete block fragments
at the end of files are not taken into account). A tablespace can be extended
by appending a new file at the end of the list.

Our tablespace concept is similar to the one of Oracle.

To have fast access to a tablespace or a log file, we put the data structures
to a hash table. Each tablespace and log file is given an unique 32-bit
identifier, its tablespace ID.

Some operating systems do not support many open files at the same time,
Therefore, we put the open files in an LRU-list. If we need to open another
file, we may close the file at the end of the LRU-list. When an I/O-operation
is pending on a file, the file cannot be closed. We take the file nodes with
pending I/O-operations out of the LRU-list and keep a count of pending
operations. When an operation completes, we decrement the count and return
the file to the LRU-list if the count drops to zero.

The data structure (Fil_shard) that keeps track of the tablespace ID to
fil_space_t* mapping are hashed on the tablespace ID. The tablespace name to
fil_space_t* mapping is stored in the same shard. A shard tracks the flushing
and open state of a file. When we run out open file handles, we use a ticketing
system to serialize the file open, see Fil_shard::reserve_open_slot() and
Fil_shard::release_open_slot().

When updating the global/shared data in Fil_system acquire the mutexes of
all shards in ascending order. The shard mutex covers the fil_space_t data
members as noted in the fil_space_t and fil_node_t definition.
