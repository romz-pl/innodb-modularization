#include <innodb/tablespace/fil_space_exists_in_mem.h>

#include <innodb/tablespace/fil_system.h>

/** Returns true if a matching tablespace exists in the InnoDB tablespace
memory cache.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name used in space_create().
@param[in]	print_err	Print detailed error information to the
                                error log if a matching tablespace is
                                not found from memory.
@param[in]	adjust_space	Whether to adjust space id on mismatch
@param[in]	heap		Heap memory
@param[in]	table_id	table ID
@return true if a matching tablespace exists in the memory cache */
bool fil_space_exists_in_mem(space_id_t space_id, const char *name,
                             bool print_err, bool adjust_space,
                             mem_heap_t *heap, table_id_t table_id) {
  auto shard = fil_system->shard_by_id(space_id);

  return (shard->space_check_exists(space_id, name, print_err, adjust_space,
                                    heap, table_id));
}
