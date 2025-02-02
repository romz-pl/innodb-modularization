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

/** @file trx/trx0sys.cc
 Transaction system

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#include <sys/types.h>
#include <new>

#include <innodb/read/ReadView.h>
#include <innodb/trx_undo/flags.h>
#include <innodb/trx_purge/trx_purge_sys_close.h>
#include <innodb/trx_rseq/trx_rsegs_init.h>
#include <innodb/trx_rseq/trx_rseg_header_create.h>
#include <innodb/trx_undo/trx_undo_t.h>
#include <innodb/tablespace/trx_sys_undo_spaces.h>
#include <innodb/trx_sys/trx_sys_flush_max_trx_id.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/trx_trx/trx_reference.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/assert_trx_in_rw_list.h>
#include <innodb/trx_types/trx_sysf_t.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/read/MVCC.h>
#include <innodb/trx_trx/trx_free_prepared.h>
#include <innodb/trx_trx/trx_lists_init_at_db_start.h>
#include <innodb/trx_trx/trx_dummy_sess.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/mtr/mtr_x_lock_space.h>
#include <innodb/mtr/mtr_start.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/string/ut_strlen.h>

#include "current_thd.h"
#include "ha_prototypes.h"
#include "sql_error.h"


#ifndef UNIV_HOTBACKUP
#include "fsp0fsp.h"

#include "log0log.h"
#include "log0recv.h"
#include "mtr0log.h"

#include "srv0srv.h"
#include "srv0start.h"


#include "page0page.h"
#include "usr0sess.h"
#include "buf0dblwr.h"




/////////////////////////////



/** Checks if a page address is the trx sys header page.
@param[in]	page_id	page id
@return true if trx sys header page */
bool trx_sys_hdr_page(const page_id_t &page_id) {
  return (page_id.space() == TRX_SYS_SPACE &&
          page_id.page_no() == TRX_SYS_PAGE_NO);
}

/** Gets a pointer to the transaction system header and x-latches its page.
 @return pointer to system header, page x-latched. */
trx_sysf_t *trx_sysf_get(mtr_t *mtr) /*!< in: mtr */
{
  buf_block_t *block;
  trx_sysf_t *header;

  ut_ad(mtr);

  block = buf_page_get(page_id_t(TRX_SYS_SPACE, TRX_SYS_PAGE_NO),
                       univ_page_size, RW_X_LATCH, mtr);
  buf_block_dbg_add_level(block, SYNC_TRX_SYS_HEADER);

  header = TRX_SYS + buf_block_get_frame(block);

  return (header);
}

/** Gets the space of the nth rollback segment slot in the trx system
 file copy.
 @return space id */
space_id_t trx_sysf_rseg_get_space(
    trx_sysf_t *sys_header, /*!< in: trx sys header */
    ulint slot,             /*!< in: slot index == rseg id */
    mtr_t *mtr)             /*!< in: mtr */
{
  ut_ad(sys_header);
  ut_ad(slot < TRX_SYS_N_RSEGS);

  return (mtr_read_ulint(sys_header + TRX_SYS_RSEGS +
                             slot * TRX_SYS_RSEG_SLOT_SIZE + TRX_SYS_RSEG_SPACE,
                         MLOG_4BYTES, mtr));
}

/** Gets the page number of the nth rollback segment slot in the trx system
 header.
 @return page number, FIL_NULL if slot unused */
page_no_t trx_sysf_rseg_get_page_no(
    trx_sysf_t *sys_header, /*!< in: trx system header */
    ulint slot,             /*!< in: slot index == rseg id */
    mtr_t *mtr)             /*!< in: mtr */
{
  ut_ad(sys_header);
  ut_ad(slot < TRX_SYS_N_RSEGS);

  return (mtr_read_ulint(sys_header + TRX_SYS_RSEGS +
                             slot * TRX_SYS_RSEG_SLOT_SIZE +
                             TRX_SYS_RSEG_PAGE_NO,
                         MLOG_4BYTES, mtr));
}

/** Sets the space id of the nth rollback segment slot in the trx system
 file copy. */
