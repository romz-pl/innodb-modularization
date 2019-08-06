#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/cpu/flags.h>
#include <innodb/random/random.h>
#include <innodb/sync_latch/latch_level_t.h>
#include <innodb/sync_rw/rw_lock_create.h>
#include <innodb/sync_rw/rw_lock_free.h>
#include <innodb/sync_rw/rw_lock_s_lock.h>
#include <innodb/sync_rw/rw_lock_s_lock_nowait.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>

#include <algorithm>

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY

/* The sharded read-write lock (for threads).

The s-lock scales better than in single rw-lock,
but the x-lock is much slower. */

/** Rw-lock with very fast, highly concurrent s-lock but slower x-lock.
It's basically array of rw-locks. When s-lock is being acquired, single
rw-lock from array is selected randomly and s-locked. Therefore, all
rw-locks from array has to be x-locked when x-lock is being acquired.

Purpose of this data structure is to reduce contention on single atomic
in single rw-lock when a lot of threads need to acquire s-lock very often,
but x-lock is very rare. */
class Sharded_rw_lock {
 public:
  void create(mysql_pfs_key_t pfs_key, latch_level_t latch_level,
              size_t n_shards) {
    m_n_shards = n_shards;

    m_shards = static_cast<Shard *>(ut_zalloc_nokey(sizeof(Shard) * n_shards));

    for_each([pfs_key, latch_level](rw_lock_t &lock) {
      static_cast<void>(latch_level);  // clang -Wunused-lambda-capture
      rw_lock_create(pfs_key, &lock, latch_level);
    });
  }

  void free() {
    ut_a(m_shards != nullptr);

    for_each([](rw_lock_t &lock) { rw_lock_free(&lock); });

    ut_free(m_shards);
    m_shards = nullptr;
    m_n_shards = 0;
  }

  size_t s_lock() {
    const size_t shard_no = ut_rnd_interval(0, m_n_shards - 1);
    rw_lock_s_lock(&m_shards[shard_no].lock);
    return shard_no;
  }

  ibool s_lock_nowait(size_t &shard_no, const char *file, ulint line) {
    shard_no = ut_rnd_interval(0, m_n_shards - 1);
    return rw_lock_s_lock_nowait(&m_shards[shard_no].lock, file, line);
  }

  void s_unlock(size_t shard_no) {
    ut_a(shard_no < m_n_shards);
    rw_lock_s_unlock(&m_shards[shard_no].lock);
  }

  void x_lock() {
    for_each([](rw_lock_t &lock) { rw_lock_x_lock(&lock); });
  }

  void x_unlock() {
    for_each([](rw_lock_t &lock) { rw_lock_x_unlock(&lock); });
  }

#ifdef UNIV_DEBUG
  bool s_own(size_t shard_no) const {
    return rw_lock_own(&m_shards[shard_no].lock, RW_LOCK_S);
  }

  bool x_own() const { return rw_lock_own(&m_shards[0].lock, RW_LOCK_X); }
#endif /* !UNIV_DEBUG */

 private:
  struct Shard {
    rw_lock_t lock;

    char pad[INNOBASE_CACHE_LINE_SIZE];
  };

  template <typename F>
  void for_each(F f) {
    std::for_each(m_shards, m_shards + m_n_shards,
                  [&f](Shard &shard) { f(shard.lock); });
  }

  Shard *m_shards = nullptr;

  size_t m_n_shards = 0;
};

#else /* !UNIV_LIBRARY */

/* For UNIV_LIBRARY, rw_lock is no-op, so sharded rw-lock is also no-op. */

class Sharded_rw_lock {
 public:
  void create(mysql_pfs_key_t pfs_key, latch_level_t latch_level,
              size_t n_shards) {}

  void free() {}

  size_t s_lock() { return 0; }

  void s_unlock(size_t shard_no) { ut_a(shard_no == 0); }

  void x_lock() {}

  void x_unlock() {}
};

#endif /* UNIV_LIBRARY */
#endif /* UNIV_HOTBACKUP */
