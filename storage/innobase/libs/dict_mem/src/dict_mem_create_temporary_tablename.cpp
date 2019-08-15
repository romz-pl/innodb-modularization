#include <innodb/dict_mem/dict_mem_create_temporary_tablename.h>

#include <innodb/atomic/atomic.h>
#include <innodb/formatting/formatting.h>
#include <innodb/dict_mem/dict_temp_file_num.h>
#include <innodb/dict_types/flags.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Create a temporary tablename like "#sql-ibtid-inc" where
  tid = the Table ID
  inc = a randomly initialized number that is incremented for each file
The table ID is a 64 bit integer, can use up to 20 digits, and is initialized
at bootstrap. The second number is 32 bits, can use up to 10 digits, and is
initialized at startup to a randomly distributed number. It is hoped that the
combination of these two numbers will provide a reasonably unique temporary
file name.
@param[in]	heap	A memory heap
@param[in]	dbtab	Table name in the form database/table name
@param[in]	id	Table id
@return A unique temporary tablename suitable for InnoDB use */
char *dict_mem_create_temporary_tablename(mem_heap_t *heap, const char *dbtab,
                                          table_id_t id) {
  size_t size;
  char *name;
  const char *dbend = strchr(dbtab, '/');
  ut_ad(dbend);
  size_t dblen = dbend - dbtab + 1;

  /* Increment a randomly initialized  number for each temp file. */
  os_atomic_increment_uint32(&dict_temp_file_num, 1);

  size = dblen + (sizeof(TEMP_FILE_PREFIX) + 3 + 20 + 1 + 10);
  name = static_cast<char *>(mem_heap_alloc(heap, size));
  memcpy(name, dbtab, dblen);
  snprintf(name + dblen, size - dblen,
           TEMP_FILE_PREFIX_INNODB UINT64PF "-" UINT32PF, id,
           dict_temp_file_num);

  return (name);
}
