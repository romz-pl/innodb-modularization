#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the number of dir slots in directory.
 @return number of slots */
ulint page_dir_get_n_slots(const page_t *page); /*!< in: index page */
