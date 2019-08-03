#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Looks for the directory slot which owns the given record.
 @return the directory slot number */
ulint page_dir_find_owner_slot(
    const rec_t *rec); /*!< in: the physical record */
