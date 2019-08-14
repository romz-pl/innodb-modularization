#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxUndoRsegs.h>

#include <queue>

typedef std::priority_queue<
    TrxUndoRsegs, std::vector<TrxUndoRsegs, ut_allocator<TrxUndoRsegs>>,
    TrxUndoRsegs>
    purge_pq_t;
