#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Dblwr_Blocks.h>
#include <innodb/error/dberr_t.h>
#include <innodb/log_arch/Arch_File_Ctx.h>
#include <innodb/allocator/UT_DELETE_ARRAY.h>

/** Doublewrite buffer context. */
class Arch_Dblwr_Ctx {
 public:
  /** Constructor: Initialize members */
  Arch_Dblwr_Ctx() {}

  /** Destructor: Close open file and free resources */
  ~Arch_Dblwr_Ctx() {
    if (m_buf != nullptr) {
      UT_DELETE_ARRAY(m_buf);
    }

    m_file_ctx.close();
  }

  /** Initialize the doublewrite buffer.
  @return error code. */
  dberr_t init(const char *path, const char *base_file, uint num_files,
               ib_uint64_t file_size);

  /** Load doublewrite buffer from the file that archiver maintains to
  keep doublewrite buffer.
  @return error code */
  dberr_t read_blocks();

  /** Get doubewrite buffer blocks.
  @return doublewrite buffer blocks */
  Arch_Dblwr_Blocks get_blocks() { return (m_blocks); }

  /** Disable copy construction */
  Arch_Dblwr_Ctx(Arch_Dblwr_Ctx const &) = delete;

  /** Disable assignment */
  Arch_Dblwr_Ctx &operator=(Arch_Dblwr_Ctx const &) = delete;

 private:
  /** Buffer to hold the contents of the doublwrite buffer. */
  byte *m_buf{nullptr};

  /** Total file size of the file which holds the doublewrite buffer. */
  uint64_t m_file_size{};

  /** Doublewrite buffer file context. */
  Arch_File_Ctx m_file_ctx{};

  /** List of doublewrite buffer blocks. */
  Arch_Dblwr_Blocks m_blocks{};
};
