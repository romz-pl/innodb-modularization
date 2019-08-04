#include <innodb/hash/hash_create.h>

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/hash/hash_cell_t.h>
#include <innodb/hash/hash_table_clear.h>
#include <innodb/random/ut_find_prime.h>
#include <innodb/hash/hash_table_t.h>

/** Creates a hash table with >= n array cells. The actual number of cells is
 chosen to be a prime number slightly bigger than n.
 @return own: created table */
hash_table_t *hash_create(ulint n) /*!< in: number of array cells */
{
  hash_cell_t *array;
  ulint prime;
  hash_table_t *table;

  prime = ut_find_prime(n);

  table = static_cast<hash_table_t *>(ut_malloc_nokey(sizeof(hash_table_t)));

  array =
      static_cast<hash_cell_t *>(ut_malloc_nokey(sizeof(hash_cell_t) * prime));

  /* The default type of hash_table is HASH_TABLE_SYNC_NONE i.e.:
  the caller is responsible for access control to the table. */
  table->type = HASH_TABLE_SYNC_NONE;
  table->cells = array;
  table->n_cells = prime;
#ifndef UNIV_HOTBACKUP
#if defined UNIV_AHI_DEBUG || defined UNIV_DEBUG
  table->adaptive = FALSE;
#endif /* UNIV_AHI_DEBUG || UNIV_DEBUG */
  table->n_sync_obj = 0;
  table->sync_obj.mutexes = NULL;
  table->heaps = NULL;
#endif /* !UNIV_HOTBACKUP */
  table->heap = NULL;
  ut_d(table->magic_n = HASH_TABLE_MAGIC_N);

  /* Initialize the cell array */
  hash_table_clear(table);

  return (table);
}
