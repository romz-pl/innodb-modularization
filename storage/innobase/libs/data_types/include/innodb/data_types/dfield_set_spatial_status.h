#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/spatial_status_t.h>

struct dfield_t;

/** Sets spatial status for "external storage"
@param[in,out]	field		field
@param[in]	spatial_status	spatial status */
void dfield_set_spatial_status(dfield_t *field,
                               spatial_status_t spatial_status);
