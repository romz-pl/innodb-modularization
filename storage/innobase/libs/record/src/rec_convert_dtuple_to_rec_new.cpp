#include <innodb/record/rec_convert_dtuple_to_rec_new.h>

#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_convert_dtuple_to_rec_comp.h>
#include <innodb/record/rec_get_converted_size_comp.h>
#include <innodb/record/rec_set_info_and_status_bits.h>
#include <innodb/record/rec_set_instant_flag_new.h>

/** Builds a new-style physical record out of a data tuple and
 stores it beginning from the start of the given buffer.
 @return pointer to the origin of physical record */
rec_t *rec_convert_dtuple_to_rec_new(
    byte *buf,                 /*!< in: start address of
                               the physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple)    /*!< in: data tuple */
{
  ulint extra_size;
  ulint status;
  rec_t *rec;
  bool instant;

  status = dtuple_get_info_bits(dtuple) & REC_NEW_STATUS_MASK;
  rec_get_converted_size_comp(index, status, dtuple->fields, dtuple->n_fields,
                              &extra_size);
  rec = buf + extra_size;

  instant = rec_convert_dtuple_to_rec_comp(
      rec, index, dtuple->fields, dtuple->n_fields, NULL, status, false);

  /* Set the info bits of the record */
  rec_set_info_and_status_bits(rec, dtuple_get_info_bits(dtuple));

  if (instant) {
    ut_ad(index->has_instant_cols());
    rec_set_instant_flag_new(rec, true);
  } else {
    rec_set_instant_flag_new(rec, false);
  }

  return (rec);
}
