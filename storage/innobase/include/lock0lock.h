/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/lock0lock.h
 The transaction lock system

 Created 5/7/1996 Heikki Tuuri
 *******************************************************/

#ifndef lock0lock_h
#define lock0lock_h

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/LockMutex.h>
#include <innodb/lock_sys/lock_sys_t.h>
#include <innodb/lock_types/trx_lock_list_t.h>
#include <innodb/lock_types/select_mode.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/lock_sys/lock_mutex_enter_nowait.h>
#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_sys/lock_mutex_enter.h>
#include <innodb/lock_sys/lock_mutex_exit.h>
#include <innodb/lock_sys/lock_wait_mutex_own.h>
#include <innodb/lock_sys/lock_wait_mutex_enter.h>
#include <innodb/lock_sys/lock_wait_mutex_exit.h>

struct lock_t;
struct lock_sys_t;
struct lock_table_t;

#include "que0types.h"

#include "srv0srv.h"
#include <innodb/vector/vector.h>
#ifndef UNIV_HOTBACKUP
#include "gis0rtree.h"
#endif /* UNIV_HOTBACKUP */
#include "lock0prdt.h"

struct trx_lock_t;

// Forward declaration
class ReadView;

extern bool innobase_deadlock_detect;


#include <innodb/lock_priv/lock_get_size.h>


/** Creates the lock system at database start. */
void lock_sys_create(
    ulint n_cells); /*!< in: number of slots in lock hash table */
/** Resize the lock hash table.
@param[in]	n_cells	number of slots in lock hash table */
void lock_sys_resize(ulint n_cells);

/** Closes the lock system at database shutdown. */
void lock_sys_close(void);


#include <innodb/lock_priv/lock_get_min_heap_no.h>
#include <innodb/lock_priv/lock_move_reorganize_page.h>
#include <innodb/lock_priv/lock_move_rec_list_end.h>
#include <innodb/lock_priv/lock_move_rec_list_start.h>
#include <innodb/lock_priv/lock_update_split_right.h>
#include <innodb/lock_priv/lock_update_merge_right.h>
#include <innodb/lock_priv/lock_update_root_raise.h>
#include <innodb/lock_priv/lock_update_copy_and_discard.h>
#include <innodb/lock_priv/lock_update_split_left.h>
#include <innodb/lock_priv/lock_update_merge_left.h>
#include <innodb/lock_priv/lock_rec_reset_and_inherit_gap_locks.h>
#include <innodb/lock_priv/lock_update_discard.h>
#include <innodb/lock_priv/lock_update_insert.h>
#include <innodb/lock_priv/lock_update_delete.h>
#include <innodb/lock_priv/lock_rec_store_on_page_infimum.h>
#include <innodb/lock_priv/lock_rec_restore_from_page_infimum.h>
#include <innodb/lock_priv/lock_rec_expl_exist_on_page.h>






/** Checks if locks of other transactions prevent an immediate insert of
 a record. If they do, first tests if the query thread should anyway
 be suspended for some reason; if not, then puts the transaction and
 the query thread to the lock wait state and inserts a waiting request
 for a gap x-lock to the lock queue.
 @return DB_SUCCESS, DB_LOCK_WAIT, or DB_DEADLOCK */
dberr_t lock_rec_insert_check_and_lock(
    ulint flags,         /*!< in: if BTR_NO_LOCKING_FLAG bit is
                         set, does nothing */
    const rec_t *rec,    /*!< in: record after which to insert */
    buf_block_t *block,  /*!< in/out: buffer block of rec */
    dict_index_t *index, /*!< in: index */
    que_thr_t *thr,      /*!< in: query thread */
    mtr_t *mtr,          /*!< in/out: mini-transaction */
    ibool *inherit)      /*!< out: set to TRUE if the new
                         inserted record maybe should inherit
                         LOCK_GAP type locks from the successor
                         record */
    MY_ATTRIBUTE((warn_unused_result));

