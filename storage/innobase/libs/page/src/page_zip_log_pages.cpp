#include <innodb/page/page_zip_log_pages.h>

/* Whether or not to log compressed page images to avoid possible
compression algorithm changes in zlib. */
bool page_zip_log_pages = true;
