#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

/** The maximum lsn we see for a page during the recovery process. If this
is bigger than the lsn we are able to scan up to, that is an indication that
the recovery failed and the database may be corrupt. */
extern lsn_t recv_max_page_lsn;
