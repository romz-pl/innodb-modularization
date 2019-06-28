#pragma once

#include <innodb/univ/univ.h>
#include <innodb/allocator/ut_allocator.h>
#include <innodb/sync_latch/latch_meta_t.h>

#include <vector>

#ifndef UNIV_LIBRARY

    typedef std::vector<latch_meta_t *, ut_allocator<latch_meta_t *>> LatchMetaData;

/** Note: This is accessed without any mutex protection. It is initialised
at startup and elements should not be added to or removed from it after
that.  See sync_latch_meta_init() */
extern LatchMetaData latch_meta;

/** Get the latch meta-data from the latch ID
@param[in]	id		Latch ID
@return the latch meta data */
inline latch_meta_t &sync_latch_get_meta(latch_id_t id) {
  ut_ad(static_cast<size_t>(id) < latch_meta.size());
  ut_ad(id == latch_meta[id]->get_id());

  return (*latch_meta[id]);
}

/** Fetch the counter for the latch
@param[in]	id		Latch ID
@return the latch counter */
inline latch_meta_t::CounterType *sync_latch_get_counter(latch_id_t id) {
  latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_counter());
}

/** Get the latch name from the latch ID
@param[in]	id		Latch ID
@return the name, will assert if not found */
inline const char *sync_latch_get_name(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_name());
}

/** Get the latch ordering level
@param[in]	id		Latch id to lookup
@return the latch level */
inline latch_level_t sync_latch_get_level(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_level());
}

#ifdef UNIV_PFS_MUTEX
/** Get the latch PFS key from the latch ID
@param[in]	id		Latch ID
@return the PFS key */
inline mysql_pfs_key_t sync_latch_get_pfs_key(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_pfs_key());
}

#ifndef UNIV_HOTBACKUP
/** String representation of the filename and line number where the
latch was created
@param[in]	id		Latch ID
@param[in]	created		Filename and line number where it was crated
@return the string representation */
std::string sync_mutex_to_string(latch_id_t id, const std::string &created);

/** Get the latch name from a sync level
@param[in]	level		Latch level to lookup
@return 0 if not found. */
const char *sync_latch_get_name(latch_level_t level);

/** Print the filename "basename"
@return the basename */
const char *sync_basename(const char *filename);
#endif /* !UNIV_HOTBACKUP */

#endif


#ifdef UNIV_PFS_MUTEX
/** Latch element
Used for mutexes which have PFS keys defined under UNIV_PFS_MUTEX.
@param[in]	id		Latch id
@param[in]	level		Latch level
@param[in]	key		PFS key */
#define LATCH_ADD_MUTEX(id, level, key) \
  latch_meta[LATCH_ID_##id] =           \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level, key))

#ifdef UNIV_PFS_RWLOCK
/** Latch element.
Used for rwlocks which have PFS keys defined under UNIV_PFS_RWLOCK.
@param[in]	id		Latch id
@param[in]	level		Latch level
@param[in]	key		PFS key */
#define LATCH_ADD_RWLOCK(id, level, key) \
  latch_meta[LATCH_ID_##id] =            \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level, key))
#else
#define LATCH_ADD_RWLOCK(id, level, key)    \
  latch_meta[LATCH_ID_##id] = UT_NEW_NOKEY( \
      latch_meta_t(LATCH_ID_##id, #id, level, #level, PSI_NOT_INSTRUMENTED))
#endif /* UNIV_PFS_RWLOCK */

#else
#define LATCH_ADD_MUTEX(id, level, key) \
  latch_meta[LATCH_ID_##id] =           \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level))
#define LATCH_ADD_RWLOCK(id, level, key) \
  latch_meta[LATCH_ID_##id] =            \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level))
#endif /* UNIV_PFS_MUTEX */

#endif
