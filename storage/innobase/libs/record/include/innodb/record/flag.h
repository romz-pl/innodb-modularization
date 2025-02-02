#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/dict_types/flags.h>
#include <innodb/dict_mem/flags.h>

/* Compact flag ORed to the extra size returned by rec_get_offsets() */
#define REC_OFFS_COMPACT ((ulint)1 << 31)
/* SQL NULL flag in offsets returned by rec_get_offsets() */
#define REC_OFFS_SQL_NULL ((ulint)1 << 31)
/* External flag in offsets returned by rec_get_offsets() */
#define REC_OFFS_EXTERNAL ((ulint)1 << 30)
/* Default value flag in offsets returned by rec_get_offsets() */
#define REC_OFFS_DEFAULT ((ulint)1 << 29)
/* Mask for offsets returned by rec_get_offsets() */
#define REC_OFFS_MASK (REC_OFFS_DEFAULT - 1)

/* The offset of heap_no in a compact record */
#define REC_NEW_HEAP_NO 4
/* The shift of heap_no in a compact record.
The status is stored in the low-order bits. */
#define REC_HEAP_NO_SHIFT 3

/* We list the byte offsets from the origin of the record, the mask,
and the shift needed to obtain each bit-field of the record. */

#define REC_NEXT 2
#define REC_NEXT_MASK 0xFFFFUL
#define REC_NEXT_SHIFT 0

#define REC_OLD_SHORT 3 /* This is single byte bit-field */
#define REC_OLD_SHORT_MASK 0x1UL
#define REC_OLD_SHORT_SHIFT 0

#define REC_OLD_N_FIELDS 4
#define REC_OLD_N_FIELDS_MASK 0x7FEUL
#define REC_OLD_N_FIELDS_SHIFT 1

#define REC_NEW_STATUS 3 /* This is single byte bit-field */
#define REC_NEW_STATUS_MASK 0x7UL
#define REC_NEW_STATUS_SHIFT 0

#define REC_OLD_HEAP_NO 5
#define REC_HEAP_NO_MASK 0xFFF8UL
#if 0 /* defined in rem0rec.h for use of page0zip.cc */
#define REC_NEW_HEAP_NO 4
#define REC_HEAP_NO_SHIFT 3
#endif

#define REC_OLD_N_OWNED 6 /* This is single byte bit-field */
#define REC_NEW_N_OWNED 5 /* This is single byte bit-field */
#define REC_N_OWNED_MASK 0xFUL
#define REC_N_OWNED_SHIFT 0

#define REC_OLD_INFO_BITS 6 /* This is single byte bit-field */
#define REC_NEW_INFO_BITS 5 /* This is single byte bit-field */
#define REC_TMP_INFO_BITS 1 /* This is single byte bit-field */
#define REC_INFO_BITS_MASK 0xF0UL
#define REC_INFO_BITS_SHIFT 0

#if REC_OLD_SHORT_MASK << (8 * (REC_OLD_SHORT - 3)) ^       \
    REC_OLD_N_FIELDS_MASK << (8 * (REC_OLD_N_FIELDS - 4)) ^ \
    REC_HEAP_NO_MASK << (8 * (REC_OLD_HEAP_NO - 4)) ^       \
    REC_N_OWNED_MASK << (8 * (REC_OLD_N_OWNED - 3)) ^       \
    REC_INFO_BITS_MASK << (8 * (REC_OLD_INFO_BITS - 3)) ^ 0xFFFFFFFFUL
#error "sum of old-style masks != 0xFFFFFFFFUL"
#endif
#if REC_NEW_STATUS_MASK << (8 * (REC_NEW_STATUS - 3)) ^ \
    REC_HEAP_NO_MASK << (8 * (REC_NEW_HEAP_NO - 4)) ^   \
    REC_N_OWNED_MASK << (8 * (REC_NEW_N_OWNED - 3)) ^   \
    REC_INFO_BITS_MASK << (8 * (REC_NEW_INFO_BITS - 3)) ^ 0xFFFFFFUL
