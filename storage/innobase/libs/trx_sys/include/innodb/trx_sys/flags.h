#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/flags.h>


/** The automatically created system rollback segment has this id */
#define TRX_SYS_SYSTEM_RSEG_ID 0

/** The offset of the transaction system header on the page */
#define TRX_SYS FSEG_PAGE_DATA

/** Transaction system header */
/*------------------------------------------------------------- @{ */
#define TRX_SYS_TRX_ID_STORE       \
  0 /*!< the maximum trx id or trx \
    number modulo                  \
    TRX_SYS_TRX_ID_UPDATE_MARGIN   \
    written to a file page by any  \
    transaction; the assignment of \
    transaction ids continues from \
    this number rounded up by      \
    TRX_SYS_TRX_ID_UPDATE_MARGIN   \
    plus                           \
    TRX_SYS_TRX_ID_UPDATE_MARGIN   \
    when the database is           \
    started */
#define TRX_SYS_FSEG_HEADER     \
  8 /*!< segment header for the \
    tablespace segment the trx  \
    system is created into */
#define TRX_SYS_RSEGS (8 + FSEG_HEADER_SIZE)
/*!< the start of the array of
rollback segment specification
slots */
/*------------------------------------------------------------- @} */

/* Originally, InnoDB defined TRX_SYS_N_RSEGS as 256 but created only one
rollback segment.  It initialized some arrays with this number of entries.
We must remember this limit in order to keep file compatibility. */
#define TRX_SYS_OLD_N_RSEGS 256

/* The system temporary tablespace was originally allocated rseg_id slot
numbers 1 through 32 in the TRX_SYS page.  But those slots were not used
because those Rollback segments were recreated at startup and after any
crash. These slots are now used for redo-enabled rollback segments.
The default number of rollback segments in the temporary tablespace
remains the same. */
#define TRX_SYS_OLD_TMP_RSEGS 32

/** Maximum length of MySQL binlog file name, in bytes. */
#define TRX_SYS_MYSQL_LOG_NAME_LEN 512
/** Contents of TRX_SYS_MYSQL_LOG_MAGIC_N_FLD */
#define TRX_SYS_MYSQL_LOG_MAGIC_N 873422344

#if UNIV_PAGE_SIZE_MIN < 4096
#error "UNIV_PAGE_SIZE_MIN < 4096"
#endif
/** The offset of the MySQL binlog offset info in the trx system header */
#define TRX_SYS_MYSQL_LOG_INFO (UNIV_PAGE_SIZE - 1000)
#define TRX_SYS_MYSQL_LOG_MAGIC_N_FLD \
  0 /*!< magic number which is        \
    TRX_SYS_MYSQL_LOG_MAGIC_N         \
    if we have valid data in the      \
    MySQL binlog info */
#define TRX_SYS_MYSQL_LOG_OFFSET_HIGH \
  4 /*!< high 4 bytes of the offset   \
    within that file */
#define TRX_SYS_MYSQL_LOG_OFFSET_LOW                             \
  8                               /*!< low 4 bytes of the offset \
                                  within that file */
#define TRX_SYS_MYSQL_LOG_NAME 12 /*!< MySQL log file name */

/** Doublewrite buffer */
/* @{ */
/** The offset of the doublewrite buffer header on the trx system header page */
#define TRX_SYS_DOUBLEWRITE (UNIV_PAGE_SIZE - 200)
/*-------------------------------------------------------------*/
#define TRX_SYS_DOUBLEWRITE_FSEG \
  0 /*!< fseg header of the fseg \
    containing the doublewrite   \
    buffer */
#define TRX_SYS_DOUBLEWRITE_MAGIC FSEG_HEADER_SIZE
/*!< 4-byte magic number which
shows if we already have
created the doublewrite
buffer */
#define TRX_SYS_DOUBLEWRITE_BLOCK1 (4 + FSEG_HEADER_SIZE)
/*!< page number of the
first page in the first
sequence of 64
(= FSP_EXTENT_SIZE) consecutive
pages in the doublewrite
buffer */
#define TRX_SYS_DOUBLEWRITE_BLOCK2 (8 + FSEG_HEADER_SIZE)
/*!< page number of the
first page in the second
sequence of 64 consecutive
pages in the doublewrite
buffer */
#define TRX_SYS_DOUBLEWRITE_REPEAT \
  12 /*!< we repeat                \
     TRX_SYS_DOUBLEWRITE_MAGIC,    \
     TRX_SYS_DOUBLEWRITE_BLOCK1,   \
     TRX_SYS_DOUBLEWRITE_BLOCK2    \
     so that if the trx sys        \
     header is half-written        \
     to disk, we still may         \
     be able to recover the        \
     information */
/** If this is not yet set to TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED_N,
we must reset the doublewrite buffer, because starting from 4.1.x the
space id of a data page is stored into
FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID. */
#define TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED (24 + FSEG_HEADER_SIZE)

/*-------------------------------------------------------------*/
/** Contents of TRX_SYS_DOUBLEWRITE_MAGIC */
#define TRX_SYS_DOUBLEWRITE_MAGIC_N 536853855
/** Contents of TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED */
#define TRX_SYS_DOUBLEWRITE_SPACE_ID_STORED_N 1783657386


/* @} */



/** When a trx id which is zero modulo this number (which must be a power of
two) is assigned, the field TRX_SYS_TRX_ID_STORE on the transaction system
page is updated */
#define TRX_SYS_TRX_ID_WRITE_MARGIN ((trx_id_t)256)





/* Rollback segment specification slot offsets */
/*-------------------------------------------------------------*/
#define TRX_SYS_RSEG_SPACE          \
  0 /* space where the segment      \
    header is placed; starting with \
    MySQL/InnoDB 5.1.7, this is     \
    UNIV_UNDEFINED if the slot is unused */
#define TRX_SYS_RSEG_PAGE_NO           \
  4 /*  page number where the segment  \
    header is placed; this is FIL_NULL \
    if the slot is unused */
/*-------------------------------------------------------------*/
/* Size of a rollback segment specification slot */
#define TRX_SYS_RSEG_SLOT_SIZE 8



/** Space id of the transaction system page (the system tablespace) */
static const space_id_t TRX_SYS_SPACE = 0;

/** Space id of system tablespace */
const space_id_t SYSTEM_TABLE_SPACE = TRX_SYS_SPACE;