void trx_sysf_rseg_set_space(
    trx_sysf_t *sys_header, /*!< in: trx sys file copy */
    ulint slot,             /*!< in: slot index == rseg id */
    space_id_t space,       /*!< in: space id */
    mtr_t *mtr)             /*!< in: mtr */
{
  ut_ad(sys_header);
  ut_ad(slot < TRX_SYS_N_RSEGS);

  mlog_write_ulint(sys_header + TRX_SYS_RSEGS + slot * TRX_SYS_RSEG_SLOT_SIZE +
                       TRX_SYS_RSEG_SPACE,
                   space, MLOG_4BYTES, mtr);
}

/** Sets the page number of the nth rollback segment slot in the trx system
 header. */
void trx_sysf_rseg_set_page_no(
    trx_sysf_t *sys_header, /*!< in: trx sys header */
    ulint slot,             /*!< in: slot index == rseg id */
    page_no_t page_no,      /*!< in: page number, FIL_NULL if the
                            slot is reset to unused */
    mtr_t *mtr)             /*!< in: mtr */
{
  ut_ad(sys_header);
  ut_ad(slot < TRX_SYS_N_RSEGS);

  mlog_write_ulint(sys_header + TRX_SYS_RSEGS + slot * TRX_SYS_RSEG_SLOT_SIZE +
                       TRX_SYS_RSEG_PAGE_NO,
                   page_no, MLOG_4BYTES, mtr);
}
#endif /* !UNIV_HOTBACKUP */

/** Writes a trx id to an index page. In case that the id size changes in
 some future version, this function should be used instead of
 mach_write_... */
void trx_write_trx_id(byte *ptr,   /*!< in: pointer to memory where written */
                      trx_id_t id) /*!< in: id */
{
  ut_ad(id > 0);
  mach_write_to_6(ptr, id);
}

#ifndef UNIV_HOTBACKUP
/** Reads a trx id from an index page. In case that the id size changes in
 some future version, this function should be used instead of
 mach_read_...
 @return id */
trx_id_t trx_read_trx_id(
    const byte *ptr) /*!< in: pointer to memory from where to read */
{
  return (mach_read_from_6(ptr));
}

/** Looks for the trx handle with the given id in rw_trx_list.
 The caller must be holding trx_sys->mutex.
 @return the trx handle or NULL if not found;
 the pointer must not be dereferenced unless lock_sys->mutex was
 acquired before calling this function and is still being held */
trx_t *trx_get_rw_trx_by_id(trx_id_t trx_id) /*!< in: trx id to search for */
{
  ut_ad(trx_id > 0);
  ut_ad(trx_sys_mutex_own());

  if (trx_sys->rw_trx_set.empty()) {
    return (NULL);
  }

  TrxIdSet::iterator it;

  it = trx_sys->rw_trx_set.find(TrxTrack(trx_id));

  return (it == trx_sys->rw_trx_set.end() ? NULL : it->m_trx);
}

/** Returns the minimum trx id in trx list. This is the smallest id for which
 the trx can possibly be active. (But, you must look at the trx->state
 to find out if the minimum trx id transaction itself is active, or already
 committed.). The caller must be holding the trx_sys_t::mutex in shared mode.
 @return the minimum trx id, or trx_sys->max_trx_id if the trx list is empty */
UNIV_INLINE
trx_id_t trx_rw_min_trx_id_low(void) {
  trx_id_t id;

  ut_ad(trx_sys_mutex_own());

  const trx_t *trx = UT_LIST_GET_LAST(trx_sys->rw_trx_list);

  if (trx == NULL) {
    id = trx_sys->max_trx_id;
  } else {
    assert_trx_in_rw_list(trx);
    id = trx->id;
  }

  return (id);
}



/** Returns the minimum trx id in rw trx list. This is the smallest id for which
 the rw trx can possibly be active. (But, you must look at the trx->state
 to find out if the minimum trx id transaction itself is active, or already
 committed.)
 @return the minimum trx id, or trx_sys->max_trx_id if rw trx list is empty */
trx_id_t trx_rw_min_trx_id(void) {
  trx_sys_mutex_enter();

  trx_id_t id = trx_rw_min_trx_id_low();

  trx_sys_mutex_exit();

  return (id);
}

/** Checks if a rw transaction with the given id is active.  If the caller is
 not holding lock_sys->mutex, the transaction may already have been committed.
 @return transaction instance if active, or NULL */
