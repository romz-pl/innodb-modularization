#include <innodb/data_types/dtuple_t.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/machine/data.h>

/* Read the trx id from the tuple (DB_TRX_ID)
@return transaction id of the tuple. */
trx_id_t dtuple_t::get_trx_id() const {
  for (ulint i = 0; i < n_fields; ++i) {
    dfield_t &field = fields[i];

    uint32_t prtype = field.type.prtype & DATA_SYS_PRTYPE_MASK;

    if (field.type.mtype == DATA_SYS && prtype == DATA_TRX_ID) {
      return (mach_read_from_6((byte *)field.data));
    }
  }

  return (0);
}
