#pragma once

#include <innodb/univ/univ.h>


#ifdef UNIV_DEBUG

/** Functor to validate the view list. */
struct ViewCheck {
  ViewCheck() : m_prev_view() {}

  void operator()(const ReadView *view) {
    ut_a(m_prev_view == NULL || view->is_closed() || view->le(m_prev_view));

    m_prev_view = view;
  }

  const ReadView *m_prev_view;
};


#endif
