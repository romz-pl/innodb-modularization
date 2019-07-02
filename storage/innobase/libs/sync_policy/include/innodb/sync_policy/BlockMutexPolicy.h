#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/LatchMetaData.h>
#include <innodb/sync_policy/MutexDebug.h>
#include <innodb/sync_latch/sync_latch_get_meta.h>

#ifndef UNIV_LIBRARY

/** Track agregate metrics policy, used by the page mutex. There are just
too many of them to count individually. */
template <typename Mutex>
class BlockMutexPolicy
#ifdef UNIV_DEBUG
    : public MutexDebug<Mutex>
#endif /* UNIV_DEBUG */
{
 public:
  typedef Mutex MutexType;
  typedef typename latch_meta_t::CounterType::Count Count;

  /** Default constructor. */
  BlockMutexPolicy()
      :
#ifdef UNIV_DEBUG
        MutexDebug<MutexType>(),
#endif /* UNIV_DEBUG */
        m_count(),
        m_id() {
    /* Do nothing */
  }

  /** Destructor */
  ~BlockMutexPolicy() {}

  /** Called when the mutex is "created". Note: Not from the constructor
  but when the mutex is initialised.
  @param[in]	mutex		Mutex instance to track
  @param[in]	id              Mutex ID
  @param[in]	filename	File where mutex was created
  @param[in]	line		Line in filename */
  void init(const MutexType &mutex, latch_id_t id, const char *filename,
            uint32_t line) UNIV_NOTHROW {
    /* It can be LATCH_ID_BUF_BLOCK_MUTEX or
    LATCH_ID_BUF_POOL_ZIP. Unfortunately, they
    are mapped to the same mutex type in the
    buffer pool code. */

    m_id = id;

    latch_meta_t &meta = sync_latch_get_meta(m_id);

    ut_ad(meta.get_id() == id);

    m_count = meta.get_counter()->sum_register();

    ut_d(MutexDebug<MutexType>::init(m_id));
  }

  /** Called when the mutex is destroyed. */
  void destroy() UNIV_NOTHROW {
    latch_meta_t &meta = sync_latch_get_meta(m_id);

    ut_ad(meta.get_id() == m_id);

    meta.get_counter()->sum_deregister(m_count);

    m_count = NULL;

    ut_d(MutexDebug<MutexType>::destroy());
  }

  /** Called after a successful mutex acquire.
  @param[in]	n_spins		Number of times the thread did
                                  spins while trying to acquire the mutex
  @param[in]	n_waits		Number of times the thread waited
                                  in some type of OS queue */
  void add(uint32_t n_spins, uint32_t n_waits) UNIV_NOTHROW {
    if (!m_count->m_enabled) {
      return;
    }

    m_count->m_spins += n_spins;
    m_count->m_waits += n_waits;

    ++m_count->m_calls;
  }

  /** Called when the mutex is locked
  @param[in]	mutex		Mutex instance that is locked
  @param[in]	filename	Filename from where it was called
  @param[in]	line		Line number from where it was called */
  void locked(const MutexType &mutex, const char *filename,
              ulint line) UNIV_NOTHROW {
    ut_d(MutexDebug<MutexType>::locked(&mutex, filename, line));
  }

  /** Called when the mutex is released
  @param[in]	mutex		Mutex instance that is released */
  void release(const MutexType &mutex) UNIV_NOTHROW {
    ut_d(MutexDebug<MutexType>::release(&mutex));
  }

  /** Called when an attempt is made to lock the mutex
  @param[in]	mutex		Mutex instance to be locked
  @param[in]	filename	Filename from where it was called
  @param[in]	line		Line number from where it was called */
  void enter(const MutexType &mutex, const char *filename,
             ulint line) UNIV_NOTHROW {
    ut_d(MutexDebug<MutexType>::enter(&mutex, filename, line));
  }

  /** Print the information about the latch
  @return the string representation */
  std::string print() const UNIV_NOTHROW;

  /** @return the latch ID */
  latch_id_t get_id() const { return (m_id); }

  /** @return the string representation */
  std::string to_string() const;

 private:
  typedef latch_meta_t::CounterType Counter;

  /** The user visible counters, registered with the meta-data.  */
  Counter::Count *m_count;

  /** Latch meta data ID */
  latch_id_t m_id;
};

template <typename Mutex>
std::string BlockMutexPolicy<Mutex>::to_string() const {
  /* I don't think it makes sense to keep track of the file name
  and line number for each block mutex. Too much of overhead. Use the
  latch id to figure out the location from the source. */
  return (sync_mutex_to_string(get_id(), "buf0buf.cc:0"));
}


#endif /* UNIV_LIBRARY */
