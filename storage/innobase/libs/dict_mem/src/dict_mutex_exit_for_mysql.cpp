#include <innodb/dict_mem/dict_mutex_exit_for_mysql.h>

#include <innodb/dict_mem/dict_sys.h>

/** Releases the dictionary system mutex for MySQL. */
void dict_mutex_exit_for_mysql(void) {
    mutex_exit(&dict_sys->mutex);
}
