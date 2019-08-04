#include <innodb/data_types/dtype_new_store_for_order_and_null_size.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/assert/assert.h>
#include <innodb/data_types/data_mysql_default_charset_coll.h>
#include <innodb/data_types/dtype_form_prtype.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/dtype_set_mblen.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/machine/data.h>

/** Reads to a type the stored information which determines its alphabetical
 ordering and the storage size of an SQL NULL value. This is the < 4.1.x
 storage format. */
void dtype_read_for_order_and_null_size(
    dtype_t *type,   /*!< in: type struct */
    const byte *buf) /*!< in: buffer for stored type order info */
{
#if 4 != DATA_ORDER_NULL_TYPE_BUF_SIZE
#error "4 != DATA_ORDER_NULL_TYPE_BUF_SIZE"
#endif

  type->mtype = buf[0] & 63;
  type->prtype = buf[1];

  if (buf[0] & 128) {
    type->prtype |= DATA_BINARY_TYPE;
  }

  type->len = mach_read_from_2(buf + 2);

  type->prtype =
      dtype_form_prtype(type->prtype, data_mysql_default_charset_coll);
  dtype_set_mblen(type);
}



/** Stores for a type the information which determines its alphabetical ordering
 and the storage size of an SQL NULL value. This is the >= 4.1.x storage
 format. */
void dtype_new_store_for_order_and_null_size(
    byte *buf,           /*!< in: buffer for
                         DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE
                         bytes where we store the info */
    const dtype_t *type, /*!< in: type struct */
    ulint prefix_len)    /*!< in: prefix length to
                      replace type->len, or 0 */
{
#if 6 != DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE
#error "6 != DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE"
#endif
  ulint len;

  ut_ad(type);
  ut_ad(type->mtype >= DATA_VARCHAR);
  ut_ad(type->mtype <= DATA_MTYPE_MAX);

  buf[0] = (byte)(type->mtype & 0xFFUL);

  if (type->prtype & DATA_BINARY_TYPE) {
    buf[0] |= 128;
  }

  /* In versions < 4.1.2 we had:	if (type->prtype & DATA_NONLATIN1) {
  buf[0] |= 64;
  }
  */

  buf[1] = (byte)(type->prtype & 0xFFUL);

  len = prefix_len ? prefix_len : type->len;

  mach_write_to_2(buf + 2, len & 0xFFFFUL);

  ut_ad(dtype_get_charset_coll(type->prtype) <= MAX_CHAR_COLL_NUM);
  mach_write_to_2(buf + 4, dtype_get_charset_coll(type->prtype));

  if (type->prtype & DATA_NOT_NULL) {
    buf[4] |= 128;
  }
}



#endif
