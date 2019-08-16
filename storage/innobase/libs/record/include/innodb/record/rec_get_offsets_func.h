#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/memory/mem_heap_t.h>

struct dict_index_t;


/** The following function determines the offsets to each field
 in the record. It can reuse a previously allocated array.
 Note that after instant ADD COLUMN, if this is a record
 from clustered index, fields in the record may be less than
 the fields defined in the clustered index. So the offsets
 size is allocated according to the clustered index fields.
 @return the new offsets */
ulint *rec_get_offsets_func(
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets,            /*!< in/out: array consisting of
                               offsets[0] allocated elements,
                               or an array from rec_get_offsets(),
                               or NULL */
    ulint n_fields,            /*!< in: maximum number of
                              initialized fields
                               (ULINT_UNDEFINED if all fields) */
#ifdef UNIV_DEBUG
    const char *file,  /*!< in: file name where called */
    ulint line,        /*!< in: line number where called */
#endif                 /* UNIV_DEBUG */
    mem_heap_t **heap) /*!< in/out: memory heap */
    MY_ATTRIBUTE((warn_unused_result));