trx_t *trx_rw_is_active_low(
    trx_id_t trx_id, /*!< in: trx id of the transaction */
    ibool *corrupt)  /*!< in: NULL or pointer to a flag
                     that will be set if corrupt */
{
  trx_t *trx;

  ut_ad(trx_sys_mutex_own());

  if (trx_id < trx_rw_min_trx_id_low()) {
    trx = NULL;
  } else if (trx_id >= trx_sys->max_trx_id) {
    /* There must be corruption: we let the caller handle the
    diagnostic prints in this case. */

    trx = NULL;
    if (corrupt != NULL) {
      *corrupt = TRUE;
    }
  } else {
    trx = trx_get_rw_trx_by_id(trx_id);
    /* We remove trx from rw trxs list and change state to
    TRX_STATE_COMMITED_IN_MEMORY in a same critical section protected by
    trx_sys->mutex, which we happen to hold here, so we expect the state of trx
    to match its presence in that list */
    ut_ad(trx == NULL || !trx_state_eq(trx, TRX_STATE_COMMITTED_IN_MEMORY));
  }

  return (trx);
}

/** Checks if a rw transaction with the given id is active. If the caller is
 not holding lock_sys->mutex, the transaction may already have been
 committed.
 @return transaction instance if active, or NULL; */
trx_t *trx_rw_is_active(trx_id_t trx_id,   /*!< in: trx id of the transaction */
                        ibool *corrupt,    /*!< in: NULL or pointer to a flag
                                           that will be set if corrupt */
                        bool do_ref_count) /*!< in: if true then increment the
                                           trx_t::n_ref_count */
{
  trx_t *trx;

  /* Fast checking. If it's smaller than minimal active trx id, just
  return NULL. */
  if (trx_sys->min_active_id.load() > trx_id) {
    return (NULL);
  }

  trx_sys_mutex_enter();

  trx = trx_rw_is_active_low(trx_id, corrupt);

  if (trx != 0) {
    trx = trx_reference(trx, do_ref_count);
  }

  trx_sys_mutex_exit();

  return (trx);
}

/** Allocates a new transaction id.
 @return new, allocated trx id */
trx_id_t trx_sys_get_new_trx_id() {
  ut_ad(trx_sys_mutex_own());

  /* VERY important: after the database is started, max_trx_id value is
  divisible by TRX_SYS_TRX_ID_WRITE_MARGIN, and the following if
  will evaluate to TRUE when this function is first time called,
  and the value for trx id will be written to disk-based header!
  Thus trx id values will not overlap when the database is
  repeatedly started! */

  if (!(trx_sys->max_trx_id % TRX_SYS_TRX_ID_WRITE_MARGIN)) {
    trx_sys_flush_max_trx_id();
  }

  return (trx_sys->max_trx_id++);
}



/** Determine if there are incomplete transactions in the system.
@return whether incomplete transactions need rollback */
bool trx_sys_need_rollback() {
  ulint n_trx;

  trx_sys_mutex_enter();

  n_trx = UT_LIST_GET_LEN(trx_sys->rw_trx_list);
  ut_ad(n_trx >= trx_sys->n_prepared_trx);
  n_trx -= trx_sys->n_prepared_trx;

  trx_sys_mutex_exit();

  return (n_trx > 0);
}

/**
Add the transaction to the RW transaction set
@param trx		transaction instance to add */
void trx_sys_rw_trx_add(trx_t *trx) {
  ut_ad(trx->id != 0);

  trx_sys->rw_trx_set.insert(TrxTrack(trx->id, trx));
  ut_d(trx->in_rw_trx_list = true);
}


///////////////////////////////



/** Check whether transaction id is valid.
@param[in]	id	transaction id to check
@param[in]	name	table name */
void ReadView::check_trx_id_sanity(trx_id_t id, const table_name_t &name) {
  if (&name == &dict_sys->dynamic_metadata->name) {
    /* The table mysql.innodb_dynamic_metadata uses a
    constant DB_TRX_ID=~0. */
    ut_ad(id == (1ULL << 48) - 1);
    return;
  }

  if (id >= trx_sys->max_trx_id) {
    ib::warn(ER_IB_MSG_1196)
        << "A transaction id"
        << " in a record of table " << name << " is newer than the"
        << " system-wide maximum.";
    ut_ad(0);
    THD *thd = current_thd;
    if (thd != NULL) {
      char table_name[MAX_FULL_NAME_LEN + 1];

      innobase_format_name(table_name, sizeof(table_name), name.m_name);

      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_SIGNAL_WARN,
                          "InnoDB: Transaction id"
                          " in a record of table"
                          " %s is newer than system-wide"
                          " maximum.",
                          table_name);
    }
  }
}



