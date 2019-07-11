#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/type.h>

/* The physical size of a list base node in bytes */
constexpr ulint FLST_BASE_NODE_SIZE = 4 + 2 * FIL_ADDR_SIZE;

/* The physical size of a list node in bytes */
constexpr ulint FLST_NODE_SIZE = 2 * FIL_ADDR_SIZE;


/*			SPACE HEADER
                        ============

File space header data structure: this data structure is contained in the
first page of a space. The space for this header is reserved in every extent
descriptor page, but used only in the first. */

/*-------------------------------------*/
#define FSP_SPACE_ID 0 /* space id */
#define FSP_NOT_USED                      \
  4 /* this field contained a value up to \
    which we know that the modifications  \
    in the database have been flushed to  \
    the file space; not used now */
#define FSP_SIZE                    \
  8 /* Current size of the space in \
    pages */
#define FSP_FREE_LIMIT                       \
  12 /* Minimum page number for which the    \
     free list has not been initialized:     \
     the pages >= this limit are, by         \
     definition, free; note that in a        \
     single-table tablespace where size      \
     < 64 pages, this number is 64, i.e.,    \
     we have initialized the space           \
     about the first extent, but have not    \
     physically allocated those pages to the \
     file */
#define FSP_SPACE_FLAGS               \
  16 /* fsp_space_t.flags, similar to \
     dict_table_t::flags */
#define FSP_FRAG_N_USED                            \
  20                /* number of used pages in the \
                    FSP_FREE_FRAG list */
#define FSP_FREE 24 /* list of free extents */
#define FSP_FREE_FRAG (24 + FLST_BASE_NODE_SIZE)
/* list of partially free extents not
belonging to any segment */
#define FSP_FULL_FRAG (24 + 2 * FLST_BASE_NODE_SIZE)
/* list of full extents not belonging
to any segment */
#define FSP_SEG_ID (24 + 3 * FLST_BASE_NODE_SIZE)
/* 8 bytes which give the first unused
segment id */
#define FSP_SEG_INODES_FULL (32 + 3 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
headers, where all the segment inode
slots are reserved */
#define FSP_SEG_INODES_FREE (32 + 4 * FLST_BASE_NODE_SIZE)
/* list of pages containing segment
headers, where not all the segment
header slots are reserved */
/*-------------------------------------*/
/* File space header size */
#define FSP_HEADER_SIZE (32 + 5 * FLST_BASE_NODE_SIZE)

#define FSP_FREE_ADD                    \
  4 /* this many free extents are added \
    to the free list from above         \
    FSP_FREE_LIMIT at a time */
/* @} */

/** Offset of the space header within a file page */
#define FSP_HEADER_OFFSET FIL_PAGE_DATA
