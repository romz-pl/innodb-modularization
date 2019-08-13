#include <innodb/data_types/dtuple_big_rec_free.h>

#include <innodb/data_types/big_rec_t.h>
#include <innodb/memory/mem_heap_free.h>

/** Frees the memory in a big rec vector. */
void dtuple_big_rec_free(big_rec_t *vector) /*!< in, own: big rec vector; it is
                                            freed in this function */
{
  mem_heap_free(vector->heap);
}