/** Checks if locks of other transactions prevent an immediate modify (update,
 delete mark, or delete unmark) of a clustered index record. If they do,
 first tests if the query thread should anyway be suspended for some
 reason; if not, then puts the transaction and the query thread to the
 lock wait state and inserts a waiting request for a record x-lock to the
 lock queue.
 @return DB_SUCCESS, DB_LOCK_WAIT, or DB_DEADLOCK */
dberr_t lock_clust_rec_modify_check_and_lock(
    ulint flags,              /*!< in: if BTR_NO_LOCKING_FLAG
                              bit is set, does nothing */
    const buf_block_t *block, /*!< in: buffer block of rec */
    const rec_t *rec,         /*!< in: record which should be
                              modified */
    dict_index_t *index,      /*!< in: clustered index */
    const ulint *offsets,     /*!< in: rec_get_offsets(rec, index) */
    que_thr_t *thr)           /*!< in: query thread */
    MY_ATTRIBUTE((warn_unused_result));
/** Checks if locks of other transactions prevent an immediate modify
 (delete mark or delete unmark) of a secondary index record.
 @return DB_SUCCESS, DB_LOCK_WAIT, or DB_DEADLOCK */
dberr_t lock_sec_rec_modify_check_and_lock(
    ulint flags,         /*!< in: if BTR_NO_LOCKING_FLAG
                         bit is set, does nothing */
    buf_block_t *block,  /*!< in/out: buffer block of rec */
    const rec_t *rec,    /*!< in: record which should be
                         modified; NOTE: as this is a secondary
                         index, we always have to modify the
                         clustered index record first: see the
                         comment below */
    dict_index_t *index, /*!< in: secondary index */
    que_thr_t *thr,      /*!< in: query thread
                         (can be NULL if BTR_NO_LOCKING_FLAG) */
    mtr_t *mtr)          /*!< in/out: mini-transaction */
    MY_ATTRIBUTE((warn_unused_result));

/** Like lock_clust_rec_read_check_and_lock(), but reads a
secondary index record.
@param[in]	flags		if BTR_NO_LOCKING_FLAG bit is set, does nothing
@param[in]	block		buffer block of rec
@param[in]	rec		user record or page supremum record which should
                                be read or passed over by a read cursor
@param[in]	index		secondary index
@param[in]	offsets		rec_get_offsets(rec, index)
@param[in]	sel_mode	select mode: SELECT_ORDINARY,
                                SELECT_SKIP_LOKCED, or SELECT_NO_WAIT
@param[in]	mode		mode of the lock which the read cursor should
                                set on records: LOCK_S or LOCK_X; the latter is
                                possible in SELECT FOR UPDATE
@param[in]	gap_mode	LOCK_ORDINARY, LOCK_GAP, or LOCK_REC_NOT_GAP
@param[in,out]	thr		query thread
@return DB_SUCCESS, DB_SUCCESS_LOCKED_REC, DB_LOCK_WAIT, DB_DEADLOCK,
DB_SKIP_LOCKED, or DB_LOCK_NOWAIT */
dberr_t lock_sec_rec_read_check_and_lock(ulint flags, const buf_block_t *block,
                                         const rec_t *rec, dict_index_t *index,
                                         const ulint *offsets,
                                         select_mode sel_mode, lock_mode mode,
                                         ulint gap_mode, que_thr_t *thr);

