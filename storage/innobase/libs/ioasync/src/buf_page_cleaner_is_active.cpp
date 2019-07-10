#include <innodb/ioasync/buf_page_cleaner_is_active.h>

/** Flag indicating if the page_cleaner is in active state. This flag
is set to TRUE by the page_cleaner thread when it is spawned and is set
back to FALSE at shutdown by the page_cleaner as well. Therefore no
need to protect it by a mutex. It is only ever read by the thread
doing the shutdown */
bool buf_page_cleaner_is_active = false;