/** Writes the value of max_trx_id to the file based trx system header. */
void trx_sys_flush_max_trx_id(void) {
  mtr_t mtr;
  trx_sysf_t *sys_header;

  ut_ad(trx_sys_mutex_own());

  if (!srv_read_only_mode) {
    mtr_start(&mtr);

    sys_header = trx_sysf_get(&mtr);

    mlog_write_ull(sys_header + TRX_SYS_TRX_ID_STORE, trx_sys->max_trx_id,
                   &mtr);

    mtr_commit(&mtr);
  }
}

/** Updates the offset information about the end of the MySQL binlog entry
 which corresponds to the transaction just being committed. In a MySQL
 replication slave updates the latest master binlog position up to which
 replication has proceeded. */
void trx_sys_update_mysql_binlog_offset(
    const char *file_name, /*!< in: MySQL log file name */
    int64_t offset,        /*!< in: position in that log file */
    ulint field,           /*!< in: offset of the MySQL log info field in
                           the trx sys header */
    mtr_t *mtr)            /*!< in: mtr */
{
  trx_sysf_t *sys_header;

  if (ut_strlen(file_name) >= TRX_SYS_MYSQL_LOG_NAME_LEN) {
    /* We cannot fit the name to the 512 bytes we have reserved */

    return;
  }

  sys_header = trx_sysf_get(mtr);

  if (mach_read_from_4(sys_header + field + TRX_SYS_MYSQL_LOG_MAGIC_N_FLD) !=
      TRX_SYS_MYSQL_LOG_MAGIC_N) {
    mlog_write_ulint(sys_header + field + TRX_SYS_MYSQL_LOG_MAGIC_N_FLD,
                     TRX_SYS_MYSQL_LOG_MAGIC_N, MLOG_4BYTES, mtr);
  }

  if (0 != strcmp((char *)(sys_header + field + TRX_SYS_MYSQL_LOG_NAME),
                  file_name)) {
    mlog_write_string(sys_header + field + TRX_SYS_MYSQL_LOG_NAME,
                      (byte *)file_name, 1 + ut_strlen(file_name), mtr);
  }

  if (mach_read_from_4(sys_header + field + TRX_SYS_MYSQL_LOG_OFFSET_HIGH) >
          0 ||
      (offset >> 32) > 0) {
    mlog_write_ulint(sys_header + field + TRX_SYS_MYSQL_LOG_OFFSET_HIGH,
                     (ulint)(offset >> 32), MLOG_4BYTES, mtr);
  }

  mlog_write_ulint(sys_header + field + TRX_SYS_MYSQL_LOG_OFFSET_LOW,
                   (ulint)(offset & 0xFFFFFFFFUL), MLOG_4BYTES, mtr);
}

/** Stores the MySQL binlog offset info in the trx system header if
 the magic number shows it valid, and print the info to stderr */
void trx_sys_print_mysql_binlog_offset(void) {
  trx_sysf_t *sys_header;
  mtr_t mtr;
  ulint trx_sys_mysql_bin_log_pos_high;
  ulint trx_sys_mysql_bin_log_pos_low;

  mtr_start(&mtr);

  sys_header = trx_sysf_get(&mtr);

  if (mach_read_from_4(sys_header + TRX_SYS_MYSQL_LOG_INFO +
                       TRX_SYS_MYSQL_LOG_MAGIC_N_FLD) !=
      TRX_SYS_MYSQL_LOG_MAGIC_N) {
    mtr_commit(&mtr);

    return;
  }

  trx_sys_mysql_bin_log_pos_high = mach_read_from_4(
      sys_header + TRX_SYS_MYSQL_LOG_INFO + TRX_SYS_MYSQL_LOG_OFFSET_HIGH);
  trx_sys_mysql_bin_log_pos_low = mach_read_from_4(
      sys_header + TRX_SYS_MYSQL_LOG_INFO + TRX_SYS_MYSQL_LOG_OFFSET_LOW);

  ib::info(ER_IB_MSG_1197) << "Last MySQL binlog file position "
                           << trx_sys_mysql_bin_log_pos_high << " "
                           << trx_sys_mysql_bin_log_pos_low << ", file name "
                           << sys_header + TRX_SYS_MYSQL_LOG_INFO +
                                  TRX_SYS_MYSQL_LOG_NAME;

  mtr_commit(&mtr);
}

