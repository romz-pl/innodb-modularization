#include <innodb/record/rec_offs_size.h>

#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>

/** Returns the total size of a physical record.
 @return size */
ulint rec_offs_size(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  return (rec_offs_data_size(offsets) + rec_offs_extra_size(offsets));
}
