#include <innodb/dict_mem/dict_col_name_is_reserved.h>

#include <innodb/string/innobase_strcasecmp.h>
#include <innodb/data_types/flags.h>

/** If the given column name is reserved for InnoDB system columns, return
 TRUE.
 @return true if name is reserved */
ibool dict_col_name_is_reserved(const char *name) /*!< in: column name */
{
/* This check reminds that if a new system column is added to
the program, it should be dealt with here. */
#if DATA_N_SYS_COLS != 3
#error "DATA_N_SYS_COLS != 3"
#endif

  static const char *reserved_names[] = {"DB_ROW_ID", "DB_TRX_ID",
                                         "DB_ROLL_PTR"};

  ulint i;

  for (i = 0; i < UT_ARR_SIZE(reserved_names); i++) {
    if (innobase_strcasecmp(name, reserved_names[i]) == 0) {
      return (TRUE);
    }
  }

  return (FALSE);
}
