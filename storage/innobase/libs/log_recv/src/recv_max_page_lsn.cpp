#include <innodb/log_recv/recv_max_page_lsn.h>

/** The maximum lsn we see for a page during the recovery process. If this
is bigger than the lsn we are able to scan up to, that is an indication that
the recovery failed and the database may be corrupt. */
lsn_t recv_max_page_lsn;
