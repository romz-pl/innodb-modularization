#pragma once

#include <innodb/univ/univ.h>

/** If key is fixed length key then cache the record offsets on first
computation. This will help save computation cycle that generate same
redundant data. */
class rec_cache_t {
 public:
  /** Constructor */
  rec_cache_t()
      : rec_size(),
        offsets(),
        sz_of_offsets(),
        fixed_len_key(),
        offsets_cached(),
        key_has_null_cols() {
    /* Do Nothing. */
  }

 public:
  /** Record size. (for fixed length key record size is constant) */
  ulint rec_size;

  /** Holds reference to cached offsets for record. */
  ulint *offsets;

  /** Size of offset array */
  uint32_t sz_of_offsets;

  /** If true, then key is fixed length key. */
  bool fixed_len_key;

  /** If true, then offset has been cached for re-use. */
  bool offsets_cached;

  /** If true, then key part can have columns that can take
  NULL values. */
  bool key_has_null_cols;
};