#error "sum of new-style masks != 0xFFFFFFUL"
#endif

/* Info bit denoting the predefined minimum record: this bit is set
if and only if the record is the first user record on a non-leaf
B-tree page that is the leftmost page on its level
(PAGE_LEVEL is nonzero and FIL_PAGE_PREV is FIL_NULL). */
#define REC_INFO_MIN_REC_FLAG 0x10UL
/* The deleted flag in info bits */
#define REC_INFO_DELETED_FLAG                  \
  0x20UL /* when bit is set to 1, it means the \
         record has been delete marked */
/* The 0x40UL can also be used in the future */
/* The instant ADD COLUMN flag. When it is set to 1, it means this record
was inserted/updated after an instant ADD COLUMN. */
#define REC_INFO_INSTANT_FLAG 0x80UL



/* Record status values */
#define REC_STATUS_ORDINARY 0
#define REC_STATUS_NODE_PTR 1
#define REC_STATUS_INFIMUM 2
#define REC_STATUS_SUPREMUM 3

/* The following four constants are needed in page0zip.cc in order to
efficiently compress and decompress pages. */

/* Length of a B-tree node pointer, in bytes */
#define REC_NODE_PTR_SIZE 4

/** SQL null flag in a 1-byte offset of ROW_FORMAT=REDUNDANT records */
#define REC_1BYTE_SQL_NULL_MASK 0x80UL
/** SQL null flag in a 2-byte offset of ROW_FORMAT=REDUNDANT records */
#define REC_2BYTE_SQL_NULL_MASK 0x8000UL

/** In a 2-byte offset of ROW_FORMAT=REDUNDANT records, the second most
significant bit denotes that the tail of a field is stored off-page. */
#define REC_2BYTE_EXTERN_MASK 0x4000UL

#ifdef UNIV_DEBUG
/* Length of the rec_get_offsets() header */
#define REC_OFFS_HEADER_SIZE 4
#else /* UNIV_DEBUG */
/* Length of the rec_get_offsets() header */
#define REC_OFFS_HEADER_SIZE 2
#endif /* UNIV_DEBUG */

/* Number of elements that should be initially allocated for the
offsets[] array, first passed to rec_get_offsets() */
#define REC_OFFS_NORMAL_SIZE 100
#define REC_OFFS_SMALL_SIZE 10



/** Number of fields flag which means it occupies two bytes */
static const uint8_t REC_N_FIELDS_TWO_BYTES_FLAG = 0x80;

/** Max number of fields which can be stored in one byte */
static const uint8_t REC_N_FIELDS_ONE_BYTE_MAX = 0x7F;



#define REC_MAX_HEAP_NO (2 * 8192 - 1)
#define REC_MAX_N_OWNED (16 - 1)

/* Maximum number of user defined fields/columns. The reserved columns
are the ones InnoDB adds internally: DB_ROW_ID, DB_TRX_ID, DB_ROLL_PTR.
We need "* 2" because mlog_parse_index() creates a dummy table object
possibly, with some of the system columns in it, and then adds the 3
system columns (again) using dict_table_add_system_columns(). The problem
is that mlog_parse_index() cannot recognize the system columns by
just having n_fields, n_uniq and the lengths of the columns. */
#define REC_MAX_N_USER_FIELDS (REC_MAX_N_FIELDS - DATA_N_SYS_COLS * 2)






/* Maximum lengths for the data in a physical record if the offsets
are given in one byte (resp. two byte) format. */
constexpr ulint REC_1BYTE_OFFS_LIMIT = 0x7FUL;
constexpr ulint REC_2BYTE_OFFS_LIMIT = 0x7FFFUL;

/* The data size of record must be smaller than this because we reserve
two upmost bits in a two byte offset for special purposes */
constexpr ulint REC_MAX_DATA_SIZE = 16384;




