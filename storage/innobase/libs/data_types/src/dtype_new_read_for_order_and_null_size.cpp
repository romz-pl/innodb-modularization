#include <innodb/data_types/dtype_new_read_for_order_and_null_size.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/assert/assert.h>
#include <innodb/data_types/data_mysql_default_charset_coll.h>
#include <innodb/data_types/dtype_form_prtype.h>
#include <innodb/data_types/dtype_is_string_type.h>
#include <innodb/data_types/dtype_set_mblen.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/machine/data.h>

/** Reads to a type the stored information which determines its alphabetical
 ordering and the storage size of an SQL NULL value. This is the >= 4.1.x
 storage format. */
void dtype_new_read_for_order_and_null_size(
    dtype_t *type,   /*!< in: type struct */
    const byte *buf) /*!< in: buffer for stored type order info */
{
  ulint charset_coll;

#if 6 != DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE
#error "6 != DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE"
#endif

  type->mtype = buf[0] & 63;
  type->prtype = buf[1];

  if (buf[0] & 128) {
    type->prtype |= DATA_BINARY_TYPE;
  }

  if (buf[4] & 128) {
    type->prtype |= DATA_NOT_NULL;
  }

  type->len = mach_read_from_2(buf + 2);

  charset_coll = mach_read_from_2(buf + 4) & CHAR_COLL_MASK;

  if (dtype_is_string_type(type->mtype)) {
    ut_a(charset_coll <= MAX_CHAR_COLL_NUM);

    if (charset_coll == 0) {
      /* This insert buffer record was inserted with MySQL
      version < 4.1.2, and the charset-collation code was not
      explicitly stored to dtype->prtype at that time. It
      must be the default charset-collation of this MySQL
      installation. */

      charset_coll = data_mysql_default_charset_coll;
    }

    type->prtype = dtype_form_prtype(type->prtype, charset_coll);
  }
  dtype_set_mblen(type);
}



#endif
