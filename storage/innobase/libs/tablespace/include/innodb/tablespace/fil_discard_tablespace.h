#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>


/** Discards a single-table tablespace. The tablespace must be cached in the
memory cache. Discarding is like deleting a tablespace, but

 1. We do not drop the table from the data dictionary;

 2. We remove all insert buffer entries for the tablespace immediately;
    in DROP TABLE they are only removed gradually in the background;

 3. When the user does IMPORT TABLESPACE, the tablespace will have the
    same id as it originally had.

 4. Free all the pages in use by the tablespace if rename=true.
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_discard_tablespace(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
