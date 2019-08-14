#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_dir_slot_t.h>
#include <innodb/univ/rec_t.h>

/** This is used to set the record offset in a directory slot.
@param[in]	rec	record on the page
@param[in]	slot	directory slot */
void page_dir_slot_set_rec(page_dir_slot_t *slot, rec_t *rec);
