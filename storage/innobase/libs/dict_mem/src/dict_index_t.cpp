#include <innodb/dict_mem/dict_index_t.h>

#include <innodb/dict_mem/dict_table_t.h>


bool dict_index_t::is_compressed() const {
  return (table->is_compressed());
}

/** Returns the number of fields before first instant ADD COLUMN */
uint32_t dict_index_t::get_instant_fields() const {
  ut_ad(has_instant_cols());
  return (n_fields - (table->n_cols - table->n_instant_cols));
}
