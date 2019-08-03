#include <innodb/record/rec_get_converted_extra_size.h>

#include <innodb/record/flag.h>

/** Returns the extra size of an old-style physical record if we know its
 data size and number of fields.
 @return extra size */
ulint rec_get_converted_extra_size(
    ulint data_size, /*!< in: data size */
    ulint n_fields,  /*!< in: number of fields */
    ulint n_ext)     /*!< in: number of externally stored columns */
{
  if (!n_ext && data_size <= REC_1BYTE_OFFS_LIMIT) {
    return (REC_N_OLD_EXTRA_BYTES + n_fields);
  }

  return (REC_N_OLD_EXTRA_BYTES + 2 * n_fields);
}
