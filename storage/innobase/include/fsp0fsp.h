/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/fsp0fsp.h
 File space management

 Created 12/18/1995 Heikki Tuuri
 *******************************************************/

#ifndef fsp0fsp_h
#define fsp0fsp_h

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fsp_header_get_field.h>
#include <innodb/tablespace/fsp_header_get_flags.h>
#include <innodb/tablespace/fsp_header_get_space_id.h>
#include <innodb/tablespace/fsp_header_get_page_size.h>
#include <innodb/tablespace/xdes_t.h>
#include <innodb/tablespace/fsp_is_dd_tablespace.h>
#include <innodb/tablespace/fsp_is_undo_tablespace.h>
#include <innodb/tablespace/fsp_is_system_tablespace.h>
#include <innodb/tablespace/fsp_is_session_temporary.h>
#include <innodb/tablespace/fsp_is_global_temporary.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>
#include <innodb/tablespace/fsp_is_system_or_temp_tablespace.h>
#include <innodb/tablespace/fsp_is_ibd_tablespace.h>
#include <innodb/tablespace/fsp_is_file_per_table.h>
#include <innodb/tablespace/fsp_is_checksum_disabled.h>
#include <innodb/math/ut_2pow_remainder.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/bit/ut_bit_get_nth.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fsp_flags_is_compressed.h>
#include <innodb/tablespace/fsp_flags_are_equal.h>
#include <innodb/tablespace/page_size_to_ssize.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>
#include <innodb/tablespace/fsp_flags_set_zip_size.h>
#include <innodb/tablespace/fsp_flags_set_page_size.h>
#include <innodb/tablespace/fsp_flags_init.h>
#include <innodb/tablespace/xdes_calc_descriptor_index.h>
#include <innodb/tablespace/xdes_get_bit.h>
#include <innodb/tablespace/xdes_calc_descriptor_page.h>
#include <innodb/tablespace/xdes_arr_size.h>
#include <innodb/tablespace/fsp_is_inode_page.h>
#include <innodb/tablespace/fsp_header_get_sdi_offset.h>
#include <innodb/tablespace/fsp_header_get_encryption_progress_offset.h>
#include <innodb/tablespace/fsp_header_get_server_version.h>
#include <innodb/tablespace/fsp_header_get_space_version.h>
#include <innodb/tablespace/xdes_state_t.h>
#include <innodb/disk/page_no_t.h>
#include <innodb/tablespace/fsp_get_extent_size_in_pages.h>
#include <innodb/tablespace/fsp_init.h>
#include <innodb/tablespace/fsp_get_pages_to_extend_ibd.h>
#include <innodb/tablespace/fseg_header_t.h>
#include <innodb/tablespace/fsp_reserve_t.h>
#include <innodb/tablespace/fsp_header_t.h>
#include <innodb/tablespace/fseg_inode_t.h>
#include <innodb/tablespace/fsp_header_init_fields.h>


#include "fut0lst.h"

#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"


#ifdef UNIV_HOTBACKUP
#include "buf0buf.h"
#endif /* UNIV_HOTBACKUP */

class DDL_Record;
extern std::vector<DDL_Record *> ts_encrypt_ddl_records;
extern mysql_cond_t resume_encryption_cond;
extern mysql_mutex_t resume_encryption_cond_m;



#ifdef UNIV_DEBUG
/** Check if the state of extent descriptor is valid.
@param[in]	state	the extent descriptor state
@return	true if state is valid, false otherwise */
bool xdes_state_is_valid(ulint state);
#endif /* UNIV_DEBUG */

#ifdef UNIV_DEBUG
struct xdes_mem_t {
  xdes_mem_t(const xdes_t *xdes) : m_xdes(xdes) {}

  const char *state_name() const;

  bool is_valid() const;
  const xdes_t *m_xdes;

  std::ostream &print(std::ostream &out) const;
};

inline std::ostream &operator<<(std::ostream &out, const xdes_mem_t &obj) {
  return (obj.print(out));
}

/** In-memory representation of the fsp_header_t file structure. */
struct fsp_header_mem_t {
  fsp_header_mem_t(const fsp_header_t *header, mtr_t *mtr);

  ulint m_space_id;
  ulint m_notused;
  ulint m_fsp_size;
  ulint m_free_limit;
  ulint m_flags;
  ulint m_fsp_frag_n_used;
  flst_bnode_t m_fsp_free;
  flst_bnode_t m_free_frag;
  flst_bnode_t m_full_frag;
  ib_id_t m_segid;
  flst_bnode_t m_inodes_full;
  flst_bnode_t m_inodes_free;

