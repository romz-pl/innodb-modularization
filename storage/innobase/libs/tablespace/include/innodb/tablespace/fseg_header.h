#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

struct mtr_t;

/** A wrapper class to print the file segment header information. */
class fseg_header {
 public:
  /** Constructor of fseg_header.
  @param[in]	header	the underlying file segment header object
  @param[in]	mtr	the mini-transaction.  No redo logs are
                          generated, only latches are checked within
                          mini-transaction */
  fseg_header(const fseg_header_t *header, mtr_t *mtr)
      : m_header(header), m_mtr(mtr) {}

  /** Print the file segment header to the given output stream.
  @param[in,out]	out	the output stream into which the object
                          is printed.
  @retval	the output stream into which the object was printed. */
  std::ostream &to_stream(std::ostream &out) const;

 private:
  /** The underlying file segment header */
  const fseg_header_t *m_header;

  /** The mini transaction, which is used mainly to check whether
  appropriate latches have been taken by the calling thread. */
  mtr_t *m_mtr;
};

/* Overloading the global output operator to print a file segment header
@param[in,out]	out	the output stream into which object will be printed
@param[in]	header	the file segment header to be printed
@retval the output stream */
inline std::ostream &operator<<(std::ostream &out, const fseg_header &header) {
  return (header.to_stream(out));
}
#endif /* UNIV_DEBUG */
