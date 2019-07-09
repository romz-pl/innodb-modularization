#include <innodb/io/os_file_read_string.h>

/** Rewind file to its start, read at most size - 1 bytes from it to str, and
NUL-terminate str. All errors are silently ignored. This function is
mostly meant to be used with temporary files.
@param[in,out]	file		File to read from
@param[in,out]	str		Buffer where to read
@param[in]	size		Size of buffer */
void os_file_read_string(FILE *file, char *str, ulint size) {
  if (size != 0) {
    rewind(file);

    size_t flen = fread(str, 1, size - 1, file);

    str[flen] = '\0';
  }
}
