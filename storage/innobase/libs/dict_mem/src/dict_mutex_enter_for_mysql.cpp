#include <innodb/dict_mem/dict_mutex_enter_for_mysql.h>

#include <innodb/dict_mem/dict_sys.h>

/** Reserves the dictionary system mutex for MySQL. */
void dict_mutex_enter_for_mysql(void) {
    mutex_enter(&dict_sys->mutex);
}
