#pragma once

#include <innodb/univ/univ.h>

#define FSP_DICT_HDR_PAGE_NO    \
  7 /*!< data dictionary header \
    page, in tablespace 0 */

/* Space id and page no where the dictionary header resides */
#define DICT_HDR_SPACE 0 /* the SYSTEM tablespace */
#define DICT_HDR_PAGE_NO FSP_DICT_HDR_PAGE_NO

/* The ibuf table and indexes's ID are assigned as the number
DICT_IBUF_ID_MIN plus the space id */
#define DICT_IBUF_ID_MIN 0xFFFFFFFF00000000ULL


/** Shift for spatial status */
#define SPATIAL_STATUS_SHIFT 12

/** Mask to encode/decode spatial status. */
#define SPATIAL_STATUS_MASK (3 << SPATIAL_STATUS_SHIFT)

/** Maximum indexed field length for tables that have atomic BLOBs.
This (3072) is the maximum index row length allowed, so we cannot create index
prefix column longer than that. */
#define REC_VERSION_56_MAX_INDEX_COL_LEN 3072

#if SPATIAL_STATUS_MASK < REC_VERSION_56_MAX_INDEX_COL_LEN
#error SPATIAL_STATUS_MASK < REC_VERSION_56_MAX_INDEX_COL_LEN
#endif


/* The insert buffer tree itself is always located in space 0. */
#define IBUF_SPACE_ID static_cast<space_id_t>(0)

/** Prefix for tmp tables, adopted from sql/table.h */
#define TEMP_FILE_PREFIX "#sql"
#define TEMP_FILE_PREFIX_LENGTH 4
#define TEMP_FILE_PREFIX_INNODB "#sql-ib"

#define TEMP_TABLE_PREFIX "#sql"
#define TEMP_TABLE_PATH_PREFIX "/" TEMP_TABLE_PREFIX




/* The ids for the basic system tables and their indexes */
#define DICT_TABLES_ID 1
#define DICT_COLUMNS_ID 2
#define DICT_INDEXES_ID 3
#define DICT_FIELDS_ID 4
/* The following is a secondary index on SYS_TABLES */
#define DICT_TABLE_IDS_ID 5
