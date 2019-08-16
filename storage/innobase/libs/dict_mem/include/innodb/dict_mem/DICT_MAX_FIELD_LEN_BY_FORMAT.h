#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/dict_table_has_atomic_blobs.h>

/** Find out maximum indexed column length by its table format.
For ROW_FORMAT=REDUNDANT and ROW_FORMAT=COMPACT, the maximum
field length is REC_ANTELOPE_MAX_INDEX_COL_LEN - 1 (767). For
ROW_FORMAT=COMPRESSED and ROW_FORMAT=DYNAMIC, the length could
be REC_VERSION_56_MAX_INDEX_COL_LEN (3072) bytes */
#define DICT_MAX_FIELD_LEN_BY_FORMAT(table)                              \
  (dict_table_has_atomic_blobs(table) ? REC_VERSION_56_MAX_INDEX_COL_LEN \
                                      : REC_ANTELOPE_MAX_INDEX_COL_LEN - 1)
