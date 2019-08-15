#include <innodb/dict_mem/dict_col_t.h>

#include <innodb/memory/mem_heap_strdupl.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Set default value
@param[in]	value	Default value
@param[in]	length	Default value length
@param[in,out]	heap	Heap to allocate memory */
void dict_col_t::set_default(const byte *value, size_t length,
                             mem_heap_t *heap) {
  ut_ad(instant_default == nullptr);
  ut_ad(length == 0 || length == UNIV_SQL_NULL || value != nullptr);

  instant_default = static_cast<dict_col_default_t *>(
      mem_heap_alloc(heap, sizeof(dict_col_default_t)));

  instant_default->col = this;

  if (length != UNIV_SQL_NULL) {
    const char *val =
        (value == nullptr ? "\0" : reinterpret_cast<const char *>(value));

    instant_default->value =
        reinterpret_cast<byte *>(mem_heap_strdupl(heap, val, length));
  } else {
    ut_ad(!(prtype & DATA_NOT_NULL));
    instant_default->value = nullptr;
  }

  instant_default->len = length;
}
