#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_rseg_t.h>

class Rsegs;

namespace undo {


/** An undo::Tablespace object is used to easily convert between
undo_space_id and undo_space_num and to create the automatic file_name
and space name.  In addition, it is used in undo::Tablespaces to track
the trx_rseg_t objects in an Rsegs vector. So we do not allocate the
Rsegs vector for each object, only when requested by the constructor. */
struct Tablespace {
  /** Constructor
  @param[in]  id    tablespace id */
  explicit Tablespace(space_id_t id);

  /** Copy Constructor
  @param[in]  other    undo tablespace to copy */
  Tablespace(Tablespace &other);

  /** Destructor */
  ~Tablespace();

  /* Determine if this undo space needs to be truncated.
  @return true if it should be truncated, false if not. */
  bool needs_truncation();

  /** Change the space_id from its current value.
  @param[in]  space_id  The new undo tablespace ID */
  void set_space_id(space_id_t space_id);

  /** Replace the standard undo space name if it exists with a copy
  of the undo tablespace name provided.
  @param[in]  new_space_name  non-standard undo space name */
  void set_space_name(const char *new_space_name);

  /** Get the undo tablespace name. Make it if not yet made.
  NOTE: This is only called from stack objects so there is no
  race condition. If it is ever called from a shared object
  like undo::spaces, then it must be protected by the caller.
  @return tablespace name created from the space_id */
  char *space_name();

  /** Replace the standard undo file name if it exists with a copy
  of the file name provided. This name can come in three forms:
  absolute path, relative path, and basename.  Undo ADD DATAFILE
  does not accept a relative path.  So if that comes in here, it
  was the scaneed name and is relative to the datadir.
  If this is just a basename, add it to srv_undo_dir.
  @param[in]  file_name  explicit undo file name */
  void set_file_name(const char *file_name);

  /** Get the undo space filename. Make it if not yet made.
  NOTE: This is only called from stack objects so there is no
  race condition. If it is ever called from a shared object
  like undo::spaces, then it must be protected by the caller.
  @return tablespace filename created from the space_id */
  char *file_name();

  /** Build a log file name based on space_id
  @param[in]	space_id	id of the undo tablespace.
  @return DB_SUCCESS or error code */
  char *make_log_file_name(space_id_t space_id);

  /** Get the undo log filename. Make it if not yet made.
  NOTE: This is only called from stack objects so there is no
  race condition. If it is ever called from a shared object
  like undo::spaces, then it must be protected by the caller.
  @return tablespace filename created from the space_id */
  char *log_file_name();

  /** Get the undo tablespace ID.
  @return tablespace ID */
  space_id_t id();

  /** Get the undo tablespace number.  This is the same as m_id
  if m_id is 0 or this is a v5.6-5.7 undo tablespace. v8+ undo
  tablespaces use a space_id from the reserved range.
  @return undo tablespace number */
  space_id_t num();

  /** Get a reference to the List of rollback segments within
  this undo tablespace.
  @return a reference to the Rsegs vector. */
  Rsegs *rsegs();

  /** Report whether this undo tablespace was explicitly created
  by an SQL statement.
  @return true if the tablespace was created explicitly. */
  bool is_explicit();

  /** Report whether this undo tablespace was implicitly created
  at startup.
  @return true if the tablespace was created implicitly. */
  bool is_implicit();

  /** Return whether the undo tablespace is active.
  @return true if active */
  bool is_active();

  /** Return whether the undo tablespace is active. For optimization purposes,
  do not take a latch.
  @return true if active */
  bool is_active_no_latch();

  /** Return the rseg at the requested rseg slot if the undo space is active.
  @param[in] slot   The slot of the rseg.  1 to 127
  @return Rseg pointer of nullptr if the space is not active. */
  trx_rseg_t *get_active(ulint slot);

  /** Return whether the undo tablespace is inactive due to
  implicit selection by the purge thread.
  @return true if marked for truncation by the purge thread */
  bool is_inactive_implicit();

  /** Return whether the undo tablespace was made inactive by
  ALTER TABLESPACE.
  @return true if altered inactive */
  bool is_inactive_explicit();

  /** Return whether the undo tablespace is empty and ready
  to be dropped.
  @return true if empty */
  bool is_empty();

  /** Set the undo tablespace active for use by transactions. */
  void set_active();

  /** Set the state of the rollback segments in this undo tablespace to
  inactive_implicit if currently active.  If the state is inactive_explicit,
  leave as is. Then put the space_id into the callers marked_space_id.
  This is done when marking a space for truncate.  It will not be used
  for new transactions until it becomes active again. */
  void set_inactive_implicit(space_id_t *marked_space_id);

  /** Make the undo tablespace inactive so that it will not be
  used for new transactions.  The purge thread will clear out
  all the undo logs, truncate it, and then mark it empty. */
  void set_inactive_explicit();

  /** Make the undo tablespace active again so that it will
  be used for new transactions.
  If current State is ___ then do:
  empty:            Set active.
  active_implicit:  Ignore.  It was not altered inactive. When it is done
                    being truncated it will go back to active.
  active_explicit:  Depends if it is marked for truncation.
    marked:         Set to inactive_implicit. the next state will be active.
    not yet:        Set to active so that it does not get truncated.  */
  void alter_active();

  /** Set the state of the undo tablespace to empty so that it
  can be dropped. */
  void set_empty();

 private:
  /** Undo Tablespace ID. */
  space_id_t m_id;

  /** Undo Tablespace number, from 1 to 127. This is the
  7-bit number that is used in a rollback pointer.
  Use id2num() to get this number from a space_id. */
  space_id_t m_num;

  /* True if this is an implicit undo tablespace */
  bool m_implicit;

  /** The tablespace name, auto-generated when needed from
  the space number. */
  char *m_space_name;

  /** The tablespace file name, auto-generated when needed
  from the space number. */
  char *m_file_name;

  /** The tablespace log file name, auto-generated when needed
  from the space number. */
  char *m_log_file_name;

  /** List of rollback segments within this tablespace.
  This is not always used. Must call init_rsegs to use it. */
  Rsegs *m_rsegs;
};



}
