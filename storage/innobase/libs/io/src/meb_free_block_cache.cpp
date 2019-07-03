#include <innodb/io/meb_free_block_cache.h>

#ifdef UNIV_HOTBACKUP

#include <innodb/allocator/ut_free.h>
#include <innodb/io/UT_DELETE.h>

/** De-allocates block cache at InnoDB shutdown. */
void meb_free_block_cache() {
  if (block_cache == NULL) {
    return;
  }

  for (Blocks::iterator it = block_cache->begin(); it != block_cache->end();
       ++it) {
    ut_a(it->m_in_use == 0);
    ut_free(it->m_ptr);
  }

  UT_DELETE(block_cache);

  block_cache = NULL;
}
#endif /* UNIV_HOTBACKUP */
