
LATCHING ORDER WITHIN THE DATABASE
==================================

The mutex or latch in the central memory object, for instance, a rollback
segment object, must be acquired before acquiring the latch or latches to
the corresponding file data structure. In the latching order below, these
file page object latches are placed immediately below the corresponding
central memory object latch or mutex.

Synchronization object			Notes
----------------------			-----

Dictionary mutex			If we have a pointer to a dictionary
|					object, e.g., a table, it can be
|					accessed without reserving the
|					dictionary mutex. We must have a
|					reservation, a memoryfix, to the
|					appropriate table object in this case,
|					and the table must be explicitly
|					released later.
V
Dictionary header
|
V
Secondary index tree latch		The tree latch protects also all
|					the B-tree non-leaf pages. These
V					can be read with the page only
Secondary index non-leaf		bufferfixed to save CPU time,
|					no s-latch is needed on the page.
|					Modification of a page requires an
|					x-latch on the page, however. If a
|					thread owns an x-latch to the tree,
|					it is allowed to latch non-leaf pages
|					even after it has acquired the fsp
|					latch.
V
Secondary index leaf			The latch on the secondary index leaf
|					can be kept while accessing the
|					clustered index, to save CPU time.
V
Clustered index tree latch		To increase concurrency, the tree
|					latch is usually released when the
|					leaf page latch has been acquired.
V
Clustered index non-leaf
|
V
Clustered index leaf
|
V
Transaction system header
|
V
Transaction undo mutex			The undo log entry must be written
|					before any index page is modified.
|					Transaction undo mutex is for the undo
|					logs the analogue of the tree latch
|					for a B-tree. If a thread has the
|					trx undo mutex reserved, it is allowed
|					to latch the undo log pages in any
|					order, and also after it has acquired
|					the fsp latch.
V
Rollback segment mutex			The rollback segment mutex must be
|					reserved, if, e.g., a new page must
|					be added to an undo log. The rollback
|					segment and the undo logs in its
|					history list can be seen as an
|					analogue of a B-tree, and the latches
|					reserved similarly, using a version of
|					lock-coupling. If an undo log must be
|					extended by a page when inserting an
|					undo log record, this corresponds to
|					a pessimistic insert in a B-tree.
V
Rollback segment header
|
V
Purge system latch
|
V
Undo log pages				If a thread owns the trx undo mutex,
|					or for a log in the history list, the
|					rseg mutex, it is allowed to latch
|					undo log pages in any order, and even
|					after it has acquired the fsp latch.
|					If a thread does not have the
|					appropriate mutex, it is allowed to
|					latch only a single undo log page in
|					a mini-transaction.
V
File space management latch		If a mini-transaction must allocate
|					several file pages, it can do that,
|					because it keeps the x-latch to the
|					file space management in its memo.
V
File system pages
|
V
lock_sys_wait_mutex			Mutex protecting lock timeout data
|
V
lock_sys_mutex				Mutex protecting lock_sys_t
|
V
trx_sys->mutex				Mutex protecting trx_sys_t
|
V
Threads mutex				Background thread scheduling mutex
|
V
query_thr_mutex				Mutex protecting query threads
|
V
trx_mutex				Mutex protecting trx_t fields
|
V
Search system mutex
|
V
Buffer pool mutexes
|
V
Log mutex
|
Any other latch
|
V
Memory pool mutex


InnoDB Mutex and Read/Write Lock Implementation
===============================================

In MySQL and InnoDB, multiple threads of execution access shared data structures. InnoDB synchronizes these accesses with its own implementation of mutexes and read/write locks. Historically, InnoDB protected the internal state of a read/write lock with an InnoDB mutex, and the internal state of an InnoDB mutex was protected by a Pthreads mutex, as in IEEE Std 1003.1c (POSIX.1c).

On many platforms, Atomic operations can often be used to synchronize the actions of multiple threads more efficiently than Pthreads. Each operation to acquire or release a lock can be done in fewer CPU instructions, wasting less time when threads contend for access to shared data structures. This in turn means greater scalability on multi-core platforms.

On platforms that support Atomic operations, InnoDB now implements mutexes and read/write locks with the built-in functions provided by the GNU Compiler Collection (GCC) for atomic memory access instead of using the Pthreads approach. More specifically, InnoDB compiled with GCC version 4.1.2 or later uses the atomic builtins instead of a pthread_mutex_t to implement InnoDB mutexes and read/write locks.

On 32-bit Microsoft Windows, InnoDB implements mutexes (but not read/write locks) with hand-written assembler instructions. Beginning with Microsoft Windows 2000, functions for Interlocked Variable Access are available that are similar to the built-in functions provided by GCC. On Windows 2000 and higher, InnoDB makes use of the Interlocked functions, which support read/write locks and 64-bit platforms.

Solaris 10 introduced library functions for atomic operations, and InnoDB uses these functions by default. When MySQL is compiled on Solaris 10 or later with a compiler that does not support the built-in functions provided by the GNU Compiler Collection (GCC) for atomic memory access, InnoDB uses the library functions.

On platforms where the GCC, Windows, or Solaris functions for atomic memory access are not available, InnoDB uses the traditional Pthreads method of implementing mutexes and read/write locks.

When MySQL starts, InnoDB writes a message to the log file indicating whether atomic memory access is used for mutexes, for mutexes and read/write locks, or neither. If suitable tools are used to build InnoDB and the target CPU supports the atomic operations required, InnoDB uses the built-in functions for mutexing. If, in addition, the compare-and-swap operation can be used on thread identifiers (pthread_t), then InnoDB uses the instructions for read-write locks as well.

If you are building from source, ensure that the build process properly takes advantage of your platform capabilities.



mutex
=====

Informal abbreviation for “mutex variable”. (Mutex itself is short for “mutual exclusion”.) The low-level object that InnoDB uses to represent and enforce exclusive-access locks to internal in-memory data structures. Once the lock is acquired, any other process, thread, and so on is prevented from acquiring the same lock. Contrast with rw-locks, which InnoDB uses to represent and enforce shared-access locks to internal in-memory data structures. Mutexes and rw-locks are known collectively as latches.


latch
=====

A lightweight structure used by InnoDB to implement a lock for its own internal memory structures, typically held for a brief time measured in milliseconds or microseconds. A general term that includes both mutexes (for exclusive access) and rw-locks (for shared access). Certain latches are the focus of InnoDB performance tuning. Statistics about latch use and contention are available through the Performance Schema interface.


lock
====

The high-level notion of an object that controls access to a resource, such as a table, row, or internal data structure, as part of a locking strategy. For intensive performance tuning, you might delve into the actual structures that implement locks, such as mutexes and latches.


rw-lock
=======

The low-level object that InnoDB uses to represent and enforce shared-access locks to internal in-memory data structures following certain rules. Contrast with mutexes, which InnoDB uses to represent and enforce exclusive access to internal in-memory data structures. Mutexes and rw-locks are known collectively as latches.

rw-lock types include s-locks (shared locks), x-locks (exclusive locks), and sx-locks (shared-exclusive locks).

    - An s-lock provides read access to a common resource.

    - An x-lock provides write access to a common resource while not permitting inconsistent reads by other threads.

    - An sx-lock provides write access to a common resource while permitting inconsistent reads by other threads. sx-locks were introduced in MySQL 5.7 to optimize concurrency and improve scalability for read-write workloads.

