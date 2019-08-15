#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/flags.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/univ/page_no_t.h>
#include "mysql_version.h"
#include <innodb/dict_mem/flags.h>

#include <limits>

/** Maximum number of threads to use for scanning data files. */
const size_t MAX_SCAN_THREADS = 8;

#ifndef UNIV_HOTBACKUP
/** Maximum number of shards supported. */
const size_t MAX_SHARDS = 64;

/** The redo log is in its own shard. */
const size_t REDO_SHARD = MAX_SHARDS - 1;

/** Number of undo shards to reserve. */
const size_t UNDO_SHARDS = 4;

/** The UNDO logs have their own shards (4). */
const size_t UNDO_SHARDS_START = REDO_SHARD - (UNDO_SHARDS + 1);
#else  /* !UNIV_HOTBACKUP */

/** Maximum number of shards supported. */
const size_t MAX_SHARDS = 1;

/** The redo log is in its own shard. */
const size_t REDO_SHARD = 0;

/** The UNDO logs have their own shards (4). */
const size_t UNDO_SHARDS_START = 0;
#endif /* !UNIV_HOTBACKUP */

/** Maximum pages to check for valid space ID during start up. */
const size_t MAX_PAGES_TO_CHECK = 3;

/** Sentinel for empty open slot. */
const size_t EMPTY_OPEN_SLOT = std::numeric_limits<size_t>::max();

/** Value of fil_space_t::magic_n */
constexpr size_t FIL_SPACE_MAGIC_N = 89472;

/** Value of fil_node_t::magic_n */
constexpr size_t FIL_NODE_MAGIC_N = 89389;



/** Initial size of a single-table tablespace in pages */
constexpr size_t FIL_IBD_FILE_INITIAL_SIZE = 7;
constexpr size_t FIL_IBT_FILE_INITIAL_SIZE = 5;

/** An empty tablespace (CREATE TABLESPACE) has minimum
of 4 pages and an empty CREATE TABLE (file_per_table) has 6 pages.
Minimum of these two is 4 */
constexpr size_t FIL_IBD_FILE_INITIAL_SIZE_5_7 = 4;

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

#define FSP_FIRST_RSEG_PAGE_NO  \
  6 /*!< first rollback segment \
    page, in tablespace 0 */


/* The following page exists in each v8 Undo Tablespace.
(space_id = SRV_LOG_SPACE_FIRST_ID - undo_space_num)
(undo_space_num = rseg_array_slot_num + 1) */

#define FSP_RSEG_ARRAY_PAGE_NO      \
  3 /*!< rollback segment directory \
    page number in each undo tablespace */
/*--------------------------------------*/
/* @} */


#include <innodb/disk/flags.h>

/* The physical size of a list base node in bytes */
constexpr ulint FLST_BASE_NODE_SIZE = 4 + 2 * FIL_ADDR_SIZE;

/* The physical size of a list node in bytes */
constexpr ulint FLST_NODE_SIZE = 2 * FIL_ADDR_SIZE;


/*			SPACE HEADER
                        ============

File space header data structure: this data structure is contained in the
first page of a space. The space for this header is reserved in every extent
descriptor page, but used only in the first. */

/*-------------------------------------*/
#define FSP_SPACE_ID 0 /* space id */
#define FSP_NOT_USED                      \
  4 /* this field contained a value up to \
    which we know that the modifications  \
    in the database have been flushed to  \
    the file space; not used now */
#define FSP_SIZE                    \
  8 /* Current size of the space in \
    pages */
#define FSP_FREE_LIMIT                       \
  12 /* Minimum page number for which the    \
     free list has not been initialized:     \
     the pages >= this limit are, by         \
     definition, free; note that in a        \
     single-table tablespace where size      \
     < 64 pages, this number is 64, i.e.,    \
     we have initialized the space           \
     about the first extent, but have not    \
     physically allocated those pages to the \
     file */
#define FSP_SPACE_FLAGS               \
  16 /* fsp_space_t.flags, similar to \
     dict_table_t::flags */
#define FSP_FRAG_N_USED                            \
  20                /* number of used pages in the \
                    FSP_FREE_FRAG list */
