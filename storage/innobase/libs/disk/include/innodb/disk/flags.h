#pragma once


#include <innodb/univ/univ.h>

/** The byte offsets on a file page for various variables. */

/** MySQL-4.0.14 space id the page belongs to (== 0) but in later
versions the 'new' checksum of the page */
#define FIL_PAGE_SPACE_OR_CHKSUM 0

/** page offset inside space */
#define FIL_PAGE_OFFSET 4

/** if there is a 'natural' predecessor of the page, its offset.
Otherwise FIL_NULL. This field is not set on BLOB pages, which are stored as a
singly-linked list. See also FIL_PAGE_NEXT. */
#define FIL_PAGE_PREV 8

/** On page 0 of the tablespace, this is the server version ID */
#define FIL_PAGE_SRV_VERSION 8

/** if there is a 'natural' successor of the page, its offset. Otherwise
FIL_NULL. B-tree index pages(FIL_PAGE_TYPE contains FIL_PAGE_INDEX) on the
same PAGE_LEVEL are maintained as a doubly linked list via FIL_PAGE_PREV and
FIL_PAGE_NEXT in the collation order of the smallest user record on each
page. */
#define FIL_PAGE_NEXT 12

/** On page 0 of the tablespace, this is the server version ID */
#define FIL_PAGE_SPACE_VERSION 12

/** lsn of the end of the newest modification log record to the page */
#define FIL_PAGE_LSN 16

/** this is only defined for the first page of the system tablespace: the file
has been flushed to disk at least up to this LSN. For FIL_PAGE_COMPRESSED
pages, we store the compressed page control information in these 8 bytes. */
#define FIL_PAGE_FILE_FLUSH_LSN 26

/** If page type is FIL_PAGE_COMPRESSED then the 8 bytes starting at
FIL_PAGE_FILE_FLUSH_LSN are broken down as follows: */

/** Control information version format (u8) */
constexpr ulint FIL_PAGE_VERSION = FIL_PAGE_FILE_FLUSH_LSN;

/** Compression algorithm (u8) */
constexpr ulint FIL_PAGE_ALGORITHM_V1 = FIL_PAGE_VERSION + 1;

/** Original page type (u16) */
constexpr ulint FIL_PAGE_ORIGINAL_TYPE_V1 = FIL_PAGE_ALGORITHM_V1 + 1;

/** Original data size in bytes (u16)*/
constexpr ulint FIL_PAGE_ORIGINAL_SIZE_V1 = FIL_PAGE_ORIGINAL_TYPE_V1 + 2;

/** Size after compression (u16) */
constexpr ulint FIL_PAGE_COMPRESS_SIZE_V1 = FIL_PAGE_ORIGINAL_SIZE_V1 + 2;

/** This overloads FIL_PAGE_FILE_FLUSH_LSN for RTREE Split Sequence Number */
constexpr ulint FIL_RTREE_SPLIT_SEQ_NUM = FIL_PAGE_FILE_FLUSH_LSN;

/** starting from 4.1.x this contains the space id of the page */
constexpr ulint FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID = 34;

/** alias for space id */
#define FIL_PAGE_SPACE_ID FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID

/** start of the data on the page */
constexpr ulint FIL_PAGE_DATA = 38;

/** File page trailer */
/** the low 4 bytes of this are used to store the page checksum, the
last 4 bytes should be identical to the last 4 bytes of FIL_PAGE_LSN */
constexpr ulint FIL_PAGE_END_LSN_OLD_CHKSUM = 8;

/** size of the page trailer */
constexpr ulint FIL_PAGE_DATA_END = 8;

/** First in address is the page offset. */
constexpr size_t FIL_ADDR_PAGE = 0;

/** Then comes 2-byte byte offset within page.*/
constexpr size_t FIL_ADDR_BYTE = 4;

/** Address size is 6 bytes. */
constexpr size_t FIL_ADDR_SIZE = 6;

/** Path separator e.g., 'dir;...;dirN' */
constexpr char FIL_PATH_SEPARATOR = ';';

