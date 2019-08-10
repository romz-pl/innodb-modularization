#pragma once

#include <innodb/univ/univ.h>

/** Clone file information */
struct Clone_File_Meta {
  /** File size in bytes */
  ib_uint64_t m_file_size;

  /** Tablespace ID for the file */
  ulint m_space_id;

  /** File index in clone data file vector */
  uint m_file_index;

  /** Chunk number for the first chunk in file */
  uint m_begin_chunk;

  /** Chunk number for the last chunk in file */
  uint m_end_chunk;

  /** File name length in bytes */
  size_t m_file_name_len;

  /** File name */
  const char *m_file_name;
};
