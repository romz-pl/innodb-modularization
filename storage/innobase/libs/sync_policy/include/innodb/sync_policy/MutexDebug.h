#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_DEBUG

#include <innodb/sync_latch/latch_t.h>
#include <innodb/thread/os_thread_id_t.h>
#include <innodb/thread/os_thread_get_curr_id.h>
#include <innodb/thread/os_thread_eq.h>

#define MUTEX_MAGIC_N 979585UL

template <typename Mutex>
class MutexDebug {
 public:
  /** For passing context to SyncDebug */
  struct Context : public latch_t {
    /** Constructor */
    Context()
        : m_mutex(),
          m_filename(),
          m_line(),
          m_thread_id(os_thread_id_t(ULINT_UNDEFINED)) {
      /* No op */
    }

    /** Create the context for SyncDebug
    @param[in]	id	ID of the latch to track */
    Context(latch_id_t id) : latch_t(id) { /* No op */
    }

    /** Set to locked state
    @param[in]	mutex		The mutex to acquire
    @param[in]	filename	File name from where to acquire
    @param[in]	line		Line number in filename */
    void locked(const Mutex *mutex, const char *filename,
                ulint line) UNIV_NOTHROW {
      m_mutex = mutex;

      m_thread_id = os_thread_get_curr_id();

      m_filename = filename;

      m_line = line;
    }

    /** Reset to unlock state */
    void release() UNIV_NOTHROW {
      m_mutex = NULL;

      m_thread_id = os_thread_id_t(ULINT_UNDEFINED);

      m_filename = NULL;

      m_line = ULINT_UNDEFINED;
    }

    /** Print information about the latch
    @return the string representation */
    virtual std::string to_string() const UNIV_NOTHROW {
      std::ostringstream msg;

      msg << m_mutex->policy().to_string();

      if (m_thread_id != os_thread_id_t(ULINT_UNDEFINED)) {
        msg << " addr: " << m_mutex << " acquired: " << locked_from().c_str();

      } else {
        msg << "Not locked";
      }

      return (msg.str());
    }

    /** @return the name of the file and line number in the file
    from where the mutex was acquired "filename:line" */
    virtual std::string locked_from() const {
      std::ostringstream msg;

      msg << sync_basename(m_filename) << ":" << m_line;

      return (std::string(msg.str()));
    }

    /** Mutex to check for lock order violation */
    const Mutex *m_mutex;

    /** Filename from where enter was called */
    const char *m_filename;

    /** Line mumber in filename */
    ulint m_line;

    /** Thread ID of the thread that own(ed) the mutex */
    os_thread_id_t m_thread_id;
  };

  /** Constructor. */
  MutexDebug() : m_magic_n(), m_context() UNIV_NOTHROW { /* No op */
  }

  /* Destructor */
  virtual ~MutexDebug() {}

  /** Mutex is being destroyed. */
  void destroy() UNIV_NOTHROW {
    ut_ad(m_context.m_thread_id == os_thread_id_t(ULINT_UNDEFINED));

    m_magic_n = 0;

    m_context.m_thread_id = 0;
  }

  /** Called when the mutex is "created". Note: Not from the constructor
  but when the mutex is initialised.
  @param[in]	id              Mutex ID */
  void init(latch_id_t id) UNIV_NOTHROW;

  /** Called when an attempt is made to lock the mutex
  @param[in]	mutex		Mutex instance to be locked
  @param[in]	filename	Filename from where it was called
  @param[in]	line		Line number from where it was called */
  void enter(const Mutex *mutex, const char *filename, ulint line) UNIV_NOTHROW;

  /** Called when the mutex is locked
  @param[in]	mutex		Mutex instance that was locked
  @param[in]	filename	Filename from where it was called
  @param[in]	line		Line number from where it was called */
  void locked(const Mutex *mutex, const char *filename,
              ulint line) UNIV_NOTHROW;

  /** Called when the mutex is released
  @param[in]	mutex		Mutex that was released */
  void release(const Mutex *mutex) UNIV_NOTHROW;

  /** @return true if thread owns the mutex */
  bool is_owned() const UNIV_NOTHROW {
    return (os_thread_eq(m_context.m_thread_id, os_thread_get_curr_id()));
  }

  /** @return the name of the file from the mutex was acquired */
  const char *get_enter_filename() const UNIV_NOTHROW {
    return (m_context.m_filename);
  }

  /** @return the name of the file from the mutex was acquired */
  ulint get_enter_line() const UNIV_NOTHROW { return (m_context.m_line); }

  /** @return id of the thread that was trying to acquire the mutex */
  os_thread_id_t get_thread_id() const UNIV_NOTHROW {
    return (m_context.m_thread_id);
  }

  /** Magic number to check for memory corruption. */
  ulint m_magic_n;

  /** Latch state of the mutex owner */
  Context m_context;
};

template <typename Mutex>
void MutexDebug<Mutex>::init(latch_id_t id) UNIV_NOTHROW {
  m_context.m_id = id;

  m_context.release();

  m_magic_n = MUTEX_MAGIC_N;
}

template <typename Mutex>
void MutexDebug<Mutex>::enter(const Mutex *mutex, const char *name,
                              ulint line) UNIV_NOTHROW {
  ut_ad(!is_owned());

  Context context(m_context.get_id());

  context.locked(mutex, name, line);

  /* Check for latch order violation. */

  sync_check_lock_validate(&context);
}

template <typename Mutex>
void MutexDebug<Mutex>::locked(const Mutex *mutex, const char *name,
                               ulint line) UNIV_NOTHROW {
  ut_ad(!is_owned());
  ut_ad(m_context.m_thread_id == os_thread_id_t(ULINT_UNDEFINED));

  m_context.locked(mutex, name, line);

  sync_check_lock_granted(&m_context);
}

template <typename Mutex>
void MutexDebug<Mutex>::release(const Mutex *mutex) UNIV_NOTHROW {
  ut_ad(is_owned());

  m_context.release();

  sync_check_unlock(&m_context);
}

#endif /* UNIV_DEBUG */


#endif /* UNIV_LIBRARY */
