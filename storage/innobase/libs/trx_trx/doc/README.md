The transaction
===============



Latching protocol for trx_lock_t::que_state.
--------------------------------------------

trx_lock_t::que_state
 captures the state of the query thread during the execution of a query.
 This is different from a transaction state. The query state of a transaction
 can be updated asynchronously by other threads.  The other threads can be
 system threads, like the timeout monitor thread or user threads executing
 other queries. Another thing to be mindful of is that there is a delay between
 when a query thread is put into LOCK_WAIT state and before it actually starts
 waiting.  Between these two events it is possible that the query thread is
 granted the lock it was waiting for, which implies that the state can be
 changed asynchronously.

 All these operations take place within the context of locking. Therefore state
 changes within the locking code must acquire both the lock mutex and the
 trx->mutex when changing trx->lock.que_state to TRX_QUE_LOCK_WAIT or
 trx->lock.wait_lock to non-NULL but when the lock wait ends it is sufficient
 to only acquire the trx->mutex.
 To query the state either of the mutexes is sufficient within the locking
 code and no mutex is required when the query thread is no longer waiting.





The transaction handle
----------------------

Normally, there is a 1:1 relationship between a transaction handle
(trx) and a session (client connection). One session is associated
with exactly one user transaction. There are some exceptions to this:

* For DDL operations, a subtransaction is allocated that modifies the
data dictionary tables. Lock waits and deadlocks are prevented by
acquiring the dict_operation_lock before starting the subtransaction
and releasing it after committing the subtransaction.

* The purge system uses a special transaction that is not associated
with any session.

* If the system crashed or it was quickly shut down while there were
transactions in the ACTIVE or PREPARED state, these transactions would
no longer be associated with a session when the server is restarted.

A session may be served by at most one thread at a time. The serving
thread of a session might change in some MySQL implementations.
Therefore we do not have os_thread_get_curr_id() assertions in the code.

Normally, only the thread that is currently associated with a running
transaction may access (read and modify) the trx object, and it may do
so without holding any mutex. The following are exceptions to this:

* trx_rollback_resurrected() may access resurrected (connectionless)
transactions while the system is already processing new user
transactions. The trx_sys->mutex prevents a race condition between it
and lock_trx_release_locks() [invoked by trx_commit()].

* Print of transactions may access transactions not associated with
the current thread. The caller must be holding trx_sys->mutex and
lock_sys->mutex.

* When a transaction handle is in the trx_sys->mysql_trx_list or
trx_sys->trx_list, some of its fields must not be modified without
holding trx_sys->mutex exclusively.

* The locking code (in particular, deadlock checking and implicit to
explicit conversion) will access transactions associated to other
connections. The locks of transactions are protected by lock_sys->mutex
and sometimes by trx->mutex.

* Killing of asynchronous transactions.

