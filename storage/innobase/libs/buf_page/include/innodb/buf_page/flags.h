#pragma once

#include <innodb/univ/univ.h>

/** Number of bits to representing a buffer pool ID */
constexpr ulint MAX_BUFFER_POOLS_BITS = 6;

/** The maximum number of buffer pools that can be defined */
constexpr ulint MAX_BUFFER_POOLS = (1 << MAX_BUFFER_POOLS_BITS);

/* the number of purge threads to use from the worker pool (currently 0 or 1) */
extern ulong srv_n_purge_threads;

/** Maximum number of concurrent buffer pool watches */
#define BUF_POOL_WATCH_SIZE (srv_n_purge_threads + 1)

/** The maximum number of page_hash locks */
constexpr ulint MAX_PAGE_HASH_LOCKS = 1024;

/** Parameters of binary buddy system for compressed pages (buf0buddy.h) */
/* @{ */
/** Zip shift value for the smallest page size */
#define BUF_BUDDY_LOW_SHIFT UNIV_ZIP_SIZE_SHIFT_MIN

/** Smallest buddy page size */
#define BUF_BUDDY_LOW (1U << BUF_BUDDY_LOW_SHIFT)

/** Actual number of buddy sizes based on current page size */
#define BUF_BUDDY_SIZES (UNIV_PAGE_SIZE_SHIFT - BUF_BUDDY_LOW_SHIFT)

/** Maximum number of buddy sizes based on the max page size */
#define BUF_BUDDY_SIZES_MAX (UNIV_PAGE_SIZE_SHIFT_MAX - BUF_BUDDY_LOW_SHIFT)

/** twice the maximum block size of the buddy system;
the underlying memory is aligned by this amount:
this must be equal to UNIV_PAGE_SIZE */
#define BUF_BUDDY_HIGH (BUF_BUDDY_LOW << BUF_BUDDY_SIZES)
/* @} */




/** The buffer pool dump/load file name */
#define SRV_BUF_DUMP_FILENAME_DEFAULT "ib_buffer_pool"

