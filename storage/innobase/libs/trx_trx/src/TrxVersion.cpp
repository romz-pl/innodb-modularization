#include <innodb/trx_trx/TrxVersion.h>

#include <innodb/trx_trx/trx_t.h>

/** Constructor */
TrxVersion::TrxVersion(trx_t *trx) : m_trx(trx), m_version(trx->version) {
  /* No op */
}