/** Checks if locks of other transactions prevent an immediate read, or passing
over by a read cursor, of a clustered index record. If they do, first tests
if the query thread should anyway be suspended for some reason; if not, then
puts the transaction and the query thread to the lock wait state and inserts a
waiting request for a record lock to the lock queue. Sets the requested mode
lock on the record.
@param[in]	flags		if BTR_NO_LOCKING_FLAG bit is set, does nothing
@param[in]	block		buffer block of rec
@param[in]	rec		user record or page supremum record which should
                                be read or passed over by a read cursor
@param[in]	index		secondary index
@param[in]	offsets		rec_get_offsets(rec, index)
@param[in]	sel_mode	select mode: SELECT_ORDINARY,
                                SELECT_SKIP_LOKCED, or SELECT_NO_WAIT
@param[in]	mode		mode of the lock which the read cursor should
                                set on records: LOCK_S or LOCK_X; the latter is
                                possible in SELECT FOR UPDATE
@param[in]	gap_mode	LOCK_ORDINARY, LOCK_GAP, or LOCK_REC_NOT_GAP
@param[in,out]	thr		query thread
@return DB_SUCCESS, DB_SUCCESS_LOCKED_REC, DB_LOCK_WAIT, DB_DEADLOCK,
DB_SKIP_LOCKED, or DB_LOCK_NOWAIT */
dberr_t lock_clust_rec_read_check_and_lock(
    ulint flags, const buf_block_t *block, const rec_t *rec,
    dict_index_t *index, const ulint *offsets, select_mode sel_mode,
    lock_mode mode, ulint gap_mode, que_thr_t *thr);

/** Checks if locks of other transactions prevent an immediate read, or passing
 over by a read cursor, of a clustered index record. If they do, first tests
 if the query thread should anyway be suspended for some reason; if not, then
 puts the transaction and the query thread to the lock wait state and inserts a
 waiting request for a record lock to the lock queue. Sets the requested mode
 lock on the record. This is an alternative version of
 lock_clust_rec_read_check_and_lock() that does not require the parameter
 "offsets".
 @return DB_SUCCESS, DB_LOCK_WAIT, or DB_DEADLOCK */
dberr_t lock_clust_rec_read_check_and_lock_alt(
    ulint flags,              /*!< in: if BTR_NO_LOCKING_FLAG
                              bit is set, does nothing */
    const buf_block_t *block, /*!< in: buffer block of rec */
    const rec_t *rec,         /*!< in: user record or page
                              supremum record which should
                              be read or passed over by a
                              read cursor */
    dict_index_t *index,      /*!< in: clustered index */
    lock_mode mode,           /*!< in: mode of the lock which
                              the read cursor should set on
                              records: LOCK_S or LOCK_X; the
                              latter is possible in
                              SELECT FOR UPDATE */
    ulint gap_mode,           /*!< in: LOCK_ORDINARY, LOCK_GAP, or
                             LOCK_REC_NOT_GAP */
    que_thr_t *thr)           /*!< in: query thread */
    MY_ATTRIBUTE((warn_unused_result));
/** Checks that a record is seen in a consistent read.
 @return true if sees, or false if an earlier version of the record
 should be retrieved */
bool lock_clust_rec_cons_read_sees(
    const rec_t *rec,     /*!< in: user record which should be read or
                          passed over by a read cursor */
    dict_index_t *index,  /*!< in: clustered index */
    const ulint *offsets, /*!< in: rec_get_offsets(rec, index) */
    ReadView *view);      /*!< in: consistent read view */
/** Checks that a non-clustered index record is seen in a consistent read.

 NOTE that a non-clustered index page contains so little information on
 its modifications that also in the case false, the present version of
 rec may be the right, but we must check this from the clustered index
 record.

 @return true if certainly sees, or false if an earlier version of the
 clustered index record might be needed */
bool lock_sec_rec_cons_read_sees(
    const rec_t *rec,          /*!< in: user record which
                               should be read or passed over
                               by a read cursor */
    const dict_index_t *index, /*!< in: index */
    const ReadView *view)      /*!< in: consistent read view */
    MY_ATTRIBUTE((warn_unused_result));
/** Locks the specified database table in the mode given. If the lock cannot
 be granted immediately, the query thread is put to wait.
 @return DB_SUCCESS, DB_LOCK_WAIT, or DB_DEADLOCK */
dberr_t lock_table(ulint flags, /*!< in: if BTR_NO_LOCKING_FLAG bit is set,
                                does nothing */
                   dict_table_t *table, /*!< in/out: database table
                                        in dictionary cache */
                   lock_mode mode,      /*!< in: lock mode */
                   que_thr_t *thr)      /*!< in: query thread */
    MY_ATTRIBUTE((warn_unused_result));
