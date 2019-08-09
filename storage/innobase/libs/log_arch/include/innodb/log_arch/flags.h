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
