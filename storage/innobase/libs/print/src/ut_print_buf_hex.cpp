#include <innodb/print/ut_print_buf_hex.h>

#include <innodb/memory_check/memory_check.h>

#include <ostream>

/** Prints the contents of a memory buffer in hex. */
void ut_print_buf_hex(std::ostream &o, /*!< in/out: output stream */
                      const void *buf, /*!< in: memory buffer */
                      ulint len)       /*!< in: length of the buffer */
{
  const byte *data;
  ulint i;

  static const char hexdigit[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  UNIV_MEM_ASSERT_RW(buf, len);

  o << "(0x";

  for (data = static_cast<const byte *>(buf), i = 0; i < len; i++) {
    byte b = *data++;
    o << hexdigit[(int)b >> 16] << hexdigit[b & 15];
  }

  o << ")";
}
