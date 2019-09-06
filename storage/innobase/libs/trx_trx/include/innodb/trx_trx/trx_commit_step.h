#pragma once

#include <innodb/univ/univ.h>

struct que_thr_t;

/** Performs an execution step for a commit type node in a query graph.
 @return query thread to run next, or NULL */
que_thr_t *trx_commit_step(que_thr_t *thr); /*!< in: query thread */
