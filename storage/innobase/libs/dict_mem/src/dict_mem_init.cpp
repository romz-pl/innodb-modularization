#include <innodb/dict_mem/dict_mem_init.h>

#include <innodb/crc32/crc32.h>
#include <innodb/formatting/formatting.h>
#include <innodb/time/ut_time.h>
#include <innodb/dict_mem/dict_temp_file_num.h>

/** Initialize dict memory variables */
void dict_mem_init(void) {
  /* Initialize a randomly distributed temporary file number */
  ib_uint32_t now = static_cast<ib_uint32_t>(ut_time());

  const byte *buf = reinterpret_cast<const byte *>(&now);

  dict_temp_file_num = ut_crc32(buf, sizeof(now));

  DBUG_PRINT("dict_mem_init", ("Starting Temporary file number is " UINT32PF,
                               dict_temp_file_num));
}
