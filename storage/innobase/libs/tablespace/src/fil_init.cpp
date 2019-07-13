#include <innodb/tablespace/fil_init.h>

#include <innodb/tablespace/fil_system.h>

/** Initializes the tablespace memory cache.
@param[in]	max_n_open	Maximum number of open files */
void fil_init(ulint max_n_open) {
  static_assert((1 << UNIV_PAGE_SIZE_SHIFT_MAX) == UNIV_PAGE_SIZE_MAX,
                "(1 << UNIV_PAGE_SIZE_SHIFT_MAX) != UNIV_PAGE_SIZE_MAX");

  static_assert((1 << UNIV_PAGE_SIZE_SHIFT_MIN) == UNIV_PAGE_SIZE_MIN,
                "(1 << UNIV_PAGE_SIZE_SHIFT_MIN) != UNIV_PAGE_SIZE_MIN");

  ut_a(fil_system == nullptr);

  ut_a(max_n_open > 0);

  fil_system = UT_NEW_NOKEY(Fil_system(MAX_SHARDS, max_n_open));
}
