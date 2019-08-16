#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/univ/page_cur_mode_t.h>

#include "sql/dd/types/spatial_reference_system.h"

struct dtuple_t;

/** Compare a GIS data tuple to a physical record.
@param[in] dtuple data tuple
@param[in] rec B-tree record
@param[in] offsets rec_get_offsets(rec)
@param[in] mode compare mode
@param[in] srs Spatial reference system of R-tree
@retval negative if dtuple is less than rec */
int cmp_dtuple_rec_with_gis(const dtuple_t *dtuple, const rec_t *rec,
                            const ulint *offsets, page_cur_mode_t mode,
                            const dd::Spatial_reference_system *srs);
