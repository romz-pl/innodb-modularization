#include <innodb/buf_frame/buf_frame_copy.h>

#include <innodb/assert/assert.h>

#include <string.h>

/** Copies contents of a buffer frame to a given buffer.
 @return buf */
byte *buf_frame_copy(byte *buf,                /*!< in: buffer to copy to */
                     const buf_frame_t *frame) /*!< in: buffer frame */
{
  ut_ad(buf && frame);

  memcpy(buf, frame, UNIV_PAGE_SIZE);

  return (buf);
}
