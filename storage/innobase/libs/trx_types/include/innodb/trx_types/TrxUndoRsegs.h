#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/Rseg_Iterator.h>

/** Rollback segements from a given transaction with trx-no
scheduled for purge. */
class TrxUndoRsegs {
 public:
  /** Default constructor */
  TrxUndoRsegs() : m_trx_no() {}

  explicit TrxUndoRsegs(trx_id_t trx_no) : m_trx_no(trx_no) {
    // Do nothing
  }

  /** Get transaction number
  @return trx_id_t - get transaction number. */
  trx_id_t get_trx_no() const { return (m_trx_no); }

  /** Add rollback segment.
  @param rseg rollback segment to add. */
  void push_back(trx_rseg_t *rseg) { m_rsegs.push_back(rseg); }

  /** Erase the element pointed by given iterator.
  @param[in]	it	iterator */
  void erase(Rseg_Iterator &it) { m_rsegs.erase(it); }

  /** Number of registered rsegs.
  @return size of rseg list. */
  ulint size() const { return (m_rsegs.size()); }

  /**
  @return an iterator to the first element */
  Rseg_Iterator begin() { return (m_rsegs.begin()); }

  /**
  @return an iterator to the end */
  Rseg_Iterator end() { return (m_rsegs.end()); }

  /** Append rollback segments from referred instance to current
  instance. */
  void append(const TrxUndoRsegs &append_from) {
    ut_ad(get_trx_no() == append_from.get_trx_no());

    m_rsegs.insert(m_rsegs.end(), append_from.m_rsegs.begin(),
                   append_from.m_rsegs.end());
  }

  /** Compare two TrxUndoRsegs based on trx_no.
  @param lhs first element to compare
  @param rhs second element to compare
  @return true if elem1 > elem2 else false.*/
  bool operator()(const TrxUndoRsegs &lhs, const TrxUndoRsegs &rhs) {
    return (lhs.m_trx_no > rhs.m_trx_no);
  }

  /** Compiler defined copy-constructor/assignment operator
  should be fine given that there is no reference to a memory
  object outside scope of class object.*/

 private:
  /** The rollback segments transaction number. */
  trx_id_t m_trx_no;

  /** Rollback segments of a transaction, scheduled for purge. */
  Rsegs_Vector m_rsegs;
};
