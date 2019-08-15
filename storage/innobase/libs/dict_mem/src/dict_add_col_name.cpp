#include <innodb/dict_mem/dict_add_col_name.h>

#include <innodb/memory/mem_heap_alloc.h>

/** Append 'name' to 'col_names'.  @see dict_table_t::col_names
 @return new column names array */
const char *dict_add_col_name(
    const char *col_names, /*!< in: existing column names, or
                           NULL */
    ulint cols,            /*!< in: number of existing columns */
    const char *name,      /*!< in: new column name */
    mem_heap_t *heap)      /*!< in: heap */
{
  ulint old_len;
  ulint new_len;
  ulint total_len;
  char *res;

  ut_ad(!cols == !col_names);

  /* Find out length of existing array. */
  if (col_names) {
    const char *s = col_names;
    ulint i;

    for (i = 0; i < cols; i++) {
      s += strlen(s) + 1;
    }

    old_len = s - col_names;
  } else {
    old_len = 0;
  }

  new_len = strlen(name) + 1;
  total_len = old_len + new_len;

  res = static_cast<char *>(mem_heap_alloc(heap, total_len));

  if (old_len > 0) {
    memcpy(res, col_names, old_len);
  }

  memcpy(res + old_len, name, new_len);

  return (res);
}
