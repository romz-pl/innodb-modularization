#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include "sql/dd/types/spatial_reference_system.h"

struct dtuple_t;

/** Compare a GIS data tuple to a physical record in rtree non-leaf node.
We need to check the page number field, since we don't store pk field in
rtree non-leaf node.
@param[in]	dtuple	data tuple
@param[in]	rec	R-tree record
@param[in]	offsets	rec_get_offsets(rec)
@retval negative if dtuple is less than rec */
int cmp_dtuple_rec_with_gis_internal(const dtuple_t *dtuple, const rec_t *rec,
                                     const ulint *offsets,
                                     const dd::Spatial_reference_system *srs);
