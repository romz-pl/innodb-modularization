
The read-write lock (for threads, not for database transactions)
================================================================

The macros should be used in rw locking and unlocking, not the corresponding function.


For performance schema instrumentation, a new set of rwlock
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
>
> rw_lock_create()
> rw_lock_x_lock()
> rw_lock_x_lock_gen()
> rw_lock_x_lock_nowait()
> rw_lock_x_unlock_gen()
> rw_lock_s_lock()
> rw_lock_s_lock_gen()
> rw_lock_s_lock_nowait()
> rw_lock_s_unlock_gen()
> rw_lock_sx_lock()
> rw_lock_sx_unlock_gen()
> rw_lock_free()
>

IMPLEMENTATION OF THE RW_LOCK
-----------------------------

The status of a rw_lock is held in lock_word. The initial value of lock_word is
X_LOCK_DECR. lock_word is decremented by 1 for each s-lock and by X_LOCK_DECR
or 1 for each x-lock. This describes the lock state for each value of lock_word:

- `lock_word == X_LOCK_DECR`:	Unlocked.

- `X_LOCK_HALF_DECR < lock_word < X_LOCK_DECR`:
                                S locked, no waiting writers.
                                (X_LOCK_DECR - lock_word) is the number
                                of S locks.

- `lock_word == X_LOCK_HALF_DECR`:	SX locked, no waiting writers.

- `0 < lock_word < X_LOCK_HALF_DECR`:
                                SX locked AND S locked, no waiting writers.
                                (X_LOCK_HALF_DECR - lock_word) is the number
                                of S locks.

- `lock_word == 0`:			X locked, no waiting writers.

- `-X_LOCK_HALF_DECR < lock_word < 0`:
                                S locked, with a waiting writer.
                                (-lock_word) is the number of S locks.

- `lock_word == -X_LOCK_HALF_DECR`:	X locked and SX locked, no waiting writers.

- `-X_LOCK_DECR < lock_word < -X_LOCK_HALF_DECR`:
                                S locked, with a waiting writer
                                which has SX lock.
                                -(lock_word + X_LOCK_HALF_DECR) is the number
                                of S locks.

- `lock_word == -X_LOCK_DECR`:	X locked with recursive X lock (2 X locks).

- `-(X_LOCK_DECR + X_LOCK_HALF_DECR) < lock_word < -X_LOCK_DECR`:
                                X locked. The number of the X locks is:
                                2 - (lock_word + X_LOCK_DECR)

- `lock_word == -(X_LOCK_DECR + X_LOCK_HALF_DECR)`:
                                X locked with recursive X lock (2 X locks)
                                and SX locked.

- `lock_word < -(X_LOCK_DECR + X_LOCK_HALF_DECR)`:
                                X locked and SX locked.
                                The number of the X locks is:
                                2 - (lock_word + X_LOCK_DECR + X_LOCK_HALF_DECR)

 LOCK COMPATIBILITY MATRIX
    S SX  X
 S  +  +  -
 SX +  -  -
 X  -  -  -

The lock_word is always read and updated atomically and consistently, so that
it always represents the state of the lock, and the state of the lock changes
with a single atomic operation. This lock_word holds all of the information
that a thread needs in order to determine if it is eligible to gain the lock
or if it must spin or sleep. The one exception to this is that writer_thread
must be verified before recursive write locks: to solve this scenario, we make
writer_thread readable by all threads, but only writeable by the x-lock or
sx-lock holder.

The other members of the lock obey the following rules to remain consistent:

recursive:	This and the writer_thread field together control the
                behaviour of recursive x-locking or sx-locking.
                lock->recursive must be FALSE in following states:
                        1) The writer_thread contains garbage i.e.: the
                        lock has just been initialized.
                        2) The lock is not x-held and there is no
                        x-waiter waiting on WAIT_EX event.
                        3) The lock is x-held or there is an x-waiter
                        waiting on WAIT_EX event but the 'pass' value
                        is non-zero.
                lock->recursive is TRUE iff:
                        1) The lock is x-held or there is an x-waiter
                        waiting on WAIT_EX event and the 'pass' value
                        is zero.
                This flag must be set after the writer_thread field
                has been updated with a memory ordering barrier.
                It is unset before the lock_word has been incremented.

writer_thread:	Is used only in recursive x-locking. Can only be safely
                read iff lock->recursive flag is TRUE.
                This field is uninitialized at lock creation time and
                is updated atomically when x-lock is acquired or when
                move_ownership is called. A thread is only allowed to
                set the value of this field to it's thread_id i.e.: a
                thread cannot set writer_thread to some other thread's
                id.

waiters:	May be set to 1 anytime, but to avoid unnecessary wake-up
                signals, it should only be set to 1 when there are threads
                waiting on event. Must be 1 when a writer starts waiting to
                ensure the current x-locking thread sends a wake-up signal
                during unlock. May only be reset to 0 immediately before a
                a wake-up signal is sent to event. On most platforms, a
                memory barrier is required after waiters is set, and before
                verifying lock_word is still held, to ensure some unlocker
                really does see the flags new value.

event:		Threads wait on event for read or writer lock when another
                thread has an x-lock or an x-lock reservation (wait_ex). A
                thread may only	wait on event after performing the following
                actions in order:
                   (1) Record the counter value of event (with os_event_reset).
                   (2) Set waiters to 1.
                   (3) Verify lock_word <= 0.
                (1) must come before (2) to ensure signal is not missed.
                (2) must come before (3) to ensure a signal is sent.
                These restrictions force the above ordering.
                Immediately before sending the wake-up signal, we should:
                   (1) Verify lock_word == X_LOCK_DECR (unlocked)
                   (2) Reset waiters to 0.

wait_ex_event:	A thread may only wait on the wait_ex_event after it has
                performed the following actions in order:
                   (1) Decrement lock_word by X_LOCK_DECR.
                   (2) Record counter value of wait_ex_event (os_event_reset,
                       called from sync_array_reserve_cell).
                   (3) Verify that lock_word < 0.
                (1) must come first to ensures no other threads become reader
                or next writer, and notifies unlocker that signal must be sent.
                (2) must come before (3) to ensure the signal is not missed.
                These restrictions force the above ordering.
                Immediately before sending the wake-up signal, we should:
                   Verify lock_word == 0 (waiting thread holds x_lock)



