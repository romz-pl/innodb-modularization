#pragma once

#include <innodb/univ/univ.h>

/** The status of online index creation */
enum online_index_status {
  /** the index is complete and ready for access */
  ONLINE_INDEX_COMPLETE = 0,
  /** the index is being created, online
  (allowing concurrent modifications) */
  ONLINE_INDEX_CREATION,
  /** secondary index creation was aborted and the index
  should be dropped as soon as index->table->n_ref_count reaches 0,
  or online table rebuild was aborted and the clustered index
  of the original table should soon be restored to
  ONLINE_INDEX_COMPLETE */
  ONLINE_INDEX_ABORTED,
  /** the online index creation was aborted, the index was
  dropped from the data dictionary and the tablespace, and it
  should be dropped from the data dictionary cache as soon as
  index->table->n_ref_count reaches 0. */
  ONLINE_INDEX_ABORTED_DROPPED
};
