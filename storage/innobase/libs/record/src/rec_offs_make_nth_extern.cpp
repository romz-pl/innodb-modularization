#include <innodb/record/rec_offs_make_nth_extern.h>

#include <innodb/record/rec_offs_nth_sql_null.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Mark the nth field as externally stored.
@param[in]	offsets		array returned by rec_get_offsets()
@param[in]	n		nth field */
void rec_offs_make_nth_extern(ulint *offsets, const ulint n) {
  ut_ad(!rec_offs_nth_sql_null(offsets, n));
  rec_offs_base(offsets)[1 + n] |= REC_OFFS_EXTERNAL;
}
