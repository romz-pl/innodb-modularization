#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_BTR_PRINT

/** Prints the info in a page header. */
void page_header_print(const page_t *page); /*!< in: index page */


#endif                               /* UNIV_BTR_PRINT */
#endif                               /* !UNIV_HOTBACKUP */
