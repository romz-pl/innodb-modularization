#include <innodb/dict_mem/dict_temp_file_num.h>

/** An interger randomly initialized at startup used to make a temporary
table name as unuique as possible. */
ib_uint32_t dict_temp_file_num = 0;
