#include <innodb/io/os_file_set_eof.h>

#include <unistd.h>
#include <sys/types.h>

/** Truncates a file at its current position.
@return true if success */
bool os_file_set_eof(FILE *file) /*!< in: file to be truncated */
{
  return (!ftruncate(fileno(file), ftell(file)));
}