/** The next value should be smaller or equal to the smallest sector size used
on any disk. A log block is required to be a portion of disk which is written
so that if the start and the end of a block get written to disk, then the
whole block gets written. This should be true even in most cases of a crash:
if this fails for a log block, then it is equivalent to a media failure in the
log. */
#define OS_FILE_LOG_BLOCK_SIZE 512

/** Size of log file's header. */
constexpr uint32_t LOG_FILE_HDR_SIZE = 4 * OS_FILE_LOG_BLOCK_SIZE;


/*
 * Page TABLESPACE Flags
 */

/** On a page of any file segment, data may be put starting from this
offset */
#define FSEG_PAGE_DATA FIL_PAGE_DATA


/* @defgroup fsp_flags InnoDB Tablespace Flag Constants @{ */

/** Width of the POST_ANTELOPE flag */
#define FSP_FLAGS_WIDTH_POST_ANTELOPE 1


/** Number of flag bits used to indicate the tablespace zip page size */
#define FSP_FLAGS_WIDTH_ZIP_SSIZE 4


/** Width of the ATOMIC_BLOBS flag.  The ability to break up a long
column into an in-record prefix and an externally stored part is available
to ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT. */
#define FSP_FLAGS_WIDTH_ATOMIC_BLOBS 1


/** Number of flag bits used to indicate the tablespace page size */
#define FSP_FLAGS_WIDTH_PAGE_SSIZE 4


/** Width of the DATA_DIR flag.  This flag indicates that the tablespace
is found in a remote location, not the default data directory. */
#define FSP_FLAGS_WIDTH_DATA_DIR 1


/** Width of the SHARED flag.  This flag indicates that the tablespace
was created with CREATE TABLESPACE and can be shared by multiple tables. */
#define FSP_FLAGS_WIDTH_SHARED 1


/** Width of the TEMPORARY flag.  This flag indicates that the tablespace
is a temporary tablespace and everything in it is temporary, meaning that
it is for a single client and should be deleted upon startup if it exists. */
#define FSP_FLAGS_WIDTH_TEMPORARY 1


/** Width of the encryption flag.  This flag indicates that the tablespace
is a tablespace with encryption. */
#define FSP_FLAGS_WIDTH_ENCRYPTION 1


/** Width of the SDI flag.  This flag indicates the presence of
tablespace dictionary.*/
#define FSP_FLAGS_WIDTH_SDI 1


/** Width of all the currently known tablespace flags */
#define FSP_FLAGS_WIDTH                                        \
  (FSP_FLAGS_WIDTH_POST_ANTELOPE + FSP_FLAGS_WIDTH_ZIP_SSIZE + \
   FSP_FLAGS_WIDTH_ATOMIC_BLOBS + FSP_FLAGS_WIDTH_PAGE_SSIZE + \
   FSP_FLAGS_WIDTH_DATA_DIR + FSP_FLAGS_WIDTH_SHARED +         \
   FSP_FLAGS_WIDTH_TEMPORARY + FSP_FLAGS_WIDTH_ENCRYPTION +    \
   FSP_FLAGS_WIDTH_SDI)


/** A mask of all the known/used bits in tablespace flags */
#define FSP_FLAGS_MASK (~(~0U << FSP_FLAGS_WIDTH))


/** Zero relative shift position of the POST_ANTELOPE field */
#define FSP_FLAGS_POS_POST_ANTELOPE 0


/** Zero relative shift position of the ZIP_SSIZE field */
#define FSP_FLAGS_POS_ZIP_SSIZE \
  (FSP_FLAGS_POS_POST_ANTELOPE + FSP_FLAGS_WIDTH_POST_ANTELOPE)


/** Zero relative shift position of the ATOMIC_BLOBS field */
#define FSP_FLAGS_POS_ATOMIC_BLOBS \
  (FSP_FLAGS_POS_ZIP_SSIZE + FSP_FLAGS_WIDTH_ZIP_SSIZE)


/** Zero relative shift position of the PAGE_SSIZE field */
#define FSP_FLAGS_POS_PAGE_SSIZE \
  (FSP_FLAGS_POS_ATOMIC_BLOBS + FSP_FLAGS_WIDTH_ATOMIC_BLOBS)


