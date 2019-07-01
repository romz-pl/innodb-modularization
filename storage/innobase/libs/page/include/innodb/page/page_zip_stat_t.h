#pragma once

#include <innodb/univ/univ.h>

/** Compression statistics for a given page size */
struct page_zip_stat_t {
  /** Number of page compressions */
  ulint compressed;
  /** Number of successful page compressions */
  ulint compressed_ok;
  /** Number of page decompressions */
  ulint decompressed;
  /** Duration of page compressions in microseconds */
  ib_uint64_t compressed_usec;
  /** Duration of page decompressions in microseconds */
  ib_uint64_t decompressed_usec;
  page_zip_stat_t()
      : /* Initialize members to 0 so that when we do
        stlmap[key].compressed++ and element with "key" does not
        exist it gets inserted with zeroed members. */
        compressed(0),
        compressed_ok(0),
        decompressed(0),
        compressed_usec(0),
        decompressed_usec(0) {}
};
