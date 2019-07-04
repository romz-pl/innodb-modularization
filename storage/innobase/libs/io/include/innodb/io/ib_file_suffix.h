#pragma once

#include <innodb/univ/univ.h>

/** Common InnoDB file extentions */
enum ib_file_suffix { NO_EXT = 0, IBD = 1, CFG = 2, CFP = 3, IBT = 4, IBU = 5 };

extern const char *dot_ext[];

#define DOT_IBD dot_ext[IBD]
#define DOT_CFG dot_ext[CFG]
#define DOT_CFP dot_ext[CFP]
#define DOT_IBT dot_ext[IBT]
#define DOT_IBU dot_ext[IBU]
