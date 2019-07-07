#include <innodb/buffer/buf_frame_align.h>

#include <innodb/align/ut_align_down.h>
#include <innodb/assert/assert.h>

/***********************************************************************
FIXME_FTS Gets the frame the pointer is pointing to. */
buf_frame_t *buf_frame_align(
    /* out: pointer to frame */
    byte *ptr) /* in: pointer to a frame */
{
  buf_frame_t *frame;

  ut_ad(ptr);

  frame = (buf_frame_t *)ut_align_down(ptr, UNIV_PAGE_SIZE);

  return (frame);
}