/** Find the page number in the TRX_SYS page for a given slot/rseg_id
@param[in]	rseg_id		slot number in the TRX_SYS page rseg array
@return page number from the TRX_SYS page rseg array */
page_no_t trx_sysf_rseg_find_page_no(ulint rseg_id) {
  page_no_t page_no;
  mtr_t mtr;
  mtr.start();

  trx_sysf_t *sys_header = trx_sysf_get(&mtr);

  page_no = trx_sysf_rseg_get_page_no(sys_header, rseg_id, &mtr);

  mtr.commit();

  return (page_no);
}

/** Look for a free slot for a rollback segment in the trx system file copy.
@param[in,out]	mtr		mtr
@return slot index or ULINT_UNDEFINED if not found */
ulint trx_sysf_rseg_find_free(mtr_t *mtr) {
  trx_sysf_t *sys_header = trx_sysf_get(mtr);

  for (ulint slot_no = 0; slot_no < TRX_SYS_N_RSEGS; slot_no++) {
    page_no_t page_no = trx_sysf_rseg_get_page_no(sys_header, slot_no, mtr);

    if (page_no == FIL_NULL) {
      return (slot_no);
    }
  }

  return (ULINT_UNDEFINED);
}

/** Creates the file page for the transaction system. This function is called
 only at the database creation, before trx_sys_init. */
static void trx_sysf_create(mtr_t *mtr) /*!< in: mtr */
{
  trx_sysf_t *sys_header;
  ulint slot_no;
  buf_block_t *block;
  page_t *page;
  ulint page_no;
  byte *ptr;
  ulint len;

  ut_ad(mtr);

  /* Note that below we first reserve the file space x-latch, and
  then enter the kernel: we must do it in this order to conform
  to the latching order rules. */

  mtr_x_lock_space(fil_space_get_sys_space(), mtr);

  /* Create the trx sys file block in a new allocated file segment */
  block = fseg_create(TRX_SYS_SPACE, 0, TRX_SYS + TRX_SYS_FSEG_HEADER, mtr);
  buf_block_dbg_add_level(block, SYNC_TRX_SYS_HEADER);

  ut_a(block->page.id.page_no() == TRX_SYS_PAGE_NO);

  page = buf_block_get_frame(block);

  mlog_write_ulint(page + FIL_PAGE_TYPE, FIL_PAGE_TYPE_TRX_SYS, MLOG_2BYTES,
                   mtr);

  /* Reset the doublewrite buffer magic number to zero so that we
  know that the doublewrite buffer has not yet been created (this
  suppresses a Valgrind warning) */

  mlog_write_ulint(page + TRX_SYS_DOUBLEWRITE + TRX_SYS_DOUBLEWRITE_MAGIC, 0,
                   MLOG_4BYTES, mtr);

  sys_header = trx_sysf_get(mtr);

  /* Start counting transaction ids from number 1 up */
  mach_write_to_8(sys_header + TRX_SYS_TRX_ID_STORE, 1);

  /* Reset the rollback segment slots.  Old versions of InnoDB
  define TRX_SYS_N_RSEGS as 256 (TRX_SYS_OLD_N_RSEGS) and expect
  that the whole array is initialized. */
  ptr = TRX_SYS_RSEGS + sys_header;
  len = std::max(TRX_SYS_OLD_N_RSEGS, TRX_SYS_N_RSEGS) * TRX_SYS_RSEG_SLOT_SIZE;
  memset(ptr, 0xff, len);
  ptr += len;
  ut_a(ptr <= page + (UNIV_PAGE_SIZE - FIL_PAGE_DATA_END));

  /* Initialize all of the page.  This part used to be uninitialized. */
  memset(ptr, 0, UNIV_PAGE_SIZE - FIL_PAGE_DATA_END + page - ptr);

  mlog_log_string(sys_header,
                  UNIV_PAGE_SIZE - FIL_PAGE_DATA_END + page - sys_header, mtr);

  /* Create the first rollback segment in the SYSTEM tablespace */
  slot_no = trx_sysf_rseg_find_free(mtr);
  page_no = trx_rseg_header_create(TRX_SYS_SPACE, univ_page_size, PAGE_NO_MAX,
                                   slot_no, mtr);

  ut_a(slot_no == TRX_SYS_SYSTEM_RSEG_ID);
  ut_a(page_no == FSP_FIRST_RSEG_PAGE_NO);
}

