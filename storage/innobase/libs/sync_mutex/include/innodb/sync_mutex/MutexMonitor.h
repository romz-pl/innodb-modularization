#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_latch/LatchMetaData.h>
#include <innodb/sync_latch/latch_meta.h>

#ifndef UNIV_LIBRARY

/** Iterate over the mutex meta data */
class MutexMonitor {
 public:
  /** Constructor */
  MutexMonitor() {}

  /** Destructor */
  ~MutexMonitor() {}

  /** Enable the mutex monitoring */
  void enable();

  /** Disable the mutex monitoring */
  void disable();

  /** Reset the mutex monitoring values */
  void reset();

  /** Invoke the callback for each active mutex collection
  @param[in,out]	callback	Functor to call
  @return false if callback returned false */
  template <typename Callback>
  bool iterate(Callback &callback) const UNIV_NOTHROW {
    LatchMetaData::iterator end = latch_meta.end();

    for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
      /* Some of the slots will be null in non-debug mode */

      if (*it == NULL) {
        continue;
      }

      latch_meta_t *latch_meta = *it;

      bool ret = callback(*latch_meta);

      if (!ret) {
        return (ret);
      }
    }

    return (true);
  }
};



#endif
