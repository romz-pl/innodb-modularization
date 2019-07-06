#pragma once

#include <innodb/univ/univ.h>

/** Alternatives for srv_checksum_algorithm, which can be changed by
setting innodb_checksum_algorithm */
enum srv_checksum_algorithm_t {
  SRV_CHECKSUM_ALGORITHM_CRC32,         /*!< Write crc32, allow crc32,
                                        innodb or none when reading */
  SRV_CHECKSUM_ALGORITHM_STRICT_CRC32,  /*!< Write crc32, allow crc32
                                        when reading */
  SRV_CHECKSUM_ALGORITHM_INNODB,        /*!< Write innodb, allow crc32,
                                        innodb or none when reading */
  SRV_CHECKSUM_ALGORITHM_STRICT_INNODB, /*!< Write innodb, allow
                                        innodb when reading */
  SRV_CHECKSUM_ALGORITHM_NONE,          /*!< Write none, allow crc32,
                                        innodb or none when reading */
  SRV_CHECKSUM_ALGORITHM_STRICT_NONE    /*!< Write none, allow none
                                        when reading */
};

inline bool is_checksum_strict(srv_checksum_algorithm_t algo) {
  return (algo == SRV_CHECKSUM_ALGORITHM_STRICT_CRC32 ||
          algo == SRV_CHECKSUM_ALGORITHM_STRICT_INNODB ||
          algo == SRV_CHECKSUM_ALGORITHM_STRICT_NONE);
}

inline bool is_checksum_strict(ulint algo) {
  return (algo == SRV_CHECKSUM_ALGORITHM_STRICT_CRC32 ||
          algo == SRV_CHECKSUM_ALGORITHM_STRICT_INNODB ||
          algo == SRV_CHECKSUM_ALGORITHM_STRICT_NONE);
}
