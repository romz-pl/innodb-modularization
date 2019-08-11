#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_pool/HazardPointer.h>

/** Class implementing buf_pool->flush_list hazard pointer */
class FlushHp : public HazardPointer {
 public:
  /** Constructor
  @param buf_pool buffer pool instance
  @param mutex	mutex that is protecting the hp. */
  FlushHp(const buf_pool_t *buf_pool, const ib_mutex_t *mutex)
      : HazardPointer(buf_pool, mutex) {}

  /** Destructor */
  virtual ~FlushHp() {}

  /** Adjust the value of hp. This happens when some
  other thread working on the same list attempts to
  remove the hp from the list.
  @param bpage	buffer block to be compared */
  void adjust(const buf_page_t *bpage);
};
