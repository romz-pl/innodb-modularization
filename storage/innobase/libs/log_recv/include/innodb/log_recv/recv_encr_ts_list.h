#pragma once

#include <innodb/univ/univ.h>

#include <list>

/** A list of tablespaces for which (un)encryption process was not
completed before crash. */
extern std::list<space_id_t> recv_encr_ts_list;
