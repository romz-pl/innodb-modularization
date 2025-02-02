#pragma once

/*****************************************************************************

Copyright (c) 2012, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/ut0counter.h

 Counter utility class

 Created 2012/04/12 by Sunny Bains
 *******************************************************/


#include <innodb/univ/univ.h>

#include <innodb/thread/os_thread_get_curr_id.h>
#include <innodb/error/ut_error.h>
#include <innodb/cpu/flags.h>

#include <my_rdtsc.h>
#include <array>
#include <atomic>



/** Default number of slots to use in ib_counter_t */
#define IB_N_SLOTS 64

/** Get the offset into the counter array. */
template <typename Type, int N>
struct generic_indexer_t {
  /** Default constructor/destructor should be OK. */

  /** @return offset within m_counter */
  static size_t offset(size_t index) UNIV_NOTHROW {
    return (((index % N) + 1) * (INNOBASE_CACHE_LINE_SIZE / sizeof(Type)));
  }
};

/** Use the result of my_timer_cycles(), which mainly uses RDTSC for cycles,
to index into the counter array. See the comments for my_timer_cycles() */
template <typename Type = ulint, int N = 1>
struct counter_indexer_t : public generic_indexer_t<Type, N> {
  /** Default constructor/destructor should be OK. */

  enum { fast = 1 };

  /** @return result from RDTSC or similar functions. */
  static size_t get_rnd_index() UNIV_NOTHROW {
    size_t c = static_cast<size_t>(my_timer_cycles());

    if (c != 0) {
      return (c);
    } else {
    /* We may go here if my_timer_cycles() returns 0,
    so we have to have the plan B for the counter. */
#if !defined(_WIN32)
      return (size_t(os_thread_get_curr_id()));
#else
      LARGE_INTEGER cnt;
      QueryPerformanceCounter(&cnt);

      return (static_cast<size_t>(cnt.QuadPart));
#endif /* !_WIN32 */
    }
  }
};

/** For counters where N=1 */
template <typename Type = ulint, int N = 1>
struct single_indexer_t {
  /** Default constructor/destructor should are OK. */

  enum { fast = 0 };

  /** @return offset within m_counter */
  static size_t offset(size_t index) UNIV_NOTHROW {
    ut_ad(N == 1);
    return ((INNOBASE_CACHE_LINE_SIZE / sizeof(Type)));
  }

  /** @return 1 */
  static size_t get_rnd_index() UNIV_NOTHROW {
    ut_ad(N == 1);
    return (1);
  }
};

#define default_indexer_t counter_indexer_t

/** Class for using fuzzy counters. The counter is not protected by any
mutex and the results are not guaranteed to be 100% accurate but close
enough. Creates an array of counters and separates each element by the
INNOBASE_CACHE_LINE_SIZE bytes */
template <typename Type, int N = IB_N_SLOTS,
          template <typename, int> class Indexer = default_indexer_t>
class ib_counter_t {
 public:
  ib_counter_t() { memset(m_counter, 0x0, sizeof(m_counter)); }

  ~ib_counter_t() { ut_ad(validate()); }

  static bool is_fast() { return (Indexer<Type, N>::fast); }

  bool validate() UNIV_NOTHROW {
#ifdef UNIV_DEBUG
    size_t n = (INNOBASE_CACHE_LINE_SIZE / sizeof(Type));

    /* Check that we aren't writing outside our defined bounds. */
    for (size_t i = 0; i < UT_ARR_SIZE(m_counter); i += n) {
      for (size_t j = 1; j < n - 1; ++j) {
        ut_ad(m_counter[i + j] == 0);
      }
    }
#endif /* UNIV_DEBUG */
    return (true);
  }

  /** If you can't use a good index id. Increment by 1. */
  void inc() UNIV_NOTHROW { add(1); }

  /** If you can't use a good index id.
  @param n is the amount to increment */
  void add(Type n) UNIV_NOTHROW {
    size_t i = m_policy.offset(m_policy.get_rnd_index());

    ut_ad(i < UT_ARR_SIZE(m_counter));

    m_counter[i] += n;
  }

  /** Use this if you can use a unique identifier, saves a
  call to get_rnd_index().
  @param	index	index into a slot
  @param	n	amount to increment */
  void add(size_t index, Type n) UNIV_NOTHROW {
    size_t i = m_policy.offset(index);

    ut_ad(i < UT_ARR_SIZE(m_counter));

    m_counter[i] += n;
  }

  /** If you can't use a good index id. Decrement by 1. */
  void dec() UNIV_NOTHROW { sub(1); }

  /** If you can't use a good index id.
  @param n the amount to decrement */
  void sub(Type n) UNIV_NOTHROW {
    size_t i = m_policy.offset(m_policy.get_rnd_index());

    ut_ad(i < UT_ARR_SIZE(m_counter));

    m_counter[i] -= n;
  }

  /** Use this if you can use a unique identifier, saves a
  call to get_rnd_index().
  @param	index	index into a slot
  @param	n	amount to decrement */
  void sub(size_t index, Type n) UNIV_NOTHROW {
    size_t i = m_policy.offset(index);

    ut_ad(i < UT_ARR_SIZE(m_counter));

    m_counter[i] -= n;
  }

  /* @return total value - not 100% accurate, since it is not atomic. */
  operator Type() const UNIV_NOTHROW {
    Type total = 0;

    for (size_t i = 0; i < N; ++i) {
      total += m_counter[m_policy.offset(i)];
    }

    return (total);
  }

  Type operator[](size_t index) const UNIV_NOTHROW {
    size_t i = m_policy.offset(index);

    ut_ad(i < UT_ARR_SIZE(m_counter));

    return (m_counter[i]);
  }

 private:
  /** Indexer into the array */
  Indexer<Type, N> m_policy;

  /** Slot 0 is unused. */
  Type m_counter[(N + 1) * (INNOBASE_CACHE_LINE_SIZE / sizeof(Type))];
};

/** Sharded atomic counter. */
namespace Counter {

using N = std::atomic<size_t>;

static_assert(INNOBASE_CACHE_LINE_SIZE >= sizeof(N),
              "Atomic counter size > INNOBASE_CACHE_LINE_SIZE");

using Pad = byte[INNOBASE_CACHE_LINE_SIZE - sizeof(N)];

/** Counter shard. */
struct Shard {
  /** Separate on cache line. */
  Pad m_pad;

  /** Sharded counter. */
  N m_n{};
};

using Shards = std::array<Shard, 128>;

/** Increment the counter.
@param[in,out]  shards          Sharded counter to increment.
@param[in] id                   Shard key. */
inline void inc(Shards &shards, size_t id) {
  shards[id % shards.size()].m_n.fetch_add(1, std::memory_order_relaxed);
}

/** Get the total value of all shards.
@param[in] shards               Shards to sum.
@return total value. */
inline size_t total(const Shards &shards) {
  size_t n = 0;

  for (const auto &shard : shards) {
    n += shard.m_n;
  }

  return (n);
}

/** Clear the counter - reset to 0.
@param[in,out] shards          Shards to clear. */
inline void clear(Shards &shards) {
  for (auto &shard : shards) {
    shard.m_n.store(0, std::memory_order_relaxed);
  }
}

}  // namespace Counter


