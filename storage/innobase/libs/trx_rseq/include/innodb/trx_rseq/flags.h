#pragma once

#include <innodb/univ/univ.h>

/* Number of undo log slots in a rollback segment file copy */
#define TRX_RSEG_N_SLOTS (UNIV_PAGE_SIZE / 16)

/* Maximum number of transactions supported by a single rollback segment */
#define TRX_RSEG_MAX_N_TRXS (TRX_RSEG_N_SLOTS / 2)

/* Undo log segment slot in a rollback segment header */
/*-------------------------------------------------------------*/
#define TRX_RSEG_SLOT_PAGE_NO            \
  0 /* Page number of the header page of \
    an undo log segment */
/*-------------------------------------------------------------*/
/* Slot size */
#define TRX_RSEG_SLOT_SIZE 4

/* The offset of the rollback segment header on its page */
#define TRX_RSEG FSEG_PAGE_DATA

/* Transaction rollback segment header */
/*-------------------------------------------------------------*/
#define TRX_RSEG_MAX_SIZE                \
  0 /* Maximum allowed size for rollback \
    segment in pages */
#define TRX_RSEG_HISTORY_SIZE        \
  4 /* Number of file pages occupied \
    by the logs in the history list */
#define TRX_RSEG_HISTORY                  \
  8 /* The update undo logs for committed \
    transactions */
#define TRX_RSEG_FSEG_HEADER (8 + FLST_BASE_NODE_SIZE)
/* Header for the file segment where
this page is placed */
#define TRX_RSEG_UNDO_SLOTS (8 + FLST_BASE_NODE_SIZE + FSEG_HEADER_SIZE)
/* Undo log segment slots */
/*-------------------------------------------------------------*/

/** The offset of the Rollback Segment Directory header on an RSEG_ARRAY page */
#define RSEG_ARRAY_HEADER FSEG_PAGE_DATA

/** Rollback Segment Array Header */
/*------------------------------------------------------------- */
/** The RSEG ARRAY base version is a number derived from the string
'RSEG' [0x 52 53 45 47] for extra validation. Each new version
increments the base version by 1. */
#define RSEG_ARRAY_VERSION 0x52534547 + 1

/** The RSEG ARRAY version offset in the header. */
#define RSEG_ARRAY_VERSION_OFFSET 0

/** The current number of rollback segments being tracked in this array */
#define RSEG_ARRAY_SIZE_OFFSET 4

/** This is the pointer to the file segment inode that tracks this
rseg array page. */
#define RSEG_ARRAY_FSEG_HEADER_OFFSET 8

/** The start of the array of rollback segment header page numbers for this
undo tablespace. The potential size of this array is limited only by the
page size minus overhead. The actual size of the array is limited by
srv_rollback_segments. */
#define RSEG_ARRAY_PAGES_OFFSET (8 + FSEG_HEADER_SIZE)

/** Reserved space at the end of an RSEG_ARRAY page reserved for future use. */
#define RSEG_ARRAY_RESERVED_BYTES 200

/* Slot size of the array of rollback segment header page numbers */
#define RSEG_ARRAY_SLOT_SIZE 4
/*------------------------------------------------------------- */

