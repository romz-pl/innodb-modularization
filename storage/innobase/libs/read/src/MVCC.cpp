#include <innodb/read/MVCC.h>

#include <innodb/allocator/UT_DELETE.h>
#include <innodb/read/ReadView.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/logger/error.h>

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

