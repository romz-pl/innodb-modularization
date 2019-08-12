#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/dtype_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/data_types/spatial_status_t.h>

#include <ostream>

struct dict_v_col_t;

/** Structure for an SQL data field */
struct dfield_t {
  void *data;       /*!< pointer to data */
  unsigned ext : 1; /*!< TRUE=externally stored, FALSE=local */
  unsigned spatial_status : 2;
  /*!< spatial status of externally stored field
  in undo log for purge */
  unsigned len; /*!< data length; UNIV_SQL_NULL if SQL null */
  dtype_t type; /*!< type of data */

  void reset() {
    data = nullptr;
    ext = FALSE;
    spatial_status = SPATIAL_UNKNOWN, len = 0;
  }

  /** Create a deep copy of this object
  @param[in]	heap	the memory heap in which the clone will be
                          created.
  @return	the cloned object. */
  dfield_t *clone(mem_heap_t *heap);

  byte *blobref() const;

  /** Obtain the LOB version number, if this is an externally
  stored field. */
  uint32_t lob_version() const;

  dfield_t()
      : data(nullptr), ext(0), spatial_status(0), len(0), type({0, 0, 0, 0}) {}

  /** Print the dfield_t object into the given output stream.
  @param[in]	out	the output stream.
  @return	the ouput stream. */
  std::ostream &print(std::ostream &out) const;

  /** Adjust and(or) set virtual column value which is read from undo
  or online DDL log
  @param[in]	vcol	virtual column definition
  @param[in]	comp	true if compact format
  @param[in]	field	virtual column value
  @param[in]	len	value length
  @param[in,out]	heap	memory heap to keep value when necessary */
  void adjust_v_data_mysql(const dict_v_col_t *vcol, bool comp,
                           const byte *field, ulint len, mem_heap_t *heap);
};

/** Overloading the global output operator to easily print the given dfield_t
object into the given output stream.
@param[in]	out	the output stream
@param[in]	obj	the given object to print.
@return the output stream. */
inline std::ostream &operator<<(std::ostream &out, const dfield_t &obj) {
  return (obj.print(out));
}
