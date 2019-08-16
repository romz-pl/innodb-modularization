#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/univ/rec_t.h>

#include <iosfwd>

/** Pretty-print a record.
@param[in,out]	o	output stream
@param[in]	rec	physical record
@param[in]	info	rec_get_info_bits(rec)
@param[in]	offsets	rec_get_offsets(rec) */
void rec_print(std::ostream &o, const rec_t *rec, ulint info,
               const ulint *offsets);


struct dict_index_t;

/** Prints a physical record. */
void rec_print(FILE *file,                 /*!< in: file where to print */
               const rec_t *rec,           /*!< in: physical record */
               const dict_index_t *index); /*!< in: record descriptor */


#endif



