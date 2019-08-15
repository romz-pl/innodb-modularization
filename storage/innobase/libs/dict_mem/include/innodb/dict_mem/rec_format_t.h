#pragma once

#include <innodb/univ/univ.h>

/** Innodb row types are a subset of the MySQL global enum row_type.
They are made into their own enum so that switch statements can account
for each of them. */
enum rec_format_enum {
  REC_FORMAT_REDUNDANT = 0,  /*!< REDUNDANT row format */
  REC_FORMAT_COMPACT = 1,    /*!< COMPACT row format */
  REC_FORMAT_COMPRESSED = 2, /*!< COMPRESSED row format */
  REC_FORMAT_DYNAMIC = 3     /*!< DYNAMIC row format */
};

typedef enum rec_format_enum rec_format_t;