/** Creates and initializes the central memory structures for the transaction
 system. This is called when the database is started.
 @return min binary heap of rsegs to purge */
purge_pq_t *trx_sys_init_at_db_start(void) {
  purge_pq_t *purge_queue;
  trx_sysf_t *sys_header;
  ib_uint64_t rows_to_undo = 0;
  const char *unit = "";

  /* We create the min binary heap here and pass ownership to
  purge when we init the purge sub-system. Purge is responsible
  for freeing the binary heap. */
  purge_queue = UT_NEW_NOKEY(purge_pq_t());
  ut_a(purge_queue != NULL);

  if (srv_force_recovery < SRV_FORCE_NO_UNDO_LOG_SCAN) {
    /* Create the memory objects for all the rollback segments
    referred to in the TRX_SYS page or any undo tablespace
    RSEG_ARRAY page. */
    trx_rsegs_init(purge_queue);
  }

  /* VERY important: after the database is started, max_trx_id value is
  divisible by TRX_SYS_TRX_ID_WRITE_MARGIN, and the 'if' in
  trx_sys_get_new_trx_id will evaluate to TRUE when the function
  is first time called, and the value for trx id will be written
  to the disk-based header! Thus trx id values will not overlap when
  the database is repeatedly started! */

  mtr_t mtr;
  mtr.start();

  sys_header = trx_sysf_get(&mtr);

  trx_sys->max_trx_id =
      2 * TRX_SYS_TRX_ID_WRITE_MARGIN +
      ut_uint64_align_up(mach_read_from_8(sys_header + TRX_SYS_TRX_ID_STORE),
                         TRX_SYS_TRX_ID_WRITE_MARGIN);

  mtr.commit();
  ut_d(trx_sys->rw_max_trx_id = trx_sys->max_trx_id);

  trx_dummy_sess = sess_open();

  trx_lists_init_at_db_start();

  /* This mutex is not strictly required, it is here only to satisfy
  the debug code (assertions). We are still running in single threaded
  bootstrap mode. */

  trx_sys_mutex_enter();

  if (UT_LIST_GET_LEN(trx_sys->rw_trx_list) > 0) {
    const trx_t *trx;

    for (trx = UT_LIST_GET_FIRST(trx_sys->rw_trx_list); trx != NULL;
         trx = UT_LIST_GET_NEXT(trx_list, trx)) {
      ut_ad(trx->is_recovered);
      assert_trx_in_rw_list(trx);

      if (trx_state_eq(trx, TRX_STATE_ACTIVE)) {
        rows_to_undo += trx->undo_no;
      }
    }

    if (rows_to_undo > 1000000000) {
      unit = "M";
      rows_to_undo = rows_to_undo / 1000000;
    }

    ib::info(ER_IB_MSG_1198)
        << UT_LIST_GET_LEN(trx_sys->rw_trx_list)
        << " transaction(s) which must be rolled back or"
           " cleaned up in total "
        << rows_to_undo << unit << " row operations to undo";

    ib::info(ER_IB_MSG_1199) << "Trx id counter is " << trx_sys->max_trx_id;
  }

  trx_sys->found_prepared_trx = trx_sys->n_prepared_trx > 0;

  trx_sys_mutex_exit();

  return (purge_queue);
}

/** Creates the trx_sys instance and initializes purge_queue and mutex. */
void trx_sys_create(void) {
  ut_ad(trx_sys == NULL);

  trx_sys = static_cast<trx_sys_t *>(ut_zalloc_nokey(sizeof(*trx_sys)));

  mutex_create(LATCH_ID_TRX_SYS, &trx_sys->mutex);

  UT_LIST_INIT(trx_sys->serialisation_list, &trx_t::no_list);
  UT_LIST_INIT(trx_sys->rw_trx_list, &trx_t::trx_list);
  UT_LIST_INIT(trx_sys->mysql_trx_list, &trx_t::mysql_trx_list);

  trx_sys->mvcc = UT_NEW_NOKEY(MVCC(1024));

  trx_sys->min_active_id = 0;

  new (&trx_sys->rw_trx_ids)
      trx_ids_t(ut_allocator<trx_id_t>(mem_key_trx_sys_t_rw_trx_ids));

  new (&trx_sys->rw_trx_set) TrxIdSet();

  new (&trx_sys->rsegs) Rsegs();
  trx_sys->rsegs.set_empty();

  new (&trx_sys->tmp_rsegs) Rsegs();
  trx_sys->tmp_rsegs.set_empty();
}

