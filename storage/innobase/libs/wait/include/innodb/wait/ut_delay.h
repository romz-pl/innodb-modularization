#pragma once

#include <innodb/univ/univ.h>

/** Runs an idle loop on CPU. The argument gives the desired delay
 in microseconds on 100 MHz Pentium + Visual C++.
 The actual duration depends on a product of `delay` and the current value of
 @@innodb_spin_wait_pause_multiplier.
 @param[in]   delay   delay in microseconds on 100 MHz Pentium, assuming
                      spin_wait_pause_multiplier is 50 (default).
 @return dummy value */
ulint ut_delay(ulint delay);