/** Creates a table IX lock object for a resurrected transaction. */
void lock_table_ix_resurrect(dict_table_t *table, /*!< in/out: table */
                             trx_t *trx);         /*!< in/out: transaction */

/** Sets a lock on a table based on the given mode.
@param[in]	table	table to lock
@param[in,out]	trx	transaction
@param[in]	mode	LOCK_X or LOCK_S
@return error code or DB_SUCCESS. */
dberr_t lock_table_for_trx(dict_table_t *table, trx_t *trx, enum lock_mode mode)
    MY_ATTRIBUTE((nonnull, warn_unused_result));

/** Removes a granted record lock of a transaction from the queue and grants
 locks to other transactions waiting in the queue if they now are entitled
 to a lock. */
void lock_rec_unlock(
    trx_t *trx,               /*!< in/out: transaction that has
                              set a record lock */
    const buf_block_t *block, /*!< in: buffer block containing rec */
    const rec_t *rec,         /*!< in: record */
    lock_mode lock_mode);     /*!< in: LOCK_S or LOCK_X */
/** Releases a transaction's locks, and releases possible other transactions
 waiting because of these locks. Change the state of the transaction to
 TRX_STATE_COMMITTED_IN_MEMORY. */
void lock_trx_release_locks(trx_t *trx); /*!< in/out: transaction */

/** Release read locks of a transacion. It is called during XA
prepare to release locks early.
@param[in,out]	trx		transaction
@param[in]	only_gap	release only GAP locks */
void lock_trx_release_read_locks(trx_t *trx, bool only_gap);

/** Removes locks on a table to be dropped.
 If remove_also_table_sx_locks is TRUE then table-level S and X locks are
 also removed in addition to other table-level and record-level locks.
 No lock, that is going to be removed, is allowed to be a wait lock. */
void lock_remove_all_on_table(
    dict_table_t *table,               /*!< in: table to be dropped
                                       or discarded */
    ibool remove_also_table_sx_locks); /*!< in: also removes
                                   table S and X locks */



#include <innodb/lock_priv/lock_rec_hash.h>
#include <innodb/lock_priv/lock_hash_get.h>
#include <innodb/lock_priv/lock_rec_find_set_bit.h>
#include <innodb/lock_priv/lock_rec_find_next_set_bit.h>





/** Checks if a lock request lock1 has to wait for request lock2.
 @return TRUE if lock1 has to wait for lock2 to be removed */
bool lock_has_to_wait(
    const lock_t *lock1,  /*!< in: waiting lock */
    const lock_t *lock2); /*!< in: another lock; NOTE that it is
                          assumed that this has a lock bit set
                          on the same record as in lock1 if the
                          locks are record locks */
/** Reports that a transaction id is insensible, i.e., in the future. */
void lock_report_trx_id_insanity(
    trx_id_t trx_id,           /*!< in: trx id */
    const rec_t *rec,          /*!< in: user record */
    const dict_index_t *index, /*!< in: index */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    trx_id_t max_trx_id);      /*!< in: trx_sys_get_max_trx_id() */

/** Prints info of locks for all transactions.
@return false if not able to obtain lock mutex and exits without
printing info */
bool lock_print_info_summary(
    FILE *file,   /*!< in: file where to print */
    ibool nowait) /*!< in: whether to wait for the lock mutex */
    MY_ATTRIBUTE((warn_unused_result));

/** Prints transaction lock wait and MVCC state.
@param[in,out]	file	file where to print
@param[in]	trx	transaction */
void lock_trx_print_wait_and_mvcc_state(FILE *file, const trx_t *trx);

/** Prints info of locks for each transaction. This function assumes that the
 caller holds the lock mutex and more importantly it will release the lock
 mutex on behalf of the caller. (This should be fixed in the future). */
void lock_print_info_all_transactions(
    FILE *file); /*!< in: file where to print */
