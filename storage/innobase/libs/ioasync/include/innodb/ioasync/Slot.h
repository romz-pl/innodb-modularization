#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Block.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/io/pfs_os_file_t.h>

#include <libaio.h>

/** File node of a tablespace or the log data space */
struct fil_node_t;

/** The asynchronous I/O context */
struct Slot {
  /** index of the slot in the aio array */
  uint16_t pos{0};

  /** true if this slot is reserved */
  bool is_reserved{false};

  /** time when reserved */
  time_t reservation_time{0};

  /** buffer used in i/o */
  byte *buf{nullptr};

  /** Buffer pointer used for actual IO. We advance this
  when partial IO is required and not buf */
  byte *ptr{nullptr};

  /** OS_FILE_READ or OS_FILE_WRITE */
  IORequest type{IORequest::UNSET};

  /** file offset in bytes */
  os_offset_t offset{0};

  /** file where to read or write */
  pfs_os_file_t file{
#ifdef UNIV_PFS_IO
      nullptr,  // m_psi
#endif
      0  // m_file
  };

  /** file name or path */
  const char *name{nullptr};

  /** used only in simulated aio: true if the physical i/o
  already made and only the slot message needs to be passed
  to the caller of os_aio_simulated_handle */
  bool io_already_done{false};

  /** The file node for which the IO is requested. */
  fil_node_t *m1{nullptr};

  /** the requester of an aio operation and which can be used
  to identify which pending aio operation was completed */
  void *m2{nullptr};

  /** AIO completion status */
  dberr_t err{DB_ERROR_UNSET};

#ifdef WIN_ASYNC_IO
  /** handle object we need in the OVERLAPPED struct */
  HANDLE handle{INVALID_HANDLE_VALUE};

  /** Windows control block for the aio request */
  OVERLAPPED control{0, 0};

  /** bytes written/read */
  DWORD n_bytes{0};

  /** length of the block to read or write */
  DWORD len{0};

#elif defined(LINUX_NATIVE_AIO)
  /** Linux control block for aio */
  struct iocb control;

  /** AIO return code */
  int ret{0};

  /** bytes written/read. */
  ssize_t n_bytes{0};

  /** length of the block to read or write */
  ulint len{0};
#else
  /** length of the block to read or write */
  ulint len{0};

  /** bytes written/read. */
  ulint n_bytes{0};
#endif /* WIN_ASYNC_IO */

  /** Length of the block before it was compressed */
  uint32 original_len{0};

  /** Buffer block for compressed pages or encrypted pages */
  Block *buf_block{nullptr};

  /** true, if we shouldn't punch a hole after writing the page */
  bool skip_punch_hole{false};

  /** Buffer for encrypt log */
  void *encrypt_log_buf{nullptr};

  Slot() {
#if defined(LINUX_NATIVE_AIO)
    memset(&control, 0, sizeof(control));
#endif /* LINUX_NATIVE_AIO */
  }
};
