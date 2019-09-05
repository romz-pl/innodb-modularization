#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>
#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/allocator/UT_DELETE.h>

#include <vector>

template <typename Pool, typename LockStrategy>
struct PoolManager {
  typedef Pool PoolType;
  typedef typename PoolType::value_type value_type;

  PoolManager(size_t size) : m_size(size) { create(); }

  ~PoolManager() {
    destroy();

    ut_a(m_pools.empty());
  }

  /** Get an element from one of the pools.
  @return instance or NULL if pool is empty. */
  value_type *get() {
    size_t index = 0;
    size_t delay = 1;
    value_type *ptr = NULL;

    do {
      m_lock_strategy.enter();

      ut_ad(!m_pools.empty());

      size_t n_pools = m_pools.size();

      PoolType *pool = m_pools[index % n_pools];

      m_lock_strategy.exit();

      ptr = pool->get();

      if (ptr == 0 && (index / n_pools) > 2) {
        if (!add_pool(n_pools)) {
          ib::error(ER_IB_MSG_FAILED_TO_ALLOCATE_WAIT, m_size, delay);

          /* There is nothing much we can do
          except crash and burn, however lets
          be a little optimistic and wait for
          a resource to be freed. */
          os_thread_sleep(delay * 1000000);

          if (delay < 32) {
            delay <<= 1;
          }

        } else {
          delay = 1;
        }
      }

      ++index;

    } while (ptr == NULL);

    return (ptr);
  }

  static void mem_free(value_type *ptr) { PoolType::mem_free(ptr); }

 private:
  /** Add a new pool
  @param n_pools Number of pools that existed when the add pool was
                  called.
  @return true on success */
  bool add_pool(size_t n_pools) {
    bool added = false;

    m_lock_strategy.enter();

    if (n_pools < m_pools.size()) {
      /* Some other thread already added a pool. */
      added = true;
    } else {
      PoolType *pool;

      ut_ad(n_pools == m_pools.size());

      pool = UT_NEW_NOKEY(PoolType(m_size));

      if (pool != NULL) {
        ut_ad(n_pools <= m_pools.size());

        m_pools.push_back(pool);

        ib::info(ER_IB_MSG_NUM_POOLS, m_pools.size());

        added = true;
      }
    }

    ut_ad(n_pools < m_pools.size() || !added);

    m_lock_strategy.exit();

    return (added);
  }

  /** Create the pool manager. */
  void create() {
    ut_a(m_size > sizeof(value_type));
    m_lock_strategy.create();

    add_pool(0);
  }

  /** Release the resources. */
  void destroy() {
    typename Pools::iterator it;
    typename Pools::iterator end = m_pools.end();

    for (it = m_pools.begin(); it != end; ++it) {
      PoolType *pool = *it;

      UT_DELETE(pool);
    }

    m_pools.clear();

    m_lock_strategy.destroy();
  }

 private:
  // Disable copying
  PoolManager(const PoolManager &);
  PoolManager &operator=(const PoolManager &);

  typedef std::vector<PoolType *, ut_allocator<PoolType *>> Pools;

  /** Size of each block */
  size_t m_size;

  /** Pools managed this manager */
  Pools m_pools;

  /** Lock strategy to use */
  LockStrategy m_lock_strategy;
};
