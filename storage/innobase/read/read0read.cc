/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file read/read0read.cc
 Cursor read

 Created 2/16/1997 Heikki Tuuri
 *******************************************************/

#include <innodb/read/ReadView.h>
#include <innodb/read/MVCC.h>
#include <innodb/read/ViewCheck.h>


#include "srv0srv.h"
#include "trx0sys.h"
















/**
Opens a read view where exactly the transactions serialized before this
point in time are seen in the view.
@param id		Creator transaction id */

void ReadView::prepare(trx_id_t id) {
  ut_ad(mutex_own(&trx_sys->mutex));

  m_creator_trx_id = id;

  m_low_limit_no = m_low_limit_id = m_up_limit_id = trx_sys->max_trx_id;

  if (!trx_sys->rw_trx_ids.empty()) {
    copy_trx_ids(trx_sys->rw_trx_ids);
  } else {
    m_ids.clear();
  }

  ut_ad(m_up_limit_id <= m_low_limit_id);

  if (UT_LIST_GET_LEN(trx_sys->serialisation_list) > 0) {
    const trx_t *trx;

    trx = UT_LIST_GET_FIRST(trx_sys->serialisation_list);

    if (trx->no < m_low_limit_no) {
      m_low_limit_no = trx->no;
    }
  }

  m_closed = false;
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