#define FSP_FREE 24 /* list of free extents */
#define FSP_FREE_FRAG (24 + FLST_BASE_NODE_SIZE)
/* list of partially free extents not
belonging to any segment */
#define FSP_FULL_FRAG (24 + 2 * FLST_BASE_NODE_SIZE)
/* list of full extents not belonging
to any segment */
#define FSP_SEG_ID (24 + 3 * FLST_BASE_NODE_SIZE)
/* 8 bytes which give the first unused
segment id */
#define FSP_SEG_INODES_FULL (32 + 3 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
headers, where all the segment inode
slots are reserved */
#define FSP_SEG_INODES_FREE (32 + 4 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
headers, where not all the segment
header slots are reserved */
/*-------------------------------------*/
/* File space header size */
#define FSP_HEADER_SIZE (32 + 5 * FLST_BASE_NODE_SIZE)

#define FSP_FREE_ADD                    \
  4 /* this many free extents are added \
    to the free list from above         \
    FSP_FREE_LIMIT at a time */
/* @} */

/** Offset of the space header within a file page */
#define FSP_HEADER_OFFSET FIL_PAGE_DATA






/*			EXTENT DESCRIPTOR
                        =================

File extent descriptor data structure: contains bits to tell which pages in
the extent are free and which contain old tuple version to clean. */

/*-------------------------------------*/
#define XDES_ID                      \
  0 /* The identifier of the segment \
    to which this extent belongs */
#define XDES_FLST_NODE              \
  8 /* The list node data structure \
    for the descriptors */
#define XDES_STATE (FLST_NODE_SIZE + 8)
/* contains state information
of the extent */
#define XDES_BITMAP (FLST_NODE_SIZE + 12)
/* Descriptor bitmap of the pages
in the extent */
/*-------------------------------------*/

#define XDES_BITS_PER_PAGE 2 /* How many bits are there per page */
#define XDES_FREE_BIT                  \
  0 /* Index of the bit which tells if \
    the page is free */
#define XDES_CLEAN_BIT               \
  1 /* NOTE: currently not used!     \
    Index of the bit which tells if  \
    there are old versions of tuples \
    on the page */




/** The number of bytes required to store SDI root page number(4)
and SDI version(4) at Page 0 */
#define FSP_SDI_HEADER_LEN 8

/** File space extent size in pages
page size | file space extent size
----------+-----------------------
   4 KiB  | 256 pages = 1 MiB
   8 KiB  | 128 pages = 1 MiB
  16 KiB  |  64 pages = 1 MiB
  32 KiB  |  64 pages = 2 MiB
  64 KiB  |  64 pages = 4 MiB
*/
#define FSP_EXTENT_SIZE                                                 \
  static_cast<page_no_t>(                                               \
      ((UNIV_PAGE_SIZE <= (16384)                                       \
            ? (1048576 / UNIV_PAGE_SIZE)                                \
            : ((UNIV_PAGE_SIZE <= (32768)) ? (2097152 / UNIV_PAGE_SIZE) \
                                           : (4194304 / UNIV_PAGE_SIZE)))))

/** File space extent size (four megabyte) in pages for MAX page size */
#define FSP_EXTENT_SIZE_MAX (4194304 / UNIV_PAGE_SIZE_MAX)

/** File space extent size (one megabyte) in pages for MIN page size */
#define FSP_EXTENT_SIZE_MIN (1048576 / UNIV_PAGE_SIZE_MIN)




/** File extent data structure size in bytes. */
#define XDES_SIZE \
  (XDES_BITMAP + UT_BITS_IN_BYTES(FSP_EXTENT_SIZE * XDES_BITS_PER_PAGE))

/** File extent data structure size in bytes for MAX page size. */
#define XDES_SIZE_MAX \
  (XDES_BITMAP + UT_BITS_IN_BYTES(FSP_EXTENT_SIZE_MAX * XDES_BITS_PER_PAGE))

/** File extent data structure size in bytes for MIN page size. */
#define XDES_SIZE_MIN \
  (XDES_BITMAP + UT_BITS_IN_BYTES(FSP_EXTENT_SIZE_MIN * XDES_BITS_PER_PAGE))

/** Offset of the descriptor array on a descriptor page */
#define XDES_ARR_OFFSET (FSP_HEADER_OFFSET + FSP_HEADER_SIZE)

