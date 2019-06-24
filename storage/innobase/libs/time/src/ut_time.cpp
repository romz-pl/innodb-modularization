#include <innodb/time/ut_time.h>

/** Returns system time. We do not specify the format of the time returned:
 the only way to manipulate it is to use the function ut_difftime.
 @return system time */
ib_time_t ut_time(void) {
    return (time(NULL));
}
