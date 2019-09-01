#include <innodb/lock_priv/lock_mode_stronger_or_eq.h>

#include <innodb/lock_priv/lock_strength_matrix.h>
#include <innodb/assert/assert.h>

/** Calculates if lock mode 1 is stronger or equal to lock mode 2.
 @return nonzero if mode1 stronger or equal to mode2 */
ulint lock_mode_stronger_or_eq(enum lock_mode mode1, /*!< in: lock mode */
                               enum lock_mode mode2) /*!< in: lock mode */
{
  ut_ad((ulint)mode1 < lock_types);
  ut_ad((ulint)mode2 < lock_types);

  return (lock_strength_matrix[mode1][mode2]);
}