  std::ostream &print(std::ostream &out) const;
};

inline std::ostream &operator<<(std::ostream &out,
                                const fsp_header_mem_t &obj) {
  return (obj.print(out));
}
#endif /* UNIV_DEBUG */








/** Gets the size of the system tablespace from the tablespace header.  If
 we do not have an auto-extending data file, this should be equal to
 the size of the data files.  If there is an auto-extending data file,
 this can be smaller.
 @return size in pages */
page_no_t fsp_header_get_tablespace_size(void);






/** Read the server version number from the DD tablespace header.
@param[out]	version	server version from tablespace header
@return false if success. */
bool fsp_header_dict_get_server_version(uint *version);






/** Reads the encryption key from the first page of a tablespace.
@param[in]	fsp_flags	tablespace flags
@param[in,out]	key		tablespace key
@param[in,out]	iv		tablespace iv
@param[in]	page	first page of a tablespace
@return true if success */
bool fsp_header_get_encryption_key(uint32_t fsp_flags, byte *key, byte *iv,
                                   page_t *page);

/** Get encryption operation type in progress from the first
page of a tablespace.
@param[in]	page		first page of a tablespace
@param[in]	page_size	tablespace page size
@return operation type
*/
encryption_op_type fsp_header_encryption_op_type_in_progress(
    const page_t *page, page_size_t page_size);

/** Check if the tablespace size information is valid.
@param[in]	space_id	the tablespace identifier
@return true if valid, false if invalid. */
bool fsp_check_tablespace_size(space_id_t space_id);



/** Get the offset of encrytion information in page 0.
@param[in]	page_size	page size.
@return	offset on success, otherwise 0. */
ulint fsp_header_get_encryption_offset(const page_size_t &page_size);

/** Write the encryption info into the space header.
@param[in]      space_id		tablespace id
@param[in]      space_flags		tablespace flags
@param[in]      encrypt_info		buffer for re-encrypt key
@param[in]      update_fsp_flags	if it need to update the space flags
@param[in,out]	mtr			mini-transaction
@return true if success. */
bool fsp_header_write_encryption(space_id_t space_id, ulint space_flags,
                                 byte *encrypt_info, bool update_fsp_flags,
                                 bool rotate_encryption, mtr_t *mtr);

/** Write the encryption progress info into the space header.
@param[in]      space_id		tablespace id
@param[in]      space_flags		tablespace flags
@param[in]      progress_info		max pages (un)encrypted
@param[in]      operation_type		typpe of operation
@param[in]      update_operation_type	is operation to be updated
@param[in,out]	mtr			mini-transaction
@return true if success. */
bool fsp_header_write_encryption_progress(
    space_id_t space_id, ulint space_flags, ulint progress_info,
    byte operation_type, bool update_operation_type, mtr_t *mtr);

/** Rotate the encryption info in the space header.
@param[in]	space		tablespace
@param[in]      encrypt_info	buffer for re-encrypt key.
@param[in,out]	mtr		mini-transaction
@return true if success. */
bool fsp_header_rotate_encryption(fil_space_t *space, byte *encrypt_info,
                                  mtr_t *mtr);

/** Initializes the space header of a new created space and creates also the
insert buffer tree root if space == 0.
@param[in]	space_id	space id
@param[in]	size		current size in blocks
@param[in,out]	mtr		min-transaction
@param[in]	is_boot		if it's for bootstrap
@return	true on success, otherwise false. */
bool fsp_header_init(space_id_t space_id, page_no_t size, mtr_t *mtr,
                     bool is_boot);

/** Increases the space size field of a space. */
void fsp_header_inc_size(space_id_t space_id, /*!< in: space id */
                         page_no_t size_inc, /*!< in: size increment in pages */
                         mtr_t *mtr);        /*!< in/out: mini-transaction */
/** Creates a new segment.
 @return the block where the segment header is placed, x-latched, NULL
 if could not create segment because of lack of space */
buf_block_t *fseg_create(
    space_id_t space,  /*!< in: space id */
    page_no_t page,    /*!< in: page where the segment header is
                       placed: if this is != 0, the page must belong
                       to another segment, if this is 0, a new page
                       will be allocated and it will belong to the
                       created segment */
    ulint byte_offset, /*!< in: byte offset of the created
                       segment header on the page */
    mtr_t *mtr);       /*!< in/out: mini-transaction */
/** Creates a new segment.
 @return the block where the segment header is placed, x-latched, NULL
 if could not create segment because of lack of space */
