#include <innodb/buffer/buf_withdraw_clock.h>

/** the clock is incremented every time a pointer to a page may become obsolete;
if the withdrwa clock has not changed, the pointer is still valid in buffer
pool. if changed, the pointer might not be in buffer pool any more. */
volatile ulint buf_withdraw_clock;
