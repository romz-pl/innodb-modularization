#pragma once

#include <innodb/univ/univ.h>

byte *page_zip_dir_find_low(byte *slot,   /*!< in: start of records */
                            byte *end,    /*!< in: end of records */
                            ulint offset); /*!< in: offset of user record */
