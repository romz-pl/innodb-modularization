#include <innodb/lock_priv/lock_rec_copy.h>

#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/lock_rec_get_n_bits.h>
#include <innodb/memory/mem_heap_dup.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/lock_priv/lock_t.h>

/** Copies a record lock to heap.
 @return copy of lock */
lock_t *lock_rec_copy(const lock_t *lock, /*!< in: record lock */
                             mem_heap_t *heap)   /*!< in: memory heap */
{
  ulint size;

  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  size = sizeof(lock_t) + lock_rec_get_n_bits(lock) / 8;

  return (static_cast<lock_t *>(mem_heap_dup(heap, lock, size)));
}
