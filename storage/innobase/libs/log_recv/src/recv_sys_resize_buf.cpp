#include <innodb/log_recv/recv_sys_resize_buf.h>

#include <innodb/allocator/ut_realloc.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/log_buffer/srv_log_buffer_size.h>
#include <innodb/logger/info.h>

/** Resize the recovery parsing buffer upto log_buffer_size */
bool recv_sys_resize_buf() {
  ut_ad(recv_sys->buf_len <= srv_log_buffer_size);

  /* If the buffer cannot be extended further, return false. */
  if (recv_sys->buf_len == srv_log_buffer_size) {
    ib::error(ER_IB_MSG_723, srv_log_buffer_size);
    return false;
  }

  /* Extend the buffer by double the current size with the resulting
  size not more than srv_log_buffer_size. */
  recv_sys->buf_len = ((recv_sys->buf_len * 2) >= srv_log_buffer_size)
                          ? srv_log_buffer_size
                          : recv_sys->buf_len * 2;

  /* Resize the buffer to the new size. */
  recv_sys->buf =
      static_cast<byte *>(ut_realloc(recv_sys->buf, recv_sys->buf_len));

  ut_ad(recv_sys->buf != nullptr);

  /* Return error and fail the recovery if not enough memory available */
  if (recv_sys->buf == nullptr) {
    ib::error(ER_IB_MSG_740);
    return false;
  }

  ib::info(ER_IB_MSG_739, recv_sys->buf_len);
  return true;
}

