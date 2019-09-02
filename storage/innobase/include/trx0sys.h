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

/** @file include/trx0sys.h
 Transaction system

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#ifndef trx0sys_h
#define trx0sys_h

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxIdSet.h>
#include <innodb/trx_types/TrxSysMutex.h>
#include <innodb/trx_types/trx_sysf_t.h>
#include <innodb/trx_types/purge_pq_t.h>
#include <innodb/trx_types/Rsegs.h>
#include <innodb/tablespace/Space_Ids.h>
#include <innodb/tablespace/trx_sys_undo_spaces.h>
#include <innodb/tablespace/consts.h>
#include <innodb/page/header.h>
#include <innodb/trx_sys/trx_sys_t.h>

#include "buf0buf.h"
#include "fil0fil.h"
#ifndef UNIV_HOTBACKUP



#include <innodb/lst/lst.h>
#endif /* !UNIV_HOTBACKUP */
#include <atomic>
#include "trx0trx.h"

#ifndef UNIV_HOTBACKUP
#include <innodb/trx_sys/trx_ut_list_t.h>


// Forward declaration
class MVCC;
class ReadView;
struct trx_sys_t;

/** The transaction system */
extern trx_sys_t *trx_sys;

/** Checks if a page address is the trx sys header page.
@param[in]	page_id	page id
@return true if trx sys header page */
UNIV_INLINE
bool trx_sys_hdr_page(const page_id_t &page_id);

/** Creates and initializes the central memory structures for the transaction
 system. This is called when the database is started.
 @return min binary heap of rsegs to purge */
purge_pq_t *trx_sys_init_at_db_start(void);
/** Creates the trx_sys instance and initializes purge_queue and mutex. */
void trx_sys_create(void);
/** Creates and initializes the transaction system at the database creation. */
void trx_sys_create_sys_pages(void);

/** Find the page number in the TRX_SYS page for a given slot/rseg_id
@param[in]	rseg_id		slot number in the TRX_SYS page rseg array
@return page number from the TRX_SYS page rseg array */
page_no_t trx_sysf_rseg_find_page_no(ulint rseg_id);

/** Look for a free slot for a rollback segment in the trx system file copy.
@param[in,out]	mtr		mtr
@return slot index or ULINT_UNDEFINED if not found */
ulint trx_sysf_rseg_find_free(mtr_t *mtr);

/** Gets a pointer to the transaction system file copy and x-locks its page.
 @return pointer to system file copy, page x-locked */
UNIV_INLINE
trx_sysf_t *trx_sysf_get(mtr_t *mtr); /*!< in: mtr */

/** Gets the space of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	mtr		mtr
@return space id */
UNIV_INLINE
space_id_t trx_sysf_rseg_get_space(trx_sysf_t *sys_header, ulint i, mtr_t *mtr);

/** Gets the page number of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	mtr		mtr
@return page number, FIL_NULL if slot unused */
UNIV_INLINE
page_no_t trx_sysf_rseg_get_page_no(trx_sysf_t *sys_header, ulint i,
                                    mtr_t *mtr);

/** Sets the space id of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	space		space id
@param[in]	mtr		mtr */
UNIV_INLINE
void trx_sysf_rseg_set_space(trx_sysf_t *sys_header, ulint i, space_id_t space,
                             mtr_t *mtr);

/** Set the page number of the nth rollback segment slot in the trx system
file copy.
@param[in]	sys_header	trx sys file copy
@param[in]	i		slot index == rseg id
@param[in]	page_no		page number, FIL_NULL if the slot is reset to
                                unused
@param[in]	mtr		mtr */
UNIV_INLINE
void trx_sysf_rseg_set_page_no(trx_sysf_t *sys_header, ulint i,
                               page_no_t page_no, mtr_t *mtr);

/** Allocates a new transaction id.
 @return new, allocated trx id */
UNIV_INLINE
trx_id_t trx_sys_get_new_trx_id();
/** Determines the maximum transaction id.
 @return maximum currently allocated trx id; will be stale after the
 next call to trx_sys_get_new_trx_id() */
UNIV_INLINE
trx_id_t trx_sys_get_max_trx_id(void);

#ifdef UNIV_DEBUG
/* Flag to control TRX_RSEG_N_SLOTS behavior debugging. */
extern uint trx_rseg_n_slots_debug;
#endif
#endif /* !UNIV_HOTBACKUP */

/** Writes a trx id to an index page. In case that the id size changes in some
future version, this function should be used instead of mach_write_...
@param[in]	ptr	pointer to memory where written
@param[in]	id	id */
UNIV_INLINE
void trx_write_trx_id(byte *ptr, trx_id_t id);

#ifndef UNIV_HOTBACKUP
/** Reads a trx id from an index page. In case that the id size changes in
 some future version, this function should be used instead of
 mach_read_...
 @return id */
