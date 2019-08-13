#include <innodb/data_types/dfield_check_typed_no_assert.h>

#include <innodb/data_types/flags.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/logger/error.h>

#ifndef UNIV_HOTBACKUP

/** Checks that a data field is typed.
 @return true if ok */
ibool dfield_check_typed_no_assert(const dfield_t *field) /*!< in: data field */
{
  if (dfield_get_type(field)->mtype > DATA_MTYPE_CURRENT_MAX ||
      dfield_get_type(field)->mtype < DATA_MTYPE_CURRENT_MIN) {
    ib::error(ER_IB_MSG_156)
        << "Data field type " << dfield_get_type(field)->mtype << ", len "
        << dfield_get_len(field);

    return (FALSE);
  }

  return (TRUE);
}

#endif
