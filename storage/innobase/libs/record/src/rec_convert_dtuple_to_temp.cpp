#include <innodb/record/rec_convert_dtuple_to_temp.h>

#include <innodb/record/rec_convert_dtuple_to_rec_comp.h>
#include <innodb/record/flag.h>

/** Builds a temporary file record out of a data tuple.
 @see rec_init_offsets_temp() */
void rec_convert_dtuple_to_temp(
    rec_t *rec,                /*!< out: record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of fields */
    const dtuple_t *v_entry)   /*!< in: dtuple contains
                               virtual column data */
{
  rec_convert_dtuple_to_rec_comp(rec, index, fields, n_fields, v_entry,
                                 REC_STATUS_ORDINARY, true);
}
