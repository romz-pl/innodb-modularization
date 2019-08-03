#include <innodb/page/page_zip_fixed_field_encode.h>

#include <innodb/assert/assert.h>

/** Encode the length of a fixed-length column.
 @return buf + length of encoded val */
byte *page_zip_fixed_field_encode(
    byte *buf, /*!< in: pointer to buffer where to write */
    ulint val) /*!< in: value to write */
{
  ut_ad(val >= 2);

  if (UNIV_LIKELY(val < 126)) {
    /*
    0 = nullable variable field of at most 255 bytes length;
    1 = not null variable field of at most 255 bytes length;
    126 = nullable variable field with maximum length >255;
    127 = not null variable field with maximum length >255
    */
    *buf++ = (byte)val;
  } else {
    *buf++ = (byte)(0x80 | val >> 8);
    *buf++ = (byte)val;
  }

  return (buf);
}
