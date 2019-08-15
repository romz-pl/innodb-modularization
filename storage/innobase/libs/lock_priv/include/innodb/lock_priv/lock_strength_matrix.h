#pragma once

#include <innodb/univ/univ.h>

/* STRONGER-OR-EQUAL RELATION (mode1=row, mode2=column)
 *    IS IX S  X  AI
 * IS +  -  -  -  -
 * IX +  +  -  -  -
 * S  +  -  +  -  -
 * X  +  +  +  +  +
 * AI -  -  -  -  +
 * See lock_mode_stronger_or_eq().
 */
static const byte lock_strength_matrix[5][5] = {
    /**         IS     IX       S     X       AI */
    /* IS */ {TRUE, FALSE, FALSE, FALSE, FALSE},
    /* IX */ {TRUE, TRUE, FALSE, FALSE, FALSE},
    /* S  */ {TRUE, FALSE, TRUE, FALSE, FALSE},
    /* X  */ {TRUE, TRUE, TRUE, TRUE, TRUE},
    /* AI */ {FALSE, FALSE, FALSE, FALSE, TRUE}};
