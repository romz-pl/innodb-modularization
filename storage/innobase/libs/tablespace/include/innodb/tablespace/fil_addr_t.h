#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_no_t.h>

#include <ostream>

/** File space address */
struct fil_addr_t {
  /* Default constructor */
  fil_addr_t() : page(FIL_NULL), boffset(0) {}

  /** Constructor
  @param[in]	p	Logical page number
  @param[in]	boff	Offset within the page */
  fil_addr_t(page_no_t p, uint32_t boff) : page(p), boffset(boff) {}

  /** Compare to instances
  @param[in]	rhs	Instance to compare with
  @return true if the page number and page offset are equal */
  bool is_equal(const fil_addr_t &rhs) const {
    return (page == rhs.page && boffset == rhs.boffset);
  }

  /** Check if the file address is null.
  @return true if null */
  bool is_null() const { return (page == FIL_NULL && boffset == 0); }

  /** Print a string representation.
  @param[in,out]	out		Stream to write to */
  std::ostream &print(std::ostream &out) const {
    out << "[fil_addr_t: page=" << page << ", boffset=" << boffset << "]";

    return (out);
  }

  /** Page number within a space */
  page_no_t page;

  /** Byte offset within the page */
  uint32_t boffset;
};

/* For printing fil_addr_t to a stream.
@param[in,out]	out		Stream to write to
@param[in]	obj		fil_addr_t instance to write */
inline std::ostream &operator<<(std::ostream &out, const fil_addr_t &obj) {
  return (obj.print(out));
}
