#pragma once

#include <innodb/univ/univ.h>


int cmp_geometry_field(ulint mtype,           /*!< in: main type */
                              ulint prtype,          /*!< in: precise type */
                              const byte *a,         /*!< in: data field */
                              unsigned int a_length, /*!< in: data field length,
                                                     not UNIV_SQL_NULL */
                              const byte *b,         /*!< in: data field */
                              unsigned int b_length); /*!< in: data field length,
                                                     not UNIV_SQL_NULL */
