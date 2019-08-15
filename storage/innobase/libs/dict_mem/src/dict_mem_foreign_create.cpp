#include <innodb/dict_mem/dict_mem_foreign_create.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/memory/mem_heap_create.h>
#include <innodb/memory/mem_heap_zalloc.h>

/** Creates and initializes a foreign constraint memory object.
 @return own: foreign constraint struct */
dict_foreign_t *dict_mem_foreign_create(void) {
  dict_foreign_t *foreign;
  mem_heap_t *heap;
  DBUG_ENTER("dict_mem_foreign_create");

  heap = mem_heap_create(100);

  foreign = static_cast<dict_foreign_t *>(
      mem_heap_zalloc(heap, sizeof(dict_foreign_t)));

  foreign->heap = heap;

  foreign->v_cols = NULL;

  DBUG_PRINT("dict_mem_foreign_create", ("heap: %p", heap));

  DBUG_RETURN(foreign);
}
