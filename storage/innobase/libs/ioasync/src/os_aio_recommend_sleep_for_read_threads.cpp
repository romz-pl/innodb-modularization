#include <innodb/ioasync/os_aio_recommend_sleep_for_read_threads.h>

/** If the following is true, read i/o handler threads try to
wait until a batch of new read requests have been posted */
bool os_aio_recommend_sleep_for_read_threads = false;
