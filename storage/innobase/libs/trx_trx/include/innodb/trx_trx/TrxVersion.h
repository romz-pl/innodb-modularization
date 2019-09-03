#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

struct TrxVersion {
  TrxVersion(trx_t *trx);

  /**
  @return true if the trx_t instance is the same */
  bool operator==(const TrxVersion &rhs) const { return (rhs.m_trx == m_trx); }

  trx_t *m_trx;
  ulint m_version;
};
