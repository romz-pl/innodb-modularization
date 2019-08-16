#include <innodb/cmp/cmp_gis_field.h>

#include <innodb/cmp/cmp_geometry_field.h>
#include <innodb/data_types/flags.h>


int rtree_key_cmp(page_cur_mode_t mode, const uchar *a, int a_len,
                  const uchar *b, int b_len,
                  const dd::Spatial_reference_system *srs);

/** Innobase uses this function to compare two gis data fields
 @return	1, 0, -1, if mode == PAGE_CUR_MBR_EQUAL. And return
 1, 0 for rest compare modes, depends on a and b qualifies the
 relationship (CONTAINT, WITHIN etc.) */
int cmp_gis_field(
    page_cur_mode_t mode,                    /*!< in: compare mode */
    const byte *a,                           /*!< in: data field */
    unsigned int a_length,                   /*!< in: data field length,
                                             not UNIV_SQL_NULL */
    const byte *b,                           /*!< in: data field */
    unsigned int b_length,                   /*!< in: data field length,
                                             not UNIV_SQL_NULL */
    const dd::Spatial_reference_system *srs) /*!< in: SRS of R-tree */
{
  if (mode == PAGE_CUR_MBR_EQUAL) {
    /* TODO: Since the DATA_GEOMETRY is not used in compare
    function, we could pass it instead of a specific type now */
    return (cmp_geometry_field(DATA_GEOMETRY, DATA_GIS_MBR, a, a_length, b,
                               b_length));
  } else {
    return (rtree_key_cmp(mode, a, a_length, b, b_length, srs));
  }
}
