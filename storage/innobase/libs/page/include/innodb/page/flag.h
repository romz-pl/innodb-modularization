#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/flags.h>
#include <innodb/page/header.h>

/*
 * Page HEAP Flags
 */

/* Heap numbers */
#define PAGE_HEAP_NO_INFIMUM 0  /* page infimum */
#define PAGE_HEAP_NO_SUPREMUM 1 /* page supremum */
#define PAGE_HEAP_NO_USER_LOW        \
  2 /* first user record in          \
    creation (insertion) order,      \
    not necessarily collation order; \
    this record may have been deleted */



/*
 * Page ZIP Flags
 */




/** Size of an compressed page directory entry */
#define PAGE_ZIP_DIR_SLOT_SIZE 2

/* Default compression level. */
#define DEFAULT_COMPRESSION_LEVEL 6
/** Start offset of the area that will be compressed */
#define PAGE_ZIP_START PAGE_NEW_SUPREMUM_END
/** Size of an compressed page directory entry */
#define PAGE_ZIP_DIR_SLOT_SIZE 2
/** Predefine the sum of DIR_SLOT, TRX_ID & ROLL_PTR */
#define PAGE_ZIP_CLUST_LEAF_SLOT_SIZE \
  (PAGE_ZIP_DIR_SLOT_SIZE + DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN)
/** Mask of record offsets */
#define PAGE_ZIP_DIR_SLOT_MASK 0x3fff
/** 'owned' flag */
#define PAGE_ZIP_DIR_SLOT_OWNED 0x4000
/** 'deleted' flag */
#define PAGE_ZIP_DIR_SLOT_DEL 0x8000



/*
 * Page DIRECTORY Flags
 */

/* Offset of the directory start down from the page end. We call the
slot with the highest file address directory start, as it points to
the first record in the list of records. */
#define PAGE_DIR FIL_PAGE_DATA_END

/* We define a slot in the page directory as two bytes */
#define PAGE_DIR_SLOT_SIZE 2

/* The offset of the physically lower end of the directory, counted from
page end, when the page is empty */
#define PAGE_EMPTY_DIR_START (PAGE_DIR + 2 * PAGE_DIR_SLOT_SIZE)

/* The maximum and minimum number of records owned by a directory slot. The
number may drop below the minimum in the first and the last slot in the
directory. */
#define PAGE_DIR_SLOT_MAX_N_OWNED 8
#define PAGE_DIR_SLOT_MIN_N_OWNED 4


