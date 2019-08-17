#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_BTR_PRINT

/** This is used to print the contents of the directory for
 debugging purposes. */
void page_dir_print(page_t *page, /*!< in: index page */
                    ulint pr_n);  /*!< in: print n first and n last entries */

#endif                               /* UNIV_BTR_PRINT */
#endif                               /* !UNIV_HOTBACKUP */
