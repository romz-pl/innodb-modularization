#include <innodb/dict_mem/dict_index_t.h>

#include <innodb/dict_mem/dict_col_get_clust_pos.h>
#include <innodb/dict_mem/dict_col_get_no.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_get_sys_col_no.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>

bool dict_index_t::is_compressed() const {
  return (table->is_compressed());
}

/** Returns the number of fields before first instant ADD COLUMN */
uint32_t dict_index_t::get_instant_fields() const {
  ut_ad(has_instant_cols());
  return (n_fields - (table->n_cols - table->n_instant_cols));
}


/** Gets the column number the nth field in an index.
@param[in] pos	position of the field
@return column number */
ulint dict_index_t::get_col_no(ulint pos) const {
  return (dict_col_get_no(get_col(pos)));
}

/** Returns the position of a system column in an index.
@param[in] type		DATA_ROW_ID, ...
@return position, ULINT_UNDEFINED if not contained */
ulint dict_index_t::get_sys_col_pos(ulint type) const {
  ut_ad(magic_n == DICT_INDEX_MAGIC_N);
  ut_ad(!dict_index_is_ibuf(this));

  if (is_clustered()) {
    return (dict_col_get_clust_pos(table->get_sys_col(type), this));
  }

  return (get_col_pos(dict_table_get_sys_col_no(table, type)));
}

/** Looks for column n in an index.
@param[in]	n		column number
@param[in]	inc_prefix	true=consider column prefixes too
@param[in]	is_virtual	true==virtual column
@return position in internal representation of the index;
ULINT_UNDEFINED if not contained */
ulint dict_index_t::get_col_pos(ulint n, bool inc_prefix,
                                bool is_virtual) const {
  const dict_field_t *field;
  const dict_col_t *col;
  ulint pos;
  ulint n_fields;

  ut_ad(magic_n == DICT_INDEX_MAGIC_N);

  if (is_virtual) {
    col = &(dict_table_get_nth_v_col(table, n)->m_col);
  } else {
    col = table->get_col(n);
  }

  if (is_clustered()) {
    return (dict_col_get_clust_pos(col, this));
  }

  n_fields = dict_index_get_n_fields(this);

  for (pos = 0; pos < n_fields; pos++) {
    field = get_field(pos);

    if (col == field->col && (inc_prefix || field->prefix_len == 0)) {
      return (pos);
    }
  }

  return (ULINT_UNDEFINED);
}

