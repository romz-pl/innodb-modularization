#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

#include <iosfwd>

/** Record lock for a page */
struct lock_rec_t {
  space_id_t space;  /*!< space id */
  page_no_t page_no; /*!< page number */
  uint32_t n_bits;   /*!< number of bits in the lock
                     bitmap; NOTE: the lock bitmap is
                     placed immediately after the
                     lock struct */

  /** Print the record lock into the given output stream
  @param[in,out]	out	the output stream
  @return the given output stream. */
  std::ostream &print(std::ostream &out) const;
};
