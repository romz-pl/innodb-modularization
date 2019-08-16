#include <innodb/cmp/cmp_get_pad_char.h>

#include <innodb/data_types/flags.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/dtype_t.h>


/** Get the pad character code point for a type.
@param[in]	type
@return		pad character code point
@retval		ULINT_UNDEFINED if no padding is specified */
ulint cmp_get_pad_char(const dtype_t *type) {
  switch (type->mtype) {
    case DATA_FIXBINARY:
    case DATA_BINARY:
      if (dtype_get_charset_coll(type->prtype) ==
          DATA_MYSQL_BINARY_CHARSET_COLL) {
        /* Starting from 5.0.18, do not pad
        VARBINARY or BINARY columns. */
        return (ULINT_UNDEFINED);
      }
      /* Fall through */
    case DATA_CHAR:
    case DATA_VARCHAR:
    case DATA_MYSQL:
    case DATA_VARMYSQL:
      /* Space is the padding character for all char and binary
      strings, and starting from 5.0.3, also for TEXT strings. */
      return (0x20);
    case DATA_GEOMETRY:
      /* DATA_GEOMETRY is binary data, not ASCII-based. */
      return (ULINT_UNDEFINED);
    case DATA_BLOB:
      if (!(type->prtype & DATA_BINARY_TYPE)) {
        return (0x20);
      }
      /* Fall through */
    default:
      /* No padding specified */
      return (ULINT_UNDEFINED);
  }
}
