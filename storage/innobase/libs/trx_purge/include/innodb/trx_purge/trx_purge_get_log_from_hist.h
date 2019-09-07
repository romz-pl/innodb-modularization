#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/fil_addr_t.h>

/** Calculates the file address of an undo log header when we have the file
 address of its history list node.
 @return file address of the log */
fil_addr_t trx_purge_get_log_from_hist(
    fil_addr_t node_addr); /*!< in: file address of the history
                           list node of the log */
