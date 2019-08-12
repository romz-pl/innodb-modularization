#include <innodb/data_types/dtuple_set_info_bits.h>

#include <innodb/data_types/dtuple_t.h>

/** Sets info bits in a data tuple. */
void dtuple_set_info_bits(dtuple_t *tuple, /*!< in: tuple */
                          ulint info_bits) /*!< in: info bits */
{
  ut_ad(tuple);

  tuple->info_bits = info_bits;
}
