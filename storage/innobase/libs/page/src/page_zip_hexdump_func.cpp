#include <innodb/page/page_zip_hexdump_func.h>


#ifdef UNIV_ZIP_DEBUG

/** Dump a block of memory on the standard error stream. */
void page_zip_hexdump_func(
    const char *name, /*!< in: name of the data structure */
    const void *buf,  /*!< in: data */
    ulint size)       /*!< in: length of the data, in bytes */
{
  const byte *s = static_cast<const byte *>(buf);
  ulint addr;
  const ulint width = 32; /* bytes per line */

  fprintf(stderr, "%s:\n", name);

  for (addr = 0; addr < size; addr += width) {
    ulint i;

    fprintf(stderr, "%04lx ", (ulong)addr);

    i = std::min(width, size - addr);

    while (i--) {
      fprintf(stderr, "%02x", *s++);
    }

    putc('\n', stderr);
  }
}


#endif