UNIV_INLINE
trx_id_t trx_read_trx_id(
    const byte *ptr); /*!< in: pointer to memory from where to read */

/** Looks for the trx instance with the given id in the rw trx_list.
 @return	the trx handle or NULL if not found */
UNIV_INLINE
trx_t *trx_get_rw_trx_by_id(trx_id_t trx_id); /*!< in: trx id to search for */
/** Returns the minimum trx id in rw trx list. This is the smallest id for which
 the trx can possibly be active. (But, you must look at the trx->state to
 find out if the minimum trx id transaction itself is active, or already
 committed.)
 @return the minimum trx id, or trx_sys->max_trx_id if the trx list is empty */
UNIV_INLINE
trx_id_t trx_rw_min_trx_id(void);

/** Checks if a rw transaction with the given id is active.
@param[in]	trx_id		trx id of the transaction
@param[in]	corrupt		NULL or pointer to a flag that will be set if
                                corrupt
@return transaction instance if active, or NULL */
UNIV_INLINE
trx_t *trx_rw_is_active_low(trx_id_t trx_id, ibool *corrupt);

/** Checks if a rw transaction with the given id is active. If the caller is
not holding trx_sys->mutex, the transaction may already have been committed.
@param[in]	trx_id		trx id of the transaction
@param[in]	corrupt		NULL or pointer to a flag that will be set if
                                corrupt
@param[in]	do_ref_count	if true then increment the trx_t::n_ref_count
@return transaction instance if active, or NULL; */
UNIV_INLINE
trx_t *trx_rw_is_active(trx_id_t trx_id, ibool *corrupt, bool do_ref_count);

#if defined UNIV_DEBUG || defined UNIV_BLOB_LIGHT_DEBUG
/** Assert that a transaction has been recovered.
 @return true */
UNIV_INLINE
ibool trx_assert_recovered(trx_id_t trx_id) /*!< in: transaction identifier */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG || UNIV_BLOB_LIGHT_DEBUG */
/** Updates the offset information about the end of the MySQL binlog entry
 which corresponds to the transaction just being committed. In a MySQL
 replication slave updates the latest master binlog position up to which
 replication has proceeded. */
void trx_sys_update_mysql_binlog_offset(
    const char *file_name, /*!< in: MySQL log file name */
    int64_t offset,        /*!< in: position in that log file */
    ulint field,           /*!< in: offset of the MySQL log info field in
                           the trx sys header */
    mtr_t *mtr);           /*!< in: mtr */
/** Prints to stderr the MySQL binlog offset info in the trx system header if
 the magic number shows it valid. */
void trx_sys_print_mysql_binlog_offset(void);
/** Shutdown/Close the transaction system. */
void trx_sys_close(void);

/** Determine if there are incomplete transactions in the system.
@return whether incomplete transactions need rollback */
UNIV_INLINE
bool trx_sys_need_rollback();

/*********************************************************************
Check if there are any active (non-prepared) transactions.
@return total number of active transactions or 0 if none */
ulint trx_sys_any_active_transactions(void);
#else  /* !UNIV_HOTBACKUP */
/** Prints to stderr the MySQL binlog info in the system header if the
 magic number shows it valid. */
void trx_sys_print_mysql_binlog_offset_from_page(
    const byte *page); /*!< in: buffer containing the trx
                       system header page, i.e., page number
                       TRX_SYS_PAGE_NO in the tablespace */
#endif /* !UNIV_HOTBACKUP */
/**
Add the transaction to the RW transaction set
@param trx		transaction instance to add */
UNIV_INLINE
void trx_sys_rw_trx_add(trx_t *trx);

#ifdef UNIV_DEBUG
/** Validate the trx_sys_t::rw_trx_list.
 @return true if the list is valid */
bool trx_sys_validate_trx_list();
#endif /* UNIV_DEBUG */

/** Initialize trx_sys_undo_spaces, called once during srv_start(). */
void trx_sys_undo_spaces_init();

/** Free the resources occupied by trx_sys_undo_spaces,
called once during thread de-initialization. */
void trx_sys_undo_spaces_deinit();


#include <innodb/trx_sys/flags.h>






/** When a trx id which is zero modulo this number (which must be a power of
two) is assigned, the field TRX_SYS_TRX_ID_STORE on the transaction system
page is updated */
#define TRX_SYS_TRX_ID_WRITE_MARGIN ((trx_id_t)256)

/** Test if trx_sys->mutex is owned. */
#define trx_sys_mutex_own() (trx_sys->mutex.is_owned())

/** Acquire the trx_sys->mutex. */
#define trx_sys_mutex_enter()     \
  do {                            \
    mutex_enter(&trx_sys->mutex); \
  } while (0)

/** Release the trx_sys->mutex. */
#define trx_sys_mutex_exit() \
  do {                       \
    trx_sys->mutex.exit();   \
  } while (0)

#include "trx0sys.ic"

#endif