buf_block_t *fseg_create_general(
    space_id_t space_id,        /*!< in: space id */
    page_no_t page,             /*!< in: page where the segment header is
                        placed: if this is != 0, the page must belong to another
                        segment, if this is 0, a new page will be allocated and
                        it will belong to the created segment */
    ulint byte_offset,          /*!< in: byte offset of the created segment
                   header on the page */
    ibool has_done_reservation, /*!< in: TRUE if the caller has
          already done the reservation for the pages with
          fsp_reserve_free_extents (at least 2 extents: one for
          the inode and the other for the segment) then there is
          no need to do the check for this individual operation */
    mtr_t *mtr);                /*!< in/out: mini-transaction */
/** Calculates the number of pages reserved by a segment, and how many pages are
 currently used.
 @return number of reserved pages */
ulint fseg_n_reserved_pages(
    fseg_header_t *header, /*!< in: segment header */
    ulint *used,           /*!< out: number of pages used (<= reserved) */
    mtr_t *mtr);           /*!< in/out: mini-transaction */
/** Allocates a single free page from a segment. This function implements
 the intelligent allocation strategy which tries to minimize
 file space fragmentation.
 @param[in,out] seg_header segment header
 @param[in] hint hint of which page would be desirable
 @param[in] direction if the new page is needed because
                                 of an index page split, and records are
                                 inserted there in order, into which
                                 direction they go alphabetically: FSP_DOWN,
                                 FSP_UP, FSP_NO_DIR
 @param[in,out] mtr mini-transaction
 @return X-latched block, or NULL if no page could be allocated */
#define fseg_alloc_free_page(seg_header, hint, direction, mtr) \
  fseg_alloc_free_page_general(seg_header, hint, direction, FALSE, mtr, mtr)
/** Allocates a single free page from a segment. This function implements
 the intelligent allocation strategy which tries to minimize file space
 fragmentation.
 @retval NULL if no page could be allocated
 @retval block, rw_lock_x_lock_count(&block->lock) == 1 if allocation succeeded
 (init_mtr == mtr, or the page was not previously freed in mtr)
 @retval block (not allocated or initialized) otherwise */
buf_block_t *fseg_alloc_free_page_general(
    fseg_header_t *seg_header,  /*!< in/out: segment header */
    page_no_t hint,             /*!< in: hint of which page would be
                                desirable */
    byte direction,             /*!< in: if the new page is needed because
                              of an index page split, and records are
                              inserted there in order, into which
                              direction they go alphabetically: FSP_DOWN,
                              FSP_UP, FSP_NO_DIR */
    ibool has_done_reservation, /*!< in: TRUE if the caller has
                  already done the reservation for the page
                  with fsp_reserve_free_extents, then there
                  is no need to do the check for this individual
                  page */
    mtr_t *mtr,                 /*!< in/out: mini-transaction */
    mtr_t *init_mtr)            /*!< in/out: mtr or another mini-transaction
                               in which the page should be initialized.
                               If init_mtr!=mtr, but the page is already
                               latched in mtr, do not initialize the page. */
    MY_ATTRIBUTE((warn_unused_result));

/** Reserves free pages from a tablespace. All mini-transactions which may
use several pages from the tablespace should call this function beforehand
and reserve enough free extents so that they certainly will be able
to do their operation, like a B-tree page split, fully. Reservations
must be released with function fil_space_release_free_extents!

The alloc_type below has the following meaning: FSP_NORMAL means an
operation which will probably result in more space usage, like an
insert in a B-tree; FSP_UNDO means allocation to undo logs: if we are
deleting rows, then this allocation will in the long run result in
less space usage (after a purge); FSP_CLEANING means allocation done
in a physical record delete (like in a purge) or other cleaning operation
which will result in less space usage in the long run. We prefer the latter
two types of allocation: when space is scarce, FSP_NORMAL allocations
will not succeed, but the latter two allocations will succeed, if possible.
The purpose is to avoid dead end where the database is full but the
user cannot free any space because these freeing operations temporarily
reserve some space.

Single-table tablespaces whose size is < FSP_EXTENT_SIZE pages are a special
case. In this function we would liberally reserve several extents for
every page split or merge in a B-tree. But we do not want to waste disk space
if the table only occupies < FSP_EXTENT_SIZE pages. That is why we apply
different rules in that special case, just ensuring that there are n_pages
free pages available.

@param[out]	n_reserved	number of extents actually reserved; if we
                                return true and the tablespace size is <
                                FSP_EXTENT_SIZE pages, then this can be 0,
                                otherwise it is n_ext
@param[in]	space_id	tablespace identifier
@param[in]	n_ext		number of extents to reserve
@param[in]	alloc_type	page reservation type (FSP_BLOB, etc)
@param[in,out]	mtr		the mini transaction
@param[in]	n_pages		for small tablespaces (tablespace size is
                                less than FSP_EXTENT_SIZE), number of free
                                pages to reserve.
@return true if we were able to make the reservation */
bool fsp_reserve_free_extents(ulint *n_reserved, space_id_t space_id,
                              ulint n_ext, fsp_reserve_t alloc_type, mtr_t *mtr,
                              page_no_t n_pages = 2);