/** The number of reserved pages in a fragment extent. */
const ulint XDES_FRAG_N_USED = 2;

/* @} */



/*			FILE SEGMENT INODE
                        ==================

Segment inode which is created for each segment in a tablespace. NOTE: in
purge we assume that a segment having only one currently used page can be
freed in a few steps, so that the freeing cannot fill the file buffer with
bufferfixed file pages. */

#include <innodb/disk/flags.h>



#define FSEG_INODE_PAGE_NODE FSEG_PAGE_DATA
/* the list node for linking
segment inode pages */

#define FSEG_ARR_OFFSET (FSEG_PAGE_DATA + FLST_NODE_SIZE)
/*-------------------------------------*/
#define FSEG_ID                             \
  0 /* 8 bytes of segment id: if this is 0, \
    it means that the header is unused */
#define FSEG_NOT_FULL_N_USED 8
/* number of used segment pages in
the FSEG_NOT_FULL list */
#define FSEG_FREE 12
/* list of free extents of this
segment */
#define FSEG_NOT_FULL (12 + FLST_BASE_NODE_SIZE)
/* list of partially free extents */
#define FSEG_FULL (12 + 2 * FLST_BASE_NODE_SIZE)
/* list of full extents */
#define FSEG_MAGIC_N (12 + 3 * FLST_BASE_NODE_SIZE)
/* magic number used in debugging */
#define FSEG_FRAG_ARR (16 + 3 * FLST_BASE_NODE_SIZE)
/* array of individual pages
belonging to this segment in fsp
fragment extent lists */
#define FSEG_FRAG_ARR_N_SLOTS (FSP_EXTENT_SIZE / 2)
/* number of slots in the array for
the fragment pages */
#define FSEG_FRAG_SLOT_SIZE              \
  4 /* a fragment page slot contains its \
    page number within space, FIL_NULL   \
    means that the slot is not in use */
/*-------------------------------------*/
#define FSEG_INODE_SIZE \
  (16 + 3 * FLST_BASE_NODE_SIZE + FSEG_FRAG_ARR_N_SLOTS * FSEG_FRAG_SLOT_SIZE)

#define FSP_SEG_INODES_PER_PAGE(page_size) \
  ((page_size.physical() - FSEG_ARR_OFFSET - 10) / FSEG_INODE_SIZE)
/* Number of segment inodes which fit on a
single page */

#define FSEG_MAGIC_N_VALUE 97937874


#define FSEG_FILLFACTOR                  \
  8 /* If this value is x, then if       \
    the number of unused but reserved    \
    pages in a segment is less than      \
    reserved pages * 1/x, and there are  \
    at least FSEG_FRAG_LIMIT used pages, \
    then we allow a new empty extent to  \
    be added to the segment in           \
    fseg_alloc_free_page. Otherwise, we  \
    use unused pages of the segment. */

#define FSEG_FRAG_LIMIT FSEG_FRAG_ARR_N_SLOTS
/* If the segment has >= this many
used pages, it may be expanded by
allocating extents to the segment;
until that only individual fragment
pages are allocated from the space */

#define FSEG_FREE_LIST_LIMIT              \
  40 /* If the reserved size of a segment \
     is at least this many extents, we    \
     allow extents to be put to the free  \
     list of the extent: at most          \
     FSEG_FREE_LIST_MAX_LEN many */
#define FSEG_FREE_LIST_MAX_LEN 4
/* @} */











/** Size of the doublewrite block in pages */
#define TRX_SYS_DOUBLEWRITE_BLOCK_SIZE FSP_EXTENT_SIZE





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






/** Default undo tablespace size in UNIV_PAGEs count (10MB). */
constexpr page_no_t SRV_UNDO_TABLESPACE_SIZE_IN_PAGES =
    ((1024 * 1024) * 10) / UNIV_PAGE_SIZE_DEF;





/** Server version that the tablespace created */
const uint32 DD_SPACE_CURRENT_SRV_VERSION = MYSQL_VERSION_ID;

/** The tablespace version that the tablespace created */
const uint32 DD_SPACE_CURRENT_SPACE_VERSION = 1;
