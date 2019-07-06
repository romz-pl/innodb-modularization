#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_no_t.h>
#include <innodb/tablespace/space_id_t.h>
#include <innodb/assert/assert.h>

#include <iosfwd>

/** Page identifier. */
class page_id_t {
 public:
  /** Default constructor */
  page_id_t() : m_space(), m_page_no(), m_fold() {}

  /** Constructor from (space, page_no).
  @param[in]	space	tablespace id
  @param[in]	page_no	page number */
  page_id_t(space_id_t space, page_no_t page_no)
      : m_space(space), m_page_no(page_no), m_fold(ULINT_UNDEFINED) {}

  page_id_t(const page_id_t &) = default;

  /** Retrieve the tablespace id.
  @return tablespace id */
  inline space_id_t space() const { return (m_space); }

  /** Retrieve the page number.
  @return page number */
  inline page_no_t page_no() const { return (m_page_no); }

  /** Retrieve the fold value.
  @return fold value */
  inline ulint fold() const {
    /* Initialize m_fold if it has not been initialized yet. */
    if (m_fold == ULINT_UNDEFINED) {
      m_fold = (m_space << 20) + m_space + m_page_no;
      ut_ad(m_fold != ULINT_UNDEFINED);
    }

    return (m_fold);
  }

  /** Copy the values from a given page_id_t object.
  @param[in]	src	page id object whose values to fetch */
  inline void copy_from(const page_id_t &src) {
    m_space = src.space();
    m_page_no = src.page_no();
    m_fold = src.fold();
  }

  /** Reset the values from a (space, page_no).
  @param[in]	space	tablespace id
  @param[in]	page_no	page number */
  inline void reset(space_id_t space, page_no_t page_no) {
    m_space = space;
    m_page_no = page_no;
    m_fold = ULINT_UNDEFINED;
  }

  /** Reset the page number only.
  @param[in]	page_no	page number */
  inline void set_page_no(page_no_t page_no) {
    m_page_no = page_no;
    m_fold = ULINT_UNDEFINED;
  }

  /** Check if a given page_id_t object is equal to the current one.
  @param[in]	a	page_id_t object to compare
  @return true if equal */
  inline bool equals_to(const page_id_t &a) const {
    return (a.space() == m_space && a.page_no() == m_page_no);
  }

 private:
  /** Tablespace id. */
  space_id_t m_space;

  /** Page number. */
  page_no_t m_page_no;

  /** A fold value derived from m_space and m_page_no,
  used in hashing. */
  mutable ulint m_fold;

  /* Disable implicit copying. */
  void operator=(const page_id_t &) = delete;

  /** Declare the overloaded global operator<< as a friend of this
  class. Refer to the global declaration for further details.  Print
  the given page_id_t object.
  @param[in,out]	out	the output stream
  @param[in]	page_id	the page_id_t object to be printed
  @return the output stream */
  friend std::ostream &operator<<(std::ostream &out, const page_id_t &page_id);
};

/** Print the given page_id_t object.
@param[in,out]	out	the output stream
@param[in]	page_id	the page_id_t object to be printed
@return the output stream */
std::ostream &operator<<(std::ostream &out, const page_id_t &page_id);