/** Zero relative shift position of the start of the DATA_DIR bit */
#define FSP_FLAGS_POS_DATA_DIR \
  (FSP_FLAGS_POS_PAGE_SSIZE + FSP_FLAGS_WIDTH_PAGE_SSIZE)


/** Zero relative shift position of the start of the SHARED bit */
#define FSP_FLAGS_POS_SHARED (FSP_FLAGS_POS_DATA_DIR + FSP_FLAGS_WIDTH_DATA_DIR)


/** Zero relative shift position of the start of the TEMPORARY bit */
#define FSP_FLAGS_POS_TEMPORARY (FSP_FLAGS_POS_SHARED + FSP_FLAGS_WIDTH_SHARED)


/** Zero relative shift position of the start of the ENCRYPTION bit */
#define FSP_FLAGS_POS_ENCRYPTION \
  (FSP_FLAGS_POS_TEMPORARY + FSP_FLAGS_WIDTH_TEMPORARY)


/** Zero relative shift position of the start of the SDI bits */
#define FSP_FLAGS_POS_SDI \
  (FSP_FLAGS_POS_ENCRYPTION + FSP_FLAGS_WIDTH_ENCRYPTION)


/** Zero relative shift position of the start of the UNUSED bits */
#define FSP_FLAGS_POS_UNUSED (FSP_FLAGS_POS_SDI + FSP_FLAGS_WIDTH_SDI)


/** Bit mask of the POST_ANTELOPE field */
#define FSP_FLAGS_MASK_POST_ANTELOPE \
  ((~(~0U << FSP_FLAGS_WIDTH_POST_ANTELOPE)) << FSP_FLAGS_POS_POST_ANTELOPE)


/** Bit mask of the ZIP_SSIZE field */
#define FSP_FLAGS_MASK_ZIP_SSIZE \
  ((~(~0U << FSP_FLAGS_WIDTH_ZIP_SSIZE)) << FSP_FLAGS_POS_ZIP_SSIZE)


/** Bit mask of the ATOMIC_BLOBS field */
#define FSP_FLAGS_MASK_ATOMIC_BLOBS \
  ((~(~0U << FSP_FLAGS_WIDTH_ATOMIC_BLOBS)) << FSP_FLAGS_POS_ATOMIC_BLOBS)


/** Bit mask of the PAGE_SSIZE field */
#define FSP_FLAGS_MASK_PAGE_SSIZE \
  ((~(~0U << FSP_FLAGS_WIDTH_PAGE_SSIZE)) << FSP_FLAGS_POS_PAGE_SSIZE)


/** Bit mask of the DATA_DIR field */
#define FSP_FLAGS_MASK_DATA_DIR \
  ((~(~0U << FSP_FLAGS_WIDTH_DATA_DIR)) << FSP_FLAGS_POS_DATA_DIR)


/** Bit mask of the SHARED field */
#define FSP_FLAGS_MASK_SHARED \
  ((~(~0U << FSP_FLAGS_WIDTH_SHARED)) << FSP_FLAGS_POS_SHARED)


/** Bit mask of the TEMPORARY field */
#define FSP_FLAGS_MASK_TEMPORARY \
  ((~(~0U << FSP_FLAGS_WIDTH_TEMPORARY)) << FSP_FLAGS_POS_TEMPORARY)


/** Bit mask of the ENCRYPTION field */
#define FSP_FLAGS_MASK_ENCRYPTION \
  ((~(~0U << FSP_FLAGS_WIDTH_ENCRYPTION)) << FSP_FLAGS_POS_ENCRYPTION)


/** Bit mask of the SDI field */
#define FSP_FLAGS_MASK_SDI \
  ((~(~0U << FSP_FLAGS_WIDTH_SDI)) << FSP_FLAGS_POS_SDI)


/** Return the value of the POST_ANTELOPE field */
#define FSP_FLAGS_GET_POST_ANTELOPE(flags) \
  ((flags & FSP_FLAGS_MASK_POST_ANTELOPE) >> FSP_FLAGS_POS_POST_ANTELOPE)


