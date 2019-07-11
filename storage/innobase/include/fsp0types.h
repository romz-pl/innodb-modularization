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

/******************************************************
@file include/fsp0types.h
File space management types

Created May 26, 2009 Vasil Dimov
*******************************************************/

#ifndef fsp0types_h
#define fsp0types_h

#include <innodb/univ/univ.h>
#include <innodb/tablespace/extent.h>


/** @name Flags for inserting records in order
If records are inserted in order, there are the following
flags to tell this (their type is made byte for the compiler
to warn if direction and hint parameters are switched in
fseg_alloc_free_page) */
/* @{ */
#define FSP_UP ((byte)111)     /*!< alphabetically upwards */
#define FSP_DOWN ((byte)112)   /*!< alphabetically downwards */
#define FSP_NO_DIR ((byte)113) /*!< no order */
/* @} */




/** @name File segment header
The file segment header points to the inode describing the file segment. */
/* @{ */
/** Data type for file segment header */
typedef byte fseg_header_t;

#define FSEG_HDR_SPACE 0   /*!< space id of the inode */
#define FSEG_HDR_PAGE_NO 4 /*!< page number of the inode */
#define FSEG_HDR_OFFSET 8  /*!< byte offset of the inode */

#define FSEG_HEADER_SIZE            \
  10 /*!< Length of the file system \
     header, in bytes */
/* @} */

#ifdef UNIV_DEBUG

struct mtr_t;

/** A wrapper class to print the file segment header information. */
class fseg_header {
 public:
  /** Constructor of fseg_header.
  @param[in]	header	the underlying file segment header object
  @param[in]	mtr	the mini-transaction.  No redo logs are
                          generated, only latches are checked within
                          mini-transaction */
  fseg_header(const fseg_header_t *header, mtr_t *mtr)
      : m_header(header), m_mtr(mtr) {}

  /** Print the file segment header to the given output stream.
  @param[in,out]	out	the output stream into which the object
                          is printed.
  @retval	the output stream into which the object was printed. */
  std::ostream &to_stream(std::ostream &out) const;

 private:
  /** The underlying file segment header */
  const fseg_header_t *m_header;

  /** The mini transaction, which is used mainly to check whether
  appropriate latches have been taken by the calling thread. */
  mtr_t *m_mtr;
};

/* Overloading the global output operator to print a file segment header
@param[in,out]	out	the output stream into which object will be printed
@param[in]	header	the file segment header to be printed
@retval the output stream */
inline std::ostream &operator<<(std::ostream &out, const fseg_header &header) {
  return (header.to_stream(out));
}
#endif /* UNIV_DEBUG */

/** Flags for fsp_reserve_free_extents */
enum fsp_reserve_t {
  FSP_NORMAL,   /* reservation during normal B-tree operations */
  FSP_UNDO,     /* reservation done for undo logging */
  FSP_CLEANING, /* reservation done during purge operations */
  FSP_BLOB      /* reservation being done for BLOB insertion */
};

/* Number of pages described in a single descriptor page: currently each page
description takes less than 1 byte; a descriptor page is repeated every
this many file pages */
/* #define XDES_DESCRIBED_PER_PAGE		UNIV_PAGE_SIZE */
/* This has been replaced with either UNIV_PAGE_SIZE or page_zip->size. */

/** @name The space low address page map
The pages at FSP_XDES_OFFSET and FSP_IBUF_BITMAP_OFFSET are repeated
every XDES_DESCRIBED_PER_PAGE pages in every tablespace. */
/* @{ */
/*--------------------------------------*/
#define FSP_XDES_OFFSET 0        /* !< extent descriptor */
#define FSP_IBUF_BITMAP_OFFSET 1 /* !< insert buffer bitmap */
                                 /* The ibuf bitmap pages are the ones whose
                                 page number is the number above plus a
                                 multiple of XDES_DESCRIBED_PER_PAGE */

