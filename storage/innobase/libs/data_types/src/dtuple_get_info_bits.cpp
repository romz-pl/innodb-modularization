#include <innodb/data_types/dtuple_get_info_bits.h>

#include <innodb/data_types/dtuple_t.h>

/** Gets info bits in a data tuple.
 @return info bits */
ulint dtuple_get_info_bits(const dtuple_t *tuple) /*!< in: tuple */
{
  ut_ad(tuple);

  return (tuple->info_bits);
}
