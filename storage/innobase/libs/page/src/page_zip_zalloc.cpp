#include <innodb/page/page_zip_zalloc.h>

#include <innodb/memory/mem_heap_zalloc.h>


extern "C" {

/** Allocate memory for zlib. */
void *page_zip_zalloc(void *opaque, /*!< in/out: memory heap */
                             uInt items, /*!< in: number of items to allocate */
                             uInt size)  /*!< in: size of an item in bytes */
{
  return (mem_heap_zalloc(static_cast<mem_heap_t *>(opaque), items * size));
}

}
