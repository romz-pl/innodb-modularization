#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/LatchCounter.h>
#include <innodb/sync_latch/latch_id_t.h>
#include <innodb/sync_latch/latch_level_t.h>

#ifndef UNIV_LIBRARY

/** Latch meta data */
template <typename Counter = LatchCounter>
class LatchMeta {
 public:
  typedef Counter CounterType;

#ifdef UNIV_PFS_MUTEX
  typedef mysql_pfs_key_t pfs_key_t;
#endif /* UNIV_PFS_MUTEX */

  /** Constructor */
  LatchMeta()
      : m_id(LATCH_ID_NONE),
        m_name(),
        m_level(SYNC_UNKNOWN),
        m_level_name()
#ifdef UNIV_PFS_MUTEX
        ,
        m_pfs_key()
#endif /* UNIV_PFS_MUTEX */
  {
  }

  /** Destructor */
  ~LatchMeta() {}

  /** Constructor
  @param[in]	id		Latch id
  @param[in]	name		Latch name
  @param[in]	level		Latch level
  @param[in]	level_name	Latch level text representation
  @param[in]	key		PFS key */
  LatchMeta(latch_id_t id, const char *name, latch_level_t level,
            const char *level_name
#ifdef UNIV_PFS_MUTEX
            ,
            pfs_key_t key
#endif /* UNIV_PFS_MUTEX */
            )
      : m_id(id),
        m_name(name),
        m_level(level),
        m_level_name(level_name)
#ifdef UNIV_PFS_MUTEX
        ,
        m_pfs_key(key)
#endif /* UNIV_PFS_MUTEX */
  {
    /* No op */
  }

  /* Less than operator.
  @param[in]	rhs		Instance to compare against
  @return true if this.get_id() < rhs.get_id() */
  bool operator<(const LatchMeta &rhs) const {
    return (get_id() < rhs.get_id());
  }

  /** @return the latch id */
  latch_id_t get_id() const { return (m_id); }

  /** @return the latch name */
  const char *get_name() const { return (m_name); }

  /** @return the latch level */
  latch_level_t get_level() const { return (m_level); }

  /** @return the latch level name */
  const char *get_level_name() const { return (m_level_name); }

#ifdef UNIV_PFS_MUTEX
  /** @return the PFS key for the latch */
  pfs_key_t get_pfs_key() const { return (m_pfs_key); }
#endif /* UNIV_PFS_MUTEX */

  /** @return the counter instance */
  Counter *get_counter() { return (&m_counter); }

 private:
  /** Latch id */
  latch_id_t m_id;

  /** Latch name */
  const char *m_name;

  /** Latch level in the ordering */
  latch_level_t m_level;

  /** Latch level text representation */
  const char *m_level_name;

#ifdef UNIV_PFS_MUTEX
  /** PFS key */
  pfs_key_t m_pfs_key;
#endif /* UNIV_PFS_MUTEX */

  /** For gathering latch statistics */
  Counter m_counter;
};

#endif
