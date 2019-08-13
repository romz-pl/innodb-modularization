#include <innodb/data_types/dfield_check_typed.h>


#ifdef UNIV_DEBUG

#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dfield_get_type.h>

/** Checks that a data field is typed. Asserts an error if not.
 @return true if ok */
ibool dfield_check_typed(const dfield_t *field) /*!< in: data field */
{
  if (dfield_get_type(field)->mtype > DATA_MTYPE_CURRENT_MAX ||
      dfield_get_type(field)->mtype < DATA_MTYPE_CURRENT_MIN) {
    ib::fatal(ER_IB_MSG_158)
        << "Data field type " << dfield_get_type(field)->mtype << ", len "
        << dfield_get_len(field);
  }

  return (TRUE);
}


#endif
