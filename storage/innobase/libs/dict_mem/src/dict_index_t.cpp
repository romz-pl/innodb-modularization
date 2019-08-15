#include <innodb/dict_mem/dict_index_t.h>

#include <innodb/dict_mem/dict_table_t.h>


bool dict_index_t::is_compressed() const {
  return (table->is_compressed());
}
