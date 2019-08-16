#include <innodb/record/rec_init_offsets_temp.h>

#include <innodb/record/rec_init_offsets_comp_ordinary.h>

/** Determine the offset to each field in temporary file.
 @see rec_convert_dtuple_to_temp() */
void rec_init_offsets_temp(
    const rec_t *rec,          /*!< in: temporary file record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets)            /*!< in/out: array of offsets;
                               in: n=rec_offs_n_fields(offsets) */
{
  rec_init_offsets_comp_ordinary(rec, true, index, offsets);
}
