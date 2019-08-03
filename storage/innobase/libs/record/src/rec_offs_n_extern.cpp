#include <innodb/record/rec_offs_n_extern.h>

#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>

/** Returns the number of extern bits set in a record.
 @return number of externally stored fields */
ulint rec_offs_n_extern(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint n = 0;

  if (rec_offs_any_extern(offsets)) {
    ulint i;

    for (i = rec_offs_n_fields(offsets); i--;) {
      if (rec_offs_nth_extern(offsets, i)) {
        n++;
      }
    }
  }

  return (n);
}
