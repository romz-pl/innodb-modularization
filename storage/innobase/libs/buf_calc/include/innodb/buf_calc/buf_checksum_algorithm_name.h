#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_calc/srv_checksum_algorithm_t.h>

/** Return a printable string describing the checksum algorithm.
 @return algorithm name */
const char *buf_checksum_algorithm_name(
    srv_checksum_algorithm_t algo); /*!< in: algorithm */