/** Calculate how many KiB of new data we will be able to insert to the
tablespace without running out of space.
@param[in]	space_id	tablespace ID
@return available space in KiB
@retval UINTMAX_MAX if unknown */
uintmax_t fsp_get_available_space_in_free_extents(space_id_t space_id);

/** Calculate how many KiB of new data we will be able to insert to the
tablespace without running out of space. Start with a space object that has
been acquired by the caller who holds it for the calculation,
@param[in]	space		tablespace object from fil_space_acquire()
@return available space in KiB */
uintmax_t fsp_get_available_space_in_free_extents(const fil_space_t *space);

/** Frees a single page of a segment. */
void fseg_free_page(fseg_header_t *seg_header, /*!< in: segment header */
                    space_id_t space_id,       /*!< in: space id */
                    page_no_t page,            /*!< in: page offset */
                    bool ahi,    /*!< in: whether we may need to drop
                                 the adaptive hash index */
                    mtr_t *mtr); /*!< in/out: mini-transaction */
/** Checks if a single page of a segment is free.
 @return true if free */
bool fseg_page_is_free(fseg_header_t *seg_header, /*!< in: segment header */
                       space_id_t space_id,       /*!< in: space id */
                       page_no_t page)            /*!< in: page offset */
    MY_ATTRIBUTE((warn_unused_result));
/** Frees part of a segment. This function can be used to free a segment
 by repeatedly calling this function in different mini-transactions.
 Doing the freeing in a single mini-transaction might result in
 too big a mini-transaction.
 @return true if freeing completed */
ibool fseg_free_step(
    fseg_header_t *header, /*!< in, own: segment header; NOTE: if the header
                           resides on the first page of the frag list
                           of the segment, this pointer becomes obsolete
                           after the last freeing step */
    bool ahi,              /*!< in: whether we may need to drop
                           the adaptive hash index */
    mtr_t *mtr)            /*!< in/out: mini-transaction */
    MY_ATTRIBUTE((warn_unused_result));
/** Frees part of a segment. Differs from fseg_free_step because this function
 leaves the header page unfreed.
 @return true if freeing completed, except the header page */
ibool fseg_free_step_not_header(
    fseg_header_t *header, /*!< in: segment header which must reside on
                           the first fragment page of the segment */
    bool ahi,              /*!< in: whether we may need to drop
                           the adaptive hash index */
    mtr_t *mtr)            /*!< in/out: mini-transaction */
    MY_ATTRIBUTE((warn_unused_result));

/** Parses a redo log record of a file page init.
 @return end of log record or NULL */
byte *fsp_parse_init_file_page(byte *ptr,           /*!< in: buffer */
                               byte *end_ptr,       /*!< in: buffer end */
                               buf_block_t *block); /*!< in: block or NULL */
#ifdef UNIV_BTR_PRINT
/** Writes info of a segment. */
void fseg_print(fseg_header_t *header, /*!< in: segment header */
                mtr_t *mtr);           /*!< in/out: mini-transaction */
#endif                                 /* UNIV_BTR_PRINT */










/** Convert a 32 bit integer tablespace flags to the 32 bit table flags.
This can only be done for a tablespace that was built as a file-per-table
tablespace. Note that the fsp_flags cannot show the difference between a
Compact and Redundant table, so an extra Compact boolean must be supplied.
                        Low order bit
                    | REDUNDANT | COMPACT | COMPRESSED | DYNAMIC
fil_space_t::flags  |     0     |    0    |     1      |    1
dict_table_t::flags |     0     |    1    |     1      |    1
@param[in]	fsp_flags	fil_space_t::flags
@param[in]	compact		true if not Redundant row format
@return tablespace flags (fil_space_t::flags) */
uint32_t fsp_flags_to_dict_tf(uint32_t fsp_flags, bool compact);

/** Gets a pointer to the space header and x-locks its page.
@param[in]	id		space id
@param[in]	page_size	page size
@param[in,out]	mtr		mini-transaction
@return pointer to the space header, page x-locked */
fsp_header_t *fsp_get_space_header(space_id_t id, const page_size_t &page_size,
                                   mtr_t *mtr);

