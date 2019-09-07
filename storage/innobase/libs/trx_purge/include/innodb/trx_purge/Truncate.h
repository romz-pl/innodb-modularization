#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_purge/Tablespace.h>
#include <innodb/trx_purge/srv_purge_rseg_truncate_frequency.h>
#include <innodb/trx_purge/id2num.h>
#include <innodb/trx_purge/spaces.h>

namespace undo {


/** Track an UNDO tablespace marked for truncate. */
class Truncate {
 public:
  Truncate()
      : m_space_id_marked(SPACE_UNKNOWN),
        m_purge_rseg_truncate_frequency(
            static_cast<ulint>(srv_purge_rseg_truncate_frequency)) {
    /* Do Nothing. */
  }

  /** Is tablespace selected for truncate.
  @return true if undo tablespace is marked for truncate */
  bool is_marked() const { return (m_space_id_marked != SPACE_UNKNOWN); }

  /** Mark the undo tablespace selected for truncate as empty
  so that it will be truncated next. */
  void set_marked_space_empty() { m_marked_space_is_empty = true; }

  /** Is tablespace selected for truncate empty of undo logs yet?
  @return true if the marked undo tablespace has no more undo logs */
  bool is_marked_space_empty() const { return (m_marked_space_is_empty); }

  /** Mark the tablespace for truncate.
  @param[in]  undo_space  undo tablespace to truncate. */
  void mark(Tablespace *undo_space) {
    /* Set the internal state of this undo space to inactive_implicit
    so that its rsegs will not be allocated to any new transaction.
    If the space is already in the inactive_explicit state, it will
    stay there.
    Note that the DD is not modified since in case of crash, the
    action must be completed before the DD is available.
    Set both the state and this marked id while this routine has
    an x_lock on m_rsegs because a concurrent user thread might issue
    undo_space->alter_active(). */
    undo_space->set_inactive_implicit(&m_space_id_marked);

    m_marked_space_is_empty = false;

    /* We found an UNDO-tablespace to truncate so set the
    local purge rseg truncate frequency to 3. This will help
    accelerate the purge action and in turn truncate. */
    set_rseg_truncate_frequency(3);
  }

  /** Get the ID of the tablespace marked for truncate.
  @return tablespace ID marked for truncate. */
  space_id_t get_marked_space_num() const {
    return (id2num(m_space_id_marked));
  }

  /** Reset for next rseg truncate. */
  void reset() {
    /* Sync with global value as we are done with
    truncate now. */
    set_rseg_truncate_frequency(
        static_cast<ulint>(srv_purge_rseg_truncate_frequency));

    m_marked_space_is_empty = false;
    m_space_id_marked = SPACE_UNKNOWN;
  }

  /** Get the undo tablespace number to start a scan.
  Re-adjust in case the spaces::size() went down.
  @return	UNDO space_num to start scanning. */
  space_id_t get_scan_space_num() const {
    s_scan_pos = s_scan_pos % undo::spaces->size();

    Tablespace *undo_space = undo::spaces->at(s_scan_pos);

    return (undo_space->num());
  }

  /** Increment the scanning position in a round-robin fashion.
  @return	UNDO space_num at incremented scanning position. */
  space_id_t increment_scan() const {
    /** Round-robin way of selecting an undo tablespace
    for the truncate operation. Once we reach the end of
    the list of known undo tablespace IDs, move back to
    the first undo tablespace ID. This will scan active
    as well as inactive undo tablespaces. */
    s_scan_pos = (s_scan_pos + 1) % undo::spaces->size();

    return (get_scan_space_num());
  }

  /** Get local rseg purge truncate frequency
  @return rseg purge truncate frequency. */
  ulint get_rseg_truncate_frequency() const {
    return (m_purge_rseg_truncate_frequency);
  }

  /** Set local rseg purge truncate frequency */
  void set_rseg_truncate_frequency(ulint frequency) {
    m_purge_rseg_truncate_frequency = frequency;
  }

 private:
  /** UNDO space ID that is marked for truncate. */
  space_id_t m_space_id_marked;

  /** This is true if the marked space is empty of undo logs
  and ready to truncate.  We leave the rsegs object 'inactive'
  until after it is truncated and rebuilt.  This allow the
  code to do the check for undo logs only once. */
  bool m_marked_space_is_empty;

  /** Rollback segment(s) purge frequency. This is a local
  value maintained along with the global value. It is set
  to the global value in the before each truncate.  But when
  a tablespace is marked for truncate it is updated to 1 and
  then minimum value among 2 is used by the purge action. */
  ulint m_purge_rseg_truncate_frequency;

  /** Start scanning for UNDO tablespace from this
  vector position. This is to avoid bias selection
  of one tablespace always. */
  static size_t s_scan_pos;

}; /* class Truncate */


}
