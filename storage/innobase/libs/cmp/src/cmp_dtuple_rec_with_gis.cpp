#include <innodb/cmp/cmp_dtuple_rec_with_gis.h>

#include <innodb/cmp/cmp_gis_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/data_types/dfield_get_data.h>


/** Compare a GIS data tuple to a physical record.
@param[in] dtuple data tuple
@param[in] rec B-tree record
@param[in] offsets rec_get_offsets(rec)
@param[in] mode compare mode
@param[in] srs Spatial reference system of R-tree
@retval negative if dtuple is less than rec */
int cmp_dtuple_rec_with_gis(
    const dtuple_t *dtuple, /*!< in: data tuple */
    const rec_t *rec,       /*!< in: physical record which differs from
                            dtuple in some of the common fields, or which
                            has an equal number or more fields than
                            dtuple */
    const ulint *offsets,   /*!< in: array returned by rec_get_offsets() */
    page_cur_mode_t mode,   /*!< in: compare mode */
    const dd::Spatial_reference_system *srs) /*!< in: SRS of R-tree */
{
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
      mode, static_cast<const byte *>(dfield_get_data(dtuple_field)),
      (unsigned)dtuple_f_len, rec_b_ptr, (unsigned)rec_f_len, srs);

  return (ret);
}

