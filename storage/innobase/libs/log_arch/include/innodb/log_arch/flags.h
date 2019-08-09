#pragma once

#include <innodb/univ/univ.h>

/** @name Archive file name prefix and constant length parameters. */
/* @{ */
/** Archive directory prefix */
const char ARCH_DIR[] = "#ib_archive";

/** Archive Log group directory prefix */
const char ARCH_LOG_DIR[] = "log_group_";

/** Archive Page group directory prefix */
const char ARCH_PAGE_DIR[] = "page_group_";

/** Archive log file prefix */
const char ARCH_LOG_FILE[] = "ib_log_";

/** Archive page file prefix */
const char ARCH_PAGE_FILE[] = "ib_page_";

/** //@} */


/** File name for the durable file which indicates whether a group was made
durable or not. Required to differentiate durable group from group left over by
crash during clone operation. */
constexpr char ARCH_PAGE_GROUP_DURABLE_FILE_NAME[] = "durable";

/** Byte length for printing LSN.
Each archive group name is appended with start LSN */
const uint MAX_LSN_DECIMAL_DIGIT = 32;

/** Max string length for archive log file name */
const uint MAX_ARCH_LOG_FILE_NAME_LEN =
    sizeof(ARCH_DIR) + 1 + sizeof(ARCH_LOG_DIR) + MAX_LSN_DECIMAL_DIGIT + 1 +
    sizeof(ARCH_LOG_FILE) + MAX_LSN_DECIMAL_DIGIT + 1;

/** Max string length for archive page file name */
const uint MAX_ARCH_PAGE_FILE_NAME_LEN =
    sizeof(ARCH_DIR) + 1 + sizeof(ARCH_PAGE_DIR) + MAX_LSN_DECIMAL_DIGIT + 1 +
    sizeof(ARCH_PAGE_FILE) + MAX_LSN_DECIMAL_DIGIT + 1;

/** Max string length for archive group directory name */
const uint MAX_ARCH_DIR_NAME_LEN =
    sizeof(ARCH_DIR) + 1 + sizeof(ARCH_PAGE_DIR) + MAX_LSN_DECIMAL_DIGIT + 1;

/** Memory block size */
constexpr uint ARCH_PAGE_BLK_SIZE = UNIV_PAGE_SIZE_DEF;


/** Archived page header file size (RESET Page) in number of blocks. */
constexpr uint ARCH_PAGE_FILE_NUM_RESET_PAGE = 1;

/** Archived file header size. No file header for this version. */
constexpr uint ARCH_PAGE_FILE_HDR_SIZE =
    ARCH_PAGE_FILE_NUM_RESET_PAGE * ARCH_PAGE_BLK_SIZE;

/** @name Page Archive doublewrite buffer file name prefix and constant length
parameters. //@{ */

/** Archive doublewrite buffer directory prefix */
constexpr char ARCH_DBLWR_DIR[] = "ib_dblwr";

/** Archive doublewrite buffer file prefix */
constexpr char ARCH_DBLWR_FILE[] = "dblwr_";

/** File name for the active file which indicates whether a group is active or
not. */
constexpr char ARCH_PAGE_GROUP_ACTIVE_FILE_NAME[] = "active";

/** Archive doublewrite buffer number of files */
constexpr uint ARCH_DBLWR_NUM_FILES = 1;

/** Archive doublewrite buffer file capacity in no. of blocks */
constexpr uint ARCH_DBLWR_FILE_CAPACITY = 3;

/** //@} */

/** @name Archive block header elements //@{ */

/** Block Header: Version is in first 1 byte. */
constexpr uint ARCH_PAGE_BLK_HEADER_VERSION_OFFSET = 0;

/** Block Header: Block Type is in next 1 byte. */
constexpr uint ARCH_PAGE_BLK_HEADER_TYPE_OFFSET = 1;

/** Block Header: Checksum is in next 4 bytes. */
constexpr uint ARCH_PAGE_BLK_HEADER_CHECKSUM_OFFSET = 2;

/** Block Header: Data length is in next 2 bytes. */
constexpr uint ARCH_PAGE_BLK_HEADER_DATA_LEN_OFFSET = 6;

