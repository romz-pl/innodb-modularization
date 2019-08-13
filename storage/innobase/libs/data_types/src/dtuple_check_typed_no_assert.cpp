#include <innodb/data_types/dtuple_check_typed_no_assert.h>

#include <innodb/data_types/dfield_check_typed_no_assert.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_print.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/logger/error.h>

#ifndef UNIV_HOTBACKUP


/** Checks that a data tuple is typed.
 @return true if ok */
ibool dtuple_check_typed_no_assert(const dtuple_t *tuple) /*!< in: tuple */
{
  const dfield_t *field;
  ulint i;

  if (dtuple_get_n_fields(tuple) > REC_MAX_N_FIELDS) {
    ib::error(ER_IB_MSG_157)
        << "Index entry has " << dtuple_get_n_fields(tuple) << " fields";
  dump:
    fputs("InnoDB: Tuple contents: ", stderr);
    dtuple_print(stderr, tuple);
    putc('\n', stderr);

    return (FALSE);
  }

  for (i = 0; i < dtuple_get_n_fields(tuple); i++) {
    field = dtuple_get_nth_field(tuple, i);

    if (!dfield_check_typed_no_assert(field)) {
      goto dump;
    }
  }

  return (TRUE);
}
#endif /* !UNIV_HOTBACKUP */
