#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_latch/latch_id_t.h>
#include <innodb/sync_latch/LatchMetaData.h>

/** All (ordered) latches, used in debugging, must derive from this class. */
struct latch_t {
  /** Constructor
  @param[in]	id	The latch ID */
  explicit latch_t(latch_id_t id = LATCH_ID_NONE) UNIV_NOTHROW : m_id(id),
                                                                 m_rw_lock(),
                                                                 m_temp_fsp() {}

  latch_t &operator=(const latch_t &) = default;

  /** Destructor */
  virtual ~latch_t() UNIV_NOTHROW {}

  /** @return the latch ID */
  latch_id_t get_id() const { return (m_id); }

  /** @return true if it is a rw-lock */
  bool is_rw_lock() const UNIV_NOTHROW { return (m_rw_lock); }

  /** Print the latch context
  @return the string representation */
  virtual std::string to_string() const = 0;

  /** @return "filename:line" from where the latch was last locked */
  virtual std::string locked_from() const = 0;

  /** @return the latch level */
  latch_level_t get_level() const UNIV_NOTHROW {
    ut_a(m_id != LATCH_ID_NONE);

#ifndef UNIV_LIBRARY
    return (sync_latch_get_level(m_id));
#else
    /* This should never be reached. This is
    added to fix compilation errors
    for library. We will never reach here because
    mutexes are disabled in library. */
    ut_error;
    return (SYNC_UNKNOWN);
#endif /* !UNIV_LIBRARY */
  }

  /** @return true if the latch is for a temporary file space*/
  bool is_temp_fsp() const UNIV_NOTHROW { return (m_temp_fsp); }

  /** Set the temporary tablespace flag. The latch order constraints
  are different for intrinsic tables. We don't always acquire the
  index->lock. We need to figure out the context and add some special
  rules during the checks. */
  void set_temp_fsp() UNIV_NOTHROW {
    ut_ad(get_id() == LATCH_ID_FIL_SPACE);
    m_temp_fsp = true;
  }

  /** @return the latch name, m_id must be set  */
  const char *get_name() const UNIV_NOTHROW {
    ut_a(m_id != LATCH_ID_NONE);

#ifndef UNIV_LIBRARY
    return (sync_latch_get_name(m_id));
#else
    /* This should never be reached. This is
    added to fix compilation errors
    for library. We will never reach here because
    mutexes are disabled in library. */
    ut_error;
    return (NULL);
#endif /* !UNIV_LIBRARY */
  }

  /** Latch ID */
  latch_id_t m_id;

  /** true if it is a rw-lock. In debug mode, rw_lock_t derives from
  this class and sets this variable. */
  bool m_rw_lock;

  /** true if it is an temporary space latch */
  bool m_temp_fsp;
};

/** Subclass this to iterate over a thread's acquired latch levels. */
struct sync_check_functor_t {
  virtual ~sync_check_functor_t() {}
  virtual bool operator()(const latch_level_t) = 0;
  virtual bool result() const = 0;
};

/** Functor to check whether the calling thread owns the btr search mutex. */
struct btrsea_sync_check : public sync_check_functor_t {
  /** Constructor
  @param[in]	has_search_latch	true if owns the latch */
  explicit btrsea_sync_check(bool has_search_latch)
      : m_result(), m_has_search_latch(has_search_latch) {}

  /** Destructor */
  virtual ~btrsea_sync_check() {}

  /** Called for every latch owned by the calling thread.
  @param[in]	level		Level of the existing latch
  @return true if the predicate check fails */
  virtual bool operator()(const latch_level_t level) {
    /* If calling thread doesn't hold search latch then
    check if there are latch level exception provided.

    Note: Optimizer has added InnoDB intrinsic table as an
    alternative to MyISAM intrinsic table. With this a new
    control flow comes into existence, it is:

    Server -> Plugin -> SE

    Plugin in this case is I_S which is sharing the latch vector
    of InnoDB and so there could be lock conflicts. Ideally
    the Plugin should use a difference namespace latch vector
    as it doesn't have any depedency with SE latching protocol.

    Added check that will allow thread to hold I_S latches */

    if (!m_has_search_latch &&
        (level != SYNC_SEARCH_SYS && level != SYNC_DICT &&
         level != SYNC_FTS_CACHE && level != SYNC_UNDO_DDL &&
         level != SYNC_DICT_OPERATION && level != SYNC_TRX_I_S_LAST_READ &&
         level != SYNC_TRX_I_S_RWLOCK)) {
      m_result = true;
      ib::error() << "Debug: Calling thread does not hold search "
                     "latch but does hold latch level "
                  << level << ".";

      return (m_result);
    }

    return (false);
  }