/** Return the value of the ZIP_SSIZE field */
#define FSP_FLAGS_GET_ZIP_SSIZE(flags) \
  ((flags & FSP_FLAGS_MASK_ZIP_SSIZE) >> FSP_FLAGS_POS_ZIP_SSIZE)


/** Return the value of the ATOMIC_BLOBS field */
#define FSP_FLAGS_HAS_ATOMIC_BLOBS(flags) \
  ((flags & FSP_FLAGS_MASK_ATOMIC_BLOBS) >> FSP_FLAGS_POS_ATOMIC_BLOBS)


/** Return the value of the PAGE_SSIZE field */
#define FSP_FLAGS_GET_PAGE_SSIZE(flags) \
  ((flags & FSP_FLAGS_MASK_PAGE_SSIZE) >> FSP_FLAGS_POS_PAGE_SSIZE)


/** Return the value of the DATA_DIR field */
#define FSP_FLAGS_HAS_DATA_DIR(flags) \
  ((flags & FSP_FLAGS_MASK_DATA_DIR) >> FSP_FLAGS_POS_DATA_DIR)


/** Return the contents of the SHARED field */
#define FSP_FLAGS_GET_SHARED(flags) \
  ((flags & FSP_FLAGS_MASK_SHARED) >> FSP_FLAGS_POS_SHARED)


/** Return the contents of the TEMPORARY field */
#define FSP_FLAGS_GET_TEMPORARY(flags) \
  ((flags & FSP_FLAGS_MASK_TEMPORARY) >> FSP_FLAGS_POS_TEMPORARY)


/** Return the contents of the ENCRYPTION field */
#define FSP_FLAGS_GET_ENCRYPTION(flags) \
  ((flags & FSP_FLAGS_MASK_ENCRYPTION) >> FSP_FLAGS_POS_ENCRYPTION)


/** Return the value of the SDI field */
#define FSP_FLAGS_HAS_SDI(flags) \
  ((flags & FSP_FLAGS_MASK_SDI) >> FSP_FLAGS_POS_SDI)


/** Return the contents of the UNUSED bits */
#define FSP_FLAGS_GET_UNUSED(flags) (flags >> FSP_FLAGS_POS_UNUSED)


/** Return true if flags are not set */
#define FSP_FLAGS_ARE_NOT_SET(flags) ((flags & FSP_FLAGS_MASK) == 0)


/** Set ENCRYPTION bit in tablespace flags */
UNIV_INLINE void fsp_flags_set_encryption(uint32_t &flags) {
  flags |= FSP_FLAGS_MASK_ENCRYPTION;
}


/** Set ENCRYPTION bit in tablespace flags */
UNIV_INLINE void fsp_flags_unset_encryption(uint32_t &flags) {
  flags &= ~FSP_FLAGS_MASK_ENCRYPTION;
}


/** Set SDI Index bit in tablespace flags */
UNIV_INLINE void fsp_flags_set_sdi(uint32_t &flags) {
  flags |= FSP_FLAGS_MASK_SDI;
}


/** Set SDI Index bit in tablespace flags */
UNIV_INLINE void fsp_flags_unset_sdi(uint32_t &flags) {
  flags &= ~FSP_FLAGS_MASK_SDI;
}


/* @} */


/* The following definitions would better belong to page0zip.h,
but we cannot include page0zip.h from rem0rec.ic, because
page0*.h includes rem0rec.h and may include rem0rec.ic. */

/** Number of bits needed for representing different compressed page sizes */
#define PAGE_ZIP_SSIZE_BITS 3

/** Maximum compressed page shift size */
#define PAGE_ZIP_SSIZE_MAX \
  (UNIV_ZIP_SIZE_SHIFT_MAX - UNIV_ZIP_SIZE_SHIFT_MIN + 1)

/* Make sure there are enough bits available to store the maximum zip
ssize, which is the number of shifts from 512. */
#if PAGE_ZIP_SSIZE_MAX >= (1 << PAGE_ZIP_SSIZE_BITS)
#error "PAGE_ZIP_SSIZE_MAX >= (1 << PAGE_ZIP_SSIZE_BITS)"
#endif
