#include <innodb/dict_mem/dict_index_get_nth_field_pos.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>


/** Looks for a matching field in an index. The column has to be the same. The
 column in index must be complete, or must contain a prefix longer than the
 column in index2. That is, we must be able to construct the prefix in index2
 from the prefix in index.
 @return position in internal representation of the index;
 ULINT_UNDEFINED if not contained */
ulint dict_index_get_nth_field_pos(
    const dict_index_t *index,  /*!< in: index from which to search */
    const dict_index_t *index2, /*!< in: index */
    ulint n)                    /*!< in: field number in index2 */
{
  const dict_field_t *field;
  const dict_field_t *field2;
  ulint n_fields;
  ulint pos;

  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  field2 = index2->get_field(n);

  n_fields = dict_index_get_n_fields(index);

  /* Are we looking for a MBR (Minimum Bound Box) field of
  a spatial index */
  bool is_mbr_fld = (n == 0 && dict_index_is_spatial(index2));

  for (pos = 0; pos < n_fields; pos++) {
    field = index->get_field(pos);

    /* The first field of a spatial index is a transformed
    MBR (Minimum Bound Box) field made out of original column,
    so its field->col still points to original cluster index
    col, but the actual content is different. So we cannot
    consider them equal if neither of them is MBR field */
    if (pos == 0 && dict_index_is_spatial(index) && !is_mbr_fld) {
      continue;
    }

    if (field->col == field2->col &&
        (field->prefix_len == 0 || (field->prefix_len >= field2->prefix_len &&
                                    field2->prefix_len != 0))) {
      return (pos);
    }
  }

  return (ULINT_UNDEFINED);
}


