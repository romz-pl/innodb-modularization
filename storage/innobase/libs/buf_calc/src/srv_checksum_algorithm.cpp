#include <innodb/buf_calc/srv_checksum_algorithm.h>

#include <innodb/buf_calc/srv_checksum_algorithm_t.h>

/** the macro MYSQL_SYSVAR_ENUM() requires "long unsigned int" and if we
use srv_checksum_algorithm_t here then we get a compiler error:
ha_innodb.cc:12251: error: cannot convert 'srv_checksum_algorithm_t*' to
  'long unsigned int*' in initialization */
ulong srv_checksum_algorithm = SRV_CHECKSUM_ALGORITHM_INNODB;
