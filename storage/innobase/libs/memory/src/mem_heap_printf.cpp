#include <innodb/memory/mem_heap_printf.h>

#include <innodb/memory/mem_heap_printf_low.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/memory/mem_heap_printf_low.h>

#include <cstdarg>

/** A simple sprintf replacement that dynamically allocates the space for the
 formatted string from the given heap. This supports a very limited set of
 the printf syntax: types 's' and 'u' and length modifier 'l' (which is
 required for the 'u' type).
 @return heap-allocated formatted string */
char *mem_heap_printf(mem_heap_t *heap,   /*!< in: memory heap */
                      const char *format, /*!< in: format string */
                      ...) {
  va_list ap;
  char *str;
  ulint len;

  /* Calculate length of string */
  len = 0;
  va_start(ap, format);
  len = mem_heap_printf_low(NULL, format, ap);
  va_end(ap);

  /* Now create it for real. */
  str = static_cast<char *>(mem_heap_alloc(heap, len));
  va_start(ap, format);
  mem_heap_printf_low(str, format, ap);
  va_end(ap);

  return (str);
}
