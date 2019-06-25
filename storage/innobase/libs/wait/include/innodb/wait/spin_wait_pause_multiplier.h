#pragma once

#include <innodb/univ/univ.h>

namespace ut {
/** The current value of @@innodb_spin_wait_pause_multiplier. Determines
how many PAUSE instructions to emit for each requested unit of delay
when calling `ut_delay(delay)`. The default value of 50 causes `delay*50` PAUSES
which was equivalent to `delay` microseconds on 100 MHz Pentium + Visual C++.
Useful on processors which have "non-standard" duration of a single PAUSE
instruction - one can compensate for longer PAUSES by setting the
spin_wait_pause_multiplier to a smaller value on such machine */
extern ulong spin_wait_pause_multiplier;
}  // namespace ut
