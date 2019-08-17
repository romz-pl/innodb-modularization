#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/univ/rec_t.h>

/** Prints record contents including the data relevant only in
 the index page context. */
void page_rec_print(const rec_t *rec,      /*!< in: physical record */
                    const ulint *offsets); /*!< in: record descriptor */

#endif