/** Creates and initializes the transaction system at the database creation. */
void trx_sys_create_sys_pages(void) {
  mtr_t mtr;

  mtr_start(&mtr);

  trx_sysf_create(&mtr);

  mtr_commit(&mtr);
}

#else  /* !UNIV_HOTBACKUP */
/** Prints to stderr the MySQL binlog info in the system header if the
 magic number shows it valid. */
void trx_sys_print_mysql_binlog_offset_from_page(
    const byte *page) /*!< in: buffer containing the trx
                      system header page, i.e., page number
                      TRX_SYS_PAGE_NO in the tablespace */
{
  const trx_sysf_t *sys_header;

  sys_header = page + TRX_SYS;

  if (mach_read_from_4(sys_header + TRX_SYS_MYSQL_LOG_INFO +
                       TRX_SYS_MYSQL_LOG_MAGIC_N_FLD) ==
      TRX_SYS_MYSQL_LOG_MAGIC_N) {
    ib::info(ER_IB_MSG_1200)
        << "Last MySQL binlog file position "
        << mach_read_from_4(sys_header + TRX_SYS_MYSQL_LOG_INFO +
                            TRX_SYS_MYSQL_LOG_OFFSET_HIGH)
        << " "
        << mach_read_from_4(sys_header + TRX_SYS_MYSQL_LOG_INFO +
                            TRX_SYS_MYSQL_LOG_OFFSET_LOW)
        << ", file name "
        << sys_header + TRX_SYS_MYSQL_LOG_INFO + TRX_SYS_MYSQL_LOG_NAME;
  }
}
#endif /* !UNIV_HOTBACKUP */

#ifndef UNIV_HOTBACKUP
/*********************************************************************
Shutdown/Close the transaction system. */
void trx_sys_close(void) {
  ut_ad(srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS);

  if (trx_sys == NULL) {
    return;
  }

  ulint size = trx_sys->mvcc->size();

  if (size > 0) {
    ib::error(ER_IB_MSG_1201) << "All read views were not closed before"
                                 " shutdown: "
                              << size << " read views open";
  }

  sess_close(trx_dummy_sess);
  trx_dummy_sess = NULL;

  trx_purge_sys_close();

  /* Free the double write data structures. */
  buf_dblwr_free();

  /* Only prepared transactions may be left in the system. Free them. */
  ut_a(UT_LIST_GET_LEN(trx_sys->rw_trx_list) == trx_sys->n_prepared_trx);

  for (trx_t *trx = UT_LIST_GET_FIRST(trx_sys->rw_trx_list); trx != NULL;
       trx = UT_LIST_GET_FIRST(trx_sys->rw_trx_list)) {
    trx_free_prepared(trx);
  }

  /* There can't be any active transactions. */
  trx_sys->rsegs.~Rsegs();

  trx_sys->tmp_rsegs.~Rsegs();

  UT_DELETE(trx_sys->mvcc);

  ut_a(UT_LIST_GET_LEN(trx_sys->rw_trx_list) == 0);
  ut_a(UT_LIST_GET_LEN(trx_sys->mysql_trx_list) == 0);
  ut_a(UT_LIST_GET_LEN(trx_sys->serialisation_list) == 0);

  /* We used placement new to create this mutex. Call the destructor. */
  mutex_free(&trx_sys->mutex);

  trx_sys->rw_trx_ids.~trx_ids_t();

  trx_sys->rw_trx_set.~TrxIdSet();

  ut_free(trx_sys);

  trx_sys = NULL;
}

