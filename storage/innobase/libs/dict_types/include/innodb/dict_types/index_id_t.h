#pragma once

#include <innodb/univ/univ.h>

#include <innodb/assert/assert.h>
#include <innodb/dict_types/flags.h>

#include <ostream>

/** Globally unique index identifier */
class index_id_t {
 public:
  /** Constructor.
  @param[in]	space_id	Tablespace identifier
  @param[in]	index_id	Index identifier */
  index_id_t(space_id_t space_id, space_index_t index_id)
      : m_space_id(space_id), m_index_id(index_id) {}

  /** Compare this to another index identifier.
  @param other	the other index identifier
  @return whether this is less than other */
  bool operator<(const index_id_t &other) const {
    return (m_space_id < other.m_space_id ||
            (m_space_id == other.m_space_id && m_index_id < other.m_index_id));
  }
  /** Compare this to another index identifier.
  @param other	the other index identifier
  @return whether the identifiers are equal */
  bool operator==(const index_id_t &other) const {
    return (m_space_id == other.m_space_id && m_index_id == other.m_index_id);
  }

  /** Convert an index_id to a 64 bit integer.
  @return a 64 bit integer */
  uint64_t conv_to_int() const {
    ut_ad((m_index_id & 0xFFFFFFFF00000000ULL) == 0);

    return (static_cast<uint64_t>(m_space_id) << 32 | m_index_id);
  }

  /** Check if the index belongs to the insert buffer.
  @return true if the index belongs to the insert buffer */
  bool is_ibuf() const {
    return (m_space_id == IBUF_SPACE_ID &&
            m_index_id == DICT_IBUF_ID_MIN + IBUF_SPACE_ID);
  }

  /** Tablespace identifier */
  space_id_t m_space_id;
  /** Index identifier within the tablespace */
  space_index_t m_index_id;
};

/** Display an index identifier.
@param[in,out]	out	the output stream
@param[in]	id	index identifier
@return the output stream */
inline std::ostream &operator<<(std::ostream &out, const index_id_t &id) {
  return (out << "[space=" << id.m_space_id << ",index=" << id.m_index_id
              << "]");
}
