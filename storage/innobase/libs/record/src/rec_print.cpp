#include <innodb/record/rec_print.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/assert/assert.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/print/ut_print_buf_hex.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_default.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_offs_validate.h>

/** Pretty-print a record.
@param[in,out]	o	output stream
@param[in]	rec	physical record
@param[in]	info	rec_get_info_bits(rec)
@param[in]	offsets	rec_get_offsets(rec) */
void rec_print(std::ostream &o, const rec_t *rec, ulint info,
               const ulint *offsets) {
  const ulint comp = rec_offs_comp(offsets);
  const ulint n = rec_offs_n_fields(offsets);

  ut_ad(rec_offs_validate(rec, NULL, offsets));

  o << (comp ? "COMPACT RECORD" : "RECORD") << "(info_bits=" << info << ", "
    << n << " fields): {";

  for (ulint i = 0; i < n; i++) {
    const byte *data;
    ulint len;

    if (i) {
      o << ',';
    }

    if (rec_offs_nth_default(offsets, i)) {
      o << "DEFAULT";
      continue;
    }

    data = rec_get_nth_field(rec, offsets, i, &len);

    if (len == UNIV_SQL_NULL) {
      o << "NULL";
      continue;
    }

    if (rec_offs_nth_extern(offsets, i)) {
      ulint local_len = len - BTR_EXTERN_FIELD_REF_SIZE;
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);

      o << '[' << local_len << '+' << BTR_EXTERN_FIELD_REF_SIZE << ']';
      ut_print_buf(o, data, local_len);
      ut_print_buf_hex(o, data + local_len, BTR_EXTERN_FIELD_REF_SIZE);
    } else {
      o << '[' << len << ']';
      ut_print_buf(o, data, len);
    }
  }

  o << "}";
}


#endif