/** @brief Convert an undo log to TRX_UNDO_PREPARED state on shutdown.

If any prepared ACTIVE transactions exist, and their rollback was
prevented by innodb_force_recovery, we convert these transactions to
XA PREPARE state in the main-memory data structures, so that shutdown
will proceed normally. These transactions will again recover as ACTIVE
on the next restart, and they will be rolled back unless
innodb_force_recovery prevents it again.

@param[in]	trx	transaction
@param[in,out]	undo	undo log to convert to TRX_UNDO_PREPARED */
static void trx_undo_fake_prepared(const trx_t *trx, trx_undo_t *undo) {
  ut_ad(srv_force_recovery >= SRV_FORCE_NO_TRX_UNDO);
  ut_ad(trx_state_eq(trx, TRX_STATE_ACTIVE));
  ut_ad(trx->is_recovered);

  if (undo != NULL) {
    ut_ad(undo->state == TRX_UNDO_ACTIVE);
    undo->state = TRX_UNDO_PREPARED;
  }
}

/*********************************************************************
Check if there are any active (non-prepared) transactions.
@return total number of active transactions or 0 if none */
ulint trx_sys_any_active_transactions(void) {
  trx_sys_mutex_enter();

  ulint total_trx = UT_LIST_GET_LEN(trx_sys->mysql_trx_list);

  if (total_trx == 0) {
    total_trx = UT_LIST_GET_LEN(trx_sys->rw_trx_list);
    ut_a(total_trx >= trx_sys->n_prepared_trx);

    if (total_trx > trx_sys->n_prepared_trx &&
        srv_force_recovery >= SRV_FORCE_NO_TRX_UNDO) {
      for (trx_t *trx = UT_LIST_GET_FIRST(trx_sys->rw_trx_list); trx != NULL;
           trx = UT_LIST_GET_NEXT(trx_list, trx)) {
        if (!trx_state_eq(trx, TRX_STATE_ACTIVE) || !trx->is_recovered) {
          continue;
        }
        /* This was a recovered transaction
        whose rollback was disabled by
        the innodb_force_recovery setting.
        Pretend that it is in XA PREPARE
        state so that shutdown will work. */
        trx_undo_fake_prepared(trx, trx->rsegs.m_redo.insert_undo);
        trx_undo_fake_prepared(trx, trx->rsegs.m_redo.update_undo);
        trx_undo_fake_prepared(trx, trx->rsegs.m_noredo.insert_undo);
        trx_undo_fake_prepared(trx, trx->rsegs.m_noredo.update_undo);
        trx->state = TRX_STATE_PREPARED;
        trx_sys->n_prepared_trx++;
      }
    }

    ut_a(total_trx >= trx_sys->n_prepared_trx);
    total_trx -= trx_sys->n_prepared_trx;
  }

  trx_sys_mutex_exit();

  return (total_trx);
}

#ifdef UNIV_DEBUG
/** Validate the trx_ut_list_t.
 @return true if valid. */
static bool trx_sys_validate_trx_list_low(
    trx_ut_list_t *trx_list) /*!< in: &trx_sys->rw_trx_list */
{
  const trx_t *trx;
  const trx_t *prev_trx = NULL;

  ut_ad(trx_sys_mutex_own());

  ut_ad(trx_list == &trx_sys->rw_trx_list);

  for (trx = UT_LIST_GET_FIRST(*trx_list); trx != NULL;
       prev_trx = trx, trx = UT_LIST_GET_NEXT(trx_list, prev_trx)) {
    check_trx_state(trx);
    ut_a(prev_trx == NULL || prev_trx->id > trx->id);
  }

  return (true);
}

/** Validate the trx_sys_t::rw_trx_list.
 @return true if the list is valid. */
bool trx_sys_validate_trx_list() {
  ut_ad(trx_sys_mutex_own());

  ut_a(trx_sys_validate_trx_list_low(&trx_sys->rw_trx_list));

  return (true);
}
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */



/** Initialize trx_sys_undo_spaces, called once during srv_start(). */
void trx_sys_undo_spaces_init() {
  trx_sys_undo_spaces = UT_NEW(Space_Ids(), mem_key_undo_spaces);

  trx_sys_undo_spaces->reserve(TRX_SYS_N_RSEGS);
}

/** Free the resources occupied by trx_sys_undo_spaces,
called once during thread de-initialization. */
void trx_sys_undo_spaces_deinit() {
  if (trx_sys_undo_spaces != nullptr) {
    trx_sys_undo_spaces->clear();
    UT_DELETE(trx_sys_undo_spaces);
    trx_sys_undo_spaces = nullptr;
  }
}
