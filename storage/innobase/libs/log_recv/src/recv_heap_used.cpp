#include <innodb/log_recv/recv_heap_used.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/memory/mem_heap_get_size.h>

/** Get the number of bytes used by all the heaps
@return number of bytes used */
#ifndef UNIV_HOTBACKUP
static size_t recv_heap_used()
#else  /* !UNIV_HOTBACKUP */
size_t meb_heap_used()
#endif /* !UNIV_HOTBACKUP */
{
  size_t size = 0;

  for (auto &space : *recv_sys->spaces) {
    if (space.second.m_heap != nullptr) {
      size += mem_heap_get_size(space.second.m_heap);
    }
  }

  return (size);
}
