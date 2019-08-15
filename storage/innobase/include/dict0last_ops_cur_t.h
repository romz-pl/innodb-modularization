#pragma once

#include <innodb/mtr/mtr_commit.h>

#include <innodb/univ/rec_t.h>
#include <innodb/mtr/mtr_t.h>

struct buf_block_t;

/** Cache position of last inserted or selected record by caching record
and holding reference to the block where record resides.
Note: We don't commit mtr and hold it beyond a transaction lifetime as this is
a special case (intrinsic table) that are not shared accross connection. */
class last_ops_cur_t {
 public:
  /** Constructor */
  last_ops_cur_t() : rec(), block(), mtr(), disable_caching(), invalid() {
    /* Do Nothing. */
  }

  /* Commit mtr and re-initialize cache record and block to NULL. */
  void release() {
    if (mtr.is_active()) {
      mtr_commit(&mtr);
    }
    rec = NULL;
    block = NULL;
    invalid = false;
  }

 public:
  /** last inserted/selected record. */
  rec_t *rec;

  /** block where record reside. */
  buf_block_t *block;

  /** active mtr that will be re-used for next insert/select. */
  mtr_t mtr;

  /** disable caching. (disabled when table involves blob/text.) */
  bool disable_caching;

  /** If index structure is undergoing structural change viz.
  split then invalidate the cached position as it would be no more
  remain valid. Will be re-cached on post-split insert. */
  bool invalid;
};

