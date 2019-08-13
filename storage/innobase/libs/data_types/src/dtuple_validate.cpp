#include <innodb/data_types/dtuple_validate.h>


#ifdef UNIV_DEBUG

#include <innodb/data_types/dtuple_t.h>

/** Validates the consistency of a tuple which must be complete, i.e,
 all fields must have been set.
 @return true if ok */
ibool dtuple_validate(const dtuple_t *tuple) /*!< in: tuple */
{
  const dfield_t *field;
  ulint n_fields;
  ulint len;
  ulint i;

  ut_ad(tuple->magic_n == DATA_TUPLE_MAGIC_N);

  n_fields = dtuple_get_n_fields(tuple);

  /* We dereference all the data of each field to test
  for memory traps */

  for (i = 0; i < n_fields; i++) {
    field = dtuple_get_nth_field(tuple, i);
    len = dfield_get_len(field);

    if (!dfield_is_null(field)) {
      const byte *data;

      data = static_cast<const byte *>(dfield_get_data(field));
#ifndef UNIV_DEBUG_VALGRIND
      ulint j;

      for (j = 0; j < len; j++) {
        data++;
      }
#endif /* !UNIV_DEBUG_VALGRIND */

      UNIV_MEM_ASSERT_RW(data, len);
    }
  }

  ut_a(dtuple_check_typed(tuple));

  return (TRUE);
}

#endif
