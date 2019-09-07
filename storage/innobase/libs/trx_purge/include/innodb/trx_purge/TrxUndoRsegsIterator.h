#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxUndoRsegs.h>
#include <innodb/disk/page_size_t.h>

struct trx_purge_t;

/** Choose the rollback segment with the smallest trx_no. */
struct TrxUndoRsegsIterator {
  /** Constructor */
  TrxUndoRsegsIterator(trx_purge_t *purge_sys);

  /** Sets the next rseg to purge in m_purge_sys.
  @return page size of the table for which the log is.
  NOTE: if rseg is NULL when this function returns this means that
  there are no rollback segments to purge and then the returned page
  size object should not be used. */
  const page_size_t set_next();

 private:
  // Disable copying
  TrxUndoRsegsIterator(const TrxUndoRsegsIterator &);
  TrxUndoRsegsIterator &operator=(const TrxUndoRsegsIterator &);

  /** The purge system pointer */
  trx_purge_t *m_purge_sys;

  /** The current element to process */
  TrxUndoRsegs m_trx_undo_rsegs;

  /** Track the current element in m_trx_undo_rseg */
  Rseg_Iterator m_iter;

  /** Sentinel value */
  static const TrxUndoRsegs NullElement;
};
