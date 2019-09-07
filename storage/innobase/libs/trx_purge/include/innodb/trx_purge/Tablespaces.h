#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_purge/Tablespace.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_s_lock.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
namespace undo {

/** List of undo tablespaces, each containing a list of
rollback segments. */
class Tablespaces {
  using Tablespaces_Vector =
      std::vector<Tablespace *, ut_allocator<Tablespace *>>;

 public:
  Tablespaces() { init(); }

  ~Tablespaces() { deinit(); }

  /** Initialize */
  void init();

  /** De-initialize */
  void deinit();

  /** Clear the contents of the list of Tablespace objects.
  This does not deallocate any memory. */
  void clear() {
    for (auto undo_space : m_spaces) {
      UT_DELETE(undo_space);
    }
    m_spaces.clear();
  }

  /** Get the number of tablespaces tracked by this object. */
  ulint size() { return (m_spaces.size()); }

  /** See if the list of tablespaces is empty. */
  bool empty() { return (m_spaces.empty()); }

  /** Get the Tablespace tracked at a position. */
  Tablespace *at(size_t pos) { return (m_spaces.at(pos)); }

  /** Add a new undo::Tablespace to the back of the vector.
  The vector has been pre-allocated to 128 so read threads will
  not loose what is pointed to. If tablespace_name and file_name
  are standard names, they are optional.
  @param[in]	ref_undo_space	undo tablespace */
  void add(Tablespace &ref_undo_space);

  /** Drop an existing explicit undo::Tablespace.
  @param[in]	undo_space	pointer to undo space */
  void drop(Tablespace *undo_space);

  /** Drop an existing explicit undo::Tablespace.
  @param[in]	ref_undo_space	reference to undo space */
  void drop(Tablespace &ref_undo_space);

  /** Check if the given space_id is in the vector.
  @param[in]  num  undo tablespace number
  @return true if space_id is found, else false */
  bool contains(space_id_t num) { return (find(num) != nullptr); }

  /** Find the given space_num in the vector.
  @param[in]  num  undo tablespace number
  @return pointer to an undo::Tablespace struct */
  Tablespace *find(space_id_t num) {
    if (m_spaces.empty()) {
      return (nullptr);
    }

    /* The sort method above puts this vector in order by
    Tablespace::num. If there are no gaps, then we should
    be able to find it quickly. */
    space_id_t slot = num - 1;
    if (slot < m_spaces.size()) {
      auto undo_space = m_spaces.at(slot);
      if (undo_space->num() == num) {
        return (undo_space);
      }
    }

    /* If there are gaps in the numbering, do a search. */
    for (auto undo_space : m_spaces) {
      if (undo_space->num() == num) {
        return (undo_space);
      }
    }

    return (nullptr);
  }

#ifdef UNIV_DEBUG
  /** Determine if this thread owns a lock on m_latch. */
  bool own_latch() {
    return (rw_lock_own(m_latch, RW_LOCK_X) || rw_lock_own(m_latch, RW_LOCK_S));
  }
#endif /* UNIV_DEBUG */

  /** Get a shared lock on m_spaces. */
  void s_lock() { rw_lock_s_lock(m_latch); }

  /** Release a shared lock on m_spaces. */
  void s_unlock() { rw_lock_s_unlock(m_latch); }

  /** Get an exclusive lock on m_spaces. */
  void x_lock() { rw_lock_x_lock(m_latch); }

  /** Release an exclusive lock on m_spaces. */
  void x_unlock() { rw_lock_x_unlock(m_latch); }

  Tablespaces_Vector m_spaces;

 private:
  /** RW lock to protect m_spaces.
  x for adding elements, s for scanning, size() etc. */
  rw_lock_t *m_latch;
};



}
