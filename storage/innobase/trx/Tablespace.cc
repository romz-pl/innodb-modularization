#include <innodb/trx_purge/Tablespace.h>

#include <innodb/trx_purge/id2num.h>
#include <innodb/trx_types/Rsegs.h>
#include <innodb/trx_purge/make_space_name.h>
#include <innodb/trx_purge/make_file_name.h>

#include "srv0srv.h"


namespace undo {

/** Constructor
@param[in]  id    tablespace id */
Tablespace::Tablespace(space_id_t id)
    : m_id(id),
      m_num(undo::id2num(id)),
      m_implicit(true),
      m_space_name(),
      m_file_name(),
      m_log_file_name(),
      m_rsegs() {}

/** Copy Constructor
@param[in]  other    undo tablespace to copy */
Tablespace::Tablespace(Tablespace &other)
    : m_id(other.id()),
      m_num(undo::id2num(other.id())),
      m_implicit(other.is_implicit()),
      m_space_name(),
      m_file_name(),
      m_log_file_name(),
      m_rsegs() {
  ut_ad(m_id == 0 || is_reserved(m_id));

  set_space_name(other.space_name());
  set_file_name(other.file_name());

  /* When the copy constructor is used, add an Rsegs
  vector. This constructor is only used in the global
  undo::Tablespaces object where rollback segments are
  tracked. */
  m_rsegs = UT_NEW_NOKEY(Rsegs());
}

/** Destructor */
Tablespace::~Tablespace() {
  if (m_space_name != nullptr) {
    ut_free(m_space_name);
    m_space_name = nullptr;
  }

  if (m_file_name != nullptr) {
    ut_free(m_file_name);
    m_file_name = nullptr;
  }

  if (m_log_file_name != nullptr) {
    ut_free(m_log_file_name);
    m_log_file_name = nullptr;
  }

  /* Clear the cached rollback segments.  */
  if (m_rsegs != nullptr) {
    UT_DELETE(m_rsegs);
    m_rsegs = nullptr;
  }
}

/* Determine if this undo space needs to be truncated.
@return true if it should be truncated, false if not. */
bool Tablespace::needs_truncation() {
  /* If it is already inactive, even implicitly, then proceed. */
  if (m_rsegs->is_inactive_implicit() || m_rsegs->is_inactive_explicit()) {
    return (true);
  }

  /* If implicit undo truncation is turned off, or if the rsegs don't exist
  yet, don't bother checking the size. */
  if (!srv_undo_log_truncate || m_rsegs == nullptr || m_rsegs->is_empty() ||
      m_rsegs->is_init()) {
    return (false);
  }

  ut_ad(m_rsegs->is_active());

  page_no_t trunc_size = std::max(
      static_cast<page_no_t>(srv_max_undo_tablespace_size / srv_page_size),
      static_cast<page_no_t>(SRV_UNDO_TABLESPACE_SIZE_IN_PAGES));
  if (fil_space_get_size(id()) > trunc_size) {
    return (true);
  }

  return (false);
}

/** Get the undo tablespace name. Make it if not yet made.
NOTE: This is only called from stack objects so there is no
race condition. If it is ever called from a shared object
like undo::spaces, then it must be protected by the caller.
@return tablespace name created from the space_id */
char *Tablespace::space_name() {
  if (m_space_name == nullptr) {
#ifndef UNIV_HOTBACKUP
    m_space_name = make_space_name(m_id);
#endif /* !UNIV_HOTBACKUP */
  }

  return (m_space_name);
}


/** Get the undo space filename. Make it if not yet made.
NOTE: This is only called from stack objects so there is no
race condition. If it is ever called from a shared object
like undo::spaces, then it must be protected by the caller.
@return tablespace filename created from the space_id */
char *Tablespace::file_name() {
  if (m_file_name == nullptr) {
    m_file_name = make_file_name(m_id);
  }

  return (m_file_name);
}


/** Get the undo log filename. Make it if not yet made.
NOTE: This is only called from stack objects so there is no
race condition. If it is ever called from a shared object
like undo::spaces, then it must be protected by the caller.
@return tablespace filename created from the space_id */
char *Tablespace::log_file_name() {
  if (m_log_file_name == nullptr) {
    m_log_file_name = make_log_file_name(m_id);
  }

  return (m_log_file_name);
}

/** Get the undo tablespace ID.
@return tablespace ID */
space_id_t Tablespace::id() { return (m_id); }

/** Get the undo tablespace number.  This is the same as m_id
if m_id is 0 or this is a v5.6-5.7 undo tablespace. v8+ undo
tablespaces use a space_id from the reserved range.
@return undo tablespace number */
space_id_t Tablespace::num() {
  ut_ad(m_num < FSP_MAX_ROLLBACK_SEGMENTS);

  return (m_num);
}

/** Get a reference to the List of rollback segments within
this undo tablespace.
@return a reference to the Rsegs vector. */
Rsegs *Tablespace::rsegs() { return (m_rsegs); }

/** Report whether this undo tablespace was explicitly created
by an SQL statement.
@return true if the tablespace was created explicitly. */
bool Tablespace::is_explicit() { return (!m_implicit); }

/** Report whether this undo tablespace was implicitly created
at startup.
@return true if the tablespace was created implicitly. */
bool Tablespace::is_implicit() { return (m_implicit); }

/** Return whether the undo tablespace is active.
@return true if active */
bool Tablespace::is_active() {
  if (m_rsegs == nullptr) {
    return (false);
  }
  m_rsegs->s_lock();
  bool ret = m_rsegs->is_active();
  m_rsegs->s_unlock();
  return (ret);
}

/** Return whether the undo tablespace is active. For optimization purposes,
do not take a latch.
@return true if active */
bool Tablespace::is_active_no_latch() {
  if (m_rsegs == nullptr) {
    return (false);
  }
  return (m_rsegs->is_active());
}

/** Return the rseg at the requested rseg slot if the undo space is active.
@param[in] slot   The slot of the rseg.  1 to 127
@return Rseg pointer of nullptr if the space is not active. */
trx_rseg_t *Tablespace::get_active(ulint slot) {
  m_rsegs->s_lock();
  if (!m_rsegs->is_active()) {
    m_rsegs->s_unlock();
    return (nullptr);
  }

  /* Mark the chosen rseg so that it will not be selected
  for UNDO truncation. */
  trx_rseg_t *rseg = m_rsegs->at(slot);
  rseg->trx_ref_count++;

  m_rsegs->s_unlock();

  return (rseg);
}

/** Return whether the undo tablespace is inactive due to
implicit selection by the purge thread.
@return true if marked for truncation by the purge thread */
bool Tablespace::is_inactive_implicit() {
  if (m_rsegs == nullptr) {
    return (false);
  }
  m_rsegs->s_lock();
  bool ret = m_rsegs->is_inactive_implicit();
  m_rsegs->s_unlock();
  return (ret);
}

/** Return whether the undo tablespace was made inactive by
ALTER TABLESPACE.
@return true if altered inactive */
bool Tablespace::is_inactive_explicit() {
  if (m_rsegs == nullptr) {
    return (false);
  }
  m_rsegs->s_lock();
  bool ret = m_rsegs->is_inactive_explicit();
  m_rsegs->s_unlock();
  return (ret);
}

/** Return whether the undo tablespace is empty and ready
to be dropped.
@return true if empty */
bool Tablespace::is_empty() {
  if (m_rsegs == nullptr) {
    return (true);
  }
  m_rsegs->s_lock();
  bool ret = m_rsegs->is_empty();
  m_rsegs->s_unlock();
  return (ret);
}

/** Set the undo tablespace active for use by transactions. */
void Tablespace::set_active() {
  m_rsegs->x_lock();
  m_rsegs->set_active();
  m_rsegs->x_unlock();
}

/** Set the state of the rollback segments in this undo tablespace to
inactive_implicit if currently active.  If the state is inactive_explicit,
leave as is. Then put the space_id into the callers marked_space_id.
This is done when marking a space for truncate.  It will not be used
for new transactions until it becomes active again. */
void Tablespace::set_inactive_implicit(space_id_t *marked_space_id) {
  m_rsegs->x_lock();
  if (m_rsegs->is_active()) {
    m_rsegs->set_inactive_implicit();
  }
  *marked_space_id = m_id;

  m_rsegs->x_unlock();
}

/** Make the undo tablespace inactive so that it will not be
used for new transactions.  The purge thread will clear out
all the undo logs, truncate it, and then mark it empty. */
void Tablespace::set_inactive_explicit() {
  m_rsegs->x_lock();
  if (m_rsegs->is_active() || m_rsegs->is_inactive_implicit()) {
    m_rsegs->set_inactive_explicit();
  }
  m_rsegs->x_unlock();
}


/** Set the state of the undo tablespace to empty so that it
can be dropped. */
void Tablespace::set_empty() {
  m_rsegs->x_lock();
  m_rsegs->set_empty();
  m_rsegs->x_unlock();
}




}
