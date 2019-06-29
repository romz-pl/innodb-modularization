#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_id_t.h>

#ifndef UNIV_LIBRARY

/* Do nothing */
template <typename Mutex>
struct NoPolicy {
  /** Default constructor. */
  NoPolicy() {}

  void init(const Mutex &, latch_id_t, const char *, uint32_t) UNIV_NOTHROW {}
  void destroy() UNIV_NOTHROW {}
  void enter(const Mutex &, const char *, ulint line) UNIV_NOTHROW {}
  void add(uint32_t, uint32_t) UNIV_NOTHROW {}
  void locked(const Mutex &, const char *, ulint) UNIV_NOTHROW {}
  void release(const Mutex &) UNIV_NOTHROW {}
  std::string to_string() const { return (""); }
  latch_id_t get_id() const;
};

#endif /* UNIV_LIBRARY */
