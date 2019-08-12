#include <innodb/buf_calc/buf_checksum_algorithm_name.h>

#include <innodb/error/ut_error.h>

/** Return a printable string describing the checksum algorithm.
 @return algorithm name */
const char *buf_checksum_algorithm_name(
    srv_checksum_algorithm_t algo) /*!< in: algorithm */
{
  switch (algo) {
    case SRV_CHECKSUM_ALGORITHM_CRC32:
      return ("crc32");
    case SRV_CHECKSUM_ALGORITHM_STRICT_CRC32:
      return ("strict_crc32");
    case SRV_CHECKSUM_ALGORITHM_INNODB:
      return ("innodb");
    case SRV_CHECKSUM_ALGORITHM_STRICT_INNODB:
      return ("strict_innodb");
    case SRV_CHECKSUM_ALGORITHM_NONE:
      return ("none");
    case SRV_CHECKSUM_ALGORITHM_STRICT_NONE:
      return ("strict_none");
  }

  ut_error;
}
