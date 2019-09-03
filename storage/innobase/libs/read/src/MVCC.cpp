#include <innodb/read/MVCC.h>

#include <innodb/allocator/UT_DELETE.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/logger/error.h>
#include <innodb/read/ReadView.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/trx_sys/trx_sys.h>
#include <innodb/trx_sys/trx_sys_get_max_trx_id.h>
#include <innodb/trx_sys/trx_sys_mutex_exit.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_sys/trx_sys_mutex_enter.h>

/** Constructor
@param size		Number of views to pre-allocate */
MVCC::MVCC(ulint size) {
  UT_LIST_INIT(m_free, &ReadView::m_view_list);
  UT_LIST_INIT(m_views, &ReadView::m_view_list);

  for (ulint i = 0; i < size; ++i) {
    ReadView *view = UT_NEW_NOKEY(ReadView());

    UT_LIST_ADD_FIRST(m_free, view);
  }
}

MVCC::~MVCC() {
  for (ReadView *view = UT_LIST_GET_FIRST(m_free); view != NULL;
       view = UT_LIST_GET_FIRST(m_free)) {
    UT_LIST_REMOVE(m_free, view);

    UT_DELETE(view);
  }

  ut_a(UT_LIST_GET_LEN(m_views) == 0);
}

#ifdef UNIV_DEBUG

/**
Validates a read view list. */
bool MVCC::validate() const {
  ViewCheck check;

  ut_ad(mutex_own(&trx_sys->mutex));

  ut_list_map(m_views, check);

  return (true);
}

#endif /* UNIV_DEBUG */

/**
Find a free view from the active list, if none found then allocate
a new view.
@return a view to use */

ReadView *MVCC::get_view() {
  ut_ad(mutex_own(&trx_sys->mutex));

  ReadView *view;

  if (UT_LIST_GET_LEN(m_free) > 0) {
    view = UT_LIST_GET_FIRST(m_free);
    UT_LIST_REMOVE(m_free, view);
  } else {
    view = UT_NEW_NOKEY(ReadView());

    if (view == NULL) {
      ib::error(ER_IB_MSG_918) << "Failed to allocate MVCC view";
    }
  }

  return (view);
}

/**
Release a view that is inactive but not closed. Caller must own
the trx_sys_t::mutex.
@param view		View to release */
void MVCC::view_release(ReadView *&view) {
  ut_ad(!srv_read_only_mode);
  ut_ad(trx_sys_mutex_own());

  uintptr_t p = reinterpret_cast<uintptr_t>(view);

  ut_a(p & 0x1);

  view = reinterpret_cast<ReadView *>(p & ~1);

  ut_ad(view->m_closed);

  /** RW transactions should not free their views here. Their views
  should freed using view_close_view() */

  ut_ad(view->m_creator_trx_id == 0);

  UT_LIST_REMOVE(m_views, view);

  UT_LIST_ADD_LAST(m_free, view);

  view = NULL;
}

ReadView *MVCC::get_view_created_by_trx_id(trx_id_t trx_id) const {
  ReadView *view;

  ut_ad(mutex_own(&trx_sys->mutex));

  for (view = UT_LIST_GET_LAST(m_views); view != NULL;
       view = UT_LIST_GET_PREV(m_view_list, view)) {
    if (view->is_closed()) {
      continue;
    }

    if (view->m_creator_trx_id == trx_id) {
      break;
    }
  }

  return (view);
}

/**
Get the oldest (active) view in the system.
@return oldest view if found or NULL */

ReadView *MVCC::get_oldest_view() const {
  ReadView *view;

  ut_ad(mutex_own(&trx_sys->mutex));

  for (view = UT_LIST_GET_LAST(m_views); view != NULL;
       view = UT_LIST_GET_PREV(m_view_list, view)) {
    if (!view->is_closed()) {
      break;
    }
  }

  return (view);
}

