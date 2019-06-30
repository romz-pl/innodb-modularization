
1. The read-write lock (for threads, not for database transactions)


2. The macros should be used in rw locking and unlocking, not the corresponding function. 


3. For performance schema instrumentation, a new set of rwlock
wrap functions are created if "UNIV_PFS_RWLOCK" is defined.
The instrumentations are not planted directly into original
functions, so that we keep the underlying function as they
are. And in case, user wants to "take out" some rwlock from
instrumentation even if performance schema (UNIV_PFS_RWLOCK)
is defined, they can do so by reinstating APIs directly link to
original underlying functions.
The instrumented function names have prefix of "pfs_rw_lock_" vs.
original name prefix of "rw_lock_". Following are list of functions
that have been instrumented:

rw_lock_create()
rw_lock_x_lock()
rw_lock_x_lock_gen()
rw_lock_x_lock_nowait()
rw_lock_x_unlock_gen()
rw_lock_s_lock()
rw_lock_s_lock_gen()
rw_lock_s_lock_nowait()
rw_lock_s_unlock_gen()
rw_lock_sx_lock()
rw_lock_sx_unlock_gen()
rw_lock_free()

