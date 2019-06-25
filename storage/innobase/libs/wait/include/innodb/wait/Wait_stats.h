#pragma once

#include <innodb/univ/univ.h>

struct Wait_stats {
  uint64_t wait_loops;

  explicit Wait_stats(uint64_t wait_loops = 0) : wait_loops(wait_loops) {}

  Wait_stats &operator+=(const Wait_stats &rhs) {
    wait_loops += rhs.wait_loops;
    return (*this);
  }

  Wait_stats operator+(const Wait_stats &rhs) const {
    return (Wait_stats{wait_loops + rhs.wait_loops});
  }

  bool any_waits() const { return (wait_loops != 0); }
};
