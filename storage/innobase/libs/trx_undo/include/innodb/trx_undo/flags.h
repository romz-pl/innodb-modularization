#pragma once

#include <innodb/univ/univ.h>

/* Types of an undo log segment */
#define TRX_UNDO_INSERT 1 /* contains undo entries for inserts */
#define TRX_UNDO_UPDATE                  \
  2 /* contains undo entries for updates \
    and delete markings: in short,       \
    modifys (the name 'UPDATE' is a      \
    historical relic) */
/* States of an undo log segment */
#define TRX_UNDO_ACTIVE                                         \
  1                        /* contains an undo log of an active \
                           transaction */
#define TRX_UNDO_CACHED 2  /* cached for quick reuse */
#define TRX_UNDO_TO_FREE 3 /* insert undo segment can be freed */
#define TRX_UNDO_TO_PURGE                 \
  4 /* update undo segment will not be    \
    reused: it can be freed in purge when \
    all undo data in it is removed */
#define TRX_UNDO_PREPARED         \
  5 /* contains an undo log of an \
    prepared transaction */




/** The offset of the undo log page header on pages of the undo log */
#define TRX_UNDO_PAGE_HDR FSEG_PAGE_DATA
/*-------------------------------------------------------------*/
/** Transaction undo log page header offsets */
/* @{ */
#define TRX_UNDO_PAGE_TYPE  \
  0 /*!< TRX_UNDO_INSERT or \
    TRX_UNDO_UPDATE */
#define TRX_UNDO_PAGE_START               \
  2 /*!< Byte offset where the undo log   \
    records for the LATEST transaction    \
    start on this page (remember that     \
    in an update undo log, the first page \
    can contain several undo logs) */
#define TRX_UNDO_PAGE_FREE                 \
  4 /*!< On each page of the undo log this \
    field contains the byte offset of the  \
    first free byte on the page */
#define TRX_UNDO_PAGE_NODE               \
  6 /*!< The file list node in the chain \
    of undo log pages */
/*-------------------------------------------------------------*/
#define TRX_UNDO_PAGE_HDR_SIZE (6 + FLST_NODE_SIZE)
/*!< Size of the transaction undo
log page header, in bytes */
/* @} */

/** An update undo segment with just one page can be reused if it has
at most this many bytes used; we must leave space at least for one new undo
log header on the page */

#define TRX_UNDO_PAGE_REUSE_LIMIT (3 * UNIV_PAGE_SIZE / 4)

/* An update undo log segment may contain several undo logs on its first page
if the undo logs took so little space that the segment could be cached and
reused. All the undo log headers are then on the first page, and the last one
owns the undo log records on subsequent pages if the segment is bigger than
one page. If an undo log is stored in a segment, then on the first page it is
allowed to have zero undo records, but if the segment extends to several
pages, then all the rest of the pages must contain at least one undo log
record. */

/** The offset of the undo log segment header on the first page of the undo
log segment */

#define TRX_UNDO_SEG_HDR (TRX_UNDO_PAGE_HDR + TRX_UNDO_PAGE_HDR_SIZE)
/** Undo log segment header */
/* @{ */
/*-------------------------------------------------------------*/
#define TRX_UNDO_STATE 0 /*!< TRX_UNDO_ACTIVE, ... */

#define TRX_UNDO_LAST_LOG                   \
  2 /*!< Offset of the last undo log header \
    on the segment header page, 0 if        \
    none */
#define TRX_UNDO_FSEG_HEADER               \
  4 /*!< Header for the file segment which \
    the undo log segment occupies */
#define TRX_UNDO_PAGE_LIST (4 + FSEG_HEADER_SIZE)
/*!< Base node for the list of pages in
the undo log segment; defined only on
the undo log segment's first page */
/*-------------------------------------------------------------*/
/** Size of the undo log segment header */
#define TRX_UNDO_SEG_HDR_SIZE (4 + FSEG_HEADER_SIZE + FLST_BASE_NODE_SIZE)
/* @} */

/** The undo log header. There can be several undo log headers on the first
page of an update undo log segment. */
/* @{ */
/*-------------------------------------------------------------*/
#define TRX_UNDO_TRX_ID 0 /*!< Transaction id */
#define TRX_UNDO_TRX_NO                  \
  8 /*!< Transaction number of the       \
    transaction; defined only if the log \
    is in a history list */
#define TRX_UNDO_DEL_MARKS                 \
  16 /*!< Defined only in an update undo   \
     log: TRUE if the transaction may have \
     done delete markings of records, and  \
     thus purge is necessary */
#define TRX_UNDO_LOG_START                    \
  18 /*!< Offset of the first undo log record \
     of this log on the header page; purge    \
     may remove undo log record from the      \
     log start, and therefore this is not     \
     necessarily the same as this log         \
     header end offset */
#define TRX_UNDO_XID_EXISTS                \
  20 /*!< TRUE if undo log header includes \
     X/Open XA transaction identification  \
     XID */
#define TRX_UNDO_DICT_TRANS                  \
  21 /*!< TRUE if the transaction is a table \
     create, index create, or drop           \
     transaction: in recovery                \
     the transaction cannot be rolled back   \
     in the usual way: a 'rollback' rather   \
     means dropping the created or dropped   \
     table, if it still exists */
#define TRX_UNDO_TABLE_ID                  \
  22 /*!< Id of the table if the preceding \
     field is TRUE. Note: deprecated */
#define TRX_UNDO_NEXT_LOG                    \
  30 /*!< Offset of the next undo log header \
     on this page, 0 if none */
#define TRX_UNDO_PREV_LOG                 \
  32 /*!< Offset of the previous undo log \
     header on this page, 0 if none */
#define TRX_UNDO_HISTORY_NODE              \
  34 /*!< If the log is put to the history \
     list, the file list node is here */
/*-------------------------------------------------------------*/
/** Size of the undo log header without XID information */
#define TRX_UNDO_LOG_OLD_HDR_SIZE (34 + FLST_NODE_SIZE)

/* Note: the writing of the undo log old header is coded by a log record
MLOG_UNDO_HDR_CREATE or MLOG_UNDO_HDR_REUSE. The appending of an XID to the
header is logged separately. In this sense, the XID is not really a member
of the undo log header. TODO: do not append the XID to the log header if XA
is not needed by the user. The XID wastes about 150 bytes of space in every
undo log. In the history list we may have millions of undo logs, which means
quite a large overhead. */

/** X/Open XA Transaction Identification (XID) */
/* @{ */
/** xid_t::formatID */
#define TRX_UNDO_XA_FORMAT (TRX_UNDO_LOG_OLD_HDR_SIZE)
/** xid_t::gtrid_length */
#define TRX_UNDO_XA_TRID_LEN (TRX_UNDO_XA_FORMAT + 4)
/** xid_t::bqual_length */
#define TRX_UNDO_XA_BQUAL_LEN (TRX_UNDO_XA_TRID_LEN + 4)
/** Distributed transaction identifier data */
#define TRX_UNDO_XA_XID (TRX_UNDO_XA_BQUAL_LEN + 4)
/*--------------------------------------------------------------*/
#define TRX_UNDO_LOG_XA_HDR_SIZE (TRX_UNDO_XA_XID + XIDDATASIZE)
/*!< Total size of the undo log header
with the XA XID */
/* @} */
