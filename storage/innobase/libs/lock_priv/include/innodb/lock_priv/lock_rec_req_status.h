#pragma once

#include <innodb/univ/univ.h>

/** Record locking request status */
enum lock_rec_req_status {
  /** Failed to acquire a lock */
  LOCK_REC_FAIL,
  /** Succeeded in acquiring a lock (implicit or already acquired) */
  LOCK_REC_SUCCESS,
  /** Explicitly created a new lock */
  LOCK_REC_SUCCESS_CREATED
};
