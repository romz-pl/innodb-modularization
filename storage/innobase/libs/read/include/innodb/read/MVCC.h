#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>

struct trx_t;
class ReadView;

/** The MVCC read view manager */
class MVCC {
 public:
  /** Constructor
  @param size		Number of views to pre-allocate */
  explicit MVCC(ulint size);

  /** Destructor.
  Free all the views in the m_free list */
  ~MVCC();

  /**
  Allocate and create a view.
  @param view		view owned by this class created for the
                          caller. Must be freed by calling close()
  @param trx		transaction creating the view */
  void view_open(ReadView *&view, trx_t *trx);

  /**
  Close a view created by the above function.
  @param view		view allocated by trx_open.
  @param own_mutex	true if caller owns trx_sys_t::mutex */
  void view_close(ReadView *&view, bool own_mutex);

  /**
  Release a view that is inactive but not closed. Caller must own
  the trx_sys_t::mutex.
  @param view		View to release */
  void view_release(ReadView *&view);

  /** Clones the oldest view and stores it in view. No need to
  call view_close(). The caller owns the view that is passed in.
  It will also move the closed views from the m_views list to the
  m_free list. This function is called by Purge to create it view.
  @param view		Preallocated view, owned by the caller */
  void clone_oldest_view(ReadView *view);

  /**
  @return the number of active views */
  ulint size() const;

  /**
  @return true if the view is active and valid */
  static bool is_view_active(ReadView *view) {
    ut_a(view != reinterpret_cast<ReadView *>(0x1));

    return (view != NULL && !(intptr_t(view) & 0x1));
  }

  /**
  Set the view creator transaction id. Note: This shouldbe set only
  for views created by RW transactions. */
  static void set_view_creator_trx_id(ReadView *view, trx_id_t id);

 private:
  /**
  Validates a read view list. */
  bool validate() const;

  /**
  Find a free view from the active list, if none found then allocate
  a new view. This function will also attempt to move delete marked
  views from the active list to the freed list.
  @return a view to use */
  ReadView *get_view();

  /**
  Get the oldest view in the system. It will also move the delete
  marked read views from the views list to the freed list.
  @return oldest view if found or NULL */
  ReadView *get_oldest_view() const;
  ReadView *get_view_created_by_trx_id(trx_id_t trx_id) const;

 private:
  // Prevent copying
  MVCC(const MVCC &);
  MVCC &operator=(const MVCC &);

 private:
  typedef UT_LIST_BASE_NODE_T(ReadView) view_list_t;

  /** Free views ready for reuse. */
  view_list_t m_free;

  /** Active and closed views, the closed views will have the
  creator trx id set to TRX_ID_MAX */
  view_list_t m_views;
};