/** Retrieve tablespace dictionary index root page number stored in the
page 0
@param[in]	space		tablespace id
@param[in]	page_size	page size
@param[in,out]	mtr		mini-transaction
@return root page num of the tablespace dictionary index copy */
page_no_t fsp_sdi_get_root_page_num(space_id_t space,
                                    const page_size_t &page_size, mtr_t *mtr);

/** Write SDI Index root page num to page 0 of tablespace.
@param[in,out]	page		page 0 frame
@param[in]	page_size	size of page
@param[in]	root_page_num	root page number of SDI
@param[in,out]	mtr		mini-transaction */
void fsp_sdi_write_root_to_page(page_t *page, const page_size_t &page_size,
                                page_no_t root_page_num, mtr_t *mtr);





/** Get the state of an xdes.
@param[in]	descr	extent descriptor
@param[in,out]	mtr	mini transaction.
@return	state */
inline xdes_state_t xdes_get_state(const xdes_t *descr, mtr_t *mtr) {
  ut_ad(descr && mtr);
  ut_ad(mtr_memo_contains_page(mtr, descr, MTR_MEMO_PAGE_SX_FIX));

  const ulint state = mach_read_from_4(descr + XDES_STATE);

  ut_ad(xdes_state_is_valid(state));
  return (static_cast<xdes_state_t>(state));
}

#ifdef UNIV_DEBUG
/** Print the extent descriptor page in user-friendly format.
@param[in]	out	the output file stream
@param[in]	xdes	the extent descriptor page
@param[in]	page_no	the page number of xdes page
@param[in]	mtr	the mini transaction.
@return None. */
std::ostream &xdes_page_print(std::ostream &out, const page_t *xdes,
                              page_no_t page_no, mtr_t *mtr);

inline bool xdes_mem_t::is_valid() const {
  const ulint state = mach_read_from_4(m_xdes + XDES_STATE);
  return (xdes_state_is_valid(state));
}

inline const char *xdes_mem_t::state_name() const {
  const ulint val = mach_read_from_4(m_xdes + XDES_STATE);

  ut_ad(xdes_state_is_valid(val));

  xdes_state_t state = static_cast<xdes_state_t>(val);

  switch (state) {
    case XDES_NOT_INITED:
      return ("XDES_NOT_INITED");
    case XDES_FREE:
      return ("XDES_FREE");
    case XDES_FREE_FRAG:
      return ("XDES_FREE_FRAG");
    case XDES_FULL_FRAG:
      return ("XDES_FULL_FRAG");
    case XDES_FSEG:
      return ("XDES_FSEG");
    case XDES_FSEG_FRAG:
      return ("XDES_FSEG_FRAG");
  }
  return ("UNKNOWN");
}

#endif /* UNIV_DEBUG */

/** Update the tablespace size information and generate redo log for it.
@param[in]	header	tablespace header.
@param[in]	size	the new tablespace size in pages.
@param[in]	mtr	the mini-transaction context. */
inline void fsp_header_size_update(fsp_header_t *header, ulint size,
                                   mtr_t *mtr) {
  DBUG_ENTER("fsp_header_size_update");

  DBUG_LOG("ib_log", "old_size=" << mach_read_from_4(header + FSP_SIZE)
                                 << ", new_size=" << size);

  mlog_write_ulint(header + FSP_SIZE, size, MLOG_4BYTES, mtr);

  DBUG_VOID_RETURN;
}

/** Determine if the tablespace has SDI.
@param[in]	space_id	Tablespace id
@return DB_SUCCESS if SDI is present else DB_ERROR
or DB_TABLESPACE_NOT_FOUND */
dberr_t fsp_has_sdi(space_id_t space_id);

/** Encrypt/Unencrypt a tablespace.
@param[in]      thd             current thread
@param[in]	space_id	Tablespace id
@param[in]	from_page	page id from where operation to be done
@param[in]	to_encrypt	true if to encrypt, false if to unencrypt
@param[in]	in_recovery	true if its called after recovery
@param[in, out]	dd_space_in	dd tablespace object
@return	0 for success, otherwise error code */
dberr_t fsp_alter_encrypt_tablespace(THD *thd, space_id_t space_id,
                                     page_no_t from_page, bool to_encrypt,
                                     bool in_recovery, void *dd_space_in);

/** Initiate roll-forward of alter encrypt in background thread */
void fsp_init_resume_alter_encrypt_tablespace();
#endif
