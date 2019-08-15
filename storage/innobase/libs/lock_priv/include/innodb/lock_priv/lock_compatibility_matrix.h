#pragma once

#include <innodb/univ/univ.h>

/* LOCK COMPATIBILITY MATRIX
 *    IS IX S  X  AI
 * IS +	 +  +  -  +
 * IX +	 +  -  -  +
 * S  +	 -  +  -  -
 * X  -	 -  -  -  -
 * AI +	 +  -  -  -
 *
 * Note that for rows, InnoDB only acquires S or X locks.
 * For tables, InnoDB normally acquires IS or IX locks.
 * S or X table locks are only acquired for LOCK TABLES.
 * Auto-increment (AI) locks are needed because of
 * statement-level MySQL binlog.
 * See also lock_mode_compatible().
 */
static const byte lock_compatibility_matrix[5][5] = {
    /**         IS     IX       S     X       AI */
    /* IS */ {TRUE, TRUE, TRUE, FALSE, TRUE},
    /* IX */ {TRUE, TRUE, FALSE, FALSE, TRUE},
    /* S  */ {TRUE, FALSE, TRUE, FALSE, FALSE},
    /* X  */ {FALSE, FALSE, FALSE, FALSE, FALSE},
    /* AI */ {TRUE, TRUE, FALSE, FALSE, FALSE}};
