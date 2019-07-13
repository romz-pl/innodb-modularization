#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/page/page_size_t.h>
#include <innodb/tablespace/space_id_t.h>

struct buf_block_t;
typedef byte buf_frame_t;

/** Callback functor. */
struct PageCallback {
  /** Default constructor */
  PageCallback() : m_page_size(0, 0, false), m_filepath() UNIV_NOTHROW {}

  virtual ~PageCallback() UNIV_NOTHROW {}

  /** Called for page 0 in the tablespace file at the start.
  @param file_size size of the file in bytes
  @param block contents of the first page in the tablespace file
  @retval DB_SUCCESS or error code. */
  virtual dberr_t init(os_offset_t file_size, const buf_block_t *block)
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Called for every page in the tablespace. If the page was not
  updated then its state must be set to BUF_PAGE_NOT_USED. For
  compressed tables the page descriptor memory will be at offset:
  block->frame + UNIV_PAGE_SIZE;
  @param offset physical offset within the file
  @param block block read from file, note it is not from the buffer pool
  @retval DB_SUCCESS or error code. */
  virtual dberr_t operator()(os_offset_t offset, buf_block_t *block)
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Set the name of the physical file and the file handle that is used
  to open it for the file that is being iterated over.
  @param filename then physical name of the tablespace file.
  @param file OS file handle */
  void set_file(const char *filename, pfs_os_file_t file) UNIV_NOTHROW {
    m_file = file;
    m_filepath = filename;
  }

  /** @return the space id of the tablespace */
  virtual space_id_t get_space_id() const
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /**
  @retval the space flags of the tablespace being iterated over */
  virtual ulint get_space_flags() const
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Set the tablespace table size.
  @param[in] page a page belonging to the tablespace */
  void set_page_size(const buf_frame_t *page) UNIV_NOTHROW;

  /** The compressed page size
  @return the compressed page size */
  const page_size_t &get_page_size() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_page_size);
  }

  /** The tablespace page size. */
  page_size_t m_page_size;

  /** File handle to the tablespace */
  pfs_os_file_t m_file;

  /** Physical file path. */
  const char *m_filepath;

  // Disable copying
  PageCallback(PageCallback &&) = delete;
  PageCallback(const PageCallback &) = delete;
  PageCallback &operator=(const PageCallback &) = delete;
};
