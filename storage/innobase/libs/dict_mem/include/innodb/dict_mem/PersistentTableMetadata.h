#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/corrupted_ids_t.h>

/** Persistent dynamic metadata for a table */
class PersistentTableMetadata {
 public:
  /** Constructor
  @param[in]	id	table id
  @param[in]	version	table dynamic metadata version */
  PersistentTableMetadata(table_id_t id, uint64 version)
      : m_id(id), m_version(version), m_corrupted_ids(), m_autoinc(0) {}

  /** Get the corrupted indexes' IDs
  @return the vector of indexes' IDs */
  const corrupted_ids_t &get_corrupted_indexes() const {
    return (m_corrupted_ids);
  }

  /** Add a corrupted index id and space id
  @param[in]	id	corrupted index id */
  void add_corrupted_index(const index_id_t id) {
    m_corrupted_ids.push_back(id);
  }

  /** Set the dynamic metadata version.
  @param[in]	version		dynamic metadata version */
  void set_version(uint64_t version) { m_version = version; }

  /** Get the dynamic metadata version */
  uint64_t get_version() const { return (m_version); }

  /** Get the table id of the metadata
  @return table id */
  table_id_t get_table_id() const { return (m_id); }

  /** Set the autoinc counter of the table if it's bigger
  @param[in]	autoinc	autoinc counter */
  void set_autoinc_if_bigger(uint64_t autoinc) {
    /* We only set the biggest autoinc counter. Callers don't
    guarantee passing a bigger number in. */
    if (autoinc > m_autoinc) {
      m_autoinc = autoinc;
    }
  }

  /** Set the autoinc counter of the table
  @param[in]	autoinc	autoinc counter */
  void set_autoinc(uint64_t autoinc) { m_autoinc = autoinc; }

  /** Get the autoinc counter of the table
  @return the autoinc counter */
  uint64_t get_autoinc() const { return (m_autoinc); }

 private:
  /** Table ID which this metadata belongs to */
  table_id_t m_id;

  /** Table dynamic metadata version of the change */
  uint64_t m_version;

  /** Storing the corrupted indexes' ID if exist, or else empty */
  corrupted_ids_t m_corrupted_ids;

  /** Autoinc counter of the table */
  uint64_t m_autoinc;

  /* TODO: We will add update_time, etc. here and APIs accordingly */
};
