#include <innodb/data_types/dfield_dup.h>

#include <innodb/data_types/dfield_is_null.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/memory/mem_heap_dup.h>
#include <innodb/memory_check/memory_check.h>

/** Copies the data pointed to by a data field. */
void dfield_dup(dfield_t *field,  /*!< in/out: data field */
                mem_heap_t *heap) /*!< in: memory heap where allocated */
{
  if (!dfield_is_null(field) && field->data != NULL) {
    UNIV_MEM_ASSERT_RW(field->data, field->len);
    field->data = mem_heap_dup(heap, field->data, field->len);
  }
}
