#include <innodb/print/ut_print_buf.h>

#include <innodb/memory_check/memory_check.h>
#include <innodb/formatting/formatting.h>
#include <innodb/print/ut_print_buf_hex.h>

#include <ostream>

/** Prints the contents of a memory buffer in hex and ascii. */
void ut_print_buf(FILE *file,      /*!< in: file where to print */
                  const void *buf, /*!< in: memory buffer */
                  ulint len)       /*!< in: length of the buffer */
{
  const byte *data;
  ulint i;

  UNIV_MEM_ASSERT_RW(buf, len);

  fprintf(file, " len " ULINTPF "; hex ", len);

  for (data = (const byte *)buf, i = 0; i < len; i++) {
    fprintf(file, "%02lx", (ulong)*data++);
  }

  fputs("; asc ", file);

  data = (const byte *)buf;

  for (i = 0; i < len; i++) {
    int c = (int)*data++;
    putc(isprint(c) ? c : ' ', file);
  }

  putc(';', file);
}


/** Prints the contents of a memory buffer in hex and ascii. */
void ut_print_buf(std::ostream &o, /*!< in/out: output stream */
                  const void *buf, /*!< in: memory buffer */
                  ulint len)       /*!< in: length of the buffer */
{
  const byte *data;
  ulint i;

  UNIV_MEM_ASSERT_RW(buf, len);

  for (data = static_cast<const byte *>(buf), i = 0; i < len; i++) {
    int c = static_cast<int>(*data++);
    o << (isprint(c) ? static_cast<char>(c) : ' ');
  }

  ut_print_buf_hex(o, buf, len);
}


