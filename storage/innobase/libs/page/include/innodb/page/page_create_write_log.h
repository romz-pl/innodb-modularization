#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_type_t.h>
#include <innodb/buf_frame/buf_frame_t.h>

struct mtr_t;


#ifndef UNIV_HOTBACKUP

void page_create_write_log(buf_frame_t *frame, mtr_t *mtr, ibool comp,
                           page_type_t page_type);

#else /* !UNIV_HOTBACKUP */

#define page_create_write_log(frame, mtr, comp, type) ((void)0)

#endif /* !UNIV_HOTBACKUP */
