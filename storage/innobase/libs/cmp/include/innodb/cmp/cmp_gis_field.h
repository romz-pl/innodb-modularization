#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_cur_mode_t.h>

#include "sql/dd/types/spatial_reference_system.h"

int cmp_gis_field(
    page_cur_mode_t mode,                    /*!< in: compare mode */
    const byte *a,                           /*!< in: data field */
    unsigned int a_length,                   /*!< in: data field length,
                                             not UNIV_SQL_NULL */
    const byte *b,                           /*!< in: data field */
    unsigned int b_length,                   /*!< in: data field length,
                                             not UNIV_SQL_NULL */
    const dd::Spatial_reference_system *srs); /*!< in: SRS of R-tree */