/** Block Header: Stop LSN is in next 8 bytes */
constexpr uint ARCH_PAGE_BLK_HEADER_STOP_LSN_OFFSET = 8;

/** Block Header: Reset LSN is in next 8 bytes */
constexpr uint ARCH_PAGE_BLK_HEADER_RESET_LSN_OFFSET = 16;

/** Block Header: Block number is in next 8 bytes */
constexpr uint ARCH_PAGE_BLK_HEADER_NUMBER_OFFSET = 24;

/** Block Header: Total length.
Keep header length in multiple of #ARCH_BLK_PAGE_ID_SIZE */
constexpr uint ARCH_PAGE_BLK_HEADER_LENGTH = 32;

/** //@} */

/** @name Page Archive reset block elements size. //@{ */

/** Serialized Reset ID: Reset LSN total size */
constexpr uint ARCH_PAGE_FILE_HEADER_RESET_LSN_SIZE = 8;

/** Serialized Reset ID: Reset block number size */
constexpr uint ARCH_PAGE_FILE_HEADER_RESET_BLOCK_NUM_SIZE = 2;

/** Serialized Reset ID: Reset block offset size */
constexpr uint ARCH_PAGE_FILE_HEADER_RESET_BLOCK_OFFSET_SIZE = 2;

/** Serialized Reset ID: Reset position total size */
constexpr uint ARCH_PAGE_FILE_HEADER_RESET_POS_SIZE =
    ARCH_PAGE_FILE_HEADER_RESET_BLOCK_NUM_SIZE +
    ARCH_PAGE_FILE_HEADER_RESET_BLOCK_OFFSET_SIZE;

/** //@} */

/** @name Page Archive data block elements //@{ */

/** Serialized page ID: tablespace ID in First 4 bytes */
constexpr uint ARCH_BLK_SPCE_ID_OFFSET = 0;

/** Serialized page ID: Page number in next 4 bytes */
constexpr uint ARCH_BLK_PAGE_NO_OFFSET = 4;

/** Serialized page ID: Total length */
constexpr uint ARCH_BLK_PAGE_ID_SIZE = 8;

/** //@} */

/** Number of memory blocks */
constexpr uint ARCH_PAGE_NUM_BLKS = 32;

/** Archived file format version */
constexpr uint ARCH_PAGE_FILE_VERSION = 1;

#ifdef UNIV_DEBUG
/** Archived page file default size in number of blocks. */
extern uint ARCH_PAGE_FILE_CAPACITY;

/** Archived page data file size (without header) in number of blocks. */
extern uint ARCH_PAGE_FILE_DATA_CAPACITY;
#else
/** Archived page file default size in number of blocks. */
constexpr uint ARCH_PAGE_FILE_CAPACITY =
    (ARCH_PAGE_BLK_SIZE - ARCH_PAGE_BLK_HEADER_LENGTH) / ARCH_BLK_PAGE_ID_SIZE;

/** Archived page data file size (without header) in number of blocks. */
constexpr uint ARCH_PAGE_FILE_DATA_CAPACITY =
    ARCH_PAGE_FILE_CAPACITY - ARCH_PAGE_FILE_NUM_RESET_PAGE;
#endif

/** Threshold for page archive reset. Attach to current reset if the number of
tracked pages between the reset request and the current reset is less than this
threshold as we allow only one reset per data block. */
constexpr uint ARCH_PAGE_RESET_THRESHOLD =
    (ARCH_PAGE_BLK_SIZE - ARCH_PAGE_BLK_HEADER_LENGTH) / ARCH_BLK_PAGE_ID_SIZE;

#ifdef UNIV_DEBUG
/** Archived page file default size in number of blocks. */
uint ARCH_PAGE_FILE_CAPACITY =
    (ARCH_PAGE_BLK_SIZE - ARCH_PAGE_BLK_HEADER_LENGTH) / ARCH_BLK_PAGE_ID_SIZE;

/** Archived page data file size (without header) in number of blocks. */
uint ARCH_PAGE_FILE_DATA_CAPACITY =
    ARCH_PAGE_FILE_CAPACITY - ARCH_PAGE_FILE_NUM_RESET_PAGE;
#endif
