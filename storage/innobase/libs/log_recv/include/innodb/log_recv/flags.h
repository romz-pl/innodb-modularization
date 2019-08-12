#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/flags.h>

/** Size of the parsing buffer; it must accommodate RECV_SCAN_SIZE many
times! */
#define RECV_PARSING_BUF_SIZE (2 * 1024 * 1024)

/** Size of block reads when the log groups are scanned forward to do a
roll-forward */
#define RECV_SCAN_SIZE (4 * UNIV_PAGE_SIZE)

/** Log records are stored in the hash table in chunks at most of this size;
this must be less than UNIV_PAGE_SIZE as it is stored in the buffer pool */
#define RECV_DATA_BLOCK_SIZE (MEM_MAX_ALLOC_IN_BUF - sizeof(recv_data_t))

/** Read-ahead area in applying log records to file pages */
const size_t RECV_READ_AHEAD_AREA = 32;


/** Copy of the LOG_HEADER_CREATOR field. */
char log_header_creator[LOG_HEADER_CREATOR_END - LOG_HEADER_CREATOR + 1];
