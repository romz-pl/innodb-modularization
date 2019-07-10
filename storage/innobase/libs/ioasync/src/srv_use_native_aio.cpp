#include <innodb/ioasync/srv_use_native_aio.h>

/* If this flag is TRUE, then we will use the native aio of the
OS (provided we compiled Innobase with it in), otherwise we will
use simulated aio we build below with threads.
Currently we support native aio on windows and linux */
#ifdef _WIN32
bool srv_use_native_aio = TRUE; /* enabled by default on Windows */
#else
bool srv_use_native_aio;
#endif
