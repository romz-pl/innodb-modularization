#include <innodb/data_types/dtuple_check_typed.h>


#ifdef UNIV_DEBUG

/** Checks that a data tuple is typed. Asserts an error if not.
 @return true if ok */
ibool dtuple_check_typed(const dtuple_t *tuple) /*!< in: tuple */
{
  const dfield_t *field;
  ulint i;

  for (i = 0; i < dtuple_get_n_fields(tuple); i++) {
    field = dtuple_get_nth_field(tuple, i);

    ut_a(dfield_check_typed(field));
  }

  return (TRUE);
}


#endif