/**
Close a view created by the above function.
@param view		view allocated by trx_open.
@param own_mutex	true if caller owns trx_sys_t::mutex */

void MVCC::view_close(ReadView *&view, bool own_mutex) {
  uintptr_t p = reinterpret_cast<uintptr_t>(view);

  /* Note: The assumption here is that AC-NL-RO transactions will
  call this function with own_mutex == false. */
  if (!own_mutex) {
    /* Sanitise the pointer first. */
    ReadView *ptr = reinterpret_cast<ReadView *>(p & ~1);

    /* Note this can be called for a read view that
    was already closed. */
    ptr->m_closed = true;

    /* Set the view as closed. */
    view = reinterpret_cast<ReadView *>(p | 0x1);
  } else {
    view = reinterpret_cast<ReadView *>(p & ~1);

    view->close();

    UT_LIST_REMOVE(m_views, view);
    UT_LIST_ADD_LAST(m_free, view);

    ut_ad(validate());

    view = NULL;
  }
}

/**
Allocate and create a view.
@param view		view owned by this class created for the
                        caller. Must be freed by calling view_close()
@param trx		transaction instance of caller */
void MVCC::view_open(ReadView *&view, trx_t *trx) {
  ut_ad(!srv_read_only_mode);

  /** If no new RW transaction has been started since the last view
  was created then reuse the the existing view. */
  if (view != NULL) {
    uintptr_t p = reinterpret_cast<uintptr_t>(view);

    view = reinterpret_cast<ReadView *>(p & ~1);

    ut_ad(view->m_closed);

    /* NOTE: This can be optimised further, for now we only
    resuse the view iff there are no active RW transactions.

    There is an inherent race here between purge and this
    thread. Purge will skip views that are marked as closed.
    Therefore we must set the low limit id after we reset the
    closed status after the check. */

    if (trx_is_autocommit_non_locking(trx) && view->empty()) {
      view->m_closed = false;

      if (view->m_low_limit_id == trx_sys_get_max_trx_id()) {
        return;
      } else {
        view->m_closed = true;
      }
    }

    mutex_enter(&trx_sys->mutex);

    UT_LIST_REMOVE(m_views, view);

  } else {
    mutex_enter(&trx_sys->mutex);

    view = get_view();
  }

  if (view != NULL) {
    view->prepare(trx->id);

    UT_LIST_ADD_FIRST(m_views, view);

    ut_ad(!view->is_closed());

    ut_ad(validate());
  }

  trx_sys_mutex_exit();
}

/** Clones the oldest view and stores it in view. No need to
call view_close(). The caller owns the view that is passed in.
This function is called by Purge to determine whether it should
purge the delete marked record or not.
@param view		Preallocated view, owned by the caller */

void MVCC::clone_oldest_view(ReadView *view) {
  mutex_enter(&trx_sys->mutex);

  ReadView *oldest_view = get_oldest_view();

  if (oldest_view == NULL) {
    view->prepare(0);

    trx_sys_mutex_exit();

  } else {
    view->copy_prepare(*oldest_view);

    trx_sys_mutex_exit();

    view->copy_complete();
  }
}

/**
@return the number of active views */

ulint MVCC::size() const {
  trx_sys_mutex_enter();

  ulint size = 0;

  for (const ReadView *view = UT_LIST_GET_FIRST(m_views); view != NULL;
       view = UT_LIST_GET_NEXT(m_view_list, view)) {
    if (!view->is_closed()) {
      ++size;
    }
  }

  trx_sys_mutex_exit();

  return (size);
}


/**
Set the view creator transaction id. Note: This shouldbe set only
for views created by RW transactions.
@param view		Set the creator trx id for this view
@param id		Transaction id to set */

void MVCC::set_view_creator_trx_id(ReadView *view, trx_id_t id) {
  ut_ad(id > 0);
  ut_ad(mutex_own(&trx_sys->mutex));

  view->creator_trx_id(id);
}
