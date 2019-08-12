#include <innodb/log_recv/recv_get_page_map.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_heap_create_typed.h>

/** Get the page map for a tablespace. It will create one if one isn't found.
@param[in]	space_id	Tablespace ID for which page map required.
@param[in]	create		false if lookup only
@return the space data or null if not found */
recv_sys_t::Space *recv_get_page_map(space_id_t space_id, bool create) {
  auto it = recv_sys->spaces->find(space_id);

  if (it != recv_sys->spaces->end()) {
    return (&it->second);

  } else if (create) {
    mem_heap_t *heap;

    heap = mem_heap_create_typed(256, MEM_HEAP_FOR_RECV_SYS);

    using Space = recv_sys_t::Space;
    using value_type = recv_sys_t::Spaces::value_type;

    auto where =
        recv_sys->spaces->insert(it, value_type(space_id, Space(heap)));

    return (&where->second);
  }

  return (nullptr);
}
