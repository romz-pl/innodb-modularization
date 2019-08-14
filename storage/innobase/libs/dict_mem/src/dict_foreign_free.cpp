#include <innodb/dict_mem/dict_foreign_free.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/allocator/UT_DELETE.h>

/** Frees a foreign key struct. */
void dict_foreign_free(
    dict_foreign_t *foreign) /*!< in, own: foreign key struct */
{
  if (foreign->v_cols != NULL) {
    UT_DELETE(foreign->v_cols);
  }

  mem_heap_free(foreign->heap);
}
