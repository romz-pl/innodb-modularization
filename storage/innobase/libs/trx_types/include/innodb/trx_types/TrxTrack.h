#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Mapping read-write transactions from id to transaction instance, for
creating read views and during trx id lookup for MVCC and locking. */
struct TrxTrack {
  explicit TrxTrack(trx_id_t id, trx_t *trx = NULL) : m_id(id), m_trx(trx) {
    // Do nothing
  }

  trx_id_t m_id;
  trx_t *m_trx;
};
