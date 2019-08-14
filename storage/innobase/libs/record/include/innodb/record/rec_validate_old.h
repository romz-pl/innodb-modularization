#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Validates the consistency of an old-style physical record.
 @return true if ok */
ibool rec_validate_old(const rec_t *rec); /*!< in: physical record */
