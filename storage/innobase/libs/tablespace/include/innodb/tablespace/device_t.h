#pragma once

#include <innodb/univ/univ.h>

/** Types of raw partitions in innodb_data_file_path */
enum device_t {

  /** Not a raw partition */
  SRV_NOT_RAW = 0,

  /** A 'newraw' partition, only to be initialized */
  SRV_NEW_RAW,

  /** An initialized raw partition */
  SRV_OLD_RAW
};
