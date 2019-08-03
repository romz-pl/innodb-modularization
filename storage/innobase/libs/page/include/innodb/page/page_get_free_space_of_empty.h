#pragma once

#include <innodb/univ/univ.h>

/** Calculates free space if a page is emptied.
 @return free space */
ulint page_get_free_space_of_empty(
    ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((const));