/** Return approximate number or record locks (bits set in the bitmap) for
 this transaction. Since delete-marked records may be removed, the
 record count will not be precise.
 The caller must be holding lock_sys->mutex. */
ulint lock_number_of_rows_locked(
    const trx_lock_t *trx_lock) /*!< in: transaction locks */
    MY_ATTRIBUTE((warn_unused_result));

/** Return the number of table locks for a transaction.
 The caller must be holding lock_sys->mutex. */
ulint lock_number_of_tables_locked(
    const trx_lock_t *trx_lock) /*!< in: transaction locks */
    MY_ATTRIBUTE((warn_unused_result));

/** Gets the type of a lock. Non-inline version for using outside of the
 lock module.
 @return LOCK_TABLE or LOCK_REC */
uint32_t lock_get_type(const lock_t *lock); /*!< in: lock */

/** Gets the id of the transaction owning a lock.
@param[in]  lock  A lock of the transaction we are interested in
@return the transaction's id */
trx_id_t lock_get_trx_id(const lock_t *lock);

/** Gets the immutable id of the transaction owning a lock
@param[in]  lock   A lock of the transaction we are interested in
@return the transaction's immutable id */
uint64_t lock_get_trx_immutable_id(const lock_t *lock);

/** Gets the immutable id of this lock.
@param[in]  lock   The lock we are interested in
@return The lock's immutable id */
uint64_t lock_get_immutable_id(const lock_t *lock);

/** Get the performance schema event (thread_id, event_id)
that created the lock.
@param[in]	lock		Lock
@param[out]	thread_id	Thread ID that created the lock
@param[out]	event_id	Event ID that created the lock
*/
void lock_get_psi_event(const lock_t *lock, ulonglong *thread_id,
                        ulonglong *event_id);

/** Get the first lock of a trx lock list.
@param[in]	trx_lock	the trx lock
@return The first lock
*/
const lock_t *lock_get_first_trx_locks(const trx_lock_t *trx_lock);

/** Get the next lock of a trx lock list.
@param[in]	lock	the current lock
@return The next lock
*/
const lock_t *lock_get_next_trx_locks(const lock_t *lock);

/** Gets the mode of a lock in a human readable string.
 The string should not be free()'d or modified.
 @return lock mode */
const char *lock_get_mode_str(const lock_t *lock); /*!< in: lock */

/** Gets the type of a lock in a human readable string.
 The string should not be free()'d or modified.
 @return lock type */
const char *lock_get_type_str(const lock_t *lock); /*!< in: lock */

/** Gets the id of the table on which the lock is.
 @return id of the table */
table_id_t lock_get_table_id(const lock_t *lock); /*!< in: lock */

/** Determine which table a lock is associated with.
@param[in]	lock	the lock
@return name of the table */
const table_name_t &lock_get_table_name(const lock_t *lock);

/** For a record lock, gets the index on which the lock is.
 @return index */
const dict_index_t *lock_rec_get_index(const lock_t *lock); /*!< in: lock */

/** For a record lock, gets the name of the index on which the lock is.
 The string should not be free()'d or modified.
 @return name of the index */
const char *lock_rec_get_index_name(const lock_t *lock); /*!< in: lock */

/** For a record lock, gets the tablespace number on which the lock is.
 @return tablespace number */
space_id_t lock_rec_get_space_id(const lock_t *lock); /*!< in: lock */

/** For a record lock, gets the page number on which the lock is.
 @return page number */
page_no_t lock_rec_get_page_no(const lock_t *lock); /*!< in: lock */
/** Check if there are any locks (table or rec) against table.
 @return true if locks exist */
bool lock_table_has_locks(
    const dict_table_t *table); /*!< in: check if there are any locks
                                held on records in this table or on the
                                table itself */

/** A thread which wakes up threads whose lock wait may have lasted too long. */
void lock_wait_timeout_thread();

/** Puts a user OS thread to wait for a lock to be released. If an error
 occurs during the wait trx->error_state associated with thr is
 != DB_SUCCESS when we return. DB_LOCK_WAIT_TIMEOUT and DB_DEADLOCK
 are possible errors. DB_DEADLOCK is returned if selective deadlock
 resolution chose this transaction as a victim. */