#define FSP_FIRST_INODE_PAGE_NO 2 /*!< in every tablespace */

/* The following pages exist in the system tablespace (space 0). */

#define FSP_IBUF_HEADER_PAGE_NO \
  3 /*!< insert buffer          \
    header page, in             \
    tablespace 0 */
#define FSP_IBUF_TREE_ROOT_PAGE_NO \
  4 /*!< insert buffer             \
    B-tree root page in            \
    tablespace 0 */
    /* The ibuf tree root page number in
    tablespace 0; its fseg inode is on the page
    number FSP_FIRST_INODE_PAGE_NO */
#define FSP_TRX_SYS_PAGE_NO \
  5 /*!< transaction        \
    system header, in       \
    tablespace 0 */
#define FSP_FIRST_RSEG_PAGE_NO  \
  6 /*!< first rollback segment \
    page, in tablespace 0 */
#define FSP_DICT_HDR_PAGE_NO    \
  7 /*!< data dictionary header \
    page, in tablespace 0 */

/* The following page exists in each v8 Undo Tablespace.
(space_id = SRV_LOG_SPACE_FIRST_ID - undo_space_num)
(undo_space_num = rseg_array_slot_num + 1) */

#define FSP_RSEG_ARRAY_PAGE_NO      \
  3 /*!< rollback segment directory \
    page number in each undo tablespace */
/*--------------------------------------*/
/* @} */

/** Validate the tablespace flags.
These flags are stored in the tablespace header at offset FSP_SPACE_FLAGS.
They should be 0 for ROW_FORMAT=COMPACT and ROW_FORMAT=REDUNDANT.
The newer row formats, COMPRESSED and DYNAMIC, will have at least
the DICT_TF_COMPACT bit set.
@param[in]	flags	Tablespace flags
@return true if valid, false if not */
bool fsp_flags_is_valid(uint32_t flags)
    MY_ATTRIBUTE((warn_unused_result, const));

/** Check if a space_id is the system temporary space ID.
@param[in]	space_id	tablespace ID
@return true if tablespace is system temporary. */
bool fsp_is_system_temporary(space_id_t space_id);

/** Check if a space_id is the system temporary space ID.
@param[in]	space_id	tablespace ID
@return true if tablespace is system temporary. */
bool fsp_is_session_temporary(space_id_t space_id);

/** Check if a space_id is the system temporary space ID.
@param[in]	space_id	tablespace ID
@return true if tablespace is system temporary. */
bool fsp_is_global_temporary(space_id_t space_id);

/** Check if checksum is disabled for the given space.
@param[in]	space_id	verify is checksum is enabled for given space.
@return true if checksum is disabled for given space. */
bool fsp_is_checksum_disabled(space_id_t space_id);

#ifdef UNIV_DEBUG
/** Skip some of the sanity checks that are time consuming even in debug mode
and can affect frequent verification runs that are done to ensure stability of
the product.
@return true if check should be skipped for given space. */
bool fsp_skip_sanity_check(space_id_t space_id);
#endif /* UNIV_DEBUG */


/** Max number of rollback segments: the number of segment specification slots
in the transaction system array; rollback segment id must fit in one (signed)
byte, therefore 128; each slot is currently 8 bytes in size. If you want
to raise the level to 256 then you will need to fix some assertions that
impose the 7 bit restriction. e.g., mach_write_to_3() */
#define TRX_SYS_N_RSEGS 128

/** Minimum and Maximum number of implicit undo tablespaces.  This kind
of undo tablespace is always created and found in --innodb-undo-directory. */
#define FSP_MIN_UNDO_TABLESPACES 2
#define FSP_MAX_UNDO_TABLESPACES (TRX_SYS_N_RSEGS - 1)
#define FSP_IMPLICIT_UNDO_TABLESPACES 2
#define FSP_MAX_ROLLBACK_SEGMENTS (TRX_SYS_N_RSEGS)

#endif /* fsp0types_h */
