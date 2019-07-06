#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/LRUHp.h>

/** Special purpose iterators to be used when scanning the LRU list.
The idea is that when one thread finishes the scan it leaves the
itr in that position and the other thread can start scan from
there */
class LRUItr : public LRUHp {
 public:
  /** Constructor
  @param buf_pool buffer pool instance
  @param mutex	mutex that is protecting the hp. */
  LRUItr(const buf_pool_t *buf_pool, const ib_mutex_t *mutex)
      : LRUHp(buf_pool, mutex) {}

  /** Destructor */
  virtual ~LRUItr() {}

  /** Selects from where to start a scan. If we have scanned
  too deep into the LRU list it resets the value to the tail
  of the LRU list.
  @return buf_page_t from where to start scan. */
  buf_page_t *start();
};