void lock_wait_suspend_thread(
    que_thr_t *thr); /*!< in: query thread associated with the
                     user OS thread */
/** Unlocks AUTO_INC type locks that were possibly reserved by a trx. This
 function should be called at the the end of an SQL statement, by the
 connection thread that owns the transaction (trx->mysql_thd). */
void lock_unlock_table_autoinc(trx_t *trx); /*!< in/out: transaction */
/** Check whether the transaction has already been rolled back because it
 was selected as a deadlock victim, or if it has to wait then cancel
 the wait lock.
 @return DB_DEADLOCK, DB_LOCK_WAIT or DB_SUCCESS */
dberr_t lock_trx_handle_wait(trx_t *trx); /*!< in/out: trx lock state */
/** Initialise the trx lock list. */
void lock_trx_lock_list_init(
    trx_lock_list_t *lock_list); /*!< List to initialise */

/** Set the lock system timeout event. */
void lock_set_timeout_event();
#ifdef UNIV_DEBUG
/** Checks that a transaction id is sensible, i.e., not in the future.
 @return true if ok */
bool lock_check_trx_id_sanity(
    trx_id_t trx_id,           /*!< in: trx id */
    const rec_t *rec,          /*!< in: user record */
    const dict_index_t *index, /*!< in: index */
    const ulint *offsets)      /*!< in: rec_get_offsets(rec, index) */
    MY_ATTRIBUTE((warn_unused_result));
/** Check if the transaction holds an exclusive lock on a record.
 @return whether the locks are held */
bool lock_trx_has_rec_x_lock(
    const trx_t *trx,          /*!< in: transaction to check */
    const dict_table_t *table, /*!< in: table to check */
    const buf_block_t *block,  /*!< in: buffer block of the record */
    ulint heap_no)             /*!< in: record heap number */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG */

/**
Allocate cached locks for the transaction.
@param trx		allocate cached record locks for this transaction */
void lock_trx_alloc_locks(trx_t *trx);



/** Lock operation struct */
struct lock_op_t {
  dict_table_t *table; /*!< table to be locked */
  lock_mode mode;      /*!< lock mode */
};



/*********************************************************************/ /**
This function is kind of wrapper to lock_rec_convert_impl_to_expl_for_trx()
function with functionailty added to facilitate lock conversion from implicit
to explicit for partial rollback cases
@param[in]	block		buffer block of rec
@param[in]	rec		user record on page
@param[in]	index		index of record
@param[in]	offsets		rec_get_offsets(rec, index)
@param[in,out]	trx		active transaction
@param[in]	heap_no		rec heap number to lock */
void lock_rec_convert_active_impl_to_expl(const buf_block_t *block,
                                          const rec_t *rec, dict_index_t *index,
                                          const ulint *offsets, trx_t *trx,
                                          ulint heap_no);

/** Removes a record lock request, waiting or granted, from the queue. */
void lock_rec_discard(lock_t *in_lock); /*!< in: record lock object: all
                                        record locks which are contained
                                        in this lock object are removed */

/** Moves the explicit locks on user records to another page if a record
 list start is moved to another page. */
void lock_rtr_move_rec_list(const buf_block_t *new_block, /*!< in: index page to
                                                          move to */
                            const buf_block_t *block,     /*!< in: index page */
                            rtr_rec_move_t *rec_move, /*!< in: recording records
                                                      moved */
                            ulint num_move); /*!< in: num of rec to move */

/** Removes record lock objects set on an index page which is discarded. This
 function does not move locks, or check for waiting locks, therefore the
 lock bitmaps must already be reset when this function is called. */
void lock_rec_free_all_from_discard_page(
    const buf_block_t *block); /*!< in: page to be discarded */

#include <innodb/lock_priv/lock_rec_trx_wait.h>

















#include "lock0lock.ic"

#endif
