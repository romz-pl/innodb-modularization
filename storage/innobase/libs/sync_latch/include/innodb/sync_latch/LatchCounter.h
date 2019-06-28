#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_os/OSMutex.h>
#include <innodb/allocator/UT_DELETE.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>

#include <algorithm>
#include <vector>

/** Default latch counter */
class LatchCounter {
 public:
  /** The counts we collect for a mutex */
  struct Count {
    /** Constructor */
    Count() UNIV_NOTHROW : m_spins(), m_waits(), m_calls(), m_enabled() {
      /* No op */
    }

    /** Rest the values to zero */
    void reset() UNIV_NOTHROW {
      m_spins = 0;
      m_waits = 0;
      m_calls = 0;
    }

    /** Number of spins trying to acquire the latch. */
    uint32_t m_spins;

    /** Number of waits trying to acquire the latch */
    uint32_t m_waits;

    /** Number of times it was called */
    uint32_t m_calls;

    /** true if enabled */
    bool m_enabled;
  };

  /** Constructor */
  LatchCounter() UNIV_NOTHROW : m_active(false) { m_mutex.init(); }

  /** Destructor */
  ~LatchCounter() UNIV_NOTHROW {
    m_mutex.destroy();

    for (Counters::iterator it = m_counters.begin(); it != m_counters.end();
         ++it) {
      Count *count = *it;

      UT_DELETE(count);
    }
  }

  /** Reset all counters to zero. It is not protected by any
  mutex and we don't care about atomicity. Unless it is a
  demonstrated problem. The information collected is not
  required for the correct functioning of the server. */
  void reset() UNIV_NOTHROW {
    m_mutex.enter();

    Counters::iterator end = m_counters.end();

    for (Counters::iterator it = m_counters.begin(); it != end; ++it) {
      (*it)->reset();
    }

    m_mutex.exit();
  }

  /** @return the aggregate counter */
  Count *sum_register() UNIV_NOTHROW {
    m_mutex.enter();

    Count *count;

    if (m_counters.empty()) {
      count = UT_NEW_NOKEY(Count());
      m_counters.push_back(count);
    } else {
      ut_a(m_counters.size() == 1);
      count = m_counters[0];
    }

    m_mutex.exit();

    return (count);
  }

  /** Deregister the count. We don't do anything
  @param[in]	count		The count instance to deregister */
  void sum_deregister(Count *count) UNIV_NOTHROW { /* Do nothing */
  }

  /** Register a single instance counter */
  void single_register(Count *count) UNIV_NOTHROW {
    m_mutex.enter();

    m_counters.push_back(count);

    m_mutex.exit();
  }

  /** Deregister a single instance counter
  @param[in]	count		The count instance to deregister */
  void single_deregister(Count *count) UNIV_NOTHROW {
    m_mutex.enter();

    m_counters.erase(std::remove(m_counters.begin(), m_counters.end(), count),
                     m_counters.end());

    m_mutex.exit();
  }

  /** Iterate over the counters */
  template <typename Callback>
  void iterate(Callback &callback) const UNIV_NOTHROW {
    Counters::const_iterator end = m_counters.end();

    for (Counters::const_iterator it = m_counters.begin(); it != end; ++it) {
      callback(*it);
    }
  }

  /** Disable the monitoring */
  void enable() UNIV_NOTHROW {
    m_mutex.enter();

    Counters::const_iterator end = m_counters.end();

    for (Counters::const_iterator it = m_counters.begin(); it != end; ++it) {
      (*it)->m_enabled = true;
    }

    m_active = true;

    m_mutex.exit();
  }

  /** Disable the monitoring */
  void disable() UNIV_NOTHROW {
    m_mutex.enter();

    Counters::const_iterator end = m_counters.end();

    for (Counters::const_iterator it = m_counters.begin(); it != end; ++it) {
      (*it)->m_enabled = false;
    }

    m_active = false;

    m_mutex.exit();
  }

  /** @return if monitoring is active */
  bool is_enabled() const UNIV_NOTHROW { return (m_active); }

 private:
  /* Disable copying */
  LatchCounter(const LatchCounter &);
  LatchCounter &operator=(const LatchCounter &);

 private:
  typedef OSMutex Mutex;
  typedef std::vector<Count *> Counters;

  /** Mutex protecting m_counters */
  Mutex m_mutex;

  /** Counters for the latches */
  Counters m_counters;

  /** if true then we collect the data */
  bool m_active;
};
