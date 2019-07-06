#include <innodb/string/ut_raw_to_hex.h>

#include <innodb/align/ut_align_offset.h>

/** Converts a raw binary data to a NUL-terminated hex string. The output is
 truncated if there is not enough space in "hex", make sure "hex_size" is at
 least (2 * raw_size + 1) if you do not want this to happen. Returns the
 actual number of characters written to "hex" (including the NUL).
 @return number of chars written */
ulint ut_raw_to_hex(const void *raw, /*!< in: raw data */
                    ulint raw_size,  /*!< in: "raw" length in bytes */
                    char *hex,       /*!< out: hex string */
                    ulint hex_size)  /*!< in: "hex" size in bytes */
{
#ifdef WORDS_BIGENDIAN

#define MK_UINT16(a, b) (((uint16)(a)) << 8 | (uint16)(b))

#define UINT16_GET_A(u) ((unsigned char)((u) >> 8))
#define UINT16_GET_B(u) ((unsigned char)((u)&0xFF))

#else /* WORDS_BIGENDIAN */

#define MK_UINT16(a, b) (((uint16)(b)) << 8 | (uint16)(a))

#define UINT16_GET_A(u) ((unsigned char)((u)&0xFF))
#define UINT16_GET_B(u) ((unsigned char)((u) >> 8))

#endif /* WORDS_BIGENDIAN */

#define MK_ALL_UINT16_WITH_A(a)                                               \
  MK_UINT16(a, '0'), MK_UINT16(a, '1'), MK_UINT16(a, '2'), MK_UINT16(a, '3'), \
      MK_UINT16(a, '4'), MK_UINT16(a, '5'), MK_UINT16(a, '6'),                \
      MK_UINT16(a, '7'), MK_UINT16(a, '8'), MK_UINT16(a, '9'),                \
      MK_UINT16(a, 'A'), MK_UINT16(a, 'B'), MK_UINT16(a, 'C'),                \
      MK_UINT16(a, 'D'), MK_UINT16(a, 'E'), MK_UINT16(a, 'F')

  static const uint16 hex_map[256] = {
      MK_ALL_UINT16_WITH_A('0'), MK_ALL_UINT16_WITH_A('1'),
      MK_ALL_UINT16_WITH_A('2'), MK_ALL_UINT16_WITH_A('3'),
      MK_ALL_UINT16_WITH_A('4'), MK_ALL_UINT16_WITH_A('5'),
      MK_ALL_UINT16_WITH_A('6'), MK_ALL_UINT16_WITH_A('7'),
      MK_ALL_UINT16_WITH_A('8'), MK_ALL_UINT16_WITH_A('9'),
      MK_ALL_UINT16_WITH_A('A'), MK_ALL_UINT16_WITH_A('B'),
      MK_ALL_UINT16_WITH_A('C'), MK_ALL_UINT16_WITH_A('D'),
      MK_ALL_UINT16_WITH_A('E'), MK_ALL_UINT16_WITH_A('F')};
  const unsigned char *rawc;
  ulint read_bytes;
  ulint write_bytes;
  ulint i;

  rawc = (const unsigned char *)raw;

  if (hex_size == 0) {
    return (0);
  }

  if (hex_size <= 2 * raw_size) {
    read_bytes = hex_size / 2;
    write_bytes = hex_size;
  } else {
    read_bytes = raw_size;
    write_bytes = 2 * raw_size + 1;
  }

#define LOOP_READ_BYTES(ASSIGN)      \
  for (i = 0; i < read_bytes; i++) { \
    ASSIGN;                          \
    hex += 2;                        \
    rawc++;                          \
  }

  if (ut_align_offset(hex, 2) == 0) {
    LOOP_READ_BYTES(*(uint16 *)hex = hex_map[*rawc]);
  } else {
    LOOP_READ_BYTES(*hex = UINT16_GET_A(hex_map[*rawc]);
                    *(hex + 1) = UINT16_GET_B(hex_map[*rawc]));
  }

  if (hex_size <= 2 * raw_size && hex_size % 2 == 0) {
    hex--;
  }

  *hex = '\0';

  return (write_bytes);
}

