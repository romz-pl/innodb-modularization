#include <innodb/data_types/dfield_print.h>

#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/flags.h>
#include <innodb/disk/flags.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/print/ut_print_buf_hex.h>


#include <ostream>

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	field	array of data fields
@param[in]	n	number of data fields */
void dfield_print(std::ostream &o, const dfield_t *field, ulint n) {
  for (ulint i = 0; i < n; i++, field++) {
    const void *data = dfield_get_data(field);
    const ulint len = dfield_get_len(field);

    if (i) {
      o << ',';
    }

    if (dfield_is_null(field)) {
      o << "NULL";
    } else if (dfield_is_ext(field)) {
      ulint local_len = len - BTR_EXTERN_FIELD_REF_SIZE;
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);

      o << '[' << local_len << '+' << BTR_EXTERN_FIELD_REF_SIZE << ']';
      ut_print_buf(o, data, local_len);
      ut_print_buf_hex(o, static_cast<const byte *>(data) + local_len,
                       BTR_EXTERN_FIELD_REF_SIZE);
    } else {
      o << '[' << len << ']';
      ut_print_buf(o, data, len);
    }
  }
}