  /** @return result from the check */
  virtual bool result() const { return (m_result); }

 private:
  /** True if all OK */
  bool m_result;

  /** If the caller owns the search latch */
  const bool m_has_search_latch;
};

/** Functor to check for dictionary latching constraints. */
struct dict_sync_check : public sync_check_functor_t {
  /** Constructor
  @param[in]	dict_mutex_allowed	true if the dict mutex
                                          is allowed */
  explicit dict_sync_check(bool dict_mutex_allowed)
      : m_result(), m_dict_mutex_allowed(dict_mutex_allowed) {}

  /** Destructor */
  virtual ~dict_sync_check() {}

  /** Check the latching constraints
  @param[in]	level		The level held by the thread */
  virtual bool operator()(const latch_level_t level) {
    if (!m_dict_mutex_allowed ||
        (level != SYNC_DICT && level != SYNC_UNDO_SPACES &&
         level != SYNC_FTS_CACHE && level != SYNC_DICT_OPERATION &&
         /* This only happens in recv_apply_hashed_log_recs. */
         level != SYNC_RECV_WRITER && level != SYNC_NO_ORDER_CHECK)) {
      m_result = true;
      ib::error() << "Debug: Dictionary latch order violation for level "
                  << level << ".";

      return (true);
    }

    return (false);
  }

  /** @return the result of the check */
  virtual bool result() const { return (m_result); }

 private:
  /** True if all OK */
  bool m_result;

  /** True if it is OK to hold the dict mutex */
  const bool m_dict_mutex_allowed;
};

/** Functor to check for given latching constraints. */
struct sync_allowed_latches : public sync_check_functor_t {
  /** Constructor
  @param[in]	from	first element in an array of latch_level_t
  @param[in]	to	last element in an array of latch_level_t */
  sync_allowed_latches(const latch_level_t *from, const latch_level_t *to)
      : m_result(), m_latches(from, to) {}

  /** Check whether the given latch_t violates the latch constraint.
  This object maintains a list of allowed latch levels, and if the given
  latch belongs to a latch level that is not there in the allowed list,
  then it is a violation.

  @param[in]	level	The latch level to check
  @return true if there is a latch ordering violation */
  virtual bool operator()(const latch_level_t level) {
    for (latches_t::const_iterator it = m_latches.begin();
         it != m_latches.end(); ++it) {
      if (level == *it) {
        m_result = false;

        /* No violation */
        return (m_result);
      }
    }

    ib::error() << "Debug: sync_allowed_latches violation for level=" << level;
    m_result = true;
    return (m_result);
  }

  /** @return the result of the check */
  virtual bool result() const { return (m_result); }

 private:
  /** Save the result of validation check here
  True if all OK */
  bool m_result;

  typedef std::vector<latch_level_t, ut_allocator<latch_level_t>> latches_t;

  /** List of latch levels that are allowed to be held */
  latches_t m_latches;
};

/** Get the latch id from a latch name.
@param[in]	name	Latch name
@return LATCH_ID_NONE. */
latch_id_t sync_latch_get_id(const char *name);

typedef ulint rw_lock_flags_t;

/* Flags to specify lock types for rw_lock_own_flagged() */
enum rw_lock_flag_t {
  RW_LOCK_FLAG_S = 1 << 0,
  RW_LOCK_FLAG_X = 1 << 1,
  RW_LOCK_FLAG_SX = 1 << 2
};

#endif /* UNIV_DBEUG */
