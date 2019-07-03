#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_offset_t.h>

/** Sparse file size information. */
struct os_file_size_t {
  /** Total size of file in bytes */
  os_offset_t m_total_size;

  /** If it is a sparse file then this is the number of bytes
  actually allocated for the file. */
  os_offset_t m_alloc_size;
};
