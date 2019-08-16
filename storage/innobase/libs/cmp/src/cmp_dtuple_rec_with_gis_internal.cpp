#include <innodb/cmp/cmp_dtuple_rec_with_gis_internal.h>

#include <innodb/cmp/cmp_data.h>
#include <innodb/cmp/cmp_gis_field.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/record/rec_get_nth_field.h>



/** Compare a GIS data tuple to a physical record in rtree non-leaf node.
We need to check the page number field, since we don't store pk field in
rtree non-leaf node.
@param[in]	dtuple		data tuple
@param[in]	rec		R-tree record
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	srs	        Spatial reference system of R-tree
@retval negative if dtuple is less than rec */
int cmp_dtuple_rec_with_gis_internal(const dtuple_t *dtuple, const rec_t *rec,
                                     const ulint *offsets,
                                     const dd::Spatial_reference_system *srs) {
  const dfield_t *dtuple_field; /* current field in logical record */
  ulint dtuple_f_len;           /* the length of the current field
                                in the logical record */
  ulint rec_f_len;              /* length of current field in rec */
  const byte *rec_b_ptr;        /* pointer to the current byte in
                                rec field */
  int ret = 0;                  /* return value */

  dtuple_field = dtuple_get_nth_field(dtuple, 0);
  dtuple_f_len = dfield_get_len(dtuple_field);

  rec_b_ptr = rec_get_nth_field(rec, offsets, 0, &rec_f_len);
  ret = cmp_gis_field(
      PAGE_CUR_WITHIN, static_cast<const byte *>(dfield_get_data(dtuple_field)),
      (unsigned)dtuple_f_len, rec_b_ptr, (unsigned)rec_f_len, srs);
  if (ret != 0) {
    return (ret);
  }

  dtuple_field = dtuple_get_nth_field(dtuple, 1);
  dtuple_f_len = dfield_get_len(dtuple_field);
  rec_b_ptr = rec_get_nth_field(rec, offsets, 1, &rec_f_len);

  return (cmp_data(dtuple_field->type.mtype, dtuple_field->type.prtype, true,
                   static_cast<const byte *>(dtuple_field->data), dtuple_f_len,
                   rec_b_ptr, rec_f_len));
}
