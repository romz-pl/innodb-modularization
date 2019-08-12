#pragma once

#include <innodb/univ/univ.h>

#include <innodb/data_types/spatial_status_t.h>

struct dfield_t;

/** Gets spatial status for "external storage"
@param[in,out]	field		field */
spatial_status_t dfield_get_spatial_status(const dfield_t *field);
