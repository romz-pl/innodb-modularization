#include <innodb/page/page_zip_set_alloc.h>

#include <innodb/page/page_zip_zalloc.h>
#include <innodb/page/page_zip_free.h>

#include <zlib.h>

/** Configure the zlib allocator to use the given memory heap. */
void page_zip_set_alloc(void *stream,     /*!< in/out: zlib stream */
                        mem_heap_t *heap) /*!< in: memory heap to use */
{
  z_stream *strm = static_cast<z_stream *>(stream);

  strm->zalloc = page_zip_zalloc;
  strm->zfree = page_zip_free;
  strm->opaque = heap;
}
