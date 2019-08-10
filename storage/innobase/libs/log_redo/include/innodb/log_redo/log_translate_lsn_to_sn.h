#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>


#ifndef UNIV_HOTBACKUP

/** Calculates sn value for given lsn value.
@see log_translate_sn_to_lsn
@param[in]	lsn	lsn value
@return sn value for the provided lsn value */
lsn_t log_translate_lsn_to_sn(lsn_t lsn);

#endif
